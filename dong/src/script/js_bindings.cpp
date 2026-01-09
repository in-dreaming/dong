#include "js_bindings.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cctype>
#include <algorithm>
#include "../core/log.h"
extern "C" {
#include "quickjs.h"
}

// Helper to get JSBindings from context opaque data
static dong::script::JSBindings* getBindingsFromContext(JSContext* ctx) {
    if (!ctx) return nullptr;
    return static_cast<dong::script::JSBindings*>(JS_GetContextOpaque(ctx));
}

// ============================================================
// Console API Implementation - extern "C" for QuickJS callbacks
// ============================================================

extern "C" {

static JSValue console_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    std::string output;
    for (int i = 0; i < argc; i++) {
        if (i > 0) output += " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            output += str;
            JS_FreeCString(ctx, str);
        } else {
            output += "[object]";
        }
    }
    DONG_LOG_INFO("[JS] %s", output.c_str());
    return JS_UNDEFINED;
}

static JSValue console_warn(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    std::string output = "[WARN] ";
    for (int i = 0; i < argc; i++) {
        if (i > 0) output += " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            output += str;
            JS_FreeCString(ctx, str);
        }
    }
    DONG_LOG_WARN("[JS] %s", output.c_str());
    return JS_UNDEFINED;
}

static JSValue console_error(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    std::string output = "[ERROR] ";
    for (int i = 0; i < argc; i++) {
        if (i > 0) output += " ";
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            output += str;
            JS_FreeCString(ctx, str);
        }
    }
    DONG_LOG_ERROR("[JS] %s", output.c_str());
    return JS_UNDEFINED;
}

} // extern "C"

namespace dong::script {

namespace {

// Helper to update layout_mode when display changes
dom::LayoutMode deriveLayoutModeFromDisplay(const std::string& display) {
    if (display == "none") {
        return dom::LayoutMode::None;
    }
    if (display == "flex") {
        return dom::LayoutMode::Flex;
    }
    if (display == "inline") {
        return dom::LayoutMode::Inline;
    }
    // block, inline-block, and others are treated as Block
    return dom::LayoutMode::Block;
}

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

std::string getComputedStyleValue(const dom::ComputedStyle& style, const std::string& css_prop) {
    if (css_prop == "display") return style.display;
    if (css_prop == "color") return style.color;
    if (css_prop == "background-color") return style.background_color;
    if (css_prop == "font-size") return std::to_string(style.font_size);
    if (css_prop == "font-weight") return style.font_weight;
    if (css_prop == "text-align") return style.text_align;
    if (css_prop == "position") return style.position;
    if (css_prop == "opacity") return std::to_string(style.opacity);
    if (css_prop == "border-radius") return std::to_string(style.border_radius);
    if (css_prop == "border-width") return std::to_string(style.border_width);
    if (css_prop == "border-color") return style.border_color;
    return "";
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

    // Seed common properties for easier inspection
    if (node) {
        const auto& style = node->getComputedStyle();
        JS_SetPropertyStr(ctx, target, "display", JS_NewString(ctx, style.display.c_str()));
        JS_SetPropertyStr(ctx, target, "color", JS_NewString(ctx, style.color.c_str()));
        JS_SetPropertyStr(ctx, target, "backgroundColor", JS_NewString(ctx, style.background_color.c_str()));
    }

    JSValue handler = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, handler, "get", JS_NewCFunction(ctx, style_proxy_get, "style_get", 3));
    JS_SetPropertyStr(ctx, handler, "set", JS_NewCFunction(ctx, style_proxy_set, "style_set", 4));

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue proxy_ctor = JS_GetPropertyStr(ctx, global, "Proxy");
    JS_FreeValue(ctx, global);

    JSValue argv_local[2] = { target, handler };
    JSValue proxy = JS_CallConstructor(ctx, proxy_ctor, 2, argv_local);
    JS_FreeValue(ctx, proxy_ctor);
    JS_FreeValue(ctx, target);
    JS_FreeValue(ctx, handler);
    return proxy;
}

} // namespace

// ============================================================
// Document API Implementation
// ============================================================

static JSValue doc_getElementById(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_NULL;
    
    const char* id = JS_ToCString(ctx, argv[0]);
    if (!id) return JS_NULL;
    
    auto dom_mgr = bindings->dom_manager_;
    if (!dom_mgr) {
        JS_FreeCString(ctx, id);
        return JS_NULL;
    }
    
    auto node = dom_mgr->getElementById(id);
    JS_FreeCString(ctx, id);
    
    if (!node) return JS_NULL;
    return bindings->createJSElement(ctx, node);
}

static JSValue doc_getElementsByTagName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsFromContext(ctx);
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
    auto bindings = getBindingsFromContext(ctx);
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
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_NULL;
    
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NULL;
    
    // For now, simple implementation - could use style engine for selector parsing
    auto dom_mgr = bindings->dom_manager_;
    JSValue result = JS_NULL;
    
    if (dom_mgr) {
        // Try parsing as ID selector
        if (selector[0] == '#') {
            auto node = dom_mgr->getElementById(selector + 1);
            if (node) result = bindings->createJSElement(ctx, node);
        }
        // Try parsing as tag selector
        else if (selector[0] != '.' && selector[0] != '[') {
            auto nodes = dom_mgr->getElementsByTagName(selector);
            if (!nodes.empty()) {
                result = bindings->createJSElement(ctx, nodes[0]);
            }
        }
        // Try parsing as class selector
        else if (selector[0] == '.') {
            auto nodes = dom_mgr->getElementsByClassName(selector + 1);
            if (!nodes.empty()) {
                result = bindings->createJSElement(ctx, nodes[0]);
            }
        }
    }
    
    JS_FreeCString(ctx, selector);
    return result;
}

static JSValue doc_querySelectorAll(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_NewArray(ctx);
    
    const char* selector = JS_ToCString(ctx, argv[0]);
    if (!selector) return JS_NewArray(ctx);
    
    auto dom_mgr = bindings->dom_manager_;
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
            JSValue elem = bindings->createJSElement(ctx, nodes[i]);
            JS_SetPropertyUint32(ctx, arr, i, elem);
        }
    }
    
    JS_FreeCString(ctx, selector);
    return arr;
}

// ============================================================
// 銆愮己鍙?銆慏OM 鍒涘缓 API
// ============================================================

static JSValue doc_createElement(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsFromContext(ctx);
    if (!bindings || argc < 1) return JS_NULL;
    
    const char* tag = JS_ToCString(ctx, argv[0]);
    if (!tag) return JS_NULL;
    
    // 鍒涘缓鏂?DOM 鑺傜偣
    auto node = std::make_shared<dong::dom::DOMNode>(dong::dom::DOMNode::NodeType::ELEMENT, tag);
    
    JS_FreeCString(ctx, tag);
    
    if (!node) return JS_NULL;
    
    // 杩斿洖 JS 鍏冪礌瀵硅薄
    return bindings->createJSElement(ctx, node);
}

static JSValue doc_createTextNode(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    auto bindings = getBindingsFromContext(ctx);
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
    else if (strcmp(prop, "fontWeight") == 0) style.font_weight = str_val;
    else if (strcmp(prop, "textAlign") == 0) style.text_align = str_val;
    else if (strcmp(prop, "display") == 0) {
        style.display = str_val;
        style.layout_mode = deriveLayoutModeFromDisplay(str_val);
    }
    else if (strcmp(prop, "position") == 0) style.position = str_val;
    else if (strcmp(prop, "opacity") == 0) style.opacity = (float)num_val;
    else if (strcmp(prop, "borderRadius") == 0) style.border_radius = (float)num_val;
    else if (strcmp(prop, "borderWidth") == 0) style.border_width = (float)num_val;
    else if (strcmp(prop, "borderColor") == 0) style.border_color = str_val;
    
    node->markLayoutDirty();
    
    return JS_UNDEFINED;
}

// className getter/setter
static JSValue elem_getClassName(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)argc;
    (void)argv;
    auto node = JSBindings::getNodeOpaque(ctx, this_val);
    if (!node) return JS_NewString(ctx, "");
    std::string cls = node->getAttribute("class");
    return JS_NewString(ctx, cls.c_str());
}

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
    // Set context opaque to this JSBindings instance for per-context lookup
    if (engine_ && engine_->getContext()) {
        JS_SetContextOpaque(engine_->getContext(), this);
    }
}

JSBindings::~JSBindings() {
    resetForNewDOM();
    // Clear context opaque
    if (engine_ && engine_->getContext()) {
        JS_SetContextOpaque(engine_->getContext(), nullptr);
    }
}

void JSBindings::initialize() {
    if (!engine_) return;

    initializeConsoleAPI();
    initializeDocumentAPI();
    initializeElementAPI();
    initializeEventAPI();
}

void JSBindings::initializeConsoleAPI() {
    if (!engine_) return;

    JSContext* ctx = engine_->getContext();
    if (!ctx) return;
    
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue console = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, console, "log",
        JS_NewCFunction(ctx, console_log, "log", 1));
    JS_SetPropertyStr(ctx, console, "warn",
        JS_NewCFunction(ctx, console_warn, "warn", 1));
    JS_SetPropertyStr(ctx, console, "error",
        JS_NewCFunction(ctx, console_error, "error", 1));

    // JS_SetPropertyStr takes ownership of 'console', so we must NOT free it afterwards.
    JS_SetPropertyStr(ctx, global, "console", console);
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
    
    // DOM 鍒涘缓 API
    JS_SetPropertyStr(ctx, document, "createElement",
        JS_NewCFunction(ctx, doc_createElement, "createElement", 1));
    JS_SetPropertyStr(ctx, document, "createTextNode",
        JS_NewCFunction(ctx, doc_createTextNode, "createTextNode", 1));

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

    // JS_SetPropertyStr takes ownership of 'document'
    JS_SetPropertyStr(ctx, global, "document", document);
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
    JSValue event_ctor = JS_NewCFunction(ctx, event_constructor, "Event", 1);
    JS_SetPropertyStr(ctx, global, "Event", event_ctor);

    JSValue mouse_event_ctor = JS_NewCFunction(ctx, mouse_event_constructor, "MouseEvent", 1);
    JS_SetPropertyStr(ctx, global, "MouseEvent", mouse_event_ctor);

    JSValue keyboard_event_ctor = JS_NewCFunction(ctx, keyboard_event_constructor, "KeyboardEvent", 1);
    JS_SetPropertyStr(ctx, global, "KeyboardEvent", keyboard_event_ctor);

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
    
    // className - use getter/setter for dynamic access
    JSAtom className_atom = JS_NewAtom(ctx, "className");
    JSValue getter = JS_NewCFunction(ctx, elem_getClassName, "get className", 0);
    JSValue setter = JS_NewCFunction(ctx, elem_setClassName, "set className", 1);
    JS_DefinePropertyGetSet(ctx, elem, className_atom, getter, setter,
        JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
    JS_FreeAtom(ctx, className_atom);
    
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
    
    // style and classList
    JS_SetPropertyStr(ctx, elem, "style", elem_getStyle(ctx, elem, 0, nullptr));
    JS_SetPropertyStr(ctx, elem, "classList", elem_getClassList(ctx, elem, 0, nullptr));

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
        // Bridge listener already registered for this (node, type)
        return;
    }

    dom::EventListener callback;

    if (type == "click" || type == "mousedown" || type == "mouseup" || type == "mousemove") {
        callback = [this, node_id, type](const dom::Event& ev) {
            this->dispatchMouseEvent(node_id, type.c_str(), ev.mouse_x, ev.mouse_y, ev.mouse_button);
        };
    } else if (type == "keydown" || type == "keyup" || type == "keypress") {
        callback = [this, node_id, type](const dom::Event& ev) {
            this->dispatchKeyEvent(node_id, type.c_str(), ev.key_code);
        };
    } else {
        // Unsupported event type for bridging; ignore.
        return;
    }

    uint64_t listener_id = event_dispatcher_->addEventListener(node, type, callback);
    if (listener_id != 0) {
        per_node[type] = listener_id;
    }
}

void JSBindings::dispatchMouseEvent(uint64_t node_id, const char* type, int32_t x, int32_t y, int32_t button) {
    if (!engine_) return;
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
        JS_SetPropertyStr(ctx, ev, "clientX", JS_NewInt32(ctx, x));
        JS_SetPropertyStr(ctx, ev, "clientY", JS_NewInt32(ctx, y));
        JS_SetPropertyStr(ctx, ev, "button", JS_NewInt32(ctx, button));
        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);

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
                std::fprintf(stderr, "[JSBindings] mouse event error: %s\
", err);
                JS_FreeCString(ctx, err);
            }
            JS_FreeValue(ctx, exc);
        }
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, ev);
    }
}

void JSBindings::dispatchKeyEvent(uint64_t node_id, const char* type, uint32_t key_code) {
    if (!engine_) return;
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
        JS_SetPropertyStr(ctx, ev, "keyCode", JS_NewInt32(ctx, (int32_t)key_code));
        JS_SetPropertyStr(ctx, ev, "charCode", JS_NewInt32(ctx, (int32_t)key_code));
        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);

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
                std::fprintf(stderr, "[JSBindings] key event error: %s\\n", err);
                JS_FreeCString(ctx, err);
            }
            JS_FreeValue(ctx, exc);
        }
        JS_FreeValue(ctx, ret);
        JS_FreeValue(ctx, ev);
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
}

// Static helper methods
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

} // namespace dong::script
