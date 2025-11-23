#include "script_engine.hpp"
#include <cstring>
#include <cstdio>

namespace dong::script {

// 简单的 console.log 实现，直接打印到 stderr
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
    // 创建 QuickJS 运行时
    runtime_ = JS_NewRuntime();
    if (!runtime_) return;

    // 创建 JavaScript 上下文
    context_ = JS_NewContext(runtime_);
    if (!context_) {
        JS_FreeRuntime(runtime_);
        runtime_ = nullptr;
        return;
    }

    // 初始化内置绑定
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
    if (!context_) return false;

    JSValue result = JS_Eval(context_, code.c_str(), code.length(), "<eval>", 0);

    if (JS_IsException(result)) {
        // 获取异常信息
        JSValue exception = JS_GetException(context_);
        const char* error_str = JS_ToCString(context_, exception);
        if (error_str) {
            std::fprintf(stderr, "[ScriptEngine] JS exception: %s\n", error_str);
            JS_FreeCString(context_, error_str);
        }
        JS_FreeValue(context_, exception);

        // 调试 console 绑定状态
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

// 【缺口3】执行代码并返回字符串化的结果
std::string ScriptEngine::evalWithReturn(const std::string& code) {
    if (!context_) return "";

    JSValue result = JS_Eval(context_, code.c_str(), code.length(), "<eval>", 0);

    if (JS_IsException(result)) {
        // 获取异常信息
        JSValue exception = JS_GetException(context_);
        const char* error_str = JS_ToCString(context_, exception);
        std::string error_msg = error_str ? error_str : "Unknown error";
        if (error_str) JS_FreeCString(context_, error_str);
        JS_FreeValue(context_, exception);
        
        std::fprintf(stderr, "[ScriptEngine] JS exception: %s\n", error_msg.c_str());
        return "";
    }

    // 将结果转换为字符串
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
        // 转换为字符串
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
        // 对象转字符串 - 调用 JSON.stringify
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

    // 返回结果（调用者需要释放）
    JSValue* ret = new JSValue(result);
    return ret;
}

void ScriptEngine::bindGlobalObject(const std::string& name, void* object) {
    if (!context_) return;

    JSValue global = JS_GetGlobalObject(context_);
    JSValue obj = JS_NewObject(context_);

    // 将对象指针存储为 opaque 数据
    JS_SetOpaque(obj, object);

    JS_SetPropertyStr(context_, global, name.c_str(), obj);
    JS_FreeValue(context_, global);
}

void ScriptEngine::bindGlobalFunction(const std::string& name, void* func) {
    // TODO: 使用 JS_NewCFunction 包装 C 函数
    (void)name;
    (void)func;
}

void ScriptEngine::processPendingTasks() {
    // TODO: 运行微任务队列（Promise）
    // 可选：定期调用 js_std_loop() 或类似的任务处理函数
}

void ScriptEngine::initializeBuiltins() {
    if (!context_) return;

    // 初始化 console 对象（可选）
    // TODO: 为 console.log 等提供实现

    // 其他内置对象初始化在 DOM 绑定层进行
}

} // namespace dong::script
