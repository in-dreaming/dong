#ifndef DONG_SDL_PLATFORM_H
#define DONG_SDL_PLATFORM_H

#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DLL export/import macros
// =============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_SDL_BUILDING_DLL
        #define DONG_SDL_PLATFORM_API __declspec(dllexport)
    #else
        #define DONG_SDL_PLATFORM_API __declspec(dllimport)
    #endif
#else
    #define DONG_SDL_PLATFORM_API __attribute__((visibility("default")))
#endif

// =============================================================================
// SDL Platform Backend
// =============================================================================
// This module provides SDL-based implementations of the Platform abstraction.
// It creates GPUDriver, SurfaceFactory, and other subsystems using SDL3.
//
// Usage:
//   1. Create SDL_Window and SDL_GPUDevice
//   2. Call dong_sdl_platform_init() with the handles
//   3. Core dong.dll will use the registered subsystems
//   4. Call dong_sdl_platform_shutdown() on exit
// =============================================================================

// =============================================================================
// Quick Setup (Convenience)
// =============================================================================

// Initialize SDL platform with existing device and window.
// Registers all SDL-based subsystems with the global Platform singleton.
// Returns 1 on success, 0 on failure.
DONG_SDL_PLATFORM_API int dong_sdl_platform_init(void* sdl_device, void* sdl_window);

// Shutdown SDL platform and cleanup all registered subsystems.
DONG_SDL_PLATFORM_API void dong_sdl_platform_shutdown(void);

// =============================================================================
// Individual Component Creation (For Advanced Use)
// =============================================================================

// Create an SDL-based GPU driver.
// sdl_device: SDL_GPUDevice* pointer
// sdl_window: SDL_Window* pointer (may be NULL for offscreen)
// Returns: DongGPUDriver* or NULL on failure
// Caller must destroy with dong_sdl_destroy_gpu_driver()
DONG_SDL_PLATFORM_API DongGPUDriver* dong_sdl_create_gpu_driver(void* sdl_device, void* sdl_window);

// Destroy an SDL GPU driver created with dong_sdl_create_gpu_driver()
DONG_SDL_PLATFORM_API void dong_sdl_destroy_gpu_driver(DongGPUDriver* driver);

// Create an SDL-based surface factory.
// sdl_device: SDL_GPUDevice* pointer
// Returns: DongSurfaceFactory* or NULL on failure
// Caller must destroy with dong_sdl_destroy_surface_factory()
DONG_SDL_PLATFORM_API DongSurfaceFactory* dong_sdl_create_surface_factory(void* sdl_device);

// Destroy an SDL surface factory
DONG_SDL_PLATFORM_API void dong_sdl_destroy_surface_factory(DongSurfaceFactory* factory);

#ifdef __cplusplus
}
#endif

#endif // DONG_SDL_PLATFORM_H
