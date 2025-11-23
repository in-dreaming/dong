#include "layout_engine.hpp"
#include <yoga/YGNode.h>
#include <yoga/YGConfig.h>
#include <yoga/Yoga.h>
#include <iostream>
#include <algorithm>

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
    for (const auto& child : dom_node->getChildren()) {
        if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            YGNode* child_yoga = createYogaNode(child);
            if (child_yoga) {
                YGNodeInsertChild(yoga_node, child_yoga, YGNodeGetChildCount(yoga_node));

                auto& child_layout = layout_cache[child.get()];
                if (!child_layout) {
                    child_layout = std::make_unique<LayoutNode>();
                }
                child_layout->yoga_node = child_yoga;
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
            if (parent_style.display == "flex") {
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

    // Heuristic intrinsic sizing for text-like elements.
    // Since Yoga does not know about our text measurement, many text containers would
    // otherwise collapse to zero height. For common tags, ensure a reasonable
    // minimum height based on font-size.
    if ((tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
         tag == "h5" || tag == "h6" || tag == "p" || tag == "button") &&
        style.height.isAuto()) {
        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
        float line_height = font_size * 1.4f;
        YGNodeStyleSetMinHeight(yoga_node, line_height);
    }
}

void Engine::mapComputedStylesToYoga(const dom::ComputedStyle& style, YGNode* yoga_node) {
    if (!yoga_node) return;

    // Set display
    if (style.display == "none") {
        YGNodeStyleSetDisplay(yoga_node, YGDisplayNone);
    } else if (style.display == "flex") {
        YGNodeStyleSetDisplay(yoga_node, YGDisplayFlex);
    } else {
        YGNodeStyleSetDisplay(yoga_node, YGDisplayFlex);
    }

    // Set flex direction
    // For display:flex, respect flex_direction; for normal block layout, prefer column
    if (style.display == "flex") {
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
        // Approximate block layout as a vertical stack
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumn);
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
        YGNodeStyleSetAlignItems(yoga_node, YGAlignFlexStart);
    }

    // Set dimensions
    bool has_explicit_width = false;
    bool has_explicit_height = false;

    if (style.width.isPixel()) {
        YGNodeStyleSetWidth(yoga_node, style.width.value);
        has_explicit_width = true;
    } else if (style.width.isPercent()) {
        YGNodeStyleSetWidthPercent(yoga_node, style.width.value);
        has_explicit_width = true;
    }

    if (style.height.isPixel()) {
        YGNodeStyleSetHeight(yoga_node, style.height.value);
        has_explicit_height = true;
    } else if (style.height.isPercent()) {
        YGNodeStyleSetHeightPercent(yoga_node, style.height.value);
        has_explicit_height = true;
    }

    // Fallback: for block-like elements without an explicit width,
    // stretch to full available width. This approximates HTML block layout
    // when we don't have intrinsic content measurement.
    if (!has_explicit_width && style.display != "inline") {
        YGNodeStyleSetWidthPercent(yoga_node, 100.0f);
    }

    // Set margins
    if (style.margin_top.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeTop, style.margin_top.value);
    } else if (style.margin_top.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeTop, style.margin_top.value);
    }

    if (style.margin_right.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeRight, style.margin_right.value);
    } else if (style.margin_right.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeRight, style.margin_right.value);
    }

    if (style.margin_bottom.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeBottom, style.margin_bottom.value);
    } else if (style.margin_bottom.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeBottom, style.margin_bottom.value);
    }

    if (style.margin_left.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeLeft, style.margin_left.value);
    } else if (style.margin_left.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeLeft, style.margin_left.value);
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
    }

    // Map border width into Yoga so that border participates in box model sizing
    if (style.border_width > 0.0f) {
        YGNodeStyleSetBorder(yoga_node, YGEdgeLeft, style.border_width);
        YGNodeStyleSetBorder(yoga_node, YGEdgeTop, style.border_width);
        YGNodeStyleSetBorder(yoga_node, YGEdgeRight, style.border_width);
        YGNodeStyleSetBorder(yoga_node, YGEdgeBottom, style.border_width);
    }

    // Set min/max width and height
    if (style.min_width.isPixel()) {
        YGNodeStyleSetMinWidth(yoga_node, style.min_width.value);
    } else if (style.min_width.isPercent()) {
        YGNodeStyleSetMinWidthPercent(yoga_node, style.min_width.value);
    }
    if (style.max_width.isPixel()) {
        YGNodeStyleSetMaxWidth(yoga_node, style.max_width.value);
    } else if (style.max_width.isPercent()) {
        YGNodeStyleSetMaxWidthPercent(yoga_node, style.max_width.value);
    }
    if (style.min_height.isPixel()) {
        YGNodeStyleSetMinHeight(yoga_node, style.min_height.value);
    } else if (style.min_height.isPercent()) {
        YGNodeStyleSetMinHeightPercent(yoga_node, style.min_height.value);
    }
    if (style.max_height.isPixel()) {
        YGNodeStyleSetMaxHeight(yoga_node, style.max_height.value);
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

} // namespace dong::layout
