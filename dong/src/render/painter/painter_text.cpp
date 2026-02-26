#include "../painter.hpp"
#include "painter_style_utils.hpp"
#include <cstdlib>
#include <unordered_set>
#include "../../layout/layout_engine.hpp"
#include "../../layout/sticky_positioning.hpp"
#include "../../core/log.h"
#include "../../dom/select_element.hpp"


namespace dong::render {

// Forward declaration for select rendering
void renderSelect(
    const dom::DOMNodePtr& node,
    const layout::LayoutNode* layout,
    const dom::ComputedStyle& style,
    float bl, float bt, float br, float bb,
    TextShaper& shaper,
    DisplayListBuilder& builder
);

namespace {

using painter_detail::collapseWhitespace;
using painter_detail::fillTextShadow;
using painter_detail::makeColorFromCss;
using painter_detail::toLowerCopy;

inline void getPaintOrigin(const layout::LayoutNode* layout, float& out_x, float& out_y) {
    out_x = layout ? layout->layout.position[0] : 0.0f;
    out_y = layout ? layout->layout.position[1] : 0.0f;
    if (layout && layout->sticky_metadata && layout->sticky_metadata->is_stuck) {
        out_x = layout->sticky_metadata->visual_x;
        out_y = layout->sticky_metadata->visual_y;
    }
}

// ========== 内容分析 ==========
struct ContentAnalysis {
    bool has_text_child = false;
    bool has_inline_element_child = false;
    size_t raw_text_len = 0;
};

ContentAnalysis analyzeChildren(const dom::DOMNodePtr& node) {
    ContentAnalysis result;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            result.raw_text_len += child->getRawTextContent().size();
            result.has_text_child = true;
        } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            const auto& cs = child->getComputedStyle();
            if (cs.display == "inline" || cs.display == "inline-block") {
                result.has_inline_element_child = true;
            }
        }
    }
    return result;
}

bool tagPrefersText(const std::string& tag) {
    static const std::unordered_set<std::string> kTextPreferredTags = {
        "h1", "h2", "h3", "h4", "h5", "h6",
        "p", "span", "button", "code", "div", "footer", "label",
        "li", "dt", "dd", "td", "th", "figcaption", "blockquote",
        "a", "em", "strong", "small", "b", "i", "u", "s",
        "cite", "mark", "sub", "sup", "abbr", "time", "data"
    };
    return kTextPreferredTags.count(tag) > 0;
}

enum class TextRenderPath { Mixed, FullText, None };

TextRenderPath determinePath(const ContentAnalysis& analysis,
                             const std::string& tag,
                             const dom::ComputedStyle& style) {
    if (!analysis.has_text_child) return TextRenderPath::None;

    bool is_flex = (style.display == "flex" || style.display == "inline-flex");
    bool prefers = tagPrefersText(tag);

    if (analysis.has_text_child && analysis.has_inline_element_child && prefers && !is_flex) {
        return TextRenderPath::Mixed;
    }
    if (prefers || !analysis.has_inline_element_child) {
        return TextRenderPath::FullText;
    }
    return TextRenderPath::None;
}

// ========== 混合路径渲染 ==========
struct MixedPathState {
    float x = 0, y = 0, inner_width = 0;
    float pad_left = 0, pad_top = 0;
    float baseline_y = 0;
    float cumulative_x = 0;
    float container_font_size = 16.0f;
    float ascent_px = 0.0f;
    float line_height_px = 0.0f;
    Color text_color;
};

MixedPathState initMixedPath(const layout::LayoutNode* layout,
                             const dom::ComputedStyle& style,
                             float bl, float bt, float br, float bb,
                             TextShaper& shaper) {
    MixedPathState s;
    float origin_x = 0.0f, origin_y = 0.0f;
    getPaintOrigin(layout, origin_x, origin_y);
    s.x = origin_x + bl;
    s.y = origin_y + bt;
    float width = std::max(0.0f, layout->layout.dimensions[0] - bl - br);
    s.pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
    s.inner_width = std::max(0.0f, width - s.pad_left - pad_right);
    s.pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    s.container_font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
    s.text_color = makeColorFromCss(style.color);

    // Compute baseline
    TextShapeRequest req{"X", style.font_family, style.font_weight, style.font_style, s.container_font_size};
    ShapedText shaped;
    if (shaper.shape(req, shaped)) {
        float scale = shaped.scale_to_pixels;
        float ascent = shaped.ascent_units > 0.0f ? shaped.ascent_units : s.container_font_size / scale;
        float line_h = shaped.line_height_units;
        if (style.line_height > 0.0f) {
            if (style.line_height_is_unitless) {
                line_h = (style.line_height * s.container_font_size) / std::max(scale, 1e-3f);
            } else {
                line_h = style.line_height / std::max(scale, 1e-3f);
            }
        }
        if (line_h <= 0.0f) line_h = s.container_font_size / scale;
        float descent_abs = shaped.descent_units < 0.0f ? -shaped.descent_units : 0.0f;
        float extra = std::max(line_h - (ascent + descent_abs), 0.0f);
        float baseline_offset = (extra * 0.5f + ascent) * scale;
        s.ascent_px = ascent * scale;
        s.line_height_px = line_h * scale;
        s.baseline_y = s.y + s.pad_top + baseline_offset;
    }
    return s;
}

// Measure the pixel width of a single space character in the container's font.
float measureSpaceWidth(const dom::ComputedStyle& style, float font_size, TextShaper& shaper) {
    TextShapeRequest req{" ", style.font_family, style.font_weight, style.font_style, font_size};
    ShapedText shaped;
    if (shaper.shape(req, shaped) && !shaped.glyphs.empty()) {
        return shaped.width_units * shaped.scale_to_pixels;
    }
    return font_size * 0.25f; // rough fallback
}

// Check whether the raw text of a node ends with whitespace (before collapse).
bool rawTextEndsWithSpace(const dom::DOMNodePtr& node) {
    const std::string& raw = node->getRawTextContent();
    if (raw.empty()) return false;
    return std::isspace(static_cast<unsigned char>(raw.back()));
}

// Check whether the raw text of a node starts with whitespace (before collapse).
bool rawTextStartsWithSpace(const dom::DOMNodePtr& node) {
    const std::string& raw = node->getRawTextContent();
    if (raw.empty()) return false;
    return std::isspace(static_cast<unsigned char>(raw.front()));
}

// Emit a single shaped text run at the current Mixed-path cursor position.
void emitInlineTextRun(const std::string& text,
                       const dom::ComputedStyle& cs,
                       const dom::ComputedStyle& container_style,
                       MixedPathState& state,
                       TextShaper& shaper,
                       DisplayListBuilder& builder) {
    if (text.empty()) return;

    float font_size = cs.font_size > 0.0f ? cs.font_size : state.container_font_size;
    std::string family = cs.font_family.empty() ? container_style.font_family : cs.font_family;
    std::string weight = cs.font_weight.empty() ? container_style.font_weight : cs.font_weight;
    std::string style  = cs.font_style.empty()  ? container_style.font_style  : cs.font_style;

    TextShapeRequest req{text, family, weight, style, font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    float scale = shaped.scale_to_pixels;
    float text_width = shaped.width_units * scale;
    float ascent = shaped.ascent_units > 0.0f ? shaped.ascent_units : font_size / scale;

    DrawGlyphRunData run;
    run.rect = {state.x + state.pad_left + state.cumulative_x,
                state.baseline_y - ascent * scale, text_width, state.line_height_px};
    run.color = makeColorFromCss(!cs.color.empty() ? cs.color : container_style.color);
    run.font_size = font_size;
    run.font_family = family;
    run.font_weight = weight;
    run.font_style  = style;
    run.font_paths = shaped.font_paths;
    run.font_path = shaped.font_path;
    run.baseline_x = run.rect.x;
    run.baseline_y = state.baseline_y;
    run.units_per_em = shaped.units_per_em;
    run.scale_to_pixels = shaped.scale_to_pixels;
    fillTextShadow(run, cs);

    for (const auto& sg : shaped.glyphs) {
        run.glyphs.push_back({sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                              sg.font_path_index, sg.units_per_em});
    }
    builder.addGlyphRun(std::move(run));
    state.cumulative_x += text_width;
}

// Forward declaration for recursive inline rendering.
void renderInlineSubtree(const dom::DOMNodePtr& node,
                         const dom::ComputedStyle& effective_style,
                         const dom::ComputedStyle& container_style,
                         MixedPathState& state,
                         TextShaper& shaper,
                         DisplayListBuilder& builder);

// Render children of an inline element, inheriting the effective computed style.
// Handles mixed content: TEXT nodes are shaped immediately; nested inline
// ELEMENT children (e.g. <em> inside <strong>) are rendered recursively with
// merged styles.
void renderInlineChildren(const dom::DOMNodePtr& node,
                          const dom::ComputedStyle& effective_style,
                          const dom::ComputedStyle& container_style,
                          MixedPathState& state,
                          TextShaper& shaper,
                          DisplayListBuilder& builder) {
    bool prev_was_text = false;
    for (const auto& gc : node->getChildren()) {
        if (!gc) continue;

        if (gc->getType() == dom::DOMNode::NodeType::TEXT) {
            // Keep trailing space so adjacent inline elements get a word gap.
            std::string raw = gc->getRawTextContent();
            std::string text = collapseWhitespace(raw);
            if (text.empty()) {
                // Whitespace-only node: if original had content, emit a space gap.
                if (!raw.empty() && state.cumulative_x > 0.0f) {
                    state.cumulative_x += measureSpaceWidth(
                        container_style, state.container_font_size, shaper);
                }
                prev_was_text = true;
                continue;
            }
            // Restore leading space when raw text started with whitespace
            bool raw_starts_with_space = !raw.empty()
                && std::isspace(static_cast<unsigned char>(raw.front()));
            if (raw_starts_with_space && state.cumulative_x > 0.0f) {
                state.cumulative_x += measureSpaceWidth(
                    container_style, state.container_font_size, shaper);
            }
            emitInlineTextRun(text, effective_style, container_style,
                              state, shaper, builder);
            // Trailing space compensation
            bool raw_ends_with_space = std::isspace(static_cast<unsigned char>(raw.back()));
            if (raw_ends_with_space) {
                state.cumulative_x += measureSpaceWidth(
                    container_style, state.container_font_size, shaper);
            }
            prev_was_text = true;
        } else if (gc->getType() == dom::DOMNode::NodeType::ELEMENT) {
            const auto& child_cs = gc->getComputedStyle();
            if (child_cs.display == "inline") {
                renderInlineSubtree(gc, child_cs, container_style,
                                    state, shaper, builder);
                prev_was_text = false;
            }
        }
    }
}

// Render an inline element subtree: optional background, then children.
void renderInlineSubtree(const dom::DOMNodePtr& node,
                         const dom::ComputedStyle& effective_style,
                         const dom::ComputedStyle& container_style,
                         MixedPathState& state,
                         TextShaper& shaper,
                         DisplayListBuilder& builder) {
    float pad_l = effective_style.padding_left.isPixel() ? effective_style.padding_left.value : 0.0f;
    float pad_r = effective_style.padding_right.isPixel() ? effective_style.padding_right.value : 0.0f;
    state.cumulative_x += pad_l;

    float start_x = state.cumulative_x;

    // Merge style: child inherits container for missing fields.
    dom::ComputedStyle merged = effective_style;
    if (merged.font_family.empty()) merged.font_family = container_style.font_family;
    if (merged.font_weight.empty()) merged.font_weight = container_style.font_weight;
    if (merged.font_style.empty())  merged.font_style  = container_style.font_style;
    if (merged.color.empty())       merged.color       = container_style.color;

    renderInlineChildren(node, merged, container_style, state, shaper, builder);

    // Background rect spanning all rendered content of this inline element.
    float content_width = state.cumulative_x - start_x;
    if (content_width > 0.0f
        && !effective_style.background_color.empty()
        && effective_style.background_color != "transparent") {
        Rect r{state.x + state.pad_left + start_x - pad_l,
               state.baseline_y - state.ascent_px,
               content_width + pad_l + pad_r, state.line_height_px};
        if (effective_style.border_radius > 0.0f)
            builder.addRoundedRect(r, makeColorFromCss(effective_style.background_color),
                                   effective_style.border_radius);
        else
            builder.addRect(r, makeColorFromCss(effective_style.background_color));
    }

    state.cumulative_x += pad_r;
    node->setAttribute("__inline_rendered__", "1");
}

void drawInlineChild(const dom::DOMNodePtr& child,
                     MixedPathState& state,
                     const dom::ComputedStyle& container_style,
                     TextShaper& shaper,
                     DisplayListBuilder& builder) {
    const auto& cs = child->getComputedStyle();
    renderInlineSubtree(child, cs, container_style, state, shaper, builder);
}

void drawTextChildAtPosition(const dom::DOMNodePtr& child,
                             float draw_x, float draw_y, float available_width,
                             float baseline_y, float line_height_px,
                             const dom::ComputedStyle& container_style,
                             float container_font_size,
                             const Color& text_color,
                             TextShaper& shaper,
                             DisplayListBuilder& builder) {
    // Use collapseWhitespace (trims both ends) for the visible glyph run.
    // Trailing-space compensation for word separation is handled in the
    // caller (drawTextChild) by adjusting cumulative_x.
    std::string text = collapseWhitespace(child->getRawTextContent());
    if (text.empty()) return;

    TextShapeRequest req{text, container_style.font_family, container_style.font_weight,
                         container_style.font_style, container_font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    float scale = shaped.scale_to_pixels;
    float text_width = shaped.width_units * scale;
    float ascent = shaped.ascent_units > 0.0f ? shaped.ascent_units : container_font_size / scale;

    // Truncate if needed
    if (text_width > available_width && available_width > 0.0f) {
        float ratio = available_width / text_width;
        size_t len = static_cast<size_t>(text.length() * ratio);
        if (len > 0) {
            size_t wb = text.find_last_of(" \t\n\r", len);
            if (wb != std::string::npos && wb > 0) len = wb;
            req.text = text.substr(0, len);
            if (shaper.shape(req, shaped) && !shaped.glyphs.empty()) {
                text_width = shaped.width_units * scale;
            }
        }
    }

    DrawGlyphRunData run;
    run.rect = {draw_x, baseline_y - ascent * scale, text_width, line_height_px};
    run.color = text_color;
    run.font_size = container_font_size;
    run.font_family = container_style.font_family;
    run.font_weight = container_style.font_weight;
    run.font_style = container_style.font_style;
    run.font_paths = shaped.font_paths;
    run.font_path = shaped.font_path;
    run.baseline_x = run.rect.x;
    run.baseline_y = baseline_y;
    run.units_per_em = shaped.units_per_em;
    run.scale_to_pixels = shaped.scale_to_pixels;
    fillTextShadow(run, container_style);

    for (const auto& sg : shaped.glyphs) {
        GlyphInstance inst{sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                          sg.font_path_index, sg.units_per_em};
        run.glyphs.push_back(inst);
    }
    builder.addGlyphRun(std::move(run));
}

void drawTextChild(const dom::DOMNodePtr& child,
                   MixedPathState& state,
                   const dom::ComputedStyle& container_style,
                   TextShaper& shaper,
                   DisplayListBuilder& builder) {
    // Compensate leading space: if the raw text starts with whitespace
    // (e.g. " and ") and we already have content on the line, add a space gap.
    // collapseWhitespace trims both ends, so the space would be lost otherwise.
    if (rawTextStartsWithSpace(child) && state.cumulative_x > 0.0f) {
        state.cumulative_x += measureSpaceWidth(container_style, state.container_font_size, shaper);
    }

    // Line break if needed
    float available = state.inner_width - state.cumulative_x;
    if (available < 20.0f) {
        state.cumulative_x = 0.0f;
        state.baseline_y += state.line_height_px;
        available = state.inner_width;
    }

    float draw_x = state.x + state.pad_left + state.cumulative_x;
    drawTextChildAtPosition(child, draw_x, state.baseline_y, available,
                            state.baseline_y, state.line_height_px,
                            container_style, state.container_font_size,
                            state.text_color, shaper, builder);

    // Advance cumulative_x by measuring text width
    std::string text = collapseWhitespace(child->getRawTextContent());
    if (!text.empty()) {
        TextShapeRequest req{text, container_style.font_family, container_style.font_weight,
                             container_style.font_style, state.container_font_size};
        ShapedText shaped;
        if (shaper.shape(req, shaped) && !shaped.glyphs.empty()) {
            state.cumulative_x += shaped.width_units * shaped.scale_to_pixels;
        }
    }
    // Add trailing space width when original text ends with whitespace
    if (rawTextEndsWithSpace(child)) {
        state.cumulative_x += measureSpaceWidth(container_style, state.container_font_size, shaper);
    }
}

float getInlineBlockWidth(const dom::DOMNodePtr& child,
                          const dom::ComputedStyle& cs,
                          layout::Engine* engine) {
    float ml = cs.margin_left.isPixel() ? cs.margin_left.value : 0.0f;
    float mr = cs.margin_right.isPixel() ? cs.margin_right.value : 0.0f;
    const auto* cl = engine ? engine->getLayout(child) : nullptr;
    float w = cl ? cl->layout.dimensions[0] : (cs.width.isPixel() ? cs.width.value : 13.0f);
    return ml + w + mr;
}

void renderMixedPath(const dom::DOMNodePtr& node,
                     const layout::LayoutNode* layout,
                     const dom::ComputedStyle& style,
                     float bl, float bt, float br, float bb,
                     TextShaper& shaper,
                     layout::Engine* engine,
                     DisplayListBuilder& builder) {
    MixedPathState state = initMixedPath(layout, style, bl, bt, br, bb, shaper);

    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            const auto& cs = child->getComputedStyle();
            const std::string tag = child->getTagName();
            if (tag == "br") {
                state.cumulative_x = 0.0f;
                state.baseline_y += state.line_height_px;
                continue;
            }
            if (cs.display == "inline") {
                drawInlineChild(child, state, style, shaper, builder);
            } else if (cs.display == "inline-block") {
                state.cumulative_x += getInlineBlockWidth(child, cs, engine);
            }
        } else if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            // Always use cumulative_x-based positioning in Mixed path so that
            // text nodes and inline elements share the same coordinate system.
            drawTextChild(child, state, style, shaper, builder);
        }
    }
}

// ========== Placeholder 样式解析 ==========
// 从 ::placeholder 伪元素获取渲染样式，回退到默认半透明颜色。
struct PlaceholderStyle {
    Color color;
    float font_size;
    std::string font_family;
    std::string font_weight;
    std::string font_style;
};

PlaceholderStyle resolvePlaceholderStyle(const dom::DOMNodePtr& node,
                                        const dom::ComputedStyle& input_style) {
    PlaceholderStyle ps;
    ps.color = makeColorFromCss(input_style.color);
    ps.font_size = input_style.font_size > 0.0f ? input_style.font_size : 16.0f;
    ps.font_family = input_style.font_family;
    ps.font_weight = input_style.font_weight;
    ps.font_style = input_style.font_style;

    auto pseudo = node->getPseudoPlaceholder();
    if (pseudo) {
        const auto& ps_style = pseudo->getComputedStyle();
        if (!ps_style.color.empty()) ps.color = makeColorFromCss(ps_style.color);
        if (ps_style.font_size > 0.0f) ps.font_size = ps_style.font_size;
        if (!ps_style.font_family.empty()) ps.font_family = ps_style.font_family;
        if (!ps_style.font_weight.empty()) ps.font_weight = ps_style.font_weight;
        if (!ps_style.font_style.empty()) ps.font_style = ps_style.font_style;
    } else {
        // Default: 50% opacity (browser-like default for placeholder)
        ps.color.a *= 0.5f;
    }
    return ps;
}

// Shared glyph-run emission for input/textarea placeholder and value text.
void emitInputGlyphRun(const std::string& text,
                       float x, float y, float w, float h, float pad_l,
                       const Color& color, float font_size,
                       const std::string& font_family,
                       const std::string& font_weight,
                       const std::string& font_style,
                       const dom::ComputedStyle& style,
                       TextShaper& shaper,
                       DisplayListBuilder& builder) {
    TextShapeRequest req{text, font_family, font_weight, font_style, font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    float scale = shaped.scale_to_pixels;
    float ascent = shaped.ascent_units > 0.0f ? shaped.ascent_units : font_size / scale;
    float line_h = shaped.line_height_units;
    if (line_h <= 0.0f) line_h = font_size / scale;
    float descent_abs = shaped.descent_units < 0.0f ? -shaped.descent_units : 0.0f;
    float extra = std::max(line_h - (ascent + descent_abs), 0.0f);
    float baseline_off = (extra * 0.5f + ascent) * scale;
    float text_h_px = line_h * scale;

    float text_y = y + (h - text_h_px) * 0.5f;
    float baseline_y = text_y + baseline_off;
    float text_x = x + pad_l;

    DrawGlyphRunData run;
    run.rect = {text_x, text_y, shaped.width_units * scale, text_h_px};
    run.color = color;
    run.font_size = font_size;
    run.font_family = font_family;
    run.font_weight = font_weight;
    run.font_style = font_style;
    run.font_paths = shaped.font_paths;
    run.font_path = shaped.font_path;
    run.baseline_x = text_x;
    run.baseline_y = baseline_y;
    run.units_per_em = shaped.units_per_em;
    run.scale_to_pixels = shaped.scale_to_pixels;
    fillTextShadow(run, style);

    for (const auto& sg : shaped.glyphs) {
        GlyphInstance inst{sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                          sg.font_path_index, sg.units_per_em};
        run.glyphs.push_back(inst);
    }
    builder.addGlyphRun(std::move(run));
}

// ========== Input 元素渲染 ==========
void renderInput(const dom::DOMNodePtr& node,
                 const layout::LayoutNode* layout,
                 const dom::ComputedStyle& style,
                 float bl, float bt, float br, float bb,
                 TextShaper& shaper,
                 DisplayListBuilder& builder) {
    float x = layout->layout.position[0] + bl;
    float y = layout->layout.position[1] + bt;
    float w = std::max(0.0f, layout->layout.dimensions[0] - bl - br);
    float h = std::max(0.0f, layout->layout.dimensions[1] - bt - bb);
    float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;

    std::string text = node->getAttribute("value");
    Color color = makeColorFromCss(style.color);
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
    std::string font_family = style.font_family;
    std::string font_weight = style.font_weight;
    std::string font_style = style.font_style;

    if (text.empty()) {
        text = node->getAttribute("placeholder");
        auto ps = resolvePlaceholderStyle(node, style);
        color = ps.color;
        font_size = ps.font_size;
        font_family = ps.font_family;
        font_weight = ps.font_weight;
        font_style = ps.font_style;
    }
    if (text.empty() || w <= 0.0f || h <= 0.0f) return;

    emitInputGlyphRun(text, x, y, w, h, pad_l,
                      color, font_size, font_family, font_weight, font_style,
                      style, shaper, builder);
}

// ========== Textarea placeholder 渲染 ==========
void renderTextareaPlaceholder(const dom::DOMNodePtr& node,
                               const layout::LayoutNode* layout,
                               const dom::ComputedStyle& style,
                               float bl, float bt, float br, float bb,
                               TextShaper& shaper,
                               DisplayListBuilder& builder) {
    // Only render placeholder when textarea has no text children with content
    bool has_content = false;
    for (const auto& child : node->getChildren()) {
        if (child && child->getType() == dom::DOMNode::NodeType::TEXT) {
            if (!child->getRawTextContent().empty()) { has_content = true; break; }
        }
    }
    if (has_content) return;

    std::string placeholder_text = node->getAttribute("placeholder");
    if (placeholder_text.empty()) return;

    float x = layout->layout.position[0] + bl;
    float y = layout->layout.position[1] + bt;
    float w = std::max(0.0f, layout->layout.dimensions[0] - bl - br);
    float h = std::max(0.0f, layout->layout.dimensions[1] - bt - bb);
    if (w <= 0.0f || h <= 0.0f) return;

    float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    auto ps = resolvePlaceholderStyle(node, style);

    emitInputGlyphRun(placeholder_text, x, y, w, h, pad_l,
                      ps.color, ps.font_size, ps.font_family, ps.font_weight, ps.font_style,
                      style, shaper, builder);
}

// ========== 完整文本路径声明（完整实现在底部） ==========
void renderFullText(const dom::DOMNodePtr& node,
                    const layout::LayoutNode* layout,
                    const dom::ComputedStyle& style,
                    const std::string& raw_text,
                    float bl, float bt, float br, float bb,
                    TextShaper& shaper,
                    DisplayListBuilder& builder);

} // anonymous namespace

// ========== 主入口 ==========
void Painter::paintTextAndInput(const dom::DOMNodePtr& node,
                               const layout::LayoutNode* layout_node,
                               const std::string& tag,
                               const dom::ComputedStyle& style,
                               bool is_hidden,
                               DisplayListBuilder& builder) {
    if (!node || !layout_node || is_hidden) return;

    auto effectiveBorderStyle = [&](const std::string& side_style) -> std::string {
        return toLowerCopy(collapseWhitespace(!side_style.empty() ? side_style : style.border_style));
    };
    auto effectiveBorderWidth = [&](float side_width, const std::string& side_style) -> float {
        float w = (side_width >= 0.0f) ? side_width : style.border_width;
        if (w < 0.0f) w = 0.0f;
        auto st = effectiveBorderStyle(side_style);
        return (st == "none" || st == "hidden") ? 0.0f : w;
    };

    float bt = effectiveBorderWidth(style.border_top_width, style.border_top_style);
    float br = effectiveBorderWidth(style.border_right_width, style.border_right_style);
    float bb = effectiveBorderWidth(style.border_bottom_width, style.border_bottom_style);
    float bl = effectiveBorderWidth(style.border_left_width, style.border_left_style);

    // 处理文本内容（如果不是特殊标签）
    // select 的展示文本由专用渲染器负责（painter_select），这里避免把 <option> 子树当作普通文本绘制。
    if (tag != "script" && tag != "style" && tag != "head" && tag != "img" && tag != "video" && tag != "select") {
        ContentAnalysis analysis = analyzeChildren(node);

        auto path = determinePath(analysis, tag, style);

        if (path == TextRenderPath::Mixed) {
            renderMixedPath(node, layout_node, style, bl, bt, br, bb, text_shaper_, layout_engine_, builder);
        } else if (path == TextRenderPath::FullText) {
            std::string raw_text;
            if (analysis.raw_text_len > 0) {
                raw_text.reserve(analysis.raw_text_len);
                for (const auto& child : node->getChildren()) {
                    if (child && child->getType() == dom::DOMNode::NodeType::TEXT) {
                        raw_text.append(child->getRawTextContent());
                    }
                }
            }
            renderFullText(node, layout_node, style, raw_text, bl, bt, br, bb, text_shaper_, builder);
        }
    }

    // Input 元素特殊渲染
    if (tag == "input") {
        renderInput(node, layout_node, style, bl, bt, br, bb, text_shaper_, builder);
    }

    // Textarea placeholder 渲染（textarea value 由通用文本路径处理）
    if (tag == "textarea") {
        renderTextareaPlaceholder(node, layout_node, style, bl, bt, br, bb, text_shaper_, builder);
    }

    // Select 元素特殊渲染
    // appearance:none 只影响“默认外观”（例如箭头），不应影响交互与下拉列表。
    if (tag == "select") {
        renderSelect(node, layout_node, style, bl, bt, br, bb, text_shaper_, builder);
        if (auto* st = dong::dom::getSelectState(node)) {
            if (st->isOpen()) {
                open_select_overlays_.push_back(OpenSelectOverlay{
                    node,
                    layout_node,
                    builder.getTranslateX(),
                    builder.getTranslateY(),
                    bl,
                    bt,
                    br,
                    bb,
                });
            }
        }
    }


}

// ========== 完整文本路径实现 ==========
namespace {

// Split a string by '\n'. Each segment is a logical line (may be empty for consecutive '\n').
std::vector<std::string> splitByNewline(const std::string& s) {
    std::vector<std::string> lines;
    size_t start = 0;
    while (start <= s.size()) {
        size_t pos = s.find('\n', start);
        if (pos == std::string::npos) {
            lines.push_back(s.substr(start));
            break;
        }
        lines.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return lines;
}

// Common state for multi-line text rendering within renderFullText.
struct FullTextRenderCtx {
    float x, y, pad_l, pad_t, inner_width;
    float baseline_offset, effective_line_height, ascent_px;
    float font_size, scale;
    Color text_color;
    std::string text_align;
    const dom::ComputedStyle& style;
    TextShaper& shaper;
    DisplayListBuilder& builder;
};

// Emit one shaped line at the given line_index.
void emitFullTextLine(const std::string& line_text, int line_index,
                      const FullTextRenderCtx& ctx) {
    if (line_text.empty()) return;

    TextShapeRequest req{line_text, ctx.style.font_family, ctx.style.font_weight,
                         ctx.style.font_style, ctx.font_size};
    ShapedText shaped;
    if (!ctx.shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    float line_width = shaped.width_units * ctx.scale;
    float line_x = ctx.x + ctx.pad_l;
    if (ctx.text_align == "center") {
        line_x += std::max(0.0f, (ctx.inner_width - line_width) * 0.5f);
    } else if (ctx.text_align == "right") {
        line_x += std::max(0.0f, ctx.inner_width - line_width);
    }

    float baseline_y = ctx.y + ctx.pad_t + ctx.baseline_offset
                       + static_cast<float>(line_index) * ctx.effective_line_height;

    DrawGlyphRunData run;
    run.rect = {line_x, baseline_y - ctx.ascent_px, line_width, ctx.effective_line_height};
    run.color = ctx.text_color;
    run.font_size = ctx.font_size;
    run.font_family = ctx.style.font_family;
    run.font_weight = ctx.style.font_weight;
    run.font_style = ctx.style.font_style;
    run.font_paths = shaped.font_paths;
    run.font_path = shaped.font_path;
    run.baseline_x = line_x;
    run.baseline_y = baseline_y;
    run.units_per_em = shaped.units_per_em;
    run.scale_to_pixels = shaped.scale_to_pixels;
    fillTextShadow(run, ctx.style);

    for (const auto& sg : shaped.glyphs) {
        run.glyphs.push_back({sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                              sg.font_path_index, sg.units_per_em});
    }
    ctx.builder.addGlyphRun(std::move(run));
}

// Wrap a single segment (no '\n') into multiple visual lines, emitting each.
// Returns the number of visual lines emitted.
int wrapAndEmitSegment(const std::string& segment, int start_line_index,
                       const FullTextRenderCtx& ctx) {
    if (segment.empty()) return 0;

    TextShapeRequest req{segment, ctx.style.font_family, ctx.style.font_weight,
                         ctx.style.font_style, ctx.font_size};
    ShapedText shaped;
    if (!ctx.shaper.shape(req, shaped) || shaped.glyphs.empty()) return 0;

    float seg_width = shaped.width_units * ctx.scale;

    // Fits on one line?
    if (seg_width <= ctx.inner_width || ctx.inner_width <= 0.0f) {
        emitFullTextLine(segment, start_line_index, ctx);
        return 1;
    }

    // Need to wrap
    float avg_char_width = seg_width / static_cast<float>(segment.length());
    size_t chars_per_line = static_cast<size_t>(ctx.inner_width / avg_char_width);
    if (chars_per_line < 1) chars_per_line = 1;

    size_t line_start = 0;
    int lines_emitted = 0;

    while (line_start < segment.length()) {
        size_t line_end = std::min(line_start + chars_per_line, segment.length());
        if (line_end < segment.length()) {
            size_t word_break = segment.find_last_of(" \t", line_end);
            if (word_break != std::string::npos && word_break > line_start) {
                line_end = word_break;
            }
        }
        std::string line_text = segment.substr(line_start, line_end - line_start);
        emitFullTextLine(line_text, start_line_index + lines_emitted, ctx);

        line_start = line_end;
        while (line_start < segment.length() &&
               (segment[line_start] == ' ' || segment[line_start] == '\t')) {
            ++line_start;
        }
        ++lines_emitted;
        if (lines_emitted > 500) break;
    }
    return lines_emitted;
}

// Determine whether newlines in text should produce forced line breaks.
bool preservesNewlines(const std::string& white_space) {
    return white_space == "pre-wrap" || white_space == "pre"
        || white_space == "pre-line";
}

void renderFullText(const dom::DOMNodePtr& node,
                    const layout::LayoutNode* layout,
                    const dom::ComputedStyle& style,
                    const std::string& raw_text,
                    float bl, float bt, float br, float bb,
                    TextShaper& shaper,
                    DisplayListBuilder& builder) {
    // Apply white-space and text-transform
    std::string text = raw_text;
    std::string white_space = toLowerCopy(collapseWhitespace(style.white_space));
    if (white_space == "normal" || white_space == "nowrap") {
        text = collapseWhitespace(text);
    } else if (white_space == "pre-line") {
        text = painter_detail::collapseSpacesPreserveNewlines(text);
    } else if (white_space == "pre-wrap") {
        text = painter_detail::collapseSpacesPreserveNewlines(text);
    }
    // "pre" keeps text as-is (no collapse)

    if (!text.empty() && !style.text_transform.empty() && style.text_transform != "none") {
        if (style.text_transform == "uppercase") {
            std::transform(text.begin(), text.end(), text.begin(),
                          [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        } else if (style.text_transform == "lowercase") {
            std::transform(text.begin(), text.end(), text.begin(),
                          [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        }
    }

    if (text.empty()) return;

    float origin_x = 0.0f, origin_y = 0.0f;
    getPaintOrigin(layout, origin_x, origin_y);
    float x = origin_x + bl;
    float y = origin_y + bt;
    float width = std::max(0.0f, layout->layout.dimensions[0] - bl - br);

    float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    float pad_r = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
    float pad_t = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;

    float inner_width = std::max(0.0f, width - pad_l - pad_r);

    Color text_color = makeColorFromCss(style.color);
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

    // Get metrics
    TextShapeRequest metrics_req{"X", style.font_family, style.font_weight, style.font_style, font_size};
    ShapedText metrics;
    if (!shaper.shape(metrics_req, metrics)) return;

    float scale = metrics.scale_to_pixels;
    float ascent_units = metrics.ascent_units;
    float descent_units = metrics.descent_units;
    float line_height_units = metrics.line_height_units;

    if (style.line_height > 0.0f) {
        if (style.line_height_is_unitless) {
            line_height_units = (style.line_height * font_size) / std::max(scale, 1e-3f);
        } else {
            line_height_units = style.line_height / std::max(scale, 1e-3f);
        }
    }
    if (line_height_units <= 0.0f) line_height_units = font_size / std::max(scale, 1e-3f);
    if (ascent_units <= 0.0f) ascent_units = font_size / std::max(scale, 1e-3f);

    float descent_abs = descent_units < 0.0f ? -descent_units : 0.0f;
    float metrics_height = ascent_units + descent_abs;
    float extra = line_height_units - metrics_height;
    if (extra < 0.0f) extra = 0.0f;

    float effective_line_height = line_height_units * scale;
    float ascent_px = ascent_units * scale;
    float baseline_offset = (extra * 0.5f + ascent_units) * scale;

    bool nowrap = (white_space == "nowrap");
    bool has_forced_newlines = preservesNewlines(white_space) && text.find('\n') != std::string::npos;

    FullTextRenderCtx ctx{x, y, pad_l, pad_t, inner_width,
                          baseline_offset, effective_line_height, ascent_px,
                          font_size, scale, text_color,
                          style.text_align, style, shaper, builder};

    // Path 1: text has forced newlines from pre-wrap / pre / pre-line
    if (has_forced_newlines) {
        auto segments = splitByNewline(text);
        int line_index = 0;
        bool allow_wrap = (white_space != "pre" && white_space != "nowrap");
        for (const auto& seg : segments) {
            if (seg.empty()) {
                ++line_index; // empty line (consecutive \n)
                continue;
            }
            if (allow_wrap) {
                int emitted = wrapAndEmitSegment(seg, line_index, ctx);
                line_index += std::max(emitted, 1);
            } else {
                emitFullTextLine(seg, line_index, ctx);
                ++line_index;
            }
            if (line_index > 500) break;
        }
        return;
    }

    // Path 2: normal / nowrap / no-newline text — original logic
    TextShapeRequest req{text, style.font_family, style.font_weight, style.font_style, font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    float text_width = shaped.width_units * scale;

    if (!nowrap && text_width > inner_width && inner_width > 0.0f) {
        int emitted = wrapAndEmitSegment(text, 0, ctx);
        (void)emitted;
    } else {
        emitFullTextLine(text, 0, ctx);
    }
}

} // anonymous namespace

} // namespace dong::render

// Include select rendering implementation
#include "painter_select.cpp"
