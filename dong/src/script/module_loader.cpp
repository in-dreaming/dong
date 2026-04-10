#include "module_loader.hpp"
#include "../core/log.h"
#include <filesystem>

namespace dong::script {

ModuleLoader::ModuleLoader(JSContext* ctx, JSRuntime* rt)
    : context_(ctx), runtime_(rt) {
    DONG_LOG_DEBUG("[ModuleLoader] Created (ctx=%p, rt=%p)", ctx, rt);
}

ModuleLoader::~ModuleLoader() {
    DONG_LOG_DEBUG("[ModuleLoader] Destroyed");
}

bool ModuleLoader::loadModule(const std::string& module_path, const std::string& code) {
    if (!context_) {
        DONG_LOG_ERROR("[ModuleLoader] Invalid context (null)");
        return false;
    }

    if (code.empty()) {
        DONG_LOG_ERROR("[ModuleLoader] Empty code for module: %s", module_path.c_str());
        return false;
    }

    DONG_LOG_INFO("[ModuleLoader] Loading module: %s (%zu bytes)", module_path.c_str(), code.length());

    // Evaluate code as an ES module using JS_EVAL_TYPE_MODULE flag.
    // This tells QuickJS to:
    // 1. Parse as module syntax (top-level imports/exports allowed)
    // 2. Queue module initialization as a pending job
    // 3. Return a module object (not execute immediately)
    JSValue result = JS_Eval(context_, code.c_str(), code.length(),
                             module_path.c_str(), JS_EVAL_TYPE_MODULE);

    if (JS_IsException(result)) {
        DONG_LOG_ERROR("[ModuleLoader] Module evaluation error: %s", module_path.c_str());

        // Dump exception details for debugging
        JSValue exc = JS_GetException(context_);
        const char* msg = JS_ToCString(context_, exc);
        if (msg) {
            DONG_LOG_ERROR("[ModuleLoader] Exception: %s", msg);
            JS_FreeCString(context_, msg);
        }
        JS_FreeValue(context_, exc);
        JS_FreeValue(context_, result);

        return false;
    }

    DONG_LOG_INFO("[ModuleLoader] Module evaluated successfully: %s", module_path.c_str());
    JS_FreeValue(context_, result);

    // Process pending jobs (module initialization, top-level awaits, etc.)
    // The module code is executed as pending jobs, not immediately.
    DONG_LOG_DEBUG("[ModuleLoader] Processing pending jobs for module: %s", module_path.c_str());

    while (JS_IsJobPending(runtime_)) {
        // Execute one pending job
        // Returns: 1 if job executed, 0 if no more jobs, -1 if error
        int ret = JS_ExecutePendingJob(runtime_, &context_);

        if (ret < 0) {
            DONG_LOG_ERROR("[ModuleLoader] Error executing pending job for module: %s", module_path.c_str());

            // Dump exception
            JSValue exc = JS_GetException(context_);
            const char* msg = JS_ToCString(context_, exc);
            if (msg) {
                DONG_LOG_ERROR("[ModuleLoader] Job exception: %s", msg);
                JS_FreeCString(context_, msg);
            }
            JS_FreeValue(context_, exc);

            return false;
        }

        if (ret == 0) {
            // No more jobs
            DONG_LOG_DEBUG("[ModuleLoader] All pending jobs completed for module: %s", module_path.c_str());
            break;
        }
    }

    DONG_LOG_INFO("[ModuleLoader] Module execution completed: %s", module_path.c_str());
    return true;
}

JSValue ModuleLoader::getModuleExports(const std::string& module_path) {
    // TODO: Implement module registry to track and retrieve module exports.
    // For now, return undefined since we don't track exports.
    // Modules typically export via global objects or direct mutations (e.g., window.*)
    (void)module_path;
    return JS_UNDEFINED;
}

std::string ModuleLoader::resolveModulePath(const std::string& path) {
    // TODO: Implement module resolution logic if needed (e.g., ./ relative paths)
    // For now, return path as-is (relative to resource_root in engine_view)
    return path;
}

} // namespace dong::script
