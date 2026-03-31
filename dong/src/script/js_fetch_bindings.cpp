#include "js_fetch_bindings.hpp"
#include "js_bindings.hpp"
#include "../core/log.h"
#include "../core/resource_loader.hpp"

#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

namespace dong::script {

// ============================================================
// Pending fetch result queue (thread-safe)
// ============================================================

struct FetchCompletion {
    JSValue resolve;
    JSValue reject;
    dong::ResourceLoadResult result;
    std::string url;
};

static std::mutex g_fetch_mutex;
static std::vector<FetchCompletion> g_fetch_completions;
static std::atomic<int> g_pending_count{0};

// ============================================================
// Response object construction (main thread only)
// ============================================================

static JSValue createResponseObject(JSContext* ctx, int status_code,
                                    const std::string& body, const std::string& url) {
    JSValue resp = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, resp, "status", JS_NewInt32(ctx, status_code));
    JS_SetPropertyStr(ctx, resp, "ok",
                      JS_NewBool(ctx, status_code >= 200 && status_code < 300));
    JS_SetPropertyStr(ctx, resp, "url", JS_NewString(ctx, url.c_str()));
    JS_SetPropertyStr(ctx, resp, "statusText",
                      JS_NewString(ctx, (status_code >= 200 && status_code < 300) ? "OK" : "Error"));

    // Capture body string for text() and json()
    JSValue body_str = JS_NewStringLen(ctx, body.c_str(), body.size());

    // response.text() -> Promise<string>
    JSValue text_func = JS_NewCFunctionData(ctx, [](JSContext* c, JSValueConst, int,
                                                     JSValueConst*, int, JSValue* data) -> JSValue {
        JSValue resolving[2];
        JSValue promise = JS_NewPromiseCapability(c, resolving);
        if (JS_IsException(promise)) return promise;

        JSValue result = JS_DupValue(c, data[0]);
        JS_Call(c, resolving[0], JS_UNDEFINED, 1, &result);
        JS_FreeValue(c, result);
        JS_FreeValue(c, resolving[0]);
        JS_FreeValue(c, resolving[1]);
        return promise;
    }, 0, 0, 1, &body_str);
    JS_SetPropertyStr(ctx, resp, "text", text_func);

    // response.json() -> Promise<object>
    JSValue json_func = JS_NewCFunctionData(ctx, [](JSContext* c, JSValueConst, int,
                                                     JSValueConst*, int, JSValue* data) -> JSValue {
        JSValue resolving[2];
        JSValue promise = JS_NewPromiseCapability(c, resolving);
        if (JS_IsException(promise)) return promise;

        const char* str = JS_ToCString(c, data[0]);
        JSValue parsed = JS_ParseJSON(c, str, strlen(str), "<fetch>");
        JS_FreeCString(c, str);

        if (JS_IsException(parsed)) {
            JSValue ex = JS_GetException(c);
            JS_Call(c, resolving[1], JS_UNDEFINED, 1, &ex);
            JS_FreeValue(c, ex);
        } else {
            JS_Call(c, resolving[0], JS_UNDEFINED, 1, &parsed);
        }
        JS_FreeValue(c, parsed);
        JS_FreeValue(c, resolving[0]);
        JS_FreeValue(c, resolving[1]);
        return promise;
    }, 0, 0, 1, &body_str);
    JS_SetPropertyStr(ctx, resp, "json", json_func);

    JS_FreeValue(ctx, body_str);
    return resp;
}

// ============================================================
// fetch() implementation
// ============================================================

static JSValue js_fetch(JSContext* ctx, JSValueConst this_val,
                        int argc, JSValueConst* argv) {
    (void)this_val;

    if (argc < 1) {
        return JS_ThrowTypeError(ctx, "fetch requires at least 1 argument");
    }

    const char* url_cstr = JS_ToCString(ctx, argv[0]);
    if (!url_cstr) {
        return JS_ThrowTypeError(ctx, "fetch: invalid URL argument");
    }
    std::string url(url_cstr);
    JS_FreeCString(ctx, url_cstr);

    std::string resource_root;
    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (bindings && bindings->dom_manager_) {
        resource_root = bindings->dom_manager_->getResourceRoot();
    }

    // Create Promise
    JSValue resolving_funcs[2];
    JSValue promise = JS_NewPromiseCapability(ctx, resolving_funcs);
    if (JS_IsException(promise)) {
        return promise;
    }

    // For local files, do synchronous resolution (fast path)
    if (!dong::isHttpUrl(url)) {
        auto result = dong::loadTextResource(url, resource_root);
        if (result.success) {
            JSValue resp = createResponseObject(ctx, 200, result.content, url);
            JS_Call(ctx, resolving_funcs[0], JS_UNDEFINED, 1, &resp);
            JS_FreeValue(ctx, resp);
        } else {
            JSValue err = JS_NewString(ctx, result.error_msg.c_str());
            JS_Call(ctx, resolving_funcs[1], JS_UNDEFINED, 1, &err);
            JS_FreeValue(ctx, err);
        }
        JS_FreeValue(ctx, resolving_funcs[0]);
        JS_FreeValue(ctx, resolving_funcs[1]);
        return promise;
    }

    // For HTTP requests, spawn a worker thread
    JSValue resolve = JS_DupValue(ctx, resolving_funcs[0]);
    JSValue reject  = JS_DupValue(ctx, resolving_funcs[1]);
    JS_FreeValue(ctx, resolving_funcs[0]);
    JS_FreeValue(ctx, resolving_funcs[1]);

    g_pending_count.fetch_add(1);

    std::thread([url, resolve, reject]() {
        auto result = dong::loadTextResource(url, "");

        std::lock_guard<std::mutex> lock(g_fetch_mutex);
        g_fetch_completions.push_back({resolve, reject, std::move(result), url});
    }).detach();

    return promise;
}

// ============================================================
// Tick: drain completed fetches on the main thread
// ============================================================

void tickPendingFetches(JSContext* ctx) {
    if (!ctx || g_pending_count.load() == 0) return;

    std::vector<FetchCompletion> completions;
    {
        std::lock_guard<std::mutex> lock(g_fetch_mutex);
        completions.swap(g_fetch_completions);
    }

    for (auto& c : completions) {
        g_pending_count.fetch_sub(1);
        if (c.result.success) {
            int status = c.result.status_code > 0 ? c.result.status_code : 200;
            JSValue resp = createResponseObject(ctx, status, c.result.content, c.url);
            JS_Call(ctx, c.resolve, JS_UNDEFINED, 1, &resp);
            JS_FreeValue(ctx, resp);
        } else {
            JSValue err = JS_NewString(ctx, c.result.error_msg.c_str());
            JS_Call(ctx, c.reject, JS_UNDEFINED, 1, &err);
            JS_FreeValue(ctx, err);
        }
        JS_FreeValue(ctx, c.resolve);
        JS_FreeValue(ctx, c.reject);
    }
}

void resetFetchState(JSContext* ctx) {
    std::lock_guard<std::mutex> lock(g_fetch_mutex);
    for (auto& c : g_fetch_completions) {
        if (ctx) {
            JS_FreeValue(ctx, c.resolve);
            JS_FreeValue(ctx, c.reject);
        }
    }
    g_fetch_completions.clear();
    g_pending_count.store(0);
}

// ============================================================
// Initialization
// ============================================================

void initializeFetchAPI(JSContext* ctx, JSBindings* bindings) {
    (void)bindings;
    if (!ctx) return;

    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "fetch",
                      JS_NewCFunction(ctx, js_fetch, "fetch", 2));
    JS_FreeValue(ctx, global);

    DONG_LOG_INFO("[Fetch] fetch() API registered");
}

} // namespace dong::script
