#pragma once

#include "../../src/render/display_list.hpp"
#include <SDL3/SDL_gpu.h>
#include <memory>

namespace dong::sdl_backend {

class GPUDevice;
class ShaderManager;
class GPUTextureSurfaceImpl;

// GPU 渲染器（从 DisplayList 渲染）
class GPUPainter {
public:
    GPUPainter(
        GPUTextureSurfaceImpl* gpu_surface,
        GPUDevice* gpu_device,
        ShaderManager* shader_manager
    );
    ~GPUPainter();

    // 初始化 GPU 管线
    bool initialize();

    // 从 DisplayList 渲染
    void renderDisplayList(const dong::render::DisplayList& display_list);

    // GPU 特定接口
    void beginFrame();
    void endFrame();
    void resizeContentTexture(uint32_t width, uint32_t height);

private:
    GPUTextureSurfaceImpl* gpu_surface_;
    GPUDevice* gpu_device_;
    ShaderManager* shader_manager_;
    SDL_GPUCommandBuffer* current_cmd_buf_;
    bool is_rendering_;
    bool in_frame_ = false;

    uint32_t content_width_ = 0;
    uint32_t content_height_ = 0;

    void setupPipelines();
    void renderDisplayListInternal(const dong::render::DisplayList& display_list);
};

} // namespace dong::sdl_backend
