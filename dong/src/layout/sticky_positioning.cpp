#include "sticky_positioning.hpp"
#include "layout_engine.hpp"
#include <algorithm>
#include <cmath>

namespace dong::layout {

dom::DOMNodePtr findScrollContainer(dom::DOMNodePtr node) {
    if (!node) return nullptr;

    auto parent = node->getParentNode();
    while (parent) {
        const auto& style = parent->getComputedStyle();
        bool is_scroll = (style.overflow == "auto" || style.overflow == "scroll" ||
                         style.overflow_y == "auto" || style.overflow_y == "scroll" ||
                         style.overflow_x == "auto" || style.overflow_x == "scroll");

        if (is_scroll) {
            return parent;
        }

        parent = parent->getParentNode();
    }

    return nullptr;
}

void computeStickyMetadata(dom::DOMNodePtr node, LayoutNode* layout) {
    if (!node || !layout) return;

    auto meta = std::make_unique<StickyMetadata>();

    meta->original_x = layout->x;
    meta->original_y = layout->y;
    meta->original_width = layout->width;
    meta->original_height = layout->height;

    meta->scroll_container = findScrollContainer(node);

    const auto& style = node->getComputedStyle();
    if (style.top.unit != dom::CSSValue::Unit::AUTO) {
        meta->inset_top = style.top.value;
    }
    if (style.bottom.unit != dom::CSSValue::Unit::AUTO) {
        meta->inset_bottom = style.bottom.value;
    }
    if (style.left.unit != dom::CSSValue::Unit::AUTO) {
        meta->inset_left = style.left.value;
    }
    if (style.right.unit != dom::CSSValue::Unit::AUTO) {
        meta->inset_right = style.right.value;
    }

    auto parent = node->getParentNode();
    if (parent) {
        if (layout->parent) {
            const auto& parent_style = parent->getComputedStyle();
            float pad_top = 0.0f, pad_left = 0.0f, pad_right = 0.0f, pad_bottom = 0.0f;

            if (parent_style.padding_top.unit == dom::CSSValue::Unit::PIXEL) {
                pad_top = parent_style.padding_top.value;
            }
            if (parent_style.padding_left.unit == dom::CSSValue::Unit::PIXEL) {
                pad_left = parent_style.padding_left.value;
            }
            if (parent_style.padding_right.unit == dom::CSSValue::Unit::PIXEL) {
                pad_right = parent_style.padding_right.value;
            }
            if (parent_style.padding_bottom.unit == dom::CSSValue::Unit::PIXEL) {
                pad_bottom = parent_style.padding_bottom.value;
            }

            meta->containing_block_x = layout->parent->x + pad_left;
            meta->containing_block_y = layout->parent->y + pad_top;
            meta->containing_block_width = layout->parent->width - pad_left - pad_right;
            meta->containing_block_height = layout->parent->height - pad_top - pad_bottom;
        } else {
            meta->containing_block_x = 0.0f;
            meta->containing_block_y = 0.0f;
            meta->containing_block_width = 10000.0f;
            meta->containing_block_height = 10000.0f;
        }
    } else {
        meta->containing_block_x = 0.0f;
        meta->containing_block_y = 0.0f;
        meta->containing_block_width = 10000.0f;
        meta->containing_block_height = 10000.0f;
    }

    meta->visual_x = meta->original_x;
    meta->visual_y = meta->original_y;
    meta->is_stuck = false;

    layout->sticky_metadata = std::move(meta);
}

void applyStickyOffset(dom::DOMNodePtr node, LayoutNode* layout,
                       float scroll_x, float scroll_y) {
    if (!node || !layout || !layout->sticky_metadata) return;

    auto& meta = layout->sticky_metadata;

    meta->visual_x = meta->original_x;
    meta->visual_y = meta->original_y;
    meta->is_stuck = false;

    if (!std::isnan(meta->inset_top)) {
        float threshold_y = meta->original_y - meta->inset_top;

        if (scroll_y >= threshold_y) {
            float target_y = scroll_y + meta->inset_top;

            float max_y = meta->containing_block_y + meta->containing_block_height - meta->original_height;
            target_y = std::min(target_y, max_y);

            target_y = std::max(target_y, meta->containing_block_y);

            meta->visual_y = target_y;
            meta->is_stuck = true;
        }
    } else if (!std::isnan(meta->inset_bottom)) {
        float viewport_bottom = scroll_y + meta->original_height;
        float threshold_y = meta->original_y + meta->original_height + meta->inset_bottom;

        if (viewport_bottom <= threshold_y) {
            float target_y = viewport_bottom - meta->original_height - meta->inset_bottom;

            float min_y = meta->containing_block_y;
            target_y = std::max(target_y, min_y);

            float max_y = meta->containing_block_y + meta->containing_block_height - meta->original_height;
            target_y = std::min(target_y, max_y);

            meta->visual_y = target_y;
            meta->is_stuck = true;
        }
    }

    if (!std::isnan(meta->inset_left)) {
        float threshold_x = meta->original_x - meta->inset_left;

        if (scroll_x >= threshold_x) {
            float target_x = scroll_x + meta->inset_left;

            float max_x = meta->containing_block_x + meta->containing_block_width - meta->original_width;
            target_x = std::min(target_x, max_x);

            target_x = std::max(target_x, meta->containing_block_x);

            meta->visual_x = target_x;
            meta->is_stuck = true;
        }
    } else if (!std::isnan(meta->inset_right)) {
        float viewport_right = scroll_x + meta->original_width;
        float threshold_x = meta->original_x + meta->original_width + meta->inset_right;

        if (viewport_right <= threshold_x) {
            float target_x = viewport_right - meta->original_width - meta->inset_right;

            float min_x = meta->containing_block_x;
            target_x = std::max(target_x, min_x);

            float max_x = meta->containing_block_x + meta->containing_block_width - meta->original_width;
            target_x = std::min(target_x, max_x);

            meta->visual_x = target_x;
            meta->is_stuck = true;
        }
    }
}

}  // namespace dong::layout
