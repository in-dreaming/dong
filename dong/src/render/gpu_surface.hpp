#pragma once

#include "render_surface.hpp"
#include <SDL3/SDL_gpu.h>
#include <memory>

namespace dong::render {

// GPU 纹理表面的完整实现
class GPUTextureSurfaceImpl : public GPUTextureSurface {
public:
    GPUTextureSurfaceImpl(
        SDL_GPUDevice* gpu_device,
        SDL_Window* window,
        uint32_t width,
        uint32_t height
    );
    ~GPUTextureSurfaceImpl();

    // RenderSurface 接口实现
    Type getType() const override { return Type::GPU_TEXTURE; }
    uint32_t getWidth() const override { return width_; }
    uint32_t getHeight() const override { return height_; }
    uint32_t getStride() const override { return width_ * 4; }

    uint32_t getGPUTextureID() const override;

    void* getCPUBuffer() override { return nullptr; }
    const void* getCPUBuffer() const override { return nullptr; }

    void markDirty() override { is_dirty_ = true; }
    bool isDirty() const override { return is_dirty_; }

    void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) override;
    void lock() override {}
    void unlock() override {}

    // GPU 特定接口
    SDL_GPUTexture* getTexture() const { return render_target_; }
    SDL_GPUDevice* getDevice() const { return gpu_device_; }
    SDL_Window* getWindow() const { return window_; }

    // 调整大小
    void resize(uint32_t width, uint32_t height);

    // 获取 Swapchain 纹理用于显示
    SDL_GPUTexture* acquireSwapchainTexture();

private:
    SDL_GPUDevice* gpu_device_;
    SDL_Window* window_;
    SDL_GPUTexture* render_target_;
    uint32_t width_;
    uint32_t height_;
    bool is_dirty_;
};

} // namespace dong::render
