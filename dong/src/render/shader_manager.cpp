#include "shader_manager.hpp"
#include "gpu_device.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3_shadercross/SDL_shadercross.h>

namespace dong::render {

static bool g_shadercross_initialized = false;

ShaderManager::ShaderManager(GPUDevice* gpu_device)
    : gpu_device_(gpu_device) {
    if (!g_shadercross_initialized) {
        if (!SDL_ShaderCross_Init()) {
            SDL_Log("Failed to initialize SDL_shadercross: %s", SDL_GetError());
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

SDL_GPUShader* ShaderManager::loadShaderFromHLSL(
    const std::string& name,
    SDL_GPUShaderStage stage,
    const char* hlsl_source,
    const char* entry_point) {

    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPU device not initialized");
        return nullptr;
    }

    if (!hlsl_source) {
        SDL_Log("HLSL source is null for shader '%s'", name.c_str());
        return nullptr;
    }

    auto it = shader_cache_.find(name);
    if (it != shader_cache_.end()) {
        return it->second;
    }

    SDL_GPUShader* shader = createShaderFromHLSL(stage, hlsl_source, entry_point, name.c_str());
    if (shader) {
        shader_cache_[name] = shader;
        SDL_Log("HLSL shader '%s' compiled successfully", name.c_str());
    } else {
        SDL_Log("Failed to compile HLSL shader '%s'", name.c_str());
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
    // 检查设备是否有效
    if (!gpu_device_) {
        shader_cache_.clear();
        return;
    }

    // 检查设备是否已初始化
    if (!gpu_device_->isInitialized()) {
        // 设备未初始化，直接清空缓存
        shader_cache_.clear();
        return;
    }

    // 关键：如果设备是外部管理的（如 SDL3Window），当外部销毁设备时，
    // SDL 会自动清理所有 shader。我们不应该尝试再次释放它们，否则会导致崩溃。
    // 直接清空缓存即可。
    if (!gpu_device_->ownsDevice()) {
        // 外部管理的设备，不尝试释放 shader（SDL 会自动清理）
        shader_cache_.clear();
        return;
    }

    // 获取设备句柄并验证
    SDL_GPUDevice* dev = gpu_device_->getHandle();
    if (!dev) {
        // 设备句柄无效，直接清空缓存
        shader_cache_.clear();
        return;
    }

    // 只有当我们拥有设备时，才尝试释放 shader
    // 安全地释放所有 shader
    for (auto& pair : shader_cache_) {
        if (pair.second) {
            // 检查 shader 指针是否看起来有效（不是明显无效的地址）
            uintptr_t shader_addr = reinterpret_cast<uintptr_t>(pair.second);
            if (shader_addr > 0x1000) {  // 基本有效性检查
                SDL_ReleaseGPUShader(dev, pair.second);
            }
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
        SDL_Log("Failed to create GPU shader: %s", SDL_GetError());
    }

    return shader;
}

SDL_GPUShader* ShaderManager::createShaderFromHLSL(
    SDL_GPUShaderStage stage,
    const char* hlsl_source,
    const char* entry_point,
    const char* debug_name) {

    if (!hlsl_source) {
        SDL_Log("Invalid HLSL source for shader '%s'", debug_name ? debug_name : "<unnamed>");
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
        SDL_Log("Unsupported shader stage for HLSL: %d", (int)stage);
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
        SDL_Log("Failed to compile SPIR-V from HLSL for shader '%s': %s", debug_name ? debug_name : "<unnamed>", SDL_GetError());
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
        SDL_Log("Failed to reflect SPIR-V for shader '%s': %s", debug_name ? debug_name : "<unnamed>", SDL_GetError());
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
        SDL_Log("Failed to compile GPU shader from SPIR-V for '%s': %s", debug_name ? debug_name : "<unnamed>", SDL_GetError());
    }

    SDL_free(metadata);
    SDL_free(spirv_data);
    SDL_DestroyProperties(props);

    return shader;
}

} // namespace dong::render
