#pragma once

#include "dong_gpu_backend_driver.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

DONG_GPU_BACKEND_API void* dong_gpu_backend_create_scene3d_render_pipeline(void* device, void* shader_program,
                                                                         uint32_t color_format,
                                                                         uint32_t depth_format);

DONG_GPU_BACKEND_API void dong_gpu_backend_destroy_scene3d_render_pipeline(void* pipeline);

DONG_GPU_BACKEND_API int dong_gpu_backend_scene3d_bind_textured_draw(void* device, void* pass_encoder,
                                                                     void* sampler_handle,
                                                                     const DongGpuTextureViewHandle* texture,
                                                                     const void* uniforms, size_t uniform_size);

#ifdef __cplusplus
}
#endif
