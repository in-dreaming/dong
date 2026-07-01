#ifndef DONG_SDL_GPU_FORMATS_H
#define DONG_SDL_GPU_FORMATS_H

#include <SDL3/SDL_gpu.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shader formats for SDL_CreateGPUDevice.
 * On Windows D3D12, do NOT include SPIRV: SDL_shadercross will upload raw SPIR-V
 * and SDL_CreateGPUGraphicsPipeline fails with E_INVALIDARG (0x80070057). */
static inline SDL_GPUShaderFormat dong_sdl_default_shader_formats(void) {
#if defined(__APPLE__)
    return SDL_GPU_SHADERFORMAT_MSL;
#elif defined(_WIN32)
    return SDL_GPU_SHADERFORMAT_DXIL;
#else
    return SDL_GPU_SHADERFORMAT_SPIRV;
#endif
}

static inline SDL_GPUShaderFormat dong_sdl_shader_formats_for_device(SDL_GPUDevice* device) {
    if (device) {
        return SDL_GetGPUShaderFormats(device);
    }
    return dong_sdl_default_shader_formats();
}

#ifdef __cplusplus
}
#endif

#endif /* DONG_SDL_GPU_FORMATS_H */
