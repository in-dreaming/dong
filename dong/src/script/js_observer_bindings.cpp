// js_observer_bindings.cpp
#include "js_observer_bindings.hpp"
#include "js_bindings.hpp"
#include "../dom/dom/observer.hpp"
#include "../core/log.h"
#include <memory>
#include <unordered_map>

#include "quickjs_compat.h"


namespace dong::script {

// Helper to dump JS errors to log
static void dumpJSError(JSContext* ctx) {
    JSValue exception = JS_GetException(ctx);
    const char* str = JS_ToCString(ctx, exception);
    if (str) {
        DONG_LOG_ERROR("[JS Error] %s", str);
        JS_FreeCString(ctx, str);
    }
    JS_FreeValue(ctx, exception);
}

// ============================================================================
// ResizeObserver Implementation
// ============================================================================

// Opaque data structure for ResizeObserver instances
struct ResizeObserverOpaque {
    std::unique_ptr<dom::ResizeObserver> observer;
    JSContext* ctx;
    JSValue callback;  // JS callback function reference
    JSValue jsObject;  // Reference to the JS object itself
    JSBindings* bindings;
};

// Store opaque pointer in a private property
static constexpr const char* PRIVATE_OPAQUE_KEY = "\xFF_opaque";

// Helper to convert DOMRectReadOnly to JS object
static JSValue createDOMRectReadOnly(JSContext* ctx, const dom::ResizeObserverEntry::DOMRectReadOnly& rect) {
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

// Helper to convert ResizeObserverSize to JS object
static JSValue createResizeObserverSize(JSContext* ctx, const dom::ResizeObserverEntry::ResizeObserverSize& size) {
    JSValue obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, obj, "inlineSize", JS_NewFloat64(ctx, size.inlineSize));
    JS_SetPropertyStr(ctx, obj, "blockSize", JS_NewFloat64(ctx, size.blockSize));
    return obj;
}

// Helper to get opaque data from JS object
static ResizeObserverOpaque* getResizeObserverOpaque(JSContext* ctx, JSValueConst obj) {
    JSValue opaqueVal = JS_GetPropertyStr(ctx, obj, PRIVATE_OPAQUE_KEY);
    if (JS_IsUndefined(opaqueVal)) {
        JS_FreeValue(ctx, opaqueVal);
        return nullptr;
    }

    int64_t ptr = 0;
    JS_ToInt64(ctx, &ptr, opaqueVal);
    JS_FreeValue(ctx, opaqueVal);

    return reinterpret_cast<ResizeObserverOpaque*>(static_cast<intptr_t>(ptr));
}

// ResizeObserver constructor: new ResizeObserver(callback)
static JSValue resizeObserverConstructor(JSContext* ctx, JSValueConst new_target,
                                         int argc, JSValueConst* argv) {
    (void)new_target;

    if (argc < 1 || !JS_IsFunction(ctx, argv[0])) {
        return JS_ThrowTypeError(ctx, "ResizeObserver constructor requires a callback function");
    }

    // Get JSBindings from context
    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (!bindings) {
        return JS_ThrowInternalError(ctx, "No JSBindings in context");
    }

    // Create opaque data
    auto* opaque = new ResizeObserverOpaque();
    opaque->ctx = ctx;
    opaque->callback = JS_DupValue(ctx, argv[0]);
    opaque->bindings = bindings;
    opaque->jsObject = JS_UNDEFINED;

    // Create C++ ResizeObserver with a lambda that calls the JS callback
    opaque->observer = std::make_unique<dom::ResizeObserver>(
        [opaque](const std::vector<dom::ResizeObserverEntry>& entries, dom::ResizeObserver* observer) {
            (void)observer;
            JSContext* ctx = opaque->ctx;

            // Convert entries to JS array
            JSValue entriesArray = JS_NewArray(ctx);
            for (size_t i = 0; i < entries.size(); i++) {
                const auto& entry = entries[i];

                // Create ResizeObserverEntry object
                JSValue entryObj = JS_NewObject(ctx);

                // target property
                if (entry.target) {
                    JSValue targetObj = opaque->bindings->createJSElement(ctx, entry.target);
                    JS_SetPropertyStr(ctx, entryObj, "target", targetObj);
                } else {
                    JS_SetPropertyStr(ctx, entryObj, "target", JS_NULL);
                }

                // contentRect property
                JSValue contentRect = createDOMRectReadOnly(ctx, entry.contentRect);
                JS_SetPropertyStr(ctx, entryObj, "contentRect", contentRect);

                // contentBoxSize array
                JSValue contentBoxSize = JS_NewArray(ctx);
                for (size_t j = 0; j < entry.contentBoxSize.size(); j++) {
                    JSValue sizeObj = createResizeObserverSize(ctx, entry.contentBoxSize[j]);
                    JS_SetPropertyUint32(ctx, contentBoxSize, static_cast<uint32_t>(j), sizeObj);
                }
                JS_SetPropertyStr(ctx, entryObj, "contentBoxSize", contentBoxSize);

                // borderBoxSize array
                JSValue borderBoxSize = JS_NewArray(ctx);
                for (size_t j = 0; j < entry.borderBoxSize.size(); j++) {
                    JSValue sizeObj = createResizeObserverSize(ctx, entry.borderBoxSize[j]);
                    JS_SetPropertyUint32(ctx, borderBoxSize, static_cast<uint32_t>(j), sizeObj);
                }
                JS_SetPropertyStr(ctx, entryObj, "borderBoxSize", borderBoxSize);

                // devicePixelContentBoxSize array
                JSValue devicePixelContentBoxSize = JS_NewArray(ctx);
                for (size_t j = 0; j < entry.devicePixelContentBoxSize.size(); j++) {
                    JSValue sizeObj = createResizeObserverSize(ctx, entry.devicePixelContentBoxSize[j]);
                    JS_SetPropertyUint32(ctx, devicePixelContentBoxSize, static_cast<uint32_t>(j), sizeObj);
                }
                JS_SetPropertyStr(ctx, entryObj, "devicePixelContentBoxSize", devicePixelContentBoxSize);

                JS_SetPropertyUint32(ctx, entriesArray, static_cast<uint32_t>(i), entryObj);
            }

            // Call the JS callback: callback(entries, this)
            JSValue args[2] = { entriesArray, opaque->jsObject };
            JSValue ret = JS_Call(ctx, opaque->callback, JS_UNDEFINED, 2, args);

            // Free the result and arguments
            if (JS_IsException(ret)) {
                dumpJSError(ctx);
            }
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, entriesArray);
        }
    );

    // Create JS object
    JSValue obj = JS_NewObject(ctx);

    // Store opaque pointer
    JSValue opaquePtr = JS_NewInt64(ctx, reinterpret_cast<intptr_t>(opaque));
    JS_SetPropertyStr(ctx, obj, PRIVATE_OPAQUE_KEY, opaquePtr);

    // Store reference to JS object in opaque
    opaque->jsObject = JS_DupValue(ctx, obj);

    // Add finalizer property that will clean up when GC collects the object
    JSValue finalizer = JS_NewCFunction(ctx, [](JSContext* ctx, JSValueConst this_val,
                                                 int argc, JSValueConst* argv) -> JSValue {
        (void)argc; (void)argv;
        auto* opaque = getResizeObserverOpaque(ctx, this_val);
        if (opaque) {
            if (!JS_IsUndefined(opaque->callback)) {
                JS_FreeValue(ctx, opaque->callback);
            }
            if (!JS_IsUndefined(opaque->jsObject)) {
                JS_FreeValue(ctx, opaque->jsObject);
            }
            delete opaque;
        }
        return JS_UNDEFINED;
    }, "__finalizer", 0);
    JS_SetPropertyStr(ctx, obj, "__finalizer", finalizer);

    return obj;
}

// ResizeObserver.prototype.observe(target)
static JSValue resizeObserverObserve(JSContext* ctx, JSValueConst this_val,
                                      int argc, JSValueConst* argv) {
    (void)argc;

    auto* opaque = getResizeObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid ResizeObserver");
    }

    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "observe() requires a target element");
    }

    auto target = JSBindings::getNodeOpaque(ctx, argv[0]);
    if (!target) {
        return JS_ThrowTypeError(ctx, "observe() target must be an Element");
    }

    opaque->observer->observe(target);
    return JS_UNDEFINED;
}

// ResizeObserver.prototype.unobserve(target)
static JSValue resizeObserverUnobserve(JSContext* ctx, JSValueConst this_val,
                                        int argc, JSValueConst* argv) {
    (void)argc;

    auto* opaque = getResizeObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid ResizeObserver");
    }

    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "unobserve() requires a target element");
    }

    auto target = JSBindings::getNodeOpaque(ctx, argv[0]);
    if (!target) {
        return JS_ThrowTypeError(ctx, "unobserve() target must be an Element");
    }

    opaque->observer->unobserve(target);
    return JS_UNDEFINED;
}

// ResizeObserver.prototype.disconnect()
static JSValue resizeObserverDisconnect(JSContext* ctx, JSValueConst this_val,
                                         int argc, JSValueConst* argv) {
    (void)argc; (void)argv;

    auto* opaque = getResizeObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid ResizeObserver");
    }

    opaque->observer->disconnect();
    return JS_UNDEFINED;
}

// ============================================================================
// MutationObserver Implementation
// ============================================================================

struct MutationObserverOpaque {
    std::unique_ptr<dom::MutationObserver> observer;
    JSContext* ctx;
    JSValue callback;
    JSValue jsObject;
    JSBindings* bindings;
};

static constexpr const char* MUTATION_OPAQUE_KEY = "\xFF_mutation_opaque";

// Helper to get MutationObserver opaque data
static MutationObserverOpaque* getMutationObserverOpaque(JSContext* ctx, JSValueConst obj) {
    JSValue opaqueVal = JS_GetPropertyStr(ctx, obj, MUTATION_OPAQUE_KEY);
    if (JS_IsUndefined(opaqueVal)) {
        JS_FreeValue(ctx, opaqueVal);
        return nullptr;
    }

    int64_t ptr = 0;
    JS_ToInt64(ctx, &ptr, opaqueVal);
    JS_FreeValue(ctx, opaqueVal);

    return reinterpret_cast<MutationObserverOpaque*>(static_cast<intptr_t>(ptr));
}

// Helper to convert MutationRecord to JS object
static JSValue createMutationRecord(JSContext* ctx, const dom::MutationRecord& record, JSBindings* bindings) {
    JSValue obj = JS_NewObject(ctx);

    // type property
    const char* typeStr = "childList";
    if (record.type == dom::MutationRecord::Type::ATTRIBUTES) {
        typeStr = "attributes";
    } else if (record.type == dom::MutationRecord::Type::CHARACTER_DATA) {
        typeStr = "characterData";
    }
    JS_SetPropertyStr(ctx, obj, "type", JS_NewString(ctx, typeStr));

    // target property
    if (record.target) {
        JSValue targetObj = bindings->createJSElement(ctx, record.target);
        JS_SetPropertyStr(ctx, obj, "target", targetObj);
    } else {
        JS_SetPropertyStr(ctx, obj, "target", JS_NULL);
    }

    // addedNodes property
    JSValue addedNodes = JS_NewArray(ctx);
    for (size_t i = 0; i < record.addedNodes.size(); i++) {
        JSValue nodeObj = bindings->createJSElement(ctx, record.addedNodes[i]);
        JS_SetPropertyUint32(ctx, addedNodes, static_cast<uint32_t>(i), nodeObj);
    }
    JS_SetPropertyStr(ctx, obj, "addedNodes", addedNodes);

    // removedNodes property
    JSValue removedNodes = JS_NewArray(ctx);
    for (size_t i = 0; i < record.removedNodes.size(); i++) {
        JSValue nodeObj = bindings->createJSElement(ctx, record.removedNodes[i]);
        JS_SetPropertyUint32(ctx, removedNodes, static_cast<uint32_t>(i), nodeObj);
    }
    JS_SetPropertyStr(ctx, obj, "removedNodes", removedNodes);

    // previousSibling property
    if (record.previousSibling) {
        JSValue siblingObj = bindings->createJSElement(ctx, record.previousSibling);
        JS_SetPropertyStr(ctx, obj, "previousSibling", siblingObj);
    } else {
        JS_SetPropertyStr(ctx, obj, "previousSibling", JS_NULL);
    }

    // nextSibling property
    if (record.nextSibling) {
        JSValue siblingObj = bindings->createJSElement(ctx, record.nextSibling);
        JS_SetPropertyStr(ctx, obj, "nextSibling", siblingObj);
    } else {
        JS_SetPropertyStr(ctx, obj, "nextSibling", JS_NULL);
    }

    // attributeName property
    if (!record.attributeName.empty()) {
        JS_SetPropertyStr(ctx, obj, "attributeName", JS_NewString(ctx, record.attributeName.c_str()));
    } else {
        JS_SetPropertyStr(ctx, obj, "attributeName", JS_NULL);
    }

    // attributeNamespace property
    if (!record.attributeNamespace.empty()) {
        JS_SetPropertyStr(ctx, obj, "attributeNamespace", JS_NewString(ctx, record.attributeNamespace.c_str()));
    } else {
        JS_SetPropertyStr(ctx, obj, "attributeNamespace", JS_NULL);
    }

    // oldValue property
    if (!record.oldValue.empty()) {
        JS_SetPropertyStr(ctx, obj, "oldValue", JS_NewString(ctx, record.oldValue.c_str()));
    } else {
        JS_SetPropertyStr(ctx, obj, "oldValue", JS_NULL);
    }

    return obj;
}

// MutationObserver constructor
static JSValue mutationObserverConstructor(JSContext* ctx, JSValueConst new_target,
                                           int argc, JSValueConst* argv) {
    (void)new_target;

    if (argc < 1 || !JS_IsFunction(ctx, argv[0])) {
        return JS_ThrowTypeError(ctx, "MutationObserver constructor requires a callback function");
    }

    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (!bindings) {
        return JS_ThrowInternalError(ctx, "No JSBindings in context");
    }

    auto* opaque = new MutationObserverOpaque();
    opaque->ctx = ctx;
    opaque->callback = JS_DupValue(ctx, argv[0]);
    opaque->bindings = bindings;
    opaque->jsObject = JS_UNDEFINED;

    // Create C++ MutationObserver
    opaque->observer = std::make_unique<dom::MutationObserver>(
        [opaque](const std::vector<dom::MutationRecord>& records, dom::MutationObserver* observer) {
            (void)observer;
            JSContext* ctx = opaque->ctx;

            // Convert records to JS array
            JSValue recordsArray = JS_NewArray(ctx);
            for (size_t i = 0; i < records.size(); i++) {
                JSValue recordObj = createMutationRecord(ctx, records[i], opaque->bindings);
                JS_SetPropertyUint32(ctx, recordsArray, static_cast<uint32_t>(i), recordObj);
            }

            // Call the JS callback
            JSValue args[2] = { recordsArray, opaque->jsObject };
            JSValue ret = JS_Call(ctx, opaque->callback, JS_UNDEFINED, 2, args);

            if (JS_IsException(ret)) {
                dumpJSError(ctx);
            }
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, recordsArray);
        }
    );

    // Create JS object
    JSValue obj = JS_NewObject(ctx);
    JSValue opaquePtr = JS_NewInt64(ctx, reinterpret_cast<intptr_t>(opaque));
    JS_SetPropertyStr(ctx, obj, MUTATION_OPAQUE_KEY, opaquePtr);
    opaque->jsObject = JS_DupValue(ctx, obj);

    return obj;
}

// MutationObserver.prototype.observe(target, options)
static JSValue mutationObserverObserve(JSContext* ctx, JSValueConst this_val,
                                        int argc, JSValueConst* argv) {
    auto* opaque = getMutationObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid MutationObserver");
    }

    if (argc < 2) {
        return JS_ThrowTypeError(ctx, "observe() requires target and options");
    }

    auto target = JSBindings::getNodeOpaque(ctx, argv[0]);
    if (!target) {
        return JS_ThrowTypeError(ctx, "observe() target must be a Node");
    }

    // Parse options
    dom::MutationObserverInit options;
    JSValue optionsObj = argv[1];

    JSValue childList = JS_GetPropertyStr(ctx, optionsObj, "childList");
    if (JS_IsBool(childList)) {
        options.childList = JS_ToBool(ctx, childList);
    }
    JS_FreeValue(ctx, childList);

    JSValue attributes = JS_GetPropertyStr(ctx, optionsObj, "attributes");
    if (JS_IsBool(attributes)) {
        options.attributes = JS_ToBool(ctx, attributes);
    }
    JS_FreeValue(ctx, attributes);

    JSValue characterData = JS_GetPropertyStr(ctx, optionsObj, "characterData");
    if (JS_IsBool(characterData)) {
        options.characterData = JS_ToBool(ctx, characterData);
    }
    JS_FreeValue(ctx, characterData);

    JSValue subtree = JS_GetPropertyStr(ctx, optionsObj, "subtree");
    if (JS_IsBool(subtree)) {
        options.subtree = JS_ToBool(ctx, subtree);
    }
    JS_FreeValue(ctx, subtree);

    JSValue attributeOldValue = JS_GetPropertyStr(ctx, optionsObj, "attributeOldValue");
    if (JS_IsBool(attributeOldValue)) {
        options.attributeOldValue = JS_ToBool(ctx, attributeOldValue);
    }
    JS_FreeValue(ctx, attributeOldValue);

    JSValue characterDataOldValue = JS_GetPropertyStr(ctx, optionsObj, "characterDataOldValue");
    if (JS_IsBool(characterDataOldValue)) {
        options.characterDataOldValue = JS_ToBool(ctx, characterDataOldValue);
    }
    JS_FreeValue(ctx, characterDataOldValue);

    // attributeFilter array
    JSValue attributeFilter = JS_GetPropertyStr(ctx, optionsObj, "attributeFilter");
    if (JS_IsArray(ctx, attributeFilter)) {
        JSValue lengthVal = JS_GetPropertyStr(ctx, attributeFilter, "length");
        uint32_t length = 0;
        JS_ToUint32(ctx, &length, lengthVal);
        JS_FreeValue(ctx, lengthVal);

        for (uint32_t i = 0; i < length; i++) {
            JSValue item = JS_GetPropertyUint32(ctx, attributeFilter, i);
            const char* str = JS_ToCString(ctx, item);
            if (str) {
                options.attributeFilter.push_back(str);
                JS_FreeCString(ctx, str);
            }
            JS_FreeValue(ctx, item);
        }
    }
    JS_FreeValue(ctx, attributeFilter);

    opaque->observer->observe(target, options);
    return JS_UNDEFINED;
}

// MutationObserver.prototype.disconnect()
static JSValue mutationObserverDisconnect(JSContext* ctx, JSValueConst this_val,
                                           int argc, JSValueConst* argv) {
    (void)argc; (void)argv;

    auto* opaque = getMutationObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid MutationObserver");
    }

    opaque->observer->disconnect();
    return JS_UNDEFINED;
}

// MutationObserver.prototype.takeRecords()
static JSValue mutationObserverTakeRecords(JSContext* ctx, JSValueConst this_val,
                                            int argc, JSValueConst* argv) {
    (void)argc; (void)argv;

    auto* opaque = getMutationObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid MutationObserver");
    }

    auto records = opaque->observer->takeRecords();
    JSValue recordsArray = JS_NewArray(ctx);
    for (size_t i = 0; i < records.size(); i++) {
        JSValue recordObj = createMutationRecord(ctx, records[i], opaque->bindings);
        JS_SetPropertyUint32(ctx, recordsArray, static_cast<uint32_t>(i), recordObj);
    }

    return recordsArray;
}

// ============================================================================
// IntersectionObserver Implementation
// ============================================================================

struct IntersectionObserverOpaque {
    std::unique_ptr<dom::IntersectionObserver> observer;
    JSContext* ctx;
    JSValue callback;
    JSValue jsObject;
    JSBindings* bindings;
};

static constexpr const char* INTERSECTION_OPAQUE_KEY = "\xFF_intersection_opaque";

// Helper to get IntersectionObserver opaque data
static IntersectionObserverOpaque* getIntersectionObserverOpaque(JSContext* ctx, JSValueConst obj) {
    JSValue opaqueVal = JS_GetPropertyStr(ctx, obj, INTERSECTION_OPAQUE_KEY);
    if (JS_IsUndefined(opaqueVal)) {
        JS_FreeValue(ctx, opaqueVal);
        return nullptr;
    }

    int64_t ptr = 0;
    JS_ToInt64(ctx, &ptr, opaqueVal);
    JS_FreeValue(ctx, opaqueVal);

    return reinterpret_cast<IntersectionObserverOpaque*>(static_cast<intptr_t>(ptr));
}

// Helper to create IntersectionObserverEntry
static JSValue createIntersectionObserverEntry(JSContext* ctx, const dom::IntersectionObserverEntry& entry, JSBindings* bindings) {
    JSValue obj = JS_NewObject(ctx);

    // target property
    if (entry.target) {
        JSValue targetObj = bindings->createJSElement(ctx, entry.target);
        JS_SetPropertyStr(ctx, obj, "target", targetObj);
    } else {
        JS_SetPropertyStr(ctx, obj, "target", JS_NULL);
    }

    // Helper to create DOMRectReadOnly from IntersectionObserverEntry::DOMRectReadOnly
    auto createRect = [ctx](const dom::IntersectionObserverEntry::DOMRectReadOnly& rect) -> JSValue {
        JSValue rectObj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, rectObj, "x", JS_NewFloat64(ctx, rect.x));
        JS_SetPropertyStr(ctx, rectObj, "y", JS_NewFloat64(ctx, rect.y));
        JS_SetPropertyStr(ctx, rectObj, "width", JS_NewFloat64(ctx, rect.width));
        JS_SetPropertyStr(ctx, rectObj, "height", JS_NewFloat64(ctx, rect.height));
        JS_SetPropertyStr(ctx, rectObj, "top", JS_NewFloat64(ctx, rect.top));
        JS_SetPropertyStr(ctx, rectObj, "right", JS_NewFloat64(ctx, rect.right));
        JS_SetPropertyStr(ctx, rectObj, "bottom", JS_NewFloat64(ctx, rect.bottom));
        JS_SetPropertyStr(ctx, rectObj, "left", JS_NewFloat64(ctx, rect.left));
        return rectObj;
    };

    // boundingClientRect property
    JSValue boundingClientRect = createRect(entry.boundingClientRect);
    JS_SetPropertyStr(ctx, obj, "boundingClientRect", boundingClientRect);

    // intersectionRect property
    JSValue intersectionRect = createRect(entry.intersectionRect);
    JS_SetPropertyStr(ctx, obj, "intersectionRect", intersectionRect);

    // rootBounds property
    JSValue rootBounds = createRect(entry.rootBounds);
    JS_SetPropertyStr(ctx, obj, "rootBounds", rootBounds);

    // intersectionRatio property
    JS_SetPropertyStr(ctx, obj, "intersectionRatio", JS_NewFloat64(ctx, entry.intersectionRatio));

    // isIntersecting property
    JS_SetPropertyStr(ctx, obj, "isIntersecting", JS_NewBool(ctx, entry.isIntersecting));

    // time property
    JS_SetPropertyStr(ctx, obj, "time", JS_NewFloat64(ctx, entry.time));

    return obj;
}

// IntersectionObserver constructor
static JSValue intersectionObserverConstructor(JSContext* ctx, JSValueConst new_target,
                                               int argc, JSValueConst* argv) {
    (void)new_target;

    if (argc < 1 || !JS_IsFunction(ctx, argv[0])) {
        return JS_ThrowTypeError(ctx, "IntersectionObserver constructor requires a callback function");
    }

    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (!bindings) {
        return JS_ThrowInternalError(ctx, "No JSBindings in context");
    }

    // Parse options
    dom::IntersectionObserverInit options;
    if (argc >= 2 && JS_IsObject(argv[1])) {
        JSValue optionsObj = argv[1];

        // root property
        JSValue root = JS_GetPropertyStr(ctx, optionsObj, "root");
        if (!JS_IsNull(root) && !JS_IsUndefined(root)) {
            options.root = JSBindings::getNodeOpaque(ctx, root);
        }
        JS_FreeValue(ctx, root);

        // rootMargin property
        JSValue rootMargin = JS_GetPropertyStr(ctx, optionsObj, "rootMargin");
        const char* marginStr = JS_ToCString(ctx, rootMargin);
        if (marginStr) {
            options.rootMargin = marginStr;
            JS_FreeCString(ctx, marginStr);
        }
        JS_FreeValue(ctx, rootMargin);

        // threshold property
        JSValue threshold = JS_GetPropertyStr(ctx, optionsObj, "threshold");
        if (JS_IsArray(ctx, threshold)) {
            JSValue lengthVal = JS_GetPropertyStr(ctx, threshold, "length");
            uint32_t length = 0;
            JS_ToUint32(ctx, &length, lengthVal);
            JS_FreeValue(ctx, lengthVal);

            options.threshold.clear();
            for (uint32_t i = 0; i < length; i++) {
                JSValue item = JS_GetPropertyUint32(ctx, threshold, i);
                double val = 0;
                JS_ToFloat64(ctx, &val, item);
                options.threshold.push_back(static_cast<float>(val));
                JS_FreeValue(ctx, item);
            }
        } else if (JS_IsNumber(threshold)) {
            double val = 0;
            JS_ToFloat64(ctx, &val, threshold);
            options.threshold = {static_cast<float>(val)};
        }
        JS_FreeValue(ctx, threshold);
    }

    auto* opaque = new IntersectionObserverOpaque();
    opaque->ctx = ctx;
    opaque->callback = JS_DupValue(ctx, argv[0]);
    opaque->bindings = bindings;
    opaque->jsObject = JS_UNDEFINED;

    // Create C++ IntersectionObserver
    opaque->observer = std::make_unique<dom::IntersectionObserver>(
        [opaque](const std::vector<dom::IntersectionObserverEntry>& entries, dom::IntersectionObserver* observer) {
            (void)observer;
            JSContext* ctx = opaque->ctx;

            // Convert entries to JS array
            JSValue entriesArray = JS_NewArray(ctx);
            for (size_t i = 0; i < entries.size(); i++) {
                JSValue entryObj = createIntersectionObserverEntry(ctx, entries[i], opaque->bindings);
                JS_SetPropertyUint32(ctx, entriesArray, static_cast<uint32_t>(i), entryObj);
            }

            // Call the JS callback
            JSValue args[2] = { entriesArray, opaque->jsObject };
            JSValue ret = JS_Call(ctx, opaque->callback, JS_UNDEFINED, 2, args);

            if (JS_IsException(ret)) {
                dumpJSError(ctx);
            }
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, entriesArray);
        },
        options
    );

    // Create JS object
    JSValue obj = JS_NewObject(ctx);
    JSValue opaquePtr = JS_NewInt64(ctx, reinterpret_cast<intptr_t>(opaque));
    JS_SetPropertyStr(ctx, obj, INTERSECTION_OPAQUE_KEY, opaquePtr);
    opaque->jsObject = JS_DupValue(ctx, obj);

    return obj;
}

// IntersectionObserver.prototype.observe(target)
static JSValue intersectionObserverObserve(JSContext* ctx, JSValueConst this_val,
                                            int argc, JSValueConst* argv) {
    (void)argc;

    auto* opaque = getIntersectionObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid IntersectionObserver");
    }

    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "observe() requires a target element");
    }

    auto target = JSBindings::getNodeOpaque(ctx, argv[0]);
    if (!target) {
        return JS_ThrowTypeError(ctx, "observe() target must be an Element");
    }

    opaque->observer->observe(target);
    return JS_UNDEFINED;
}

// IntersectionObserver.prototype.unobserve(target)
static JSValue intersectionObserverUnobserve(JSContext* ctx, JSValueConst this_val,
                                              int argc, JSValueConst* argv) {
    (void)argc;

    auto* opaque = getIntersectionObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid IntersectionObserver");
    }

    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "unobserve() requires a target element");
    }

    auto target = JSBindings::getNodeOpaque(ctx, argv[0]);
    if (!target) {
        return JS_ThrowTypeError(ctx, "unobserve() target must be an Element");
    }

    opaque->observer->unobserve(target);
    return JS_UNDEFINED;
}

// IntersectionObserver.prototype.disconnect()
static JSValue intersectionObserverDisconnect(JSContext* ctx, JSValueConst this_val,
                                               int argc, JSValueConst* argv) {
    (void)argc; (void)argv;

    auto* opaque = getIntersectionObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid IntersectionObserver");
    }

    opaque->observer->disconnect();
    return JS_UNDEFINED;
}

// IntersectionObserver.prototype.takeRecords()
static JSValue intersectionObserverTakeRecords(JSContext* ctx, JSValueConst this_val,
                                                int argc, JSValueConst* argv) {
    (void)argc; (void)argv;

    auto* opaque = getIntersectionObserverOpaque(ctx, this_val);
    if (!opaque || !opaque->observer) {
        return JS_ThrowTypeError(ctx, "Invalid IntersectionObserver");
    }

    auto entries = opaque->observer->takeRecords();
    JSValue entriesArray = JS_NewArray(ctx);
    for (size_t i = 0; i < entries.size(); i++) {
        JSValue entryObj = createIntersectionObserverEntry(ctx, entries[i], opaque->bindings);
        JS_SetPropertyUint32(ctx, entriesArray, static_cast<uint32_t>(i), entryObj);
    }

    return entriesArray;
}

// ============================================================================
// Public API
// ============================================================================

void initializeObserverAPI(JSContext* ctx, JSBindings* bindings) {
    (void)bindings;

    // ResizeObserver
    JSValue resizeObserverCtor = JS_NewCFunction2(ctx, resizeObserverConstructor,
                                                   "ResizeObserver", 1,
                                                   JS_CFUNC_constructor, 0);

    JSValue resizeObserverProto = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, resizeObserverProto, "observe",
                      JS_NewCFunction(ctx, resizeObserverObserve, "observe", 1));
    JS_SetPropertyStr(ctx, resizeObserverProto, "unobserve",
                      JS_NewCFunction(ctx, resizeObserverUnobserve, "unobserve", 1));
    JS_SetPropertyStr(ctx, resizeObserverProto, "disconnect",
                      JS_NewCFunction(ctx, resizeObserverDisconnect, "disconnect", 0));

    JS_SetPropertyStr(ctx, resizeObserverCtor, "prototype", resizeObserverProto);

    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "ResizeObserver", resizeObserverCtor);

    // MutationObserver
    JSValue mutationObserverCtor = JS_NewCFunction2(ctx, mutationObserverConstructor,
                                                     "MutationObserver", 1,
                                                     JS_CFUNC_constructor, 0);

    JSValue mutationObserverProto = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, mutationObserverProto, "observe",
                      JS_NewCFunction(ctx, mutationObserverObserve, "observe", 2));
    JS_SetPropertyStr(ctx, mutationObserverProto, "disconnect",
                      JS_NewCFunction(ctx, mutationObserverDisconnect, "disconnect", 0));
    JS_SetPropertyStr(ctx, mutationObserverProto, "takeRecords",
                      JS_NewCFunction(ctx, mutationObserverTakeRecords, "takeRecords", 0));

    JS_SetPropertyStr(ctx, mutationObserverCtor, "prototype", mutationObserverProto);
    JS_SetPropertyStr(ctx, global, "MutationObserver", mutationObserverCtor);

    // IntersectionObserver
    JSValue intersectionObserverCtor = JS_NewCFunction2(ctx, intersectionObserverConstructor,
                                                         "IntersectionObserver", 1,
                                                         JS_CFUNC_constructor, 0);

    JSValue intersectionObserverProto = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, intersectionObserverProto, "observe",
                      JS_NewCFunction(ctx, intersectionObserverObserve, "observe", 1));
    JS_SetPropertyStr(ctx, intersectionObserverProto, "unobserve",
                      JS_NewCFunction(ctx, intersectionObserverUnobserve, "unobserve", 1));
    JS_SetPropertyStr(ctx, intersectionObserverProto, "disconnect",
                      JS_NewCFunction(ctx, intersectionObserverDisconnect, "disconnect", 0));
    JS_SetPropertyStr(ctx, intersectionObserverProto, "takeRecords",
                      JS_NewCFunction(ctx, intersectionObserverTakeRecords, "takeRecords", 0));

    JS_SetPropertyStr(ctx, intersectionObserverCtor, "prototype", intersectionObserverProto);
    JS_SetPropertyStr(ctx, global, "IntersectionObserver", intersectionObserverCtor);

    JS_FreeValue(ctx, global);
}

} // namespace dong::script
