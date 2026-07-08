#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace dong::dom {
class Manager;
class EventDispatcher;
class FocusManager;
class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;
} // namespace dong::dom

namespace dong::layout {
class Engine;
}

namespace dong::render {
class OverlayDraw;
}

namespace dong::script {

class PorfforHost;
class PorfforScriptRegistry;

// Porffor-backed script engine (AOT modules, no runtime eval).
class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    bool eval(const std::string& code);
    std::string evalWithReturn(const std::string& code);
    bool evalModule(const std::string& module_path, const std::string& code);

    void processPendingTasks();

    bool runModule(const std::string& module_name);
    bool callExport(const std::string& module_name, const std::string& export_name);
    bool callExport1(const std::string& module_name, const std::string& export_name, double arg0);
    void setDefaultModule(const std::string& module_name) { default_module_ = module_name; }
    const std::string& defaultModule() const { return default_module_; }

    PorfforScriptRegistry* registry() { return registry_.get(); }
    PorfforHost* host() { return host_.get(); }

private:
    std::unique_ptr<PorfforHost> host_;
    std::unique_ptr<PorfforScriptRegistry> registry_;
    std::string default_module_;
};

using ScriptEnginePtr = std::unique_ptr<ScriptEngine>;

} // namespace dong::script
