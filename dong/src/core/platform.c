// Platform singleton implementation
// This file provides the global Platform instance for dependency injection.

#include "dong_platform.h"
#include <stdlib.h>
#include <string.h>

// Override the macro for this implementation file
#undef DONG_PLATFORM_API
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_PLATFORM_API __declspec(dllexport)
    #else
        #define DONG_PLATFORM_API
    #endif
#else
    #define DONG_PLATFORM_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Platform Implementation
// =============================================================================

typedef struct DongPlatformImpl {
    DongGPUDriver* gpu_driver;
    DongSurfaceFactory* surface_factory;
    DongFileSystem* file_system;
    DongLogger* logger;
} DongPlatformImpl;

// Global singleton storage
static DongPlatformImpl g_platform_instance = {0};
static int g_platform_initialized = 0;

// Cast impl to opaque type
static inline DongPlatform* impl_to_platform(DongPlatformImpl* impl) {
    return (DongPlatform*)impl;
}

static inline DongPlatformImpl* platform_to_impl(DongPlatform* platform) {
    return (DongPlatformImpl*)platform;
}

// =============================================================================
// Public API
// =============================================================================

DONG_PLATFORM_API DongPlatform* dong_platform_get(void) {
    if (!g_platform_initialized) {
        memset(&g_platform_instance, 0, sizeof(g_platform_instance));
        g_platform_initialized = 1;
    }
    return impl_to_platform(&g_platform_instance);
}

DONG_PLATFORM_API void dong_platform_reset(void) {
    memset(&g_platform_instance, 0, sizeof(g_platform_instance));
    g_platform_initialized = 0;
}

DONG_PLATFORM_API void dong_platform_set_gpu_driver(DongPlatform* platform, DongGPUDriver* driver) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->gpu_driver = driver;
}

DONG_PLATFORM_API void dong_platform_set_surface_factory(DongPlatform* platform, DongSurfaceFactory* factory) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->surface_factory = factory;
}

DONG_PLATFORM_API void dong_platform_set_file_system(DongPlatform* platform, DongFileSystem* fs) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->file_system = fs;
}

DONG_PLATFORM_API void dong_platform_set_logger(DongPlatform* platform, DongLogger* logger) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->logger = logger;
}

DONG_PLATFORM_API DongGPUDriver* dong_platform_get_gpu_driver(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->gpu_driver;
}

DONG_PLATFORM_API DongSurfaceFactory* dong_platform_get_surface_factory(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->surface_factory;
}

DONG_PLATFORM_API DongFileSystem* dong_platform_get_file_system(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->file_system;
}

DONG_PLATFORM_API DongLogger* dong_platform_get_logger(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->logger;
}

#ifdef __cplusplus
}
#endif
