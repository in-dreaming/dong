#include "shader_manager.hpp"
#include "gpu_device.hpp"
#include <SDL3/SDL_log.h>

namespace dong::render {

ShaderManager::ShaderManager(GPUDevice* gpu_device)
    : gpu_device_(gpu_device) {
}

ShaderManager::~ShaderManager() {
    releaseAll();
}

SDL_GPUShader* ShaderManager::loadShader(
    const std::string& name,
    SDL_GPUShaderStage stage,
    const ShaderSource& source,
    const char* entry_point) {
    
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPU device not initialized");
        return nullptr;
    }

    // 检查是否已经缓存
    auto it = shader_cache_.find(name);
    if (it != shader_cache_.end()) {
        return it->second;
    }

    // 创建新着色器
    SDL_GPUShader* shader = createShaderFromBinary(stage, source, entry_point);
    if (shader) {
        shader_cache_[name] = shader;
        SDL_Log("Shader '%s' loaded successfully", name.c_str());
    } else {
        SDL_Log("Failed to load shader '%s'", name.c_str());
    }

    return shader;
}

SDL_GPUShader* ShaderManager::getShader(const std::string& name) const {
    auto it = shader_cache_.find(name);
    if (it != shader_cache_.end()) {
        return it->second;
    }
    return nullptr;
}

void ShaderManager::releaseShader(const std::string& name) {
    auto it = shader_cache_.find(name);
    if (it != shader_cache_.end()) {
        if (it->second && gpu_device_ && gpu_device_->isInitialized()) {
            SDL_ReleaseGPUShader(gpu_device_->getHandle(), it->second);
        }
        shader_cache_.erase(it);
    }
}

void ShaderManager::releaseAll() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        shader_cache_.clear();
        return;
    }

    for (auto& pair : shader_cache_) {
        if (pair.second) {
            SDL_ReleaseGPUShader(gpu_device_->getHandle(), pair.second);
        }
    }
    shader_cache_.clear();
}

SDL_GPUShader* ShaderManager::createShaderFromBinary(
    SDL_GPUShaderStage stage,
    const ShaderSource& source,
    const char* entry_point) {
    
    if (!source.data || source.size == 0) {
        SDL_Log("Invalid shader source");
        return nullptr;
    }

    SDL_GPUShaderCreateInfo shader_info = {
        .code = source.data,
        .code_size = source.size,
        .entrypoint = entry_point,
        .stage = stage,
        .format = gpu_device_->getShaderFormat(),
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0,
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu_device_->getHandle(), &shader_info);
    if (!shader) {
        SDL_Log("Failed to create GPU shader: %s", SDL_GetError());
    }

    return shader;
}

} // namespace dong::render
