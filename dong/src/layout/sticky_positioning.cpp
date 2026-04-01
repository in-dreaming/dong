#include "sticky_positioning.hpp"
#include "layout_engine.hpp"
#include "../core/log.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>

namespace dong {
namespace layout {

namespace {

inline float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

inline bool hasInset(float inset) {
    return !std::isnan(inset);
}

struct ScrollportRect {
    float x;
    float y;
    float w;
    float h;

    ScrollportRect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}
};

ScrollportRect getScrollportRect(const StickyMetadata& meta) {
    ScrollportRect r;
    if (!meta.scroll_container) {
        return r;
    }
    r.x = meta.scroll_container->getClientLeft();
    r.y = meta.scroll_container->getClientTop();
    r.w = meta.scroll_container->getClientWidth();
    r.h = meta.scroll_container->getClientHeight();
    return r;
}

float maxVisualY(const StickyMetadata& meta) {
    return meta.containing_block_y + meta.containing_block_height - meta.original_height;
}

float maxVisualX(const StickyMetadata& meta) {
    return meta.containing_block_x + meta.containing_block_width - meta.original_width;
}

float resolveInsetPx(const dom::CSSValue& v, float reference_size) {
    if (!v.isSet() || v.isAuto()) {
        return NAN;
    }
    if (v.isPixel()) {
        return v.value;
    }
    if (v.isPercent()) {
        return reference_size * (v.value / 100.0f);
    }
    // TODO: support calc()/em/rem/vw/vh when layout context is available.
    return NAN;
}

} // namespace

dom::DOMNodePtr findScrollContainer(dom::DOMNodePtr node) {
    if (!node) return nullptr;

    dom::DOMNodePtr parent = node->getParentNode();
    while (parent) {
        const dom::ComputedStyle& style = parent->getComputedStyle();
        bool is_scroll = (style.overflow == dom::CSSOverflow::Auto || style.overflow == dom::CSSOverflow::Scroll ||
                         style.overflow_y == dom::CSSOverflow::Auto || style.overflow_y == dom::CSSOverflow::Scroll ||
                         style.overflow_x == dom::CSSOverflow::Auto || style.overflow_x == dom::CSSOverflow::Scroll);

        if (is_scroll) {
            return parent;
        }

        parent = parent->getParentNode();
    }

    return nullptr;
}

void computeStickyMetadata(dom::DOMNodePtr node, LayoutNode* layout) {
    if (!node || !layout) return;

    std::unique_ptr<StickyMetadata> meta(new StickyMetadata());

    meta->original_x = layout->x;
    meta->original_y = layout->y;
    meta->original_width = layout->width;
    meta->original_height = layout->height;

    meta->scroll_container = findScrollContainer(node);

    dom::DOMNodePtr parent = node->getParentNode();
    if (parent) {
        if (layout->parent) {
            const dom::ComputedStyle& parent_style = parent->getComputedStyle();
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

    const dom::ComputedStyle& style = node->getComputedStyle();
    meta->inset_top = resolveInsetPx(style.top, meta->containing_block_height);
    meta->inset_bottom = resolveInsetPx(style.bottom, meta->containing_block_height);
    meta->inset_left = resolveInsetPx(style.left, meta->containing_block_width);
    meta->inset_right = resolveInsetPx(style.right, meta->containing_block_width);

    meta->visual_x = meta->original_x;
    meta->visual_y = meta->original_y;
    meta->is_stuck = false;

    layout->sticky_metadata = std::move(meta);
}

void applyStickyOffset(dom::DOMNodePtr node, LayoutNode* layout, float scroll_x, float scroll_y) {
    if (!node || !layout || !layout->sticky_metadata) return;

    StickyMetadata* meta = layout->sticky_metadata.get();

    meta->visual_x = meta->original_x;
    meta->visual_y = meta->original_y;
    meta->is_stuck = false;

    const ScrollportRect sp = getScrollportRect(*meta);

    static const bool kDbgSticky = ([]() {
        const char* v = std::getenv("DONG_DEBUG_STICKY");
        return v && v[0] == '1';
    })();
    const bool dbg_this = kDbgSticky && node->getAttribute("id") == "sticky";
    if (dbg_this) {
        DONG_LOG_INFO(
            "[StickyDbg] begin scroll=(%.1f,%.1f) sp=(%.1f,%.1f,%.1f,%.1f) orig=(%.1f,%.1f) inset_top=%.1f",
            scroll_x, scroll_y, sp.x, sp.y, sp.w, sp.h, meta->original_x, meta->original_y, meta->inset_top);
    }

    // Note: `scroll_x/scroll_y` are in scroll-container content coordinates.
    // Layout positions are in global coordinates and the painter applies a `-scroll`
    // translate for scroll containers. Therefore, when a sticky element is stuck we
    // must set `visual_*` in the pre-translate space so that after `-scroll` it lands
    // at `scrollport + inset`.

    // --- Vertical stickiness ---
    if (hasInset(meta->inset_top)) {
        const float threshold_scroll_y = meta->original_y - (sp.y + meta->inset_top);
        if (dbg_this) {
            DONG_LOG_INFO("[StickyDbg] top: threshold=%.1f scroll_y=%.1f", threshold_scroll_y, scroll_y);
        }
        if (scroll_y >= threshold_scroll_y) {
            float target_y = sp.y + meta->inset_top + scroll_y;
            target_y = clampf(target_y, meta->containing_block_y, maxVisualY(*meta));
            meta->visual_y = target_y;
            meta->is_stuck = true;
        }
    } else if (hasInset(meta->inset_bottom) && sp.h > 0.0f) {
        const float scrollport_bottom = sp.y + sp.h;
        const float target_screen_y = scrollport_bottom - meta->inset_bottom - meta->original_height;
        const float current_screen_y = meta->original_y - scroll_y;
        if (current_screen_y > target_screen_y) {
            float target_y = target_screen_y + scroll_y;
            target_y = clampf(target_y, meta->containing_block_y, maxVisualY(*meta));
            meta->visual_y = target_y;
            meta->is_stuck = true;
        }
    }

    if (dbg_this) {
        DONG_LOG_INFO("[StickyDbg] end: is_stuck=%d visual_y=%.1f", meta->is_stuck ? 1 : 0, meta->visual_y);
    }

    // --- Horizontal stickiness ---
    if (hasInset(meta->inset_left)) {
        const float threshold_scroll_x = meta->original_x - (sp.x + meta->inset_left);
        if (scroll_x >= threshold_scroll_x) {
            float target_x = sp.x + meta->inset_left + scroll_x;
            target_x = clampf(target_x, meta->containing_block_x, maxVisualX(*meta));
            meta->visual_x = target_x;
            meta->is_stuck = true;
        }
    } else if (hasInset(meta->inset_right) && sp.w > 0.0f) {
        const float scrollport_right = sp.x + sp.w;
        const float target_screen_x = scrollport_right - meta->inset_right - meta->original_width;
        const float current_screen_x = meta->original_x - scroll_x;
        if (current_screen_x > target_screen_x) {
            float target_x = target_screen_x + scroll_x;
            target_x = clampf(target_x, meta->containing_block_x, maxVisualX(*meta));
            meta->visual_x = target_x;
            meta->is_stuck = true;
        }
    }
}

}  // namespace layout
}  // namespace dong
