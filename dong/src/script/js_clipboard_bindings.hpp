#pragma once

#include "quickjs_compat.h"

namespace dong::script {

// Initialize navigator.clipboard API (readText/writeText with Promise-based results)
void initializeClipboardAPI(JSContext* ctx);

} // namespace dong::script
