// JS navigator.clipboard bindings
// Provides navigator.clipboard.readText() / writeText() returning Promises.

#include "js_clipboard_bindings.hpp"
#include "quickjs_compat.h"
#include "dong_platform.h"
#include "dong_clipboard.h"
#include "../core/log.h"

#include <cstdlib>
#include <cstring>

namespace dong::script {

// navigator.clipboard.readText() -> Promise<string>
static JSValue js_clipboard_readText(JSContext* ctx, JSValueConst this_val,
                                     int argc, JSValueConst* argv) {
    (void)this_val; (void)argc; (void)argv;

    JSValue resolving_funcs[2];
    JSValue promise = JS_NewPromiseCapability(ctx, resolving_funcs);
    if (JS_IsException(promise)) return promise;

    DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
    if (!cb) {
        JSValue err = JS_NewString(ctx, "Clipboard not available");
        JS_Call(ctx, resolving_funcs[1], JS_UNDEFINED, 1, &err);
        JS_FreeValue(ctx, err);
    } else {
        char* text = dong_clipboard_get_text(cb);
        JSValue result = JS_NewString(ctx, text ? text : "");
        JS_Call(ctx, resolving_funcs[0], JS_UNDEFINED, 1, &result);
        JS_FreeValue(ctx, result);
        free(text);
    }

    JS_FreeValue(ctx, resolving_funcs[0]);
    JS_FreeValue(ctx, resolving_funcs[1]);
    return promise;
}

// navigator.clipboard.writeText(text) -> Promise<undefined>
static JSValue js_clipboard_writeText(JSContext* ctx, JSValueConst this_val,
                                      int argc, JSValueConst* argv) {
    (void)this_val;

    JSValue resolving_funcs[2];
    JSValue promise = JS_NewPromiseCapability(ctx, resolving_funcs);
    if (JS_IsException(promise)) return promise;

    DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
    if (!cb) {
        JSValue err = JS_NewString(ctx, "Clipboard not available");
        JS_Call(ctx, resolving_funcs[1], JS_UNDEFINED, 1, &err);
        JS_FreeValue(ctx, err);
    } else {
        const char* text = NULL;
        if (argc > 0) {
            text = JS_ToCString(ctx, argv[0]);
        }
        int ok = dong_clipboard_set_text(cb, text ? text : "");
        if (text) {
            JS_FreeCString(ctx, text);
        }

        if (ok) {
            JS_Call(ctx, resolving_funcs[0], JS_UNDEFINED, 0, NULL);
        } else {
            JSValue err = JS_NewString(ctx, "Failed to write to clipboard");
            JS_Call(ctx, resolving_funcs[1], JS_UNDEFINED, 1, &err);
            JS_FreeValue(ctx, err);
        }
    }

    JS_FreeValue(ctx, resolving_funcs[0]);
    JS_FreeValue(ctx, resolving_funcs[1]);
    return promise;
}

void initializeClipboardAPI(JSContext* ctx) {
    if (!ctx) return;

    JSValue global = JS_GetGlobalObject(ctx);

    // Create navigator object if it doesn't exist
    JSValue navigator = JS_GetPropertyStr(ctx, global, "navigator");
    if (JS_IsUndefined(navigator) || JS_IsException(navigator)) {
        JS_FreeValue(ctx, navigator);
        navigator = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, global, "navigator", JS_DupValue(ctx, navigator));
    }

    // Set navigator.userAgent
    JSValue ua = JS_GetPropertyStr(ctx, navigator, "userAgent");
    if (JS_IsUndefined(ua)) {
        JS_SetPropertyStr(ctx, navigator, "userAgent", JS_NewString(ctx, "Dong/1.0"));
    }
    JS_FreeValue(ctx, ua);

    // Create navigator.clipboard object
    JSValue clipboard = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, clipboard, "readText",
        JS_NewCFunction(ctx, js_clipboard_readText, "readText", 0));
    JS_SetPropertyStr(ctx, clipboard, "writeText",
        JS_NewCFunction(ctx, js_clipboard_writeText, "writeText", 1));
    JS_SetPropertyStr(ctx, navigator, "clipboard", clipboard);

    JS_FreeValue(ctx, navigator);
    JS_FreeValue(ctx, global);
}

} // namespace dong::script
