#pragma once
#include "../dom/css/computed_style.hpp"

namespace dong::layout {

inline bool shouldSkipLayoutNode(const dom::ComputedStyle& style) {
    return style.display == dom::CSSDisplay::Contents;
}

}  // namespace dong::layout
