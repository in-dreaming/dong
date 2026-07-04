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
                                       const std::string& export_name) {
    const dong_porf_handler_t* handler =
        dong_porf_find_handler(module_name.c_str(), export_name.c_str());
    if (!handler || !handler->main_fn) {
        DONG_LOG_WARN("[PorfforRegistry] handler not found: %s::%s", module_name.c_str(),
                      export_name.c_str());
        return false;
    }

    const dong_porf_module_t shim = {
        handler->handler_module,
        handler->main_fn,
        handler->memory,
        handler->memory_pages,
    };
    PorfforHost_setActiveModule(&shim);
    if (!ensureModuleMemory(&shim)) {
        return false;
    }

    const int rc = handler->main_fn();
    DONG_LOG_DEBUG("[PorfforRegistry] callExport %s::%s rc=%d", module_name.c_str(),
                   export_name.c_str(), rc);
    return rc == 0;
}

} // namespace dong::script
