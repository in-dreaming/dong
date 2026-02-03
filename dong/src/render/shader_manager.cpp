#include "shader_manager.hpp"
#include "gpu_device.hpp"
#include "../core/log.h"
#include <SDL3_shadercross/SDL_shadercross.h>
#include <mutex>
#include <fstream>
#include <sstream>


namespace dong::render {

static bool g_shadercross_initialized = false;

// 全局 shader 缓存：按 (device, shader_name) 缓存编译后的 shader
// 这样多个 View 共享同一个 GPU device 时可以复用 shader
static std::mutex g_shader_cache_mutex;
static std::unordered_map<SDL_GPUDevice*, std::unordered_map<std::string, SDL_GPUShader*>> g_global_shader_cache;

ShaderManager::ShaderManager(GPUDevice* gpu_device)
    : gpu_device_(gpu_device) {
    if (!g_shadercross_initialized) {
        if (!SDL_ShaderCross_Init()) {
            DONG_LOG_ERROR("Failed to initialize SDL_shadercross: %s", SDL_GetError());
        } else {
            g_shadercross_initialized = true;
        }
    }
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
        DONG_LOG_ERROR("GPU device not initialized");
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
        DONG_LOG_INFO("Shader '%s' loaded successfully", name.c_str());
    } else {
        DONG_LOG_ERROR("Failed to load shader '%s'", name.c_str());
    }

    return shader;
}

SDL_GPUShader* ShaderManager::loadShaderFromHLSL(
    const std::string& name,
    SDL_GPUShaderStage stage,
    const char* hlsl_source,
    const char* entry_point) {

    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_ERROR("GPU device not initialized");
        return nullptr;
    }

    if (!hlsl_source) {
        DONG_LOG_ERROR("HLSL source is null for shader '%s'", name.c_str());
        return nullptr;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    
    // 先检查全局缓存
    {
        std::lock_guard<std::mutex> lock(g_shader_cache_mutex);
        auto dev_it = g_global_shader_cache.find(dev);
        if (dev_it != g_global_shader_cache.end()) {
            auto shader_it = dev_it->second.find(name);
            if (shader_it != dev_it->second.end()) {
                // 全局缓存命中，也更新本地缓存
                shader_cache_[name] = shader_it->second;
                return shader_it->second;
            }
        }
    }

    // 检查本地缓存
    auto it = shader_cache_.find(name);
    if (it != shader_cache_.end()) {
        return it->second;
    }

    // 编译新 shader
    SDL_GPUShader* shader = createShaderFromHLSL(stage, hlsl_source, entry_point, name.c_str());
    if (shader) {
        shader_cache_[name] = shader;

        // 添加到全局缓存
        {
            std::lock_guard<std::mutex> lock(g_shader_cache_mutex);
            g_global_shader_cache[dev][name] = shader;
        }

        DONG_LOG_INFO("HLSL shader '%s' compiled successfully", name.c_str());
    } else {
        DONG_LOG_ERROR("Failed to compile HLSL shader '%s'", name.c_str());
    }

    return shader;
}

SDL_GPUShader* ShaderManager::loadShaderFromHLSLFile(
    const std::string& name,
    SDL_GPUShaderStage stage,
    const char* hlsl_file_path,
    const char* entry_point) {

    if (!hlsl_file_path || hlsl_file_path[0] == '\0') {
        DONG_LOG_ERROR("HLSL file path is empty for shader '%s'", name.c_str());
        return nullptr;
    }

    std::ifstream in(hlsl_file_path, std::ios::in | std::ios::binary);
    if (!in) {
        DONG_LOG_ERROR("Failed to open HLSL file '%s' for shader '%s'", hlsl_file_path, name.c_str());
        return nullptr;
    }

    std::ostringstream ss;
    ss << in.rdbuf();
    std::string source = ss.str();
    if (source.empty()) {
        DONG_LOG_ERROR("HLSL file '%s' is empty for shader '%s'", hlsl_file_path, name.c_str());
        return nullptr;
    }

    // 直接复用现有的 HLSL 编译路径（包含全局缓存）
    return loadShaderFromHLSL(name, stage, source.c_str(), entry_point);
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
    // 只清空本地缓存引用，不释放 shader（由全局缓存管理）
    // 全局缓存的 shader 会在设备销毁时由 SDL 自动清理
    shader_cache_.clear();
}

SDL_GPUShader* ShaderManager::createShaderFromBinary(
    SDL_GPUShaderStage stage,
    const ShaderSource& source,
    const char* entry_point) {
    
    if (!source.data || source.size == 0) {
        DONG_LOG_ERROR("Invalid shader source");
        return nullptr;
    }

    SDL_GPUShaderCreateInfo shader_info{};
    shader_info.code_size = source.size;
    shader_info.code = reinterpret_cast<const Uint8*>(source.data);
    shader_info.entrypoint = entry_point;
    shader_info.format = gpu_device_->getShaderFormat();
    shader_info.stage = stage;
    shader_info.num_samplers = 0;
    shader_info.num_storage_textures = 0;
    shader_info.num_storage_buffers = 0;
    shader_info.num_uniform_buffers = 0;

    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu_device_->getHandle(), &shader_info);
    if (!shader) {
        DONG_LOG_ERROR("Failed to create GPU shader: %s", SDL_GetError());
    }

    return shader;
}

SDL_GPUShader* ShaderManager::createShaderFromHLSL(
    SDL_GPUShaderStage stage,
    const char* hlsl_source,
    const char* entry_point,
    const char* debug_name) {

    if (!hlsl_source) {
        DONG_LOG_ERROR("Invalid HLSL source for shader '%s'", debug_name ? debug_name : "<unnamed>");
        return nullptr;
    }

    SDL_ShaderCross_ShaderStage sc_stage;
    switch (stage) {
    case SDL_GPU_SHADERSTAGE_VERTEX:
        sc_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        break;
    case SDL_GPU_SHADERSTAGE_FRAGMENT:
        sc_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        break;
    default:
        DONG_LOG_ERROR("Unsupported shader stage for HLSL: %d", (int)stage);
        return nullptr;
    }

    SDL_ShaderCross_HLSL_Info hlsl_info{};
    hlsl_info.source = hlsl_source;
    hlsl_info.entrypoint = entry_point;
    hlsl_info.include_dir = nullptr;
    hlsl_info.defines = nullptr;
    hlsl_info.shader_stage = sc_stage;
    hlsl_info.props = 0;

    size_t spirv_size = 0;
    void* spirv_data = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_size);
    if (!spirv_data || spirv_size == 0) {
        DONG_LOG_ERROR("Failed to compile SPIR-V from HLSL for shader '%s': %s", debug_name ? debug_name : "<unnamed>", SDL_GetError());
        if (spirv_data) {
            SDL_free(spirv_data);
        }
        return nullptr;
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    if (debug_name) {
        SDL_SetStringProperty(props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_NAME_STRING, debug_name);
    }

    SDL_ShaderCross_GraphicsShaderMetadata* metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(
        (const Uint8*)spirv_data,
        spirv_size,
        props
    );
    if (!metadata) {
        DONG_LOG_ERROR("Failed to reflect SPIR-V for shader '%s': %s", debug_name ? debug_name : "<unnamed>", SDL_GetError());
        SDL_free(spirv_data);
        SDL_DestroyProperties(props);
        return nullptr;
    }

    SDL_ShaderCross_SPIRV_Info spirv_info{};
    spirv_info.bytecode = (const Uint8*)spirv_data;
    spirv_info.bytecode_size = spirv_size;
    spirv_info.entrypoint = entry_point;
    spirv_info.shader_stage = sc_stage;
    spirv_info.props = props;

    SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
        gpu_device_->getHandle(),
        &spirv_info,
        &metadata->resource_info,
        props
    );

    if (!shader) {
        DONG_LOG_ERROR("Failed to compile GPU shader from SPIR-V for '%s': %s", debug_name ? debug_name : "<unnamed>", SDL_GetError());
    }

    SDL_free(metadata);
    SDL_free(spirv_data);
    SDL_DestroyProperties(props);

    return shader;
}

} // namespace dong::render
