#pragma once

#include <memory>
#include "gpu_ir.hpp"
#include <SDL3/SDL_gpu.h>

struct SDL_Window;

namespace dong::render {

class GPUDevice;
class ShaderManager;
class ResourceManager;


// 支持多后端选择的枚举，目前仅实现 SDL_gpu 后端
enum class GPUBackendType : uint8_t {
    SDL_GPU = 0,
};

// 抽象 GPUDriver，后端（基于 SDL_gpu / OpenGL / Metal 等）实现该接口
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

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // 执行一串 GPUCommandList（调用方负责编译 DisplayList）
    virtual void execute(const GPUCommandList& commands) = 0;
    
    // Offscreen rendering (optional, default implementations do nothing)
    virtual void beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height) {
        (void)target; (void)width; (void)height;
    }
    virtual void endFrameOffscreen() {}
};

// GPUDriver 工厂：根据后端类型创建具体实现
std::unique_ptr<GPUDriver> CreateGPUDriver(
    GPUBackendType backend,
    GPUDevice* device,
    SDL_Window* window,
    ShaderManager* shader_manager
);

} // namespace dong::render
