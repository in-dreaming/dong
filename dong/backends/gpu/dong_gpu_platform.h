#ifndef DONG_GPU_PLATFORM_H
#define DONG_GPU_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_GPU_BUILDING_DLL
        #define DONG_GPU_PLATFORM_API __declspec(dllexport)
    #else
        #define DONG_GPU_PLATFORM_API __declspec(dllimport)
    #endif
#else
    #define DONG_GPU_PLATFORM_API __attribute__((visibility("default")))
#endif

// Initialize the in-dreaming/gpu platform backend.
// Registers DongGPUDriver with the global Platform singleton.
// embedded_mode=1: host owns GpuGraph/present (gpu_ui_inject); 0: driver presents swapchain.
// Returns 1 on success, 0 on failure.
DONG_GPU_PLATFORM_API int dong_gpu_platform_init_ex(int embedded_mode);
DONG_GPU_PLATFORM_API int dong_gpu_platform_init(void);

// Shutdown gpu platform backend.
DONG_GPU_PLATFORM_API void dong_gpu_platform_shutdown(void);

DONG_GPU_PLATFORM_API struct DongGPUDriver* dong_gpu_platform_get_driver(void);

#ifdef __cplusplus
}
#endif

#endif // DONG_GPU_PLATFORM_H
