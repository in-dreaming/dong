#include "js_bindings.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>

extern "C" {
#include "quickjs.h"
}

namespace dong::script {

// Helper: Get JSBindings pointer from global context
static JSBindings* g_bindings = nullptr;

// ============================================================
// Console API Implementation
// ============================================================

static JSValue console_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    for (int i = 0; i < argc; i++) {
        if (i > 0) std::cout << " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            std::cout << str;
            JS_FreeCString(ctx, str);
        } else {
            std::cout << "[object]";
        }
    }
    std::cout << std::endl;
    return JS_UNDEFINED;
}

static JSValue console_warn(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::cout << "[WARN] ";
    for (int i = 0; i < argc; i++) {
        if (i > 0) std::cout << " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            std::cout << str;
            JS_FreeCString(ctx, str);
        }
    }
    std::cout << std::endl;
    return JS_UNDEFINED;
}

static JSValue console_error(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    std::cerr << "[ERROR] ";
    for (int i = 0; i < argc; i++) {
        if (i > 0) std::cerr << " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            std::cerr << str;
            JS_FreeCString(ctx, str);
        }
    }
    std::cerr << std::endl;
    return JS_UNDEFINED;
}

// ============================================================
// Document API Implementation
// ============================================================

static JSValue doc_getElementById(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (!g_bindings || argc < 1) return JS_NULL;
    
    const char* id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_NULL;
    
    auto dom_mgr = g_bindings->dom_manager_;
    if (!dom_mgr) {
        JS_FreeCString(ctx, id);
        return JS_NULL;
    }
    
    auto node = dom_mgr->getElementById(id);
    JS_FreeCString(ctx, id);
    
    if (!node) return JS_NULL;
    return g_bindings->createJSElement(ctx, node);
}

static JSValue doc_getElementsByTagName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (!g_bindings || argc < 1) return JS_NewArray(ctx);
    
    const char* tag = JS_ToCString(ctx, argv[0]);
    if (!tag) return JS_NewArray(ctx);
    
    auto dom_mgr = g_bindings->dom_manager_;
    JSValue arr = JS_NewArray(ctx);
    
    if (dom_mgr) {
        auto nodes = dom_mgr->getElementsByTagName(tag);
        for (size_t i = 0; i < nodes.size(); ++i) {
            JSValue elem = g_bindings->createJSElement(ctx, nodes[i]);
            JS_SetPropertyUint32(ctx, arr, i, elem);
        }
    }
    
    JS_FreeCString(ctx, tag);
    return arr;
}

static JSValue doc_getElementsByClassName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (!g_bindings || argc < 1) return JS_NewArray(ctx);
    
    const char* cls = JS_ToCString(ctx, argv[0]);
    if (!cls) return JS_NewArray(ctx);
    
    auto dom_mgr = g_bindings->dom_manager_;
    JSValue arr = JS_NewArray(ctx);
    
    if (dom_mgr) {
        auto nodes = dom_mgr->getElementsByClassName(cls);
        for (size_t i = 0; i < nodes.size(); ++i) {
            JSValue elem = g_bindings->createJSElement(ctx, nodes[i]);
            JS_SetPropertyUint32(ctx, arr, i, elem);
        }
    }
    
    JS_FreeCString(ctx, cls);
    return arr;
}

static JSValue doc_querySelector(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (!g_bindings || argc < 1) return JS_NULL;
    
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NULL;
    
    // For now, simple implementation - could use style engine for selector parsing
    auto dom_mgr = g_bindings->dom_manager_;
    JSValue result = JS_NULL;
    
    if (dom_mgr) {
        // Try parsing as ID selector
        if (selector[0] == '#') {
            auto node = dom_mgr->getElementById(selector + 1);
            if (node) result = g_bindings->createJSElement(ctx, node);
        }
        // Try parsing as tag selector
        else if (selector[0] != '.' && selector[0] != '[') {
            auto nodes = dom_mgr->getElementsByTagName(selector);
            if (!nodes.empty()) {
                result = g_bindings->createJSElement(ctx, nodes[0]);
            }
        }
        // Try parsing as class selector
        else if (selector[0] == '.') {
            auto nodes = dom_mgr->getElementsByClassName(selector + 1);
            if (!nodes.empty()) {
                result = g_bindings->createJSElement(ctx, nodes[0]);
            }
        }
    }
    
    JS_FreeCString(ctx, selector);
    return result;
}

static JSValue doc_querySelectorAll(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (!g_bindings || argc < 1) return JS_NewArray(ctx);
    
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NewArray(ctx);
    
    auto dom_mgr = g_bindings->dom_manager_;
    JSValue arr = JS_NewArray(ctx);
    
    if (dom_mgr) {
        std::vector<dom::DOMNodePtr> nodes;
        
        // Parse selector
        if (selector[0] == '#') {
            auto node = dom_mgr->getElementById(selector + 1);
            if (node) nodes.push_back(node);
        }
        else if (selector[0] != '.' && selector[0] != '[') {
            nodes = dom_mgr->getElementsByTagName(selector);
        }
        else if (selector[0] == '.') {
            nodes = dom_mgr->getElementsByClassName(selector + 1);
        }
        
        for (size_t i = 0; i < nodes.size(); ++i) {
            JSValue elem = g_bindings->createJSElement(ctx, nodes[i]);
            JS_SetPropertyUint32(ctx, arr, i, elem);
        }
    }
    
    JS_FreeCString(ctx, selector);
    return arr;
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
    
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (node && attr_name && attr_value) {
        node->setAttribute(attr_name, attr_value);
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

static JSValue elem_getTextContent(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    
    std::string text = node->getTextContent();
    return JS_NewString(ctx, text.c_str());
}

static JSValue elem_setTextContent(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 1) return JS_UNDEFINED;
    
    const char* text = JS_ToCString(ctx, argv[0]);
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    
    if (node && text) {
        node->setTextContent(text);
    }
    
    if (text) JS_FreeCString(ctx, text);
    return JS_UNDEFINED;
}

static JSValue elem_addEventListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_UNDEFINED;
    
    const char* event_type = JS_ToCString(ctx, argv[0]);
    if (!event_type) return JS_UNDEFINED;
    
    // TODO: Store event listener callback and attach to event system
    // For now, just consume the arguments
    
    JS_FreeCString(ctx, event_type);
    return JS_UNDEFINED;
}

static JSValue elem_removeEventListener(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    if (argc < 2) return JS_UNDEFINED;
    
    const char* event_type = JS_ToCString(ctx, argv[0]);
    if (!event_type) return JS_UNDEFINED;
    
    JS_FreeCString(ctx, event_type);
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
    JS_SetPropertyStr(ctx, style_obj, "fontWeight", JS_NewString(ctx, style.font_weight.c_str()));
    JS_SetPropertyStr(ctx, style_obj, "textAlign", JS_NewString(ctx, style.text_align.c_str()));
    JS_SetPropertyStr(ctx, style_obj, "display", JS_NewString(ctx, style.display.c_str()));
    JS_SetPropertyStr(ctx, style_obj, "position", JS_NewString(ctx, style.position.c_str()));
    
    return style_obj;
}

static JSValue elem_getTagName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_UNDEFINED;
    
    std::string tag = node->getTagName();
    return JS_NewString(ctx, tag.c_str());
}

static JSValue elem_getChildren(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewArray(ctx);
    
    JSValue arr = JS_NewArray(ctx);
    const auto& children = node->getChildren();
    
    for (size_t i = 0; i < children.size(); ++i) {
        JSValue child = g_bindings->createJSElement(ctx, children[i]);
        JS_SetPropertyUint32(ctx, arr, i, child);
    }
    
    return arr;
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

static JSValue event_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue event = JS_NewObject(ctx);
    
    if (argc > 0) {
        const char* type = JS_ToCString(ctx, argv[0]);
        if (type) {
            JS_SetPropertyStr(ctx, event, "type", JS_NewString(ctx, type));
            JS_FreeCString(ctx, type);
        }
    }
    
    // Event properties
    JS_SetPropertyStr(ctx, event, "bubbles", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "cancelable", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "defaultPrevented", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "cancelBubble", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "timeStamp", JS_NewInt32(ctx, 0));
    
    // Event methods
    JS_SetPropertyStr(ctx, event, "preventDefault",
        JS_NewCFunction(ctx, event_preventDefault, "preventDefault", 0));
    JS_SetPropertyStr(ctx, event, "stopPropagation",
        JS_NewCFunction(ctx, event_stopPropagation, "stopPropagation", 0));
    
    return event;
}

static JSValue mouse_event_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue event = event_constructor(ctx, this_val, argc, argv);
    
    // MouseEvent-specific properties
    JS_SetPropertyStr(ctx, event, "clientX", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "clientY", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "screenX", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "screenY", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "button", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "buttons", JS_NewInt32(ctx, 0));
    
    return event;
}

static JSValue keyboard_event_constructor(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    JSValue event = event_constructor(ctx, this_val, argc, argv);
    
    // KeyboardEvent-specific properties
    JS_SetPropertyStr(ctx, event, "key", JS_NewString(ctx, ""));
    JS_SetPropertyStr(ctx, event, "code", JS_NewString(ctx, ""));
    JS_SetPropertyStr(ctx, event, "keyCode", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "charCode", JS_NewInt32(ctx, 0));
    JS_SetPropertyStr(ctx, event, "ctrlKey", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "shiftKey", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "altKey", JS_FALSE);
    JS_SetPropertyStr(ctx, event, "metaKey", JS_FALSE);
    
    return event;
}

// ============================================================
// JSBindings Implementation
// ============================================================

JSBindings::JSBindings(ScriptEngine* engine, dom::Manager* dom_manager, 
                       dom::EventDispatcher* event_dispatcher)
    : engine_(engine), dom_manager_(dom_manager), event_dispatcher_(event_dispatcher) {
    g_bindings = this;
}

JSBindings::~JSBindings() {
    g_bindings = nullptr;
}

void JSBindings::initialize() {
    if (!engine_) return;

    initializeConsoleAPI();
    initializeDocumentAPI();
    initializeElementAPI();
    initializeEventAPI();
    
    std::cout << "✓ JavaScript API bindings initialized" << std::endl;
}

void JSBindings::initializeConsoleAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue console = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, console, "log",
        JS_NewCFunction(ctx, console_log, "log", -1));
    JS_SetPropertyStr(ctx, console, "warn",
        JS_NewCFunction(ctx, console_warn, "warn", -1));
    JS_SetPropertyStr(ctx, console, "error",
        JS_NewCFunction(ctx, console_error, "error", -1));

    JS_SetPropertyStr(ctx, global, "console", console);
    JS_FreeValue(ctx, console);
    JS_FreeValue(ctx, global);
}

void JSBindings::initializeDocumentAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue document = JS_NewObject(ctx);

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

    JS_SetPropertyStr(ctx, global, "document", document);
    JS_FreeValue(ctx, document);
    JS_FreeValue(ctx, global);
}

void JSBindings::initializeElementAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue element_proto = JS_NewObject(ctx);

    // Element methods
    JS_SetPropertyStr(ctx, element_proto, "getAttribute",
        JS_NewCFunction(ctx, elem_getAttribute, "getAttribute", 1));
    JS_SetPropertyStr(ctx, element_proto, "setAttribute",
        JS_NewCFunction(ctx, elem_setAttribute, "setAttribute", 2));
    JS_SetPropertyStr(ctx, element_proto, "appendChild",
        JS_NewCFunction(ctx, elem_appendChild, "appendChild", 1));
    JS_SetPropertyStr(ctx, element_proto, "removeChild",
        JS_NewCFunction(ctx, elem_removeChild, "removeChild", 1));
    JS_SetPropertyStr(ctx, element_proto, "addEventListener",
        JS_NewCFunction(ctx, elem_addEventListener, "addEventListener", 2));
    JS_SetPropertyStr(ctx, element_proto, "removeEventListener",
        JS_NewCFunction(ctx, elem_removeEventListener, "removeEventListener", 2));
    JS_SetPropertyStr(ctx, element_proto, "getComputedStyle",
        JS_NewCFunction(ctx, elem_getComputedStyle, "getComputedStyle", 0));

    JS_SetPropertyStr(ctx, global, "Element", element_proto);
    JS_FreeValue(ctx, element_proto);
    JS_FreeValue(ctx, global);
}

void JSBindings::initializeEventAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    
    JSValue global = JS_GetGlobalObject(ctx);

    // Event constructors
    JSValue event_ctor = JS_NewCFunction(ctx, event_constructor, "Event", 1);
    JS_SetPropertyStr(ctx, global, "Event", event_ctor);
    JS_FreeValue(ctx, event_ctor);

    JSValue mouse_event_ctor = JS_NewCFunction(ctx, mouse_event_constructor, "MouseEvent", 1);
    JS_SetPropertyStr(ctx, global, "MouseEvent", mouse_event_ctor);
    JS_FreeValue(ctx, mouse_event_ctor);

    JSValue keyboard_event_ctor = JS_NewCFunction(ctx, keyboard_event_constructor, "KeyboardEvent", 1);
    JS_SetPropertyStr(ctx, global, "KeyboardEvent", keyboard_event_ctor);
    JS_FreeValue(ctx, keyboard_event_ctor);

    JS_FreeValue(ctx, global);
}

JSValue JSBindings::createJSElement(JSContext* ctx, const dom::DOMNodePtr& node) {
    if (!node) return JS_NULL;

    JSValue elem = JS_NewObject(ctx);
    
    // Store DOM node pointer
    setNodeOpaque(ctx, elem, node);
    
    // Expose properties and methods
    // Properties
    JS_SetPropertyStr(ctx, elem, "tagName", JS_NewString(ctx, node->getTagName().c_str()));
    JS_SetPropertyStr(ctx, elem, "id", 
        JS_NewString(ctx, node->hasAttribute("id") ? node->getAttribute("id").c_str() : ""));
    JS_SetPropertyStr(ctx, elem, "className",
        JS_NewString(ctx, node->hasAttribute("class") ? node->getAttribute("class").c_str() : ""));
    JS_SetPropertyStr(ctx, elem, "textContent", JS_NewString(ctx, node->getTextContent().c_str()));
    
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

    return elem;
}

// Static helper methods
dom::DOMNodePtr JSBindings::getNodeOpaque(JSContext* ctx, JSValue val) {
    if (!g_bindings) return nullptr;

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

    auto it = g_bindings->id_to_node_.find(static_cast<uint64_t>(id));
    if (it != g_bindings->id_to_node_.end()) {
        return it->second;
    }
    return nullptr;
}

void JSBindings::setNodeOpaque(JSContext* ctx, JSValue val, dom::DOMNodePtr node) {
    if (!g_bindings || !node) return;

    uint64_t id = g_bindings->next_js_id_++;
    g_bindings->id_to_node_[id] = node;

    JS_SetPropertyStr(ctx, val, "__node_id__", JS_NewInt64(ctx, static_cast<int64_t>(id)));
}

} // namespace dong::script
