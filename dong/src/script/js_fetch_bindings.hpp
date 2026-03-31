#pragma once

#include "quickjs_compat.h"
#include <string>

namespace dong::script {

class JSBindings;

// Initialize the global fetch() API.
void initializeFetchAPI(JSContext* ctx, JSBindings* bindings);

// Tick pending fetch requests: resolves/rejects completed requests on the main thread.
// Must be called each frame from the engine tick loop.
void tickPendingFetches(JSContext* ctx);

// Cancel all pending fetches and free resources (called on shutdown / DOM reset).
void resetFetchState(JSContext* ctx);

} // namespace dong::script
