#include "script_engine.hpp"
#include <cstring>
#include <cstdio>
#include "../core/log.h"

namespace dong::script {

// 绠€鍗曠殑 console.log 瀹炵幇锛岀洿鎺ユ墦鍗板埌 stderr
static JSValue js_console_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
    (void)this_val;
    for (int i = 0; i < argc; i++) {
        if (i > 0) std::fprintf(stderr, " ");
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            std::fprintf(stderr, "%s", str);
            JS_FreeCString(ctx, str);
        }
    }
    std::fprintf(stderr, "\n");
    return JS_UNDEFINED;
}

ScriptEngine::ScriptEngine() : runtime_(nullptr), context_(nullptr) {
    // 鍒涘缓 QuickJS 杩愯鏃?
    runtime_ = JS_NewRuntime();
    if (!runtime_) return;

    // 鍒涘缓 JavaScript 涓婁笅鏂?
    context_ = JS_NewContext(runtime_);
    if (!context_) {
        JS_FreeRuntime(runtime_);
        runtime_ = nullptr;
        return;
    }

    // 鍒濆鍖栧唴缃粦瀹?
    initializeBuiltins();
}

ScriptEngine::~ScriptEngine() {
    // NOTE: QuickJS teardown is temporarily disabled to avoid a crash during
    // demo shutdown. The OS will reclaim this memory when the process exits.
    // if (context_) {
    //     JS_FreeContext(context_);
    // }
    // if (runtime_) {
    //     JS_FreeRuntime(runtime_);
    // }
}

bool ScriptEngine::eval(const std::string& code) {
    DONG_LOG_DEBUG("[ScriptEngine::eval] Entry, context_=%p, code.length()=%zu", (void*)context_, code.length());
    if (!context_) {
        DONG_LOG_DEBUG("[ScriptEngine::eval] context_ is null, returning false");
        return false;
    }

    // Print first 100 chars of code for debugging
    std::string preview = code.substr(0, 100);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Code preview: %.100s...", preview.c_str());
    
    // Test 1: simple expression
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 1: simple expression...");
    JSValue test1 = JS_Eval(context_, "1+1", 3, "<test1>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(test1)) {
        DONG_LOG_DEBUG("[ScriptEngine::eval] Test 1 FAILED!");
        JS_FreeValue(context_, test1);
        return false;
    }
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 1 passed");
    JS_FreeValue(context_, test1);
    
    // Test 2: Check if console exists
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 2: checking console object..."); fflush(stdout); fflush(stderr);
    JSValue global = JS_GetGlobalObject(context_);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 2a: got global"); fflush(stdout); fflush(stderr);
    JSValue console_val = JS_GetPropertyStr(context_, global, "console");
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 2b: got console property"); fflush(stdout); fflush(stderr);
    DONG_LOG_DEBUG("[ScriptEngine::eval] console is undefined: %d, null: %d, object: %d",
            JS_IsUndefined(console_val), JS_IsNull(console_val), JS_IsObject(console_val));
    fflush(stdout); fflush(stderr);
    
    if (JS_IsObject(console_val)) {
        DONG_LOG_DEBUG("[ScriptEngine::eval] Test 2c: getting log property..."); fflush(stdout); fflush(stderr);
        JSValue log_val = JS_GetPropertyStr(context_, console_val, "log");
        DONG_LOG_DEBUG("[ScriptEngine::eval] Test 2d: got log property"); fflush(stdout); fflush(stderr);
        DONG_LOG_DEBUG("[ScriptEngine::eval] console.log is undefined: %d, function: %d",
                JS_IsUndefined(log_val), JS_IsFunction(context_, log_val));
        fflush(stdout); fflush(stderr);
        JS_FreeValue(context_, log_val);
    }
    JS_FreeValue(context_, console_val);
    JS_FreeValue(context_, global);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 2 done"); fflush(stdout); fflush(stderr);
    
    // Test 3: Try calling console.log directly via JS_Call
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 3: calling console.log via JS_Call..."); fflush(stdout); fflush(stderr);
    global = JS_GetGlobalObject(context_);
    console_val = JS_GetPropertyStr(context_, global, "console");
    if (JS_IsObject(console_val)) {
        JSValue log_func = JS_GetPropertyStr(context_, console_val, "log");
        if (JS_IsFunction(context_, log_func)) {
            JSValue arg = JS_NewString(context_, "direct call test");
            DONG_LOG_DEBUG("[ScriptEngine::eval] Test 3a: About to call JS_Call..."); fflush(stdout); fflush(stderr);
            JSValue call_result = JS_Call(context_, log_func, console_val, 1, &arg);
            DONG_LOG_DEBUG("[ScriptEngine::eval] Test 3b: JS_Call returned"); fflush(stdout); fflush(stderr);
            JS_FreeValue(context_, call_result);
            JS_FreeValue(context_, arg);
        }
        JS_FreeValue(context_, log_func);
    }
    JS_FreeValue(context_, console_val);
    JS_FreeValue(context_, global);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 3 done"); fflush(stdout); fflush(stderr);
    
    // Test 4: Narrowing down the crash
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4: Narrowing down..."); fflush(stdout); fflush(stderr);
    
    // Test 4a: Simple number
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4a: Just a number..."); fflush(stdout); fflush(stderr);
    const char* test4a_code = "42";
    JSValue test4a = JS_Eval(context_, test4a_code, strlen(test4a_code), "<test4a>", JS_EVAL_TYPE_GLOBAL);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4a returned, exception: %d", JS_IsException(test4a)); fflush(stdout); fflush(stderr);
    JS_FreeValue(context_, test4a);
    
    // Test 4b: Simple string
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4b: Just a string..."); fflush(stdout); fflush(stderr);
    const char* test4b_code = "'hello'";
    JSValue test4b = JS_Eval(context_, test4b_code, strlen(test4b_code), "<test4b>", JS_EVAL_TYPE_GLOBAL);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4b returned, exception: %d", JS_IsException(test4b)); fflush(stdout); fflush(stderr);
    JS_FreeValue(context_, test4b);
    
    // Test 4c: Object literal
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4c: Object literal..."); fflush(stdout); fflush(stderr);
    const char* test4c_code = "({a:1})";
    JSValue test4c = JS_Eval(context_, test4c_code, strlen(test4c_code), "<test4c>", JS_EVAL_TYPE_GLOBAL);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4c returned, exception: %d", JS_IsException(test4c)); fflush(stdout); fflush(stderr);
    JS_FreeValue(context_, test4c);
    
    // Test 4d: Array literal
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4d: Array literal..."); fflush(stdout); fflush(stderr);
    const char* test4d_code = "[1,2,3]";
    JSValue test4d = JS_Eval(context_, test4d_code, strlen(test4d_code), "<test4d>", JS_EVAL_TYPE_GLOBAL);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4d returned, exception: %d", JS_IsException(test4d)); fflush(stdout); fflush(stderr);
    JS_FreeValue(context_, test4d);
    
    // Test 4e: var declaration (global scope, no lexical env needed)
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4e: var y = 1..."); fflush(stdout); fflush(stderr);
    const char* test4e_code = "var y = 1";
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4e: about to compile..."); fflush(stdout); fflush(stderr);
    JSValue test4e_compiled = JS_Eval(context_, test4e_code, strlen(test4e_code), "<test4e>", JS_EVAL_FLAG_COMPILE_ONLY);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4e: compile returned, exception: %d", JS_IsException(test4e_compiled)); fflush(stdout); fflush(stderr);
    if (!JS_IsException(test4e_compiled)) {
        DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4e: about to execute..."); fflush(stdout); fflush(stderr);
        JSValue test4e = JS_EvalFunction(context_, test4e_compiled);
        DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4e returned, exception: %d", JS_IsException(test4e)); fflush(stdout); fflush(stderr);
        JS_FreeValue(context_, test4e);
    } else {
        JS_FreeValue(context_, test4e_compiled);
    }
    
    // Test 4f: let declaration (needs lexical env)
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4f: let x = 1..."); fflush(stdout); fflush(stderr);
    const char* test4f_code = "let x = 1";
    JSValue test4f = JS_Eval(context_, test4f_code, strlen(test4f_code), "<test4f>", JS_EVAL_TYPE_GLOBAL);
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4f returned, exception: %d", JS_IsException(test4f)); fflush(stdout); fflush(stderr);
    JS_FreeValue(context_, test4f);
    
    DONG_LOG_DEBUG("[ScriptEngine::eval] Test 4 done"); fflush(stdout); fflush(stderr);
    
    // Skip Test 5 for now - go directly to actual code
    DONG_LOG_DEBUG("[ScriptEngine::eval] Going to actual eval..."); fflush(stdout); fflush(stderr);
    
    DONG_LOG_DEBUG("[ScriptEngine::eval] Calling JS_Eval with actual code..."); fflush(stdout); fflush(stderr);
    JSValue result = JS_Eval(context_, code.c_str(), code.length(), "<eval>", JS_EVAL_TYPE_GLOBAL);
    DONG_LOG_DEBUG("[ScriptEngine::eval] JS_Eval returned"); fflush(stdout); fflush(stderr);

    if (JS_IsException(result)) {
        // 鑾峰彇寮傚父淇℃伅
        JSValue exception = JS_GetException(context_);
        const char* error_str = JS_ToCString(context_, exception);
        if (error_str) {
            std::fprintf(stderr, "[ScriptEngine] JS exception: %s\n", error_str);
            JS_FreeCString(context_, error_str);
        }
        JS_FreeValue(context_, exception);

        // 璋冭瘯 console 缁戝畾鐘舵€?
        JSValue global = JS_GetGlobalObject(context_);
        JSValue console_val = JS_GetPropertyStr(context_, global, "console");
        const char* console_type = JS_IsUndefined(console_val) ? "undefined" :
                                   JS_IsNull(console_val) ? "null" :
                                   JS_IsObject(console_val) ? "object" : "other";

        JSValue log_val = JS_UNDEFINED;
        const char* log_type = "missing";
        if (!JS_IsUndefined(console_val) && !JS_IsNull(console_val)) {
            log_val = JS_GetPropertyStr(context_, console_val, "log");
            if (JS_IsUndefined(log_val)) {
                log_type = "undefined";
            } else if (JS_IsFunction(context_, log_val)) {
                log_type = "function";
            } else {
                log_type = "non-function";
            }
        }

        std::fprintf(stderr, "[ScriptEngine] debug: console=%s, log=%s\n", console_type, log_type);

        if (!JS_IsUndefined(log_val)) {
            JS_FreeValue(context_, log_val);
        }
        JS_FreeValue(context_, console_val);
        JS_FreeValue(context_, global);

        return false;
    }

    JS_FreeValue(context_, result);
    return true;
}

// 銆愮己鍙?銆戞墽琛屼唬鐮佸苟杩斿洖瀛楃涓插寲鐨勭粨鏋?
std::string ScriptEngine::evalWithReturn(const std::string& code) {
    if (!context_) return "";

    JSValue result = JS_Eval(context_, code.c_str(), code.length(), "<eval>", 0);

    if (JS_IsException(result)) {
        // 鑾峰彇寮傚父淇℃伅
        JSValue exception = JS_GetException(context_);
        const char* error_str = JS_ToCString(context_, exception);
        std::string error_msg = error_str ? error_str : "Unknown error";
        if (error_str) JS_FreeCString(context_, error_str);
        JS_FreeValue(context_, exception);
        
        std::fprintf(stderr, "[ScriptEngine] JS exception: %s\n", error_msg.c_str());
        return "";
    }

    // 灏嗙粨鏋滆浆鎹负瀛楃涓?
    std::string return_value;
    
    if (JS_IsString(result)) {
        const char* str = JS_ToCString(context_, result);
        if (str) {
            return_value = str;
            JS_FreeCString(context_, str);
        }
    } else if (JS_IsNumber(result)) {
        double num = 0;
        JS_ToFloat64(context_, &num, result);
        // 杞崲涓哄瓧绗︿覆
        char buf[64];
        snprintf(buf, sizeof(buf), "%.17g", num);
        return_value = buf;
    } else if (JS_IsBool(result)) {
        return_value = JS_ToBool(context_, result) ? "true" : "false";
    } else if (JS_IsNull(result)) {
        return_value = "null";
    } else if (JS_IsUndefined(result)) {
        return_value = "undefined";
    } else if (JS_IsObject(result)) {
        // 瀵硅薄杞瓧绗︿覆 - 璋冪敤 JSON.stringify
        JSValue global = JS_GetGlobalObject(context_);
        JSValue json = JS_GetPropertyStr(context_, global, "JSON");
        if (!JS_IsUndefined(json) && !JS_IsNull(json)) {
            JSValue stringify = JS_GetPropertyStr(context_, json, "stringify");
            if (JS_IsFunction(context_, stringify)) {
                JSValue str_result = JS_Call(context_, stringify, json, 1, &result);
                if (!JS_IsException(str_result)) {
                    const char* str = JS_ToCString(context_, str_result);
                    if (str) {
                        return_value = str;
                        JS_FreeCString(context_, str);
                    }
                }
                JS_FreeValue(context_, str_result);
            }
            JS_FreeValue(context_, stringify);
        }
        JS_FreeValue(context_, json);
        JS_FreeValue(context_, global);
    }

    JS_FreeValue(context_, result);
    return return_value;
}

JSValue* ScriptEngine::callFunction(const std::string& function_name, int argc, JSValue* argv) {
    if (!context_) return nullptr;

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
        JSValue exception = JS_GetException(context_);
        JS_FreeValue(context_, exception);
        return nullptr;
    }

    // 杩斿洖缁撴灉锛堣皟鐢ㄨ€呴渶瑕侀噴鏀撅級
    JSValue* ret = new JSValue(result);
    return ret;
}

void ScriptEngine::bindGlobalObject(const std::string& name, void* object) {
    if (!context_) return;

    JSValue global = JS_GetGlobalObject(context_);
    JSValue obj = JS_NewObject(context_);

    // 灏嗗璞℃寚閽堝瓨鍌ㄤ负 opaque 鏁版嵁
    JS_SetOpaque(obj, object);

    JS_SetPropertyStr(context_, global, name.c_str(), obj);
    JS_FreeValue(context_, global);
}

void ScriptEngine::bindGlobalFunction(const std::string& name, void* func) {
    // TODO: 浣跨敤 JS_NewCFunction 鍖呰 C 鍑芥暟
    (void)name;
    (void)func;
}

void ScriptEngine::processPendingTasks() {
    // TODO: 杩愯寰换鍔￠槦鍒楋紙Promise锛?
    // 鍙€夛細瀹氭湡璋冪敤 js_std_loop() 鎴栫被浼肩殑浠诲姟澶勭悊鍑芥暟
}

void ScriptEngine::initializeBuiltins() {
    if (!context_) return;

    // 鍒濆鍖?console 瀵硅薄锛堝彲閫夛級
    // TODO: 涓?console.log 绛夋彁渚涘疄鐜?

    // 鍏朵粬鍐呯疆瀵硅薄鍒濆鍖栧湪 DOM 缁戝畾灞傝繘琛?
}

} // namespace dong::script
