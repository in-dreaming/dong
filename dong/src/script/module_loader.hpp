#pragma once

#include <string>
#include <memory>
#include "quickjs_compat.h"

namespace dong::script {

/**
 * ModuleLoader: Handles ES6 module script loading and execution via QuickJS.
 *
 * Responsibilities:
 * - Load module code from filesystem
 * - Evaluate modules using JS_EVAL_TYPE_MODULE
 * - Process pending jobs (module initialization, imports)
 * - Provide error handling and logging
 */
class ModuleLoader {
public:
    ModuleLoader(JSContext* ctx, JSRuntime* rt);
    ~ModuleLoader();

    /**
     * Load and execute an ES module.
     *
     * @param module_path: Path to module (used for error reporting and stack traces)
     * @param code: Module source code to evaluate
     * @return true if module loaded and executed successfully, false otherwise
     */
    bool loadModule(const std::string& module_path, const std::string& code);

    /**
     * Get module exports object (currently returns undefined).
     * TODO: Implement module registry to track and retrieve exports.
     */
    JSValue getModuleExports(const std::string& module_path);

private:
    JSContext* context_;
    JSRuntime* runtime_;

    /**
     * Resolve module path for error reporting.
     * TODO: Implement proper module resolution if needed.
     */
    std::string resolveModulePath(const std::string& path);
};

} // namespace dong::script
