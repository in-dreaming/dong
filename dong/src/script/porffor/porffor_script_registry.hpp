#pragma once

#include <cstddef>
#include <string>

namespace dong::script {

class PorfforHost;

class PorfforScriptRegistry {
public:
    explicit PorfforScriptRegistry(PorfforHost* host);

    bool run(const std::string& module_name);
    bool callExport(const std::string& module_name, const std::string& export_name);

    const std::string& activeModule() const { return active_module_; }

private:
    PorfforHost* host_ = nullptr;
    std::string active_module_;
};

} // namespace dong::script
