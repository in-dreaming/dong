#include "layout_engine.hpp"
#include <yoga/YGNode.h>
#include <yoga/YGConfig.h>
#include <yoga/Yoga.h>
#include <iostream>

namespace dong::layout {

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

    // Create Yoga tree from DOM tree
    YGNode* yoga_root = createYogaNode(root);
    if (!yoga_root) return;

    // Set root size
    YGNodeStyleSetWidth(yoga_root, width);
    YGNodeStyleSetHeight(yoga_root, height);

    // Calculate layout
    YGNodeCalculateLayout(yoga_root, width, height, YGDirectionLTR);

    // Extract layout info back to cache (recursively)
    std::function<void(dom::DOMNodePtr, YGNode*)> extractLayoutRecursive =
        [this, &extractLayoutRecursive](dom::DOMNodePtr dom_node, YGNode* yoga_node) {
            if (!dom_node || !yoga_node) return;

            auto& node_layout = layout_cache[dom_node.get()];
            if (!node_layout) {
                node_layout = std::make_unique<LayoutNode>();
            }
            node_layout->yoga_node = yoga_node;
            node_layout->x = YGNodeLayoutGetLeft(yoga_node);
            node_layout->y = YGNodeLayoutGetTop(yoga_node);
            node_layout->width = YGNodeLayoutGetWidth(yoga_node);
            node_layout->height = YGNodeLayoutGetHeight(yoga_node);

            // Recursively extract child layouts
            uint32_t child_count = YGNodeGetChildCount(yoga_node);
            for (uint32_t i = 0; i < child_count; ++i) {
                YGNode* child_yoga = YGNodeGetChild(yoga_node, i);
                if (child_yoga && i < dom_node->getChildren().size()) {
                    const auto& child_dom = dom_node->getChildren()[i];
                    if (child_dom && child_dom->getType() == dom::DOMNode::NodeType::ELEMENT) {
                        extractLayoutRecursive(child_dom, child_yoga);
                    }
                }
            }
        };

    extractLayoutRecursive(root, yoga_root);
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

    YGNode* yoga_node = YGNodeNewWithConfig(yoga_config);
    if (!yoga_node) return nullptr;

    // Apply DOM styles to Yoga node
    applyDOMStylesToYoga(dom_node, yoga_node);

    // Recursively create Yoga nodes for children
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
    mapComputedStylesToYoga(style, yoga_node);
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
    if (style.flex_direction == "column") {
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumn);
    } else if (style.flex_direction == "column-reverse") {
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumnReverse);
    } else if (style.flex_direction == "row-reverse") {
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRowReverse);
    } else {
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRow);
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
    if (style.width.isPixel()) {
        YGNodeStyleSetWidth(yoga_node, style.width.value);
    } else if (style.width.isPercent()) {
        YGNodeStyleSetWidthPercent(yoga_node, style.width.value);
    }

    if (style.height.isPixel()) {
        YGNodeStyleSetHeight(yoga_node, style.height.value);
    } else if (style.height.isPercent()) {
        YGNodeStyleSetHeightPercent(yoga_node, style.height.value);
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
    YGNodeStyleSetFlexGrow(yoga_node, style.flex_grow);
    YGNodeStyleSetFlexShrink(yoga_node, style.flex_shrink);
    if (style.flex_basis.isPixel()) {
        YGNodeStyleSetFlexBasis(yoga_node, style.flex_basis.value);
    } else if (style.flex_basis.isPercent()) {
        YGNodeStyleSetFlexBasisPercent(yoga_node, style.flex_basis.value);
    }

    // Set min/max width and height (if CSS supports them)
    // TODO: Add min-width, max-width, min-height, max-height support to ComputedStyle
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
