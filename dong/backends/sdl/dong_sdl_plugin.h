#ifndef DONG_SDL_PLUGIN_H
#define DONG_SDL_PLUGIN_H

#include "dong_plugin_api.h"
#include "dong_sdl_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SDL Backend Plugin
// =============================================================================
// This module provides a complete SDL-based backend for the Dong engine.
// It implements the DongPlugin interface and provides:
//   - Window management (SDL_Window)
//   - GPU rendering (SDL_GPU)
//   - Input handling (SDL Events)
//   - Video decoding (FFmpeg integration)
//
// Usage:
//   1. Call dong_sdl_plugin_create() to create the plugin instance
//   2. Pass the returned vtable to dong_engine_create()
//   3. The plugin will auto-register DongGPUDriver to Platform singleton
//   4. Call dong_sdl_plugin_destroy() on shutdown
// =============================================================================

// SDL plugin configuration
typedef struct dong_sdl_plugin_config_t {
    // Window settings
    const char* window_title;
    uint32_t window_width;
    uint32_t window_height;
    int window_resizable;
    int window_high_dpi;

    // GPU settings
    int gpu_debug_mode;
    int gpu_prefer_mailbox;  // vs vsync

    // Video settings (FFmpeg)
    int video_enabled;
} dong_sdl_plugin_config_t;

// Default configuration helper
static inline dong_sdl_plugin_config_t dong_sdl_plugin_config_default(void) {
    dong_sdl_plugin_config_t cfg = {0};
    cfg.window_title = "Dong App";
    cfg.window_width = 1280;
    cfg.window_height = 720;
    cfg.window_resizable = 1;
    cfg.window_high_dpi = 1;
    cfg.gpu_debug_mode = 0;
    cfg.gpu_prefer_mailbox = 1;
    cfg.video_enabled = 1;
    return cfg;
}

// Create SDL backend plugin instance
// Returns: plugin vtable pointer, or NULL on failure
DONG_SDL_PLATFORM_API dong_plugin_vtable_t* dong_sdl_plugin_create(
    const dong_sdl_plugin_config_t* config);

// Destroy SDL backend plugin instance
// This will cleanup all SDL resources and unregister from Platform
DONG_SDL_PLATFORM_API void dong_sdl_plugin_destroy(dong_plugin_vtable_t* plugin);

// Check if SDL backend is available (SDL3 installed)
DONG_SDL_PLATFORM_API int dong_sdl_plugin_available(void);

// Get last error message (valid until next call)
DONG_SDL_PLATFORM_API const char* dong_sdl_plugin_get_error(void);

// =============================================================================
// Internal SDL types (for advanced use only)
// =============================================================================
// These allow accessing raw SDL handles when needed

// Get raw SDL_Window from dong_window_t
DONG_SDL_PLATFORM_API void* dong_sdl_plugin_get_window_handle(dong_window_t* window);

// Get raw SDL_GPUDevice from dong_gpu_device_t
DONG_SDL_PLATFORM_API void* dong_sdl_plugin_get_gpu_device_handle(dong_gpu_device_t* device);

// =============================================================================
// GPU Driver Registration (called internally by plugin)
// =============================================================================
// The plugin automatically creates and registers a DongGPUDriver to the
// Platform singleton during initialization. Core engine accesses GPU
// functionality through the C API (dong_gpu_driver.h), not directly.

#ifdef __cplusplus
}
#endif

#endif // DONG_SDL_PLUGIN_H
