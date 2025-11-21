#pragma once

#include <memory>
#include <string>
#include <functional>
#include "../dom/dom_node.hpp"
#include "../dom/event_system.hpp"

// Forward declarations - will be included in cpp file
extern "C" {
    struct JSRuntime;
    struct JSContext;
}

// JSValue is a scalar type in QuickJS (int64_t), not a struct
using JSValue = int64_t;



namespace dong::script {

// JavaScript 脚本引擎（QuickJS 包装）
class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    // 执行 JavaScript 代码
    bool eval(const std::string& code);

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
