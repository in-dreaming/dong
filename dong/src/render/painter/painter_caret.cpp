// Caret and selection rendering for contenteditable elements

#include "../painter.hpp"
#include "painter_style_utils.hpp"
#include "../../dom/contenteditable.hpp"
#include "../../dom/selection.hpp"
#include "../../dom/input_element.hpp"
#include "../../layout/layout_engine.hpp"
#include "../../core/string_utils.h"
#include <vector>
#include <algorithm>
#include <functional>

namespace dong::render {

// Collect all text nodes in document order within a subtree.
static void collectTextNodes(const dom::DOMNodePtr& node, std::vector<dom::DOMNodePtr>& out) {
    if (!node) return;
    if (node->getType() == dom::DOMNode::NodeType::TEXT) {
        out.push_back(node);
        return;
    }
    for (const auto& child : node->getChildren()) {
        collectTextNodes(child, out);
    }
}

// Helper: compute text x-offset for a given raw byte offset in a text node.
// If use_collapsed is true, maps raw offset to collapsed text before shaping
// (needed when text node has IFC layout, which uses collapsed whitespace).
static float computeTextXOffset(const dom::DOMNodePtr& text_node,
                                 uint32_t char_offset,
                                 const dom::ComputedStyle& style,
                                 float font_size,
                                 TextShaper& shaper,
                                 bool use_collapsed = false) {
    if (!text_node || char_offset == 0) return 0.0f;

    std::string raw_text = text_node->getRawTextContent();
    if (char_offset > raw_text.size()) char_offset = static_cast<uint32_t>(raw_text.size());

    std::string prefix;
    if (use_collapsed) {
        // Map the raw byte offset to collapsed text.
        // Walk through raw text tracking collapsed position.
        std::string collapsed = dong::collapseWhitespace(raw_text);
        // Find how many collapsed bytes correspond to raw_text[0..char_offset]
        uint32_t col_pos = 0;
        bool in_space = false;
        bool started = false;
        for (uint32_t i = 0; i < char_offset && i < static_cast<uint32_t>(raw_text.size()); i++) {
            if (dong::isAsciiWhitespace(raw_text[i])) {
                if (started && !in_space) {
                    col_pos++;
                }
                in_space = true;
            } else {
                started = true;
                in_space = false;
                col_pos++;
            }
        }
        if (col_pos > collapsed.size()) col_pos = static_cast<uint32_t>(collapsed.size());
        prefix = collapsed.substr(0, col_pos);
    } else {
        prefix = raw_text.substr(0, char_offset);
    }

    TextShapeRequest req;
    req.text = prefix;
    req.font_family = style.font_family;
    req.font_size = font_size;
    req.font_weight = toString(style.font_weight);
    req.font_style = toString(style.font_style);

    ShapedText shaped;
    if (shaper.shape(req, shaped) && shaped.units_per_em > 0) {
        float scale = font_size / static_cast<float>(shaped.units_per_em);
        return shaped.width_units * scale;
    }
    return 0.0f;
}

// Helper: measure the pixel width of a text string
static float measureTextWidth(const std::string& text,
                               const dom::ComputedStyle& style,
                               float font_size,
                               TextShaper& shaper) {
    if (text.empty()) return 0.0f;
    TextShapeRequest req;
    req.text = text;
    req.font_family = style.font_family;
    req.font_size = font_size;
    req.font_weight = toString(style.font_weight);
    req.font_style = toString(style.font_style);
    ShapedText shaped;
    if (shaper.shape(req, shaped) && shaped.units_per_em > 0) {
        float scale = font_size / static_cast<float>(shaped.units_per_em);
        return shaped.width_units * scale;
    }
    return text.size() * font_size * 0.6f;
}

static float measureSpaceWidth(const dom::ComputedStyle& style,
                                float font_size,
                                TextShaper& shaper) {
    return measureTextWidth(" ", style, font_size, shaper);
}

// Simulate the rendering pipeline's cumulative_x computation to find the
// exact caret X position.  This replaces computeTextNodeBaseX + computeTextXOffset
// which had whitespace-handling mismatches with the actual text renderer.
//
// The renderer: collapseWhitespace(raw) -> trimmed text, then adds separate
// space gaps for leading/trailing whitespace.  We replicate that logic here
// so the caret lines up pixel-perfectly with the rendered glyphs.
static float computeCaretXFromRendering(const dom::DOMNodePtr& text_node,
                                         uint32_t char_offset,
                                         layout::Engine* layout_engine,
                                         TextShaper& shaper) {
    if (!text_node) return 0.0f;

    auto ce_root = text_node->getParent();
    while (ce_root) {
        if (ce_root->hasAttribute("contenteditable")) break;
        auto next = ce_root->getParent();
        if (!next) break;
        ce_root = next;
    }
    if (!ce_root) return 0.0f;

    auto* ce_layout = layout_engine->getLayout(ce_root);
    if (!ce_layout) return 0.0f;

    const auto& ce_style = ce_root->getComputedStyle();
    float pad_left = ce_style.padding_left.isPixel() ? ce_style.padding_left.value : 0.0f;
    float border_left = ce_style.border_left_width >= 0.0f ? ce_style.border_left_width : 0.0f;
    float ce_fs = ce_style.font_size > 0.0f ? ce_style.font_size : 16.0f;

    float origin_x = ce_layout->x + border_left + pad_left;
    float cum_x = 0.0f;
    bool found = false;
    float result_x = 0.0f;

    // Advance cum_x for a text node the same way the renderer does.
    auto advanceText = [&](const std::string& raw,
                           const dom::ComputedStyle& st,
                           float fs) {
        std::string collapsed = dong::collapseWhitespace(raw);
        if (collapsed.empty()) {
            if (!raw.empty() && cum_x > 0.0f)
                cum_x += measureSpaceWidth(st, fs, shaper);
            return;
        }
        if (!raw.empty() && dong::isAsciiWhitespace(raw.front()) && cum_x > 0.0f)
            cum_x += measureSpaceWidth(st, fs, shaper);
        cum_x += measureTextWidth(collapsed, st, fs, shaper);
        if (!raw.empty() && dong::isAsciiWhitespace(raw.back()))
            cum_x += measureSpaceWidth(st, fs, shaper);
    };

    // Compute caret X within the target text node.
    auto computeOffsetInNode = [&](const std::string& raw,
                                    uint32_t off,
                                    const dom::ComputedStyle& st,
                                    float fs) -> float {
        std::string collapsed = dong::collapseWhitespace(raw);
        bool has_leading = !raw.empty() && dong::isAsciiWhitespace(raw.front()) && cum_x > 0.0f;
        float leading = has_leading ? measureSpaceWidth(st, fs, shaper) : 0.0f;

        if (off == 0) return 0.0f;

        if (off >= static_cast<uint32_t>(raw.size())) {
            return leading + measureTextWidth(collapsed, st, fs, shaper);
        }

        // Map raw byte offset -> collapsed byte offset
        uint32_t col_pos = 0;
        bool in_sp = false, started = false;
        for (uint32_t i = 0; i < off && i < static_cast<uint32_t>(raw.size()); ++i) {
            if (dong::isAsciiWhitespace(raw[i])) {
                if (started && !in_sp) col_pos++;
                in_sp = true;
            } else {
                started = true;
                in_sp = false;
                col_pos++;
            }
        }
        if (col_pos > static_cast<uint32_t>(collapsed.size()))
            col_pos = static_cast<uint32_t>(collapsed.size());

        std::string prefix = collapsed.substr(0, col_pos);
        return leading + measureTextWidth(prefix, st, fs, shaper);
    };

    std::function<bool(const dom::DOMNodePtr&,
                        const dom::ComputedStyle&, float)> walk;
    walk = [&](const dom::DOMNodePtr& parent,
               const dom::ComputedStyle& st, float fs) -> bool {
        for (const auto& child : parent->getChildren()) {
            if (found) return true;

            if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                if (child == text_node) {
                    found = true;
                    result_x = origin_x + cum_x +
                        computeOffsetInNode(child->getRawTextContent(),
                                            char_offset, st, fs);
                    return true;
                }
                advanceText(child->getRawTextContent(), st, fs);
            } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                if (child->getTagName() == "br") {
                    cum_x = 0.0f;
                } else {
                    const auto& cs = child->getComputedStyle();
                    float cfs = cs.font_size > 0.0f ? cs.font_size : fs;
                    if (walk(child, cs, cfs)) return true;
                }
            }
        }
        return false;
    };

    walk(ce_root, ce_style, ce_fs);
    return found ? result_x : origin_x;
}

// If a text node is immediately preceded by a <br> in the same parent, its visual
// line starts from the container line start. In this case IFC text layout x can be
// stale (still on previous run), so caret/selection should use computed base X.
static bool hasImmediatePrecedingBr(const dom::DOMNodePtr& text_node) {
    if (!text_node) return false;
    auto parent = text_node->getParent();
    if (!parent) return false;
    const auto& siblings = parent->getChildren();
    for (size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i] == text_node) {
            if (i == 0) return false;
            auto prev = siblings[i - 1];
            return prev &&
                   prev->getType() == dom::DOMNode::NodeType::ELEMENT &&
                   prev->getTagName() == "br";
        }
    }
    return false;
}

// Compute the line Y offset for a text node by counting <br> elements
// that precede it in document order. Walks up to the contenteditable root
// (or nearest block ancestor) so that text inside inline <span> elements
// correctly accounts for <br> siblings of the span.
static float computeTextNodeLineY(const dom::DOMNodePtr& text_node,
                                    layout::Engine* layout_engine) {
    if (!text_node) return 0.0f;

    // Find the contenteditable root (or nearest block-level ancestor)
    auto ce_root = text_node->getParent();
    while (ce_root) {
        if (ce_root->hasAttribute("contenteditable")) break;
        auto next = ce_root->getParent();
        if (!next) break;
        ce_root = next;
    }
    if (!ce_root) return 0.0f;

    const auto& style = ce_root->getComputedStyle();
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
    float line_height = font_size * 1.2f;
    if (style.has_line_height && style.line_height > 0.0f) {
        if (style.line_height_is_unitless) {
            line_height = style.line_height * font_size;
        } else {
            line_height = style.line_height;
        }
    }

    // Walk CE root's children in document order, counting <br> elements
    // before we reach the target text_node (which may be nested in a <span>).
    float y_offset = 0.0f;
    bool found = false;

    // Recursive lambda to traverse in document order
    std::function<bool(const dom::DOMNodePtr&)> walk;
    walk = [&](const dom::DOMNodePtr& node) -> bool {
        if (node == text_node) { found = true; return true; }
        if (node->getType() == dom::DOMNode::NodeType::ELEMENT &&
            node->getTagName() == "br") {
            y_offset += line_height;
            return false;
        }
        for (const auto& child : node->getChildren()) {
            if (walk(child)) return true;
        }
        return false;
    };

    walk(ce_root);
    return y_offset;
}

void paintContentEditableCaret(const dom::DOMNodePtr& focused_editable,
                               const dom::Selection& selection,
                               layout::Engine* layout_engine,
                               TextShaper& shaper,
                               DisplayListBuilder& builder,
                               bool caret_visible) {
    if (!focused_editable || !layout_engine || !caret_visible) return;
    if (selection.getRangeCount() == 0) return;

    auto* range = const_cast<dom::Selection&>(selection).getRangeAt(0);
    if (!range || !range->isCollapsed()) return;

    auto container = range->getStartContainer();
    uint32_t offset = range->getStartOffset();
    if (!container) return;

    // Find the layout node for the caret position
    // Always walk up to the CE root for Y computation to avoid double-counting
    // line offsets (inline <span> layout Y already includes line position).
    dom::DOMNodePtr ce_root = container;
    if (ce_root->getType() == dom::DOMNode::NodeType::TEXT) {
        ce_root = ce_root->getParent();
    }
    while (ce_root && !ce_root->hasAttribute("contenteditable")) {
        ce_root = ce_root->getParent();
    }

    auto* layout_node = ce_root ? layout_engine->getLayout(ce_root) : nullptr;
    if (!layout_node) {
        // fallback: walk up from container parent
        dom::DOMNodePtr lt = container;
        if (lt->getType() == dom::DOMNode::NodeType::TEXT) lt = lt->getParent();
        while (lt) {
            layout_node = layout_engine->getLayout(lt);
            if (layout_node) { ce_root = lt; break; }
            lt = lt->getParent();
        }
        if (!layout_node) return;
    }

    const auto& style = ce_root->getComputedStyle();
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
    float caret_height = font_size * 1.2f;

    float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float border_top = style.border_top_width >= 0.0f ? style.border_top_width : 0.0f;

    // Compute caret X and Y
    float caret_x, caret_y;

    if (container->getType() == dom::DOMNode::NodeType::TEXT) {
        caret_x = computeCaretXFromRendering(container, offset, layout_engine, shaper);
        caret_y = layout_node->y + border_top + pad_top;
        caret_y += computeTextNodeLineY(container, layout_engine);
    } else {
        float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
        float border_left = style.border_left_width >= 0.0f ? style.border_left_width : 0.0f;
        caret_x = layout_node->x + border_left + pad_left;
        caret_y = layout_node->y + border_top + pad_top;
    }

    // Resolve caret color
    Color caret_color = {0, 0, 0, 255};
    if (!style.caret_color.empty() && style.caret_color != "auto") {
        caret_color = painter_detail::makeColorFromCss(style.caret_color);
    } else if (!style.color.empty()) {
        caret_color = painter_detail::makeColorFromCss(style.color);
    }

    // Draw caret as a thin rectangle (1px wide)
    Rect caret_rect;
    caret_rect.x = caret_x;
    caret_rect.y = caret_y;
    caret_rect.width = 1.0f;
    caret_rect.height = caret_height;

    builder.addRect(caret_rect, caret_color);
}

void paintSelectionHighlight(const dom::DOMNodePtr& focused_editable,
                              const dom::Selection& selection,
                              layout::Engine* layout_engine,
                              TextShaper& shaper,
                              DisplayListBuilder& builder) {
    if (!focused_editable || !layout_engine) return;
    if (selection.getRangeCount() == 0 || selection.isCollapsed()) return;

    auto* range = const_cast<dom::Selection&>(selection).getRangeAt(0);
    if (!range) return;

    auto start_container = range->getStartContainer();
    uint32_t start_offset = range->getStartOffset();
    auto end_container = range->getEndContainer();
    uint32_t end_offset = range->getEndOffset();
    if (!start_container || !end_container) return;

    // Selection highlight color (standard blue, semi-transparent)
    Color highlight_color = {0x33 / 255.0f, 0x99 / 255.0f, 0xFF / 255.0f, 0x60 / 255.0f};

    // Lambda: highlight a portion of a text node
    auto highlightTextNode = [&](const dom::DOMNodePtr& text_node,
                                  uint32_t from_offset, uint32_t to_offset) {
        if (!text_node || text_node->getType() != dom::DOMNode::NodeType::TEXT) return;
        auto parent = text_node->getParent();
        if (!parent) return;

        const auto& style = parent->getComputedStyle();
        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
        float lh = font_size * 1.2f;

        dom::DOMNodePtr ce_root = parent;
        while (ce_root && !ce_root->hasAttribute("contenteditable")) {
            ce_root = ce_root->getParent();
        }
        auto* ce_ln = ce_root ? layout_engine->getLayout(ce_root) : nullptr;
        if (!ce_ln) return;

        const auto& ce_style = ce_root->getComputedStyle();
        float pad_top = ce_style.padding_top.isPixel() ? ce_style.padding_top.value : 0.0f;
        float border_top = ce_style.border_top_width >= 0.0f ? ce_style.border_top_width : 0.0f;
        float by = ce_ln->y + border_top + pad_top + computeTextNodeLineY(text_node, layout_engine);

        float sx = computeCaretXFromRendering(text_node, from_offset, layout_engine, shaper);
        float ex = computeCaretXFromRendering(text_node, to_offset, layout_engine, shaper);

        if (ex > sx) {
            Rect rect;
            rect.x = sx;
            rect.y = by;
            rect.width = ex - sx;
            rect.height = lh;
            builder.addRect(rect, highlight_color);
        }
    };

    // Same text node
    if (start_container == end_container &&
        start_container->getType() == dom::DOMNode::NodeType::TEXT) {
        highlightTextNode(start_container, start_offset, end_offset);
        return;
    }

    // Multi-node: highlight start, intermediate, and end text nodes
    // Collect all text nodes under the editable root to find intermediates
    std::vector<dom::DOMNodePtr> all_text;
    collectTextNodes(focused_editable, all_text);

    // Resolve start/end to text nodes
    dom::DOMNodePtr start_text = start_container;
    uint32_t s_off = start_offset;
    dom::DOMNodePtr end_text = end_container;
    uint32_t e_off = end_offset;

    if (start_text->getType() != dom::DOMNode::NodeType::TEXT) {
        if (!all_text.empty()) { start_text = all_text.front(); s_off = 0; }
        else return;
    }
    if (end_text->getType() != dom::DOMNode::NodeType::TEXT) {
        if (!all_text.empty()) {
            end_text = all_text.back();
            e_off = static_cast<uint32_t>(end_text->getRawTextContent().size());
        } else return;
    }

    int start_idx = -1, end_idx = -1;
    for (int i = 0; i < static_cast<int>(all_text.size()); ++i) {
        if (all_text[i] == start_text) start_idx = i;
        if (all_text[i] == end_text) end_idx = i;
    }
    if (start_idx < 0 || end_idx < 0 || start_idx > end_idx) return;

    // Highlight start node (from start_offset to end of text)
    if (start_text->getType() == dom::DOMNode::NodeType::TEXT) {
        uint32_t text_len = static_cast<uint32_t>(start_text->getRawTextContent().size());
        highlightTextNode(start_text, s_off, text_len);
    }

    // Highlight intermediate nodes (entire text)
    for (int i = start_idx + 1; i < end_idx; ++i) {
        uint32_t text_len = static_cast<uint32_t>(all_text[i]->getRawTextContent().size());
        highlightTextNode(all_text[i], 0, text_len);
    }

    // Highlight end node (from 0 to end_offset)
    if (end_text->getType() == dom::DOMNode::NodeType::TEXT && end_text != start_text) {
        highlightTextNode(end_text, 0, e_off);
    }
}

// ========== Input/Textarea caret and selection rendering ==========

// Helper: compute the pixel X offset for a given character index in an input's value
static float computeInputTextXOffset(const std::string& value,
                                      size_t char_index,
                                      const dom::ComputedStyle& style,
                                      float font_size,
                                      TextShaper& shaper) {
    if (char_index == 0 || value.empty()) return 0.0f;

    // Convert char_index to byte offset
    size_t byte_offset = 0;
    size_t count = 0;
    for (size_t i = 0; i < value.size() && count < char_index; ) {
        unsigned char c = value[i];
        if ((c & 0x80) == 0) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else i += 4;
        count++;
        byte_offset = i;
    }

    std::string prefix = value.substr(0, byte_offset);
    TextShapeRequest req;
    req.text = prefix;
    req.font_family = style.font_family;
    req.font_size = font_size;
    req.font_weight = toString(style.font_weight);
    req.font_style = toString(style.font_style);

    ShapedText shaped;
    if (shaper.shape(req, shaped) && shaped.units_per_em > 0) {
        float scale = font_size / static_cast<float>(shaped.units_per_em);
        return shaped.width_units * scale;
    }
    return prefix.size() * font_size * 0.6f;
}

// Helper: get input layout coordinates (content area origin)
struct InputLayoutInfo {
    float content_x = 0;   // x of text content area start
    float content_y = 0;   // y of text content area start
    float content_w = 0;   // width of text content area
    float content_h = 0;   // height of text content area
    float font_size = 16.0f;
    float pad_left = 0;
    bool valid = false;
};

static InputLayoutInfo getInputLayoutInfo(const dom::DOMNodePtr& node,
                                           layout::Engine* layout_engine) {
    InputLayoutInfo info;
    auto* layout_node = layout_engine->getLayout(node);
    if (!layout_node) return info;

    const auto& style = node->getComputedStyle();

    float bl = style.border_left_width >= 0.0f ? style.border_left_width : 0.0f;
    float bt = style.border_top_width >= 0.0f ? style.border_top_width : 0.0f;
    float br = style.border_right_width >= 0.0f ? style.border_right_width : 0.0f;
    float bb = style.border_bottom_width >= 0.0f ? style.border_bottom_width : 0.0f;

    // For border style "none"/"hidden", width is 0
    auto effectiveBW = [&](float w, dom::CSSBorderStyle side_style) -> float {
        if (w < 0.0f) w = 0.0f;
        dom::CSSBorderStyle st = (side_style != dom::CSSBorderStyleUnset) ? side_style : style.border_style;
        if (st == dom::CSSBorderStyle::None || st == dom::CSSBorderStyle::Hidden) return 0.0f;
        return w;
    };
    bl = effectiveBW(bl, style.border_left_style);
    bt = effectiveBW(bt, style.border_top_style);
    br = effectiveBW(br, style.border_right_style);
    bb = effectiveBW(bb, style.border_bottom_style);

    float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    float pad_t = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;

    info.content_x = layout_node->layout.position[0] + bl + pad_l;
    info.content_y = layout_node->layout.position[1] + bt + pad_t;
    info.content_w = std::max(0.0f, layout_node->layout.dimensions[0] - bl - br
                     - pad_l - (style.padding_right.isPixel() ? style.padding_right.value : 0.0f));
    info.content_h = std::max(0.0f, layout_node->layout.dimensions[1] - bt - bb
                     - pad_t - (style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f));
    info.font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
    info.pad_left = pad_l;
    info.valid = true;
    return info;
}

// Helper: get the effective line height for an input/textarea element.
static float getEffectiveLineHeight(const dom::ComputedStyle& style, float font_size, TextShaper& shaper) {
    TextShapeRequest mreq{"X", style.font_family, toString(style.font_weight), toString(style.font_style), font_size};
    ShapedText mshaped;
    if (shaper.shape(mreq, mshaped) && mshaped.scale_to_pixels > 0.0f) {
        float scale = mshaped.scale_to_pixels;
        float lh_units = mshaped.line_height_units;
        if (style.line_height > 0.0f) {
            if (style.line_height_is_unitless)
                lh_units = (style.line_height * font_size) / std::max(scale, 1e-3f);
            else
                lh_units = style.line_height / std::max(scale, 1e-3f);
        }
        if (lh_units <= 0.0f) lh_units = font_size / std::max(scale, 1e-3f);
        return lh_units * scale;
    }
    return font_size * 1.2f;
}

// Helper: split a string by '\n' into lines.
static std::vector<std::string> splitLines(const std::string& s) {
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

// Helper: count UTF-8 characters in a string.
static size_t utf8Len(const std::string& s) {
    size_t count = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = s[i];
        if ((c & 0x80) == 0) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else i += 4;
        count++;
    }
    return count;
}

// Resolve a global char index to (line_index, column_chars_within_line).
// chars_before_line is set to the total chars before the returned line.
struct LineCol {
    int line = 0;
    size_t col = 0;          // char offset within the line
    std::string line_text;   // the text of this line
};
static LineCol resolveLineCol(const std::string& value, size_t char_index) {
    auto lines = splitLines(value);
    size_t chars_consumed = 0;
    for (int i = 0; i < static_cast<int>(lines.size()); i++) {
        size_t line_chars = utf8Len(lines[i]);
        if (char_index <= chars_consumed + line_chars) {
            return {i, char_index - chars_consumed, lines[i]};
        }
        chars_consumed += line_chars + 1; // +1 for '\n'
    }
    // Past end: last line, end position
    if (!lines.empty()) {
        return {static_cast<int>(lines.size()) - 1, utf8Len(lines.back()), lines.back()};
    }
    return {0, 0, ""};
}

void paintInputCaret(const dom::DOMNodePtr& input_node,
                     layout::Engine* layout_engine,
                     TextShaper& shaper,
                     DisplayListBuilder& builder) {
    if (!input_node || !layout_engine) return;

    auto* state = dom::getInputState(input_node);
    if (!state) return;

    // Don't draw caret if there's a selection
    if (state->hasSelection()) return;

    auto info = getInputLayoutInfo(input_node, layout_engine);
    if (!info.valid) return;

    const auto& style = input_node->getComputedStyle();
    bool is_textarea = (input_node->getTagName() == "textarea");
    float line_height = is_textarea ? getEffectiveLineHeight(style, info.font_size, shaper) : (info.font_size * 1.2f);

    // Resolve caret position
    float caret_x, caret_y;
    if (is_textarea) {
        auto lc = resolveLineCol(state->getValue(), state->getCursorPosition());
        caret_x = info.content_x + computeInputTextXOffset(lc.line_text, lc.col, style, info.font_size, shaper);
        caret_y = info.content_y + lc.line * line_height;
    } else {
        caret_x = info.content_x + computeInputTextXOffset(
            state->getValue(), state->getCursorPosition(), style, info.font_size, shaper);
        caret_y = info.content_y + (info.content_h - line_height) * 0.5f;
        if (caret_y < info.content_y) caret_y = info.content_y;
    }

    // Resolve caret color
    Color caret_color = {0, 0, 0, 255};
    if (!style.caret_color.empty() && style.caret_color != "auto") {
        caret_color = painter_detail::makeColorFromCss(style.caret_color);
    } else if (!style.color.empty()) {
        caret_color = painter_detail::makeColorFromCss(style.color);
    }

    Rect caret_rect;
    caret_rect.x = caret_x;
    caret_rect.y = caret_y;
    caret_rect.width = 1.0f;
    caret_rect.height = line_height;

    builder.addRect(caret_rect, caret_color);
}

void paintInputSelectionHighlight(const dom::DOMNodePtr& input_node,
                                   layout::Engine* layout_engine,
                                   TextShaper& shaper,
                                   DisplayListBuilder& builder) {
    if (!input_node || !layout_engine) return;

    auto* state = dom::getInputState(input_node);
    if (!state || !state->hasSelection()) return;

    auto info = getInputLayoutInfo(input_node, layout_engine);
    if (!info.valid) return;

    const auto& style = input_node->getComputedStyle();
    bool is_textarea = (input_node->getTagName() == "textarea");
    float line_height = is_textarea ? getEffectiveLineHeight(style, info.font_size, shaper) : (info.font_size * 1.2f);

    size_t sel_start = state->getSelectionStart();
    size_t sel_end = state->getSelectionEnd();

    // Selection highlight color: blue semi-transparent
    Color highlight_color = {0x33 / 255.0f, 0x99 / 255.0f, 0xFF / 255.0f, 0x60 / 255.0f};

    if (is_textarea) {
        auto lc_start = resolveLineCol(state->getValue(), sel_start);
        auto lc_end = resolveLineCol(state->getValue(), sel_end);

        for (int line = lc_start.line; line <= lc_end.line; line++) {
            // Determine the start/end column for this line
            size_t col_start = (line == lc_start.line) ? lc_start.col : 0;
            std::string line_text;
            // Resolve line text
            if (line == lc_start.line) line_text = lc_start.line_text;
            else if (line == lc_end.line) line_text = lc_end.line_text;
            else {
                // Intermediate line — resolve it
                auto lines = splitLines(state->getValue());
                if (line < static_cast<int>(lines.size())) line_text = lines[line];
            }
            size_t line_chars = utf8Len(line_text);
            size_t col_end = (line == lc_end.line) ? lc_end.col : line_chars;

            float sx = info.content_x + computeInputTextXOffset(line_text, col_start, style, info.font_size, shaper);
            float ex = info.content_x + computeInputTextXOffset(line_text, col_end, style, info.font_size, shaper);
            // For lines that extend to end, add a small extra width to show the selection
            if (line != lc_end.line && col_end == line_chars) {
                ex += info.font_size * 0.3f;
            }

            if (ex > sx) {
                Rect rect;
                rect.x = sx;
                rect.y = info.content_y + line * line_height;
                rect.width = ex - sx;
                rect.height = line_height;
                builder.addRect(rect, highlight_color);
            }
        }
    } else {
        // Single-line input
        const std::string& value = state->getValue();
        float sx = info.content_x + computeInputTextXOffset(value, sel_start, style, info.font_size, shaper);
        float ex = info.content_x + computeInputTextXOffset(value, sel_end, style, info.font_size, shaper);

        if (ex <= sx) return;

        float hy = info.content_y + (info.content_h - line_height) * 0.5f;
        if (hy < info.content_y) hy = info.content_y;

        Rect rect;
        rect.x = sx;
        rect.y = hy;
        rect.width = ex - sx;
        rect.height = line_height;
        builder.addRect(rect, highlight_color);
    }
}

} // namespace dong::render
