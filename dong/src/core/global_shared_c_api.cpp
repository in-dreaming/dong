/**
 * GlobalShared C API - C wrapper for GlobalShared C++ class
 * 
 * Provides C ABI for Scene3D and other C modules to access GlobalShared
 */

#include "global_shared.hpp"
#include "dong_gpu_driver.h"
#include "log.h"

// C API 实现
extern "C" {

/**
 * Initialize GlobalShared with the given GPU driver
 * Must be called before creating any engines in multi-view scenarios
 */
int dong_global_shared_initialize(DongGPUDriver* driver) {
    if (!driver) {
        DONG_LOG_ERROR("dong_global_shared_initialize: driver is NULL");
        return 0;
    }
    
    bool result = dong::GlobalShared::initialize(driver);
    return result ? 1 : 0;
}

/**
 * Check if GlobalShared is initialized
 */
int dong_global_shared_is_initialized(void) {
    return dong::GlobalShared::isInitialized() ? 1 : 0;
}

/**
 * Add a reference to GlobalShared (call when creating a new View/Engine)
 */
void dong_global_shared_add_ref(void) {
    auto* instance = dong::GlobalShared::instance();
    if (instance) {
        instance->addRef();
    }
}

/**
 * Release a reference to GlobalShared (call when destroying a View/Engine)
 */
void dong_global_shared_release(void) {
    auto* instance = dong::GlobalShared::instance();
    if (instance) {
        instance->release();
    }
}

/**
 * Get current reference count
 */
int dong_global_shared_get_ref_count(void) {
    auto* instance = dong::GlobalShared::instance();
    if (instance) {
        return instance->refCount();
    }
    return 0;
}

/**
 * Shutdown GlobalShared (call at application exit)
 */
void dong_global_shared_shutdown(void) {
    dong::GlobalShared::shutdown();
}

/**
 * Log current memory stats
 */
void dong_global_shared_log_stats(void) {
    auto* instance = dong::GlobalShared::instance();
    if (!instance) {
        DONG_LOG_INFO("GlobalShared: not initialized");
        return;
    }
    
    auto stats = instance->getStats();
    float mb = stats.total_memory_bytes / (1024.0f * 1024.0f);
    
    DONG_LOG_INFO("GlobalShared stats: ref_count=%d, glyph_tiers=%d, memory=%.2f MB",
                  stats.view_ref_count,
                  stats.glyph_atlas_tier_count,
                  mb);
}

} // extern "C"
