#include "registry.h"

#include "porffor_script_registry.hpp"
#include "dong_porf_host.hpp"
#include "../../core/log.h"

#include <cstring>
#include <utility>

namespace dong::script {
namespace {

struct LoadedFrame {
    PorfforScriptRegistry* registry = nullptr;
    std::string module_name;
    const dong_porf_module* mod = nullptr;
};

thread_local std::vector<LoadedFrame> g_instance_stack;
thread_local LoadedFrame g_active_instance{};

void pushInstanceActivation(PorfforScriptRegistry* registry, const std::string& module_name,
                            const dong_porf_module* mod) {
    g_instance_stack.push_back(g_active_instance);
    if (g_active_instance.registry &&
        (g_active_instance.registry != registry ||
         g_active_instance.module_name != module_name)) {
        g_active_instance.registry->captureModuleInstance(g_active_instance.module_name,
                                                        g_active_instance.mod);
    }
    registry->applyModuleInstance(module_name, mod);
    g_active_instance = {registry, module_name, mod};
}

void popInstanceActivation(PorfforScriptRegistry* registry, const std::string& module_name,
                           const dong_porf_module* mod) {
    registry->captureModuleInstance(module_name, mod);

    if (g_instance_stack.empty()) {
        g_active_instance = {};
        return;
    }

    LoadedFrame previous = g_instance_stack.back();
    g_instance_stack.pop_back();
    g_active_instance = previous;
    if (previous.registry && previous.mod) {
        previous.registry->applyModuleInstance(previous.module_name, previous.mod);
    }
}

class ActiveHostScope {
public:
    explicit ActiveHostScope(PorfforHost* host) : previous_(PorfforHost_active()) {
        PorfforHost_setActiveHost(host);
    }
    ~ActiveHostScope() { PorfforHost_setActiveHost(previous_); }

private:
    PorfforHost* previous_;
};

class ModuleInstanceScope {
public:
    ModuleInstanceScope(PorfforScriptRegistry* registry, const std::string& module_name,
                        const dong_porf_module* mod)
        : registry_(registry), module_name_(module_name), mod_(mod) {
        pushInstanceActivation(registry_, module_name_, mod_);
    }
    ~ModuleInstanceScope() { popInstanceActivation(registry_, module_name_, mod_); }

private:
    PorfforScriptRegistry* registry_;
    std::string module_name_;
    const dong_porf_module* mod_;
};

} // namespace

PorfforScriptRegistry::PorfforScriptRegistry(PorfforHost* host) : host_(host) {}

PorfforScriptRegistry::ModuleInstance& PorfforScriptRegistry::instanceFor(
    const std::string& module_name) {
    return instances_[module_name];
}

void PorfforScriptRegistry::ensureInstanceInitialized(const std::string& module_name,
                                                      const dong_porf_module* mod) {
    if (!mod || !mod->memory || !mod->memory_pages) {
        return;
    }

    ModuleInstance& inst = instanceFor(module_name);
    if (inst.initialized) {
        return;
    }

    inst.memory_pages = *mod->memory_pages;
    if (inst.memory_pages == 0) {
        inst.memory_pages = 2;
    }
    inst.memory.assign(static_cast<size_t>(inst.memory_pages) * 65536u, 0);

    char* previous_memory = *mod->memory;
    *mod->memory = nullptr;
    if (mod->init_fn) {
        mod->init_fn();
    }
    if (mod->memory && *mod->memory) {
        const size_t bytes = static_cast<size_t>(inst.memory_pages) * 65536u;
        std::memcpy(inst.memory.data(), *mod->memory, bytes);
        std::free(*mod->memory);
    }
    *mod->memory = inst.memory.data();

    if (mod->state_capture && mod->state_size > 0) {
        inst.state_snapshot.assign(mod->state_size, 0);
        mod->state_capture(inst.state_snapshot.data());
    }
    inst.initialized = true;
}

void PorfforScriptRegistry::captureModuleInstance(const std::string& module_name,
                                                const dong_porf_module* mod) {
    if (!mod || !mod->memory || !mod->memory_pages) {
        return;
    }

    auto it = instances_.find(module_name);
    if (it == instances_.end() || !it->second.initialized) {
        return;
    }

    ModuleInstance& inst = it->second;
    if (mod->state_capture && mod->state_size > 0) {
        if (inst.state_snapshot.size() != mod->state_size) {
            inst.state_snapshot.assign(mod->state_size, 0);
        }
        mod->state_capture(inst.state_snapshot.data());
    }
}

void PorfforScriptRegistry::applyModuleInstance(const std::string& module_name,
                                                const dong_porf_module* mod) {
    if (!mod || !mod->memory || !mod->memory_pages) {
        return;
    }

    ensureInstanceInitialized(module_name, mod);
    ModuleInstance& inst = instanceFor(module_name);
    *mod->memory = inst.memory.data();
    *mod->memory_pages = inst.memory_pages;
    if (host_) {
        host_->setActiveMemory(*mod->memory, mod->memory_pages);
    }
    if (mod->state_apply && mod->state_size > 0 && !inst.state_snapshot.empty()) {
        mod->state_apply(inst.state_snapshot.data());
    }
}

bool PorfforScriptRegistry::activateModuleInstance(const std::string& module_name,
                                                 const dong_porf_module* mod) {
    if (!mod || !mod->memory || !mod->memory_pages) {
        return false;
    }
    applyModuleInstance(module_name, mod);
    return true;
}

void PorfforScriptRegistry::deactivateModuleInstance(const std::string& module_name,
                                                     const dong_porf_module* mod) {
    captureModuleInstance(module_name, mod);
}

bool PorfforScriptRegistry::run(const std::string& module_name) {
    const dong_porf_module_t* mod = dong_porf_find_module(module_name.c_str());
    if (!mod || !mod->main_fn) {
        DONG_LOG_ERROR("[PorfforRegistry] module not found: %s", module_name.c_str());
        return false;
    }

    ActiveHostScope host_scope(host_);
    ModuleInstanceScope instance_scope(this, module_name, mod);
    active_module_ = module_name;
    PorfforHost_setActiveModule(mod);
    if (!activateModuleInstance(module_name, mod)) {
        return false;
    }

    const int rc = mod->main_fn();
    DONG_LOG_INFO("[PorfforRegistry] ran module %s rc=%d", module_name.c_str(), rc);
    return rc == 0;
}

bool PorfforScriptRegistry::callExport(const std::string& module_name,
                                       const std::string& export_name, const double* args,
                                       int arg_count) {
    const std::string prev_active = active_module_;
    const dong_porf_module_t* prev_mod = PorfforHost_activeModule();

    const dong_porf_handler_t* handler =
        dong_porf_find_handler(module_name.c_str(), export_name.c_str());
    if (!handler) {
        DONG_LOG_WARN("[PorfforRegistry] handler not found: %s::%s", module_name.c_str(),
                      export_name.c_str());
        return false;
    }

    if (handler->param_count != arg_count) {
        DONG_LOG_WARN("[PorfforRegistry] handler %s::%s expected %d args, got %d",
                      module_name.c_str(), export_name.c_str(), handler->param_count, arg_count);
        return false;
    }

    const dong_porf_module_t* template_mod = dong_porf_find_module(module_name.c_str());
    const std::string instance_key =
        handler->legacy_handler_module ? handler->legacy_handler_module : module_name;

    const dong_porf_module_t shim = {
        instance_key.c_str(),
        nullptr,
        handler->memory,
        handler->memory_pages,
        template_mod ? template_mod->init_fn : nullptr,
        template_mod ? template_mod->state_capture : nullptr,
        template_mod ? template_mod->state_apply : nullptr,
        template_mod ? template_mod->state_size : 0,
    };

    ActiveHostScope host_scope(host_);
    ModuleInstanceScope instance_scope(this, instance_key, &shim);
    active_module_ = module_name;
    PorfforHost_setActiveModule(&shim);
    if (!activateModuleInstance(instance_key, &shim)) {
        active_module_ = prev_active;
        if (prev_mod) {
            PorfforHost_setActiveModule(prev_mod);
        }
        return false;
    }

    if (host_) {
        host_->pushResultSlot();
    }

    int rc = -1;
    if (arg_count == 0) {
        if (!handler->fn0) {
            DONG_LOG_WARN("[PorfforRegistry] handler %s::%s missing fn0", module_name.c_str(),
                          export_name.c_str());
            if (host_) {
                host_->popResultSlot();
            }
            active_module_ = prev_active;
            if (prev_mod) {
                PorfforHost_setActiveModule(prev_mod);
            }
            return false;
        }
        rc = handler->fn0();
    } else if (arg_count == 1) {
        if (!handler->fn1 || !args) {
            DONG_LOG_WARN("[PorfforRegistry] handler %s::%s missing fn1", module_name.c_str(),
                          export_name.c_str());
            if (host_) {
                host_->popResultSlot();
            }
            active_module_ = prev_active;
            if (prev_mod) {
                PorfforHost_setActiveModule(prev_mod);
            }
            return false;
        }
        rc = handler->fn1(args[0]);
    } else {
        DONG_LOG_WARN("[PorfforRegistry] handler %s::%s unsupported arg_count %d",
                      module_name.c_str(), export_name.c_str(), arg_count);
        if (host_) {
            host_->popResultSlot();
        }
        active_module_ = prev_active;
        if (prev_mod) {
            PorfforHost_setActiveModule(prev_mod);
        }
        return false;
    }

    if (host_) {
        host_->popResultSlot();
    }

    active_module_ = prev_active;
    if (prev_mod) {
        PorfforHost_setActiveModule(prev_mod);
    } else if (!prev_active.empty()) {
        const dong_porf_module_t* restore = dong_porf_find_module(prev_active.c_str());
        if (restore) {
            PorfforHost_setActiveModule(restore);
        }
    }

    DONG_LOG_DEBUG("[PorfforRegistry] callExport %s::%s rc=%d", module_name.c_str(),
                   export_name.c_str(), rc);
    return rc == 0;
}

} // namespace dong::script
