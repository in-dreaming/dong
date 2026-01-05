#include "layout_engine.hpp"
#include <yoga/YGNode.h>
#include <yoga/YGConfig.h>
#include <yoga/Yoga.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cmath>
#include <SDL3/SDL_log.h>

#include "../render/text_shaper.hpp"

namespace dong::layout {

// DirtyRect implementation
void DirtyRect::expand(float px, float py, float pw, float ph) {
    if (pw <= 0 || ph <= 0) return;
    
    if (is_empty) {
        x = px;
        y = py;
        width = pw;
        height = ph;
        is_empty = false;
    } else {
        float new_x = std::min(x, px);
        float new_y = std::min(y, py);
        float new_right = std::max(x + width, px + pw);
        float new_bottom = std::max(y + height, py + ph);
        x = new_x;
        y = new_y;
        width = new_right - new_x;
        height = new_bottom - new_y;
    }
}

bool DirtyRect::intersects(float px, float py, float pw, float ph) const {
    if (is_empty || pw <= 0 || ph <= 0) return false;
    return !(px + pw <= x || px >= x + width ||
             py + ph <= y || py >= y + height);
}

namespace {

using dong::render::TextShaper;
using dong::render::TextShapeRequest;
using dong::render::ShapedText;

static std::string collapseWhitespace(const std::string& input) {
    if (input.empty()) return "";
    std::string output;
    output.reserve(input.size());
    bool in_space = false;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
        } else {
            output.push_back(c);
            in_space = false;
        }
    }
    size_t first = output.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = output.find_last_not_of(' ');
    return output.substr(first, last - first + 1);
}

float computeIntrinsicTextHeight(const dom::DOMNodePtr& node) {
    if (!node) return 0.0f;
    const auto& style = node->getComputedStyle();
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

    // 收集纯文本子节点（忽略嵌套元素）
    bool has_text_child = false;
    bool has_element_child = false;
    std::string raw_text;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            raw_text += child->getTextContent();
            has_text_child = true;
        } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            has_element_child = true;
        }
    }
    const std::string tag = node->getTagName();
    const bool tag_prefers_text =
        tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
        tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
        tag == "button" || tag == "code" || tag == "div" || tag == "footer";
    if (!has_text_child || (!tag_prefers_text && has_element_child)) {
        return 0.0f;
    }

    std::string text = collapseWhitespace(raw_text);
    if (text.empty()) return 0.0f;

    TextShaper shaper;
    TextShapeRequest req{};
    req.text = text;
    req.font_family = style.font_family;
    req.font_weight = style.font_weight;
    req.font_size = font_size;

    ShapedText shaped{};
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) {
        SDL_Log("[computeIntrinsicTextHeight] shaping failed for text='%s' font_size=%.1f", 
                text.c_str(), font_size);
        return 0.0f;
    }

    const float scale = shaped.scale_to_pixels;
    
    // Validate scale
    if (scale <= 0.0f || scale > 1.0f || !std::isfinite(scale)) {
        SDL_Log("[computeIntrinsicTextHeight] INVALID scale=%.6f for text='%s'", scale, text.substr(0, 20).c_str());
        return font_size * 1.2f;  // fallback
    }
    
    float effective_line_height = shaped.line_height_units * scale;
    
    // Validate line_height_units
    if (shaped.line_height_units <= 0.0f || shaped.line_height_units > 100000.0f || !std::isfinite(shaped.line_height_units)) {
        SDL_Log("[computeIntrinsicTextHeight] INVALID line_height_units=%.1f for text='%s'", 
                shaped.line_height_units, text.substr(0, 20).c_str());
        return font_size * 1.2f;  // fallback
    }
    
    // 对标浏览器对 `line-height: normal` 的近似：不少于 ~1.2 * font-size，
    // 避免大字号文本的行盒过小，导致后续块级元素"顶上来"覆盖文字。
    const float min_line_height = font_size * 1.2f;
    if (effective_line_height < min_line_height) {
        effective_line_height = min_line_height;
    }

    float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

    float result = effective_line_height + pad_top + pad_bottom;
    
    // Final validation
    if (result > 10000.0f || result < 0.0f || !std::isfinite(result)) {
        SDL_Log("[computeIntrinsicTextHeight] INVALID result=%.1f for text='%s' (effective_line_height=%.1f scale=%.6f line_height_units=%.1f)",
                result, text.substr(0, 20).c_str(), effective_line_height, scale, shaped.line_height_units);
        return font_size * 1.2f;  // fallback
    }
    
    return result;
}

// Collapse vertical margins between two sibling block-like elements so that
// the gap between them is closer to CSS's max(margin-bottom, margin-top)
// instead of a simple sum. This is a simplified approximation that focuses
// on pixel margins and common block layouts.
void collapseVerticalMarginBetweenSiblings(const dom::DOMNodePtr& prev_node,
                                           YGNode* prev_yoga,
                                           const dom::DOMNodePtr& curr_node,
                                           YGNode* curr_yoga) {
    if (!prev_node || !curr_node || !prev_yoga || !curr_yoga) {
        return;
    }

    const auto& prev_style = prev_node->getComputedStyle();
    const auto& curr_style = curr_node->getComputedStyle();

    // Do not attempt to collapse margins for flex items or non-visible blocks here.
    if (prev_style.layout_mode == dom::LayoutMode::Flex ||
        curr_style.layout_mode == dom::LayoutMode::Flex) {
        return;
    }

    float prev_mb = 0.0f;
    if (prev_style.margin_bottom.isPixel()) {
        prev_mb = prev_style.margin_bottom.value;
    }

    float curr_mt = 0.0f;
    if (curr_style.margin_top.isPixel()) {
        curr_mt = curr_style.margin_top.value;
    }

    if (prev_mb <= 0.0f || curr_mt <= 0.0f) {
        return;
    }

    // We want gap = prev_mb + adjusted_top ~= max(prev_mb, curr_mt).
    // Setting adjusted_top = max(curr_mt - prev_mb, 0) achieves that:
    // - If curr_mt >= prev_mb: gap = prev_mb + (curr_mt - prev_mb) = curr_mt
    // - If curr_mt <  prev_mb: gap = prev_mb
    float adjusted_top = curr_mt - prev_mb;
    if (adjusted_top < 0.0f) {
        adjusted_top = 0.0f;
    }

    YGNodeStyleSetMargin(curr_yoga, YGEdgeTop, adjusted_top);
}

dom::DOMNodePtr firstElementChild(const dom::DOMNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    for (const auto& child : node->getChildren()) {
        if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            return child;
        }
    }
    return nullptr;
}

struct InlineMetrics {
    float content_width_px = 0.0f;
    float line_height_px = 0.0f;
    float baseline_from_content_top_px = 0.0f;
};

static bool computeInlineMetricsForNode(const dom::DOMNodePtr& node,
                                        InlineMetrics& out_metrics,
                                        float fallback_font_size_px) {
    if (!node) {
        return false;
    }

    const auto& style = node->getComputedStyle();
    float font_size_px = style.font_size > 0.0f ? style.font_size : fallback_font_size_px;
    if (font_size_px <= 0.0f) {
        font_size_px = 16.0f;
    }

    // 收集该元素直接子节点中的文本，忽略嵌套元素，保持与 Painter 文本绘制一致
    bool has_text_child = false;
    bool has_element_child = false;
    std::string raw_text;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            raw_text += child->getTextContent();
            has_text_child = true;
        } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            has_element_child = true;
        }
    }

    const std::string tag = node->getTagName();
    const bool tag_prefers_text =
        tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
        tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
        tag == "button" || tag == "code" || tag == "div" || tag == "footer";

    if (!has_text_child || (!tag_prefers_text && has_element_child)) {
        return false;
    }

    std::string text = collapseWhitespace(raw_text);
    if (text.empty()) {
        return false;
    }

    TextShaper shaper;
    TextShapeRequest req{};
    req.text = text;
    req.font_family = style.font_family;
    req.font_weight = style.font_weight;
    req.font_size = font_size_px;

    ShapedText shaped{};
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) {
        return false;
    }

    const float scale = shaped.scale_to_pixels;

    // 宽度：整段文本的 glyph 范围
    float content_width_units = shaped.width_units;
    if (content_width_units < 0.0f) {
        content_width_units = 0.0f;
    }
    out_metrics.content_width_px = content_width_units * scale;

    // 行高与 baseline：复用 Painter 中的度量逻辑，保证一致
    float line_height_units = shaped.line_height_units;
    float ascent_units = shaped.ascent_units;
    float descent_units = shaped.descent_units;

    // 应用 CSS line-height 属性
    if (style.line_height > 0.0f) {
        if (style.line_height_is_unitless) {
            // 倍数：line-height * font-size
            float css_line_height_px = style.line_height * font_size_px;
            line_height_units = css_line_height_px / std::max(scale, 1e-3f);
        } else {
            // 像素值
            line_height_units = style.line_height / std::max(scale, 1e-3f);
        }
    }

    if (line_height_units <= 0.0f) {
        line_height_units = font_size_px / std::max(scale, 1e-3f);
    }
    // 对 `line-height: normal` 做一个浏览器级别的下限：不少于 1.2 * font-size，
    // 保证行盒高度足够容纳大字号 glyph，避免块级兄弟元素压住上一行文本。
    const float min_line_height_px = font_size_px * 1.2f;
    if (line_height_units * scale < min_line_height_px) {
        line_height_units = min_line_height_px / std::max(scale, 1e-3f);
    }
    if (ascent_units <= 0.0f) {
        ascent_units = font_size_px / std::max(scale, 1e-3f);
    }

    float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
    float metrics_height_units = ascent_units + descent_abs_units;
    if (metrics_height_units <= 0.0f) {
        metrics_height_units = line_height_units;
    }
    float extra_leading_units = line_height_units - metrics_height_units;
    if (extra_leading_units < 0.0f) {
        extra_leading_units = 0.0f;
    }
    float top_leading_units = extra_leading_units * 0.5f;

    out_metrics.line_height_px = line_height_units * scale;
    out_metrics.baseline_from_content_top_px = (top_leading_units + ascent_units) * scale;
    return true;
}

bool isInlineLevelDisplay(const std::string& display) {
    return display == "inline" || display == "inline-block";
}

bool isInlineFormattingContext(const dom::DOMNodePtr& node) {
    if (!node || node->getType() != dom::DOMNode::NodeType::ELEMENT) {
        return false;
    }

    const auto& style = node->getComputedStyle();

    // 不在根级大容器上启用内联格式化，避免一次性重写太多布局
    const std::string tag = node->getTagName();
    if (tag == "html" || tag == "body") {
        return false;
    }

    if (style.display == "none" || style.display == "flex") {
        return false;
    }

    bool has_inline_child = false;
    bool has_block_like_child = false;

    for (const auto& child : node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
            continue;
        }
        const auto& cs = child->getComputedStyle();
        
        if (isInlineLevelDisplay(cs.display)) {
            has_inline_child = true;
        } else if (cs.display == "block" || cs.display == "flex" || cs.display == "none") {
            has_block_like_child = true;
        }
    }

    // 仅当存在 inline/inline-block 子元素，且没有混入其他 block/flex 子元素时，
    // 将该容器视为内联格式化上下文。这样可以安全地覆盖 align_basic_layout 等场景，
    // 又不影响通用 block 布局。
    return has_inline_child && !has_block_like_child;
}

} // anonymous namespace

LayoutNode::~LayoutNode() {
    if (yoga_node) {
        YGNodeFree(yoga_node);
    }
}

Engine::Engine() {
    yoga_config = YGConfigNew();
    if (!yoga_config) {
        std::cerr << "Failed to create Yoga config\n";
    }
}

Engine::~Engine() {
    layout_cache.clear();
    if (yoga_config) {
        YGConfigFree(yoga_config);
    }
}

void Engine::calculateLayout(dom::DOMNodePtr root, float width, float height) {
    if (!root || !yoga_config) return;

    // Reset dirty rect for this layout pass
    dirty_rect_ = DirtyRect();

    // Check if any node is dirty; if not, skip layout entirely
    std::function<bool(const dom::DOMNodePtr&)> hasAnyDirtyNode =
        [&hasAnyDirtyNode](const dom::DOMNodePtr& node) -> bool {
            if (!node) return false;
            if (node->isLayoutDirty()) return true;
            for (const auto& child : node->getChildren()) {
                if (hasAnyDirtyNode(child)) return true;
            }
            return false;
        };

    if (!hasAnyDirtyNode(root)) {
        // Everything is clean, no need to relayout
        return;
    }

    // Rebuild layout tree from scratch each time for now to avoid stale Yoga nodes
    layout_cache.clear();

    // Prefer the first real element (e.g., <html>) as the Yoga root instead of the #document node
    dom::DOMNodePtr layout_root_dom = root;
    if (root->getType() != dom::DOMNode::NodeType::ELEMENT) {
        auto element_child = firstElementChild(root);
        if (element_child) {
            layout_root_dom = element_child;
        }
    }

    if (!layout_root_dom) return;

    // Create Yoga tree from the chosen DOM root
    YGNode* yoga_root = createYogaNode(layout_root_dom);
    if (!yoga_root) return;

    // Set viewport size on Yoga root (now the <html> element in most cases)
    YGNodeStyleSetWidth(yoga_root, width);
    YGNodeStyleSetHeight(yoga_root, height);


    // Calculate layout
    YGNodeCalculateLayout(yoga_root, width, height, YGDirectionLTR);

    // Extract layout info back to cache (recursively). Convert Yoga's relative
    // positions into absolute coordinates so the painter can use them directly.
    std::function<void(dom::DOMNodePtr, YGNode*, float, float)> extractLayoutRecursive =
        [this, &extractLayoutRecursive](dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                        float parent_x, float parent_y) {
            if (!dom_node || !yoga_node) return;

            auto& node_layout = layout_cache[dom_node.get()];
            bool is_new_layout = false;
            if (!node_layout) {
                node_layout = std::make_unique<LayoutNode>();
                is_new_layout = true;
            }
            node_layout->yoga_node = yoga_node;

            const float left = YGNodeLayoutGetLeft(yoga_node);
            const float top = YGNodeLayoutGetTop(yoga_node);
            const float width = YGNodeLayoutGetWidth(yoga_node);
            const float height = YGNodeLayoutGetHeight(yoga_node);

            float new_x = parent_x + left;
            float new_y = parent_y + top;
            float new_width = width;
            float new_height = height;

            // Check if layout changed: position or size differs
            bool layout_changed = is_new_layout || 
                                  node_layout->x != new_x ||
                                  node_layout->y != new_y ||
                                  node_layout->width != new_width ||
                                  node_layout->height != new_height;

            node_layout->x = new_x;
            node_layout->y = new_y;
            node_layout->width = new_width;
            node_layout->height = new_height;
            node_layout->layout_recalculated = true;

            // Keep legacy fields and new layout struct in sync (absolute coords)
            node_layout->layout.position[0] = node_layout->x;
            node_layout->layout.position[1] = node_layout->y;
            node_layout->layout.dimensions[0] = node_layout->width;
            node_layout->layout.dimensions[1] = node_layout->height;

            // If this node's layout changed, add its area to dirty rect
            if (layout_changed) {
                dirty_rect_.expand(new_x, new_y, new_width, new_height);
            }

            // Recursively extract child layouts. Only ELEMENT nodes get Yoga children.
            uint32_t child_count = YGNodeGetChildCount(yoga_node);
            uint32_t yoga_child_index = 0;
            for (const auto& child_dom : dom_node->getChildren()) {
                if (!child_dom || child_dom->getType() != dom::DOMNode::NodeType::ELEMENT) {
                    continue;
                }
                if (yoga_child_index >= child_count) {
                    break;
                }
                YGNode* child_yoga = YGNodeGetChild(yoga_node, yoga_child_index);
                if (child_yoga) {
                    extractLayoutRecursive(child_dom, child_yoga,
                                            node_layout->x, node_layout->y);
                }
                ++yoga_child_index;
            }
        };

    extractLayoutRecursive(layout_root_dom, yoga_root, 0.0f, 0.0f);

    // After Yoga layout, apply Block Formatting Context adjustments:
    // - margin: auto horizontal centering
    // - margin-left: auto / margin-right: auto alignment
    layoutBlockFormattingContext(layout_root_dom);

    // After Yoga layout, run inline formatting context layout to adjust
    // inline/inline-block children inside suitable containers.
    layoutInlineFormattingContexts(layout_root_dom);

    // Then layout positioned elements (position:absolute) relative to their containing blocks.
    layoutPositionedElements(layout_root_dom);

    // Ensure the #document node itself still has a layout entry so rendering can start from it
    if (layout_root_dom.get() != root.get()) {
        auto& doc_layout = layout_cache[root.get()];
        if (!doc_layout) {
            doc_layout = std::make_unique<LayoutNode>();
        }
        doc_layout->x = 0.0f;
        doc_layout->y = 0.0f;
        doc_layout->width = width;
        doc_layout->height = height;
        doc_layout->layout.position[0] = 0.0f;
        doc_layout->layout.position[1] = 0.0f;
        doc_layout->layout.dimensions[0] = width;
        doc_layout->layout.dimensions[1] = height;
        doc_layout->yoga_node = nullptr; // #document is synthetic; it doesn't have a corresponding Yoga node
    }
}

const LayoutNode* Engine::getLayout(dom::DOMNodePtr node) const {
    if (!node) return nullptr;
    
    auto it = layout_cache.find(node.get());
    if (it != layout_cache.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Engine::markDirty(dom::DOMNodePtr node) {
    if (!node) return;

    // Find corresponding Yoga node and mark dirty
    auto layout = getLayout(node);
    if (layout && layout->yoga_node) {
        YGNodeMarkDirty(layout->yoga_node);
    }
}

YGNode* Engine::createYogaNode(dom::DOMNodePtr dom_node) {
    if (!dom_node || !yoga_config) return nullptr;

    // Reuse existing Yoga node for this DOM node if it already exists in the cache,
    // otherwise create a new one. This avoids reallocating the Yoga tree on every
    // layout pass and lets us rely on Yoga's own dirty-marking for subtrees.
    auto& node_layout = layout_cache[dom_node.get()];
    if (!node_layout) {
        node_layout = std::make_unique<LayoutNode>();
    }

    YGNode* yoga_node = node_layout->yoga_node;
    if (!yoga_node) {
        yoga_node = YGNodeNewWithConfig(yoga_config);
        if (!yoga_node) return nullptr;
        node_layout->yoga_node = yoga_node;
    } else {
        // Clear existing children so we can rebuild the hierarchy to match DOM
        const uint32_t child_count = YGNodeGetChildCount(yoga_node);
        for (uint32_t i = 0; i < child_count; ++i) {
            YGNodeRemoveChild(yoga_node, YGNodeGetChild(yoga_node, 0));
        }
    }

    // Apply DOM styles to Yoga node
    applyDOMStylesToYoga(dom_node, yoga_node);

    // Recursively create or reuse Yoga nodes for children
    const auto& parent_style = dom_node->getComputedStyle();
    const bool parent_is_block_like = (parent_style.layout_mode == dom::LayoutMode::Block);

    dom::DOMNodePtr prev_element_child;
    YGNode* prev_child_yoga = nullptr;

    for (const auto& child : dom_node->getChildren()) {
        if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            YGNode* child_yoga = createYogaNode(child);
            if (child_yoga) {
                // Approximate vertical margin collapsing between sibling block items
                // for non-flex parents, so that gaps match CSS block layout better.
                if (parent_is_block_like && prev_child_yoga) {
                    collapseVerticalMarginBetweenSiblings(prev_element_child, prev_child_yoga,
                                                          child, child_yoga);
                }

                YGNodeInsertChild(yoga_node, child_yoga, YGNodeGetChildCount(yoga_node));

                auto& child_layout = layout_cache[child.get()];
                if (!child_layout) {
                    child_layout = std::make_unique<LayoutNode>();
                }
                child_layout->yoga_node = child_yoga;

                prev_element_child = child;
                prev_child_yoga = child_yoga;
            }
        }
    }

    return yoga_node;
}

void Engine::applyDOMStylesToYoga(dom::DOMNodePtr dom_node, YGNode* yoga_node) {
    if (!dom_node || !yoga_node) return;


    const auto& style = dom_node->getComputedStyle();

    const std::string& tag = dom_node->getTagName();

    mapComputedStylesToYoga(style, yoga_node);

    // 如果当前节点是内联格式化上下文（包含 inline/inline-block 子元素），
    // 设置为 flex-direction: row，让 Yoga 能正确计算容器高度
    if (isInlineFormattingContext(dom_node)) {
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRow);
        YGNodeStyleSetFlexWrap(yoga_node, YGWrapWrap); // 允许换行
    }

    bool width_converted_to_max = false;
    float converted_width_value = 0.0f;
    if (style.width.isPercent() && style.width.value >= 100.0f &&
        style.max_width.isPixel() && style.max_width.value > 0.0f) {
        width_converted_to_max = true;
        converted_width_value = style.max_width.value;
        YGNodeStyleSetWidth(yoga_node, converted_width_value);
    }

    // If parent is a row-direction flex container and this node has an explicit width
    // but no custom flex-basis, propagate that width as the flex-basis so Yoga treats it
    // as the main-axis base size. Otherwise, percent widths on flex items shrink to content.
    bool parent_row_flex = false;
    if (auto parent = dom_node->getParent()) {
        if (parent->getType() == dom::DOMNode::NodeType::ELEMENT) {
            const auto& parent_style = parent->getComputedStyle();
            if (parent_style.layout_mode == dom::LayoutMode::Flex) {
                std::string dir = parent_style.flex_direction;
                if (dir.empty()) dir = "row";
                if (dir == "row" || dir == "row-reverse") {
                    parent_row_flex = true;
                }
            }
        }
    }

    bool has_explicit_width = style.width.isPixel() || style.width.isPercent() || width_converted_to_max;
    bool has_custom_flex_basis = style.flex_basis.unit != dom::CSSValue::Unit::AUTO;
    if (parent_row_flex && has_explicit_width && !has_custom_flex_basis) {
        if (width_converted_to_max) {
            YGNodeStyleSetFlexBasis(yoga_node, converted_width_value);
        } else if (style.width.isPixel()) {
            YGNodeStyleSetFlexBasis(yoga_node, style.width.value);
        } else if (style.width.isPercent()) {
            YGNodeStyleSetFlexBasisPercent(yoga_node, style.width.value);
        }
    }

    // 基于真实字体度量的 intrinsic sizing：
    // 如果是文本主导的块级元素，且高度为 auto，则使用 TextShaper
    // 计算一行文字的行高，并将 padding 一起纳入最小高度，避免文本容器高度
    // 仅由 padding 决定，导致与浏览器差异过大。
    if ((tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
         tag == "h5" || tag == "h6" || tag == "p" || tag == "button" ||
         tag == "div") &&
        style.height.isAuto()) {
        float intrinsic_h = computeIntrinsicTextHeight(dom_node);
        if (intrinsic_h > 0.0f && intrinsic_h < 10000.0f) {
            YGNodeStyleSetMinHeight(yoga_node, intrinsic_h);
            // Also set the height explicitly to avoid Yoga calculating a huge value
            YGNodeStyleSetHeight(yoga_node, intrinsic_h);
        }
    }
}

void Engine::mapComputedStylesToYoga(const dom::ComputedStyle& style, YGNode* yoga_node) {
    if (!yoga_node) return;

    // Set display based on layout mode
    switch (style.layout_mode) {
        case dom::LayoutMode::None:
            YGNodeStyleSetDisplay(yoga_node, YGDisplayNone);
            break;
        case dom::LayoutMode::Block:
        case dom::LayoutMode::Inline:
        case dom::LayoutMode::Flex:
        default:
            YGNodeStyleSetDisplay(yoga_node, YGDisplayFlex);
            break;
    }

    // Set flex direction
    // For flex layout, respect flex_direction; for block/inline, approximate as vertical stack
    if (style.layout_mode == dom::LayoutMode::Flex) {
        if (style.flex_direction == "column") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumn);
        } else if (style.flex_direction == "column-reverse") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumnReverse);
        } else if (style.flex_direction == "row-reverse") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRowReverse);
        } else {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRow);
        }
    } else {
        // Approximate block/inline layout as a vertical stack
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumn);
        
        // 但如果是 inline-block 元素，设置为 row 方向，让 Yoga 能正确计算其尺寸
        if (style.display == "inline-block") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRow);
        }
    }

    // Set justify content
    if (style.justify_content == "flex-end") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifyFlexEnd);
    } else if (style.justify_content == "center") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifyCenter);
    } else if (style.justify_content == "space-between") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifySpaceBetween);
    } else if (style.justify_content == "space-around") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifySpaceAround);
    } else {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifyFlexStart);
    }

    // Set align items
    if (style.align_items == "flex-end") {
        YGNodeStyleSetAlignItems(yoga_node, YGAlignFlexEnd);
    } else if (style.align_items == "center") {
        YGNodeStyleSetAlignItems(yoga_node, YGAlignCenter);
    } else if (style.align_items == "stretch") {
        YGNodeStyleSetAlignItems(yoga_node, YGAlignStretch);
    } else {
        // Default to flex-start, not stretch
        YGNodeStyleSetAlignItems(yoga_node, YGAlignFlexStart);
    }

    // Set gap for flex containers
    if (style.gap > 0.0f) {
        YGNodeStyleSetGap(yoga_node, YGGutterAll, style.gap);
    }

    // Set position type and offsets (relative/absolute)
    if (style.position == "absolute") {
        YGNodeStyleSetPositionType(yoga_node, YGPositionTypeAbsolute);
    } else {
        // static/relative/fixed 目前统一映射为 Yoga 的 relative，
        // 但通过位置属性（top/right/bottom/left）实现 relative 偏移。
        YGNodeStyleSetPositionType(yoga_node, YGPositionTypeRelative);
    }

    auto setPositionIfNeeded = [&](YGEdge edge, const dom::CSSValue& v) {
        using Unit = dom::CSSValue::Unit;
        if (v.unit == Unit::PIXEL) {
            YGNodeStyleSetPosition(yoga_node, edge, v.value);
        } else if (v.unit == Unit::PERCENT) {
            YGNodeStyleSetPositionPercent(yoga_node, edge, v.value);
        }
    };

    setPositionIfNeeded(YGEdgeTop, style.top);
    setPositionIfNeeded(YGEdgeRight, style.right);
    setPositionIfNeeded(YGEdgeBottom, style.bottom);
    setPositionIfNeeded(YGEdgeLeft, style.left);

    // Set dimensions
    // Note: Yoga uses border-box model internally. When CSS box-sizing is content-box,
    // we need to add padding and border to the specified width/height.
    bool has_explicit_width = false;
    bool has_explicit_height = false;

    // Calculate padding and border for content-box adjustment
    float pad_h = 0.0f; // horizontal padding (left + right)
    float pad_v = 0.0f; // vertical padding (top + bottom)
    float border_h = 0.0f; // horizontal border (left + right)
    float border_v = 0.0f; // vertical border (top + bottom)
    
    if (style.box_sizing == "content-box") {
        if (style.padding_left.isPixel()) pad_h += style.padding_left.value;
        if (style.padding_right.isPixel()) pad_h += style.padding_right.value;
        if (style.padding_top.isPixel()) pad_v += style.padding_top.value;
        if (style.padding_bottom.isPixel()) pad_v += style.padding_bottom.value;
        border_h = style.border_width * 2.0f;
        border_v = style.border_width * 2.0f;
    }

    if (style.width.isPixel()) {
        float width_for_yoga = style.width.value;
        if (style.box_sizing == "content-box") {
            // content-box: CSS width is content width, add padding and border for Yoga
            width_for_yoga += pad_h + border_h;
        }
        YGNodeStyleSetWidth(yoga_node, width_for_yoga);
        has_explicit_width = true;
    } else if (style.width.isPercent()) {
        YGNodeStyleSetWidthPercent(yoga_node, style.width.value);
        has_explicit_width = true;
    }

    if (style.height.isPixel()) {
        float height_for_yoga = style.height.value;
        if (style.box_sizing == "content-box") {
            // content-box: CSS height is content height, add padding and border for Yoga
            height_for_yoga += pad_v + border_v;
        }
        YGNodeStyleSetHeight(yoga_node, height_for_yoga);
        has_explicit_height = true;
    } else if (style.height.isPercent()) {
        YGNodeStyleSetHeightPercent(yoga_node, style.height.value);
        has_explicit_height = true;
    }

    // Fallback: for block-level elements without an explicit width,
    // stretch to full available width. This approximates HTML block layout
    // when we don't have intrinsic content measurement.
    //
    // IMPORTANT: Do not apply this fallback to:
    // - position:absolute elements (should use intrinsic/shrink-to-fit sizing)
    // - inline-block elements (should size to content, not stretch to 100%)
    // Otherwise Yoga will give them a 100% width box, which breaks the
    // expected CSS semantics.
    const bool is_inline_block = (style.display == "inline-block");
    if (!has_explicit_width &&
        style.layout_mode == dom::LayoutMode::Block &&
        style.position != "absolute" &&
        !is_inline_block) {
        YGNodeStyleSetWidthPercent(yoga_node, 100.0f);
    }

    // Set margins
    // Note: margin: auto is handled differently depending on context:
    // - In flex containers: Yoga's YGNodeStyleSetMarginAuto handles it correctly
    // - In block formatting context: layoutBlockFormattingContext() handles it after Yoga layout
    if (style.margin_top.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeTop, style.margin_top.value);
    } else if (style.margin_top.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeTop, style.margin_top.value);
    } else if (style.margin_top.isAuto()) {
        // margin-top: auto - in flex context, this enables vertical centering
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeTop);
    }

    if (style.margin_right.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeRight, style.margin_right.value);
    } else if (style.margin_right.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeRight, style.margin_right.value);
    } else if (style.margin_right.isAuto()) {
        // margin-right: auto - in flex context, pushes element to the left
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeRight);
    }

    if (style.margin_bottom.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeBottom, style.margin_bottom.value);
    } else if (style.margin_bottom.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeBottom, style.margin_bottom.value);
    } else if (style.margin_bottom.isAuto()) {
        // margin-bottom: auto - in flex context, this enables vertical centering
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeBottom);
    }

    if (style.margin_left.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeLeft, style.margin_left.value);
    } else if (style.margin_left.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeLeft, style.margin_left.value);
    } else if (style.margin_left.isAuto()) {
        // margin-left: auto - in flex context, pushes element to the right
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeLeft);
    }

    // Set padding
    if (style.padding_top.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeTop, style.padding_top.value);
    } else if (style.padding_top.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeTop, style.padding_top.value);
    }
    if (style.padding_right.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeRight, style.padding_right.value);
    } else if (style.padding_right.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeRight, style.padding_right.value);
    }
    if (style.padding_bottom.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeBottom, style.padding_bottom.value);
    } else if (style.padding_bottom.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeBottom, style.padding_bottom.value);
    }
    if (style.padding_left.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeLeft, style.padding_left.value);
    } else if (style.padding_left.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeLeft, style.padding_left.value);
    }

    // Set flex grow/shrink
    float flex_grow = style.flex_grow;
    float flex_shrink = style.flex_shrink;
    dom::CSSValue flex_basis = style.flex_basis;

    // Support `flex: <number>` shorthand as flex-grow with a 0 basis
    if (style.flex != 0.0f) {
        if (flex_grow == 0.0f) {
            flex_grow = style.flex;
        }
        // Keep existing shrink unless explicitly overridden
        if (flex_shrink == 1.0f) {
            flex_shrink = 1.0f;
        }
        // When flex shorthand is used and flex-basis is auto, default to 0
        if (flex_basis.unit == dom::CSSValue::Unit::AUTO) {
            flex_basis = dom::CSSValue(0.0f, dom::CSSValue::Unit::PIXEL);
        }
    }

    YGNodeStyleSetFlexGrow(yoga_node, flex_grow);
    YGNodeStyleSetFlexShrink(yoga_node, flex_shrink);
    if (flex_basis.isPixel()) {
        YGNodeStyleSetFlexBasis(yoga_node, flex_basis.value);
    } else if (flex_basis.isPercent()) {
        YGNodeStyleSetFlexBasisPercent(yoga_node, flex_basis.value);
    } else {
        // For auto flex-basis in block layout, set to 0 to prevent Yoga from
        // using content-based sizing which can cause incorrect height calculations
        if (style.layout_mode == dom::LayoutMode::Block) {
            YGNodeStyleSetFlexBasis(yoga_node, 0.0f);
        } else {
            YGNodeStyleSetFlexBasisAuto(yoga_node);
        }
    }

    // Map border width into Yoga so that border participates in box model sizing
    if (style.border_width > 0.0f) {
        YGNodeStyleSetBorder(yoga_node, YGEdgeLeft, style.border_width);
        YGNodeStyleSetBorder(yoga_node, YGEdgeTop, style.border_width);
        YGNodeStyleSetBorder(yoga_node, YGEdgeRight, style.border_width);
        YGNodeStyleSetBorder(yoga_node, YGEdgeBottom, style.border_width);
    }

    // Set min/max width and height
    // Note: min/max constraints also need box-sizing adjustment for content-box
    if (style.min_width.isPixel()) {
        float min_w = style.min_width.value;
        if (style.box_sizing == "content-box") {
            min_w += pad_h + border_h;
        }
        YGNodeStyleSetMinWidth(yoga_node, min_w);
    } else if (style.min_width.isPercent()) {
        YGNodeStyleSetMinWidthPercent(yoga_node, style.min_width.value);
    }
    if (style.max_width.isPixel()) {
        float max_w = style.max_width.value;
        if (style.box_sizing == "content-box") {
            max_w += pad_h + border_h;
        }
        YGNodeStyleSetMaxWidth(yoga_node, max_w);
    } else if (style.max_width.isPercent()) {
        YGNodeStyleSetMaxWidthPercent(yoga_node, style.max_width.value);
    }
    if (style.min_height.isPixel()) {
        float min_h = style.min_height.value;
        if (style.box_sizing == "content-box") {
            min_h += pad_v + border_v;
        }
        YGNodeStyleSetMinHeight(yoga_node, min_h);
    } else if (style.min_height.isPercent()) {
        YGNodeStyleSetMinHeightPercent(yoga_node, style.min_height.value);
    }
    if (style.max_height.isPixel()) {
        float max_h = style.max_height.value;
        if (style.box_sizing == "content-box") {
            max_h += pad_v + border_v;
        }
        YGNodeStyleSetMaxHeight(yoga_node, max_h);
    } else if (style.max_height.isPercent()) {
        YGNodeStyleSetMaxHeightPercent(yoga_node, style.max_height.value);
    }
}

float Engine::parsePixelValue(const dom::CSSValue& value, float parent_size) {
    switch (value.unit) {
        case dom::CSSValue::Unit::PIXEL:
            return value.value;
        case dom::CSSValue::Unit::PERCENT:
            // If given as percent but we need pixels, use 0 as fallback
            // (caller should use parsePercentValue for percentages)
            return 0.0f;
        case dom::CSSValue::Unit::AUTO:
        case dom::CSSValue::Unit::INHERIT:
            return 0.0f;
        default:
            return 0.0f;
    }
}

float Engine::parsePercentValue(const dom::CSSValue& value, float parent_size) {
    if (value.unit == dom::CSSValue::Unit::PERCENT && parent_size > 0) {
        return (value.value / 100.0f) * parent_size;
    }
    return 0.0f;
}

void Engine::destroyYogaNode(YGNode* node) {
    if (node) {
        YGNodeFree(node);
    }
}

// ============================================================================
// Block Formatting Context: margin: auto handling
// ============================================================================
//
// CSS 2.1 §10.3.3: Block-level, non-replaced elements in normal flow
//
// The constraint equation for horizontal layout:
//   margin-left + border-left + padding-left + width + padding-right + border-right + margin-right = containing block width
//
// When margin-left and margin-right are both 'auto':
//   - If width is not 'auto': margins are equal, centering the element
//   - If width is 'auto': margins compute to 0
//
// When only margin-left is 'auto':
//   - Element is right-aligned within containing block
//
// When only margin-right is 'auto':
//   - Element is left-aligned (default behavior, no adjustment needed)
//
// Note: margin-top: auto and margin-bottom: auto compute to 0 in block formatting context
// (they only have special meaning in flex/grid contexts)
//
void Engine::layoutBlockFormattingContext(dom::DOMNodePtr root) {
    if (!root) {
        return;
    }

    std::function<void(const dom::DOMNodePtr&, const dom::DOMNodePtr&)> walk;
    walk = [this, &walk](const dom::DOMNodePtr& node, const dom::DOMNodePtr& parent) {
        if (!node) {
            return;
        }

        const auto& style = node->getComputedStyle();

        // Only process block-level elements in normal flow
        // Skip: display:none, position:absolute/fixed, inline/inline-block, flex items
        const bool is_block_in_flow =
            style.layout_mode == dom::LayoutMode::Block &&
            style.position != "absolute" &&
            style.position != "fixed" &&
            style.display != "inline" &&
            style.display != "inline-block";

        if (is_block_in_flow && parent) {
            const bool margin_left_auto = style.margin_left.isAuto();
            const bool margin_right_auto = style.margin_right.isAuto();

            // Check if parent is a flex container - margin:auto has different semantics in flex
            const auto& parent_style = parent->getComputedStyle();
            const bool parent_is_flex = (parent_style.layout_mode == dom::LayoutMode::Flex);

            // Only apply block formatting context margin:auto if parent is NOT flex
            // (Yoga handles margin:auto correctly for flex items)
            if (!parent_is_flex && (margin_left_auto || margin_right_auto)) {
                auto it_layout = layout_cache.find(node.get());
                auto it_parent_layout = layout_cache.find(parent.get());

                if (it_layout != layout_cache.end() && it_layout->second &&
                    it_parent_layout != layout_cache.end() && it_parent_layout->second) {
                    
                    LayoutNode* layout = it_layout->second.get();
                    LayoutNode* parent_layout = it_parent_layout->second.get();

                    // Get containing block's content width (parent width minus its padding)
                    float parent_pad_left = parent_style.padding_left.isPixel() ? parent_style.padding_left.value : 0.0f;
                    float parent_pad_right = parent_style.padding_right.isPixel() ? parent_style.padding_right.value : 0.0f;
                    float containing_block_width = parent_layout->width - parent_pad_left - parent_pad_right;

                    // Element's box width (already computed by Yoga, includes border/padding if box-sizing: border-box)
                    float element_width = layout->width;

                    // Get explicit margin values (non-auto margins)
                    float margin_left_px = 0.0f;
                    float margin_right_px = 0.0f;
                    if (!margin_left_auto && style.margin_left.isPixel()) {
                        margin_left_px = style.margin_left.value;
                    } else if (!margin_left_auto && style.margin_left.isPercent()) {
                        margin_left_px = parsePercentValue(style.margin_left, containing_block_width);
                    }
                    if (!margin_right_auto && style.margin_right.isPixel()) {
                        margin_right_px = style.margin_right.value;
                    } else if (!margin_right_auto && style.margin_right.isPercent()) {
                        margin_right_px = parsePercentValue(style.margin_right, containing_block_width);
                    }

                    // Calculate remaining space
                    float remaining_space = containing_block_width - element_width - margin_left_px - margin_right_px;
                    if (remaining_space < 0.0f) {
                        remaining_space = 0.0f;
                    }

                    // Calculate new X position based on margin:auto rules
                    float new_x = layout->x;
                    float content_start_x = parent_layout->x + parent_pad_left;

                    if (margin_left_auto && margin_right_auto) {
                        // Both auto: center the element
                        // margin-left = margin-right = remaining_space / 2
                        float auto_margin = remaining_space / 2.0f;
                        new_x = content_start_x + auto_margin;
                    } else if (margin_left_auto && !margin_right_auto) {
                        // Only margin-left is auto: right-align
                        // margin-left = remaining_space
                        new_x = content_start_x + remaining_space;
                    } else if (!margin_left_auto && margin_right_auto) {
                        // Only margin-right is auto: left-align (default)
                        // margin-right absorbs remaining space, element stays at left
                        new_x = content_start_x + margin_left_px;
                    }

                    // Apply the new position if changed
                    if (layout->x != new_x) {
                        float old_x = layout->x;
                        layout->x = new_x;
                        layout->layout.position[0] = new_x;

                        // Mark dirty region
                        dirty_rect_.expand(old_x, layout->y, layout->width, layout->height);
                        dirty_rect_.expand(new_x, layout->y, layout->width, layout->height);

                        // Recursively update all descendant positions
                        // Since we changed this node's X, all children need their absolute X updated
                        float delta_x = new_x - old_x;
                        std::function<void(const dom::DOMNodePtr&)> updateChildPositions;
                        updateChildPositions = [this, delta_x, &updateChildPositions](const dom::DOMNodePtr& child_node) {
                            if (!child_node) return;
                            for (const auto& grandchild : child_node->getChildren()) {
                                if (!grandchild || grandchild->getType() != dom::DOMNode::NodeType::ELEMENT) {
                                    continue;
                                }
                                auto it = layout_cache.find(grandchild.get());
                                if (it != layout_cache.end() && it->second) {
                                    LayoutNode* child_layout = it->second.get();
                                    child_layout->x += delta_x;
                                    child_layout->layout.position[0] = child_layout->x;
                                    dirty_rect_.expand(child_layout->x, child_layout->y,
                                                       child_layout->width, child_layout->height);
                                }
                                updateChildPositions(grandchild);
                            }
                        };
                        updateChildPositions(node);
                    }
                }
            }
        }

        // Recursively process children
        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                walk(child, node);
            }
        }
    };

    // Start from root with no parent
    walk(root, nullptr);
}

void Engine::layoutInlineFormattingContexts(dom::DOMNodePtr root) {
    if (!root) {
        return;
    }

    std::function<void(const dom::DOMNodePtr&)> walk;
    walk = [this, &walk](const dom::DOMNodePtr& node) {
        if (!node) {
            return;
        }

        if (isInlineFormattingContext(node)) {
            auto it_container = layout_cache.find(node.get());
            if (it_container != layout_cache.end() && it_container->second) {
                LayoutNode* container_layout = it_container->second.get();
                const auto& container_style = node->getComputedStyle();

                float container_x = container_layout->x;
                float container_y = container_layout->y;
                float container_w = container_layout->width;

                float pad_left = container_style.padding_left.isPixel() ? container_style.padding_left.value : 0.0f;
                float pad_right = container_style.padding_right.isPixel() ? container_style.padding_right.value : 0.0f;
                float pad_top = container_style.padding_top.isPixel() ? container_style.padding_top.value : 0.0f;

                float content_x = container_x + pad_left;
                float content_y = container_y + pad_top;
                float content_w = container_w - pad_left - pad_right;
                if (content_w <= 0.0f) {
                    content_w = container_w;
                }

                InlineMetrics container_metrics{};
                bool has_container_text_metrics = computeInlineMetricsForNode(node, container_metrics, container_style.font_size);
                float container_baseline_from_border_top = 0.0f;
                float container_line_height_px = 0.0f;
                if (has_container_text_metrics) {
                    float border_w_container = container_style.border_width;
                    if (border_w_container < 0.0f) {
                        border_w_container = 0.0f;
                    }
                    container_line_height_px = container_metrics.line_height_px;
                    container_baseline_from_border_top =
                        border_w_container +
                        (container_style.padding_top.isPixel() ? container_style.padding_top.value : 0.0f) +
                        container_metrics.baseline_from_content_top_px;
                }

                struct InlineItem {
                    dom::DOMNodePtr node;
                    float margin_left = 0.0f;
                    float margin_right = 0.0f;
                    float preferred_width = 0.0f;
                    float preferred_height = 0.0f;
                    float baseline_from_border_top = 0.0f;
                    float line_height_px = 0.0f;
                    float offset_x_in_content = 0.0f;
                    std::string vertical_align = "baseline"; // baseline, top, middle, bottom
                };

                std::vector<InlineItem> items;
                items.reserve(node->getChildren().size());

                for (const auto& child : node->getChildren()) {
                    if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                        continue;
                    }

                    const auto& child_style = child->getComputedStyle();
                    
                    if (child_style.position == "absolute") {
                        // 绝对定位元素不参与内联格式化上下文，由专门的定位布局阶段处理
                        continue;
                    }
                    if (!isInlineLevelDisplay(child_style.display)) {
                        continue;
                    }

                    InlineMetrics metrics{};
                    bool has_text_metrics = computeInlineMetricsForNode(child, metrics, container_style.font_size);

                    // 即使没有文本度量，只要有显式 width/height，也应该参与布局
                    bool has_explicit_width = child_style.width.isPixel() || child_style.width.isPercent();
                    bool has_explicit_height = child_style.height.isPixel() || child_style.height.isPercent();
                    
                    if (!has_text_metrics && !has_explicit_width && !has_explicit_height) {
                        // 既没有文本内容，也没有显式尺寸，跳过
                        continue;
                    }

                    InlineItem item{};
                    item.node = child;

                    item.margin_left = parsePixelValue(child_style.margin_left, content_w);
                    item.margin_right = parsePixelValue(child_style.margin_right, content_w);

                    float pad_l = child_style.padding_left.isPixel() ? child_style.padding_left.value : 0.0f;
                    float pad_r = child_style.padding_right.isPixel() ? child_style.padding_right.value : 0.0f;
                    float pad_t = child_style.padding_top.isPixel() ? child_style.padding_top.value : 0.0f;
                    float pad_b = child_style.padding_bottom.isPixel() ? child_style.padding_bottom.value : 0.0f;
                    float border_w = child_style.border_width;
                    if (border_w < 0.0f) {
                        border_w = 0.0f;
                    }

                    // 计算宽度：优先使用 CSS 显式指定的宽度
                    float width_px = 0.0f;
                    if (child_style.width.isPixel()) {
                        width_px = child_style.width.value;
                    } else if (child_style.width.isPercent()) {
                        width_px = parsePercentValue(child_style.width, content_w);
                    } else if (has_text_metrics) {
                        width_px = metrics.content_width_px + pad_l + pad_r + border_w * 2.0f;
                    }
                    if (width_px <= 0.0f && has_text_metrics) {
                        width_px = metrics.line_height_px;
                    }
                    if (width_px <= 0.0f) {
                        width_px = 16.0f; // 最小 fallback
                    }

                    // 计算高度：优先使用 CSS 指定的高度，否则使用文本行高 + padding + border
                    float height_px = 0.0f;
                    if (child_style.height.isPixel()) {
                        height_px = child_style.height.value;
                    } else if (child_style.height.isPercent()) {
                        // 百分比高度暂不支持，使用内容高度
                        if (has_text_metrics) {
                            height_px = metrics.line_height_px + pad_t + pad_b + border_w * 2.0f;
                        }
                    } else if (has_text_metrics) {
                        height_px = metrics.line_height_px + pad_t + pad_b + border_w * 2.0f;
                    }
                    if (height_px <= 0.0f && has_text_metrics) {
                        height_px = metrics.line_height_px;
                    }
                    if (height_px <= 0.0f) {
                        height_px = 16.0f; // 最小 fallback
                    }

                    item.preferred_width = width_px;
                    item.preferred_height = height_px;
                    item.line_height_px = height_px;  // 使用元素高度作为行高贡献
                    
                    // baseline 计算：有文本时使用文本 baseline，否则使用底部对齐近似
                    if (has_text_metrics) {
                        item.baseline_from_border_top = border_w + pad_t + metrics.baseline_from_content_top_px;
                    } else {
                        // 无文本内容时，baseline 近似为元素底部（符合 CSS inline-block 的默认行为）
                        item.baseline_from_border_top = height_px;
                    }
                    item.vertical_align = child_style.vertical_align;

                    items.push_back(item);
                }

                if (!items.empty() && content_w > 0.0f) {
                    struct LineInfo {
                        std::vector<size_t> item_indices;
                        float max_baseline_from_border_top = 0.0f;
                        float max_line_height_px = 0.0f;
                    };

                    std::vector<LineInfo> lines;
                    LineInfo current_line{};
                    if (has_container_text_metrics) {
                        current_line.max_baseline_from_border_top = container_baseline_from_border_top;
                        current_line.max_line_height_px = container_line_height_px;
                    }
                    float current_line_used_w = 0.0f;

                    for (size_t i = 0; i < items.size(); ++i) {
                        InlineItem& item = items[i];
                        float total_w = item.margin_left + item.preferred_width + item.margin_right;

                        if (!current_line.item_indices.empty() && current_line_used_w + total_w > content_w + 0.1f) {
                            lines.push_back(current_line);
                            current_line = LineInfo{};
                            if (has_container_text_metrics) {
                                current_line.max_baseline_from_border_top = container_baseline_from_border_top;
                                current_line.max_line_height_px = container_line_height_px;
                            }
                            current_line_used_w = 0.0f;
                        }

                        item.offset_x_in_content = current_line_used_w + item.margin_left;
                        current_line.item_indices.push_back(i);
                        current_line_used_w += total_w;

                        if (item.baseline_from_border_top > current_line.max_baseline_from_border_top) {
                            current_line.max_baseline_from_border_top = item.baseline_from_border_top;
                        }
                        if (item.line_height_px > current_line.max_line_height_px) {
                            current_line.max_line_height_px = item.line_height_px;
                        }
                    }

                    if (!current_line.item_indices.empty()) {
                        lines.push_back(current_line);
                    }

                    float current_line_top = content_y;
                    for (const LineInfo& line : lines) {
                        float baseline_y = current_line_top + line.max_baseline_from_border_top;

                        for (size_t idx : line.item_indices) {
                            InlineItem& item = items[idx];
                            auto it_child_layout = layout_cache.find(item.node.get());
                            if (it_child_layout == layout_cache.end() || !it_child_layout->second) {
                                continue;
                            }
                            LayoutNode* child_layout = it_child_layout->second.get();

                            float new_x = content_x + item.offset_x_in_content;
                            float new_y = 0.0f;

                            // 根据 vertical-align 计算 Y 坐标
                            const std::string& va = item.vertical_align;
                            if (va == "top") {
                                // 顶部对齐：元素顶部与行顶部对齐
                                new_y = current_line_top;
                            } else if (va == "bottom") {
                                // 底部对齐：元素底部与行底部对齐
                                new_y = current_line_top + line.max_line_height_px - item.preferred_height;
                            } else if (va == "middle") {
                                // 中间对齐：元素中心与行中心对齐
                                float line_center = current_line_top + line.max_line_height_px * 0.5f;
                                new_y = line_center - item.preferred_height * 0.5f;
                            } else {
                                // baseline（默认）：元素 baseline 与行 baseline 对齐
                                new_y = baseline_y - item.baseline_from_border_top;
                            }

                            bool layout_changed = (child_layout->x != new_x) || (child_layout->y != new_y) ||
                                                  (child_layout->width != item.preferred_width) ||
                                                  (child_layout->height != item.preferred_height);

                            child_layout->x = new_x;
                            child_layout->y = new_y;
                            child_layout->width = item.preferred_width;
                            // 使用计算出的 preferred_height，它已经考虑了 CSS 指定的高度
                            child_layout->height = item.preferred_height;

                            child_layout->layout.position[0] = child_layout->x;
                            child_layout->layout.position[1] = child_layout->y;
                            child_layout->layout.dimensions[0] = child_layout->width;
                            child_layout->layout.dimensions[1] = child_layout->height;

                            if (layout_changed) {
                                dirty_rect_.expand(child_layout->x,
                                                   child_layout->y,
                                                   child_layout->width,
                                                   child_layout->height);
                            }
                        }

                        current_line_top += line.max_line_height_px;
                    }
                }
            }
        }

        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                walk(child);
            }
        }
    };

    walk(root);
}

void Engine::layoutPositionedElements(dom::DOMNodePtr root) {
    if (!root) {
        return;
    }

    std::function<void(const dom::DOMNodePtr&)> walk;
    walk = [this, root, &walk](const dom::DOMNodePtr& node) {
        if (!node) {
            return;
        }

        const auto& style = node->getComputedStyle();

        // Handle position: relative
        // Relative positioning: element stays in normal flow but is visually offset
        // by top/right/bottom/left values. This offset does NOT affect sibling layout.
        if (style.position == "relative") {
            auto it_layout = layout_cache.find(node.get());
            if (it_layout != layout_cache.end() && it_layout->second) {
                LayoutNode* layout = it_layout->second.get();

                // Compute offset values
                auto computeOffsetPx = [this](const dom::CSSValue& v, float reference_size) -> float {
                    if (v.isPixel()) {
                        return v.value;
                    }
                    if (v.isPercent()) {
                        return parsePercentValue(v, reference_size);
                    }
                    return 0.0f;
                };

                // For relative positioning, percentages are relative to the containing block
                // (parent's content box). We use the layout dimensions as approximation.
                float ref_w = layout->width;
                float ref_h = layout->height;
                if (auto parent = node->getParent()) {
                    auto it_parent = layout_cache.find(parent.get());
                    if (it_parent != layout_cache.end() && it_parent->second) {
                        ref_w = it_parent->second->width;
                        ref_h = it_parent->second->height;
                    }
                }

                bool has_left = style.left.isPixel() || style.left.isPercent();
                bool has_right = style.right.isPixel() || style.right.isPercent();
                bool has_top = style.top.isPixel() || style.top.isPercent();
                bool has_bottom = style.bottom.isPixel() || style.bottom.isPercent();

                float offset_x = 0.0f;
                float offset_y = 0.0f;

                // Horizontal: left takes precedence over right
                if (has_left) {
                    offset_x = computeOffsetPx(style.left, ref_w);
                } else if (has_right) {
                    offset_x = -computeOffsetPx(style.right, ref_w);
                }

                // Vertical: top takes precedence over bottom
                if (has_top) {
                    offset_y = computeOffsetPx(style.top, ref_h);
                } else if (has_bottom) {
                    offset_y = -computeOffsetPx(style.bottom, ref_h);
                }

                // Apply offset to the layout position
                if (offset_x != 0.0f || offset_y != 0.0f) {
                    float new_x = layout->x + offset_x;
                    float new_y = layout->y + offset_y;

                    bool layout_changed = (layout->x != new_x) || (layout->y != new_y);

                    layout->x = new_x;
                    layout->y = new_y;
                    layout->layout.position[0] = new_x;
                    layout->layout.position[1] = new_y;

                    if (layout_changed) {
                        dirty_rect_.expand(new_x, new_y, layout->width, layout->height);
                    }
                }
            }
        }

        // Handle position: absolute
        if (style.position == "absolute") {
            // Find containing block: nearest ancestor with position != static.
            dom::DOMNodePtr containing_block = nullptr;
            dom::DOMNodePtr current = node->getParent();
            while (current) {
                const auto& cs = current->getComputedStyle();
                if (cs.position != "static") {
                    containing_block = current;
                    break;
                }
                current = current->getParent();
            }

            if (!containing_block) {
                // Fallback to the layout root if no positioned ancestor is found.
                containing_block = root;
            }

            auto it_layout = layout_cache.find(node.get());
            auto it_cb_layout = layout_cache.find(containing_block.get());
            if (it_layout != layout_cache.end() && it_layout->second &&
                it_cb_layout != layout_cache.end() && it_cb_layout->second) {
                LayoutNode* layout = it_layout->second.get();
                LayoutNode* cb_layout = it_cb_layout->second.get();

                const auto& cb_style = containing_block->getComputedStyle();
                const auto& abs_style = node->getComputedStyle();

                std::string debug_class = node->getAttribute("class");
                bool debug_is_abs_badge = debug_class.find("abs-badge") != std::string::npos;

                float cb_x = cb_layout->x;
                float cb_y = cb_layout->y;
                float cb_w = cb_layout->width;
                float cb_h = cb_layout->height;

                if (cb_w <= 0.0f || cb_h <= 0.0f) {
                    // Degenerate containing block, keep existing layout.
                } else {
                    // 若 absolute 盒子的 CSS width/height 为 auto（未显式指定），
                    // 则无论 Yoga 给出的尺寸为何，都基于文本内容做一次 intrinsic sizing：
                    // 宽度 = 文本内容宽 + 水平 padding + border；
                    // 高度 = 一行文本行高 + 垂直 padding + border。
                    const bool abs_width_auto = abs_style.width.isAuto();
                    const bool abs_height_auto = abs_style.height.isAuto();
                    if (abs_width_auto || abs_height_auto ||
                        layout->width <= 0.0f || layout->height <= 0.0f) {
                        InlineMetrics metrics{};
                        if (computeInlineMetricsForNode(node, metrics, abs_style.font_size)) {
                            float pad_l = abs_style.padding_left.isPixel() ? abs_style.padding_left.value : 0.0f;
                            float pad_r = abs_style.padding_right.isPixel() ? abs_style.padding_right.value : 0.0f;
                            float pad_t = abs_style.padding_top.isPixel() ? abs_style.padding_top.value : 0.0f;
                            float pad_b = abs_style.padding_bottom.isPixel() ? abs_style.padding_bottom.value : 0.0f;
                            float border_w = abs_style.border_width;
                            if (border_w < 0.0f) {
                                border_w = 0.0f;
                            }
                            float box_w_intrinsic = metrics.content_width_px + pad_l + pad_r + border_w * 2.0f;
                            float box_h_intrinsic = metrics.line_height_px + pad_t + pad_b + border_w * 2.0f;

                            if (debug_is_abs_badge) {
                                SDL_Log("[LayoutEngine] ABS badge intrinsic: content_w=%.2f line_h=%.2f pad_l=%.1f pad_r=%.1f pad_t=%.1f pad_b=%.1f border=%.1f -> box_w=%.2f box_h=%.2f (width_auto=%d height_auto=%d before: w=%.2f h=%.2f)",
                                        metrics.content_width_px, metrics.line_height_px,
                                        pad_l, pad_r, pad_t, pad_b, border_w,
                                        box_w_intrinsic, box_h_intrinsic,
                                        abs_width_auto ? 1 : 0, abs_height_auto ? 1 : 0,
                                        layout->width, layout->height);
                            }

                            if (box_w_intrinsic > 0.0f && box_h_intrinsic > 0.0f) {
                                if (abs_width_auto || layout->width <= 0.0f) {
                                    layout->width = box_w_intrinsic;
                                }
                                if (abs_height_auto || layout->height <= 0.0f) {
                                    layout->height = box_h_intrinsic;
                                }
                                layout->layout.dimensions[0] = layout->width;
                                layout->layout.dimensions[1] = layout->height;

                                if (debug_is_abs_badge) {
                                    SDL_Log("[LayoutEngine] ABS badge final size: w=%.2f h=%.2f",
                                            layout->width, layout->height);
                                }
                            }
                        } else if (debug_is_abs_badge) {
                            SDL_Log("[LayoutEngine] ABS badge intrinsic metrics FAILED (font_size=%.1f)",
                                    abs_style.font_size);
                        }
                    } else if (debug_is_abs_badge) {
                        SDL_Log("[LayoutEngine] ABS badge intrinsic sizing skipped (width_auto=%d height_auto=%d layout_w=%.2f layout_h=%.2f)",
                                abs_width_auto ? 1 : 0, abs_height_auto ? 1 : 0,
                                layout->width, layout->height);
                    }

                    // In CSS，包含块通常是 padding box。这里先基于 border box，后续可视需要加上 padding。
                    auto computeOffsetPx = [this](const dom::CSSValue& v, float parent_size) -> float {
                        if (v.isPixel()) {
                            return v.value;
                        }
                        if (v.isPercent()) {
                            return parsePercentValue(v, parent_size);
                        }
                        return 0.0f;
                    };

                    bool has_left = style.left.isPixel() || style.left.isPercent();
                    bool has_right = style.right.isPixel() || style.right.isPercent();
                    bool has_top = style.top.isPixel() || style.top.isPercent();
                    bool has_bottom = style.bottom.isPixel() || style.bottom.isPercent();

                    float left_px = has_left ? computeOffsetPx(style.left, cb_w) : 0.0f;
                    float right_px = has_right ? computeOffsetPx(style.right, cb_w) : 0.0f;
                    float top_px = has_top ? computeOffsetPx(style.top, cb_h) : 0.0f;
                    float bottom_px = has_bottom ? computeOffsetPx(style.bottom, cb_h) : 0.0f;

                    if (debug_is_abs_badge) {
                        SDL_Log("[LayoutEngine] ABS badge offsets: has_left=%d has_right=%d has_top=%d has_bottom=%d left_px=%.1f right_px=%.1f top_px=%.1f bottom_px=%.1f cb=(%.1f,%.1f,%.1f,%.1f) box=(%.1f,%.1f)",
                                has_left ? 1 : 0, has_right ? 1 : 0,
                                has_top ? 1 : 0, has_bottom ? 1 : 0,
                                left_px, right_px, top_px, bottom_px,
                                cb_x, cb_y, cb_w, cb_h,
                                layout->width, layout->height);
                    }

                    float box_w = layout->width;
                    float box_h = layout->height;

                    float offset_x_local = 0.0f;
                    float offset_y_local = 0.0f;

                    // Horizontal: prefer left if specified; otherwise use right.
                    if (has_left) {
                        offset_x_local = left_px;
                    } else if (has_right) {
                        offset_x_local = cb_w - right_px - box_w;
                    } else {
                        // No explicit offset: keep the original relative offset within containing block.
                        offset_x_local = layout->x - cb_x;
                    }

                    // Vertical: prefer top if specified; otherwise use bottom.
                    if (has_top) {
                        offset_y_local = top_px;
                    } else if (has_bottom) {
                        offset_y_local = cb_h - bottom_px - box_h;
                    } else {
                        offset_y_local = layout->y - cb_y;
                    }

                    float new_x = cb_x + offset_x_local;
                    float new_y = cb_y + offset_y_local;

                    bool layout_changed = (layout->x != new_x) || (layout->y != new_y);

                    layout->x = new_x;
                    layout->y = new_y;
                    layout->layout.position[0] = new_x;
                    layout->layout.position[1] = new_y;

                    if (layout_changed) {
                        dirty_rect_.expand(new_x, new_y, layout->width, layout->height);
                    }
                }
            }
        }

        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                walk(child);
            }
        }
    };

    walk(root);
}

} // namespace dong::layout
