#pragma once

#include <functional>
#include <memory>
#include <string>
#include "../dom/dom_node.hpp"
#include "../dom/event_system.hpp"



extern "C" {
#include "quickjs.h"
}



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
    void bindGlobalFunction(const std::string& name, void* func);

    // 获取原生 QuickJS 对象（用于高级集成）
    JSContext* getContext() const { return context_; }
    JSRuntime* getRuntime() const { return runtime_; }

    // 处理待定的 JS 任务
    void processPendingTasks();

private:
    JSRuntime* runtime_;
    JSContext* context_;

    void initializeBuiltins();
};

using ScriptEnginePtr = std::unique_ptr<ScriptEngine>;

} // namespace dong::script
