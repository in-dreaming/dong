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

static JSValue elem_getFirstElementChild(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto first = node->getFirstElementChild();
    if (!first) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, first);
}

static JSValue elem_getLastElementChild(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NULL;
    auto last = node->getLastElementChild();
    if (!last) return JS_NULL;
    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NULL;
    return bindings->createJSElement(ctx, last);
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
// Modern DOM manipulation methods (Element.before/after/etc.)
// ============================================================

// Helper to convert JSValue to DOM node or text node
static dom::DOMNodePtr convertToNode(JSContext* ctx, JSValueConst val, dong::dom::Manager* dom_mgr) {
    (void)dom_mgr; // Not needed since we create text nodes directly

    // If it's already a node, return it
    auto node = JSBindings::getNodeOpaque(ctx, val);
    if (node) return node;

    // If it's a string, create a text node
    if (JS_IsString(val)) {
        const char* str = JS_ToCString(ctx, val);
        if (str) {
            auto text_node = std::make_shared<dom::DOMNode>(dom::DOMNode::NodeType::TEXT, "");
            text_node->setNodeValue(str);
            JS_FreeCString(ctx, str);
            return text_node;
        }
    }

    return nullptr;
}

static JSValue elem_before(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    auto parent = node->getParentNode();
    if (!parent) return JS_UNDEFINED;

    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings || !bindings->dom_manager_) return JS_UNDEFINED;

    // Insert each argument before this node
    for (int i = 0; i < argc; ++i) {
        auto newNode = convertToNode(ctx, argv[i], bindings->dom_manager_);
        if (newNode) {
            parent->insertBefore(newNode, node);
        }
    }

    return JS_UNDEFINED;
}

static JSValue elem_after(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    auto parent = node->getParentNode();
    if (!parent) return JS_UNDEFINED;

    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings || !bindings->dom_manager_) return JS_UNDEFINED;

    // Find the next sibling to insert before
    auto nextSibling = node->getNextSibling();

    // Insert each argument after this node
    for (int i = 0; i < argc; ++i) {
        auto newNode = convertToNode(ctx, argv[i], bindings->dom_manager_);
        if (newNode) {
            parent->insertBefore(newNode, nextSibling);
        }
    }

    return JS_UNDEFINED;
}

static JSValue elem_replaceWith(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    auto parent = node->getParentNode();
    if (!parent) return JS_UNDEFINED;

    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings || !bindings->dom_manager_) return JS_UNDEFINED;

    auto nextSibling = node->getNextSibling();

    // Insert all new nodes before this node
    for (int i = 0; i < argc; ++i) {
        auto newNode = convertToNode(ctx, argv[i], bindings->dom_manager_);
        if (newNode) {
            parent->insertBefore(newNode, node);
        }
    }

    // Remove this node
    parent->removeChild(node);

    return JS_UNDEFINED;
}

static JSValue elem_prepend(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings || !bindings->dom_manager_) return JS_UNDEFINED;

    auto firstChild = node->getFirstChild();

    // Insert each argument at the beginning
    for (int i = 0; i < argc; ++i) {
        auto newNode = convertToNode(ctx, argv[i], bindings->dom_manager_);
        if (newNode) {
            node->insertBefore(newNode, firstChild);
        }
    }

    return JS_UNDEFINED;
}

static JSValue elem_append(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings || !bindings->dom_manager_) return JS_UNDEFINED;

    // Append each argument to the end
    for (int i = 0; i < argc; ++i) {
        auto newNode = convertToNode(ctx, argv[i], bindings->dom_manager_);
        if (newNode) {
            node->appendChild(newNode);
        }
    }

    return JS_UNDEFINED;
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

// ============================================================
// tabIndex property
// ============================================================

static JSValue elem_getTabIndex(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewInt32(ctx, -1);

    std::string tabindex = node->getAttribute("tabindex");
    if (tabindex.empty()) {
        // Default tabIndex is -1 for non-focusable elements, 0 for focusable elements
        // For simplicity, return -1 if not explicitly set
        return JS_NewInt32(ctx, -1);
    }

    try {
        int value = std::stoi(tabindex);
        return JS_NewInt32(ctx, value);
    } catch (...) {
        return JS_NewInt32(ctx, -1);
    }
}

static JSValue elem_setTabIndex(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    int32_t value;
    if (JS_ToInt32(ctx, &value, argv[0]) < 0) {
        return JS_UNDEFINED;
    }

    node->setAttribute("tabindex", std::to_string(value));
    return JS_UNDEFINED;
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

// Select element - selectedIndex
static JSValue select_getSelectedIndex(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewInt32(ctx, -1);

    // Find selected option index
    int index = 0;
    auto children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        auto& child = children[i];
        if (child->getTagName() == "option") {
            if (child->hasAttribute("selected")) {
                return JS_NewInt32(ctx, static_cast<int32_t>(index));
            }
            index++;
        }
    }

    // Return 0 if no option selected (first option is default)
    return JS_NewInt32(ctx, index > 0 ? 0 : -1);
}

static JSValue select_setSelectedIndex(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    int32_t new_index = 0;
    JS_ToInt32(ctx, &new_index, argv[0]);

    // Clear all selected attributes and set new one
    int index = 0;
    auto children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        auto& child = children[i];
        if (child->getTagName() == "option") {
            if (index == new_index) {
                child->setAttribute("selected", "");
            } else {
                child->removeAttribute("selected");
            }
            index++;
        }
    }

    return JS_UNDEFINED;
}

// Select element - options (returns array of option elements)
static JSValue select_getOptions(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewArray(ctx);

    auto* bindings = getBindingsFromCtx(ctx);
    if (!bindings) return JS_NewArray(ctx);

    JSValue arr = JS_NewArray(ctx);
    uint32_t arr_index = 0;

    auto children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        auto& child = children[i];
        if (child->getTagName() == "option") {
            JSValue opt = bindings->createJSElement(ctx, child);
            JS_SetPropertyUint32(ctx, arr, arr_index++, opt);
        }
    }

    return arr;
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

static JSValue elem_setScrollTop(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    double value;
    if (JS_ToFloat64(ctx, &value, argv[0]) != 0) return JS_UNDEFINED;

    node->setScrollTop(static_cast<float>(value));
    return JS_UNDEFINED;
}

static JSValue elem_setScrollLeft(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    double value;
    if (JS_ToFloat64(ctx, &value, argv[0]) != 0) return JS_UNDEFINED;

    node->setScrollLeft(static_cast<float>(value));
    return JS_UNDEFINED;
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
    DEFINE_GETTER(ctx, elem, "firstElementChild", elem_getFirstElementChild);
    DEFINE_GETTER(ctx, elem, "lastElementChild", elem_getLastElementChild);
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

    // Modern DOM manipulation methods
    JS_SetPropertyStr(ctx, elem, "before",
        JS_NewCFunction(ctx, elem_before, "before", 0));
    JS_SetPropertyStr(ctx, elem, "after",
        JS_NewCFunction(ctx, elem_after, "after", 0));
    JS_SetPropertyStr(ctx, elem, "replaceWith",
        JS_NewCFunction(ctx, elem_replaceWith, "replaceWith", 0));
    JS_SetPropertyStr(ctx, elem, "prepend",
        JS_NewCFunction(ctx, elem_prepend, "prepend", 0));
    JS_SetPropertyStr(ctx, elem, "append",
        JS_NewCFunction(ctx, elem_append, "append", 0));

    // hidden property
    DEFINE_GETTER_SETTER(ctx, elem, "hidden", elem_getHidden, elem_setHidden);

    // dataset proxy
    JS_SetPropertyStr(ctx, elem, "dataset", createDatasetProxy(ctx, elem, node));

    // Scroll/geometry properties (with setters for scrollTop/scrollLeft)
    DEFINE_GETTER_SETTER(ctx, elem, "scrollTop", elem_getScrollTop, elem_setScrollTop);
    DEFINE_GETTER_SETTER(ctx, elem, "scrollLeft", elem_getScrollLeft, elem_setScrollLeft);
    DEFINE_GETTER(ctx, elem, "scrollWidth", elem_getScrollWidth);
    DEFINE_GETTER(ctx, elem, "scrollHeight", elem_getScrollHeight);
    DEFINE_GETTER(ctx, elem, "clientWidth", elem_getClientWidth);
    DEFINE_GETTER(ctx, elem, "clientHeight", elem_getClientHeight);
    DEFINE_GETTER(ctx, elem, "offsetWidth", elem_getOffsetWidth);
    DEFINE_GETTER(ctx, elem, "offsetHeight", elem_getOffsetHeight);
    DEFINE_GETTER(ctx, elem, "offsetTop", elem_getOffsetTop);
    DEFINE_GETTER(ctx, elem, "offsetLeft", elem_getOffsetLeft);

    // tabIndex property
    DEFINE_GETTER_SETTER(ctx, elem, "tabIndex", elem_getTabIndex, elem_setTabIndex);
}

// ============================================================
// HTMLAnchorElement properties
// ============================================================

static JSValue anchor_getHref(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    std::string href = node->getAttribute("href");
    return JS_NewString(ctx, href.c_str());
}

static JSValue anchor_setHref(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    const char* str = JS_ToCString(ctx, argv[0]);
    if (str) {
        node->setAttribute("href", str);
        JS_FreeCString(ctx, str);
    }
    return JS_UNDEFINED;
}

static JSValue anchor_getTarget(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    std::string target = node->getAttribute("target");
    return JS_NewString(ctx, target.c_str());
}

static JSValue anchor_setTarget(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    const char* str = JS_ToCString(ctx, argv[0]);
    if (str) {
        node->setAttribute("target", str);
        JS_FreeCString(ctx, str);
    }
    return JS_UNDEFINED;
}

// ============================================================
// HTMLImageElement properties
// ============================================================

static JSValue img_getSrc(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    std::string src = node->getAttribute("src");
    return JS_NewString(ctx, src.c_str());
}

static JSValue img_setSrc(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;

    const char* str = JS_ToCString(ctx, argv[0]);
    if (!str) return JS_UNDEFINED;

    std::string src(str);
    JS_FreeCString(ctx, str);

    // Set the src attribute
    node->setAttribute("src", src);

    // Dispatch load/error event
    // In real browsers, these events are async, but for simplicity we dispatch synchronously
    // TODO: Make async using setTimeout or actual image loading callback
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
            // Determine event type
            // For now: empty src = error, non-empty = load
            // TODO: Check actual image file existence/loading status
            const char* event_type = src.empty() ? "error" : "load";
            bindings->dispatchSimpleEvent(nid, event_type);
        }
    }

    return JS_UNDEFINED;
}

static JSValue img_getAlt(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    std::string alt = node->getAttribute("alt");
    return JS_NewString(ctx, alt.c_str());
}

static JSValue img_setAlt(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    const char* str = JS_ToCString(ctx, argv[0]);
    if (str) {
        node->setAttribute("alt", str);
        JS_FreeCString(ctx, str);
    }
    return JS_UNDEFINED;
}

static JSValue img_getNaturalWidth(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    // TODO: Return actual loaded image width
    // For now, return 0 (image dimensions not yet tracked)
    return JS_NewInt32(ctx, 0);
}

static JSValue img_getNaturalHeight(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    // TODO: Return actual loaded image height
    // For now, return 0 (image dimensions not yet tracked)
    return JS_NewInt32(ctx, 0);
}

static JSValue img_getComplete(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc; (void)argv;
    // TODO: Track image load status
    // For now, always return true (images are loaded synchronously)
    return JS_NewBool(ctx, true);
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
        // Select element bindings
        DEFINE_GETTER_SETTER(ctx, elem, "value", input_getValue, input_setValue);
        DEFINE_GETTER_SETTER(ctx, elem, "disabled", input_getDisabled, input_setDisabled);
        DEFINE_GETTER_SETTER(ctx, elem, "name", input_getName, input_setName);
        DEFINE_GETTER_SETTER(ctx, elem, "selectedIndex", select_getSelectedIndex, select_setSelectedIndex);
        DEFINE_GETTER(ctx, elem, "options", select_getOptions);
    } else if (tag == "a") {
        // Anchor element bindings
        DEFINE_GETTER_SETTER(ctx, elem, "href", anchor_getHref, anchor_setHref);
        DEFINE_GETTER_SETTER(ctx, elem, "target", anchor_getTarget, anchor_setTarget);
    } else if (tag == "img") {
        // Image element bindings
        DEFINE_GETTER_SETTER(ctx, elem, "src", img_getSrc, img_setSrc);
        DEFINE_GETTER_SETTER(ctx, elem, "alt", img_getAlt, img_setAlt);
        DEFINE_GETTER(ctx, elem, "naturalWidth", img_getNaturalWidth);
        DEFINE_GETTER(ctx, elem, "naturalHeight", img_getNaturalHeight);
        DEFINE_GETTER(ctx, elem, "complete", img_getComplete);
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
