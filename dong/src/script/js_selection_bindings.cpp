// JS Selection/Range bindings implementation

#include "js_selection_bindings.hpp"
#include "js_bindings.hpp"
#include "../dom/selection.hpp"
#include "../dom/range.hpp"
#include "../core/log.h"

#include <string>

namespace dong::script {

// Per-view Selection instance (stored via context opaque)
static dong::dom::Selection* getViewSelection(JSContext* ctx) {
    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (!bindings) return nullptr;
    // Selection is stored as user data on the bindings (simplified approach)
    // We use a static map keyed by JSBindings pointer
    static std::unordered_map<void*, dong::dom::Selection> s_selections;
    return &s_selections[bindings];
}

// ============================================================
// Range JS class
// ============================================================

static JSClassID js_range_class_id = 0;

static void js_range_finalizer(JSRuntime* rt, JSValue val) {
    (void)rt;
    auto* range = static_cast<dong::dom::Range*>(JS_GetOpaque(val, js_range_class_id));
    delete range;
}

static JSClassDef js_range_class = {
    "Range",                  // class_name
    js_range_finalizer,       // finalizer
};

static JSValue js_range_ctor(JSContext* ctx, JSValueConst new_target,
                              int argc, JSValueConst* argv) {
    (void)new_target; (void)argc; (void)argv;

    JSValue obj = JS_NewObjectClass(ctx, js_range_class_id);
    if (JS_IsException(obj)) return obj;

    auto* range = new dong::dom::Range();
    JS_SetOpaque(obj, range);
    return obj;
}

static dong::dom::Range* getRange(JSContext* ctx, JSValueConst this_val) {
    return static_cast<dong::dom::Range*>(JS_GetOpaque(this_val, js_range_class_id));
}

static JSValue js_range_get_collapsed(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    return r ? JS_NewBool(ctx, r->isCollapsed()) : JS_FALSE;
}

static JSValue js_range_get_startOffset(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    return r ? JS_NewInt32(ctx, r->getStartOffset()) : JS_NewInt32(ctx, 0);
}

static JSValue js_range_get_endOffset(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    return r ? JS_NewInt32(ctx, r->getEndOffset()) : JS_NewInt32(ctx, 0);
}

static JSValue js_range_get_startContainer(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    if (!r || !r->getStartContainer()) return JS_NULL;
    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, r->getStartContainer());
}

static JSValue js_range_get_endContainer(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    if (!r || !r->getEndContainer()) return JS_NULL;
    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, r->getEndContainer());
}

static JSValue js_range_setStart(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto* r = getRange(ctx, this_val);
    if (!r || argc < 2) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, argv[0]);
    int32_t offset = 0;
    JS_ToInt32(ctx, &offset, argv[1]);
    r->setStart(node, static_cast<uint32_t>(offset));
    return JS_UNDEFINED;
}

static JSValue js_range_setEnd(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto* r = getRange(ctx, this_val);
    if (!r || argc < 2) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, argv[0]);
    int32_t offset = 0;
    JS_ToInt32(ctx, &offset, argv[1]);
    r->setEnd(node, static_cast<uint32_t>(offset));
    return JS_UNDEFINED;
}

static JSValue js_range_collapse(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto* r = getRange(ctx, this_val);
    if (!r) return JS_UNDEFINED;
    bool to_start = true;
    if (argc > 0) to_start = JS_ToBool(ctx, argv[0]);
    r->collapse(to_start);
    return JS_UNDEFINED;
}

static JSValue js_range_selectNodeContents(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto* r = getRange(ctx, this_val);
    if (!r || argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, argv[0]);
    r->selectNodeContents(node);
    return JS_UNDEFINED;
}

static JSValue js_range_selectNode(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto* r = getRange(ctx, this_val);
    if (!r || argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, argv[0]);
    r->selectNode(node);
    return JS_UNDEFINED;
}

static JSValue js_range_toString(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    if (!r) return JS_NewString(ctx, "");
    return JS_NewString(ctx, r->toString().c_str());
}

static JSValue js_range_cloneRange(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    if (!r) return JS_NULL;

    JSValue obj = JS_NewObjectClass(ctx, js_range_class_id);
    if (JS_IsException(obj)) return obj;
    auto* clone = new dong::dom::Range(r->cloneRange());
    JS_SetOpaque(obj, clone);
    return obj;
}

static JSValue js_range_deleteContents(JSContext* ctx, JSValueConst this_val, int, JSValueConst*) {
    auto* r = getRange(ctx, this_val);
    if (r) r->deleteContents();
    return JS_UNDEFINED;
}

// ============================================================
// Selection JS API
// ============================================================

static JSValue js_getSelection(JSContext* ctx, JSValueConst, int, JSValueConst*) {
    auto* sel = getViewSelection(ctx);
    if (!sel) return JS_NULL;

    JSValue obj = JS_NewObject(ctx);

    // anchorNode, anchorOffset, focusNode, focusOffset
    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));

    auto bind_sel_prop = [&](const char* name, auto getter) {
        JS_SetPropertyStr(ctx, obj, name, JS_NewCFunction(ctx, getter, name, 0));
    };

    // rangeCount
    JS_SetPropertyStr(ctx, obj, "rangeCount", JS_NewInt32(ctx, sel->getRangeCount()));
    JS_SetPropertyStr(ctx, obj, "isCollapsed", JS_NewBool(ctx, sel->isCollapsed()));

    // toString()
    auto sel_toString = [](JSContext* c, JSValueConst, int, JSValueConst*) -> JSValue {
        auto* s = getViewSelection(c);
        return s ? JS_NewString(c, s->toString().c_str()) : JS_NewString(c, "");
    };
    JS_SetPropertyStr(ctx, obj, "toString", JS_NewCFunction(ctx, sel_toString, "toString", 0));

    // getRangeAt(index)
    auto sel_getRangeAt = [](JSContext* c, JSValueConst, int argc, JSValueConst* argv) -> JSValue {
        auto* s = getViewSelection(c);
        if (!s || argc < 1) return JS_NULL;
        int32_t idx = 0;
        JS_ToInt32(c, &idx, argv[0]);
        auto* range = s->getRangeAt(static_cast<uint32_t>(idx));
        if (!range) return JS_NULL;

        JSValue obj = JS_NewObjectClass(c, js_range_class_id);
        if (JS_IsException(obj)) return obj;
        auto* clone = new dong::dom::Range(range->cloneRange());
        JS_SetOpaque(obj, clone);
        return obj;
    };
    JS_SetPropertyStr(ctx, obj, "getRangeAt", JS_NewCFunction(ctx, sel_getRangeAt, "getRangeAt", 1));

    // addRange(range)
    auto sel_addRange = [](JSContext* c, JSValueConst, int argc, JSValueConst* argv) -> JSValue {
        auto* s = getViewSelection(c);
        if (!s || argc < 1) return JS_UNDEFINED;
        auto* range = static_cast<dong::dom::Range*>(JS_GetOpaque(argv[0], js_range_class_id));
        if (range) s->addRange(*range);
        return JS_UNDEFINED;
    };
    JS_SetPropertyStr(ctx, obj, "addRange", JS_NewCFunction(ctx, sel_addRange, "addRange", 1));

    // removeAllRanges()
    auto sel_removeAllRanges = [](JSContext* c, JSValueConst, int, JSValueConst*) -> JSValue {
        auto* s = getViewSelection(c);
        if (s) s->removeAllRanges();
        return JS_UNDEFINED;
    };
    JS_SetPropertyStr(ctx, obj, "removeAllRanges", JS_NewCFunction(ctx, sel_removeAllRanges, "removeAllRanges", 0));

    // collapse(node, offset)
    auto sel_collapse = [](JSContext* c, JSValueConst, int argc, JSValueConst* argv) -> JSValue {
        auto* s = getViewSelection(c);
        if (!s || argc < 2) return JS_UNDEFINED;
        auto node = JSBindings::getNodeOpaque(c, argv[0]);
        int32_t offset = 0;
        JS_ToInt32(c, &offset, argv[1]);
        s->collapse(node, static_cast<uint32_t>(offset));
        return JS_UNDEFINED;
    };
    JS_SetPropertyStr(ctx, obj, "collapse", JS_NewCFunction(ctx, sel_collapse, "collapse", 2));

    // selectAllChildren(node)
    auto sel_selectAllChildren = [](JSContext* c, JSValueConst, int argc, JSValueConst* argv) -> JSValue {
        auto* s = getViewSelection(c);
        if (!s || argc < 1) return JS_UNDEFINED;
        auto node = JSBindings::getNodeOpaque(c, argv[0]);
        s->selectAllChildren(node);
        return JS_UNDEFINED;
    };
    JS_SetPropertyStr(ctx, obj, "selectAllChildren", JS_NewCFunction(ctx, sel_selectAllChildren, "selectAllChildren", 1));

    return obj;
}

// ============================================================
// document.createRange()
// ============================================================

static JSValue js_createRange(JSContext* ctx, JSValueConst, int, JSValueConst*) {
    JSValue obj = JS_NewObjectClass(ctx, js_range_class_id);
    if (JS_IsException(obj)) return obj;
    auto* range = new dong::dom::Range();
    JS_SetOpaque(obj, range);
    return obj;
}

// ============================================================
// Initialization
// ============================================================

void initializeSelectionAPI(JSContext* ctx, JSBindings* bindings) {
    (void)bindings;
    if (!ctx) return;

    // Register Range class
    JS_NewClassID(&js_range_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_range_class_id, &js_range_class);

    // Set Range prototype
    JSValue range_proto = JS_NewObject(ctx);

    // Range getters
    JSAtom collapsed_atom = JS_NewAtom(ctx, "collapsed");
    JS_DefinePropertyGetSet(ctx, range_proto, collapsed_atom,
        JS_NewCFunction(ctx, js_range_get_collapsed, "get collapsed", 0), JS_UNDEFINED,
        JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE);
    JS_FreeAtom(ctx, collapsed_atom);

    JSAtom startOffset_atom = JS_NewAtom(ctx, "startOffset");
    JS_DefinePropertyGetSet(ctx, range_proto, startOffset_atom,
        JS_NewCFunction(ctx, js_range_get_startOffset, "get startOffset", 0), JS_UNDEFINED,
        JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE);
    JS_FreeAtom(ctx, startOffset_atom);

    JSAtom endOffset_atom = JS_NewAtom(ctx, "endOffset");
    JS_DefinePropertyGetSet(ctx, range_proto, endOffset_atom,
        JS_NewCFunction(ctx, js_range_get_endOffset, "get endOffset", 0), JS_UNDEFINED,
        JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE);
    JS_FreeAtom(ctx, endOffset_atom);

    JSAtom startContainer_atom = JS_NewAtom(ctx, "startContainer");
    JS_DefinePropertyGetSet(ctx, range_proto, startContainer_atom,
        JS_NewCFunction(ctx, js_range_get_startContainer, "get startContainer", 0), JS_UNDEFINED,
        JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE);
    JS_FreeAtom(ctx, startContainer_atom);

    JSAtom endContainer_atom = JS_NewAtom(ctx, "endContainer");
    JS_DefinePropertyGetSet(ctx, range_proto, endContainer_atom,
        JS_NewCFunction(ctx, js_range_get_endContainer, "get endContainer", 0), JS_UNDEFINED,
        JS_PROP_CONFIGURABLE | JS_PROP_ENUMERABLE);
    JS_FreeAtom(ctx, endContainer_atom);

    // Range methods
    JS_SetPropertyStr(ctx, range_proto, "setStart", JS_NewCFunction(ctx, js_range_setStart, "setStart", 2));
    JS_SetPropertyStr(ctx, range_proto, "setEnd", JS_NewCFunction(ctx, js_range_setEnd, "setEnd", 2));
    JS_SetPropertyStr(ctx, range_proto, "collapse", JS_NewCFunction(ctx, js_range_collapse, "collapse", 1));
    JS_SetPropertyStr(ctx, range_proto, "selectNodeContents", JS_NewCFunction(ctx, js_range_selectNodeContents, "selectNodeContents", 1));
    JS_SetPropertyStr(ctx, range_proto, "selectNode", JS_NewCFunction(ctx, js_range_selectNode, "selectNode", 1));
    JS_SetPropertyStr(ctx, range_proto, "toString", JS_NewCFunction(ctx, js_range_toString, "toString", 0));
    JS_SetPropertyStr(ctx, range_proto, "cloneRange", JS_NewCFunction(ctx, js_range_cloneRange, "cloneRange", 0));
    JS_SetPropertyStr(ctx, range_proto, "deleteContents", JS_NewCFunction(ctx, js_range_deleteContents, "deleteContents", 0));

    JS_SetClassProto(ctx, js_range_class_id, range_proto);

    // Register on globals
    JSValue global = JS_GetGlobalObject(ctx);

    // Range constructor
    JSValue range_ctor = JS_NewCFunction2(ctx, js_range_ctor, "Range", 0, JS_CFUNC_constructor, 0);
    JS_SetPropertyStr(ctx, global, "Range", range_ctor);

    // window.getSelection()
    JS_SetPropertyStr(ctx, global, "getSelection", JS_NewCFunction(ctx, js_getSelection, "getSelection", 0));

    // document.getSelection() and document.createRange()
    JSValue document = JS_GetPropertyStr(ctx, global, "document");
    if (!JS_IsUndefined(document) && !JS_IsException(document)) {
        JS_SetPropertyStr(ctx, document, "getSelection", JS_NewCFunction(ctx, js_getSelection, "getSelection", 0));
        JS_SetPropertyStr(ctx, document, "createRange", JS_NewCFunction(ctx, js_createRange, "createRange", 0));
    }
    JS_FreeValue(ctx, document);
    JS_FreeValue(ctx, global);
}

} // namespace dong::script
