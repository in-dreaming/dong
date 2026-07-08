#include "script_engine_porffor.hpp"

#include "dong_porf_host.hpp"
#include "porffor_script_registry.hpp"
#include "../../core/log.h"

namespace dong::script {

ScriptEngine::ScriptEngine()
    : host_(std::make_unique<PorfforHost>()),
      registry_(std::make_unique<PorfforScriptRegistry>(host_.get())) {
    host_->setRegistry(registry_.get());
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
    host_->processFetches();
    host_->processTimers(host_->timeNow());
}

bool ScriptEngine::runModule(const std::string& module_name) {
    return registry_->run(module_name);
}

bool ScriptEngine::callExport(const std::string& module_name, const std::string& export_name) {
    return registry_->callExport(module_name, export_name, nullptr, 0);
}

bool ScriptEngine::callExport1(const std::string& module_name, const std::string& export_name,
                               double arg0) {
    return registry_->callExport(module_name, export_name, &arg0, 1);
}

} // namespace dong::script
