#pragma once

#ifdef DONG_SCRIPT_ENGINE_PORFFOR
#include "porffor/script_engine_porffor.hpp"
#else

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include "../dom/dom/dom_node.hpp"
#include "../dom/event_system.hpp"

#include "quickjs_compat.h"
#include "module_loader.hpp"

namespace dong::script {

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    bool eval(const std::string& code);
    std::string evalWithReturn(const std::string& code);
    JSValue* callFunction(const std::string& function_name, int argc = 0, JSValue* argv = nullptr);

    void bindGlobalObject(const std::string& name, void* object);
    void bindGlobalFunction(const std::string& name, JSCFunction* func, int argc = 0);

    JSContext* getContext() const { return context_; }
    JSRuntime* getRuntime() const { return runtime_; }

    void processPendingTasks();
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

#endif // DONG_SCRIPT_ENGINE_PORFFOR
