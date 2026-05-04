#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include "gpu_ir.hpp"

// Forward declarations for C API types
struct DongGPUDriver;
struct DongSurface;

namespace dong::render {

class ResourceManager;

// Opaque GPU texture handle (backend-specific)
using GPUTextureHandle = void*;

// 支持多后端选择的枚举
enum class GPUBackendType : uint8_t {
    SDL_GPU = 0,
    VULKAN = 1,
    METAL = 2,
    D3D12 = 3,
};

// 抽象 GPUDriver，后端（基于 SDL_gpu / OpenGL / Metal 等）实现该接口
// Note: Core 层通过此抽象接口访问 GPU 能力，不直接依赖 SDL
class GPUDriver {
public:
    virtual ~GPUDriver() = default;

    // 后端初始化接口，由工厂创建实例后调用
    virtual bool initialize() = 0;

    // 预处理命令列表中的资源（如 glyph 纹理上传）
    // 必须在 beginFrame() 之前调用，避免在 render pass 中进行纹理上传
    virtual void prepareResources(const GPUCommandList& commands) { (void)commands; }

    // 可选：注入资源管理器（用于图片 decode / atlas 构建等）
    virtual void setImageResourceManager(ResourceManager* manager) { (void)manager; }

    // Optional: update a dynamic RGBA texture (e.g. video frames) identified by a stable string key.
    // The backend may ignore this if unsupported.
    virtual bool updateExternalImageRGBA(const std::string& key,
                                         const uint8_t* rgba,
                                         uint32_t width,
                                         uint32_t height,
                                         uint32_t stride_bytes) {
        (void)key;
        (void)rgba;
        (void)width;
        (void)height;
        (void)stride_bytes;
        return false;
    }

    // Dynamic YUV420P textures (e.g. video frames).
    // Implementations should upload planes to GPU textures and perform YUV->RGB in shader.
    virtual bool updateExternalImageYUV420P(const std::string& key,
                                            const uint8_t* plane_y,
                                            uint32_t stride_y,
                                            const uint8_t* plane_u,
                                            uint32_t stride_u,
                                            const uint8_t* plane_v,
                                            uint32_t stride_v,
                                            uint32_t width,
                                            uint32_t height) {
        (void)key;
        (void)plane_y;
        (void)stride_y;
        (void)plane_u;
        (void)stride_u;
        (void)plane_v;
        (void)stride_v;
        (void)width;
        (void)height;
        return false;
    }

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // 执行一串 GPUCommandList（调用方负责编译 DisplayList）
    virtual void execute(const GPUCommandList& commands) = 0;
    
    // Offscreen rendering (optional, default implementations do nothing)
    // target is an opaque GPUTextureHandle (backend-specific native texture)
    virtual void beginFrameOffscreen(GPUTextureHandle target, uint32_t width, uint32_t height) {
        (void)target; (void)width; (void)height;
    }
    virtual void endFrameOffscreen() {}

    // Get the underlying C API driver (for advanced use cases)
    // Returns DongGPUDriver* which can be cast to backend-specific type
    virtual DongGPUDriver* getNativeDriver() = 0;
};

// GPUDriver 工厂：根据后端类型创建具体实现
// Note: This factory is implemented in the backend (e.g., backends/sdl/)
// to avoid core -> backend dependency
using GPUDriverFactory = std::unique_ptr<GPUDriver>(*)(
    GPUBackendType backend,
    void* native_device,    // e.g., SDL_GPUDevice* for SDL backend
    void* native_window,    // e.g., SDL_Window* for SDL backend
    void* shader_manager    // backend-specific shader manager
);

// Set the factory function (called by backend during initialization)
void SetGPUDriverFactory(GPUDriverFactory factory);

// Create a GPUDriver using the registered factory
std::unique_ptr<GPUDriver> CreateGPUDriver(
    GPUBackendType backend,
    void* native_device,
    void* native_window,
    void* shader_manager
);

} // namespace dong::render
