#ifndef DONG_GPU_BACKEND_DRIVER_H
#define DONG_GPU_BACKEND_DRIVER_H

#include "dong_gpu_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_GPU_BUILDING_DLL
        #define DONG_GPU_BACKEND_API __declspec(dllexport)
    #else
        #define DONG_GPU_BACKEND_API __declspec(dllimport)
    #endif
#else
    #define DONG_GPU_BACKEND_API __attribute__((visibility("default")))
#endif

struct DongUiGraphContext;

DONG_GPU_BACKEND_API DongGPUDriver* dong_gpu_backend_create_driver(void);
DONG_GPU_BACKEND_API void dong_gpu_backend_destroy_driver(DongGPUDriver* driver);
DONG_GPU_BACKEND_API struct DongUiGraphContext* dong_gpu_backend_get_ui_graph(DongGPUDriver* driver);
DONG_GPU_BACKEND_API void dong_gpu_driver_set_external_swapchain(DongGPUDriver* driver,
                                                                 void* surface,
                                                                 void* queue,
                                                                 int owns_surface);
DONG_GPU_BACKEND_API void dong_gpu_driver_set_external_window(DongGPUDriver* driver, void* window);

typedef struct DongGpuTextureViewHandle {
    uint32_t index;
    uint32_t generation;
} DongGpuTextureViewHandle;

DONG_GPU_BACKEND_API int dong_gpu_backend_get_texture_shader_view(DongGPUDriver* driver,
                                                                  DongGPUTexture texture,
                                                                  DongGpuTextureViewHandle* out_view);

DONG_GPU_BACKEND_API int dong_gpu_backend_readback_texture_rgba(DongGPUDriver* driver,
                                                                DongGPUTexture texture,
                                                                uint32_t width,
                                                                uint32_t height,
                                                                uint8_t* out_rgba,
                                                                size_t out_rgba_bytes);

#ifdef __cplusplus
}
#endif

#endif
