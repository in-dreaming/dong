#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <SDL3/SDL_gpu.h>

namespace dong::sdl_backend {

// GPU 设备管理：负责 SDL GPU 设备的生命周期和状态
class GPUDevice {
public:
    struct CreateInfo {
        SDL_GPUShaderFormat shader_format;
        bool debug_mode = false;
    };

    GPUDevice() = default;
    ~GPUDevice();

    // 初始化 GPU 设备
    bool initialize(const CreateInfo& info);

    // 采用外部创建的 GPU 设备（用于宿主应用已创建设备的情况）
    void adoptExternal(SDL_GPUDevice* external_device, SDL_GPUShaderFormat format);

    // 获取 SDL GPU 设备句柄
    SDL_GPUDevice* getHandle() const { return device_; }

    // 获取当前的着色器格式
    SDL_GPUShaderFormat getShaderFormat() const { return shader_format_; }

    // 获取命令缓冲区
    SDL_GPUCommandBuffer* acquireCommandBuffer() const;

    // 提交命令缓冲区
    void submitCommandBuffer(SDL_GPUCommandBuffer* cmd_buf) const;

    // 等待 GPU 空闲
    void waitForGPU() const;

    // 检查是否初始化
    bool isInitialized() const { return device_ != nullptr; }

    // 检查是否拥有设备（false 表示设备由外部管理）
    bool ownsDevice() const { return owns_device_; }

private:
    SDL_GPUDevice* device_ = nullptr;
    SDL_GPUShaderFormat shader_format_ = SDL_GPU_SHADERFORMAT_INVALID;
    bool owns_device_ = false; // true: we created the device and must destroy it
};

} // namespace dong::sdl_backend
