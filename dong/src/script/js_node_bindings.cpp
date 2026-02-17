#include "js_node_bindings.hpp"
#include "js_bindings.hpp"
#include "../core/log.h"
#include "../dom/input_element.hpp"

#include <string>
#include <algorithm>
#include <cctype>

extern "C" {
#include "quickjs.h"
}

// Helper to get JSBindings from context opaque data
static dong::script::JSBindings* getBindingsFromCtx(JSContext* ctx) {
    if (!ctx) return nullptr;
    return static_cast<dong::script::JSBindings*>(JS_GetContextOpaque(ctx));
}

namespace dong::script {

// ============================================================
// Node interface getters
// ============================================================

static JSValue elem_getParentNode(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto parent = node->getParentNode();
    if (!parent) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, parent);
}

static JSValue elem_getParentElement(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto parent = node->getParentElement();
    if (!parent) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, parent);
}

static JSValue elem_getChildNodes(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewArray(ctx);
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NewArray(ctx);
    JSValue arr = JS_NewArray(ctx);
    const auto& children = node->getChildNodes();
    for (size_t i = 0; i < children.size(); ++i) {
        JS_SetPropertyUint32(ctx, arr, static_cast<uint32_t>(i), bindings->createJSElement(ctx, children[i]));
    }
    return arr;
}

static JSValue elem_getChildren(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewArray(ctx);
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NewArray(ctx);
    JSValue arr = JS_NewArray(ctx);
    uint32_t idx = 0;
    for (const auto& child : node->getChildren()) {
        if (child && child->getNodeType() == dom::DOMNode::NodeType::ELEMENT) {
            JS_SetPropertyUint32(ctx, arr, idx++, bindings->createJSElement(ctx, child));
        }
    }
    return arr;
}

static JSValue elem_getFirstChild(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto first = node->getFirstChild();
    if (!first) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, first);
}

static JSValue elem_getLastChild(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto last = node->getLastChild();
    if (!last) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, last);
}

static JSValue elem_getNextSibling(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto sib = node->getNextSibling();
    if (!sib) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, sib);
}

static JSValue elem_getPreviousSibling(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto sib = node->getPreviousSibling();
    if (!sib) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, sib);
}

static JSValue elem_getNextElementSibling(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto sib = node->getNextElementSibling();
    if (!sib) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, sib);
}

static JSValue elem_getPreviousElementSibling(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto sib = node->getPreviousElementSibling();
    if (!sib) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, sib);
}

static JSValue elem_getNodeType(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    return JS_NewInt32(ctx, static_cast<int>(node->getNodeType()));
}

static JSValue elem_getNodeName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    std::string name = node->getNodeName();
    // Element node names are uppercase in HTML
    if (node->getNodeType() == dom::DOMNode::NodeType::ELEMENT) {
        std::transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    }
    return JS_NewString(ctx, name.c_str());
}

static JSValue elem_getNodeValue(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    if (node->getNodeType() == dom::DOMNode::NodeType::TEXT) {
        return JS_NewString(ctx, node->getTextContent().c_str());
    }
    return JS_NULL;
}

static JSValue elem_getChildElementCount(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewInt32(ctx, 0);
    return JS_NewInt32(ctx, static_cast<int32_t>(node->getChildElementCount()));
}

static JSValue elem_insertBefore(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_UNDEFINED;
    auto parent = JSBindings::getNodeOpaque(ctx, this_val);
    auto newChild = JSBindings::getNodeOpaque(ctx, argv[0]);
    dom::DOMNodePtr refChild;
    if (!JS_IsNull(argv[1]) && !JS_IsUndefined(argv[1])) {
        refChild = JSBindings::getNodeOpaque(ctx, argv[1]);
    }
    if (parent && newChild) {
        parent->insertBefore(newChild, refChild);
        return JS_DupValue(ctx, argv[0]);
    }
    return JS_UNDEFINED;
}

static JSValue elem_hasChildNodes(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    return JS_NewBool(ctx, node->hasChildNodes());
}

static JSValue elem_contains(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    auto other = JSBindings::getNodeOpaque(ctx, argv[0]);
    if (!node || !other) return JS_FALSE;
    return JS_NewBool(ctx, node->contains(other));
}

static JSValue elem_cloneNode(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    bool deep = false;
    if (argc > 0) {
        deep = JS_ToBool(ctx, argv[0]) != 0;
    }
    auto clone = node->cloneNode(deep);
    if (!clone) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, clone);
}

// ============================================================
// Element interface methods
// ============================================================

static JSValue elem_querySelector(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_NULL;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NULL;
    auto found = node->querySelector(selector);
    JS_FreeCString(ctx, selector);
    if (!found) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, found);
}

static JSValue elem_querySelectorAll(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_NewArray(ctx);
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewArray(ctx);
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NewArray(ctx);
    auto results = node->querySelectorAll(selector);
    JS_FreeCString(ctx, selector);
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NewArray(ctx);
    JSValue arr = JS_NewArray(ctx);
    for (size_t i = 0; i < results.size(); ++i) {
        JS_SetPropertyUint32(ctx, arr, static_cast<uint32_t>(i), bindings->createJSElement(ctx, results[i]));
    }
    return arr;
}

static JSValue elem_matches(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_FALSE;
    bool result = node->matches(selector);
    JS_FreeCString(ctx, selector);
    return JS_NewBool(ctx, result);
}

static JSValue elem_closest(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_NULL;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NULL;
    auto found = node->closest(selector);
    JS_FreeCString(ctx, selector);
    if (!found) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, found);
}

static JSValue elem_getBoundingClientRect(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) {
        JSValue obj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, 0));
        JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, 0));
        JS_SetPropertyStr(ctx, obj, "width", JS_NewFloat64(ctx, 0));
        JS_SetPropertyStr(ctx, obj, "height", JS_NewFloat64(ctx, 0));
        JS_SetPropertyStr(ctx, obj, "top", JS_NewFloat64(ctx, 0));
        JS_SetPropertyStr(ctx, obj, "right", JS_NewFloat64(ctx, 0));
        JS_SetPropertyStr(ctx, obj, "bottom", JS_NewFloat64(ctx, 0));
        JS_SetPropertyStr(ctx, obj, "left", JS_NewFloat64(ctx, 0));
        return obj;
    }
    auto rect = node->getBoundingClientRect();
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "x", JS_NewFloat64(ctx, rect.x));
    JS_SetPropertyStr(ctx, obj, "y", JS_NewFloat64(ctx, rect.y));
    JS_SetPropertyStr(ctx, obj, "width", JS_NewFloat64(ctx, rect.width));
    JS_SetPropertyStr(ctx, obj, "height", JS_NewFloat64(ctx, rect.height));
    JS_SetPropertyStr(ctx, obj, "top", JS_NewFloat64(ctx, rect.top));
    JS_SetPropertyStr(ctx, obj, "right", JS_NewFloat64(ctx, rect.right));
    JS_SetPropertyStr(ctx, obj, "bottom", JS_NewFloat64(ctx, rect.bottom));
    JS_SetPropertyStr(ctx, obj, "left", JS_NewFloat64(ctx, rect.left));
    return obj;
}

static JSValue elem_remove(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (node) {
        node->remove();
    }
    return JS_UNDEFINED;
}

static JSValue elem_dispatchEvent(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;

    // Read type from the event object
    JSValue type_val = JS_GetPropertyStr(ctx, argv[0], "type");
    const char* type = JS_ToCString(ctx, type_val);
    JS_FreeValue(ctx, type_val);
    if (!type) return JS_FALSE;

    auto* bindings = getBindingsFromCtx(ctx);
    if (bindings) {
        uint64_t nid = bindings->getNodeIdFor(node);
        if (!nid) {
            // Ensure the node has an ID
            JSValue tmp = bindings->createJSElement(ctx, node);
            JS_FreeValue(ctx, tmp);
            nid = bindings->getNodeIdFor(node);
        }
        if (nid) {
            bindings->dispatchSimpleEvent(nid, type);
        }
    }

    JS_FreeCString(ctx, type);
    return JS_TRUE;
}

static JSValue elem_getHidden(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    return JS_NewBool(ctx, node->hasAttribute("hidden"));
}

static JSValue elem_setHidden(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    if (JS_ToBool(ctx, argv[0])) {
        node->setAttribute("hidden", "");
    } else {
        node->removeAttribute("hidden");
    }
    node->markStyleDirty();
    node->markLayoutDirty();
    return JS_UNDEFINED;
}

static JSValue elem_hasAttribute(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_FALSE;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_FALSE;
    bool result = node->hasAttribute(name);
    JS_FreeCString(ctx, name);
    return JS_NewBool(ctx, result);
}

static JSValue elem_removeAttribute(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) return JS_UNDEFINED;
    node->removeAttribute(name);
    JS_FreeCString(ctx, name);
    return JS_UNDEFINED;
}

// ============================================================
// dataset Proxy
// ============================================================

static std::string dataAttrToDataset(const std::string& attr) {
    // "data-foo-bar" -> "fooBar"
    if (attr.size() <= 5 || attr.substr(0, 5) != "data-") return attr;
    std::string result;
    bool capitalize_next = false;
    for (size_t i = 5; i < attr.size(); ++i) {
        if (attr[i] == '-') {
            capitalize_next = true;
        } else {
            if (capitalize_next) {
                result += static_cast<char>(std::toupper(static_cast<unsigned char>(attr[i])));
                capitalize_next = false;
            } else {
                result += attr[i];
            }
        }
    }
    return result;
}

static std::string datasetToDataAttr(const std::string& prop) {
    // "fooBar" -> "data-foo-bar"
    std::string result = "data-";
    for (char c : prop) {
        if (std::isupper(static_cast<unsigned char>(c))) {
            result += '-';
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else {
            result += c;
        }
    }
    return result;
}

static JSValue dataset_proxy_get(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc < 2) return JS_UNDEFINED;
    JSValueConst target = argv[0];
    JSValueConst prop = argv[1];

    if (JS_IsSymbol(prop)) {
        JSAtom atom = JS_ValueToAtom(ctx, prop);
        JSValue val = JS_GetProperty(ctx, target, atom);
        JS_FreeAtom(ctx, atom);
        return val;
    }

    const char* prop_cstr = JS_ToCString(ctx, prop);
    if (!prop_cstr) return JS_UNDEFINED;
    std::string prop_str(prop_cstr);
    JS_FreeCString(ctx, prop_cstr);

    if (prop_str == "__element__") {
        return JS_GetPropertyStr(ctx, target, "__element__");
    }

    JSValue element = JS_GetPropertyStr(ctx, target, "__element__");
    auto node = JSBindings::getNodeOpaque(ctx, element);
    JS_FreeValue(ctx, element);
    if (!node) return JS_UNDEFINED;

    std::string data_attr = datasetToDataAttr(prop_str);
    if (node->hasAttribute(data_attr)) {
        return JS_NewString(ctx, node->getAttribute(data_attr).c_str());
    }
    return JS_UNDEFINED;
}

static JSValue dataset_proxy_set(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    if (argc < 3) return JS_FALSE;
    JSValueConst target = argv[0];
    JSValueConst prop = argv[1];
    JSValueConst value = argv[2];

    if (JS_IsSymbol(prop)) return JS_TRUE;

    const char* prop_cstr = JS_ToCString(ctx, prop);
    if (!prop_cstr) return JS_FALSE;
    std::string prop_str(prop_cstr);
    JS_FreeCString(ctx, prop_cstr);

    JSValue element = JS_GetPropertyStr(ctx, target, "__element__");
    auto node = JSBindings::getNodeOpaque(ctx, element);
    JS_FreeValue(ctx, element);
    if (!node) return JS_FALSE;

    std::string data_attr = datasetToDataAttr(prop_str);
    const char* val_str = JS_ToCString(ctx, value);
    if (val_str) {
        node->setAttribute(data_attr, val_str);
        JS_FreeCString(ctx, val_str);
    }
    return JS_TRUE;
}

static JSValue createDatasetProxy(JSContext* ctx, JSValueConst element, const dom::DOMNodePtr& node) {
    JSValue target = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, target, "__element__", JS_DupValue(ctx, element));

    // Seed existing data-* attributes
    if (node) {
        for (const auto& attr : node->getAttributes()) {
            if (attr.first.size() > 5 && attr.first.substr(0, 5) == "data-") {
                std::string key = dataAttrToDataset(attr.first);
                JS_SetPropertyStr(ctx, target, key.c_str(), JS_NewString(ctx, attr.second.c_str()));
            }
        }
    }

    JSValue handler = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, handler, "get", JS_NewCFunction(ctx, dataset_proxy_get, "dataset_get", 3));
    JS_SetPropertyStr(ctx, handler, "set", JS_NewCFunction(ctx, dataset_proxy_set, "dataset_set", 4));

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue proxy_ctor = JS_GetPropertyStr(ctx, global, "Proxy");
    JS_FreeValue(ctx, global);

    JSValue args[2] = { target, handler };
    JSValue proxy = JS_CallConstructor(ctx, proxy_ctor, 2, args);
    JS_FreeValue(ctx, proxy_ctor);
    JS_FreeValue(ctx, target);
    JS_FreeValue(ctx, handler);
    return proxy;
}

// ============================================================
// HTMLElement-specific bindings (input, textarea, select)
// ============================================================

static JSValue input_getValue(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewString(ctx, "");
    // First check if there's an InputElementState
    auto* state = dom::getInputState(node);
    if (state) {
        return JS_NewString(ctx, state->getValue().c_str());
    }
    return JS_NewString(ctx, node->getAttribute("value").c_str());
}

static JSValue input_setValue(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    const char* val = JS_ToCString(ctx, argv[0]);
    if (val) {
        node->setAttribute("value", val);
        auto* state = dom::getInputState(node);
        if (state) {
            state->setValue(val);
        }
        JS_FreeCString(ctx, val);
        node->markLayoutDirty();
    }
    return JS_UNDEFINED;
}

static JSValue input_getChecked(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    return JS_NewBool(ctx, node->hasAttribute("checked"));
}

static JSValue input_setChecked(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    if (JS_ToBool(ctx, argv[0])) {
        node->setAttribute("checked", "");
    } else {
        node->removeAttribute("checked");
    }
    return JS_UNDEFINED;
}

static JSValue input_getDisabled(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_FALSE;
    return JS_NewBool(ctx, node->hasAttribute("disabled"));
}

static JSValue input_setDisabled(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    if (JS_ToBool(ctx, argv[0])) {
        node->setAttribute("disabled", "");
    } else {
        node->removeAttribute("disabled");
    }
    return JS_UNDEFINED;
}

static JSValue input_getType(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewString(ctx, "text");
    std::string type = node->getAttribute("type");
    return JS_NewString(ctx, type.empty() ? "text" : type.c_str());
}

static JSValue input_setType(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    const char* val = JS_ToCString(ctx, argv[0]);
    if (val) {
        node->setAttribute("type", val);
        JS_FreeCString(ctx, val);
    }
    return JS_UNDEFINED;
}

static JSValue input_getName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewString(ctx, "");
    return JS_NewString(ctx, node->getAttribute("name").c_str());
}

static JSValue input_setName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    const char* val = JS_ToCString(ctx, argv[0]);
    if (val) {
        node->setAttribute("name", val);
        JS_FreeCString(ctx, val);
    }
    return JS_UNDEFINED;
}

// Scroll properties
static JSValue elem_getScrollTop(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getScrollTop());
}

static JSValue elem_getScrollLeft(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getScrollLeft());
}

static JSValue elem_getScrollWidth(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getScrollWidth());
}

static JSValue elem_getScrollHeight(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getScrollHeight());
}

static JSValue elem_getClientWidth(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getClientWidth());
}

static JSValue elem_getClientHeight(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getClientHeight());
}

static JSValue elem_getOffsetWidth(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getOffsetWidth());
}

static JSValue elem_getOffsetHeight(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getOffsetHeight());
}

static JSValue elem_getOffsetTop(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getOffsetTop());
}

static JSValue elem_getOffsetLeft(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewFloat64(ctx, 0);
    return JS_NewFloat64(ctx, node->getOffsetLeft());
}

// ============================================================
// Helper macro for getter/setter property definition
// ============================================================
#define DEFINE_GETTER(ctx, elem, name, fn) \
    do { \
        JSAtom atom = JS_NewAtom(ctx, name); \
        JSValue g = JS_NewCFunction(ctx, fn, "get " name, 0); \
        JS_DefinePropertyGetSet(ctx, elem, atom, g, JS_UNDEFINED, \
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE); \
        JS_FreeAtom(ctx, atom); \
    } while(0)

#define DEFINE_GETTER_SETTER(ctx, elem, name, getter_fn, setter_fn) \
    do { \
        JSAtom atom = JS_NewAtom(ctx, name); \
        JSValue g = JS_NewCFunction(ctx, getter_fn, "get " name, 0); \
        JSValue s = JS_NewCFunction(ctx, setter_fn, "set " name, 1); \
        JS_DefinePropertyGetSet(ctx, elem, atom, g, s, \
            JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE); \
        JS_FreeAtom(ctx, atom); \
    } while(0)

// ============================================================
// Public binding functions
// ============================================================

void bindNodeProperties(JSContext* ctx, JSValue elem, const dom::DOMNodePtr& node, JSBindings* bindings) {
    (void)bindings;

    // Tree traversal getters
    DEFINE_GETTER(ctx, elem, "parentNode", elem_getParentNode);
    DEFINE_GETTER(ctx, elem, "parentElement", elem_getParentElement);
    DEFINE_GETTER(ctx, elem, "childNodes", elem_getChildNodes);
    DEFINE_GETTER(ctx, elem, "children", elem_getChildren);
    DEFINE_GETTER(ctx, elem, "firstChild", elem_getFirstChild);
    DEFINE_GETTER(ctx, elem, "lastChild", elem_getLastChild);
    DEFINE_GETTER(ctx, elem, "nextSibling", elem_getNextSibling);
    DEFINE_GETTER(ctx, elem, "previousSibling", elem_getPreviousSibling);
    DEFINE_GETTER(ctx, elem, "nextElementSibling", elem_getNextElementSibling);
    DEFINE_GETTER(ctx, elem, "previousElementSibling", elem_getPreviousElementSibling);
    DEFINE_GETTER(ctx, elem, "nodeType", elem_getNodeType);
    DEFINE_GETTER(ctx, elem, "nodeName", elem_getNodeName);
    DEFINE_GETTER(ctx, elem, "nodeValue", elem_getNodeValue);
    DEFINE_GETTER(ctx, elem, "childElementCount", elem_getChildElementCount);

    // Node methods
    JS_SetPropertyStr(ctx, elem, "insertBefore",
        JS_NewCFunction(ctx, elem_insertBefore, "insertBefore", 2));
    JS_SetPropertyStr(ctx, elem, "hasChildNodes",
        JS_NewCFunction(ctx, elem_hasChildNodes, "hasChildNodes", 0));
    JS_SetPropertyStr(ctx, elem, "contains",
        JS_NewCFunction(ctx, elem_contains, "contains", 1));
    JS_SetPropertyStr(ctx, elem, "cloneNode",
        JS_NewCFunction(ctx, elem_cloneNode, "cloneNode", 1));
}

void bindElementProperties(JSContext* ctx, JSValue elem, const dom::DOMNodePtr& node, JSBindings* bindings) {
    (void)bindings;

    // Element query methods
    JS_SetPropertyStr(ctx, elem, "querySelector",
        JS_NewCFunction(ctx, elem_querySelector, "querySelector", 1));
    JS_SetPropertyStr(ctx, elem, "querySelectorAll",
        JS_NewCFunction(ctx, elem_querySelectorAll, "querySelectorAll", 1));
    JS_SetPropertyStr(ctx, elem, "matches",
        JS_NewCFunction(ctx, elem_matches, "matches", 1));
    JS_SetPropertyStr(ctx, elem, "closest",
        JS_NewCFunction(ctx, elem_closest, "closest", 1));
    JS_SetPropertyStr(ctx, elem, "getBoundingClientRect",
        JS_NewCFunction(ctx, elem_getBoundingClientRect, "getBoundingClientRect", 0));
    JS_SetPropertyStr(ctx, elem, "remove",
        JS_NewCFunction(ctx, elem_remove, "remove", 0));
    JS_SetPropertyStr(ctx, elem, "dispatchEvent",
        JS_NewCFunction(ctx, elem_dispatchEvent, "dispatchEvent", 1));
    JS_SetPropertyStr(ctx, elem, "hasAttribute",
        JS_NewCFunction(ctx, elem_hasAttribute, "hasAttribute", 1));
    JS_SetPropertyStr(ctx, elem, "removeAttribute",
        JS_NewCFunction(ctx, elem_removeAttribute, "removeAttribute", 1));

    // hidden property
    DEFINE_GETTER_SETTER(ctx, elem, "hidden", elem_getHidden, elem_setHidden);

    // dataset proxy
    JS_SetPropertyStr(ctx, elem, "dataset", createDatasetProxy(ctx, elem, node));

    // Scroll/geometry getters
    DEFINE_GETTER(ctx, elem, "scrollTop", elem_getScrollTop);
    DEFINE_GETTER(ctx, elem, "scrollLeft", elem_getScrollLeft);
    DEFINE_GETTER(ctx, elem, "scrollWidth", elem_getScrollWidth);
    DEFINE_GETTER(ctx, elem, "scrollHeight", elem_getScrollHeight);
    DEFINE_GETTER(ctx, elem, "clientWidth", elem_getClientWidth);
    DEFINE_GETTER(ctx, elem, "clientHeight", elem_getClientHeight);
    DEFINE_GETTER(ctx, elem, "offsetWidth", elem_getOffsetWidth);
    DEFINE_GETTER(ctx, elem, "offsetHeight", elem_getOffsetHeight);
    DEFINE_GETTER(ctx, elem, "offsetTop", elem_getOffsetTop);
    DEFINE_GETTER(ctx, elem, "offsetLeft", elem_getOffsetLeft);
}

void bindHTMLElementProperties(JSContext* ctx, JSValue elem, const dom::DOMNodePtr& node, JSBindings* bindings) {
    (void)bindings;
    if (!node) return;

    const std::string& tag = node->getTagName();
    if (tag == "input" || tag == "textarea") {
        DEFINE_GETTER_SETTER(ctx, elem, "value", input_getValue, input_setValue);
        DEFINE_GETTER_SETTER(ctx, elem, "checked", input_getChecked, input_setChecked);
        DEFINE_GETTER_SETTER(ctx, elem, "disabled", input_getDisabled, input_setDisabled);
        DEFINE_GETTER_SETTER(ctx, elem, "type", input_getType, input_setType);
        DEFINE_GETTER_SETTER(ctx, elem, "name", input_getName, input_setName);
    } else if (tag == "select") {
        // Basic select stubs
        DEFINE_GETTER_SETTER(ctx, elem, "value", input_getValue, input_setValue);
        DEFINE_GETTER_SETTER(ctx, elem, "disabled", input_getDisabled, input_setDisabled);
        DEFINE_GETTER_SETTER(ctx, elem, "name", input_getName, input_setName);
    }
}

void initializeNodeConstants(JSContext* ctx) {
    JSValue global = JS_GetGlobalObject(ctx);

    JSValue node_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, node_obj, "ELEMENT_NODE", JS_NewInt32(ctx, 1));
    JS_SetPropertyStr(ctx, node_obj, "ATTRIBUTE_NODE", JS_NewInt32(ctx, 2));
    JS_SetPropertyStr(ctx, node_obj, "TEXT_NODE", JS_NewInt32(ctx, 3));
    JS_SetPropertyStr(ctx, node_obj, "CDATA_SECTION_NODE", JS_NewInt32(ctx, 4));
    JS_SetPropertyStr(ctx, node_obj, "PROCESSING_INSTRUCTION_NODE", JS_NewInt32(ctx, 7));
    JS_SetPropertyStr(ctx, node_obj, "COMMENT_NODE", JS_NewInt32(ctx, 8));
    JS_SetPropertyStr(ctx, node_obj, "DOCUMENT_NODE", JS_NewInt32(ctx, 9));
    JS_SetPropertyStr(ctx, node_obj, "DOCUMENT_TYPE_NODE", JS_NewInt32(ctx, 10));
    JS_SetPropertyStr(ctx, node_obj, "DOCUMENT_FRAGMENT_NODE", JS_NewInt32(ctx, 11));

    JS_SetPropertyStr(ctx, global, "Node", node_obj);
    JS_FreeValue(ctx, global);
}

#undef DEFINE_GETTER
#undef DEFINE_GETTER_SETTER

} // namespace dong::script
