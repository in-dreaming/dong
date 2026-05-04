// Text hit testing implementation

#include "text_hit_testing.hpp"
#include "../layout/layout_engine.hpp"
#include "../render/text_shaper.hpp"
#include "../core/string_utils.h"
#include "../core/log.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>

namespace dong::dom {

static bool debugCeHitTestEnabled() {
    const char* v = std::getenv("DONG_DEBUG_CE_HITTEST");
    return v && v[0] == '1';
}

// CE caret hit-testing tends to feel slightly right-shifted in real GUI clicks.
// Keep a small global left bias so "click before glyph" maps to expected boundary.
// NOTE: too large bias hurts narrow trailing glyphs (e.g. 't', 'y') at line end.
static constexpr float kCeCaretLeftBiasPx = 1.0f;
static constexpr float kCeCaretStartSnapPx = 2.0f;
static constexpr float kCeCaretEndSnapPx = 1.5f;

// Snap a byte offset to the nearest UTF-8 character boundary (backward).
static uint32_t utf8SnapBack(const std::string& s, uint32_t offset) {
    if (offset == 0 || offset >= s.size()) return offset;
    while (offset > 0 && (static_cast<uint8_t>(s[offset]) & 0xC0) == 0x80) {
        offset--;
    }
    return offset;
}

// Advance to the start of the next UTF-8 character after the one at `offset`.
static uint32_t utf8NextChar(const std::string& s, uint32_t offset) {
    if (offset >= s.size()) return static_cast<uint32_t>(s.size());
    uint8_t c = static_cast<uint8_t>(s[offset]);
    uint32_t len = 1;
    if (c >= 0xF0) len = 4;
    else if (c >= 0xE0) len = 3;
    else if (c >= 0xC0) len = 2;
    uint32_t next = offset + len;
    return next > static_cast<uint32_t>(s.size()) ? static_cast<uint32_t>(s.size()) : next;
}

// Map a byte offset in collapsed text back to the corresponding byte offset in raw text.
// collapseWhitespace: collapses consecutive whitespace to single space, trims leading/trailing.
static uint32_t collapsedOffsetToRaw(const std::string& raw, uint32_t collapsed_offset) {
    if (collapsed_offset == 0) {
        // Offset 0 in collapsed = skip leading whitespace in raw
        uint32_t i = 0;
        while (i < raw.size() && dong::isAsciiWhitespace(raw[i])) i++;
        return i;
    }

    // Walk through raw text, tracking the collapsed offset
    uint32_t col_pos = 0;   // current position in collapsed text
    bool in_space = false;
    bool started = false;    // have we passed leading whitespace?

    for (uint32_t i = 0; i < static_cast<uint32_t>(raw.size()); i++) {
        if (col_pos >= collapsed_offset) return i;

        if (dong::isAsciiWhitespace(raw[i])) {
            if (started && !in_space) {
                col_pos++;  // first whitespace becomes a single space
                if (col_pos >= collapsed_offset) return i + 1;
            }
            in_space = true;
        } else {
            started = true;
            in_space = false;
            col_pos++;
        }
    }
    return static_cast<uint32_t>(raw.size());
}

TextHitResult TextHitTester::hitTestAt(const DOMNodePtr& root,
                                        dong::layout::Engine* layout_engine,
                                        int32_t x, int32_t y,
                                        dong::render::TextShaper* shaper) {
    if (!root || !layout_engine) return {};
    return hitTestRecursive(root, layout_engine, x, y, shaper);
}

// Compute the pixel width of a text string using text shaper or fallback.
static float measureTextWidth(const std::string& text,
                               const dong::dom::ComputedStyle& style,
                               float font_size,
                               dong::render::TextShaper* shaper) {
    if (text.empty()) return 0.0f;
    if (shaper) {
        dong::render::TextShapeRequest req;
        req.text = text;
        req.font_family = style.font_family;
        req.font_size = font_size;
        req.font_weight = toString(style.font_weight);
        req.font_style = toString(style.font_style);
        dong::render::ShapedText shaped;
        if (shaper->shape(req, shaped) && shaped.units_per_em > 0) {
            float scale = font_size / static_cast<float>(shaped.units_per_em);
            return shaped.width_units * scale;
        }
    }
    // Rough fallback: 0.6 * font_size per character
    return text.size() * font_size * 0.6f;
}

// Compute the line_height for a given style.
static float computeLineHeight(const dong::dom::ComputedStyle& style, float font_size) {
    float line_height = font_size * 1.2f;
    if (style.has_line_height && style.line_height > 0.0f) {
        if (style.line_height_is_unitless) {
            line_height = style.line_height * font_size;
        } else {
            line_height = style.line_height;
        }
    }
    return line_height;
}

// Compute the line Y offset for a text node by counting <br> elements
// that precede it in document order within the contenteditable root.
// This handles text inside inline <span> elements correctly (BRs are
// siblings of the span, not children of it).
static float computeTextNodeBrLineY(const DOMNodePtr& text_node,
                                      float font_size) {
    if (!text_node) return 0.0f;

    // Walk up to contenteditable root
    auto ce_root = text_node->getParent();
    while (ce_root) {
        if (ce_root->hasAttribute("contenteditable")) break;
        auto next = ce_root->getParent();
        if (!next) break;
        ce_root = next;
    }
    if (!ce_root) return 0.0f;

    const auto& ce_style = ce_root->getComputedStyle();
    float ce_font_size = ce_style.font_size > 0.0f ? ce_style.font_size : 16.0f;
    float line_height = computeLineHeight(ce_style, ce_font_size);

    // Document-order traversal counting BRs before text_node
    float y_offset = 0.0f;
    bool found = false;

    std::function<bool(const DOMNodePtr&)> walk;
    walk = [&](const DOMNodePtr& node) -> bool {
        if (node == text_node) { found = true; return true; }
        if (node->getType() == DOMNode::NodeType::ELEMENT &&
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

TextHitResult TextHitTester::hitTestRecursive(const DOMNodePtr& node,
                                               dong::layout::Engine* layout_engine,
                                               int32_t x, int32_t y,
                                               dong::render::TextShaper* shaper) {
    if (!node || !layout_engine) return {};

    // If this is a text node, check if click is within its computed text region
    if (node->getType() == DOMNode::NodeType::TEXT) {
        auto parent = node->getParent();
        if (!parent) return {};

        // FAST PATH: If the text node itself has a layout entry (from IFC layout),
        // use its X for horizontal positioning (accurate), but compute Y bounds
        // from parent content area + BR counting (IFC Y includes half-leading offset
        // which doesn't match visual line positions).
        const auto* text_layout = layout_engine->getLayout(node);
        if (text_layout && text_layout->width > 0) {
            float tlx = text_layout->x;
            float tw = text_layout->width;

            // Walk up to contenteditable root for Y computation.
            // We must NOT use parent or span layout Y because IFC layout Y
            // includes half-leading offset that doesn't match visual line top.
            auto ce_root = parent;
            while (ce_root) {
                if (ce_root->hasAttribute("contenteditable")) break;
                auto next = ce_root->getParent();
                if (!next) break;
                ce_root = next;
            }
            const auto* ce_layout = ce_root ? layout_engine->getLayout(ce_root) : nullptr;
            if (!ce_layout) {
                // fallback: walk up from parent
                auto layout_ancestor = parent;
                ce_layout = layout_engine->getLayout(layout_ancestor);
                while (!ce_layout && layout_ancestor) {
                    layout_ancestor = layout_ancestor->getParent();
                    if (layout_ancestor) ce_layout = layout_engine->getLayout(layout_ancestor);
                }
                if (!ce_layout) return {};
                ce_root = layout_ancestor;
            }

            const auto& style = parent->getComputedStyle();
            float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
            // Use CE root's padding/border for Y, not parent's (parent could be inline span)
            const auto& ce_style = ce_root->getComputedStyle();
            float pad_top = ce_style.padding_top.isPixel() ? ce_style.padding_top.value : 0.0f;
            float border_top = ce_style.border_top_width >= 0.0f ? ce_style.border_top_width : 0.0f;
            float line_height = computeLineHeight(ce_style, ce_style.font_size > 0.0f ? ce_style.font_size : 16.0f);
            float br_y_offset = computeTextNodeBrLineY(node, font_size);

            float line_top = ce_layout->y + border_top + pad_top + br_y_offset;
            float line_bottom = line_top + line_height;

            bool parent_is_ce = parent->isContentEditable();

            // Determine if this is the last/first text node in the CE root subtree.
            bool is_last_text = false;
            bool is_first_text = false;
            if (parent_is_ce && ce_root) {
                // Find first and last text nodes in document order under CE root
                std::function<DOMNodePtr(const DOMNodePtr&)> findFirst;
                findFirst = [&](const DOMNodePtr& n) -> DOMNodePtr {
                    if (n->getType() == DOMNode::NodeType::TEXT) return n;
                    for (const auto& c : n->getChildren()) {
                        auto r = findFirst(c);
                        if (r) return r;
                    }
                    return nullptr;
                };
                std::function<DOMNodePtr(const DOMNodePtr&)> findLast;
                findLast = [&](const DOMNodePtr& n) -> DOMNodePtr {
                    if (n->getType() == DOMNode::NodeType::TEXT) return n;
                    const auto& ch = n->getChildren();
                    for (auto it = ch.rbegin(); it != ch.rend(); ++it) {
                        auto r = findLast(*it);
                        if (r) return r;
                    }
                    return nullptr;
                };
                is_first_text = (findFirst(ce_root) == node);
                is_last_text = (findLast(ce_root) == node);
            }

            // Check vertical bounds using computed line positions.
            if (y < line_top && !is_first_text) {
                return {};
            }
            if (y > line_bottom && !is_last_text) {
                return {};
            }

            // Check horizontal: reject only if click is left of this text node
            if (x < tlx) {
                return {};
            }

            TextHitResult result;
            result.text_node = node;
            result.found = true;

            float local_x = static_cast<float>(x) - tlx;
            if (local_x < 0.0f) local_x = 0.0f;

            const std::string& raw_text = node->getRawTextContent();
            // IFC layout uses collapsed whitespace for measurement.
            // We must use the same collapsed text for binary search, then map back.
            std::string text = dong::collapseWhitespace(raw_text);

            // End-of-text check: click at or near right edge -> cursor after last char.
            // Do this BEFORE applying the left bias so narrow trailing glyphs
            // (like 't', 'y') are not skipped by the bias.
            if (local_x >= tw - kCeCaretEndSnapPx) {
                result.char_offset = collapsedOffsetToRaw(raw_text, static_cast<uint32_t>(text.size()));
                if (debugCeHitTestEnabled()) {
                    DONG_LOG_WARN("[CE-HitTest][FAST-END] x=%d y=%d tlx=%.1f tw=%.1f local_x=%.2f -> off=%u text=\"%s\"",
                                  x, y, tlx, tw, local_x, result.char_offset, raw_text.c_str());
                }
                return result;
            }

            // Apply a tiny left bias so "click before char" maps to the expected boundary.
            local_x = std::max(0.0f, local_x - kCeCaretLeftBiasPx);

            // Left-edge snap: clicking very close to glyph run start should place
            // caret before the first character, not after it.
            if (local_x <= kCeCaretStartSnapPx) {
                result.char_offset = collapsedOffsetToRaw(raw_text, 0);
                if (debugCeHitTestEnabled()) {
                    DONG_LOG_WARN("[CE-HitTest][FAST] x=%d y=%d tlx=%.1f line=[%.1f,%.1f] local_x=%.2f -> off=%u text=\"%s\"",
                                  x, y, tlx, line_top, line_bottom, local_x, result.char_offset, raw_text.c_str());
                }
                return result;
            }

            if (text.empty() || !shaper) {
                if (tw > 0.0f && !text.empty()) {
                    float ratio = std::clamp(local_x / tw, 0.0f, 1.0f);
                    uint32_t col_offset = static_cast<uint32_t>(ratio * text.size());
                    result.char_offset = collapsedOffsetToRaw(raw_text, col_offset);
                } else {
                    result.char_offset = collapsedOffsetToRaw(raw_text, 0);
                }
            } else {
                // Binary search with shaper on collapsed text
                uint32_t lo = 0, hi = static_cast<uint32_t>(text.size());
                while (lo < hi) {
                    uint32_t mid = lo + (hi - lo) / 2;
                    mid = utf8SnapBack(text, mid);
                    if (mid <= lo && lo < hi) { mid = lo; }
                    dong::render::TextShapeRequest req;
                    req.text = text.substr(0, mid);
                    req.font_family = style.font_family;
                    req.font_size = font_size;
                    req.font_weight = toString(style.font_weight);
                    req.font_style = toString(style.font_style);
                    dong::render::ShapedText shaped;
                    if (shaper->shape(req, shaped) && shaped.units_per_em > 0) {
                        float scale = font_size / static_cast<float>(shaped.units_per_em);
                        float prefix_w = shaped.width_units * scale;
                        if (prefix_w < local_x) lo = utf8NextChar(text, mid);
                        else hi = mid;
                    } else {
                        lo = utf8NextChar(text, mid);
                    }
                }
                // Snap to nearest character boundary (left or right edge)
                lo = utf8SnapBack(text, lo);
                if (lo > 0 && lo <= static_cast<uint32_t>(text.size())) {
                    uint32_t prev_lo = utf8SnapBack(text, lo - 1);
                    dong::render::TextShapeRequest req_prev;
                    req_prev.text = text.substr(0, prev_lo);
                    req_prev.font_family = style.font_family;
                    req_prev.font_size = font_size;
                    req_prev.font_weight = toString(style.font_weight);
                    req_prev.font_style = toString(style.font_style);
                    dong::render::ShapedText shaped_prev;
                    if (shaper->shape(req_prev, shaped_prev) && shaped_prev.units_per_em > 0) {
                        float scale = font_size / static_cast<float>(shaped_prev.units_per_em);
                        float left_edge = shaped_prev.width_units * scale;
                        dong::render::TextShapeRequest req_cur;
                        req_cur.text = text.substr(0, lo);
                        req_cur.font_family = style.font_family;
                        req_cur.font_size = font_size;
                        req_cur.font_weight = toString(style.font_weight);
                        req_cur.font_style = toString(style.font_style);
                        dong::render::ShapedText shaped_cur;
                        if (shaper->shape(req_cur, shaped_cur) && shaped_cur.units_per_em > 0) {
                            float right_edge = shaped_cur.width_units * (font_size / static_cast<float>(shaped_cur.units_per_em));
                            float char_mid = (left_edge + right_edge) / 2.0f;
                            if (local_x < char_mid) {
                                lo = prev_lo;
                            }
                        }
                    }
                }
                uint32_t col_offset = std::min(lo, static_cast<uint32_t>(text.size()));
                result.char_offset = collapsedOffsetToRaw(raw_text, col_offset);
            }
            if (debugCeHitTestEnabled()) {
                DONG_LOG_WARN("[CE-HitTest][FAST] x=%d y=%d tlx=%.1f line=[%.1f,%.1f] local_x=%.2f -> off=%u text=\"%s\"",
                              x, y, tlx, line_top, line_bottom, local_x, result.char_offset, raw_text.c_str());
            }
            return result;
        }

        // SLOW PATH: text node has no layout entry — fall back to parent layout + manual computation
        auto layout_ancestor = parent;
        const auto* layout = layout_engine->getLayout(layout_ancestor);
        while (!layout && layout_ancestor) {
            layout_ancestor = layout_ancestor->getParent();
            if (layout_ancestor) {
                layout = layout_engine->getLayout(layout_ancestor);
            }
        }
        if (!layout) return {};

        float lx = layout->x;
        float ly = layout->y;
        float w = layout->width;
        float h = layout->height;

        // First check: is click within the parent's vertical bounds?
        // For contenteditable parents, skip upper Y bound check because <br> line breaks
        // can extend content beyond the layout-computed height.
        bool parent_is_ce = parent->isContentEditable();
        if (y < ly || (!parent_is_ce && y > ly + h)) {
            return {};
        }

        // Use layout ancestor's style for padding/border since layout comes from it
        const auto& la_style = layout_ancestor->getComputedStyle();
        float pad_left = la_style.padding_left.isPixel() ? la_style.padding_left.value : 0.0f;
        float pad_top = la_style.padding_top.isPixel() ? la_style.padding_top.value : 0.0f;
        float border_left = la_style.border_left_width >= 0.0f ? la_style.border_left_width : 0.0f;
        float border_top = la_style.border_top_width >= 0.0f ? la_style.border_top_width : 0.0f;

        const auto& style = parent->getComputedStyle();
        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

        float content_start_x = lx + border_left + pad_left;

        // If parent has multiple children (mixed text + element), we need to
        // compute each text node's x-range to do precise hit testing.
        // Also handle <br> line breaks: reset X and advance Y.
        const auto& siblings = parent->getChildren();
        if (siblings.size() > 1) {
            float line_height = font_size * 1.2f;
            if (style.has_line_height && style.line_height > 0.0f) {
                if (style.line_height_is_unitless) {
                    line_height = style.line_height * font_size;
                } else {
                    line_height = style.line_height;
                }
            }

            float cursor_x = content_start_x;
            float line_y = ly + border_top + pad_top;

            float text_start_x = 0.0f;
            float text_end_x = 0.0f;
            float text_line_y = 0.0f;
            bool found_self = false;

            for (const auto& sibling : siblings) {
                if (sibling->getType() == DOMNode::NodeType::ELEMENT &&
                    sibling->getTagName() == "br") {
                    cursor_x = content_start_x;
                    line_y += line_height;
                    continue;
                }

                if (sibling == node) {
                    text_start_x = cursor_x;
                    text_line_y = line_y;
                    float stw = measureTextWidth(node->getRawTextContent(), style, font_size, shaper);
                    text_end_x = cursor_x + stw;
                    found_self = true;
                    cursor_x += stw;
                } else if (sibling->getType() == DOMNode::NodeType::TEXT) {
                    float stw = measureTextWidth(sibling->getRawTextContent(), style, font_size, shaper);
                    cursor_x += stw;
                } else {
                    const auto* sib_layout = layout_engine->getLayout(sibling);
                    if (sib_layout) {
                        cursor_x += sib_layout->width;
                    }
                }
            }

            if (!found_self) return {};

            if (y < text_line_y || y > text_line_y + line_height) {
                return {};
            }

            if (x < text_start_x) {
                return {};
            }

            TextHitResult result;
            result.text_node = node;
            result.found = true;

            float local_x = static_cast<float>(x) - text_start_x;
            if (local_x < 0.0f) local_x = 0.0f;

            const std::string& text = node->getRawTextContent();

            // End-of-text check: click at or near right edge -> cursor after last char
            float text_w = text_end_x - text_start_x;
            if (text_w > 0.0f && local_x >= text_w - kCeCaretEndSnapPx) {
                result.char_offset = static_cast<uint32_t>(text.size());
                if (debugCeHitTestEnabled()) {
                    DONG_LOG_WARN("[CE-HitTest][SLOW-END] x=%d y=%d text_start_x=%.1f local_x=%.2f -> off=%u text=\"%s\"",
                                  x, y, text_start_x, local_x, result.char_offset, text.c_str());
                }
                return result;
            }

            local_x = std::max(0.0f, local_x - kCeCaretLeftBiasPx);

            if (local_x <= kCeCaretStartSnapPx) {
                result.char_offset = 0;
                if (debugCeHitTestEnabled()) {
                    DONG_LOG_WARN("[CE-HitTest][SLOW] x=%d y=%d text_start_x=%.1f line_y=%.1f local_x=%.2f -> off=%u text=\"%s\"",
                                  x, y, text_start_x, text_line_y, local_x, result.char_offset, text.c_str());
                }
                return result;
            }
            if (text.empty() || !shaper) {
                float total_w = text_end_x - text_start_x;
                if (total_w > 0.0f) {
                    float ratio = std::clamp(local_x / total_w, 0.0f, 1.0f);
                    result.char_offset = static_cast<uint32_t>(ratio * text.size());
                } else {
                    result.char_offset = 0;
                }
            } else {
                uint32_t lo = 0, hi = static_cast<uint32_t>(text.size());
                while (lo < hi) {
                    uint32_t mid = lo + (hi - lo) / 2;
                    mid = utf8SnapBack(text, mid);
                    if (mid <= lo && lo < hi) { mid = lo; }
                    dong::render::TextShapeRequest req;
                    req.text = text.substr(0, mid);
                    req.font_family = style.font_family;
                    req.font_size = font_size;
                    req.font_weight = toString(style.font_weight);
                    req.font_style = toString(style.font_style);
                    dong::render::ShapedText shaped;
                    if (shaper->shape(req, shaped) && shaped.units_per_em > 0) {
                        float scale = font_size / static_cast<float>(shaped.units_per_em);
                        float prefix_w = shaped.width_units * scale;
                        if (prefix_w < local_x) lo = utf8NextChar(text, mid);
                        else hi = mid;
                    } else {
                        lo = utf8NextChar(text, mid);
                    }
                }
                lo = utf8SnapBack(text, lo);
                if (lo > 0 && lo <= static_cast<uint32_t>(text.size())) {
                    uint32_t prev_lo = utf8SnapBack(text, lo - 1);
                    dong::render::TextShapeRequest req_prev;
                    req_prev.text = text.substr(0, prev_lo);
                    req_prev.font_family = style.font_family;
                    req_prev.font_size = font_size;
                    req_prev.font_weight = toString(style.font_weight);
                    req_prev.font_style = toString(style.font_style);
                    dong::render::ShapedText shaped_prev;
                    if (shaper->shape(req_prev, shaped_prev) && shaped_prev.units_per_em > 0) {
                        float scale = font_size / static_cast<float>(shaped_prev.units_per_em);
                        float left_edge = shaped_prev.width_units * scale;
                        dong::render::TextShapeRequest req_cur;
                        req_cur.text = text.substr(0, lo);
                        req_cur.font_family = style.font_family;
                        req_cur.font_size = font_size;
                        req_cur.font_weight = toString(style.font_weight);
                        req_cur.font_style = toString(style.font_style);
                        dong::render::ShapedText shaped_cur;
                        if (shaper->shape(req_cur, shaped_cur) && shaped_cur.units_per_em > 0) {
                            float right_edge = shaped_cur.width_units * (font_size / static_cast<float>(shaped_cur.units_per_em));
                            float char_mid = (left_edge + right_edge) / 2.0f;
                            if (local_x < char_mid) {
                                lo = prev_lo;
                            }
                        }
                    }
                }
                result.char_offset = std::min(lo, static_cast<uint32_t>(text.size()));
            }
            if (debugCeHitTestEnabled()) {
                DONG_LOG_WARN("[CE-HitTest][SLOW] x=%d y=%d text_start_x=%.1f line_y=%.1f local_x=%.2f -> off=%u text=\"%s\"",
                              x, y, text_start_x, text_line_y, local_x, result.char_offset, text.c_str());
            }
            return result;
        }

        // Single-child case: text node is the only child, use parent bounds directly
        if (x < lx || x > lx + w) return {};

        TextHitResult result;
        result.text_node = node;
        result.char_offset = estimateCharOffset(node, layout_engine, x, shaper);
        result.found = true;
        return result;
    }

    // For element nodes, check bounds then recurse into children
    const auto* layout = layout_engine->getLayout(node);
    if (layout) {
        float lx = layout->x;
        float ly = layout->y;
        float w = layout->width;
        float h = layout->height;

        // For contenteditable elements, skip Y upper-bound check because
        // dynamically inserted <br> line breaks may extend content beyond
        // the layout height (Yoga doesn't know about <br> lines).
        //
        // For inline wrappers (e.g. execCommand creates <span>), do not do strict
        // element-box clipping here: inline layout boxes can be narrow/offset and
        // still contain hittable text descendants. Let text-node hit testing decide.
        bool is_ce = node->isContentEditable();
        bool is_inline_like = false;
        if (node->getType() == DOMNode::NodeType::ELEMENT) {
            const auto& cs = node->getComputedStyle();
            const auto d = cs.display;
            is_inline_like = (d == CSSDisplay::Inline || d == CSSDisplay::InlineBlock || d == CSSDisplay::InlineFlex || d == CSSDisplay::InlineTable);
        }
        const bool has_reliable_box = (w > 0.0f && h > 0.0f);
        const bool strict_clip = !is_ce && !is_inline_like && has_reliable_box;
        if (strict_clip) {
            if (x < lx || x > lx + w || y < ly || y > ly + h) {
                return {};
            }
        }
    }

    // Check children in reverse order (front-to-back)
    const auto& children = node->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto result = hitTestRecursive(*it, layout_engine, x, y, shaper);
        if (result.found) return result;
    }

    // If this is the contenteditable ROOT element and no child matched,
    // find the nearest text node on the closest line and snap to it.
    // This handles clicks past line ends, in empty space below text, etc.
    // IMPORTANT: only trigger on the actual CE root (has contenteditable attribute),
    // NOT on descendants that merely inherit contenteditable (like <b>, <span>).
    // Otherwise inline wrappers would absorb clicks meant for other lines.
    const bool is_ce_root = node->hasAttribute("contenteditable") &&
                            node->getAttribute("contenteditable") != "false";
    if (is_ce_root && !children.empty()) {
        // Collect all text nodes under this CE root
        std::vector<DOMNodePtr> text_nodes;
        std::function<void(const DOMNodePtr&)> collect;
        collect = [&](const DOMNodePtr& n) {
            if (n->getType() == DOMNode::NodeType::TEXT) {
                text_nodes.push_back(n);
                return;
            }
            for (const auto& c : n->getChildren()) collect(c);
        };
        collect(node);

        if (!text_nodes.empty()) {
            // Compute line + x-distance for each text node and pick the best insertion target.
            // This fixes cases where clicking in line-2 left blank area incorrectly snaps to
            // line-1 text, simply because line-mid distance happened to be similar.
            const auto& ce_style = node->getComputedStyle();
            float ce_fs = ce_style.font_size > 0.0f ? ce_style.font_size : 16.0f;
            float ce_lh = computeLineHeight(ce_style, ce_fs);
            float pad_top = ce_style.padding_top.isPixel() ? ce_style.padding_top.value : 0.0f;
            float border_top = ce_style.border_top_width >= 0.0f ? ce_style.border_top_width : 0.0f;
            float base_y = (layout ? layout->y : 0.0f) + border_top + pad_top;

            DOMNodePtr best_node;
            float best_line_penalty = 1e9f; // 0 when y is in line box, else distance to line box
            float best_x_penalty = 1e9f;    // distance to [text_left, text_right]
            bool best_is_after = false;

            for (const auto& tn : text_nodes) {
                float br_y = computeTextNodeBrLineY(tn, ce_fs);
                float line_top = base_y + br_y;
                float line_bottom = line_top + ce_lh;
                float y_f = static_cast<float>(y);
                float line_penalty = 0.0f;
                if (y_f < line_top) {
                    line_penalty = line_top - y_f;
                } else if (y_f > line_bottom) {
                    line_penalty = y_f - line_bottom;
                }

                const auto* tn_layout = layout_engine->getLayout(tn);
                float text_left = 0.0f;
                float text_right = 0.0f;
                bool has_layout = (tn_layout && tn_layout->width > 0.0f);
                if (has_layout) {
                    text_left = tn_layout->x;
                    text_right = tn_layout->x + tn_layout->width;
                }
                float x_f = static_cast<float>(x);
                float x_penalty = 0.0f;
                bool is_after = false;
                if (has_layout) {
                    if (x_f < text_left) {
                        x_penalty = text_left - x_f;
                    } else if (x_f > text_right) {
                        x_penalty = x_f - text_right;
                        is_after = true;
                    }
                } else {
                    // Unknown layout: deprioritize against nodes with layout.
                    x_penalty = 1e6f;
                    is_after = true;
                }

                if (line_penalty < best_line_penalty ||
                    (line_penalty == best_line_penalty && x_penalty < best_x_penalty)) {
                    best_line_penalty = line_penalty;
                    best_x_penalty = x_penalty;
                    best_node = tn;
                    best_is_after = is_after;
                }
            }

            if (best_node) {
                TextHitResult result;
                result.text_node = best_node;
                result.found = true;
                // For clicks in CE blank area / between inline fragments, compute the
                // nearest insertion offset in the chosen line text instead of forcing
                // only begin/end. Keep end-snap when clearly after the node.
                if (best_is_after) {
                    result.char_offset = static_cast<uint32_t>(best_node->getRawTextContent().size());
                } else {
                    result.char_offset = estimateCharOffset(best_node, layout_engine, x, shaper);
                }
                if (debugCeHitTestEnabled()) {
                    DONG_LOG_WARN("[CE-HitTest][FALLBACK] x=%d y=%d line_pen=%.2f x_pen=%.2f after=%d off=%u text=\"%s\"",
                                  x, y, best_line_penalty, best_x_penalty, best_is_after ? 1 : 0,
                                  result.char_offset, best_node->getRawTextContent().c_str());
                }
                return result;
            }
        }
    }

    return {};
}

uint32_t TextHitTester::estimateCharOffset(const DOMNodePtr& text_node,
                                            dong::layout::Engine* layout_engine,
                                            int32_t x,
                                            dong::render::TextShaper* shaper) {
    if (!text_node) return 0;

    const std::string& text = text_node->getRawTextContent();
    if (text.empty()) return 0;

    auto parent = text_node->getParent();
    if (!parent) return 0;

    // Use text node's own layout if available (from IFC)
    const auto* text_layout = layout_engine->getLayout(text_node);
    float content_x;
    if (text_layout && text_layout->width > 0) {
        content_x = static_cast<float>(x) - text_layout->x;
        if (content_x < 0.0f) content_x = 0.0f;
        if (content_x >= text_layout->width - kCeCaretEndSnapPx)
            return static_cast<uint32_t>(text.size());
        content_x = std::max(0.0f, content_x - kCeCaretLeftBiasPx);
        if (content_x <= kCeCaretStartSnapPx) return 0;
    } else {
        const auto* layout = layout_engine->getLayout(parent);
        auto la = parent;
        while (!layout && la) {
            la = la->getParent();
            if (la) layout = layout_engine->getLayout(la);
        }
        if (!layout || layout->width <= 0) return 0;

        const auto& pstyle = parent->getComputedStyle();
        float pad_left = pstyle.padding_left.isPixel() ? pstyle.padding_left.value : 0.0f;
        float border_left = pstyle.border_left_width >= 0.0f ? pstyle.border_left_width : 0.0f;
        content_x = static_cast<float>(x) - layout->x - border_left - pad_left;
        if (content_x < 0.0f) content_x = 0.0f;
        content_x = std::max(0.0f, content_x - kCeCaretLeftBiasPx);
        if (content_x <= kCeCaretStartSnapPx) return 0;
    }

    const auto& style = parent->getComputedStyle();

    // Use text shaper for accurate character offset if available
    if (shaper) {
        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

        // Binary search: find the smallest prefix whose shaped width >= content_x
        uint32_t lo = 0;
        uint32_t hi = static_cast<uint32_t>(text.size());

        while (lo < hi) {
            uint32_t mid = lo + (hi - lo) / 2;
            // Snap mid to UTF-8 character boundary
            mid = utf8SnapBack(text, mid);
            if (mid <= lo && lo < hi) { mid = lo; }

            dong::render::TextShapeRequest req;
            req.text = text.substr(0, mid);
            req.font_family = style.font_family;
            req.font_size = font_size;
            req.font_weight = toString(style.font_weight);
            req.font_style = toString(style.font_style);

            dong::render::ShapedText shaped;
            if (shaper->shape(req, shaped) && shaped.units_per_em > 0) {
                float scale = font_size / static_cast<float>(shaped.units_per_em);
                float prefix_width = shaped.width_units * scale;

                if (prefix_width < content_x) {
                    lo = utf8NextChar(text, mid);
                } else {
                    hi = mid;
                }
            } else {
                // Shaping failed, fall back to proportional
                lo = utf8NextChar(text, mid);
            }
        }
        // Snap to nearest character boundary
        lo = utf8SnapBack(text, lo);
        if (lo > 0 && lo <= static_cast<uint32_t>(text.size())) {
            uint32_t prev_lo = utf8SnapBack(text, lo - 1);
            dong::render::TextShapeRequest req_prev;
            req_prev.text = text.substr(0, prev_lo);
            req_prev.font_family = style.font_family;
            req_prev.font_size = font_size;
            req_prev.font_weight = toString(style.font_weight);
            req_prev.font_style = toString(style.font_style);
            dong::render::ShapedText shaped_prev;
            if (shaper->shape(req_prev, shaped_prev) && shaped_prev.units_per_em > 0) {
                float scale_prev = font_size / static_cast<float>(shaped_prev.units_per_em);
                float left_edge = shaped_prev.width_units * scale_prev;
                dong::render::TextShapeRequest req_cur;
                req_cur.text = text.substr(0, lo);
                req_cur.font_family = style.font_family;
                req_cur.font_size = font_size;
                req_cur.font_weight = toString(style.font_weight);
                req_cur.font_style = toString(style.font_style);
                dong::render::ShapedText shaped_cur;
                if (shaper->shape(req_cur, shaped_cur) && shaped_cur.units_per_em > 0) {
                    float right_edge = shaped_cur.width_units * (font_size / static_cast<float>(shaped_cur.units_per_em));
                    float char_mid = (left_edge + right_edge) / 2.0f;
                    if (content_x < char_mid) {
                        lo = prev_lo;
                    }
                }
            }
        }
        return std::min(lo, static_cast<uint32_t>(text.size()));
    }

    // Fallback: proportional estimate (no shaper)
    float content_width;
    if (text_layout && text_layout->width > 0) {
        content_width = text_layout->width;
    } else {
        const auto* layout = layout_engine->getLayout(parent);
        auto la2 = parent;
        while (!layout && la2) {
            la2 = la2->getParent();
            if (la2) layout = layout_engine->getLayout(la2);
        }
        if (!layout) return 0;
        float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
        float border_left = style.border_left_width >= 0.0f ? style.border_left_width : 0.0f;
        float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
        float border_right = style.border_right_width >= 0.0f ? style.border_right_width : 0.0f;
        content_width = layout->width - border_left - pad_left - border_right - pad_right;
        if (content_width <= 0.0f) content_width = layout->width;
    }

    float ratio = std::clamp(content_x / content_width, 0.0f, 1.0f);
    uint32_t estimated = static_cast<uint32_t>(ratio * text.size());
    return std::min(estimated, static_cast<uint32_t>(text.size()));
}

} // namespace dong::dom
