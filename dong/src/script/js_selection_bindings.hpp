#pragma once

#include "quickjs_compat.h"

namespace dong::script {

class JSBindings;

// Initialize Selection/Range JS API:
// - window.getSelection() / document.getSelection()
// - document.createRange()
// - Range prototype methods
// - Selection prototype methods
void initializeSelectionAPI(JSContext* ctx, JSBindings* bindings);

} // namespace dong::script
