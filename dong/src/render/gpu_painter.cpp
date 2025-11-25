#include "gpu_painter.hpp"
#include "gpu_device.hpp"
#include "shader_manager.hpp"
#include "gpu_surface.hpp"
#include <SDL3/SDL_log.h>
#include <cstring>

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
    if (in_frame_ && gpu_device_ && current_cmd_buf_) {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
    }
    current_cmd_buf_ = nullptr;
    in_frame_ = false;
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

    content_width_ = gpu_surface_->getWidth();
    content_height_ = gpu_surface_->getHeight();
    
    if (content_width_ == 0 || content_height_ == 0) {
        content_width_ = 960;
        content_height_ = 600;
    }

    setupPipelines();
    SDL_Log("GPU painter initialized");
    return true;
}

void GPUPainter::renderDisplayList(const DisplayList& display_list) {
    (void)display_list;
    
    if (!gpu_surface_ || !gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPUPainter::renderDisplayList: invalid GPU surface or device");
        return;
    }

    beginFrame();
    if (!is_rendering_ || !current_cmd_buf_) {
        return;
    }

    renderDisplayListInternal(display_list);

    endFrame();
}

void GPUPainter::renderDisplayListInternal(const DisplayList& display_list) {
    (void)display_list;
    // TODO: 实际渲染 DisplayList
    SDL_Log("GPUPainter::renderDisplayListInternal: stub");
}

void GPUPainter::beginFrame() {
    if (is_rendering_) {
        SDL_Log("GPUPainter::beginFrame: already in frame");
        return;
    }

    if (!gpu_device_ || !gpu_device_->isInitialized()) {
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
    if (!is_rendering_) {
        return;
    }

    if (current_cmd_buf_ && gpu_device_) {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
    }

    current_cmd_buf_ = nullptr;
    is_rendering_ = false;
}

void GPUPainter::resizeContentTexture(uint32_t width, uint32_t height) {
    content_width_ = width;
    content_height_ = height;
}

void GPUPainter::setupPipelines() {
    // TODO: 设置渲染管线
}

} // namespace dong::render
