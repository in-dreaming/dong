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

private:
    GPUTextureSurfaceImpl* gpu_surface_;
    GPUDevice* gpu_device_;
    ShaderManager* shader_manager_;
    SDL_GPUCommandBuffer* current_cmd_buf_;
    bool is_rendering_;

    void setupPipelines();
    void drawRect(float x, float y, float width, float height, 
                  uint32_t color, float radius = 0.0f);
    void drawText(const std::string& text, float x, float y, 
                  uint32_t color, float font_size);
};

} // namespace dong::render
