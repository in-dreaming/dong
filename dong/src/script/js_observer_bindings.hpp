// js_observer_bindings.hpp
#pragma once

extern "C" {
#include "quickjs.h"
}

namespace dong::script {

class JSBindings;

// Initialize Observer-related APIs (ResizeObserver, MutationObserver, IntersectionObserver)
void initializeObserverAPI(JSContext* ctx, JSBindings* bindings);

} // namespace dong::script
