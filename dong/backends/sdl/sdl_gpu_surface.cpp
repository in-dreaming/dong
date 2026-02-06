#include "sdl_gpu_surface.hpp"
#include "../../src/core/log.h"

namespace dong::sdl_backend {

GPUTextureSurfaceImpl::GPUTextureSurfaceImpl(
    SDL_GPUDevice* gpu_device,
    SDL_Window* window,
    uint32_t width,
    uint32_t height
) : dong::render::GPUTextureSurface(width, height, 0),
    gpu_device_(gpu_device),
    window_(window),
    width_(width),
    height_(height),
    is_dirty_(true) {
    
    // 创建渲染目标纹理
    SDL_GPUTextureCreateInfo texture_info = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = width,
        .height = height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
    };

    render_target_ = SDL_CreateGPUTexture(gpu_device_, &texture_info);
    if (!render_target_) {
        DONG_LOG_ERROR("Failed to create GPU texture: %s", SDL_GetError());
    } else {
        DONG_LOG_INFO("GPU texture created: %u x %u", width, height);
    }
}

GPUTextureSurfaceImpl::~GPUTextureSurfaceImpl() {
    if (render_target_ && gpu_device_) {
        SDL_ReleaseGPUTexture(gpu_device_, render_target_);
        render_target_ = nullptr;
    }
}

uint32_t GPUTextureSurfaceImpl::getGPUTextureID() const {
    // SDL GPU 不提供纹理 ID，因为它使用不透明句柄
    // 返回指针作为唯一标识
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(render_target_));
}

void GPUTextureSurfaceImpl::clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // GPU 清空由 GPU painter 在 render pass 中处理
    is_dirty_ = true;
}

void GPUTextureSurfaceImpl::resize(uint32_t width, uint32_t height) {
    if (width == width_ && height == height_) {
        return;
    }

    // 释放旧纹理
    if (render_target_ && gpu_device_) {
        SDL_ReleaseGPUTexture(gpu_device_, render_target_);
    }

    width_ = width;
    height_ = height;

    // 创建新纹理
    SDL_GPUTextureCreateInfo texture_info = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = width,
        .height = height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
    };

    render_target_ = SDL_CreateGPUTexture(gpu_device_, &texture_info);
    if (!render_target_) {
        DONG_LOG_ERROR("Failed to recreate GPU texture: %s", SDL_GetError());
    } else {
        DONG_LOG_INFO("GPU texture resized to: %u x %u", width, height);
    }

    is_dirty_ = true;
}

SDL_GPUTexture* GPUTextureSurfaceImpl::acquireSwapchainTexture() {
    if (!gpu_device_ || !window_) {
        DONG_LOG_ERROR("Cannot acquire swapchain texture: invalid device/window");
        return nullptr;
    }

    Uint32 swapchain_width = 0;
    Uint32 swapchain_height = 0;
    SDL_GPUTexture* swapchain_texture = nullptr;

    // 为了满足 SDL_WaitAndAcquireGPUSwapchainTexture 的接口，这里临时获取一个命令缓冲
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu_device_);
    if (!cmd) {
        DONG_LOG_ERROR("Failed to acquire command buffer for swapchain texture: %s", SDL_GetError());
        return nullptr;
    }

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            cmd,
            window_,
            &swapchain_texture,
            &swapchain_width,
            &swapchain_height)) {
        DONG_LOG_ERROR("Failed to acquire swapchain texture: %s", SDL_GetError());
        return nullptr;
    }

    // 当前实现仅获取 swapchain 纹理，不在此处提交绘制命令
    return swapchain_texture;
}

} // namespace dong::sdl_backend
