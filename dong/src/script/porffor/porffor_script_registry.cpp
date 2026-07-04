#include "registry.h"

#include "porffor_script_registry.hpp"
#include "dong_porf_host.hpp"
#include "../../core/log.h"

#include <cstring>

namespace dong::script {

PorfforScriptRegistry::PorfforScriptRegistry(PorfforHost* host) : host_(host) {}

static bool ensureModuleMemory(const dong_porf_module_t* mod) {
    if (!mod || !mod->memory || !mod->memory_pages) {
        return false;
    }
    if (!*mod->memory) {
        *mod->memory =
            static_cast<char*>(std::calloc(1, static_cast<size_t>(*mod->memory_pages) * 65536));
    }
    return *mod->memory != nullptr;
}

bool PorfforScriptRegistry::run(const std::string& module_name) {
    const dong_porf_module_t* mod = dong_porf_find_module(module_name.c_str());
    if (!mod || !mod->main_fn) {
        DONG_LOG_ERROR("[PorfforRegistry] module not found: %s", module_name.c_str());
        return false;
    }

    active_module_ = module_name;
    PorfforHost_setActiveModule(mod);
    if (!ensureModuleMemory(mod)) {
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

    const dong_porf_module_t shim = {
        handler->legacy_handler_module ? handler->legacy_handler_module : module_name.c_str(),
        nullptr,
        handler->memory,
        handler->memory_pages,
    };
    active_module_ = module_name;
    PorfforHost_setActiveModule(&shim);
    if (!ensureModuleMemory(&shim)) {
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
