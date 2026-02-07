#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <string>
#include <SDL3/SDL_gpu.h>

namespace dong::sdl_backend {

class GPUDevice;

// Compute pipeline 信息
struct ComputePipelineInfo {
    uint32_t num_samplers = 0;
    uint32_t num_readonly_storage_textures = 0;
    uint32_t num_readonly_storage_buffers = 0;
    uint32_t num_readwrite_storage_textures = 0;
    uint32_t num_readwrite_storage_buffers = 0;
    uint32_t num_uniform_buffers = 0;
    uint32_t threadcount_x = 8;
    uint32_t threadcount_y = 8;
    uint32_t threadcount_z = 1;
};

// 着色器管理：负责着色器的加载、编译缓存和生命周期
class ShaderManager {
public:
    struct ShaderSource {
        const uint8_t* data;
        size_t size;
    };

    explicit ShaderManager(GPUDevice* gpu_device);
    ~ShaderManager();

    // 从预编译的二进制数据加载着色器
    SDL_GPUShader* loadShader(
        const std::string& name,
        SDL_GPUShaderStage stage,
        const ShaderSource& source,
        const char* entry_point = "main"
    );

    // 从 HLSL 源码加载并编译着色器（HLSL -> SPIR-V -> 目标后端）
    SDL_GPUShader* loadShaderFromHLSL(
        const std::string& name,
        SDL_GPUShaderStage stage,
        const char* hlsl_source,
        const char* entry_point = "main"
    );

    // 从 HLSL 文件加载并编译着色器（读取文本 -> HLSL -> SPIR-V -> 目标后端）
    SDL_GPUShader* loadShaderFromHLSLFile(
        const std::string& name,
        SDL_GPUShaderStage stage,
        const char* hlsl_file_path,
        const char* entry_point = "main"
    );

    // 从 HLSL 源码编译 compute pipeline
    SDL_GPUComputePipeline* loadComputePipelineFromHLSL(
        const std::string& name,
        const char* hlsl_source,
        const ComputePipelineInfo& info,
        const char* entry_point = "main"
    );

    // 从 HLSL 文件加载 compute pipeline
    SDL_GPUComputePipeline* loadComputePipelineFromHLSLFile(
        const std::string& name,
        const char* hlsl_file_path,
        const ComputePipelineInfo& info,
        const char* entry_point = "main"
    );

    // 获取已缓存的着色器
    SDL_GPUShader* getShader(const std::string& name) const;

    // 获取已缓存的 compute pipeline
    SDL_GPUComputePipeline* getComputePipeline(const std::string& name) const;

    // 释放着色器
    void releaseShader(const std::string& name);

    // 释放 compute pipeline
    void releaseComputePipeline(const std::string& name);

    // 释放所有着色器
    void releaseAll();

private:
    GPUDevice* gpu_device_;
    std::unordered_map<std::string, SDL_GPUShader*> shader_cache_;
    std::unordered_map<std::string, SDL_GPUComputePipeline*> compute_pipeline_cache_;

    SDL_GPUShader* createShaderFromBinary(
        SDL_GPUShaderStage stage,
        const ShaderSource& source,
        const char* entry_point
    );

    SDL_GPUShader* createShaderFromHLSL(
        SDL_GPUShaderStage stage,
        const char* hlsl_source,
        const char* entry_point,
        const char* debug_name
    );

    SDL_GPUComputePipeline* createComputePipelineFromHLSL(
        const char* hlsl_source,
        const ComputePipelineInfo& info,
        const char* entry_point,
        const char* debug_name
    );
};

} // namespace dong::sdl_backend
