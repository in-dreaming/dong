#pragma once

#include "painter.hpp"
#include <SDL3/SDL_gpu.h>
#include <memory>
#include "../dom/dom_manager.hpp"
#include "../layout/layout_engine.hpp"

namespace dong::render {

class GPUDevice;
class ShaderManager;
class GPUTextureSurfaceImpl;

// GPU 加速的 Painter 实现（当前独立于 Skia Painter，不继承基类）
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

    // 渲染 DOM 树
    void render(dom::Manager* dom_manager, layout::Engine* layout_engine);

    // 绘制图片
    void drawImage(const std::string& src, float x, float y, float width, float height, uint8_t alpha);

    // GPU 特定接口
    void beginFrame();
    void endFrame();
    void uploadCPUPixelsToGPU(const void* cpu_buffer, uint32_t width, uint32_t height);
    void renderInternal();
    // 调整内容纹理尺寸以匹配 View/窗口大小
    void resizeContentTexture(uint32_t width, uint32_t height);

private:
    GPUTextureSurfaceImpl* gpu_surface_;
    GPUDevice* gpu_device_;
    ShaderManager* shader_manager_;
    SDL_GPUCommandBuffer* current_cmd_buf_;
    bool is_rendering_;

    SDL_GPUShader* fullscreen_vs_ = nullptr;
    SDL_GPUShader* fullscreen_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* fullscreen_pipeline_ = nullptr;

    // 像素上传管道
    SDL_GPUTexture* content_texture_ = nullptr;
    SDL_GPUSampler* content_sampler_ = nullptr;
    SDL_GPUBuffer* quad_vertex_buffer_ = nullptr;
    uint32_t content_width_ = 0;
    uint32_t content_height_ = 0;

    void setupPipelines();
    void setupContentTexture(uint32_t width, uint32_t height);
    void drawRect(float x, float y, float width, float height, 
                  uint32_t color, float radius = 0.0f);
    void drawText(const std::string& text, float x, float y, 
                  uint32_t color, float font_size);
};

} // namespace dong::render
