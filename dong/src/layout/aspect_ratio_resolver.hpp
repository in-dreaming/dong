#pragma once
#include "../dom/dom/dom_node.hpp"
#include <yoga/Yoga.h>

namespace dong::layout {

void resolveAspectRatio(dom::DOMNodePtr node, YGNodeRef yoga_node);

}  // namespace dong::layout
