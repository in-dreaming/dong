#pragma once

// QuickJS headers (built/installed by zig) rely on `NAN` for JS_FLOAT64_NAN on MSVC/clang-cl.
// Ensure `NAN` is available before including `quickjs.h`.
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "quickjs.h"
#ifdef __cplusplus
}
#endif
