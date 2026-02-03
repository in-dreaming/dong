#ifndef DONG_PLATFORM_H
#define DONG_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DLL export/import macros
// =============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_PLATFORM_API __declspec(dllexport)
    #else
        // Platform symbols may be built into static libs (e.g. dong_render/appcore); avoid dllimport.
        #define DONG_PLATFORM_API
    #endif
#else
    #define DONG_PLATFORM_API __attribute__((visibility("default")))
#endif


// =============================================================================
// Platform Abstraction Layer
// =============================================================================
// The Platform singleton provides dependency injection for GPU, Surface, FileSystem,
// and Logger subsystems. This enables the core dong.dll to remain platform-agnostic.
//
// Usage:
//   1. Application initializes a backend (e.g., SDL backend)
//   2. Backend registers its implementations via dong_platform_set_*()
//   3. Core engine accesses them via dong_platform_get_*()
// =============================================================================

// Forward declarations for subsystem types
typedef struct DongGPUDriver DongGPUDriver;
typedef struct DongSurface DongSurface;
typedef struct DongSurfaceFactory DongSurfaceFactory;

// Full definitions (for external injection)
#include "dong_file_system.h"
#include "dong_logger.h"


// Platform singleton (opaque)
typedef struct DongPlatform DongPlatform;

// =============================================================================
// Platform Singleton Access
// =============================================================================

// Get the global Platform singleton.
// Always returns a valid pointer (lazily initialized on first call).
DONG_PLATFORM_API DongPlatform* dong_platform_get(void);

// Reset the platform to initial state (for testing/shutdown).
// After this call, all registered subsystems are cleared.
DONG_PLATFORM_API void dong_platform_reset(void);

// =============================================================================
// Subsystem Registration
// =============================================================================

// Register GPU driver implementation.
// Ownership is NOT transferred; caller must keep the driver alive while registered.
DONG_PLATFORM_API void dong_platform_set_gpu_driver(DongPlatform* platform, DongGPUDriver* driver);

// Register Surface factory implementation.
DONG_PLATFORM_API void dong_platform_set_surface_factory(DongPlatform* platform, DongSurfaceFactory* factory);

// Register FileSystem implementation.
DONG_PLATFORM_API void dong_platform_set_file_system(DongPlatform* platform, DongFileSystem* fs);

// Register Logger implementation.
DONG_PLATFORM_API void dong_platform_set_logger(DongPlatform* platform, DongLogger* logger);

// =============================================================================
// Subsystem Access
// =============================================================================

// Get registered GPU driver (may be NULL if not registered).
DONG_PLATFORM_API DongGPUDriver* dong_platform_get_gpu_driver(DongPlatform* platform);

// Get registered Surface factory (may be NULL if not registered).
DONG_PLATFORM_API DongSurfaceFactory* dong_platform_get_surface_factory(DongPlatform* platform);

// Get registered FileSystem (may be NULL if not registered).
DONG_PLATFORM_API DongFileSystem* dong_platform_get_file_system(DongPlatform* platform);

// Get registered Logger (may be NULL if not registered).
DONG_PLATFORM_API DongLogger* dong_platform_get_logger(DongPlatform* platform);

// =============================================================================
// Convenience Macros (C++)
// =============================================================================
#ifdef __cplusplus
// Quick access macros for C++ code
#define DONG_PLATFORM()   dong_platform_get()
#define DONG_GPU()        dong_platform_get_gpu_driver(dong_platform_get())
#define DONG_SURFACES()   dong_platform_get_surface_factory(dong_platform_get())
#define DONG_FS()         dong_platform_get_file_system(dong_platform_get())
#define DONG_LOGGER()     dong_platform_get_logger(dong_platform_get())
#endif

#ifdef __cplusplus
}
#endif

#endif // DONG_PLATFORM_H
