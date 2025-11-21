#include "script_engine.hpp"
#include <cstring>

// QuickJS forward declarations (will be linked at build time)
extern "C" {
typedef struct JSRuntime JSRuntime;
typedef struct JSContext JSContext;
typedef int64_t JSValue;  // QuickJS uses tagged union
typedef JSValue JSValueConst;

JSRuntime* JS_NewRuntime(void);
void JS_FreeRuntime(JSRuntime* rt);
JSContext* JS_NewContext(JSRuntime* rt);
void JS_FreeContext(JSContext* ctx);
JSValue JS_Eval(JSContext* ctx, const char* input, size_t input_len, const char* filename, int flags);
int JS_IsException(JSValue v);
JSValue JS_GetException(JSContext* ctx);
const char* JS_ToCString(JSContext* ctx, JSValue val);
void JS_FreeCString(JSContext* ctx, const char* str);
void JS_FreeValue(JSContext* ctx, JSValue v);
JSValue JS_GetGlobalObject(JSContext* ctx);
JSValue JS_GetPropertyStr(JSContext* ctx, JSValue obj, const char* prop);
int JS_IsFunction(JSContext* ctx, JSValue v);
JSValue JS_Call(JSContext* ctx, JSValue func, JSValue this_obj, int argc, JSValueConst* argv);
JSValue JS_NewObject(JSContext* ctx);
void JS_SetOpaque(JSValue obj, void* opaque);
JSValue JS_NewCFunction(JSContext* ctx, JSValue (*func)(JSContext*, JSValueConst, int, JSValueConst*), const char* name, int length);
int JS_SetPropertyStr(JSContext* ctx, JSValue obj, const char* prop, JSValue val);
JSValue JS_NewArray(JSContext* ctx);
#define JS_UNDEFINED (JSValue)0
}

namespace dong::script {

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
    if (context_) {
        JS_FreeContext(context_);
    }
    if (runtime_) {
        JS_FreeRuntime(runtime_);
    }
}

bool ScriptEngine::eval(const std::string& code) {
    if (!context_) return false;

    JSValue result = JS_Eval(context_, code.c_str(), code.length(), "<eval>", 0);

    if (JS_IsException(result)) {
        // 获取异常信息
        JSValue exception = JS_GetException(context_);
        const char* error_str = JS_ToCString(context_, exception);
        if (error_str) {
            // TODO: 记录错误
            JS_FreeCString(context_, error_str);
        }
        JS_FreeValue(context_, exception);
        return false;
    }

    JS_FreeValue(context_, result);
    return true;
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
