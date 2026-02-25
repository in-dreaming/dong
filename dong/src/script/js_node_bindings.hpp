#pragma once

#include "../dom/dom/dom_node.hpp"

#include "quickjs_compat.h"


namespace dong::script {

class JSBindings;

// Bind Node interface properties (parentNode, childNodes, firstChild, etc.)
void bindNodeProperties(JSContext* ctx, JSValue elem, const dom::DOMNodePtr& node, JSBindings* bindings);

// Bind Element interface properties (querySelector, matches, closest, etc.)
void bindElementProperties(JSContext* ctx, JSValue elem, const dom::DOMNodePtr& node, JSBindings* bindings);

// Bind HTMLElement-specific properties (input.value, input.checked, etc.)
void bindHTMLElementProperties(JSContext* ctx, JSValue elem, const dom::DOMNodePtr& node, JSBindings* bindings);

// Initialize Node global constants (Node.ELEMENT_NODE, etc.)
void initializeNodeConstants(JSContext* ctx);

} // namespace dong::script
