#pragma once

#include "../dom/dom/dom_node.hpp"
#include "../layout/layout_engine.hpp"
#include <vector>

namespace dong::input {

enum class NavDirection { Up, Down, Left, Right };

// Find the best candidate element to navigate to from `current` in `dir`.
// Returns nullptr if no suitable candidate.
// `candidates` should be pre-filtered to only focusable, visible elements.
// `layout_engine` is used to obtain layout rects for scoring.
dom::DOMNodePtr findSpatialNavTarget(
    const dom::DOMNodePtr& current,
    NavDirection dir,
    const std::vector<dom::DOMNodePtr>& candidates,
    layout::Engine* layout_engine);

} // namespace dong::input
