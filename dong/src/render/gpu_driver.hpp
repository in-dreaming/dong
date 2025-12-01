#pragma once

#include "gpu_ir.hpp"
#include <SDL3/SDL_gpu.h>

namespace dong::render {

// 抽象 GPUDriver，后端（基于 SDL_gpu / OpenGL / Metal 等）实现该接口
class GPUDriver {
public:
    virtual ~GPUDriver() = default;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // 执行一串 GPUCommandList（调用方负责编译 DisplayList）
    virtual void execute(const GPUCommandList& commands) = 0;
    
    // Offscreen rendering (optional, default implementations do nothing)
    virtual void beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height) {
        (void)target; (void)width; (void)height;
    }
    virtual void endFrameOffscreen() {}
};

} // namespace dong::render
