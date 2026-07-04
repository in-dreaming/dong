#include "script_engine_porffor.hpp"

#include "dong_porf_host.hpp"
#include "porffor_script_registry.hpp"
#include "../../core/log.h"

namespace dong::script {

ScriptEngine::ScriptEngine()
    : host_(std::make_unique<PorfforHost>()),
      registry_(std::make_unique<PorfforScriptRegistry>(host_.get())) {
    host_->setRegistry(registry_.get());
    default_module_ = "hello_dom";
}

ScriptEngine::~ScriptEngine() = default;

bool ScriptEngine::eval(const std::string& code) {
    DONG_LOG_WARN("[ScriptEngine/Porffor] runtime eval unsupported (%zu bytes)", code.size());
    return false;
}

std::string ScriptEngine::evalWithReturn(const std::string& code) {
    (void)code;
    return {};
}

bool ScriptEngine::evalModule(const std::string& module_path, const std::string& code) {
    (void)module_path;
    (void)code;
    DONG_LOG_WARN("[ScriptEngine/Porffor] runtime ES modules unsupported");
    return false;
}

void ScriptEngine::processPendingTasks() {
    host_->processTimers(host_->timeNow());
}

bool ScriptEngine::runModule(const std::string& module_name) {
    return registry_->run(module_name);
}

} // namespace dong::script
