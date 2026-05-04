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
    // 基类不管理 GPU 资源，由后端子类（如 GPUTextureSurfaceImpl）负责释放
    // SDL 后端在 ~GPUTextureSurfaceImpl() 中调用 SDL_ReleaseGPUTexture
}

void GPUTextureSurface::clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // 基类实现为空，由后端子类实现具体的 GPU 清空操作
    // SDL 后端在渲染 pass 中处理清空
    is_dirty_ = true;
}

void GPUTextureSurface::lock() {
    // 基类实现为空，由后端子类实现 GPU->CPU 同步（如需要）
    // 当前 SDL 后端不需要显式 lock，渲染时自动处理同步
}

void GPUTextureSurface::unlock() {
    // 基类实现为空，由后端子类实现 CPU->GPU 同步（如需要）
    // 当前 SDL 后端不需要显式 unlock
}

} // namespace dong::render
