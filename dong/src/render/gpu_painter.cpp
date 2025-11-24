#include "gpu_painter.hpp"
#include "gpu_device.hpp"
#include "shader_manager.hpp"
#include "gpu_surface.hpp"
#include "../dom/dom_manager.hpp"
#include "../layout/layout_engine.hpp"
#include <SDL3/SDL_log.h>

namespace dong::render {

GPUPainter::GPUPainter(
    GPUTextureSurfaceImpl* gpu_surface,
    GPUDevice* gpu_device,
    ShaderManager* shader_manager
) : gpu_surface_(gpu_surface),
    gpu_device_(gpu_device),
    shader_manager_(shader_manager),
    current_cmd_buf_(nullptr),
    is_rendering_(false) {
}

GPUPainter::~GPUPainter() {
    // 清理资源
}

bool GPUPainter::initialize() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPU device not initialized");
        return false;
    }

    if (!gpu_surface_) {
        SDL_Log("GPU surface not initialized");
        return false;
    }

    setupPipelines();
    SDL_Log("GPU painter initialized");

    return true;
}

void GPUPainter::render(dom::Manager* dom_manager, layout::Engine* layout_engine) {
    (void)dom_manager;
    (void)layout_engine;

    if (!gpu_surface_ || !gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPUPainter::render: invalid GPU surface or device");
        return;
    }

    beginFrame();
    if (!is_rendering_ || !current_cmd_buf_) {
        return;
    }

    // 使用 GPU render pass 对当前 render target 进行清屏
    SDL_GPUColorTargetInfo color_target{};
    color_target.texture = gpu_surface_->getTexture();
    color_target.mip_level = 0;
    color_target.layer_or_depth_plane = 0;
    color_target.clear_color = SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f}; // 先清成白色
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;
    color_target.resolve_texture = nullptr;
    color_target.resolve_mip_level = 0;
    color_target.resolve_layer = 0;
    color_target.cycle = false;
    color_target.cycle_resolve_texture = false;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(
        current_cmd_buf_,
        &color_target,
        1,
        nullptr
    );

    if (!pass) {
        SDL_Log("Failed to begin GPU render pass: %s", SDL_GetError());
        endFrame();
        return;
    }

    // 目前只做清屏，不提交任何 draw 调用
    SDL_EndGPURenderPass(pass);

    endFrame();
}

void GPUPainter::beginFrame() {
    if (is_rendering_) {
        SDL_Log("Already rendering");
        return;
    }

    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
    if (!current_cmd_buf_) {
        SDL_Log("Failed to acquire command buffer");
        return;
    }

    is_rendering_ = true;
}

void GPUPainter::endFrame() {
    if (!is_rendering_ || !current_cmd_buf_) {
        return;
    }

    gpu_device_->submitCommandBuffer(current_cmd_buf_);
    current_cmd_buf_ = nullptr;
    is_rendering_ = false;
}

void GPUPainter::setupPipelines() {
    // 后续实现 GPU Pipeline 设置
    // 包括 vertex/fragment shader 编译和 pipeline 创建
    SDL_Log("GPU pipelines setup (placeholder)");
}

void GPUPainter::drawRect(float x, float y, float width, float height, 
                          uint32_t color, float radius) {
    // 后续实现使用 GPU shader 绘制矩形
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)color;
    (void)radius;
}

void GPUPainter::drawText(const std::string& text, float x, float y, 
                          uint32_t color, float font_size) {
    // 后续实现 GPU 文字渲染
    (void)text;
    (void)x;
    (void)y;
    (void)color;
    (void)font_size;
}

void GPUPainter::drawImage(const std::string& src, float x, float y, 
                           float width, float height, uint8_t alpha) {
    // 后续实现 GPU 图片渲染
    (void)src;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)alpha;
}

} // namespace dong::render
