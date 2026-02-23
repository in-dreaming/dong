#pragma once

#include "../dom/dom/dom_node.hpp"
#include <memory>

namespace dong::layout {

struct LayoutNode;

struct StickyMetadata {
    float original_x = 0.0f;
    float original_y = 0.0f;
    float original_width = 0.0f;
    float original_height = 0.0f;

    float containing_block_x = 0.0f;
    float containing_block_y = 0.0f;
    float containing_block_width = 0.0f;
    float containing_block_height = 0.0f;

    dom::DOMNodePtr scroll_container;

    float inset_top = NAN;
    float inset_bottom = NAN;
    float inset_left = NAN;
    float inset_right = NAN;

    bool is_stuck = false;
    float visual_x = 0.0f;
    float visual_y = 0.0f;
};

void computeStickyMetadata(dom::DOMNodePtr node, LayoutNode* layout);

void applyStickyOffset(dom::DOMNodePtr node, LayoutNode* layout,
                       float scroll_x, float scroll_y);

dom::DOMNodePtr findScrollContainer(dom::DOMNodePtr node);

}  // namespace dong::layout
