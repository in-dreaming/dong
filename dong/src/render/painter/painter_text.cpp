#include "../painter.hpp"
#include "painter_style_utils.hpp"
#include <cstdlib>
#include <unordered_set>
#include "../../layout/layout_engine.hpp"

namespace dong::render {
namespace {

using painter_detail::collapseWhitespace;
using painter_detail::fillTextShadow;
using painter_detail::makeColorFromCss;
using painter_detail::toLowerCopy;

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
        "p", "span", "button", "code", "div", "footer", "label"
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
    s.x = layout->layout.position[0] + bl;
    s.y = layout->layout.position[1] + bt;
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

void drawInlineChild(const dom::DOMNodePtr& child,
                     MixedPathState& state,
                     const dom::ComputedStyle& container_style,
                     TextShaper& shaper,
                     DisplayListBuilder& builder) {
    const auto& cs = child->getComputedStyle();
    std::string text;
    for (const auto& gc : child->getChildren()) {
        if (gc && gc->getType() == dom::DOMNode::NodeType::TEXT) {
            text += gc->getRawTextContent();
        }
    }
    text = collapseWhitespace(text);
    if (text.empty()) return;

    float font_size = cs.font_size > 0.0f ? cs.font_size : state.container_font_size;
    TextShapeRequest req{text, cs.font_family.empty() ? container_style.font_family : cs.font_family,
                         cs.font_weight.empty() ? container_style.font_weight : cs.font_weight,
                         cs.font_style.empty() ? container_style.font_style : cs.font_style,
                         font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    float scale = shaped.scale_to_pixels;
    float text_width = shaped.width_units * scale;
    float ascent = shaped.ascent_units > 0.0f ? shaped.ascent_units : font_size / scale;
    float pad_l = cs.padding_left.isPixel() ? cs.padding_left.value : 0.0f;
    float pad_r = cs.padding_right.isPixel() ? cs.padding_right.value : 0.0f;

    // Background
    if (!cs.background_color.empty() && cs.background_color != "transparent") {
        Rect r{state.x + state.pad_left + state.cumulative_x,
               state.baseline_y - state.ascent_px,
               text_width + pad_l + pad_r, state.line_height_px};
        if (cs.border_radius > 0.0f)
            builder.addRoundedRect(r, makeColorFromCss(cs.background_color), cs.border_radius);
        else
            builder.addRect(r, makeColorFromCss(cs.background_color));
    }

    // Glyph run
    DrawGlyphRunData run;
    run.rect = {state.x + state.pad_left + state.cumulative_x + pad_l,
                state.baseline_y - ascent * scale, text_width, state.line_height_px};
    run.color = makeColorFromCss(!cs.color.empty() ? cs.color : container_style.color);
    run.font_size = font_size;
    run.font_family = cs.font_family.empty() ? container_style.font_family : cs.font_family;
    run.font_weight = cs.font_weight.empty() ? container_style.font_weight : cs.font_weight;
    run.font_style = cs.font_style.empty() ? container_style.font_style : cs.font_style;
    run.font_paths = shaped.font_paths;
    run.font_path = shaped.font_path;
    run.baseline_x = run.rect.x;
    run.baseline_y = state.baseline_y;
    run.units_per_em = shaped.units_per_em;
    run.scale_to_pixels = shaped.scale_to_pixels;
    fillTextShadow(run, cs);

    for (const auto& sg : shaped.glyphs) {
        GlyphInstance inst{sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                          sg.font_path_index, sg.units_per_em};
        run.glyphs.push_back(inst);
    }
    builder.addGlyphRun(std::move(run));
    state.cumulative_x += text_width + pad_l + pad_r;
    child->setAttribute("__inline_rendered__", "1");
}

void drawTextChild(const dom::DOMNodePtr& child,
                   MixedPathState& state,
                   const dom::ComputedStyle& container_style,
                   TextShaper& shaper,
                   DisplayListBuilder& builder) {
    std::string text = collapseWhitespace(child->getRawTextContent());
    if (text.empty()) return;

    // Line break if needed
    float available = state.inner_width - state.cumulative_x;
    if (available < 20.0f) {
        state.cumulative_x = 0.0f;
        state.baseline_y += state.line_height_px;
        available = state.inner_width;
    }

    TextShapeRequest req{text, container_style.font_family, container_style.font_weight,
                         container_style.font_style, state.container_font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    float scale = shaped.scale_to_pixels;
    float text_width = shaped.width_units * scale;
    float ascent = shaped.ascent_units > 0.0f ? shaped.ascent_units : state.container_font_size / scale;

    // Truncate if needed
    if (text_width > available && available > 0.0f) {
        float ratio = available / text_width;
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
    run.rect = {state.x + state.pad_left + state.cumulative_x,
                state.baseline_y - ascent * scale, text_width, state.line_height_px};
    run.color = state.text_color;
    run.font_size = state.container_font_size;
    run.font_family = container_style.font_family;
    run.font_weight = container_style.font_weight;
    run.font_style = container_style.font_style;
    run.font_paths = shaped.font_paths;
    run.font_path = shaped.font_path;
    run.baseline_x = run.rect.x;
    run.baseline_y = state.baseline_y;
    run.units_per_em = shaped.units_per_em;
    run.scale_to_pixels = shaped.scale_to_pixels;
    fillTextShadow(run, container_style);

    for (const auto& sg : shaped.glyphs) {
        GlyphInstance inst{sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                          sg.font_path_index, sg.units_per_em};
        run.glyphs.push_back(inst);
    }
    builder.addGlyphRun(std::move(run));
    state.cumulative_x += text_width;
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
            drawTextChild(child, state, style, shaper, builder);
        }
    }

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
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

    std::string text = node->getAttribute("value");
    Color color = makeColorFromCss(style.color);
    std::string placeholder = node->getAttribute("placeholder");
    if (text.empty()) {
        text = node->getAttribute("placeholder");
        color.a *= 0.5f;
    }
    if (text.empty() || w <= 0.0f || h <= 0.0f) return;

    TextShapeRequest req{text, style.font_family, style.font_weight, style.font_style, font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) {
        return;
    }
    
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
    run.font_family = style.font_family;
    run.font_weight = style.font_weight;
    run.font_style = style.font_style;
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
    if (tag != "script" && tag != "style" && tag != "head" && tag != "img" && tag != "video") {
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
}

// ========== 完整文本路径实现 ==========
namespace {

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
    }

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

    float x = layout->layout.position[0] + bl;
    float y = layout->layout.position[1] + bt;
    float width = std::max(0.0f, layout->layout.dimensions[0] - bl - br);
    float height = std::max(0.0f, layout->layout.dimensions[1] - bt - bb);

    float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    float pad_r = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
    float pad_t = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float pad_b = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

    float inner_width = std::max(0.0f, width - pad_l - pad_r);
    float inner_height = std::max(0.0f, height - pad_t - pad_b);
    (void)inner_height;

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

    // Handle white-space: nowrap - single line, no wrapping
    bool nowrap = (white_space == "nowrap");

    // Shape full text
    TextShapeRequest req{text, style.font_family, style.font_weight, style.font_style, font_size};
    ShapedText shaped;
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) return;

    // Simple single-line rendering for most cases
    float text_width = shaped.width_units * scale;

    // Calculate alignment
    float text_x = x + pad_l;
    std::string text_align = style.text_align;
    if (text_align == "center") {
        text_x = x + pad_l + std::max(0.0f, (inner_width - text_width) * 0.5f);
    } else if (text_align == "right") {
        text_x = x + pad_l + std::max(0.0f, inner_width - text_width);
    }

    // For overflow, we should clip or wrap
    if (std::getenv("DONG_DEBUG_BUTTON_WRAP")) {
        if (node && node->getTagName() == "button") {
            DONG_LOG_INFO("[PainterText] button text_width=%.2f inner_width=%.2f nowrap=%d text='%s'",
                          text_width, inner_width, nowrap ? 1 : 0, text.c_str());
        }
    }
    if (!nowrap && text_width > inner_width && inner_width > 0.0f) {

        // Simple wrap: estimate characters per line
        float avg_char_width = text_width / static_cast<float>(text.length());
        size_t chars_per_line = static_cast<size_t>(inner_width / avg_char_width);
        if (chars_per_line < 1) chars_per_line = 1;

        size_t line_start = 0;
        int line_index = 0;

        while (line_start < text.length()) {
            // Find break point
            size_t line_end = std::min(line_start + chars_per_line, text.length());
            if (line_end < text.length()) {
                // Try to break at word boundary
                size_t word_break = text.find_last_of(" \t\n\r", line_end);
                if (word_break != std::string::npos && word_break > line_start) {
                    line_end = word_break;
                }
            }

            std::string line_text = text.substr(line_start, line_end - line_start);
            if (!line_text.empty()) {
                TextShapeRequest line_req{line_text, style.font_family, style.font_weight,
                                         style.font_style, font_size};
                ShapedText line_shaped;
                if (shaper.shape(line_req, line_shaped) && !line_shaped.glyphs.empty()) {
                    float line_width = line_shaped.width_units * scale;

                    float line_x = x + pad_l;
                    if (text_align == "center") {
                        line_x = x + pad_l + std::max(0.0f, (inner_width - line_width) * 0.5f);
                    } else if (text_align == "right") {
                        line_x = x + pad_l + std::max(0.0f, inner_width - line_width);
                    }

                    float baseline_y = y + pad_t + baseline_offset + static_cast<float>(line_index) * effective_line_height;

                    DrawGlyphRunData run;
                    run.rect = {line_x, baseline_y - ascent_px, line_width, effective_line_height};
                    run.color = text_color;
                    run.font_size = font_size;
                    run.font_family = style.font_family;
                    run.font_weight = style.font_weight;
                    run.font_style = style.font_style;
                    run.font_paths = line_shaped.font_paths;
                    run.font_path = line_shaped.font_path;
                    run.baseline_x = line_x;
                    run.baseline_y = baseline_y;
                    run.units_per_em = line_shaped.units_per_em;
                    run.scale_to_pixels = line_shaped.scale_to_pixels;
                    fillTextShadow(run, style);

                    for (const auto& sg : line_shaped.glyphs) {
                        GlyphInstance inst{sg.glyph_id, sg.pen_x_units, sg.pen_y_units,
                                          sg.font_path_index, sg.units_per_em};
                        run.glyphs.push_back(inst);
                    }
                    builder.addGlyphRun(std::move(run));
                }
            }

            line_start = line_end;
            // Skip whitespace at start of next line
            while (line_start < text.length() && std::isspace(static_cast<unsigned char>(text[line_start]))) {
                ++line_start;
            }
            ++line_index;

            // Safety limit
            if (line_index > 100) break;
        }
    } else {
        // Single line rendering
        float baseline_y = y + pad_t + baseline_offset;

        DrawGlyphRunData run;
        run.rect = {text_x, baseline_y - ascent_px,
                   nowrap ? text_width : std::min(text_width, inner_width),
                   effective_line_height};
        run.color = text_color;
        run.font_size = font_size;
        run.font_family = style.font_family;
        run.font_weight = style.font_weight;
        run.font_style = style.font_style;
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
}

} // anonymous namespace

} // namespace dong::render
