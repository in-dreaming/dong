#ifndef DONG_SDL_EXECUTE_H
#define DONG_SDL_EXECUTE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque executor handle (implemented in C++).
typedef struct DongSDLExecutor DongSDLExecutor;

// Create/destroy.
DongSDLExecutor* dong_sdl_executor_create(void* sdl_device, void* sdl_window);
void dong_sdl_executor_destroy(DongSDLExecutor* exec);

// GPUCommandList execution.
int dong_sdl_executor_execute(DongSDLExecutor* exec, const void* command_list);
void dong_sdl_executor_prepare_resources(DongSDLExecutor* exec, const void* command_list);

// Dynamic external images (video frames, etc.)
int dong_sdl_executor_update_external_image_rgba(DongSDLExecutor* exec, const char* key,
                                                const uint8_t* rgba, uint32_t width,
                                                uint32_t height, uint32_t stride_bytes);

int dong_sdl_executor_update_external_image_yuv420p(DongSDLExecutor* exec, const char* key,
                                                    const uint8_t* plane_y, uint32_t stride_y,
                                                    const uint8_t* plane_u, uint32_t stride_u,
                                                    const uint8_t* plane_v, uint32_t stride_v,
                                                    uint32_t width, uint32_t height);

// Resource root for resolving relative image paths.
void dong_sdl_executor_set_resource_root(DongSDLExecutor* exec, const char* root);

// Offscreen rendering.
int dong_sdl_executor_begin_frame_offscreen(DongSDLExecutor* exec, void* sdl_target_texture,
                                           uint32_t width, uint32_t height);
int dong_sdl_executor_end_frame_offscreen(DongSDLExecutor* exec);


#ifdef __cplusplus
}
#endif

#endif // DONG_SDL_EXECUTE_H
