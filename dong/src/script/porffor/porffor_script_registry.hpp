#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct dong_porf_module;

namespace dong::script {

class PorfforHost;

class PorfforScriptRegistry {
public:
    explicit PorfforScriptRegistry(PorfforHost* host);

    bool run(const std::string& module_name);
    bool callExport(const std::string& module_name, const std::string& export_name,
                    const double* args = nullptr, int arg_count = 0);

    const std::string& activeModule() const { return active_module_; }

    void captureModuleInstance(const std::string& module_name, const dong_porf_module* mod);
    void applyModuleInstance(const std::string& module_name, const dong_porf_module* mod);

private:
    struct ModuleInstance {
        std::vector<char> memory;
        unsigned int memory_pages = 0;
        std::vector<uint8_t> state_snapshot;
        bool initialized = false;
    };

    ModuleInstance& instanceFor(const std::string& module_name);
    void ensureInstanceInitialized(const std::string& module_name, const dong_porf_module* mod);
    bool activateModuleInstance(const std::string& module_name, const dong_porf_module* mod);
    void deactivateModuleInstance(const std::string& module_name, const dong_porf_module* mod);

    PorfforHost* host_ = nullptr;
    std::string active_module_;
    std::unordered_map<std::string, ModuleInstance> instances_;
};

} // namespace dong::script
