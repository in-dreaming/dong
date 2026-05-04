/**
 * GlobalShared C API Header
 * 
 * Process-level resource sharing for multi-view scenarios (3D screens, etc.)
 * 
 * Usage:
 *   1. Call dong_global_shared_initialize(driver) before creating engines
 *   2. Each engine creation calls dong_global_shared_add_ref()
 *   3. Each engine destruction calls dong_global_shared_release()
 *   4. Call dong_global_shared_shutdown() at application exit
 */

#ifndef DONG_GLOBAL_SHARED_H
#define DONG_GLOBAL_SHARED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
struct DongGPUDriver;
typedef struct DongGPUDriver DongGPUDriver;

/**
 * Initialize GlobalShared with the given GPU driver
 * Must be called before creating any engines in multi-view scenarios
 * 
 * @param driver The GPU driver to use for creating shared resources
 * @return 1 on success, 0 on failure
 */
int dong_global_shared_initialize(DongGPUDriver* driver);

/**
 * Check if GlobalShared is initialized
 * @return 1 if initialized, 0 otherwise
 */
int dong_global_shared_is_initialized(void);

/**
 * Add a reference to GlobalShared (call when creating a new View/Engine)
 */
void dong_global_shared_add_ref(void);

/**
 * Release a reference to GlobalShared (call when destroying a View/Engine)
 */
void dong_global_shared_release(void);

/**
 * Get current reference count
 * @return Current reference count, or 0 if not initialized
 */
int dong_global_shared_get_ref_count(void);

/**
 * Shutdown GlobalShared (call at application exit)
 */
void dong_global_shared_shutdown(void);

/**
 * Log current memory stats
 */
void dong_global_shared_log_stats(void);

#ifdef __cplusplus
}
#endif

#endif // DONG_GLOBAL_SHARED_H
