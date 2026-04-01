#include "js_bindings.hpp"
#include "js_node_bindings.hpp"
#include "js_observer_bindings.hpp"
#include "js_clipboard_bindings.hpp"
#include "js_selection_bindings.hpp"
#include "js_fetch_bindings.hpp"
#include "../dom/editing_commands.hpp"
#include "../dom/contenteditable.hpp"
#include "../dom/selection.hpp"
#include "../layout/layout_engine.hpp"
#include "../dom/details_element.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <unordered_set>
#include "../core/log.h"
#include "../dom/css/style_engine.hpp"
#include "../dom/focus_manager.hpp"
#include "../dom/input_element.hpp"
#include "../dom/select_element.hpp"
#include "quickjs_compat.h"


// Active JSBindings for current view operation (single-threaded).
// In shared-JS mode, multiple JSBindings share one JSContext, so
// we can't rely solely on JS_GetContextOpaque. Before any view operation
// (tick, loadHTML, input, evalScript), the EngineView sets this to the
// correct JSBindings for that view.
static dong::script::JSBindings* s_active_bindings = nullptr;

// Helper to get JSBindings from context opaque data
static dong::script::JSBindings* getBindingsFromContext(JSContext* ctx) {
    // In shared-JS mode, prefer the explicitly activated bindings.
    if (s_active_bindings) return s_active_bindings;
    if (!ctx) return nullptr;
    return static_cast<dong::script::JSBindings*>(JS_GetContextOpaque(ctx));
}

// ---- Per-view document binding support ----
// In shared-JS mode, a per-view document object stores a pointer to the
// JSBindings that owns it via an internal "__db" property.  When a doc_*
// callback is invoked on such an object, we read the pointer from this_val
// so the correct DOM tree is accessed regardless of which view is "active".

static void setDocBindings(JSContext* ctx, JSValue doc, dong::script::JSBindings* bindings) {
    JS_SetPropertyStr(ctx, doc, "__db",
                      JS_NewInt64(ctx, reinterpret_cast<int64_t>(bindings)));
}

// Resolve JSBindings for a doc_* callback.  Checks this_val (the document
// object the method was called on) first; falls back to context-level lookup.
static dong::script::JSBindings* getBindingsForDoc(JSContext* ctx, JSValueConst this_val) {
    JSValue v = JS_GetPropertyStr(ctx, this_val, "__db");
    if (JS_IsNumber(v)) {
        int64_t ptr = 0;
        JS_ToInt64(ctx, &ptr, v);
        JS_FreeValue(ctx, v);
        if (ptr) return reinterpret_cast<dong::script::JSBindings*>(ptr);
    } else {
        JS_FreeValue(ctx, v);
    }
    return getBindingsFromContext(ctx);
}

static bool debugScriptEval() {
    return std::getenv("DONG_DEBUG_SCRIPT_EVAL") != nullptr;
}

// (Removed global forward declarations - these functions are now declared below)

// ============================================================
// Console API Implementation - extern "C" for QuickJS callbacks
// ============================================================

// Macros for common getter/setter patterns to reduce .text bloat
#define DONG_JS_STRING_GETTER(name, expr) \
    static JSValue name(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) { \
        (void)argc; (void)argv; \
        auto node = dong::script::JSBindings::getNodeOpaque(ctx, this_val); \
        if (!node) return JS_UNDEFINED; \
        return JS_NewString(ctx, (expr).c_str()); \
    }

#define DONG_JS_STRING_SETTER(name, method) \
    static JSValue name(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) { \
        (void)this_val; \
        if (argc < 1) return JS_UNDEFINED; \
        auto node = dong::script::JSBindings::getNodeOpaque(ctx, this_val); \
        const char* s = JS_ToCString(ctx, argv[0]); \
        if (node && s) { node->method(s); node->markLayoutDirty(); } \
        if (s) JS_FreeCString(ctx, s); \
        return JS_UNDEFINED; \
    }

#define DONG_JS_ATTR_GETTER(name, attr) \
    static JSValue name(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) { \
        (void)argc; (void)argv; \
        auto node = dong::script::JSBindings::getNodeOpaque(ctx, this_val); \
        if (!node) return JS_NewString(ctx, ""); \
        return JS_NewString(ctx, node->getAttribute(attr).c_str()); \
    }

extern "C" {

// Forward declarations for CSSOM API functions
static JSValue css_supports(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
JSValue window_matchMedia(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static JSValue console_impl(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int level) {
    (void)this_val;
    std::string output;
    for (int i = 0; i < argc; i++) {
        if (i > 0) output += " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) { output += str; JS_FreeCString(ctx, str); }
        else { output += "[object]"; }
    }
    switch (level) {
    case 0: DONG_LOG_INFO("[JS] %s", output.c_str()); break;
    case 1: DONG_LOG_WARN("[JS] %s", output.c_str()); break;
    default: DONG_LOG_ERROR("[JS] %s", output.c_str()); break;
    }
    return JS_UNDEFINED;
}

static JSValue console_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return console_impl(ctx, this_val, argc, argv, 0);
}
static JSValue console_warn(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return console_impl(ctx, this_val, argc, argv, 1);
}
static JSValue console_error(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return console_impl(ctx, this_val, argc, argv, 2);
}

} // extern "C"

// ============================================================
// Window-level API stubs (alert, confirm, prompt)
// ============================================================

extern "C" {

static JSValue js_alert(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc > 0) {
        const char* str = JS_ToCString(ctx, argv[0]);
        if (str) {
            DONG_LOG_INFO("[JS:alert] %s", str);
            JS_FreeCString(ctx, str);
        }
    }
    return JS_UNDEFINED;
}

static JSValue js_confirm(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc > 0) {
        const char* str = JS_ToCString(ctx, argv[0]);
        if (str) {
            DONG_LOG_INFO("[JS:confirm] %s", str);
            JS_FreeCString(ctx, str);
        }
    }
    return JS_TRUE;
}

static JSValue js_prompt(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc > 0) {
        const char* str = JS_ToCString(ctx, argv[0]);
        if (str) {
            DONG_LOG_INFO("[JS:prompt] %s", str);
            JS_FreeCString(ctx, str);
        }
    }
    // Return default value if provided, otherwise null
    if (argc > 1) {
        return JS_DupValue(ctx, argv[1]);
    }
    return JS_NULL;
}

// ============================================================
// structuredClone API - Deep clone values
// ============================================================

static JSValue structuredClone_impl(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue js_structuredClone(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc < 1) {
        JS_ThrowTypeError(ctx, "structuredClone requires at least one argument");
        return JS_UNDEFINED;
    }
    return structuredClone_impl(ctx, this_val, argc, argv);
}

// Internal recursive clone implementation
static JSValue structuredClone_impl(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val; (void)argc;

    JSValue value = argv[0];
    int tag = JS_VALUE_GET_TAG(value);

    // Handle primitives
    if (tag == JS_TAG_NULL) return JS_NULL;
    if (tag == JS_TAG_BOOL) return JS_DupValue(ctx, value);
    if (tag == JS_TAG_INT || tag == JS_TAG_FLOAT64) return JS_DupValue(ctx, value);
    if (JS_IsString(value)) return JS_DupValue(ctx, value);

    // Handle array
    if (JS_IsArray(ctx, value)) {
        JSValue arr = JS_NewArray(ctx);

        // Get length property
        JSValue lenVal = JS_GetPropertyStr(ctx, value, "length");
        uint32_t len = 0;
        JS_ToUint32(ctx, &len, lenVal);
        JS_FreeValue(ctx, lenVal);

        for (uint32_t i = 0; i < len; i++) {
            JSValue elem = JS_GetPropertyUint32(ctx, value, i);
            JSValue cloned = structuredClone_impl(ctx, this_val, 1, &elem);
            JS_SetPropertyUint32(ctx, arr, i, cloned);
            JS_FreeValue(ctx, elem);
        }
        return arr;
    }

    // Handle plain objects
    if (JS_IsObject(value)) {
        // Check if it's a function or special object (we skip these)
        if (JS_IsFunction(ctx, value)) {
            JS_ThrowTypeError(ctx, "structuredClone() cannot clone functions");
            return JS_UNDEFINED;
        }

        JSValue obj = JS_NewObject(ctx);

        // Iterate over properties
        JSPropertyEnum* props = nullptr;
        uint32_t prop_count = 0;
        if (JS_GetOwnPropertyNames(ctx, &props, &prop_count, value, JS_GPN_ENUM_ONLY | JS_GPN_STRING_MASK) == 0) {
            for (uint32_t i = 0; i < prop_count; i++) {
                JSValue prop_val = JS_GetPropertyInternal(ctx, value, props[i].atom, value, 0);
                JSValue cloned = structuredClone_impl(ctx, this_val, 1, &prop_val);
                JS_SetPropertyInternal(ctx, obj, props[i].atom, cloned, obj, JS_PROP_THROW);
                JS_FreeValue(ctx, prop_val);
                JS_FreeAtom(ctx, props[i].atom);
            }
            JS_FreePropertyEnum(ctx, props, prop_count);
        }
        return obj;
    }

    // For unsupported types, return as-is (simplified for this implementation)
    return JS_DupValue(ctx, value);
}

// ============================================================
// DOMRect API
// ============================================================

static JSValue domrect_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static JSValue domrect_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;

    JSValue obj = JS_NewObject(ctx);

    double x = 0, y = 0, width = 0, height = 0;
    if (argc > 0) JS_ToFloat64(ctx, &x, argv[0]);
    if (argc > 1) JS_ToFloat64(ctx, &y, argv[1]);
    if (argc > 2) JS_ToFloat64(ctx, &width, argv[2]);
    if (argc > 3) JS_ToFloat64(ctx, &height, argv[3]);

    double right = x + width;
    double bottom = y + height;

    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, y));
    JS_SetPropertyStr(ctx, obj, "width", JS_NewFloat64(ctx, width));
    JS_SetPropertyStr(ctx, obj, "height", JS_NewFloat64(ctx, height));
    JS_SetPropertyStr(ctx, obj, "top", JS_NewFloat64(ctx, y));
    JS_SetPropertyStr(ctx, obj, "right", JS_NewFloat64(ctx, right));
    JS_SetPropertyStr(ctx, obj, "bottom", JS_NewFloat64(ctx, bottom));
    JS_SetPropertyStr(ctx, obj, "left", JS_NewFloat64(ctx, x));

    return obj;
}

// ============================================================
// FormData API
// ============================================================

static JSValue formdata_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_append(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_get(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_getAll(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_has(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_delete(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_set(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_entries(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_keys(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue formdata_values(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

// Helper function to collect form controls for FormData serialization
static void collectFormControlsForFormData(const dong::dom::DOMNodePtr& node, std::vector<dong::dom::DOMNodePtr>& out) {
    if (!node) return;

    for (const auto& ch : node->getChildren()) {
        if (!ch) continue;
        if (ch->getType() == dong::dom::DOMNode::NodeType::ELEMENT) {
            const std::string& tag = ch->getTagName();
            // Only collect elements that can have a name attribute and contribute to form data
            if (tag == "input" || tag == "textarea" || tag == "select" || tag == "button") {
                out.push_back(ch);
            }
        }
        collectFormControlsForFormData(ch, out);
    }
}

// Helper function to serialize a form control for FormData
static bool serializeFormControl(const dong::dom::DOMNodePtr& control,
                                 std::vector<std::pair<std::string, std::string>>& entries) {
    if (!control) return false;

    // Skip disabled controls
    if (control->hasAttribute("disabled")) {
        return false;
    }

    // Skip controls without a name attribute
    if (!control->hasAttribute("name")) {
        return false;
    }

    const std::string& name = control->getAttribute("name");
    const std::string& tag = control->getTagName();

    if (tag == "input") {
        // Get input type
        std::string input_type = "text";
        if (control->hasAttribute("type")) {
            input_type = control->getAttribute("type");
        }

        // For checkbox and radio, only serialize if checked
        if (input_type == "checkbox" || input_type == "radio") {
            if (!control->hasAttribute("checked")) {
                return false; // Not checked, don't serialize
            }
        }

        // Get the value, default to "on" for checkbox/radio without explicit value
        std::string value;
        if (input_type == "checkbox" || input_type == "radio") {
            if (control->hasAttribute("value")) {
                value = control->getAttribute("value");
            } else {
                value = "on";
            }
        } else if (input_type == "file") {
            // File input not yet supported, just send the filename
            value = ""; // Would normally send the file data
        } else {
            if (control->hasAttribute("value")) {
                value = control->getAttribute("value");
            } else if (auto* st = dong::dom::getInputState(control)) {
                value = st->getValue();
            } else {
                value = "";
            }
        }

        entries.push_back({name, value});
        return true;
    }

    if (tag == "textarea") {
        std::string value;
        if (auto* st = dong::dom::getInputState(control)) {
            value = st->getValue();
        } else if (control->hasAttribute("value")) {
            value = control->getAttribute("value");
        } else {
            // Get text content from textarea
            value = control->getTextContent();
        }
        entries.push_back({name, value});
        return true;
    }

    if (tag == "select") {
        if (auto* st = dong::dom::getSelectState(control)) {
            std::string value = st->getSelectedValue();
            entries.push_back({name, value});
            return true;
        } else if (control->hasAttribute("value")) {
            entries.push_back({name, control->getAttribute("value")});
            return true;
        }
    }

    // button element with type="submit" or type="button"
    // Only include if it was clicked (not implemented here)
    // For now, exclude button elements from automatic serialization

    return false;
}

// Opaque object to store form data
struct FormDataStorage {
    std::vector<std::pair<std::string, std::string>> entries;
};

static JSClassID g_formdata_class_id = 0;

static void formdata_finalizer(JSRuntime* rt, JSValue val) {
    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque(val, g_formdata_class_id));
    delete storage;
}

static JSClassDef g_formdata_class_def = {
    /* class_name */ "FormData",
    /* finalizer */ formdata_finalizer,
};

static JSValue formdata_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;

    JSValue obj = JS_NewObjectClass(ctx, g_formdata_class_id);
    if (JS_IsException(obj)) return obj;

    // Create storage
    auto* storage = new FormDataStorage();
    JS_SetOpaque(obj, storage);

    // Handle form element argument if argc > 0
    if (argc > 0) {
        auto form = dong::script::JSBindings::getNodeOpaque(ctx, argv[0]);
        if (form && form->getTagName() == "form") {
            // Collect all form controls
            std::vector<dong::dom::DOMNodePtr> controls;
            controls.reserve(32);
            collectFormControlsForFormData(form, controls);

            // Serialize each control
            for (const auto& control : controls) {
                serializeFormControl(control, storage->entries);
            }
        }
    }

    JS_SetPropertyStr(ctx, obj, "append",
        JS_NewCFunction(ctx, formdata_append, "append", 2));
    JS_SetPropertyStr(ctx, obj, "get",
        JS_NewCFunction(ctx, formdata_get, "get", 1));
    JS_SetPropertyStr(ctx, obj, "getAll",
        JS_NewCFunction(ctx, formdata_getAll, "getAll", 1));
    JS_SetPropertyStr(ctx, obj, "has",
        JS_NewCFunction(ctx, formdata_has, "has", 1));
    JS_SetPropertyStr(ctx, obj, "delete",
        JS_NewCFunction(ctx, formdata_delete, "delete", 1));
    JS_SetPropertyStr(ctx, obj, "set",
        JS_NewCFunction(ctx, formdata_set, "set", 2));
    JS_SetPropertyStr(ctx, obj, "entries",
        JS_NewCFunction(ctx, formdata_entries, "entries", 0));
    JS_SetPropertyStr(ctx, obj, "keys",
        JS_NewCFunction(ctx, formdata_keys, "keys", 0));
    JS_SetPropertyStr(ctx, obj, "values",
        JS_NewCFunction(ctx, formdata_values, "values", 0));

    return obj;
}

static JSValue formdata_append(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;

    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_UNDEFINED;

    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_UNDEFINED;

    const char* value = "";
    if (argc > 1) {
        value = JS_ToCString(ctx, argv[1]);
        if (!value) {
            JS_FreeCString(ctx, name);
            return JS_UNDEFINED;
        }
    }

    storage->entries.push_back({name, value ? value : ""});

    JS_FreeCString(ctx, name);
    if (value) JS_FreeCString(ctx, value);

    return JS_UNDEFINED;
}

static JSValue formdata_get(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_NULL;

    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_NULL;

    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_NULL;

    for (const auto& entry : storage->entries) {
        if (entry.first == name) {
            JS_FreeCString(ctx, name);
            return JS_NewString(ctx, entry.second.c_str());
        }
    }

    JS_FreeCString(ctx, name);
    return JS_NULL;
}

static JSValue formdata_getAll(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_NewArray(ctx);

    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_NewArray(ctx);

    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_NewArray(ctx);

    JSValue arr = JS_NewArray(ctx);
    uint32_t idx = 0;
    for (const auto& entry : storage->entries) {
        if (entry.first == name) {
            JS_SetPropertyUint32(ctx, arr, idx++, JS_NewString(ctx, entry.second.c_str()));
        }
    }

    JS_FreeCString(ctx, name);
    return arr;
}

static JSValue formdata_has(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;

    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_FALSE;

    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_FALSE;

    for (const auto& entry : storage->entries) {
        if (entry.first == name) {
            JS_FreeCString(ctx, name);
            return JS_TRUE;
        }
    }

    JS_FreeCString(ctx, name);
    return JS_FALSE;
}

static JSValue formdata_delete(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;

    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_UNDEFINED;

    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_UNDEFINED;

    auto it = std::remove_if(storage->entries.begin(), storage->entries.end(),
        [name](const auto& entry) { return entry.first == name; });
    storage->entries.erase(it, storage->entries.end());

    JS_FreeCString(ctx, name);
    return JS_UNDEFINED;
}

static JSValue formdata_set(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;

    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_UNDEFINED;

    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_UNDEFINED;

    const char* value = "";
    if (argc > 1) {
        value = JS_ToCString(ctx, argv[1]);
        if (!value) {
            JS_FreeCString(ctx, name);
            return JS_UNDEFINED;
        }
    }

    // Remove all existing entries with this name
    formdata_delete(ctx, this_val, 1, argv);

    // Add new entry
    storage->entries.push_back({name, value ? value : ""});

    JS_FreeCString(ctx, name);
    if (value) JS_FreeCString(ctx, value);

    return JS_UNDEFINED;
}

static JSValue formdata_entries(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_NewArray(ctx);

    JSValue arr = JS_NewArray(ctx);
    for (uint32_t i = 0; i < storage->entries.size(); i++) {
        JSValue entry = JS_NewArray(ctx);
        JS_SetPropertyUint32(ctx, entry, 0, JS_NewString(ctx, storage->entries[i].first.c_str()));
        JS_SetPropertyUint32(ctx, entry, 1, JS_NewString(ctx, storage->entries[i].second.c_str()));
        JS_SetPropertyUint32(ctx, arr, i, entry);
    }
    return arr;
}

static JSValue formdata_keys(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_NewArray(ctx);

    JSValue arr = JS_NewArray(ctx);
    for (uint32_t i = 0; i < storage->entries.size(); i++) {
        JS_SetPropertyUint32(ctx, arr, i, JS_NewString(ctx, storage->entries[i].first.c_str()));
    }
    return arr;
}

static JSValue formdata_values(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto* storage = static_cast<FormDataStorage*>(JS_GetOpaque((JSValue)this_val, g_formdata_class_id));
    if (!storage) return JS_NewArray(ctx);

    JSValue arr = JS_NewArray(ctx);
    for (uint32_t i = 0; i < storage->entries.size(); i++) {
        JS_SetPropertyUint32(ctx, arr, i, JS_NewString(ctx, storage->entries[i].second.c_str()));
    }
    return arr;
}

// ============================================================
// DOMParser API
// ============================================================

static JSValue domparser_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
static JSValue domparser_parseFromString(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

static JSValue domparser_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val; (void)argc; (void)argv;

    JSValue obj = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, obj, "parseFromString",
        JS_NewCFunction(ctx, domparser_parseFromString, "parseFromString", 2));

    return obj;
}

static JSValue domparser_parseFromString(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) {
        JS_ThrowTypeError(ctx, "parseFromString requires 2 arguments");
        return JS_UNDEFINED;
    }

    const char* str = JS_ToCString(ctx, argv[0]);
    const char* mimeType = JS_ToCString(ctx, argv[1]);

    if (!str || !mimeType) {
        if (str) JS_FreeCString(ctx, str);
        if (mimeType) JS_FreeCString(ctx, mimeType);
        JS_ThrowTypeError(ctx, "Invalid arguments to parseFromString");
        return JS_UNDEFINED;
    }

    // Only support text/html for now
    if (std::string(mimeType) != "text/html") {
        JS_FreeCString(ctx, str);
        JS_FreeCString(ctx, mimeType);
        JS_ThrowTypeError(ctx, "Unsupported MIME type: %s", mimeType);
        return JS_UNDEFINED;
    }

    // Get JSBindings from context
    auto bindings = static_cast<dong::script::JSBindings*>(JS_GetContextOpaque(ctx));
    if (!bindings || !bindings->dom_manager_) {
        JS_FreeCString(ctx, str);
        JS_FreeCString(ctx, mimeType);
        return JS_NULL;
    }

    // Parse HTML into standalone document (no connection to live DOM)
    using dong::dom::HTMLParser;
    HTMLParser parser;
    auto root = parser.parse(std::string(str));

    JS_FreeCString(ctx, str);
    JS_FreeCString(ctx, mimeType);

    if (!root) {
        return JS_NULL;
    }

    // Build a document-like JS object wrapping the parsed tree.
    JSValue doc = JS_NewObject(ctx);

    // Helper to search the parsed tree (no dom_manager available)
    std::function<dong::dom::DOMNodePtr(const dong::dom::DOMNodePtr&, const std::string&)> findByTag;
    findByTag = [&](const dong::dom::DOMNodePtr& node, const std::string& tag) -> dong::dom::DOMNodePtr {
        if (!node) return nullptr;
        if (node->getType() == dong::dom::DOMNode::NodeType::ELEMENT) {
            std::string name = node->getTagName();
            // Lowercase comparison
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (name == tag) return node;
        }
        for (auto& child : node->getChildren()) {
            auto found = findByTag(child, tag);
            if (found) return found;
        }
        return nullptr;
    };

    // document.documentElement = <html>
    auto html_node = findByTag(root, "html");
    if (!html_node) html_node = root; // fallback
    JSValue doc_elem = bindings->createJSElement(ctx, html_node);
    JS_SetPropertyStr(ctx, doc, "documentElement", doc_elem);

    // document.body = <body>
    auto body_node = findByTag(root, "body");
    if (body_node) {
        JSValue body_js = bindings->createJSElement(ctx, body_node);
        JS_SetPropertyStr(ctx, doc, "body", body_js);
    } else {
        JS_SetPropertyStr(ctx, doc, "body", JS_NULL);
    }

    // document.head = <head>
    auto head_node = findByTag(root, "head");
    if (head_node) {
        JSValue head_js = bindings->createJSElement(ctx, head_node);
        JS_SetPropertyStr(ctx, doc, "head", head_js);
    } else {
        JS_SetPropertyStr(ctx, doc, "head", JS_NULL);
    }

    return doc;
}

} // extern "C"

namespace dong::script {

// CSS API initialization functions
void initializeCSSAPI(JSContext* ctx);
void initializeWindowCSSAPI(JSContext* ctx);

// Forward declarations for multi-view support
static void ensureDongViewsObject(JSContext* ctx);
static JSValue js_dong_getView(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

namespace {

std::string camelToCss(const std::string& property) {
    std::string css;
    css.reserve(property.size() * 2);
    for (char c : property) {
        if (std::isupper(static_cast<unsigned char>(c))) {
            css.push_back('-');
            css.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else {
            css.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }
    return css;
}

std::string getComputedStyleValue(const dom::ComputedStyle& style, const std::string& css_prop);

std::string jsValueToString(JSContext* ctx, JSValueConst value) {
    JSValue str = JS_ToString(ctx, value);
    if (JS_IsException(str)) {
        JS_FreeValue(ctx, str);
        return "";
    }
    const char* cstr = JS_ToCString(ctx, str);
    std::string result = cstr ? cstr : "";
    if (cstr) {
        JS_FreeCString(ctx, cstr);
    }
    JS_FreeValue(ctx, str);
    return result;
}

std::string formatCssLengthPx(float value) {
    if (std::fabs(value - std::round(value)) < 0.001f) {
        return std::to_string(static_cast<int>(std::round(value))) + "px";
    }
    std::ostringstream oss;
    oss << value << "px";
    return oss.str();
}

std::string getStyleValueForJS(const dom::DOMNodePtr& node, const std::string& css_prop) {
    if (!node) return "";

    std::string inline_value = node->getInlineStyleProperty(css_prop);
    if (!inline_value.empty()) {
        return inline_value;
    }

    const auto& style = node->getComputedStyle();
    if (css_prop == "padding") {
        return formatCssLengthPx(style.padding_top.resolvePixels(0.0f, 16.0f, 800.0f, 600.0f));
    }
    if (css_prop == "border-width") {
        const float width = style.border_top_width >= 0.0f ? style.border_top_width : style.border_width;
        return formatCssLengthPx(width);
    }

    return getComputedStyleValue(style, css_prop);
}

std::string getComputedStyleValue(const dom::ComputedStyle& style, const std::string& css_prop) {
    if (css_prop == "display") return dong::dom::toString(style.display);
    if (css_prop == "color") return style.color;

    // Backgrounds
    if (css_prop == "background-color") return style.background_color;
    if (css_prop == "background-image") return style.background_image;
    if (css_prop == "background-size") return style.background_size;
    if (css_prop == "background-repeat") return dong::dom::toString(style.background_repeat);
    if (css_prop == "background-position") return style.background_position;
    if (css_prop == "background-attachment") return dong::dom::toString(style.background_attachment);
    if (css_prop == "background-clip") return dong::dom::toString(style.background_clip);
    if (css_prop == "background-origin") return dong::dom::toString(style.background_origin);

    // Typography
    if (css_prop == "font-size") return std::to_string(style.font_size);
    if (css_prop == "font-weight") return dong::dom::toString(style.font_weight);
    if (css_prop == "text-align") return dong::dom::toString(style.text_align);

    // Visual
    if (css_prop == "position") return dong::dom::toString(style.position);
    if (css_prop == "opacity") return std::to_string(style.opacity);

    // Border
    if (css_prop == "border-radius") return std::to_string(style.border_radius);
    if (css_prop == "border-width") return std::to_string(style.border_width);
    if (css_prop == "border-color") return style.border_color;

    return "";
}

// ============================================================
// CSSOM (minimal): document.styleSheets + insertRule/deleteRule
// ============================================================

bool getSheetEngineIndex(JSContext* ctx, JSValueConst sheet_obj, int32_t& out_index) {
    JSValue v = JS_GetPropertyStr(ctx, sheet_obj, "__sheet_engine_index__");
    if (JS_IsUndefined(v) || JS_IsNull(v)) {
        JS_FreeValue(ctx, v);
        return false;
    }
    int32_t idx = -1;
    JS_ToInt32(ctx, &idx, v);
    JS_FreeValue(ctx, v);
    if (idx < 0) return false;
    out_index = idx;
    return true;
}

JSValue buildCssRulesArray(JSContext* ctx, const dom::Stylesheet* sheet) {
    JSValue arr = JS_NewArray(ctx);
    if (!sheet) return arr;

    const auto& rules = sheet->getRules();
    for (size_t i = 0; i < rules.size(); ++i) {
        JSValue rule_obj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, rule_obj, "selectorText", JS_NewString(ctx, rules[i].selector.c_str()));
        // Best-effort: we don't currently preserve original declarations text.
        JS_SetPropertyStr(ctx, rule_obj, "cssText", JS_NewString(ctx, rules[i].selector.c_str()));
        JS_SetPropertyUint32(ctx, arr, static_cast<uint32_t>(i), rule_obj);
    }

    return arr;
}

void syncCssRulesProperty(JSContext* ctx, JSValueConst sheet_obj, const dom::Stylesheet* sheet) {
    JSValue arr = buildCssRulesArray(ctx, sheet);
    JSValue sheet_dup = JS_DupValue(ctx, sheet_obj);
    JS_SetPropertyStr(ctx, sheet_dup, "cssRules", arr);
    JS_FreeValue(ctx, sheet_dup);
}

JSValue createJSCSSStyleSheet(JSContext* ctx, size_t sheet_engine_index) {
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "__sheet_engine_index__", JS_NewInt32(ctx, static_cast<int32_t>(sheet_engine_index)));

    // Methods
    JS_SetPropertyStr(ctx, obj, "insertRule", JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
        auto bindings = getBindingsFromContext(c);
        if (!bindings || !bindings->dom_manager_) return JS_EXCEPTION;
        auto* se = bindings->dom_manager_->getStyleEngine();
        auto root = bindings->dom_manager_->getRoot();
        if (!se || !root) return JS_EXCEPTION;

        int32_t sheet_index = -1;
        if (!getSheetEngineIndex(c, this_val, sheet_index)) {
            return JS_ThrowTypeError(c, "Invalid CSSStyleSheet object");
        }

        if (argc < 1) {
            return JS_ThrowTypeError(c, "insertRule(ruleText, index?) requires ruleText");
        }

        const char* rule_text = JS_ToCString(c, argv[0]);
        if (!rule_text) {
            return JS_ThrowTypeError(c, "insertRule: ruleText must be string");
        }

        dom::Stylesheet* sheet = se->stylesheetAt(static_cast<size_t>(sheet_index));
        if (!sheet) {
            JS_FreeCString(c, rule_text);
            return JS_ThrowTypeError(c, "insertRule: sheet index out of range");
        }

        int32_t insert_index = static_cast<int32_t>(sheet->ruleCount());
        if (argc >= 2 && !JS_IsUndefined(argv[1])) {
            JS_ToInt32(c, &insert_index, argv[1]);
        }
        if (insert_index < 0) insert_index = 0;

        auto rules = se->parseCSS(rule_text);
        JS_FreeCString(c, rule_text);
        if (rules.empty()) {
            return JS_ThrowTypeError(c, "insertRule: failed to parse rule");
        }

        bool ok = sheet->insertRuleAt(static_cast<size_t>(insert_index), rules[0]);
        if (!ok) {
            return JS_ThrowTypeError(c, "insertRule: index out of range");
        }

        se->rebuildRuleIndex();
        se->computeStylesIncremental(root);
        root->markLayoutDirty();
        syncCssRulesProperty(c, this_val, sheet);

        return JS_NewInt32(c, insert_index);
    }, "insertRule", 2));

    JS_SetPropertyStr(ctx, obj, "deleteRule", JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
        auto bindings = getBindingsFromContext(c);
        if (!bindings || !bindings->dom_manager_) return JS_EXCEPTION;
        auto* se = bindings->dom_manager_->getStyleEngine();
        auto root = bindings->dom_manager_->getRoot();
        if (!se || !root) return JS_EXCEPTION;

        int32_t sheet_index = -1;
        if (!getSheetEngineIndex(c, this_val, sheet_index)) {
            return JS_ThrowTypeError(c, "Invalid CSSStyleSheet object");
        }

        if (argc < 1) {
            return JS_ThrowTypeError(c, "deleteRule(index) requires index");
        }

        int32_t del_index = -1;
        JS_ToInt32(c, &del_index, argv[0]);
        if (del_index < 0) {
            return JS_ThrowTypeError(c, "deleteRule: index out of range");
        }

        dom::Stylesheet* sheet = se->stylesheetAt(static_cast<size_t>(sheet_index));
        if (!sheet) {
            return JS_ThrowTypeError(c, "deleteRule: sheet index out of range");
        }

        bool ok = sheet->deleteRuleAt(static_cast<size_t>(del_index));
        if (!ok) {
            return JS_ThrowTypeError(c, "deleteRule: index out of range");
        }

        se->rebuildRuleIndex();
        se->computeStylesIncremental(root);
        root->markLayoutDirty();
        syncCssRulesProperty(c, this_val, sheet);

        return JS_UNDEFINED;
    }, "deleteRule", 1));

    // Properties
    if (auto bindings = getBindingsFromContext(ctx); bindings && bindings->dom_manager_) {
        if (auto* se = bindings->dom_manager_->getStyleEngine()) {
            const dom::Stylesheet* sheet = se->stylesheetAt(sheet_engine_index);
            syncCssRulesProperty(ctx, obj, sheet);
        }
    }

    return obj;
}

JSValue doc_getStyleSheets(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    (void)argc;
    (void)argv;

    JSValue arr = JS_NewArray(ctx);

    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || !bindings->dom_manager_) {
        return arr;
    }

    auto* se = bindings->dom_manager_->getStyleEngine();
    if (!se) {
        return arr;
    }

    const size_t count = se->stylesheetCount();
    uint32_t out_i = 0;

    // Skip UA stylesheet at index 0.
    for (size_t i = 1; i < count; ++i) {
        JSValue sheet_obj = createJSCSSStyleSheet(ctx, i);
        JS_SetPropertyUint32(ctx, arr, out_i++, sheet_obj);
    }

    return arr;
}

JSValue computed_style_getPropertyValue(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) {
        return JS_NewString(ctx, "");
    }

    JSValue element = JS_GetPropertyStr(ctx, this_val, "__element__");
    auto node = JSBindings::getNodeOpaque(ctx, element);
    JS_FreeValue(ctx, element);
    if (!node) {
        return JS_NewString(ctx, "");
    }

    const char* prop = JS_ToCString(ctx, argv[0]);
    if (!prop) {
        return JS_NewString(ctx, "");
    }

    const bool dbg = debugScriptEval();
    if (dbg) {
        DONG_LOG_INFO("[JS] computedStyle.getPropertyValue('%s')", prop);
    }

    std::string css_prop(prop);
    JS_FreeCString(ctx, prop);

    // Normalize to lowercase (CSS property names are case-insensitive)
    std::transform(css_prop.begin(), css_prop.end(), css_prop.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    std::string val = getComputedStyleValue(node->getComputedStyle(), css_prop);
    if (dbg) {
        DONG_LOG_INFO("[JS] computedStyle.getPropertyValue -> '%s'", val.c_str());
    }

    return JS_NewString(ctx, val.c_str());
}

JSValue window_getComputedStyle(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;

    if (argc < 1) {
        return JS_NULL;
    }

    const bool dbg = debugScriptEval();
    if (dbg) {
        DONG_LOG_INFO("[JS] window.getComputedStyle(...) begin");
    }

    JSValue element = JS_DupValue(ctx, argv[0]);
    auto node = JSBindings::getNodeOpaque(ctx, element);
    if (!node) {
        JS_FreeValue(ctx, element);
        return JS_NULL;
    }

    auto bindings = getBindingsFromContext(ctx);
    if (bindings && bindings->layout_engine_ && bindings->dom_manager_) {
        auto root = bindings->dom_manager_->getRoot();
        const float vw = bindings->layout_engine_->getViewportWidth();
        const float vh = bindings->layout_engine_->getViewportHeight();
        if (root && vw > 0.0f && vh > 0.0f) {
            bindings->layout_engine_->calculateLayout(root, vw, vh);
            root->clearLayoutDirtyRecursive();
        }
    }

    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "__element__", element);
    JS_SetPropertyStr(ctx, obj, "getPropertyValue",
        JS_NewCFunction(ctx, computed_style_getPropertyValue, "getPropertyValue", 1));

    JS_SetPropertyStr(ctx, obj, "display", JS_NewString(ctx, getStyleValueForJS(node, "display").c_str()));
    JS_SetPropertyStr(ctx, obj, "color", JS_NewString(ctx, getStyleValueForJS(node, "color").c_str()));
    JS_SetPropertyStr(ctx, obj, "backgroundColor", JS_NewString(ctx, getStyleValueForJS(node, "background-color").c_str()));
    JS_SetPropertyStr(ctx, obj, "padding", JS_NewString(ctx, getStyleValueForJS(node, "padding").c_str()));
    JS_SetPropertyStr(ctx, obj, "borderWidth", JS_NewString(ctx, getStyleValueForJS(node, "border-width").c_str()));

    if (dbg) {
        DONG_LOG_INFO("[JS] window.getComputedStyle(...) -> object");
    }

    return obj;
}

JSValue style_proxy_get(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc < 2) {
        return JS_UNDEFINED;
    }

    JSValueConst target = argv[0];
    JSValueConst prop = argv[1];

    if (JS_IsSymbol(prop)) {
        JSAtom atom = JS_ValueToAtom(ctx, prop);
        JSValue val = JS_GetProperty(ctx, target, atom);
        JS_FreeAtom(ctx, atom);
        return val;
    }

    const char* prop_cstr = JS_ToCString(ctx, prop);
    if (!prop_cstr) {
        return JS_UNDEFINED;
    }
    std::string prop_str(prop_cstr);
    JS_FreeCString(ctx, prop_cstr);

    if (prop_str == "__element__") {
        return JS_GetPropertyStr(ctx, target, "__element__");
    }

    JSAtom atom = JS_ValueToAtom(ctx, prop);
    JSValue stored = JS_GetProperty(ctx, target, atom);
    JS_FreeAtom(ctx, atom);
    if (!JS_IsUndefined(stored)) {
        return stored;
    }
    JS_FreeValue(ctx, stored);

    JSValue element = JS_GetPropertyStr(ctx, target, "__element__");
    if (JS_IsUndefined(element)) {
        JS_FreeValue(ctx, element);
        return JS_UNDEFINED;
    }
    auto node = JSBindings::getNodeOpaque(ctx, element);
    JS_FreeValue(ctx, element);
    if (!node) {
        return JS_UNDEFINED;
    }

    std::string css_prop = camelToCss(prop_str);
    std::string inline_value = node->getInlineStyleProperty(css_prop);
    std::string final_value = inline_value;
    if (final_value.empty()) {
        final_value = getComputedStyleValue(node->getComputedStyle(), css_prop);
    }

    if (final_value.empty()) {
        return JS_UNDEFINED;
    }
    return JS_NewString(ctx, final_value.c_str());
}

JSValue style_proxy_set(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc < 3) {
        return JS_FALSE;
    }

    JSValueConst target = argv[0];
    JSValueConst prop = argv[1];
    JSValueConst value = argv[2];

    if (JS_IsSymbol(prop)) {
        JSAtom atom = JS_ValueToAtom(ctx, prop);
        JSValue target_dup = JS_DupValue(ctx, target);
        int rc = JS_SetProperty(ctx, target_dup, atom, JS_DupValue(ctx, value));
        JS_FreeAtom(ctx, atom);
        JS_FreeValue(ctx, target_dup);
        return rc == 0 ? JS_TRUE : JS_FALSE;
    }

    const char* prop_cstr = JS_ToCString(ctx, prop);
    if (!prop_cstr) {
        return JS_FALSE;
    }
    std::string prop_str(prop_cstr);
    JS_FreeCString(ctx, prop_cstr);

    if (prop_str == "__element__") {
        JSValue target_dup = JS_DupValue(ctx, target);
        JS_SetPropertyStr(ctx, target_dup, "__element__", JS_DupValue(ctx, value));
        JS_FreeValue(ctx, target_dup);
        return JS_TRUE;
    }

    JSValue element = JS_GetPropertyStr(ctx, target, "__element__");
    if (JS_IsUndefined(element)) {
        JS_FreeValue(ctx, element);
        return JS_FALSE;
    }
    auto node = JSBindings::getNodeOpaque(ctx, element);
    JS_FreeValue(ctx, element);
    if (!node) {
        return JS_FALSE;
    }

    std::string css_prop = camelToCss(prop_str);
    std::string value_str = jsValueToString(ctx, value);
    node->setInlineStyleProperty(css_prop, value_str);
    if (std::getenv("DONG_DEBUG_QUERYSELECTOR")) {
        DONG_LOG_INFO("[style.set] %s=%s", css_prop.c_str(), value_str.c_str());
    }

    JSValue target_dup = JS_DupValue(ctx, target);
    JSAtom atom = JS_ValueToAtom(ctx, prop);
    JS_SetProperty(ctx, target_dup, atom, JS_DupValue(ctx, value));
    JS_FreeAtom(ctx, atom);
    JS_FreeValue(ctx, target_dup);

    return JS_TRUE;
}

JSValue createStyleProxy(JSContext* ctx, JSValueConst element, const dom::DOMNodePtr& node) {
    JSValue target = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, target, "__element__", JS_DupValue(ctx, element));

    // Seed common properties for easier inspection.
    if (node) {
        const auto& style = node->getComputedStyle();
        JS_SetPropertyStr(ctx, target, "display", JS_NewString(ctx, dong::dom::toString(style.display)));
        JS_SetPropertyStr(ctx, target, "color", JS_NewString(ctx, style.color.c_str()));
        JS_SetPropertyStr(ctx, target, "backgroundColor", JS_NewString(ctx, style.background_color.c_str()));
    }

    // Add CSSOM standard methods to target object
    // getPropertyValue(propertyName)
    JS_SetPropertyStr(ctx, target, "getPropertyValue", JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
        if (argc < 1) {
            return JS_UNDEFINED;
        }

        const char* prop_cstr = JS_ToCString(c, argv[0]);
        if (!prop_cstr) {
            return JS_UNDEFINED;
        }
        std::string prop_str(prop_cstr);
        JS_FreeCString(c, prop_cstr);

        JSValue element = JS_GetPropertyStr(c, this_val, "__element__");
        if (JS_IsUndefined(element)) {
            JS_FreeValue(c, element);
            return JS_NewString(c, "");
        }
        auto node = JSBindings::getNodeOpaque(c, element);
        JS_FreeValue(c, element);
        if (!node) {
            return JS_NewString(c, "");
        }

        // prop_str is already in CSS format (kebab-case)
        std::string inline_value = node->getInlineStyleProperty(prop_str);
        std::string final_value = inline_value;
        if (final_value.empty()) {
            final_value = getComputedStyleValue(node->getComputedStyle(), prop_str);
        }

        return JS_NewString(c, final_value.c_str());
    }, "getPropertyValue", 1));

    // setProperty(propertyName, value, priority?)
    JS_SetPropertyStr(ctx, target, "setProperty", JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
        if (argc < 2) {
            return JS_UNDEFINED;
        }

        const char* prop_cstr = JS_ToCString(c, argv[0]);
        if (!prop_cstr) {
            return JS_UNDEFINED;
        }
        std::string prop_str(prop_cstr);
        JS_FreeCString(c, prop_cstr);

        std::string value_str = jsValueToString(c, argv[1]);

        // Handle priority parameter (optional third parameter: "" or "important")
        if (argc >= 3) {
            const char* priority_cstr = JS_ToCString(c, argv[2]);
            if (priority_cstr && std::string(priority_cstr) == "important") {
                value_str += " !important";
            }
            if (priority_cstr) {
                JS_FreeCString(c, priority_cstr);
            }
        }

        JSValue element = JS_GetPropertyStr(c, this_val, "__element__");
        if (JS_IsUndefined(element)) {
            JS_FreeValue(c, element);
            return JS_UNDEFINED;
        }
        auto node = JSBindings::getNodeOpaque(c, element);
        JS_FreeValue(c, element);
        if (!node) {
            return JS_UNDEFINED;
        }

        // prop_str is already in CSS format (kebab-case)
        node->setInlineStyleProperty(prop_str, value_str);

        return JS_UNDEFINED;
    }, "setProperty", 3));

    // removeProperty(propertyName)
    JS_SetPropertyStr(ctx, target, "removeProperty", JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
        if (argc < 1) {
            return JS_NewString(c, "");
        }

        const char* prop_cstr = JS_ToCString(c, argv[0]);
        if (!prop_cstr) {
            return JS_NewString(c, "");
        }
        std::string prop_str(prop_cstr);
        JS_FreeCString(c, prop_cstr);

        JSValue element = JS_GetPropertyStr(c, this_val, "__element__");
        if (JS_IsUndefined(element)) {
            JS_FreeValue(c, element);
            return JS_NewString(c, "");
        }
        auto node = JSBindings::getNodeOpaque(c, element);
        JS_FreeValue(c, element);
        if (!node) {
            return JS_NewString(c, "");
        }

        // Get old value before removing
        std::string old_value = node->getInlineStyleProperty(prop_str);

        // Remove the property by setting it to empty string
        node->setInlineStyleProperty(prop_str, "");

        return JS_NewString(c, old_value.c_str());
    }, "removeProperty", 1));

    // cssText getter - returns serialized inline styles
    JSValue cssText_getter = JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
        (void)argc; (void)argv;
        JSValue element = JS_GetPropertyStr(c, this_val, "__element__");
        if (JS_IsUndefined(element)) {
            JS_FreeValue(c, element);
            return JS_NewString(c, "");
        }
        auto node = JSBindings::getNodeOpaque(c, element);
        JS_FreeValue(c, element);
        if (!node) {
            return JS_NewString(c, "");
        }

        // Build cssText from inline styles, preserving declaration order when possible
        return JS_NewString(c, node->getInlineStyleCssText().c_str());
    }, "get cssText", 0);

    // cssText setter - parses and sets inline styles
    JSValue cssText_setter = JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
        if (argc < 1) {
            return JS_UNDEFINED;
        }

        const char* css_text = JS_ToCString(c, argv[0]);
        if (!css_text) {
            return JS_UNDEFINED;
        }

        JSValue element = JS_GetPropertyStr(c, this_val, "__element__");
        if (JS_IsUndefined(element)) {
            JS_FreeCString(c, css_text);
            JS_FreeValue(c, element);
            return JS_UNDEFINED;
        }
        auto node = JSBindings::getNodeOpaque(c, element);
        JS_FreeValue(c, element);
        if (!node) {
            JS_FreeCString(c, css_text);
            return JS_UNDEFINED;
        }

        // Clear existing inline styles
        auto inline_styles = node->getInlineStyles();
        for (const auto& [prop, val] : inline_styles) {
            node->setInlineStyleProperty(prop, "");
        }

        // Parse and set new styles
        std::string text(css_text);
        size_t pos = 0;
        while (pos < text.length()) {
            // Skip whitespace and semicolons
            while (pos < text.length() && (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\n' ||
                                          text[pos] == '\r' || text[pos] == ';')) {
                pos++;
            }
            if (pos >= text.length()) break;

            // Extract property name
            size_t prop_end = text.find(':', pos);
            if (prop_end == std::string::npos) break;

            std::string prop = text.substr(pos, prop_end - pos);
            // Trim property name
            size_t prop_start = prop.find_first_not_of(" \t");
            size_t prop_trim_end = prop.find_last_not_of(" \t");
            if (prop_start == std::string::npos) {
                pos = prop_end + 1;
                continue;
            }
            prop = prop.substr(prop_start, prop_trim_end - prop_start + 1);

            pos = prop_end + 1;

            // Skip whitespace after colon
            while (pos < text.length() && (text[pos] == ' ' || text[pos] == '\t')) {
                pos++;
            }

            // Extract value up to semicolon or end
            size_t value_start = pos;
            while (pos < text.length() && text[pos] != ';') {
                if (text[pos] == '"' || text[pos] == '\'') {
                    char quote = text[pos];
                    pos++;
                    while (pos < text.length() && text[pos] != quote) {
                        if (text[pos] == '\\') pos++; // Skip escaped characters
                        pos++;
                    }
                    if (pos < text.length()) pos++; // Skip closing quote
                } else {
                    pos++;
                }
            }

            std::string val = text.substr(value_start, pos - value_start);
            // Trim value
            size_t val_trim_end = val.find_last_not_of(" \t");
            if (val_trim_end != std::string::npos) {
                val = val.substr(0, val_trim_end + 1);
            }

            // Trim !important suffix
            size_t important_pos = val.find("!important");
            if (important_pos != std::string::npos) {
                val = val.substr(0, important_pos);
                val_trim_end = val.find_last_not_of(" \t");
                if (val_trim_end != std::string::npos) {
                    val = val.substr(0, val_trim_end + 1);
                }
                val += " !important";
            }

            // Set the property
            node->setInlineStyleProperty(prop, val);

            pos++;
        }

        // Store the raw cssText string so the getter can return it in declaration order.
        node->setInlineStyleAttrString(std::string(css_text));

        JS_FreeCString(c, css_text);
        return JS_UNDEFINED;
    }, "set cssText", 1);

    // Register cssText as a getter/setter property
    {
        JSAtom cssText_atom = JS_NewAtom(ctx, "cssText");
        JS_DefinePropertyGetSet(ctx, target, cssText_atom, cssText_getter, cssText_setter,
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
        JS_FreeAtom(ctx, cssText_atom);
    }

    JSValue handler = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, handler, "get", JS_NewCFunction(ctx, style_proxy_get, "style_get", 3));
    JS_SetPropertyStr(ctx, handler, "set", JS_NewCFunction(ctx, style_proxy_set, "style_set", 4));

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue proxy_ctor = JS_GetPropertyStr(ctx, global, "Proxy");
    JS_FreeValue(ctx, global);

    if (!JS_IsFunction(ctx, proxy_ctor)) {
        if (debugScriptEval()) {
            DONG_LOG_WARN("[JS] Proxy constructor not available; returning plain style object");
        }
        JS_FreeValue(ctx, proxy_ctor);
        JS_FreeValue(ctx, handler);
        return target;
    }

    JSValue argv_local[2] = { target, handler };
    JSValue proxy = JS_CallConstructor(ctx, proxy_ctor, 2, argv_local);
    JS_FreeValue(ctx, proxy_ctor);

    if (JS_IsException(proxy)) {
        if (debugScriptEval()) {
            DONG_LOG_WARN("[JS] Proxy construction failed; returning plain style object");
        }
        JS_FreeValue(ctx, proxy);
        JS_FreeValue(ctx, handler);
        return target;
    }

    JS_FreeValue(ctx, target);
    JS_FreeValue(ctx, handler);
    return proxy;
}

} // namespace

// ============================================================
// Document API Implementation
// ============================================================

static JSValue doc_getHead(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings) return JS_NULL;

    auto dom_mgr = bindings->dom_manager_;
    if (!dom_mgr) return JS_NULL;

    // Find <head> element
    auto head_nodes = dom_mgr->getElementsByTagName("head");
    if (head_nodes.empty()) return JS_NULL;

    return bindings->createJSElement(ctx, head_nodes[0]);
}

static JSValue doc_getActiveElement(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings) return JS_NULL;

    auto focus_mgr = bindings->focus_manager_;
    if (!focus_mgr) return JS_NULL;

    auto focused = focus_mgr->getFocusedElement();
    if (!focused) return JS_NULL;

    return bindings->createJSElement(ctx, focused);
}

static JSValue doc_getElementById(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || argc < 1) return JS_NULL;

    const bool dbg = debugScriptEval();
    if (dbg) {
        DONG_LOG_INFO("[JS] document.getElementById(...) enter");
    }

    const char* id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_NULL;

    if (dbg) {
        DONG_LOG_INFO("[JS] document.getElementById('%s')", id);
    }

    auto dom_mgr = bindings->dom_manager_;
    if (!dom_mgr) {
        JS_FreeCString(ctx, id);
        return JS_NULL;
    }

    auto node = dom_mgr->getElementById(id);
    JS_FreeCString(ctx, id);

    if (dbg) {
        DONG_LOG_INFO("[JS] document.getElementById -> node=%p", (void*)node.get());
    }

    if (!node) return JS_NULL;
    return bindings->createJSElement(ctx, node);
}

static JSValue doc_getElementsByTagName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || argc < 1) return JS_NewArray(ctx);
    
    const char* tag = JS_ToCString(ctx, argv[0]);
    if (!tag) return JS_NewArray(ctx);
    
    auto dom_mgr = bindings->dom_manager_;
    JSValue arr = JS_NewArray(ctx);
    
    if (dom_mgr) {
        auto nodes = dom_mgr->getElementsByTagName(tag);
        for (size_t i = 0; i < nodes.size(); ++i) {
            JSValue elem = bindings->createJSElement(ctx, nodes[i]);
            JS_SetPropertyUint32(ctx, arr, i, elem);
        }
    }
    
    JS_FreeCString(ctx, tag);
    return arr;
}

static JSValue doc_getElementsByClassName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || argc < 1) return JS_NewArray(ctx);
    
    const char* cls = JS_ToCString(ctx, argv[0]);
    if (!cls) return JS_NewArray(ctx);
    
    auto dom_mgr = bindings->dom_manager_;
    JSValue arr = JS_NewArray(ctx);
    
    if (dom_mgr) {
        auto nodes = dom_mgr->getElementsByClassName(cls);
        for (size_t i = 0; i < nodes.size(); ++i) {
            JSValue elem = bindings->createJSElement(ctx, nodes[i]);
            JS_SetPropertyUint32(ctx, arr, i, elem);
        }
    }
    
    JS_FreeCString(ctx, cls);
    return arr;
}

static JSValue doc_querySelector(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || argc < 1) return JS_NULL;

    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NULL;

    auto dom_mgr = bindings->dom_manager_;
    JSValue result = JS_NULL;

    if (dom_mgr) {
        auto root = dom_mgr->getRoot();
        if (root) {
            auto node = root->querySelector(selector);
            if (node) {
                result = bindings->createJSElement(ctx, node);
            }
        }
    }

    JS_FreeCString(ctx, selector);
    return result;
}

static JSValue doc_querySelectorAll(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || argc < 1) return JS_NewArray(ctx);

    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NewArray(ctx);

    auto dom_mgr = bindings->dom_manager_;
    JSValue arr = JS_NewArray(ctx);

    if (dom_mgr) {
        auto root = dom_mgr->getRoot();
        if (root) {
            auto nodes = root->querySelectorAll(selector);
            if (std::getenv("DONG_DEBUG_QUERYSELECTOR")) {
                DONG_LOG_INFO("[querySelectorAll] selector='%s' -> %zu", selector, nodes.size());
            }
            for (size_t i = 0; i < nodes.size(); ++i) {
                JSValue elem = bindings->createJSElement(ctx, nodes[i]);
                JS_SetPropertyUint32(ctx, arr, static_cast<uint32_t>(i), elem);
            }
        }
    }

    JS_FreeCString(ctx, selector);
    return arr;
}

// ============================================================
// Document: elementFromPoint / hasFocus / scrollingElement
// ============================================================

namespace {

dong::dom::DOMNodePtr hitTestRecursiveForDocument(const dong::dom::DOMNodePtr& node,
                                                  dong::layout::Engine* layout_engine,
                                                  int32_t x,
                                                  int32_t y) {
    if (!node || !layout_engine) return nullptr;

    const auto* layout = layout_engine->getLayout(node);
    if (!layout) return nullptr;

    const float lx = layout->x;
    const float ly = layout->y;
    const float w = layout->width;
    const float h = layout->height;

    const bool in_bounds = (x >= lx && x <= lx + w && y >= ly && y <= ly + h);

    // Important: even if a parent box doesn't contain the point, a positioned child can.
    // So we traverse children first without an early "out of bounds" reject.
    const auto& children = node->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto child_hit = hitTestRecursiveForDocument(*it, layout_engine, x, y);
        if (child_hit) {
            return child_hit;
        }
    }

    return in_bounds ? node : nullptr;

}

} // namespace

static JSValue doc_elementFromPoint(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || !bindings->dom_manager_ || !bindings->layout_engine_) return JS_NULL;
    if (argc < 2) return JS_NULL;

    int32_t x = 0;
    int32_t y = 0;
    if (JS_ToInt32(ctx, &x, argv[0]) != 0) return JS_NULL;
    if (JS_ToInt32(ctx, &y, argv[1]) != 0) return JS_NULL;

    auto root = bindings->dom_manager_->getRoot();
    if (!root) return JS_NULL;

    // Flush layout on demand, to match browser behavior.
    // We currently can't cheaply query "any dirty node" from here, so do a conservative flush.
    const float vw = bindings->layout_engine_->getViewportWidth();
    const float vh = bindings->layout_engine_->getViewportHeight();
    if (vw > 0.0f && vh > 0.0f) {
        bindings->layout_engine_->calculateLayout(root, vw, vh);
        root->clearLayoutDirtyRecursive();
    }


    auto hit = hitTestRecursiveForDocument(root, bindings->layout_engine_, x, y);

    while (hit && hit->getType() != dong::dom::DOMNode::NodeType::ELEMENT) {
        hit = hit->getParent();
    }

    if (!hit) return JS_NULL;
    return bindings->createJSElement(ctx, hit);
}

static JSValue doc_hasFocus(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || !bindings->focus_manager_) return JS_FALSE;
    return JS_NewBool(ctx, bindings->focus_manager_->getFocusedElement() != nullptr);
}

static JSValue doc_getScrollingElement(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;

    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || !bindings->dom_manager_) return JS_NULL;

    // Return document.documentElement first so identity checks remain true.
    JSValue doc_elem = JS_GetPropertyStr(ctx, this_val, "documentElement");
    if (!JS_IsUndefined(doc_elem) && !JS_IsNull(doc_elem)) {
        return doc_elem;
    }
    JS_FreeValue(ctx, doc_elem);

    auto html_nodes = bindings->dom_manager_->getElementsByTagName("html");
    if (!html_nodes.empty() && html_nodes[0]) {
        return bindings->createJSElement(ctx, html_nodes[0]);
    }
    auto body_nodes = bindings->dom_manager_->getElementsByTagName("body");
    if (!body_nodes.empty() && body_nodes[0]) {
        return bindings->createJSElement(ctx, body_nodes[0]);
    }
    return JS_NULL;
}

// ============================================================
// 銆愮己鍙?銆慏OM 鍒涘缓 API
// ============================================================


static JSValue doc_createElement(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || argc < 1) return JS_NULL;
    
    const char* tag = JS_ToCString(ctx, argv[0]);
    if (!tag) return JS_NULL;
    
    // 鍒涘缓鏂?DOM 鑺傜偣
    auto node = std::make_shared<dong::dom::DOMNode>(dong::dom::DOMNode::NodeType::ELEMENT, tag);
    
    JS_FreeCString(ctx, tag);
    
    if (!node) return JS_NULL;

    dong::dom::StyleEngine::applyDefaultStyleForNode(node);
    
    // 杩斿洖 JS 鍏冪礌瀵硅薄
    return bindings->createJSElement(ctx, node);
}

static JSValue doc_createTextNode(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings || argc < 1) return JS_NULL;
    
    const char* text = JS_ToCString(ctx, argv[0]);
    if (!text) return JS_NULL;
    
    // 鍒涘缓鏂囨湰鑺傜偣
    auto node = std::make_shared<dong::dom::DOMNode>(dong::dom::DOMNode::NodeType::TEXT, "");
    node->setTextContent(text);
    
    JS_FreeCString(ctx, text);
    
    if (!node) return JS_NULL;
    
    return bindings->createJSElement(ctx, node);
}

static JSValue doc_createComment(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsForDoc(ctx, this_val);
    if (!bindings) return JS_NULL;

    std::string comment_text;
    if (argc >= 1) {
        const char* text = JS_ToCString(ctx, argv[0]);
        if (text) {
            comment_text = text;
            JS_FreeCString(ctx, text);
        }
    }

    auto node = std::make_shared<dong::dom::DOMNode>(dong::dom::DOMNode::NodeType::COMMENT, "");
    node->setTextContent(comment_text);

    return bindings->createJSElement(ctx, node);
}

static JSValue doc_write(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || !bindings->dom_manager_) return JS_UNDEFINED;

    std::string html;
    html.reserve(128);
    for (int i = 0; i < argc; ++i) {
        const char* part = JS_ToCString(ctx, argv[i]);
        if (!part) continue;
        html.append(part);
        JS_FreeCString(ctx, part);
    }

    if (html.empty()) return JS_UNDEFINED;

    auto script_node = bindings->getCurrentExecutingScript();
    if (script_node) {
        script_node->insertAdjacentHTML("beforebegin", html);
        script_node->markStyleDirty();
        script_node->markLayoutDirty();
        return JS_UNDEFINED;
    }

    auto body_nodes = bindings->dom_manager_->getElementsByTagName("body");
    if (!body_nodes.empty() && body_nodes[0]) {
        body_nodes[0]->insertAdjacentHTML("beforeend", html);
        body_nodes[0]->markStyleDirty();
        body_nodes[0]->markLayoutDirty();
    }

    return JS_UNDEFINED;
}

// ============================================================
// Element API Implementation
// ============================================================

static JSValue elem_getAttribute(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    
    const char* attr_name = JS_ToCString(ctx, argv[0]);
    if (!attr_name) return JS_UNDEFINED;
    
    // Get node from this object
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    JSValue result = JS_UNDEFINED;
    
    if (node && node->hasAttribute(attr_name)) {
        std::string value = node->getAttribute(attr_name);
        result = JS_NewString(ctx, value.c_str());
    }
    
    JS_FreeCString(ctx, attr_name);
    return result;
}

static JSValue elem_setAttribute(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_UNDEFINED;

    const char* attr_name = JS_ToCString(ctx, argv[0]);
    const char* attr_value = JS_ToCString(ctx, argv[1]);

    const bool dbg = debugScriptEval();
    if (dbg && attr_name) {
        DONG_LOG_INFO("[JS] element.setAttribute('%s', ...)", attr_name);
    }

    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (node && attr_name && attr_value) {
        node->setAttribute(attr_name, attr_value);
    }

    if (dbg) {
        DONG_LOG_INFO("[JS] element.setAttribute done (node=%p)", (void*)node.get());
    }

    if (attr_name) JS_FreeCString(ctx, attr_name);
    if (attr_value) JS_FreeCString(ctx, attr_value);

    return JS_UNDEFINED;
}

static JSValue elem_appendChild(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    
    auto parent = JSBindings::getNodeOpaque(ctx, this_val);
    auto child = JSBindings::getNodeOpaque(ctx, argv[0]);
    
    if (parent && child) {
        parent->appendChild(child);
        return JS_DupValue(ctx, argv[0]);
    }
    
    return JS_UNDEFINED;
}

static JSValue elem_removeChild(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    
    auto parent = JSBindings::getNodeOpaque(ctx, this_val);
    auto child = JSBindings::getNodeOpaque(ctx, argv[0]);
    
    if (parent && child) {
        parent->removeChild(child);
        return JS_DupValue(ctx, argv[0]);
    }
    
    return JS_UNDEFINED;
}

DONG_JS_STRING_GETTER(elem_getTextContent, node->getTextContent())
DONG_JS_STRING_SETTER(elem_setTextContent, setTextContent)

DONG_JS_STRING_GETTER(elem_getInnerHTML, node->getInnerHTML())
DONG_JS_STRING_SETTER(elem_setInnerHTML, setInnerHTML)

DONG_JS_STRING_GETTER(elem_getOuterHTML, node->getOuterHTML())

static JSValue elem_setOuterHTML(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;

    const char* html = JS_ToCString(ctx, argv[0]);
    auto node = JSBindings::getNodeOpaque(ctx, this_val);

    if (node && html) {
        node->setOuterHTML(html);
        // Parent should be marked dirty since this node is replaced
        if (auto parent = node->getParent()) {
            parent->markLayoutDirty();
        }
    }

    if (html) JS_FreeCString(ctx, html);
    return JS_UNDEFINED;
}

static JSValue elem_insertAdjacentHTML(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_UNDEFINED;

    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    const char* position = JS_ToCString(ctx, argv[0]);
    const char* html = JS_ToCString(ctx, argv[1]);

    if (position && html) {
        node->insertAdjacentHTML(position, html);
        node->markLayoutDirty();
    }

    if (position) JS_FreeCString(ctx, position);
    if (html) JS_FreeCString(ctx, html);
    return JS_UNDEFINED;
}

static JSValue elem_focus(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (node) {
        node->focus();
        // Match engine mouse path: mousedown on contenteditable sets last_editable_root_
        // so document.execCommand still resolves the editing host after focus moves (e.g. toolbar button).
        auto* bindings = getBindingsFromContext(ctx);
        if (bindings && node->isContentEditable() && !dong::dom::isInputElement(node)) {
            auto er = dong::dom::ContentEditableState::findEditableRoot(node);
            if (er) {
                bindings->last_editable_root_ = er;
            }
        }
    }
    return JS_UNDEFINED;
}

static JSValue elem_blur(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (node) {
        node->blur();
    }
    return JS_UNDEFINED;
}

static JSValue elem_addEventListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_UNDEFINED;

    // Expect (type, handler)
    if (!JS_IsString(argv[0]) || !JS_IsFunction(ctx, argv[1])) {
        return JS_UNDEFINED;
    }

    // Read the internal node id stored on the element wrapper
    JSValue id_val = JS_GetPropertyStr(ctx, this_val, "__node_id__");
    if (JS_IsUndefined(id_val)) {
        JS_FreeValue(ctx, id_val);
        return JS_UNDEFINED;
    }

    int64_t node_id = 0;
    if (JS_ToInt64(ctx, &node_id, id_val) != 0) {
        JS_FreeValue(ctx, id_val);
        return JS_UNDEFINED;
    }
    JS_FreeValue(ctx, id_val);

    const char* type_cstr = JS_ToCString(ctx, argv[0]);
    if (!type_cstr) {
        return JS_UNDEFINED;
    }

    auto bindings = getBindingsFromContext(ctx);
    if (bindings) {
        std::string type_str(type_cstr);
        bindings->registerEventListener(static_cast<uint64_t>(node_id), type_str, argv[1]);

        // Bridge this JS listener into the C++ DOM event system so that
        // native EventDispatcher can handle bubbling along the DOM tree.
        dom::DOMNodePtr node = JSBindings::getNodeOpaque(ctx, this_val);
        if (node) {
            bindings->ensureEventBridgeForNode(node, type_str, static_cast<uint64_t>(node_id));
        }
    }

    JS_FreeCString(ctx, type_cstr);
    return JS_UNDEFINED;
}

static JSValue elem_removeEventListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_UNDEFINED;

    // Expect (type, handler)
    if (!JS_IsString(argv[0]) || !JS_IsFunction(ctx, argv[1])) {
        return JS_UNDEFINED;
    }

    JSValue id_val = JS_GetPropertyStr(ctx, this_val, "__node_id__");
    if (JS_IsUndefined(id_val)) {
        JS_FreeValue(ctx, id_val);
        return JS_UNDEFINED;
    }

    int64_t node_id = 0;
    if (JS_ToInt64(ctx, &node_id, id_val) != 0) {
        JS_FreeValue(ctx, id_val);
        return JS_UNDEFINED;
    }
    JS_FreeValue(ctx, id_val);

    const char* type_cstr = JS_ToCString(ctx, argv[0]);
    if (!type_cstr) {
        return JS_UNDEFINED;
    }

    auto bindings = getBindingsFromContext(ctx);
    if (bindings) {
        bindings->removeEventListener(static_cast<uint64_t>(node_id), std::string(type_cstr), argv[1]);
    }

    JS_FreeCString(ctx, type_cstr);
    return JS_UNDEFINED;
}

static JSValue elem_getComputedStyle(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewObject(ctx);
    
    const auto& style = node->getComputedStyle();
    JSValue style_obj = JS_NewObject(ctx);
    
    // Expose computed style properties
    JS_SetPropertyStr(ctx, style_obj, "color", JS_NewString(ctx, style.color.c_str()));
    JS_SetPropertyStr(ctx, style_obj, "backgroundColor", JS_NewString(ctx, style.background_color.c_str()));
    JS_SetPropertyStr(ctx, style_obj, "fontSize", JS_NewFloat64(ctx, style.font_size));
    JS_SetPropertyStr(ctx, style_obj, "fontWeight", JS_NewString(ctx, dong::dom::toString(style.font_weight)));
    JS_SetPropertyStr(ctx, style_obj, "textAlign", JS_NewString(ctx, dong::dom::toString(style.text_align)));
    JS_SetPropertyStr(ctx, style_obj, "display", JS_NewString(ctx, dong::dom::toString(style.display)));
    JS_SetPropertyStr(ctx, style_obj, "position", JS_NewString(ctx, dong::dom::toString(style.position)));
    
    return style_obj;
}

DONG_JS_STRING_GETTER(elem_getTagName, node->getTagName())

static JSValue elem_getChildren(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewArray(ctx);
    
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings) return JS_NewArray(ctx);
    
    JSValue arr = JS_NewArray(ctx);
    const auto& children = node->getChildren();
    
    for (size_t i = 0; i < children.size(); ++i) {
        JSValue child = bindings->createJSElement(ctx, children[i]);
        JS_SetPropertyUint32(ctx, arr, i, child);
    }
    
    return arr;
}

// ============================================================
// HTMLMediaElement (minimal): <video> play/pause/paused/currentTime
// ============================================================

static bool video_isPlayingAttr(const dong::dom::DOMNodePtr& node) {
    if (!node) return false;
    const std::string v = node->getAttribute("__dong_video_playing");
    return v == "1" || v == "true";
}

static JSValue video_play(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    node->setAttribute("__dong_video_playing", "1");
    return JS_UNDEFINED;
}

static JSValue video_pause(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    node->setAttribute("__dong_video_playing", "0");
    return JS_UNDEFINED;
}

static JSValue video_getPaused(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_TRUE;
    const bool playing = video_isPlayingAttr(node);
    const bool ended = (node->getAttribute("__dong_video_ended") == "1");
    return JS_NewBool(ctx, (!playing) || ended);
}

static JSValue video_getEnded(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    return JS_NewBool(ctx, node->getAttribute("__dong_video_ended") == "1");
}

static JSValue video_getSeeking(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    return JS_NewBool(ctx, node->getAttribute("__dong_video_seeking") == "1");
}

static JSValue video_getCurrentTime(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0.0);
    const std::string s = node->getAttribute("__dong_video_currentTime");
    if (s.empty()) return JS_NewFloat64(ctx, 0.0);
    char* end = nullptr;
    const double v = std::strtod(s.c_str(), &end);
    if (!end || end == s.c_str()) return JS_NewFloat64(ctx, 0.0);
    return JS_NewFloat64(ctx, v);
}

static JSValue video_setCurrentTime(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    double t = 0.0;
    if (JS_ToFloat64(ctx, &t, argv[0]) != 0) {
        return JS_UNDEFINED;
    }
    if (t < 0.0) t = 0.0;

    node->setAttribute("__dong_video_seek", std::to_string(t));
    node->setAttribute("__dong_video_seeking", "1");
    node->setAttribute("__dong_video_ended", "0");
    return JS_UNDEFINED;
}

static JSValue video_getDuration(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0.0);
    const std::string s = node->getAttribute("__dong_video_duration");
    if (s.empty()) return JS_NewFloat64(ctx, 0.0);
    char* end = nullptr;
    const double v = std::strtod(s.c_str(), &end);
    if (!end || end == s.c_str()) return JS_NewFloat64(ctx, 0.0);
    return JS_NewFloat64(ctx, v);
}

// ============================================================
// HTMLDetailsElement.open property
// ============================================================

static JSValue details_getOpen(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    if (node->getTagName() != "details") return JS_FALSE;

    auto* state = dong::dom::getDetailsState(node);
    if (state) {
        return JS_NewBool(ctx, state->isOpen());
    }
    // Fallback to checking open attribute
    return JS_NewBool(ctx, node->hasAttribute("open"));
}

static JSValue details_setOpen(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    if (node->getTagName() != "details") return JS_UNDEFINED;

    bool new_open = false;
    if (JS_ToBool(ctx, argv[0]) == 1) {
        new_open = true;
    }

    auto* state = dong::dom::getDetailsState(node);
    if (state) {
        bool was_open = state->isOpen();
        if (was_open != new_open) {
            state->setOpen(new_open);
            state->applyOpenStateToDOM(node);
            // Toggle event is dispatched when open state changes
            // Note: This doesn't dispatch the event here since it's called from JS
            // The engine will handle dispatching, or we could dispatch here
        }
    } else {
        if (new_open) {
            node->setAttribute("open", "");
        } else {
            node->removeAttribute("open");
        }
    }

    return JS_UNDEFINED;
}

// ============================================================
// 样式修改 API - element.style.*
// ============================================================

// 获取样式对象（代理，允许读写）
static JSValue elem_getStyle(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewObject(ctx);
    return createStyleProxy(ctx, this_val, node);
}

static JSValue elem_setStyle(JSContext* ctx, JSValueConst this_val, JSValue style_obj, const char* prop, JSValue value) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    
    auto& style = node->getComputedStyle();
    
    std::string str_val;
    double num_val = 0;
    
    if (JS_IsString(value)) {
        const char* s = JS_ToCString(ctx, value);
        if (s) {
            str_val = s;
            JS_FreeCString(ctx, s);
        }
    } else if (JS_IsNumber(value)) {
        JS_ToFloat64(ctx, &num_val, value);
    }
    
    if (strcmp(prop, "color") == 0) style.color = str_val;
    else if (strcmp(prop, "backgroundColor") == 0) style.background_color = str_val;
    else if (strcmp(prop, "fontSize") == 0) style.font_size = (float)num_val;
    else if (strcmp(prop, "fontWeight") == 0) style.font_weight = dong::dom::fontWeightFromString(str_val);
    else if (strcmp(prop, "textAlign") == 0) style.text_align = dong::dom::textAlignFromString(str_val);
    else if (strcmp(prop, "display") == 0) {
        style.setDisplay(str_val);
    }
    else if (strcmp(prop, "position") == 0) style.position = dong::dom::positionFromString(str_val);
    else if (strcmp(prop, "opacity") == 0) style.opacity = (float)num_val;
    else if (strcmp(prop, "borderRadius") == 0) style.border_radius = (float)num_val;
    else if (strcmp(prop, "borderWidth") == 0) style.border_width = (float)num_val;
    else if (strcmp(prop, "borderColor") == 0) style.border_color = str_val;
    
    node->markLayoutDirty();
    
    return JS_UNDEFINED;
}

// className getter/setter
DONG_JS_ATTR_GETTER(elem_getClassName, "class")

static JSValue elem_setClassName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    
    const char* cls = JS_ToCString(ctx, argv[0]);
    if (cls) {
        node->setAttribute("class", cls);
        JS_FreeCString(ctx, cls);
        
        // Trigger style recomputation
        auto* bindings = getBindingsFromContext(ctx);
        if (bindings && bindings->dom_manager_) {
            bindings->dom_manager_->recomputeNodeStyle(node);
        }
    }
    return JS_UNDEFINED;
}

DONG_JS_ATTR_GETTER(elem_getId, "id")

static JSValue elem_setId(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    const char* id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_UNDEFINED;

    if (id[0]) {
        node->setAttribute("id", id);
    } else {
        node->removeAttribute("id");
    }

    JS_FreeCString(ctx, id);

    // Trigger style recomputation (#id selectors)
    auto* bindings = getBindingsFromContext(ctx);
    if (bindings && bindings->dom_manager_) {
        bindings->dom_manager_->recomputeNodeStyle(node);
    }

    return JS_UNDEFINED;
}

// classList 支持 - 从 __element__ 取节点

static JSValue elem_classList_add(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;

    JSValue elem = JS_GetPropertyStr(ctx, this_val, "__element__");
    auto node = JSBindings::getNodeOpaque(ctx, elem);
    JS_FreeValue(ctx, elem);
    if (!node) return JS_UNDEFINED;

    const char* cls = JS_ToCString(ctx, argv[0]);
    if (!cls) return JS_UNDEFINED;

    std::string classes = node->getAttribute("class");
    std::string cls_str(cls);

    bool found = false;
    size_t start = 0;
    while (start < classes.size()) {
        size_t end = classes.find(' ', start);
        if (end == std::string::npos) end = classes.size();
        if (classes.substr(start, end - start) == cls_str) {
            found = true;
            break;
        }
        start = end + 1;
    }

    if (!found) {
        if (classes.empty()) {
            classes = cls_str;
        } else {
            classes += " " + cls_str;
        }
        node->setAttribute("class", classes);
        
        // Trigger style recomputation
        auto* bindings = getBindingsFromContext(ctx);
        if (bindings && bindings->dom_manager_) {
            bindings->dom_manager_->recomputeNodeStyle(node);
        }
    }

    JS_FreeCString(ctx, cls);
    return JS_UNDEFINED;
}

static JSValue elem_classList_remove(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;

    JSValue elem = JS_GetPropertyStr(ctx, this_val, "__element__");
    auto node = JSBindings::getNodeOpaque(ctx, elem);
    JS_FreeValue(ctx, elem);
    if (!node) return JS_UNDEFINED;

    const char* cls = JS_ToCString(ctx, argv[0]);
    if (!cls) return JS_UNDEFINED;

    std::string classes = node->getAttribute("class");
    std::string cls_str(cls);

    std::string result;
    size_t start = 0;
    while (start < classes.size()) {
        size_t end = classes.find(' ', start);
        if (end == std::string::npos) end = classes.size();
        std::string token = classes.substr(start, end - start);
        if (!token.empty() && token != cls_str) {
            if (!result.empty()) result += ' ';
            result += token;
        }
        start = end + 1;
    }

    node->setAttribute("class", result);
    
    // Trigger style recomputation
    auto* bindings = getBindingsFromContext(ctx);
    if (bindings && bindings->dom_manager_) {
        bindings->dom_manager_->recomputeNodeStyle(node);
    }
    
    JS_FreeCString(ctx, cls);
    return JS_UNDEFINED;
}

static JSValue elem_classList_toggle(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;

    JSValue elem = JS_GetPropertyStr(ctx, this_val, "__element__");
    auto node = JSBindings::getNodeOpaque(ctx, elem);
    JS_FreeValue(ctx, elem);
    if (!node) return JS_UNDEFINED;

    const char* cls = JS_ToCString(ctx, argv[0]);
    if (!cls) return JS_UNDEFINED;

    std::string classes = node->getAttribute("class");
    std::string cls_str(cls);

    bool found = false;
    std::string result;
    size_t start = 0;
    while (start < classes.size()) {
        size_t end = classes.find(' ', start);
        if (end == std::string::npos) end = classes.size();
        std::string token = classes.substr(start, end - start);
        if (!token.empty()) {
            if (token == cls_str) {
                found = true;
            } else {
                if (!result.empty()) result += ' ';
                result += token;
            }
        }
        start = end + 1;
    }

    if (!found) {
        if (!result.empty()) result += ' ';
        result += cls_str;
    }

    node->setAttribute("class", result);
    
    // Trigger style recomputation
    auto* bindings = getBindingsFromContext(ctx);
    if (bindings && bindings->dom_manager_) {
        bindings->dom_manager_->recomputeNodeStyle(node);
    }
    
    JS_FreeCString(ctx, cls);
    return JS_NewBool(ctx, !found);
}

static JSValue elem_classList_contains(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;

    JSValue elem = JS_GetPropertyStr(ctx, this_val, "__element__");
    auto node = JSBindings::getNodeOpaque(ctx, elem);
    JS_FreeValue(ctx, elem);
    if (!node) return JS_FALSE;

    const char* cls = JS_ToCString(ctx, argv[0]);
    if (!cls) return JS_FALSE;

    std::string classes = node->getAttribute("class");
    std::string cls_str(cls);
    JS_FreeCString(ctx, cls);

    size_t start = 0;
    while (start < classes.size()) {
        size_t end = classes.find(' ', start);
        if (end == std::string::npos) end = classes.size();
        if (classes.substr(start, end - start) == cls_str) {
            return JS_TRUE;
        }
        start = end + 1;
    }
    return JS_FALSE;
}

static JSValue elem_getClassList(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewObject(ctx);

    JSValue classList = JS_NewObject(ctx);

    JSValue node_ref = JS_DupValue(ctx, this_val);
    JS_SetPropertyStr(ctx, classList, "__element__", node_ref);

    JS_SetPropertyStr(ctx, classList, "add",
        JS_NewCFunction(ctx, elem_classList_add, "add", 1));
    JS_SetPropertyStr(ctx, classList, "remove",
        JS_NewCFunction(ctx, elem_classList_remove, "remove", 1));
    JS_SetPropertyStr(ctx, classList, "toggle",
        JS_NewCFunction(ctx, elem_classList_toggle, "toggle", 1));
    JS_SetPropertyStr(ctx, classList, "contains",
        JS_NewCFunction(ctx, elem_classList_contains, "contains", 1));

    return classList;
}


// ============================================================
// Event API Implementation
// ============================================================

static JSValue event_preventDefault(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue prevent_default = JS_GetPropertyStr(ctx, this_val, "defaultPrevented");
    JS_SetPropertyStr(ctx, this_val, "defaultPrevented", JS_TRUE);
    JS_FreeValue(ctx, prevent_default);
    return JS_UNDEFINED;
}

static JSValue event_stopPropagation(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue cancel_bubble = JS_GetPropertyStr(ctx, this_val, "cancelBubble");
    JS_SetPropertyStr(ctx, this_val, "cancelBubble", JS_TRUE);
    JS_FreeValue(ctx, cancel_bubble);
    return JS_UNDEFINED;
}

static JSValue event_stopImmediatePropagation(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    // stopImmediatePropagation also stops propagation
    JSValue cancel_bubble = JS_GetPropertyStr(ctx, this_val, "cancelBubble");
    JS_SetPropertyStr(ctx, this_val, "cancelBubble", JS_TRUE);
    JS_SetPropertyStr(ctx, this_val, "__stoppedImmediate", JS_TRUE);
    JS_FreeValue(ctx, cancel_bubble);
    return JS_UNDEFINED;
}

static JSValue event_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;

    JSValue event = JS_NewObject(ctx);

    if (argc > 0) {
        const char* type = JS_ToCString(ctx, argv[0]);
        if (type) {
            JS_SetPropertyStr(ctx, event, "type", JS_NewString(ctx, type));
            JS_FreeCString(ctx, type);
        }
    }

    bool bubbles = false;
    bool cancelable = false;
    if (argc > 1 && JS_IsObject(argv[1])) {
        JSValue b = JS_GetPropertyStr(ctx, argv[1], "bubbles");
        if (!JS_IsUndefined(b)) {
            bubbles = JS_ToBool(ctx, b);
        }
        JS_FreeValue(ctx, b);

        JSValue c = JS_GetPropertyStr(ctx, argv[1], "cancelable");
        if (!JS_IsUndefined(c)) {
            cancelable = JS_ToBool(ctx, c);
        }
        JS_FreeValue(ctx, c);
    }

    // Event properties
    JS_SetPropertyStr(ctx, event, "bubbles", JS_NewBool(ctx, bubbles));
    JS_SetPropertyStr(ctx, event, "cancelable", JS_NewBool(ctx, cancelable));
    JS_SetPropertyStr(ctx, event, "defaultPrevented", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "cancelBubble", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "timeStamp", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "isTrusted", JS_FALSE);


    // Event methods
    JS_SetPropertyStr(ctx, event, "preventDefault",
        JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
    JS_SetPropertyStr(ctx, event, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    JS_SetPropertyStr(ctx, event, "stopImmediatePropagation",
        JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

    return event;
}


static JSValue mouse_event_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue event = event_constructor(ctx, this_val, argc, argv);
    
    // MouseEvent-specific properties
    JS_SetPropertyStr(ctx, event, "clientX", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "clientY", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "pageX", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "pageY", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "movementX", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "movementY", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "screenX", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "screenY", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "button", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "buttons", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "relatedTarget", JS_NULL);

    return event;
}

static JSValue keyboard_event_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue event = event_constructor(ctx, this_val, argc, argv);
    
    // KeyboardEvent-specific properties
    JS_SetPropertyStr(ctx, event, "key", JS_NewString(ctx, ""));
    JS_SetPropertyStr(ctx, event, "code", JS_NewString(ctx, ""));
    JS_SetPropertyStr(ctx, event, "keyCode", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "charCode", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "repeat", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "ctrlKey", JS_FALSE);

    JS_SetPropertyStr(ctx, event, "shiftKey", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "altKey", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "metaKey", JS_FALSE);
    
    return event;
}

// ============================================================
// JSBindings Implementation
// ============================================================

JSBindings::JSBindings(ScriptEngine* engine,
                       dom::Manager* dom_manager,
                       dong::layout::Engine* layout_engine,
                       dom::EventDispatcher* event_dispatcher,
                       dom::FocusManager* focus_manager)
    : engine_(engine),
      dom_manager_(dom_manager),
      layout_engine_(layout_engine),
      event_dispatcher_(event_dispatcher),
      focus_manager_(focus_manager),
      script_start_time_(std::chrono::steady_clock::now()) {
    // Set context opaque to this JSBindings instance for per-context lookup
    if (engine_ && engine_->getContext()) {
        JS_SetContextOpaque(engine_->getContext(), this);
    }
}

JSBindings::~JSBindings() {
    resetForNewDOM();
    // Clear context opaque only if it still points to us
    if (engine_ && engine_->getContext()) {
        if (JS_GetContextOpaque(engine_->getContext()) == this) {
            JS_SetContextOpaque(engine_->getContext(), nullptr);
        }
    }
    // Clear active bindings if it's us
    if (s_active_bindings == this) {
        s_active_bindings = nullptr;
    }
}

void JSBindings::initialize() {
    if (!engine_) return;

    initializeConsoleAPI();
    initializePerformanceAPI();
    initializeDocumentAPI();
    initializeElementAPI();
    initializeEventAPI();

    // Initialize CSS API: CSS.supports() and window.matchMedia()
    JSContext* ctx = engine_->getContext();
    if (ctx) {
        initializeCSSAPI(ctx);
        initializeWindowCSSAPI(ctx);
        initializeNodeConstants(ctx);
        initializeObserverAPI(ctx, this);
        initializeClipboardAPI(ctx);
        initializeSelectionAPI(ctx, this);
        initializeFetchAPI(ctx, this);

        extern void registerTextLayoutAPI(JSContext* ctx);
        registerTextLayoutAPI(ctx);

        extern void registerSceneGraphAPI(JSContext* ctx);
        registerSceneGraphAPI(ctx);
    }
}

void JSBindings::scanAndRegisterInlineEventHandlers() {
    DONG_LOG_INFO("[JSBindings] scanAndRegisterInlineEventHandlers starting");
    if (!engine_ || !dom_manager_) {
        DONG_LOG_WARN("[JSBindings] Cannot scan: engine=%p, dom_manager=%p", (void*)engine_, (void*)dom_manager_);
        return;
    }

    JSContext* ctx = engine_->getContext();
    if (!ctx) {
        DONG_LOG_WARN("[JSBindings] Cannot scan: context is null");
        return;
    }

    auto root = dom_manager_->getRoot();
    if (!root) {
        DONG_LOG_WARN("[JSBindings] Cannot scan: root is null");
        return;
    }
    DONG_LOG_INFO("[JSBindings] Starting DOM scan for inline event handlers");

    const char* event_attrs[] = {
        "onclick", "onchange", "oninput", "onfocus", "onblur",
        "onkeydown", "onkeyup", "onkeypress",
        "onmousedown", "onmouseup", "onmousemove", "onmouseover", "onmouseout",
        "onsubmit", "onload", "onerror", nullptr
    };

    // Recursively scan DOM tree for elements with inline event handlers
    int found_count = 0;
    std::function<void(const dom::DOMNodePtr&)> scanNode = [&](const dom::DOMNodePtr& node) {
        if (!node) return;
        if (node->getType() != dom::DOMNode::NodeType::ELEMENT) {
            // Skip non-element nodes but still recurse into children
            for (const auto& child : node->getChildren()) {
                scanNode(child);
            }
            return;
        }

        // Check if this element has any inline event handlers
        bool has_inline_event = false;
        std::string found_attrs;
        for (const char** attr = event_attrs; *attr; ++attr) {
            if (node->hasAttribute(*attr)) {
                has_inline_event = true;
                if (!found_attrs.empty()) found_attrs += ", ";
                found_attrs += *attr;
            }
        }

        if (has_inline_event) {
            DONG_LOG_INFO("[JSBindings] Found element <%s> with inline handlers: %s", node->getTagName().c_str(), found_attrs.c_str());
            found_count++;
            // Create JS element for this node - this will register the event handlers
            JSValue elem = createJSElement(ctx, node);
            JS_FreeValue(ctx, elem);
        }

        // Recurse into children
        for (const auto& child : node->getChildren()) {
            scanNode(child);
        }
    };

    scanNode(root);
    DONG_LOG_INFO("[JSBindings] DOM scan completed, found %d elements with inline event handlers", found_count);
}

// ============================================================
// Timer API: setTimeout / clearTimeout / setInterval / clearInterval
// ============================================================

static JSValue js_setTimeout(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_NewInt32(ctx, 0);

    if (!JS_IsFunction(ctx, argv[0])) return JS_NewInt32(ctx, 0);

    double delay_ms = 0.0;
    if (argc >= 2) JS_ToFloat64(ctx, &delay_ms, argv[1]);
    if (delay_ms < 0.0) delay_ms = 0.0;

    JSBindings::TimerEntry entry;
    entry.id = bindings->next_timer_id_++;
    entry.interval = -1.0; // one-shot
    entry.callback = JS_DupValue(ctx, argv[0]);
    entry.cancelled = false;

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        now - bindings->script_start_time_).count() / 1e6;
    entry.fire_at = elapsed + delay_ms / 1000.0;

    uint32_t id = entry.id;
    bindings->timers_[id] = std::move(entry);
    return JS_NewInt32(ctx, static_cast<int32_t>(id));
}

static JSValue js_clearTimeout(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_UNDEFINED;

    int32_t id = 0;
    JS_ToInt32(ctx, &id, argv[0]);
    auto it = bindings->timers_.find(static_cast<uint32_t>(id));
    if (it != bindings->timers_.end()) {
        it->second.cancelled = true;
    }
    return JS_UNDEFINED;
}

static JSValue js_setInterval(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_NewInt32(ctx, 0);

    if (!JS_IsFunction(ctx, argv[0])) return JS_NewInt32(ctx, 0);

    double interval_ms = 0.0;
    if (argc >= 2) JS_ToFloat64(ctx, &interval_ms, argv[1]);
    if (interval_ms < 4.0) interval_ms = 4.0; // match browser minimum

    JSBindings::TimerEntry entry;
    entry.id = bindings->next_timer_id_++;
    entry.interval = interval_ms / 1000.0;
    entry.callback = JS_DupValue(ctx, argv[0]);
    entry.cancelled = false;

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        now - bindings->script_start_time_).count() / 1e6;
    entry.fire_at = elapsed + entry.interval;

    uint32_t id = entry.id;
    bindings->timers_[id] = std::move(entry);
    return JS_NewInt32(ctx, static_cast<int32_t>(id));
}

static JSValue js_clearInterval(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    return js_clearTimeout(ctx, this_val, argc, argv);
}

// ============================================================
// requestAnimationFrame / cancelAnimationFrame
// ============================================================

static JSValue js_requestAnimationFrame(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_NewInt32(ctx, 0);
    if (!JS_IsFunction(ctx, argv[0])) return JS_NewInt32(ctx, 0);

    JSBindings::RAFEntry entry;
    entry.id = bindings->next_raf_id_++;
    entry.callback = JS_DupValue(ctx, argv[0]);
    bindings->raf_callbacks_.push_back(std::move(entry));
    return JS_NewInt32(ctx, static_cast<int32_t>(entry.id));
}

static JSValue js_cancelAnimationFrame(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_UNDEFINED;

    int32_t id = 0;
    JS_ToInt32(ctx, &id, argv[0]);
    auto& cbs = bindings->raf_callbacks_;
    for (auto it = cbs.begin(); it != cbs.end(); ++it) {
        if (it->id == static_cast<uint32_t>(id)) {
            JS_FreeValue(ctx, it->callback);
            cbs.erase(it);
            break;
        }
    }
    return JS_UNDEFINED;
}

void JSBindings::tickAnimationFrames(double timestamp_ms) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    if (raf_callbacks_.empty()) return;

    auto callbacks = std::move(raf_callbacks_);
    raf_callbacks_.clear();

    for (auto& entry : callbacks) {
        JSValue ts = JS_NewFloat64(ctx, timestamp_ms);
        JSValue result = JS_Call(ctx, entry.callback, JS_UNDEFINED, 1, &ts);
        JS_FreeValue(ctx, ts);
        if (JS_IsException(result)) {
            JSValue ex = JS_GetException(ctx);
            auto* err_str = JS_ToCString(ctx, ex);
            DONG_LOG_ERROR("[rAF] Exception in animation frame callback: %s", err_str ? err_str : "(unknown)");
            if (err_str) JS_FreeCString(ctx, err_str);
            JS_FreeValue(ctx, ex);
        }
        JS_FreeValue(ctx, result);
        JS_FreeValue(ctx, entry.callback);
    }
}

void JSBindings::initializeConsoleAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    // Register FormData class (needs JSRuntime, done once here)
    if (g_formdata_class_id == 0) {
        JS_NewClassID(&g_formdata_class_id);
        JS_NewClass(JS_GetRuntime(ctx), g_formdata_class_id, &g_formdata_class_def);
    }

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue console = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, global, "window", JS_DupValue(ctx, global));
    JS_SetPropertyStr(ctx, global, "self", JS_DupValue(ctx, global));

    JS_SetPropertyStr(ctx, console, "log",
        JS_NewCFunction(ctx, console_log, "log", 1));
    JS_SetPropertyStr(ctx, console, "warn",
        JS_NewCFunction(ctx, console_warn, "warn", 1));
    JS_SetPropertyStr(ctx, console, "error",
        JS_NewCFunction(ctx, console_error, "error", 1));

    // JS_SetPropertyStr takes ownership of 'console', so we must NOT free it afterwards.
    JS_SetPropertyStr(ctx, global, "console", console);

    // Window-level browser API stubs
    JS_SetPropertyStr(ctx, global, "alert",
        JS_NewCFunction(ctx, js_alert, "alert", 1));
    JS_SetPropertyStr(ctx, global, "confirm",
        JS_NewCFunction(ctx, js_confirm, "confirm", 1));
    JS_SetPropertyStr(ctx, global, "prompt",
        JS_NewCFunction(ctx, js_prompt, "prompt", 2));

    // structuredClone global function
    JS_SetPropertyStr(ctx, global, "structuredClone",
        JS_NewCFunction(ctx, js_structuredClone, "structuredClone", 1));

    // Timer API
    JS_SetPropertyStr(ctx, global, "setTimeout",
        JS_NewCFunction(ctx, js_setTimeout, "setTimeout", 2));
    JS_SetPropertyStr(ctx, global, "clearTimeout",
        JS_NewCFunction(ctx, js_clearTimeout, "clearTimeout", 1));
    JS_SetPropertyStr(ctx, global, "setInterval",
        JS_NewCFunction(ctx, js_setInterval, "setInterval", 2));
    JS_SetPropertyStr(ctx, global, "clearInterval",
        JS_NewCFunction(ctx, js_clearInterval, "clearInterval", 1));

    // requestAnimationFrame / cancelAnimationFrame
    JS_SetPropertyStr(ctx, global, "requestAnimationFrame",
        JS_NewCFunction(ctx, js_requestAnimationFrame, "requestAnimationFrame", 1));
    JS_SetPropertyStr(ctx, global, "cancelAnimationFrame",
        JS_NewCFunction(ctx, js_cancelAnimationFrame, "cancelAnimationFrame", 1));

    // window.addEventListener / removeEventListener (delegates to document/body)
    JS_SetPropertyStr(ctx, global, "addEventListener",
        JS_NewCFunction(ctx, elem_addEventListener, "addEventListener", 2));
    JS_SetPropertyStr(ctx, global, "removeEventListener",
        JS_NewCFunction(ctx, elem_removeEventListener, "removeEventListener", 2));

    // DOMRect global constructor
    JS_SetPropertyStr(ctx, global, "DOMRect",
        JS_NewCFunction2(ctx, domrect_constructor, "DOMRect", 4, JS_CFUNC_constructor, 0));

    // FormData global constructor (accepts optional form argument)
    JS_SetPropertyStr(ctx, global, "FormData",
        JS_NewCFunction2(ctx, formdata_constructor, "FormData", 1, JS_CFUNC_constructor, 0));

    // DOMParser global constructor
    JS_SetPropertyStr(ctx, global, "DOMParser",
        JS_NewCFunction2(ctx, domparser_constructor, "DOMParser", 0, JS_CFUNC_constructor, 0));

    JS_FreeValue(ctx, global);
}

// ============================================================
// Performance API
// ============================================================

static JSValue performance_now(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val; (void)argc; (void)argv;

    auto bindings = getBindingsFromContext(ctx);
    if (!bindings) return JS_NewFloat64(ctx, 0.0);

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        now - bindings->script_start_time_);

    // Convert to milliseconds with fractional precision
    double ms = duration.count() / 1000.0;

    return JS_NewFloat64(ctx, ms);
}

// ============================================================
// CSSOM API: CSS.supports() and window.matchMedia()
// ============================================================

// List of CSS properties known to be supported by Dong
static bool isCssPropertySupported(const std::string& property) {
    static const std::unordered_set<std::string> supported_properties = {
        // Display
        "display", "visibility", "opacity",

        // Box model
        "width", "height", "min-width", "max-width", "min-height", "max-height",
        "margin", "margin-top", "margin-right", "margin-bottom", "margin-left",
        "padding", "padding-top", "padding-right", "padding-bottom", "padding-left",
        "border", "border-width", "border-color", "border-style", "border-radius",
        "border-top", "border-right", "border-bottom", "border-left",
        "border-top-width", "border-right-width", "border-bottom-width", "border-left-width",
        "border-top-color", "border-right-color", "border-bottom-color", "border-left-color",
        "border-top-style", "border-right-style", "border-bottom-style", "border-left-style",
        "box-sizing", "outline", "outline-width", "outline-color", "outline-style", "outline-offset",

        // Logical properties
        "margin-inline", "margin-inline-start", "margin-inline-end",
        "margin-block", "margin-block-start", "margin-block-end",
        "padding-inline", "padding-inline-start", "padding-inline-end",
        "padding-block", "padding-block-start", "padding-block-end",
        "border-inline", "border-inline-start", "border-inline-end",
        "border-block", "border-block-start", "border-block-end",
        "inset-inline", "inset-inline-start", "inset-inline-end",
        "inset-block", "inset-block-start", "inset-block-end",

        // Positioning
        "position", "top", "right", "bottom", "left",
        "z-index", "overflow", "overflow-x", "overflow-y",
        "float", "clear",

        // Flexbox
        "flex", "flex-direction", "flex-wrap", "flex-flow",
        "justify-content", "align-items", "align-content", "align-self", "place-items", "place-content", "place-self",
        "flex-grow", "flex-shrink", "flex-basis", "order",
        "gap", "row-gap", "column-gap",

        // Typography
        "font", "font-family", "font-size", "font-weight", "font-style",
        "font-variant", "line-height", "letter-spacing", "word-spacing",
        "text-align", "text-decoration", "text-transform", "text-overflow",
        "white-space", "word-break", "text-indent",

        // Color
        "color", "background", "background-color", "background-image",
        "background-repeat", "background-position", "background-size",
        "background-attachment", "background-clip", "background-origin",

        // Lists
        "list-style", "list-style-type", "list-style-position", "list-style-image",

        // Tables
        "border-collapse", "border-spacing", "caption-side",

        // Cursor
        "cursor", "pointer-events", "user-select",

        // Tables
        "table-layout",

        // Misc
        "aspect-ratio", "content-visibility", "contain",
        "color-scheme", "accent-color", "hyphens",
        "counter-reset", "counter-increment",
        "quotes", "image-rendering", "resize", "will-change", "writing-mode"
    };

    // Normalize property name (lowercase, without vendor prefix check)
    std::string prop = property;
    std::transform(prop.begin(), prop.end(), prop.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    return supported_properties.count(prop) > 0;
}

static JSValue css_supports(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;

    if (argc < 1) {
        return JS_FALSE;
    }

    // Overload 1: CSS.supports(property, value)
    if (argc >= 2 && !JS_IsUndefined(argv[1])) {
        const char* property = JS_ToCString(ctx, argv[0]);
        const char* value = JS_ToCString(ctx, argv[1]);

        bool supported = false;
        if (property && value) {
            // Check if property is supported
            if (isCssPropertySupported(property)) {
                // For now, assume all basic values are supported for supported properties
                // This is a simplified check - a production implementation would validate
                // the specific value syntax for each property
                supported = true;

                // Additional checks for specific value types
                std::string val_str(value);
                std::transform(val_str.begin(), val_str.end(), val_str.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });

                // Reject obviously invalid/unsupported values
                if (val_str.find("grid") != std::string::npos) {
                    supported = false;  // Grid layout not supported
                }
                if (val_str.find("content-visibility: auto") != std::string::npos ||
                    val_str.find("content-visibility:hidden") != std::string::npos) {
                    supported = false;  // Content visibility not fully supported
                }
            }
        }

        if (property) JS_FreeCString(ctx, property);
        if (value) JS_FreeCString(ctx, value);

        return JS_NewBool(ctx, supported);
    }

    // Overload 2: CSS.supports(conditionText)
    // Example: CSS.supports("(display: grid)")
    const char* condition_text = JS_ToCString(ctx, argv[0]);
    bool supported = false;

    if (condition_text) {
        std::string cond(condition_text);
        std::transform(cond.begin(), cond.end(), cond.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        // Check for (property: value) pattern
        size_t paren_start = cond.find('(');
        size_t paren_end = cond.rfind(')');
        size_t colon = cond.find(':');

        if (paren_start != std::string::npos && paren_end != std::string::npos &&
            colon > paren_start && colon < paren_end) {

            std::string prop = cond.substr(paren_start + 1, colon - paren_start - 1);
            std::string val = cond.substr(colon + 1, paren_end - colon - 1);

            // Trim whitespace
            auto trim = [](std::string s) {
                size_t start = s.find_first_not_of(" \t");
                size_t end = s.find_last_not_of(" \t");
                return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
            };
            prop = trim(prop);
            val = trim(val);

            supported = isCssPropertySupported(prop);

            // Check if value is valid
            if (supported && !val.empty()) {
                std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });

                // Reject obviously invalid/unsupported values
                if (val.find("grid") != std::string::npos) {
                    supported = false;
                }
            }
        }

        JS_FreeCString(ctx, condition_text);
    }

    return JS_NewBool(ctx, supported);
}

JSValue window_matchMedia(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;

    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "matchMedia requires a query parameter");
    }

    const char* query = JS_ToCString(ctx, argv[0]);
    if (!query) {
        return JS_ThrowTypeError(ctx, "matchMedia requires a query parameter");
    }

    auto bindings = getBindingsFromContext(ctx);
    bool matches = false;

    if (bindings && bindings->dom_manager_) {
        std::string query_str(query);
        std::transform(query_str.begin(), query_str.end(), query_str.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        // Note: In a production implementation, you'd need:
        // 1. A way to access the view width/height
        // 2. System color scheme detection
        // For now, we provide basic support:

        // Complex queries are not supported yet.
        if (query_str.find(',') != std::string::npos ||
            query_str.find(" and ") != std::string::npos ||
            query_str.find(" or ") != std::string::npos) {
            matches = false;
        }
        // (prefers-color-scheme: light) - assume true
        else if (query_str.find("(prefers-color-scheme: light)") != std::string::npos) {
            matches = true;
        }
        // (prefers-color-scheme: dark) - assume false for now
        else if (query_str.find("(prefers-color-scheme: dark)") != std::string::npos) {
            matches = false;
        }
        // (prefers-reduced-motion: reduce) - assume false
        else if (query_str.find("(prefers-reduced-motion: reduce)") != std::string::npos) {
            matches = false;
        }
        // (orientation: portrait) - height >= width
        else if (query_str.find("(orientation: portrait)") != std::string::npos) {
            float vw = bindings->layout_engine_ ? bindings->layout_engine_->getViewportWidth() : 800.0f;
            float vh = bindings->layout_engine_ ? bindings->layout_engine_->getViewportHeight() : 600.0f;
            matches = (vh >= vw);
        }
        // (orientation: landscape) - width > height
        else if (query_str.find("(orientation: landscape)") != std::string::npos) {
            float vw = bindings->layout_engine_ ? bindings->layout_engine_->getViewportWidth() : 800.0f;
            float vh = bindings->layout_engine_ ? bindings->layout_engine_->getViewportHeight() : 600.0f;
            matches = (vw > vh);
        }
        // (min-width: <value>) - parse and check if matches default width
        else if (query_str.find("(min-width:") != std::string::npos) {
            // Extract width value (simplified - assumes px units)
            size_t width_start = query_str.find("(min-width:") + 11;
            size_t width_end = query_str.find(')', width_start);
            if (width_end != std::string::npos) {
                std::string width_str = query_str.substr(width_start, width_end - width_start);
                // Basic parsing for "100px" format
                size_t px_pos = width_str.find("px");
                if (px_pos != std::string::npos) {
                    width_str = width_str.substr(0, px_pos);
                }
                int min_width = std::stoi(width_str);
                // Assume default viewport width of 800px
                matches = (800 >= min_width);
            }
        }
        // (max-width: <value>) - parse and check if matches default width
        else if (query_str.find("(max-width:") != std::string::npos) {
            // Extract width value
            size_t width_start = query_str.find("(max-width:") + 11;
            size_t width_end = query_str.find(')', width_start);
            if (width_end != std::string::npos) {
                std::string width_str = query_str.substr(width_start, width_end - width_start);
                size_t px_pos = width_str.find("px");
                if (px_pos != std::string::npos) {
                    width_str = width_str.substr(0, px_pos);
                }
                int max_width = std::stoi(width_str);
                // Assume default viewport width of 800px
                matches = (800 <= max_width);
            }
        }
        // (min-height: <value>) - parse and check if matches default height
        else if (query_str.find("(min-height:") != std::string::npos) {
            size_t height_start = query_str.find("(min-height:") + 12;
            size_t height_end = query_str.find(')', height_start);
            if (height_end != std::string::npos) {
                std::string height_str = query_str.substr(height_start, height_end - height_start);
                size_t px_pos = height_str.find("px");
                if (px_pos != std::string::npos) {
                    height_str = height_str.substr(0, px_pos);
                }
                int min_height = std::stoi(height_str);
                // Assume default viewport height of 600px
                matches = (600 >= min_height);
            }
        }
        // (max-height: <value>) - parse and check if matches default height
        else if (query_str.find("(max-height:") != std::string::npos) {
            size_t height_start = query_str.find("(max-height:") + 12;
            size_t height_end = query_str.find(')', height_start);
            if (height_end != std::string::npos) {
                std::string height_str = query_str.substr(height_start, height_end - height_start);
                size_t px_pos = height_str.find("px");
                if (px_pos != std::string::npos) {
                    height_str = height_str.substr(0, px_pos);
                }
                int max_height = std::stoi(height_str);
                // Assume default viewport height of 600px
                matches = (600 <= max_height);
            }
        }
        // Default: unsupported queries return false
    }

    // Create MediaQueryList object with matches property
    JSValue mql = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, mql, "media", JS_NewString(ctx, query));
    JS_SetPropertyStr(ctx, mql, "matches", JS_NewBool(ctx, matches));

    // Store the query for potential future updates
    JS_SetPropertyStr(ctx, mql, "__query__", JS_NewString(ctx, query));

    // Add onchange property (for future implementation of change events)
    JS_SetPropertyStr(ctx, mql, "onchange", JS_UNDEFINED);

    // Add addEventListener and removeEventListener methods
    JS_SetPropertyStr(ctx, mql, "addEventListener", JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* a_argv) {
        (void)this_val; (void)argc; (void)a_argv;
        // For now, just log that listener was added (real change detection requires viewport tracking)
        return JS_UNDEFINED;
    }, "addEventListener", 2));

    JS_SetPropertyStr(ctx, mql, "removeEventListener", JS_NewCFunction(ctx, [](JSContext* c, JSValueConst this_val, int argc, JSValueConst* a_argv) {
        (void)this_val; (void)argc; (void)a_argv;
        return JS_UNDEFINED;
    }, "removeEventListener", 2));

    JS_FreeCString(ctx, query);

    return mql;
}

void initializeCSSAPI(JSContext* ctx) {
    if (!ctx) return;

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue css = JS_NewObject(ctx);

    // CSS.supports(property, value)
    JS_SetPropertyStr(ctx, css, "supports",
        JS_NewCFunction(ctx, css_supports, "supports", 1));

    JS_SetPropertyStr(ctx, global, "CSS", css);
    JS_FreeValue(ctx, global);
}

void initializeWindowCSSAPI(JSContext* ctx) {
    if (!ctx) return;

    JSValue global = JS_GetGlobalObject(ctx);

    // window.matchMedia(query)
    JS_SetPropertyStr(ctx, global, "matchMedia",
        JS_NewCFunction(ctx, window_matchMedia, "matchMedia", 1));

    JS_FreeValue(ctx, global);
}

void JSBindings::initializePerformanceAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue performance = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, performance, "now",
        JS_NewCFunction(ctx, performance_now, "now", 0));

    JS_SetPropertyStr(ctx, global, "performance", performance);
    JS_FreeValue(ctx, global);
}

// Shared static C function for document.execCommand binding
static JSValue js_doc_execCommand(JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;
    const char* cmd = JS_ToCString(c, argv[0]);
    if (!cmd) return JS_FALSE;
    std::string command(cmd);
    JS_FreeCString(c, cmd);

    std::string value;
    if (argc >= 3) {
        const char* val = JS_ToCString(c, argv[2]);
        if (val) {
            value = val;
            JS_FreeCString(c, val);
        }
    }

    auto* bindings = static_cast<dong::script::JSBindings*>(JS_GetContextOpaque(c));
    if (!bindings) { DONG_LOG_WARN("[JS] execCommand(%s): no bindings", command.c_str()); return JS_FALSE; }

    auto* fm = bindings->focus_manager_;
    if (!fm) { DONG_LOG_WARN("[JS] execCommand(%s): no focus_manager", command.c_str()); return JS_FALSE; }
    auto focused = fm->getFocusedElement();

    DONG_LOG_WARN("[JS] execCommand(%s): focused=%s ce=%d",
                  command.c_str(),
                  focused ? focused->getTagName().c_str() : "null",
                  focused ? (int)focused->isContentEditable() : -1);

    dong::dom::DOMNodePtr editable_root;
    if (focused && focused->isContentEditable()) {
        editable_root = dong::dom::ContentEditableState::findEditableRoot(focused);
    }
    if (!editable_root && bindings->last_editable_root_) {
        editable_root = bindings->last_editable_root_;
    }
    if (!editable_root) { DONG_LOG_WARN("[JS] execCommand(%s): NO editable_root", command.c_str()); return JS_FALSE; }

    if (!bindings->selection_) return JS_FALSE;
    dong::dom::Selection& sel = *bindings->selection_;
    DONG_LOG_WARN("[JS] execCommand(%s): editable_root=%s sel.rangeCount=%u sel.collapsed=%d",
            command.c_str(), editable_root->getTagName().c_str(),
            sel.getRangeCount(), (int)sel.isCollapsed());
    bool result = dong::dom::execCommand(editable_root, sel, command, value);
    DONG_LOG_WARN("[JS] execCommand(%s): result=%d", command.c_str(), (int)result);
    return JS_NewBool(c, result);
}

// Shared static C function for document.queryCommandSupported binding
static JSValue js_doc_queryCommandSupported(JSContext* c, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;
    const char* cmd = JS_ToCString(c, argv[0]);
    if (!cmd) return JS_FALSE;
    std::string command(cmd);
    JS_FreeCString(c, cmd);
    return JS_NewBool(c, dong::dom::queryCommandSupported(command));
}
void JSBindings::initializeDocumentAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue document = JS_NewObject(ctx);

    // window.getComputedStyle
    JS_SetPropertyStr(ctx, global, "getComputedStyle",
        JS_NewCFunction(ctx, window_getComputedStyle, "getComputedStyle", 1));

    // Document methods
    JS_SetPropertyStr(ctx, document, "getElementById",
        JS_NewCFunction(ctx, doc_getElementById, "getElementById", 1));
    JS_SetPropertyStr(ctx, document, "getElementsByTagName",
        JS_NewCFunction(ctx, doc_getElementsByTagName, "getElementsByTagName", 1));
    JS_SetPropertyStr(ctx, document, "getElementsByClassName",
        JS_NewCFunction(ctx, doc_getElementsByClassName, "getElementsByClassName", 1));
    JS_SetPropertyStr(ctx, document, "querySelector",
        JS_NewCFunction(ctx, doc_querySelector, "querySelector", 1));
    JS_SetPropertyStr(ctx, document, "querySelectorAll",
        JS_NewCFunction(ctx, doc_querySelectorAll, "querySelectorAll", 1));
    JS_SetPropertyStr(ctx, document, "write",
        JS_NewCFunction(ctx, doc_write, "write", 1));

    // elementFromPoint / hasFocus / scrollingElement
    JS_SetPropertyStr(ctx, document, "elementFromPoint",
        JS_NewCFunction(ctx, doc_elementFromPoint, "elementFromPoint", 2));
    JS_SetPropertyStr(ctx, document, "hasFocus",
        JS_NewCFunction(ctx, doc_hasFocus, "hasFocus", 0));

    JSAtom scrolling_atom = JS_NewAtom(ctx, "scrollingElement");
    JSValue scrolling_getter = JS_NewCFunction(ctx, doc_getScrollingElement, "get scrollingElement", 0);
    JS_DefinePropertyGetSet(ctx, document, scrolling_atom, scrolling_getter, JS_UNDEFINED,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, scrolling_atom);

    // DOM 鍒涘缓 API
    JS_SetPropertyStr(ctx, document, "createElement",
        JS_NewCFunction(ctx, doc_createElement, "createElement", 1));
    JS_SetPropertyStr(ctx, document, "createTextNode",
        JS_NewCFunction(ctx, doc_createTextNode, "createTextNode", 1));
    JS_SetPropertyStr(ctx, document, "createComment",
        JS_NewCFunction(ctx, doc_createComment, "createComment", 1));

    // Document object references
    JSValue body = JS_NewObject(ctx);
    if (dom_manager_) {
        auto root = dom_manager_->getRoot();
        if (root) {
            auto body_nodes = dom_manager_->getElementsByTagName("body");
            if (!body_nodes.empty()) {
                body = createJSElement(ctx, body_nodes[0]);
            }
        }
    }
    JS_SetPropertyStr(ctx, document, "body", body);

    JSValue html = JS_NewObject(ctx);
    if (dom_manager_) {
        auto html_nodes = dom_manager_->getElementsByTagName("html");
        if (!html_nodes.empty()) {
            html = createJSElement(ctx, html_nodes[0]);
        }
    }
    JS_SetPropertyStr(ctx, document, "documentElement", html);

    // Document.head property (getter-based)
    JSAtom head_atom = JS_NewAtom(ctx, "head");
    JSValue head_getter = JS_NewCFunction(ctx, doc_getHead, "get head", 0);
    JS_DefinePropertyGetSet(ctx, document, head_atom, head_getter, JS_UNDEFINED,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, head_atom);

    // Document.activeElement property (getter-based)
    JSAtom active_atom = JS_NewAtom(ctx, "activeElement");
    JSValue active_getter = JS_NewCFunction(ctx, doc_getActiveElement, "get activeElement", 0);
    JS_DefinePropertyGetSet(ctx, document, active_atom, active_getter, JS_UNDEFINED,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, active_atom);

    // CSSOM: document.styleSheets
    JS_SetPropertyStr(ctx, document, "styleSheets", doc_getStyleSheets(ctx, document, 0, nullptr));

    // document.addEventListener / removeEventListener
    // Delegate to body node for event handling
    JS_SetPropertyStr(ctx, document, "addEventListener",
        JS_NewCFunction(ctx, elem_addEventListener, "addEventListener", 2));
    JS_SetPropertyStr(ctx, document, "removeEventListener",
        JS_NewCFunction(ctx, elem_removeEventListener, "removeEventListener", 2));

    // document.execCommand(command, showUI, value)
    JS_SetPropertyStr(ctx, document, "execCommand",
        JS_NewCFunction(ctx, js_doc_execCommand, "execCommand", 3));

    // document.queryCommandSupported(command)
    JS_SetPropertyStr(ctx, document, "queryCommandSupported",
        JS_NewCFunction(ctx, js_doc_queryCommandSupported, "queryCommandSupported", 1));

    // Give document a __node_id__ if body exists, so event listeners can be registered
    if (dom_manager_) {
        auto body_nodes = dom_manager_->getElementsByTagName("body");
        if (!body_nodes.empty()) {
            setNodeOpaque(ctx, document, body_nodes[0]);
            // Also set on global (window) so window.addEventListener works
            setNodeOpaque(ctx, global, body_nodes[0]);
        }
    }

    // JS_SetPropertyStr takes ownership of 'document'
    JS_SetPropertyStr(ctx, global, "document", document);

    // window = globalThis (standard Web compat)
    JS_SetPropertyStr(ctx, global, "window", JS_DupValue(ctx, global));

    // document.defaultView = window (= globalThis)
    JS_SetPropertyStr(ctx, document, "defaultView", JS_DupValue(ctx, global));

    // Initialize dong.views namespace + dong.getView()
    ensureDongViewsObject(ctx);

    // If this view has a name, also register it on dong.views
    if (!view_name_.empty()) {
        JSValue dong_obj = JS_GetPropertyStr(ctx, global, "dong");
        JSValue views = JS_GetPropertyStr(ctx, dong_obj, "views");
        JS_SetPropertyStr(ctx, views, view_name_.c_str(), JS_DupValue(ctx, global));
        JS_FreeValue(ctx, views);
        JS_FreeValue(ctx, dong_obj);
    }

    JS_FreeValue(ctx, global);
}

void JSBindings::initializeElementAPI() {
    // Temporarily disabled to debug crash; element methods are installed per-instance in createJSElement.
    return;
}

void JSBindings::initializeEventAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    
    JSValue global = JS_GetGlobalObject(ctx);

    // Event constructors
    JSValue event_ctor = JS_NewCFunction2(ctx, event_constructor, "Event", 1, JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Event", event_ctor);

    JSValue mouse_event_ctor = JS_NewCFunction2(ctx, mouse_event_constructor, "MouseEvent", 1, JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "MouseEvent", mouse_event_ctor);

    JSValue keyboard_event_ctor = JS_NewCFunction2(ctx, keyboard_event_constructor, "KeyboardEvent", 1, JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "KeyboardEvent", keyboard_event_ctor);


    JS_FreeValue(ctx, global);
}

JSValue JSBindings::createJSElement(JSContext* ctx, const dom::DOMNodePtr& node) {
    if (!node) return JS_NULL;

    const bool dbg = debugScriptEval();
    if (dbg) {
        DONG_LOG_INFO("[JS] createJSElement begin node=%p tag=%s", (void*)node.get(), node->getTagName().c_str());
    }

    // Check if this node already has a JS wrapper (to avoid duplicate inline handler registration)
    auto bindings = getBindingsFromContext(ctx);
    bool already_has_wrapper = false;
    if (bindings) {
        auto it = bindings->node_to_id_.find(node.get());
        if (it != bindings->node_to_id_.end()) {
            already_has_wrapper = true;
        }
    }

    JSValue elem = JS_NewObject(ctx);
    
    // Store DOM node pointer
    setNodeOpaque(ctx, elem, node);
    
    // Expose properties and methods
    // Properties
    JS_SetPropertyStr(ctx, elem, "tagName", JS_NewString(ctx, node->getTagName().c_str()));

    // id - use getter/setter for dynamic access
    JSAtom id_atom = JS_NewAtom(ctx, "id");
    JSValue id_getter = JS_NewCFunction(ctx, elem_getId, "get id", 0);
    JSValue id_setter = JS_NewCFunction(ctx, elem_setId, "set id", 1);
    JS_DefinePropertyGetSet(ctx, elem, id_atom, id_getter, id_setter,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, id_atom);

    // className - use getter/setter for dynamic access

    JSAtom className_atom = JS_NewAtom(ctx, "className");
    JSValue getter = JS_NewCFunction(ctx, elem_getClassName, "get className", 0);
    JSValue setter = JS_NewCFunction(ctx, elem_setClassName, "set className", 1);
    JS_DefinePropertyGetSet(ctx, elem, className_atom, getter, setter,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, className_atom);
    
    // textContent - use getter/setter for dynamic access
    JSAtom textContent_atom = JS_NewAtom(ctx, "textContent");
    JSValue tc_getter = JS_NewCFunction(ctx, elem_getTextContent, "get textContent", 0);
    JSValue tc_setter = JS_NewCFunction(ctx, elem_setTextContent, "set textContent", 1);
    JS_DefinePropertyGetSet(ctx, elem, textContent_atom, tc_getter, tc_setter,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, textContent_atom);

    // innerHTML - use getter/setter for dynamic access
    JSAtom innerHTML_atom = JS_NewAtom(ctx, "innerHTML");
    JSValue ih_getter = JS_NewCFunction(ctx, elem_getInnerHTML, "get innerHTML", 0);
    JSValue ih_setter = JS_NewCFunction(ctx, elem_setInnerHTML, "set innerHTML", 1);
    JS_DefinePropertyGetSet(ctx, elem, innerHTML_atom, ih_getter, ih_setter,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, innerHTML_atom);

    // outerHTML - use getter/setter for dynamic access
    JSAtom outerHTML_atom = JS_NewAtom(ctx, "outerHTML");
    JSValue oh_getter = JS_NewCFunction(ctx, elem_getOuterHTML, "get outerHTML", 0);
    JSValue oh_setter = JS_NewCFunction(ctx, elem_setOuterHTML, "set outerHTML", 1);
    JS_DefinePropertyGetSet(ctx, elem, outerHTML_atom, oh_getter, oh_setter,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, outerHTML_atom);

    // <video> minimal media API
    if (node->getTagName() == "video") {
        JS_SetPropertyStr(ctx, elem, "play", JS_NewCFunction(ctx, video_play, "play", 0));
        JS_SetPropertyStr(ctx, elem, "pause", JS_NewCFunction(ctx, video_pause, "pause", 0));

        JSAtom paused_atom = JS_NewAtom(ctx, "paused");
        JSValue paused_getter = JS_NewCFunction(ctx, video_getPaused, "get paused", 0);
        JS_DefinePropertyGetSet(ctx, elem, paused_atom, paused_getter, JS_UNDEFINED,
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
        JS_FreeAtom(ctx, paused_atom);

        JSAtom ended_atom = JS_NewAtom(ctx, "ended");
        JSValue ended_getter = JS_NewCFunction(ctx, video_getEnded, "get ended", 0);
        JS_DefinePropertyGetSet(ctx, elem, ended_atom, ended_getter, JS_UNDEFINED,
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
        JS_FreeAtom(ctx, ended_atom);

        JSAtom seeking_atom = JS_NewAtom(ctx, "seeking");
        JSValue seeking_getter = JS_NewCFunction(ctx, video_getSeeking, "get seeking", 0);
        JS_DefinePropertyGetSet(ctx, elem, seeking_atom, seeking_getter, JS_UNDEFINED,
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
        JS_FreeAtom(ctx, seeking_atom);

        JSAtom ct_atom = JS_NewAtom(ctx, "currentTime");
        JSValue ct_getter = JS_NewCFunction(ctx, video_getCurrentTime, "get currentTime", 0);
        JSValue ct_setter = JS_NewCFunction(ctx, video_setCurrentTime, "set currentTime", 1);
        JS_DefinePropertyGetSet(ctx, elem, ct_atom, ct_getter, ct_setter,
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
        JS_FreeAtom(ctx, ct_atom);

        JSAtom dur_atom = JS_NewAtom(ctx, "duration");
        JSValue dur_getter = JS_NewCFunction(ctx, video_getDuration, "get duration", 0);
        JS_DefinePropertyGetSet(ctx, elem, dur_atom, dur_getter, JS_UNDEFINED,
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
        JS_FreeAtom(ctx, dur_atom);

    }

    // Define open property for <details> elements
    if (node->getTagName() == "details") {
        JSAtom open_atom = JS_NewAtom(ctx, "open");
        JSValue open_getter = JS_NewCFunction(ctx, details_getOpen, "get open", 0);
        JSValue open_setter = JS_NewCFunction(ctx, details_setOpen, "set open", 1);
        JS_DefinePropertyGetSet(ctx, elem, open_atom, open_getter, open_setter,
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
        JS_FreeAtom(ctx, open_atom);
    }

    // Methods
    JS_SetPropertyStr(ctx, elem, "getAttribute",
        JS_NewCFunction(ctx, elem_getAttribute, "getAttribute", 1));
    JS_SetPropertyStr(ctx, elem, "setAttribute",
        JS_NewCFunction(ctx, elem_setAttribute, "setAttribute", 2));
    JS_SetPropertyStr(ctx, elem, "appendChild",
        JS_NewCFunction(ctx, elem_appendChild, "appendChild", 1));
    JS_SetPropertyStr(ctx, elem, "removeChild",
        JS_NewCFunction(ctx, elem_removeChild, "removeChild", 1));
    JS_SetPropertyStr(ctx, elem, "addEventListener",
        JS_NewCFunction(ctx, elem_addEventListener, "addEventListener", 2));
    JS_SetPropertyStr(ctx, elem, "removeEventListener",
        JS_NewCFunction(ctx, elem_removeEventListener, "removeEventListener", 2));
    JS_SetPropertyStr(ctx, elem, "getComputedStyle",
        JS_NewCFunction(ctx, elem_getComputedStyle, "getComputedStyle", 0));
    JS_SetPropertyStr(ctx, elem, "getChildren",
        JS_NewCFunction(ctx, elem_getChildren, "getChildren", 0));
    JS_SetPropertyStr(ctx, elem, "insertAdjacentHTML",
        JS_NewCFunction(ctx, elem_insertAdjacentHTML, "insertAdjacentHTML", 2));

    // focus and blur
    JS_SetPropertyStr(ctx, elem, "focus",
        JS_NewCFunction(ctx, elem_focus, "focus", 0));
    JS_SetPropertyStr(ctx, elem, "blur",
        JS_NewCFunction(ctx, elem_blur, "blur", 0));

    // style and classList
    JS_SetPropertyStr(ctx, elem, "style", elem_getStyle(ctx, elem, 0, nullptr));
    JS_SetPropertyStr(ctx, elem, "classList", elem_getClassList(ctx, elem, 0, nullptr));

    // Bind Node, Element, and HTMLElement interface properties (from js_node_bindings.cpp)
    bindNodeProperties(ctx, elem, node, bindings);
    bindElementProperties(ctx, elem, node, bindings);
    bindHTMLElementProperties(ctx, elem, node, bindings);

    // Handle inline event handlers (onclick, onchange, etc.) - only on first creation
    if (bindings && !already_has_wrapper) {
        const char* event_attrs[] = {
            "onclick", "onchange", "oninput", "onfocus", "onblur",
            "onkeydown", "onkeyup", "onkeypress",
            "onmousedown", "onmouseup", "onmousemove", "onmouseover", "onmouseout",
            "onsubmit", "onload", "onerror", nullptr
        };

        for (const char** attr = event_attrs; *attr; ++attr) {
            if (node->hasAttribute(*attr)) {
                std::string js_code = node->getAttribute(*attr);
                if (!js_code.empty()) {
                    // Create a wrapper function that executes the inline code
                    std::string wrapper = "(function(event) { " + js_code + " })";

                    // Evaluate to get the function
                    JSValue func = JS_Eval(ctx, wrapper.c_str(), wrapper.length(),
                                           "<inline_event>", JS_EVAL_TYPE_GLOBAL);

                    if (JS_IsException(func)) {
                        JSValue exc = JS_GetException(ctx);
                        const char* err = JS_ToCString(ctx, exc);
                        DONG_LOG_ERROR("[JSBindings] Failed to create inline handler for %s: %s", *attr, err ? err : "unknown");
                        if (err) JS_FreeCString(ctx, err);
                        JS_FreeValue(ctx, exc);
                    } else if (JS_IsFunction(ctx, func)) {
                        // Extract event type (e.g., "onclick" -> "click")
                        std::string event_type(*attr + 2); // Skip "on" prefix

                        // Get node_id (now it should be set after setNodeOpaque)
                        uint64_t node_id = bindings->getNodeIdFor(node);
                        if (node_id != 0) {
                            // Register the listener
                            JSValue fn_dup = JS_DupValue(ctx, func);
                            bindings->registerEventListener(node_id, event_type, fn_dup);
                            JS_FreeValue(ctx, fn_dup);

                            // Ensure event bridge
                            bindings->ensureEventBridgeForNode(node, event_type, node_id);
                            DONG_LOG_INFO("[JSBindings] Registered inline handler for %s='%s' on node_id=%llu",
                                         *attr, js_code.c_str(), (unsigned long long)node_id);
                        }
                    }
                    JS_FreeValue(ctx, func);
                }
            }
        }
    }

    if (dbg) {
        DONG_LOG_INFO("[JS] createJSElement done node=%p", (void*)node.get());
    }

    return elem;
}

uint64_t JSBindings::getNodeIdFor(const dom::DOMNodePtr& node) const {
    if (!node) return 0;
    auto it = node_to_id_.find(node.get());
    if (it != node_to_id_.end()) {
        return it->second;
    }
    return 0;
}

void JSBindings::registerEventListener(uint64_t node_id, const std::string& type, JSValueConst handler) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    JSValue fn = JS_DupValue(ctx, handler);
    listeners_[node_id][type].push_back(fn);
}

void JSBindings::removeEventListener(uint64_t node_id, const std::string& type, JSValueConst handler) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    auto node_it = listeners_.find(node_id);
    if (node_it == listeners_.end()) return;

    auto& type_map = node_it->second;
    auto type_it = type_map.find(type);
    if (type_it == type_map.end()) return;

    auto& entries = type_it->second;
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        // QuickJS API 鍦ㄤ笉鍚岀増鏈腑鏈彁渚涚粺涓€鐨?JS_IsStrictEqual/JS_StrictEqual锛?
        // 杩欓噷鎴戜滑鍙渶瑕佸垽鏂袱涓嚱鏁版槸鍚︽槸鍚屼竴涓?JS 瀵硅薄锛屾墍浠ョ敤 tag+鎸囬拡姣旇緝鍗冲彲銆?
        JSValue a = *it;
        JSValue b = handler;
        if (JS_VALUE_GET_TAG(a) == JS_VALUE_GET_TAG(b) && JS_VALUE_GET_PTR(a) == JS_VALUE_GET_PTR(b)) {
            JS_FreeValue(ctx, *it);
            entries.erase(it);
            break;
        }
    }

    if (entries.empty()) {
        type_map.erase(type_it);
    }
    if (type_map.empty()) {
        listeners_.erase(node_it);
    }
}

void JSBindings::ensureEventBridgeForNode(const dom::DOMNodePtr& node, const std::string& type, uint64_t node_id) {
    if (!event_dispatcher_ || !node || node_id == 0) {
        return;
    }

    void* key = node.get();
    auto& per_node = event_bridge_ids_[key];
    if (per_node.find(type) != per_node.end()) {
        return;
    }

    dom::EventListener callback;

    if (type == "click" || type == "mousedown" || type == "mouseup" || type == "mousemove" ||
        type == "mouseenter" || type == "mouseleave" || type == "mouseover" || type == "mouseout") {
        callback = [this, node_id, type](const dom::Event& ev) {
            uint64_t related_id = 0;
            if (ev.related_target) {
                related_id = this->getNodeIdFor(ev.related_target);
            }
            this->dispatchMouseEvent(node_id, type.c_str(), ev.mouse_x, ev.mouse_y, ev.mouse_button, ev.is_trusted, related_id);
        };
    } else if (type == "keydown" || type == "keyup" || type == "keypress") {
        callback = [this, node_id, type](const dom::Event& ev) {
            this->dispatchKeyEvent(node_id, type.c_str(), ev.key_code, ev.is_trusted, ev.repeat,
                                  ev.alt_key, ev.ctrl_key, ev.shift_key, ev.meta_key);
        };
    } else if (type == "focus" || type == "blur") {
        // FocusEvent with relatedTarget support
        callback = [this, node_id, type](const dom::Event& ev) {
            uint64_t related_id = 0;
            if (ev.related_target) {
                related_id = this->getNodeIdFor(ev.related_target);
            }
            this->dispatchFocusEvent(node_id, type.c_str(), related_id);
        };
    } else if (type == "change" || type == "input" ||
               type == "submit" || type == "scroll" || type == "resize" ||
               type == "DOMContentLoaded" || type == "wheel") {
        callback = [this, node_id, type](const dom::Event&) {
            this->dispatchSimpleEvent(node_id, type.c_str());
        };
    } else if (type == "compositionstart" || type == "compositionupdate" || type == "compositionend") {
        callback = [this, node_id, type](const dom::Event& ev) {
            this->dispatchCompositionEvent(node_id, type.c_str(), ev.input_data.c_str());
        };
    } else if (type == "copy" || type == "cut" || type == "paste") {
        callback = [this, node_id, type](const dom::Event& ev) {
            this->dispatchClipboardEvent(node_id, type.c_str(), ev.input_data.c_str());
        };
    } else if (type == "beforeinput") {
        callback = [this, node_id](const dom::Event& ev) {
            this->dispatchBeforeInputEvent(node_id, ev.input_type.c_str(), ev.input_data.c_str());
        };
    } else if (type == "dragstart" || type == "drag" || type == "dragend" || type == "drop" ||
               type == "dragenter" || type == "dragleave" || type == "dragover") {
        // Drag & Drop events with data transfer support
        callback = [this, node_id, type](const dom::Event& ev) {
            this->dispatchDragDropEvent(node_id, type.c_str(), ev.mouse_x, ev.mouse_y, ev.data_transfer, ev.is_trusted);
        };
    } else {
        // Unknown event type - still create a simple bridge
        callback = [this, node_id, type](const dom::Event&) {
            this->dispatchSimpleEvent(node_id, type.c_str());
        };
    }

    uint64_t listener_id = event_dispatcher_->addEventListener(node, type, callback);
    if (listener_id != 0) {
        per_node[type] = listener_id;
    }
}

namespace {

bool jsGetBoolProp(JSContext* ctx, JSValueConst obj, const char* name, bool default_value) {
    JSValue v = JS_GetPropertyStr(ctx, obj, name);
    if (JS_IsUndefined(v)) {
        JS_FreeValue(ctx, v);
        return default_value;
    }
    const bool r = JS_ToBool(ctx, v) != 0;
    JS_FreeValue(ctx, v);
    return r;
}

void mapKeyCodeToStrings(uint32_t key_code, std::string& key_str, std::string& code_str) {
    key_str.clear();
    code_str.clear();

    if (key_code >= 32 && key_code < 127) {
        key_str = std::string(1, (char)key_code);
        if (key_code >= 'a' && key_code <= 'z') {
            code_str = std::string("Key") + (char)(key_code - 32);
        } else if (key_code >= 'A' && key_code <= 'Z') {
            code_str = std::string("Key") + (char)key_code;
        } else if (key_code >= '0' && key_code <= '9') {
            code_str = std::string("Digit") + (char)key_code;
        } else if (key_code == ' ') {
            key_str = " ";
            code_str = "Space";
        } else {
            code_str = key_str;
        }
        return;
    }

    if (key_code == 13) { key_str = "Enter"; code_str = "Enter"; return; }
    if (key_code == 27) { key_str = "Escape"; code_str = "Escape"; return; }
    if (key_code == 8) { key_str = "Backspace"; code_str = "Backspace"; return; }
    if (key_code == 9) { key_str = "Tab"; code_str = "Tab"; return; }
    if (key_code == 127) { key_str = "Delete"; code_str = "Delete"; return; }

    if (key_code == 0x40000050) { key_str = "ArrowLeft"; code_str = "ArrowLeft"; return; }
    if (key_code == 0x4000004F) { key_str = "ArrowRight"; code_str = "ArrowRight"; return; }
    if (key_code == 0x40000052) { key_str = "ArrowUp"; code_str = "ArrowUp"; return; }
    if (key_code == 0x40000051) { key_str = "ArrowDown"; code_str = "ArrowDown"; return; }
}

} // namespace

void JSBindings::dispatchEventToChain(const dom::DOMNodePtr& target,
                                     const std::string& type,
                                     JSValue event) {
    if (!engine_ || !target) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    const bool bubbles = jsGetBoolProp(ctx, event, "bubbles", true);

    // Set target once.
    {
        JSValue target_js = createJSElement(ctx, target);
        JS_SetPropertyStr(ctx, event, "target", target_js);
    }

    dom::DOMNodePtr current = target;
    while (current) {
        const bool stop_immediate = jsGetBoolProp(ctx, event, "__stoppedImmediate", false);
        if (stop_immediate) {
            break;
        }

        JSValue current_js = createJSElement(ctx, current);
        JSValue this_val = JS_DupValue(ctx, current_js);
        JS_SetPropertyStr(ctx, event, "currentTarget", current_js);

        uint64_t current_id = getNodeIdFor(current);
        if (current_id != 0) {
            auto node_it = listeners_.find(current_id);
            if (node_it != listeners_.end()) {
                auto& type_map = node_it->second;
                auto type_it = type_map.find(type);
                if (type_it != type_map.end()) {
                    auto& funcs = type_it->second;
                    for (auto& fn : funcs) {
                        if (!JS_IsFunction(ctx, fn)) continue;

                        JSValue ret = JS_Call(ctx, fn, this_val, 1, &event);
                        if (JS_IsException(ret)) {
                            JSValue exc = JS_GetException(ctx);
                            const char* err = JS_ToCString(ctx, exc);
                            if (err) {
                                std::fprintf(stderr, "[JSBindings] event(%s) error: %s\n", type.c_str(), err);
                                JS_FreeCString(ctx, err);
                            }
                            JS_FreeValue(ctx, exc);
                        }
                        JS_FreeValue(ctx, ret);

                        if (jsGetBoolProp(ctx, event, "__stoppedImmediate", false)) {
                            break;
                        }
                    }
                }
            }
        }

        JS_FreeValue(ctx, this_val);

        if (!bubbles) {
            break;
        }

        if (jsGetBoolProp(ctx, event, "cancelBubble", false)) {
            break;
        }

        current = current->getParent();
    }
}

bool JSBindings::dispatchCancelableEventToChain(const dom::DOMNodePtr& target,
                                              const std::string& type,
                                              JSValue event) {
    dispatchEventToChain(target, type, event);
    return jsGetBoolProp(engine_->getContext(), event, "defaultPrevented", false);
}

void JSBindings::dispatchMouseEvent(uint64_t node_id, const char* type, int32_t x, int32_t y, int32_t button,
                                  bool is_trusted, uint64_t related_node_id) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    if (!type || !type[0]) return;

    auto dom_it = id_to_node_.find(node_id);
    if (dom_it == id_to_node_.end() || !dom_it->second) {
        return;
    }

    const dom::DOMNodePtr target_node = dom_it->second;

    const int32_t movement_x = has_last_mouse_pos_ ? (x - last_mouse_x_) : 0;
    const int32_t movement_y = has_last_mouse_pos_ ? (y - last_mouse_y_) : 0;
    has_last_mouse_pos_ = true;
    last_mouse_x_ = x;
    last_mouse_y_ = y;

    // Calculate pageX/pageY by adding scroll offset from document root
    // pageX = clientX + scrollX, pageY = clientY + scrollY
    float scroll_x = 0.0f;
    float scroll_y = 0.0f;
    if (dom_manager_) {
        auto root = dom_manager_->getRoot();
        if (root) {
            // Use the document/body element's scroll offset
            auto bodies = dom_manager_->getElementsByTagName("body");
            if (!bodies.empty()) {
                scroll_x = bodies[0]->getScrollX();
                scroll_y = bodies[0]->getScrollY();
            } else {
                scroll_x = root->getScrollX();
                scroll_y = root->getScrollY();
            }
        }
    }
    const int32_t page_x = static_cast<int32_t>(x + scroll_x);
    const int32_t page_y = static_cast<int32_t>(y + scroll_y);

    JSValue ev = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
    JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "isTrusted", JS_NewBool(ctx, is_trusted));

    JS_SetPropertyStr(ctx, ev, "clientX", JS_NewInt32(ctx, x));
    JS_SetPropertyStr(ctx, ev, "clientY", JS_NewInt32(ctx, y));
    JS_SetPropertyStr(ctx, ev, "pageX", JS_NewInt32(ctx, page_x));
    JS_SetPropertyStr(ctx, ev, "pageY", JS_NewInt32(ctx, page_y));
    JS_SetPropertyStr(ctx, ev, "movementX", JS_NewInt32(ctx, movement_x));
    JS_SetPropertyStr(ctx, ev, "movementY", JS_NewInt32(ctx, movement_y));

    JS_SetPropertyStr(ctx, ev, "button", JS_NewInt32(ctx, button));

    // offsetX/offsetY: relative to target element's bounding rect
    {
        auto rect = target_node->getBoundingClientRect();
        JS_SetPropertyStr(ctx, ev, "offsetX", JS_NewFloat64(ctx, x - rect.x));
        JS_SetPropertyStr(ctx, ev, "offsetY", JS_NewFloat64(ctx, y - rect.y));
    }

    if (related_node_id != 0) {
        auto rel_it = id_to_node_.find(related_node_id);
        if (rel_it != id_to_node_.end() && rel_it->second) {
            JS_SetPropertyStr(ctx, ev, "relatedTarget", createJSElement(ctx, rel_it->second));
        } else {
            JS_SetPropertyStr(ctx, ev, "relatedTarget", JS_NULL);
        }
    } else {
        JS_SetPropertyStr(ctx, ev, "relatedTarget", JS_NULL);
    }

    // Modifier keys (default false)
    JS_SetPropertyStr(ctx, ev, "altKey", JS_FALSE);
    JS_SetPropertyStr(ctx, ev, "ctrlKey", JS_FALSE);
    JS_SetPropertyStr(ctx, ev, "shiftKey", JS_FALSE);
    JS_SetPropertyStr(ctx, ev, "metaKey", JS_FALSE);

    // PointerEvent compatibility
    JS_SetPropertyStr(ctx, ev, "pointerId", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, ev, "pointerType", JS_NewString(ctx, "mouse"));
    JS_SetPropertyStr(ctx, ev, "pressure", JS_NewFloat64(ctx, button != 0 ? 0.5 : 0.0));
    JS_SetPropertyStr(ctx, ev, "isPrimary", JS_TRUE);

    // Event methods
    JS_SetPropertyStr(ctx, ev, "preventDefault",
        JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
    JS_SetPropertyStr(ctx, ev, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    JS_SetPropertyStr(ctx, ev, "stopImmediatePropagation",
        JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

    dispatchEventToChain(target_node, type, ev);
    JS_FreeValue(ctx, ev);
}

void JSBindings::dispatchKeyEvent(uint64_t node_id, const char* type, uint32_t key_code,
                                bool is_trusted, bool repeat,
                                bool alt_key, bool ctrl_key, bool shift_key, bool meta_key) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    if (!type || !type[0]) return;

    auto dom_it = id_to_node_.find(node_id);
    if (dom_it == id_to_node_.end() || !dom_it->second) {
        return;
    }

    const dom::DOMNodePtr target_node = dom_it->second;

    std::string key_str;
    std::string code_str;
    mapKeyCodeToStrings(key_code, key_str, code_str);

    JSValue ev = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
    JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "isTrusted", JS_NewBool(ctx, is_trusted));

    JS_SetPropertyStr(ctx, ev, "keyCode", JS_NewInt32(ctx, (int32_t)key_code));
    JS_SetPropertyStr(ctx, ev, "charCode", JS_NewInt32(ctx, (int32_t)key_code));
    JS_SetPropertyStr(ctx, ev, "which", JS_NewInt32(ctx, (int32_t)key_code));
    JS_SetPropertyStr(ctx, ev, "key", JS_NewString(ctx, key_str.c_str()));
    JS_SetPropertyStr(ctx, ev, "code", JS_NewString(ctx, code_str.c_str()));
    JS_SetPropertyStr(ctx, ev, "repeat", JS_NewBool(ctx, repeat));

    // Modifier keys
    JS_SetPropertyStr(ctx, ev, "altKey", JS_NewBool(ctx, alt_key));
    JS_SetPropertyStr(ctx, ev, "ctrlKey", JS_NewBool(ctx, ctrl_key));
    JS_SetPropertyStr(ctx, ev, "shiftKey", JS_NewBool(ctx, shift_key));
    JS_SetPropertyStr(ctx, ev, "metaKey", JS_NewBool(ctx, meta_key));


    // Event methods
    JS_SetPropertyStr(ctx, ev, "preventDefault",
        JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
    JS_SetPropertyStr(ctx, ev, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    JS_SetPropertyStr(ctx, ev, "stopImmediatePropagation",
        JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

    dispatchEventToChain(target_node, type, ev);
    JS_FreeValue(ctx, ev);
}

void JSBindings::dispatchDragDropEvent(uint64_t node_id, const char* type, int32_t x, int32_t y, const std::string& data_transfer, bool is_trusted) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    if (!type || !type[0]) return;

    auto dom_it = id_to_node_.find(node_id);
    if (dom_it == id_to_node_.end() || !dom_it->second) {
        return;
    }

    const dom::DOMNodePtr target_node = dom_it->second;

    JSValue ev = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
    JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "isTrusted", JS_NewBool(ctx, is_trusted));

    // Mouse position properties
    JS_SetPropertyStr(ctx, ev, "clientX", JS_NewInt32(ctx, x));
    JS_SetPropertyStr(ctx, ev, "clientY", JS_NewInt32(ctx, y));
    JS_SetPropertyStr(ctx, ev, "pageX", JS_NewInt32(ctx, x));
    JS_SetPropertyStr(ctx, ev, "pageY", JS_NewInt32(ctx, y));

    // Data transfer property (simplified for Phase 1)
    JSValue data_transfer_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, data_transfer_obj, "data", JS_NewString(ctx, data_transfer.c_str()));
    JS_SetPropertyStr(ctx, ev, "dataTransfer", data_transfer_obj);

    // Calculate offsetX/offsetY relative to target element
    if (layout_engine_) {
        auto layout_node = layout_engine_->getLayout(target_node);
        if (layout_node) {
            float elem_x = layout_node->x;
            float elem_y = layout_node->y;
            int32_t offset_x = x - static_cast<int32_t>(elem_x);
            int32_t offset_y = y - static_cast<int32_t>(elem_y);
            JS_SetPropertyStr(ctx, ev, "offsetX", JS_NewInt32(ctx, offset_x));
            JS_SetPropertyStr(ctx, ev, "offsetY", JS_NewInt32(ctx, offset_y));
        }
    }

    // Event methods
    JS_SetPropertyStr(ctx, ev, "preventDefault",
        JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
    JS_SetPropertyStr(ctx, ev, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    JS_SetPropertyStr(ctx, ev, "stopImmediatePropagation",
        JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

    dispatchEventToChain(target_node, type, ev);
    JS_FreeValue(ctx, ev);
}

void JSBindings::dispatchSimpleEvent(uint64_t node_id, const char* type) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    if (!type || !type[0]) return;

    auto dom_it = id_to_node_.find(node_id);
    if (dom_it == id_to_node_.end() || !dom_it->second) {
        return;
    }

    const dom::DOMNodePtr target_node = dom_it->second;

    JSValue ev = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
    JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "isTrusted", JS_TRUE);

    // Event methods
    JS_SetPropertyStr(ctx, ev, "preventDefault",
        JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
    JS_SetPropertyStr(ctx, ev, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    JS_SetPropertyStr(ctx, ev, "stopImmediatePropagation",
        JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

    dispatchEventToChain(target_node, type, ev);
    JS_FreeValue(ctx, ev);
}

bool JSBindings::dispatchEventObject(uint64_t node_id, JSValueConst event_obj) {
    if (!engine_) return true;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return true;

    auto dom_it = id_to_node_.find(node_id);
    if (dom_it == id_to_node_.end() || !dom_it->second) {
        return true;
    }

    // Require a string 'type'
    JSValue type_val = JS_GetPropertyStr(ctx, event_obj, "type");
    const char* type_cstr = JS_ToCString(ctx, type_val);
    JS_FreeValue(ctx, type_val);
    if (!type_cstr || !type_cstr[0]) {
        if (type_cstr) JS_FreeCString(ctx, type_cstr);
        return true;
    }

    // Spec: Event.isTrusted is always false for dispatchEvent.
    JS_SetPropertyStr(ctx, (JSValue)event_obj, "isTrusted", JS_FALSE);

    const std::string type(type_cstr);
    JS_FreeCString(ctx, type_cstr);

    JSValue ev = JS_DupValue(ctx, event_obj);
    const bool prevented = dispatchCancelableEventToChain(dom_it->second, type, ev);
    JS_FreeValue(ctx, ev);

    return !prevented;
}

bool JSBindings::dispatchBeforeInputEvent(uint64_t node_id, const char* input_type, const char* data) {
    if (!engine_) return false;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return false;
    if (!input_type || !input_type[0]) return false;

    auto dom_it = id_to_node_.find(node_id);
    if (dom_it == id_to_node_.end() || !dom_it->second) {
        return false;
    }

    JSValue ev = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, "beforeinput"));
    JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "isTrusted", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "inputType", JS_NewString(ctx, input_type));
    JS_SetPropertyStr(ctx, ev, "data", data ? JS_NewString(ctx, data) : JS_NULL);

    // Event methods
    JS_SetPropertyStr(ctx, ev, "preventDefault",
        JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
    JS_SetPropertyStr(ctx, ev, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    JS_SetPropertyStr(ctx, ev, "stopImmediatePropagation",
        JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

    const bool prevented = dispatchCancelableEventToChain(dom_it->second, "beforeinput", ev);
    JS_FreeValue(ctx, ev);
    return prevented;
}

void JSBindings::dispatchInputEvent(uint64_t node_id, const char* input_type, const char* data) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    if (!input_type || !input_type[0]) return;

    auto dom_it = id_to_node_.find(node_id);
    if (dom_it == id_to_node_.end() || !dom_it->second) {
        return;
    }

    JSValue ev = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, "input"));
    JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
    JS_SetPropertyStr(ctx, ev, "cancelable", JS_FALSE);
    JS_SetPropertyStr(ctx, ev, "isTrusted", JS_TRUE);

    JS_SetPropertyStr(ctx, ev, "inputType", JS_NewString(ctx, input_type));
    JS_SetPropertyStr(ctx, ev, "data", data ? JS_NewString(ctx, data) : JS_NULL);

    JS_SetPropertyStr(ctx, ev, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    JS_SetPropertyStr(ctx, ev, "stopImmediatePropagation",
        JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

    dispatchEventToChain(dom_it->second, "input", ev);
    JS_FreeValue(ctx, ev);
}

void JSBindings::dispatchCompositionEvent(uint64_t node_id, const char* type, const char* data) {
    if (!engine_) return;
    if (!type || !type[0]) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    auto node_it = listeners_.find(node_id);
    if (node_it == listeners_.end()) return;
    auto& type_map = node_it->second;
    auto type_it = type_map.find(type);
    if (type_it == type_map.end()) return;

    auto& funcs = type_it->second;
    for (auto& fn : funcs) {
        if (!JS_IsFunction(ctx, fn)) continue;

        JSValue ev = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "data", data ? JS_NewString(ctx, data) : JS_NewString(ctx, ""));

        JSValue this_val = JS_UNDEFINED;
        auto dom_it = id_to_node_.find(node_id);
        if (dom_it != id_to_node_.end()) {
            JSValue target = createJSElement(ctx, dom_it->second);
            JSValue current_target = JS_DupValue(ctx, target);
            this_val = JS_DupValue(ctx, target);
            JS_SetPropertyStr(ctx, ev, "target", target);
            JS_SetPropertyStr(ctx, ev, "currentTarget", current_target);
        }

        JSValue ret = JS_Call(ctx, fn, this_val, 1, &ev);
        if (JS_IsException(ret)) {
            JSValue exc = JS_GetException(ctx);
            const char* err = JS_ToCString(ctx, exc);
            if (err) {
                std::fprintf(stderr, "[JSBindings] CompositionEvent error: %s\n", err);
                JS_FreeCString(ctx, err);
            }
            JS_FreeValue(ctx, exc);
        }
        JS_FreeValue(ctx, ret);
        if (!JS_IsUndefined(this_val)) {
            JS_FreeValue(ctx, this_val);
        }
        JS_FreeValue(ctx, ev);
    }
}

void JSBindings::dispatchFocusEvent(uint64_t node_id, const char* type, uint64_t related_node_id) {
    if (!engine_) return;
    if (!type || !type[0]) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    auto node_it = listeners_.find(node_id);
    if (node_it == listeners_.end()) return;
    auto& type_map = node_it->second;
    auto type_it = type_map.find(type);
    if (type_it == type_map.end()) return;

    auto& funcs = type_it->second;
    for (auto& fn : funcs) {
        if (!JS_IsFunction(ctx, fn)) continue;

        // Create FocusEvent object with relatedTarget property
        JSValue ev = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "cancelable", JS_FALSE);  // FocusEvent is not cancelable

        auto dom_it = id_to_node_.find(node_id);
        if (dom_it != id_to_node_.end()) {
            JSValue target = createJSElement(ctx, dom_it->second);
            JS_SetPropertyStr(ctx, ev, "target", target);
            JS_SetPropertyStr(ctx, ev, "currentTarget", JS_DupValue(ctx, target));
        }

        // Set relatedTarget property
        if (related_node_id != 0) {
            auto related_it = id_to_node_.find(related_node_id);
            if (related_it != id_to_node_.end()) {
                JSValue related_target = createJSElement(ctx, related_it->second);
                JS_SetPropertyStr(ctx, ev, "relatedTarget", related_target);
            } else {
                JS_SetPropertyStr(ctx, ev, "relatedTarget", JS_NULL);
            }
        } else {
            JS_SetPropertyStr(ctx, ev, "relatedTarget", JS_NULL);
        }

        JSValue ret = JS_Call(ctx, fn, JS_UNDEFINED, 1, &ev);
        if (JS_IsException(ret)) {
            JSValue exc = JS_GetException(ctx);
            const char* err = JS_ToCString(ctx, exc);
            if (err) {
                std::fprintf(stderr, "[JSBindings] FocusEvent(%s) error: %s\n", type, err);
                JS_FreeCString(ctx, err);
            }
            JS_FreeValue(ctx, exc);
        }
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, ev);
    }
}

void JSBindings::dispatchMediaEvent(uint64_t node_id, const char* type, double current_time, double duration, const char* message) {
    if (!engine_) return;
    if (!type || !type[0]) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    auto node_it = listeners_.find(node_id);
    if (node_it == listeners_.end()) return;
    auto& type_map = node_it->second;
    auto type_it = type_map.find(type);
    if (type_it == type_map.end()) return;

    auto& funcs = type_it->second;
    for (auto& fn : funcs) {
        if (!JS_IsFunction(ctx, fn)) continue;

        JSValue ev = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "currentTime", JS_NewFloat64(ctx, current_time));
        JS_SetPropertyStr(ctx, ev, "duration", JS_NewFloat64(ctx, duration));
        if (message && message[0]) {
            JS_SetPropertyStr(ctx, ev, "message", JS_NewString(ctx, message));
            JS_SetPropertyStr(ctx, ev, "error", JS_NewString(ctx, message));
        }

        auto dom_it = id_to_node_.find(node_id);
        if (dom_it != id_to_node_.end()) {
            JSValue target = createJSElement(ctx, dom_it->second);
            JS_SetPropertyStr(ctx, ev, "target", target);
            JS_SetPropertyStr(ctx, ev, "currentTarget", JS_DupValue(ctx, target));
        }

        JSValue ret = JS_Call(ctx, fn, JS_UNDEFINED, 1, &ev);
        if (JS_IsException(ret)) {
            JSValue exc = JS_GetException(ctx);
            const char* err = JS_ToCString(ctx, exc);
            if (err) {
                std::fprintf(stderr, "[JSBindings] media event(%s) error: %s\\n", type, err);
                JS_FreeCString(ctx, err);
            }
            JS_FreeValue(ctx, exc);
        }
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, ev);
    }
}

void JSBindings::dispatchClipboardEvent(uint64_t node_id, const char* type, const char* data) {
    if (!engine_) return;
    if (!type || !type[0]) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    auto node_it = listeners_.find(node_id);
    if (node_it == listeners_.end()) return;
    auto& type_map = node_it->second;
    auto type_it = type_map.find(type);
    if (type_it == type_map.end()) return;

    auto& funcs = type_it->second;
    for (auto& fn : funcs) {
        if (!JS_IsFunction(ctx, fn)) continue;

        // Create ClipboardEvent-like object with clipboardData property
        JSValue ev = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, type));
        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "isTrusted", JS_TRUE);

        // Set modifier keys
        JS_SetPropertyStr(ctx, ev, "altKey", JS_FALSE);
        JS_SetPropertyStr(ctx, ev, "ctrlKey", JS_TRUE);  // Clipboard events typically use Ctrl
        JS_SetPropertyStr(ctx, ev, "shiftKey", JS_FALSE);
        JS_SetPropertyStr(ctx, ev, "metaKey", JS_FALSE);

        // Create simplified clipboardData object
        // In a full implementation, this would be a DataTransfer object
        JSValue clipboard_data = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, clipboard_data, "text/plain",
            data ? JS_NewString(ctx, data) : JS_NewString(ctx, ""));

        // Add getData/setData methods for clipboardData
        // These provide a simplified API matching the Clipboard DataTransfer interface
        JS_SetPropertyStr(ctx, ev, "clipboardData", clipboard_data);

        // Event methods
        JS_SetPropertyStr(ctx, ev, "preventDefault",
            JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
        JS_SetPropertyStr(ctx, ev, "stopPropagation",
            JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
        JS_SetPropertyStr(ctx, ev, "stopImmediatePropagation",
            JS_NewCFunction(ctx, event_stopImmediatePropagation, "stopImmediatePropagation", 0));

        // Set target and currentTarget
        auto dom_it = id_to_node_.find(node_id);
        if (dom_it != id_to_node_.end()) {
            JSValue target = createJSElement(ctx, dom_it->second);
            JS_SetPropertyStr(ctx, ev, "target", target);
            JS_SetPropertyStr(ctx, ev, "currentTarget", JS_DupValue(ctx, target));
        }

        const bool prevented = dispatchCancelableEventToChain(dom_it->second, type, ev);
        JS_FreeValue(ctx, ev);

        // If not prevented and this is a paste event with data, the actual paste would happen here
        // This is a placeholder for future clipboard integration
        (void)prevented;
    }
}

bool JSBindings::hasEventListeners(uint64_t node_id, const char* type) const {
    auto node_it = listeners_.find(node_id);
    if (node_it == listeners_.end()) return false;
    const auto& type_map = node_it->second;
    auto type_it = type_map.find(type);
    if (type_it == type_map.end()) return false;
    return !type_it->second.empty();
}

void JSBindings::resetForNewDOM() {
    if (engine_) {
        JSContext* ctx = engine_->getContext();
        if (ctx) {
            for (auto& node_pair : listeners_) {
                for (auto& type_pair : node_pair.second) {
                    for (auto& fn : type_pair.second) {
                        JS_FreeValue(ctx, fn);
                    }
                }
            }
        }
    }

    listeners_.clear();

    // Unregister bridge listeners from the C++ EventDispatcher
    if (event_dispatcher_) {
        for (auto& node_entry : event_bridge_ids_) {
            void* raw = node_entry.first;
            auto id_it = node_to_id_.find(raw);
            if (id_it == node_to_id_.end()) continue;

            uint64_t node_id = id_it->second;
            auto dom_it = id_to_node_.find(node_id);
            if (dom_it == id_to_node_.end()) continue;

            dom::DOMNodePtr node = dom_it->second;
            for (auto& type_pair : node_entry.second) {
                const std::string& type = type_pair.first;
                uint64_t listener_id = type_pair.second;
                event_dispatcher_->removeEventListener(node, type, listener_id);
            }
        }
    }

    event_bridge_ids_.clear();
    id_to_node_.clear();
    node_to_id_.clear();

    // Cancel all pending timers
    if (engine_) {
        JSContext* ctx = engine_->getContext();
        if (ctx) {
            for (auto& [id, entry] : timers_) {
                JS_FreeValue(ctx, entry.callback);
            }
        }
    }
    timers_.clear();
}

void JSBindings::tickTimers(double current_time_sec) {
    if (!engine_) return;
    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    // Collect timers that are due (avoid modifying map while iterating)
    std::vector<uint32_t> due_ids;
    for (auto& [id, entry] : timers_) {
        if (!entry.cancelled && current_time_sec >= entry.fire_at) {
            due_ids.push_back(id);
        }
    }

    for (uint32_t id : due_ids) {
        auto it = timers_.find(id);
        if (it == timers_.end()) continue;
        auto& entry = it->second;
        if (entry.cancelled) {
            JS_FreeValue(ctx, entry.callback);
            timers_.erase(it);
            continue;
        }

        JSValue cb = JS_DupValue(ctx, entry.callback);

        if (entry.interval >= 0.0) {
            // Repeating: reschedule before calling (so callback can clearInterval)
            entry.fire_at = current_time_sec + entry.interval;
        } else {
            // One-shot: remove before calling (so callback can safely re-register)
            JS_FreeValue(ctx, entry.callback);
            timers_.erase(it);
        }

        JSValue result = JS_Call(ctx, cb, JS_UNDEFINED, 0, nullptr);
        JS_FreeValue(ctx, cb);
        if (JS_IsException(result)) {
            JSValue ex = JS_GetException(ctx);
            auto* err_str = JS_ToCString(ctx, ex);
            DONG_LOG_ERROR("[Timer] Exception in timer callback: %s", err_str ? err_str : "(unknown)");
            if (err_str) JS_FreeCString(ctx, err_str);
            JS_FreeValue(ctx, ex);
        }
        JS_FreeValue(ctx, result);
    }

    // Purge cancelled one-shots
    for (auto it = timers_.begin(); it != timers_.end(); ) {
        if (it->second.cancelled) {
            JS_FreeValue(ctx, it->second.callback);
            it = timers_.erase(it);
        } else {
            ++it;
        }
    }
}


dom::DOMNodePtr JSBindings::getNodeOpaque(JSContext* ctx, JSValue val) {
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings) return nullptr;

    JSValue id_val = JS_GetPropertyStr(ctx, val, "__node_id__");
    if (JS_IsUndefined(id_val)) {
        JS_FreeValue(ctx, id_val);
        return nullptr;
    }

    int64_t id = 0;
    if (JS_ToInt64(ctx, &id, id_val) != 0) {
        JS_FreeValue(ctx, id_val);
        return nullptr;
    }
    JS_FreeValue(ctx, id_val);

    auto it = bindings->id_to_node_.find(static_cast<uint64_t>(id));
    if (it != bindings->id_to_node_.end()) {
        return it->second;
    }
    return nullptr;
}

void JSBindings::setNodeOpaque(JSContext* ctx, JSValue val, dom::DOMNodePtr node) {
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || !node) return;

    uint64_t id = 0;
    auto it = bindings->node_to_id_.find(node.get());
    if (it != bindings->node_to_id_.end()) {
        id = it->second;
    } else {
        id = bindings->next_js_id_++;
        bindings->id_to_node_[id] = node;
        bindings->node_to_id_[node.get()] = id;
    }

    JS_SetPropertyStr(ctx, val, "__node_id__", JS_NewInt64(ctx, static_cast<int64_t>(id)));
}

void JSBindings::setActiveBindings(JSBindings* bindings) {
    s_active_bindings = bindings;
}

// ============================================================
// Multi-view: dong.getView(name) / dong.views registry
// ============================================================

// C callback for dong.getView(name) — retrieves a named view's window object.
static JSValue js_dong_getView(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_UNDEFINED;

    // Look up dong.views[name]
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue dong_obj = JS_GetPropertyStr(ctx, global, "dong");
    if (JS_IsUndefined(dong_obj) || JS_IsNull(dong_obj)) {
        JS_FreeValue(ctx, dong_obj);
        JS_FreeValue(ctx, global);
        JS_FreeCString(ctx, name);
        return JS_UNDEFINED;
    }
    JSValue views = JS_GetPropertyStr(ctx, dong_obj, "views");
    JSValue result = JS_UNDEFINED;
    if (!JS_IsUndefined(views) && !JS_IsNull(views)) {
        result = JS_GetPropertyStr(ctx, views, name);
    }
    JS_FreeValue(ctx, views);
    JS_FreeValue(ctx, dong_obj);
    JS_FreeValue(ctx, global);
    JS_FreeCString(ctx, name);
    return result;
}

// Ensure the global `dong` object and `dong.views` sub-object exist.
static void ensureDongViewsObject(JSContext* ctx) {
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue dong_obj = JS_GetPropertyStr(ctx, global, "dong");
    if (JS_IsUndefined(dong_obj) || JS_IsNull(dong_obj)) {
        JS_FreeValue(ctx, dong_obj);
        dong_obj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, dong_obj, "views", JS_NewObject(ctx));
        JS_SetPropertyStr(ctx, dong_obj, "getView",
            JS_NewCFunction(ctx, js_dong_getView, "getView", 1));
        JS_SetPropertyStr(ctx, global, "dong", dong_obj);
    } else {
        JSValue views = JS_GetPropertyStr(ctx, dong_obj, "views");
        if (JS_IsUndefined(views) || JS_IsNull(views)) {
            JS_FreeValue(ctx, views);
            JS_SetPropertyStr(ctx, dong_obj, "views", JS_NewObject(ctx));
        } else {
            JS_FreeValue(ctx, views);
        }
        JS_FreeValue(ctx, dong_obj);
    }
    JS_FreeValue(ctx, global);
}

void JSBindings::registerAsNamedView() {
    if (!engine_ || view_name_.empty()) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;

    // Ensure dong.views exists
    ensureDongViewsObject(ctx);

    // Build the per-view document object (same as initializeDocumentAPI but on a new object)
    JSValue doc = JS_NewObject(ctx);

    // Store owning JSBindings pointer so doc_* callbacks resolve the correct DOM
    setDocBindings(ctx, doc, this);

    JS_SetPropertyStr(ctx, doc, "getElementById",
        JS_NewCFunction(ctx, doc_getElementById, "getElementById", 1));
    JS_SetPropertyStr(ctx, doc, "getElementsByTagName",
        JS_NewCFunction(ctx, doc_getElementsByTagName, "getElementsByTagName", 1));
    JS_SetPropertyStr(ctx, doc, "getElementsByClassName",
        JS_NewCFunction(ctx, doc_getElementsByClassName, "getElementsByClassName", 1));
    JS_SetPropertyStr(ctx, doc, "querySelector",
        JS_NewCFunction(ctx, doc_querySelector, "querySelector", 1));
    JS_SetPropertyStr(ctx, doc, "querySelectorAll",
        JS_NewCFunction(ctx, doc_querySelectorAll, "querySelectorAll", 1));
    JS_SetPropertyStr(ctx, doc, "elementFromPoint",
        JS_NewCFunction(ctx, doc_elementFromPoint, "elementFromPoint", 2));
    JS_SetPropertyStr(ctx, doc, "hasFocus",
        JS_NewCFunction(ctx, doc_hasFocus, "hasFocus", 0));
    JS_SetPropertyStr(ctx, doc, "createElement",
        JS_NewCFunction(ctx, doc_createElement, "createElement", 1));
    JS_SetPropertyStr(ctx, doc, "createTextNode",
        JS_NewCFunction(ctx, doc_createTextNode, "createTextNode", 1));
    JS_SetPropertyStr(ctx, doc, "createComment",
        JS_NewCFunction(ctx, doc_createComment, "createComment", 1));
    JS_SetPropertyStr(ctx, doc, "addEventListener",
        JS_NewCFunction(ctx, elem_addEventListener, "addEventListener", 2));
    JS_SetPropertyStr(ctx, doc, "removeEventListener",
        JS_NewCFunction(ctx, elem_removeEventListener, "removeEventListener", 2));

    // document.execCommand / queryCommandSupported (shared implementation)
    JS_SetPropertyStr(ctx, doc, "execCommand",
        JS_NewCFunction(ctx, js_doc_execCommand, "execCommand", 3));
    JS_SetPropertyStr(ctx, doc, "queryCommandSupported",
        JS_NewCFunction(ctx, js_doc_queryCommandSupported, "queryCommandSupported", 1));

    // Document object references
    if (dom_manager_) {
        auto body_nodes = dom_manager_->getElementsByTagName("body");
        if (!body_nodes.empty()) {
            JS_SetPropertyStr(ctx, doc, "body", createJSElement(ctx, body_nodes[0]));
            setNodeOpaque(ctx, doc, body_nodes[0]);
        }
        auto html_nodes = dom_manager_->getElementsByTagName("html");
        if (!html_nodes.empty()) {
            JS_SetPropertyStr(ctx, doc, "documentElement", createJSElement(ctx, html_nodes[0]));
        }
    }

    // Build the per-view window object
    JSValue win = JS_NewObject(ctx);

    // window.document = doc
    JS_SetPropertyStr(ctx, win, "document", JS_DupValue(ctx, doc));

    // window.getComputedStyle
    JS_SetPropertyStr(ctx, win, "getComputedStyle",
        JS_NewCFunction(ctx, window_getComputedStyle, "getComputedStyle", 1));

    // document.defaultView = window (circular ref)
    JS_SetPropertyStr(ctx, doc, "defaultView", JS_DupValue(ctx, win));

    // Register on dong.views[name]
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue dong_obj = JS_GetPropertyStr(ctx, global, "dong");
    JSValue views = JS_GetPropertyStr(ctx, dong_obj, "views");
    JS_SetPropertyStr(ctx, views, view_name_.c_str(), JS_DupValue(ctx, win));
    JS_FreeValue(ctx, views);
    JS_FreeValue(ctx, dong_obj);
    JS_FreeValue(ctx, global);

    JS_FreeValue(ctx, win);
    JS_FreeValue(ctx, doc);

    DONG_LOG_INFO("[JSBindings] Registered named view '%s' on dong.views", view_name_.c_str());
}

} // namespace dong::script
