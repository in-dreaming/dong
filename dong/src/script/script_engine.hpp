#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include "../dom/dom/dom_node.hpp"
#include "../dom/event_system.hpp"



#include "quickjs_compat.h"
#include "module_loader.hpp"




namespace dong::script {

// JavaScript 脚本引擎（QuickJS 包装）
class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    // 执行 JavaScript 代码
    bool eval(const std::string& code);

    // 【缺口3】执行 JavaScript 代码并返回结果
    std::string evalWithReturn(const std::string& code);

    // 调用 JavaScript 函数
    JSValue* callFunction(const std::string& function_name, int argc = 0, JSValue* argv = nullptr);

    // 绑定 C++ 对象到 JavaScript 上下文
    void bindGlobalObject(const std::string& name, void* object);
    void bindGlobalFunction(const std::string& name, JSCFunction* func, int argc = 0);

    // 获取原生 QuickJS 对象（用于高级集成）
    JSContext* getContext() const { return context_; }
    JSRuntime* getRuntime() const { return runtime_; }

    // 处理待定的 JS 任务
    void processPendingTasks();

    // 【Phase 3】执行 ES 模块脚本
    // 加载并执行 ES 模块代码，使用 JS_EVAL_TYPE_MODULE 标志
    // 自动处理待定的 job（模块初始化）
    bool evalModule(const std::string& module_path, const std::string& code);

private:
    static int interruptHandler(JSRuntime* rt, void* opaque);

    JSRuntime* runtime_;
    JSContext* context_;
    uint64_t interrupt_deadline_ns_ = 0;

    std::unique_ptr<ModuleLoader> module_loader_;

    void initializeBuiltins();
};

using ScriptEnginePtr = std::unique_ptr<ScriptEngine>;

} // namespace dong::script
