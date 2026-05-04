#include "aspect_ratio_resolver.hpp"
#include "../core/log.h"
#include <cmath>

namespace dong::layout {

void resolveAspectRatio(dom::DOMNodePtr node, YGNodeRef yoga_node) {
    if (!node || !yoga_node) return;

    const auto& style = node->getComputedStyle();
    float ratio = style.aspect_ratio;

    if (ratio <= 0.0f) return;

    const auto& width = style.width;
    const auto& height = style.height;
    const auto& min_width = style.min_width;
    const auto& max_width = style.max_width;
    const auto& min_height = style.min_height;
    const auto& max_height = style.max_height;

    bool width_is_auto = (width.unit == dom::CSSValue::Unit::AUTO);
    bool height_is_auto = (height.unit == dom::CSSValue::Unit::AUTO);

    if (width_is_auto && height_is_auto) {
        return;
    }

    if (!width_is_auto && !height_is_auto) {
        return;
    }

    if (!width_is_auto && height_is_auto) {
        float w = width.value;
        float computed_height = w / ratio;

        if (min_height.unit != dom::CSSValue::Unit::AUTO) {
            computed_height = std::max(computed_height, min_height.value);
        }
        if (max_height.unit != dom::CSSValue::Unit::AUTO) {
            computed_height = std::min(computed_height, max_height.value);
        }

        YGNodeStyleSetHeight(yoga_node, computed_height);
        DONG_LOG_DEBUG("Aspect ratio: width=%.2f, computed height=%.2f (ratio=%.3f)", w, computed_height, ratio);
    }
    else if (width_is_auto && !height_is_auto) {
        float h = height.value;
        float computed_width = h * ratio;

        if (min_width.unit != dom::CSSValue::Unit::AUTO) {
            computed_width = std::max(computed_width, min_width.value);
        }
        if (max_width.unit != dom::CSSValue::Unit::AUTO) {
            computed_width = std::min(computed_width, max_width.value);
        }

        YGNodeStyleSetWidth(yoga_node, computed_width);
        DONG_LOG_DEBUG("Aspect ratio: height=%.2f, computed width=%.2f (ratio=%.3f)", h, computed_width, ratio);
    }
}

}  // namespace dong::layout
