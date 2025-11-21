#include "render_surface.hpp"
#include <cstring>

namespace dong::render {

// ============================================================
// CPUBufferSurface Implementation
// ============================================================

CPUBufferSurface::CPUBufferSurface(uint32_t width, uint32_t height)
    : width_(width), height_(height), is_dirty_(true) {
    uint32_t size = width_ * height_ * 4; // RGBA
    buffer_ = new uint8_t[size];
    std::memset(buffer_, 0, size); // 初始化为透明黑
}

CPUBufferSurface::~CPUBufferSurface() {
    delete[] buffer_;
}

void CPUBufferSurface::clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
    uint32_t* pixels = reinterpret_cast<uint32_t*>(buffer_);
    uint32_t size = width_ * height_;
    for (uint32_t i = 0; i < size; ++i) {
        pixels[i] = color;
    }
    is_dirty_ = true;
}

void CPUBufferSurface::resize(uint32_t new_width, uint32_t new_height) {
    if (new_width == width_ && new_height == height_) return;

    delete[] buffer_;
    width_ = new_width;
    height_ = new_height;

    uint32_t size = width_ * height_ * 4;
    buffer_ = new uint8_t[size];
    std::memset(buffer_, 0, size);
    is_dirty_ = true;
}

// ============================================================
// GPUTextureSurface Implementation
// ============================================================

GPUTextureSurface::GPUTextureSurface(uint32_t width, uint32_t height, uint32_t texture_id)
    : width_(width), height_(height), texture_id_(texture_id), is_dirty_(true) {
}

GPUTextureSurface::~GPUTextureSurface() {
    // TODO: 释放 GPU 纹理资源（需要 GL 上下文）
}

void GPUTextureSurface::clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // TODO: 通过 OpenGL/Vulkan API 清空 GPU 纹理
    is_dirty_ = true;
}

void GPUTextureSurface::lock() {
    // TODO: 如果需要 CPU 访问 GPU 纹理，可以在这里进行 GPU->CPU 同步
    // 例如：glReadPixels()
}

void GPUTextureSurface::unlock() {
    // TODO: CPU 修改完成后，进行 CPU->GPU 同步
    // 例如：glTexImage2D() 或 glTexSubImage2D()
}

} // namespace dong::render
