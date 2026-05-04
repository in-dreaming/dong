#include "sdl_gpu_device.hpp"
#include "../../src/core/log.h"
#include "../../src/core/profiler.h"
#include <cstring>


namespace dong::sdl_backend {

GPUDevice::~GPUDevice() {
    // Only destroy devices that we created ourselves. External devices are
    // owned by the host (e.g., SDL3Window) and must not be destroyed here.
    if (device_ && owns_device_) {
        SDL_DestroyGPUDevice(device_);
    }
    device_ = nullptr;
}

bool GPUDevice::initialize(const CreateInfo& info) {
    if (device_) {
        DONG_LOG_WARN("GPU device already initialized");
        return false;
    }

    // Create GPU device
    // SDL3 will choose the best GPU backend for the platform
    SDL_GPUShaderFormat format_flags = info.shader_format;

    device_ = SDL_CreateGPUDevice(format_flags, info.debug_mode, nullptr);
    if (!device_) {
        DONG_LOG_ERROR("Failed to create GPU device: %s", SDL_GetError());
        return false;
    }

    shader_format_ = info.shader_format;
    owns_device_ = true;
    DONG_LOG_INFO("GPU device initialized successfully with format: %d", shader_format_);

    return true;
}

void GPUDevice::adoptExternal(SDL_GPUDevice* external_device, SDL_GPUShaderFormat format) {
    if (!external_device) {
        DONG_LOG_WARN("Cannot adopt null GPU device");
        return;
    }

    // If we currently own an internally created device, destroy it first.
    if (device_ && owns_device_) {
        SDL_DestroyGPUDevice(device_);
        device_ = nullptr;
    }

    // Adopt external device without taking ownership (lifetime managed by host)
    device_ = external_device;
    shader_format_ = format;
    owns_device_ = false;
    DONG_LOG_INFO("GPU device adopted from external source with format: %d", shader_format_);
}

SDL_GPUCommandBuffer* GPUDevice::acquireCommandBuffer() const {
    if (!device_) {
        DONG_LOG_WARN("GPU device not initialized");
        return nullptr;
    }
    return SDL_AcquireGPUCommandBuffer(device_);
}

void GPUDevice::submitCommandBuffer(SDL_GPUCommandBuffer* cmd_buf) const {
    if (!device_) {
        DONG_LOG_WARN("GPU device not initialized");
        return;
    }
    if (cmd_buf) {
        DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBuffer", "gpu");
        SDL_SubmitGPUCommandBuffer(cmd_buf);
    }
}


void GPUDevice::waitForGPU() const {
    if (device_) {
        SDL_WaitForGPUIdle(device_);
    }
}

} // namespace dong::sdl_backend
