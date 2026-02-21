#include "script_engine.hpp"

#include "../core/log.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace dong::script {

namespace {

using SteadyClock = std::chrono::steady_clock;

uint64_t steadyNowNs() {
    const auto now = SteadyClock::now().time_since_epoch();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}

uint64_t getScriptTimeoutNs() {
    // Default: 2000ms to avoid hard hangs during page/script load.
    const char* s = std::getenv("DONG_SCRIPT_TIMEOUT_MS");
    if (!s || !*s) {
        return 2000ULL * 1000ULL * 1000ULL;
    }
    char* end = nullptr;
    long ms = std::strtol(s, &end, 10);
    if (!end || end == s) {
        return 2000ULL * 1000ULL * 1000ULL;
    }
    if (ms <= 0) {
        return 0;
    }
    return static_cast<uint64_t>(ms) * 1000ULL * 1000ULL;
}

void printJsValueToStderr(JSContext* ctx, JSValueConst v) {
    const char* s = JS_ToCString(ctx, v);
    if (s) {
        std::fprintf(stderr, "%s", s);
        JS_FreeCString(ctx, s);
    }
}

void dumpJsException(JSContext* ctx, const char* where) {
    JSValue exception = JS_GetException(ctx);

    std::fprintf(stderr, "[ScriptEngine] JS exception (%s): ", where ? where : "unknown");
    printJsValueToStderr(ctx, exception);
    std::fprintf(stderr, "\n");

    // Try to print stack if present.
    JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
    if (!JS_IsUndefined(stack) && !JS_IsNull(stack)) {
        std::fprintf(stderr, "[ScriptEngine] stack: ");
        printJsValueToStderr(ctx, stack);
        std::fprintf(stderr, "\n");
    }
    JS_FreeValue(ctx, stack);
    JS_FreeValue(ctx, exception);
}

std::string jsValueToStdString(JSContext* ctx, JSValueConst v) {
    if (JS_IsString(v)) {
        const char* s = JS_ToCString(ctx, v);
        if (!s) {
            return {};
        }
        std::string out = s;
        JS_FreeCString(ctx, s);
        return out;
    }

    if (JS_IsNumber(v)) {
        double num = 0;
        JS_ToFloat64(ctx, &num, v);
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.17g", num);
        return buf;
    }

    if (JS_IsBool(v)) {
        return JS_ToBool(ctx, v) ? "true" : "false";
    }

    if (JS_IsNull(v)) {
        return "null";
    }

    if (JS_IsUndefined(v)) {
        return "undefined";
    }

    if (JS_IsObject(v)) {
        // Best-effort: JSON.stringify(value)
        JSValue global = JS_GetGlobalObject(ctx);
        JSValue json = JS_GetPropertyStr(ctx, global, "JSON");
        JSValue stringify = JS_UNDEFINED;
        std::string out;

        if (!JS_IsUndefined(json) && !JS_IsNull(json)) {
            stringify = JS_GetPropertyStr(ctx, json, "stringify");
            if (JS_IsFunction(ctx, stringify)) {
                JSValue str_val = JS_Call(ctx, stringify, json, 1, &v);
                if (!JS_IsException(str_val)) {
                    out = jsValueToStdString(ctx, str_val);
                }
                JS_FreeValue(ctx, str_val);
            }
        }

        JS_FreeValue(ctx, stringify);
        JS_FreeValue(ctx, json);
        JS_FreeValue(ctx, global);
        return out;
    }

    // Fallback: stringify via JS_ToCString.
    const char* s = JS_ToCString(ctx, v);
    if (!s) {
        return {};
    }
    std::string out = s;
    JS_FreeCString(ctx, s);
    return out;
}

} // namespace

int ScriptEngine::interruptHandler(JSRuntime* rt, void* opaque) {
    (void)rt;
    auto* self = static_cast<ScriptEngine*>(opaque);
    if (!self) {
        return 0;
    }

    const uint64_t deadline = self->interrupt_deadline_ns_;
    if (deadline == 0) {
        return 0;
    }

    return steadyNowNs() > deadline ? 1 : 0;
}

ScriptEngine::ScriptEngine() : runtime_(nullptr), context_(nullptr) {
    runtime_ = JS_NewRuntime();
    if (!runtime_) {
        return;
    }

    JS_SetInterruptHandler(runtime_, &ScriptEngine::interruptHandler, this);

    context_ = JS_NewContext(runtime_);
    if (!context_) {
        JS_FreeRuntime(runtime_);
        runtime_ = nullptr;
        return;
    }

    initializeBuiltins();
}

ScriptEngine::~ScriptEngine() {
    // QuickJS teardown is intentionally guarded:
    // - In some configurations we still leak JSValues (C-side refs), and JS_FreeRuntime asserts.
    // - Default behavior keeps the process stable; opt-in teardown for leak hunting.

    const bool teardown = (std::getenv("DONG_QUICKJS_TEARDOWN") != nullptr);
    if (!teardown) {
        return;
    }

    processPendingTasks();

    if (context_) {
        JS_FreeContext(context_);
        context_ = nullptr;
    }

    if (runtime_) {
        JS_RunGC(runtime_);
        JS_FreeRuntime(runtime_);
        runtime_ = nullptr;
    }
}

bool ScriptEngine::eval(const std::string& code) {
    if (!context_) {
        return false;
    }

    const bool dbg = (std::getenv("DONG_DEBUG_SCRIPT_EVAL") != nullptr);
    if (dbg) {
        DONG_LOG_INFO("[ScriptEngine] eval begin (len=%zu)", code.length());
    }

    if (const char* dump_path = std::getenv("DONG_DEBUG_DUMP_SCRIPT_PATH")) {
        if (dump_path[0] != '\0') {
            if (FILE* f = std::fopen(dump_path, "wb")) {
                (void)std::fwrite(code.data(), 1, code.size(), f);
                std::fclose(f);
                if (dbg) {
                    DONG_LOG_INFO("[ScriptEngine] dumped script to %s", dump_path);
                }
            } else if (dbg) {
                DONG_LOG_WARN("[ScriptEngine] failed to open dump path: %s", dump_path);
            }
        }
    }

    if (dbg) {
        DONG_LOG_INFO("[ScriptEngine] eval compile begin");
    }

    JSValue compiled = JS_Eval(context_, code.c_str(), code.length(), "<eval>",
                               JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_COMPILE_ONLY);

    if (dbg) {
        DONG_LOG_INFO("[ScriptEngine] eval compile done (is_exception=%d)", JS_IsException(compiled));
    }

    if (JS_IsException(compiled)) {
        dumpJsException(context_, "eval/compile");
        JS_FreeValue(context_, compiled);
        return false;
    }

    const uint64_t timeout_ns = getScriptTimeoutNs();
    if (timeout_ns != 0) {
        interrupt_deadline_ns_ = steadyNowNs() + timeout_ns;
    } else {
        interrupt_deadline_ns_ = 0;
    }

    if (dbg) {
        DONG_LOG_INFO("[ScriptEngine] eval exec begin");
    }

    JSValue result = JS_EvalFunction(context_, compiled);

    // Always clear the deadline after a single eval.
    interrupt_deadline_ns_ = 0;

    if (dbg) {
        DONG_LOG_INFO("[ScriptEngine] eval exec done (is_exception=%d)", JS_IsException(result));
    }

    if (JS_IsException(result)) {
        dumpJsException(context_, "eval/exec");
        JS_FreeValue(context_, result);
        return false;
    }

    JS_FreeValue(context_, result);
    processPendingTasks();

    if (dbg) {
        DONG_LOG_INFO("[ScriptEngine] eval done");
    }

    return true;
}

std::string ScriptEngine::evalWithReturn(const std::string& code) {
    if (!context_) {
        return "";
    }

    JSValue compiled = JS_Eval(context_, code.c_str(), code.length(), "<eval>",
                               JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(compiled)) {
        dumpJsException(context_, "evalWithReturn/compile");
        JS_FreeValue(context_, compiled);
        return "";
    }

    const uint64_t timeout_ns = getScriptTimeoutNs();
    if (timeout_ns != 0) {
        interrupt_deadline_ns_ = steadyNowNs() + timeout_ns;
    } else {
        interrupt_deadline_ns_ = 0;
    }

    JSValue result = JS_EvalFunction(context_, compiled);
    interrupt_deadline_ns_ = 0;

    if (JS_IsException(result)) {
        dumpJsException(context_, "evalWithReturn/exec");
        JS_FreeValue(context_, result);
        return "";
    }

    std::string out = jsValueToStdString(context_, result);
    JS_FreeValue(context_, result);
    processPendingTasks();
    return out;
}

JSValue* ScriptEngine::callFunction(const std::string& function_name, int argc, JSValue* argv) {
    if (!context_) {
        return nullptr;
    }

    JSValue global = JS_GetGlobalObject(context_);
    JSValue func = JS_GetPropertyStr(context_, global, function_name.c_str());

    if (!JS_IsFunction(context_, func)) {
        JS_FreeValue(context_, func);
        JS_FreeValue(context_, global);
        return nullptr;
    }

    JSValue result = JS_Call(context_, func, global, argc, argv);

    JS_FreeValue(context_, func);
    JS_FreeValue(context_, global);

    if (JS_IsException(result)) {
        dumpJsException(context_, "callFunction");
        JS_FreeValue(context_, result);
        return nullptr;
    }

    return new JSValue(result);
}

void ScriptEngine::bindGlobalObject(const std::string& name, void* object) {
    if (!context_) {
        return;
    }

    JSValue global = JS_GetGlobalObject(context_);
    JSValue obj = JS_NewObject(context_);

    // Store as opaque pointer. Callers must know how to retrieve it.
    JS_SetOpaque(obj, object);

    JS_SetPropertyStr(context_, global, name.c_str(), obj);
    JS_FreeValue(context_, global);
}

void ScriptEngine::bindGlobalFunction(const std::string& name, JSCFunction* func, int argc) {
    if (!context_) {
        return;
    }

    JSValue global = JS_GetGlobalObject(context_);
    JSValue js_func = JS_NewCFunction(context_, func, name.c_str(), argc);
    JS_SetPropertyStr(context_, global, name.c_str(), js_func);
    JS_FreeValue(context_, global);
}

void ScriptEngine::processPendingTasks() {
    if (!context_ || !runtime_) {
        return;
    }

    JSContext* ctx = nullptr;
    int executed = 0;
    constexpr int kMaxJobs = 1000;

    while (executed < kMaxJobs) {
        const int rc = JS_ExecutePendingJob(runtime_, &ctx);
        if (rc < 0) {
            if (ctx) {
                dumpJsException(ctx, "microtask");
            }
            break;
        }
        if (rc == 0) {
            break;
        }
        ++executed;
    }
}

void ScriptEngine::initializeBuiltins() {
    // Builtins like console are registered by the DOM bindings layer.
}

} // namespace dong::script
