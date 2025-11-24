#include "gpu_device.hpp"
#include <SDL3/SDL_log.h>
#include <cstring>

namespace dong::render {

GPUDevice::~GPUDevice() {
    if (device_) {
        SDL_DestroyGPUDevice(device_);
        device_ = nullptr;
    }
}

bool GPUDevice::initialize(const CreateInfo& info) {
    if (device_) {
        SDL_Log("GPU device already initialized");
        return false;
    }

    // Create GPU device
    // SDL3 will choose the best GPU backend for the platform
    SDL_GPUShaderFormat format_flags = info.shader_format;

    device_ = SDL_CreateGPUDevice(format_flags, info.debug_mode, nullptr);
    if (!device_) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        return false;
    }

    shader_format_ = info.shader_format;
    SDL_Log("GPU device initialized successfully with format: %d", shader_format_);

    return true;
}

void GPUDevice::adoptExternal(SDL_GPUDevice* external_device, SDL_GPUShaderFormat format) {
    if (!external_device) {
        SDL_Log("Cannot adopt null GPU device");
        return;
    }

    // Adopt external device without destroying it
    device_ = external_device;
    shader_format_ = format;
    SDL_Log("GPU device adopted from external source with format: %d", shader_format_);
}

SDL_GPUCommandBuffer* GPUDevice::acquireCommandBuffer() const {
    if (!device_) {
        SDL_Log("GPU device not initialized");
        return nullptr;
    }
    return SDL_AcquireGPUCommandBuffer(device_);
}

void GPUDevice::submitCommandBuffer(SDL_GPUCommandBuffer* cmd_buf) const {
    if (!device_) {
        SDL_Log("GPU device not initialized");
        return;
    }
    if (cmd_buf) {
        SDL_SubmitGPUCommandBuffer(cmd_buf);
    }
}

void GPUDevice::waitForGPU() const {
    if (device_) {
        SDL_WaitForGPUIdle(device_);
    }
}

} // namespace dong::render
