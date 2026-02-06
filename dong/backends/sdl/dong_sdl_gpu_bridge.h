#ifndef DONG_SDL_GPU_BRIDGE_H
#define DONG_SDL_GPU_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque bridge handle (implemented in C++).
typedef struct DongSDLGPUBridge DongSDLGPUBridge;

// Forward declaration for C API GPU Driver
typedef struct DongGPUDriver DongGPUDriver;

DongSDLGPUBridge* dong_sdl_gpu_bridge_create(void* sdl_device, void* sdl_window, DongGPUDriver* driver);
void dong_sdl_gpu_bridge_destroy(DongSDLGPUBridge* bridge);

// Execute one frame worth of GPUCommandList.
int dong_sdl_gpu_bridge_execute(DongSDLGPUBridge* bridge, const void* command_list);

// Resource preupload phase (glyphs/images) without drawing.
int dong_sdl_gpu_bridge_prepare_resources(DongSDLGPUBridge* bridge, const void* command_list);

// External images (video frames, etc.)
int dong_sdl_gpu_bridge_update_external_image_rgba(DongSDLGPUBridge* bridge, const char* key,
                                                  const uint8_t* rgba, uint32_t width,
                                                  uint32_t height, uint32_t stride_bytes);

int dong_sdl_gpu_bridge_update_external_image_yuv420p(DongSDLGPUBridge* bridge, const char* key,
                                                      const uint8_t* plane_y, uint32_t stride_y,
                                                      const uint8_t* plane_u, uint32_t stride_u,
                                                      const uint8_t* plane_v, uint32_t stride_v,
                                                      uint32_t width, uint32_t height);

void dong_sdl_gpu_bridge_set_resource_root(DongSDLGPUBridge* bridge, const char* root);

// Offscreen rendering.
int dong_sdl_gpu_bridge_begin_frame_offscreen(DongSDLGPUBridge* bridge, void* sdl_target_texture,
                                             uint32_t width, uint32_t height);
int dong_sdl_gpu_bridge_end_frame_offscreen(DongSDLGPUBridge* bridge);

#ifdef __cplusplus
}
#endif

#endif // DONG_SDL_GPU_BRIDGE_H
