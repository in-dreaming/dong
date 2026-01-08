#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <vector>
#include <cstring>

namespace dong::utils {

// ============================================================================
// 顶点结构
// ============================================================================

// 3D 顶点（位置 + 法线 + 颜色）
struct Vertex3D {
    float x, y, z;    // 位置
    float nx, ny, nz; // 法线
    float r, g, b;    // 颜色
};

// 纹理顶点（位置 + UV）
struct VertexUV {
    float x, y, z;
    float u, v;
};

// ============================================================================
// Uniform 结构
// ============================================================================

// 3D 场景 Uniform（MVP + Model + 光照）
struct Uniforms3D {
    float mvp[16];
    float model[16];
    float lightDir[4];
    float ambient[4];
};

// 纹理 Uniform（MVP + 颜色 + 高亮）
struct UniformsTextured {
    float mvp[16];
    float model[16];
    float color[4];
    float highlight[4]; // x=hovered, y=selected, z=unused, w=unused
};

// ============================================================================
// 着色器源码
// ============================================================================

// 3D 顶点着色器（位置 + 法线 + 颜色）
inline const char* kVertexShader3D = R"(
struct VSInput {
    float3 position : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 color : TEXCOORD2;
};

struct VSOutput {
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 color : TEXCOORD2;
};

cbuffer Uniforms : register(b0, space1) {
    float4x4 uMVP;
    float4x4 uModel;
    float4 uLightDir;
    float4 uAmbient;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(uMVP, float4(input.position, 1.0));
    output.worldPos = mul(uModel, float4(input.position, 1.0)).xyz;
    output.normal = mul((float3x3)uModel, input.normal);
    output.color = input.color;
    return output;
}
)";

// 3D 片段着色器（漫反射光照）
inline const char* kFragmentShader3D = R"(
cbuffer Uniforms : register(b0, space3) {
    float4x4 uMVP;
    float4x4 uModel;
    float4 uLightDir;
    float4 uAmbient;
};

struct PSInput {
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 color : TEXCOORD2;
};

float4 main(PSInput input) : SV_Target0 {
    float3 N = normalize(input.normal);
    float3 L = normalize(uLightDir.xyz);
    
    float diff = max(dot(N, L), 0.0);
    float3 diffuse = input.color * diff;
    float3 ambient = input.color * uAmbient.xyz;
    float3 result = ambient + diffuse * 0.8;
    return float4(result, 1.0);
}
)";

// 地面网格片段着色器（距离淡出）
inline const char* kFragmentShaderGrid = R"(
cbuffer Uniforms : register(b0, space3) {
    float4x4 uMVP;
    float4x4 uModel;
    float4 uLightDir;
    float4 uAmbient;
};

struct PSInput {
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 color : TEXCOORD2;
};

float4 main(PSInput input) : SV_Target0 {
    float dist = length(input.worldPos.xz);
    float fade = 1.0 - smoothstep(8.0, 15.0, dist);
    return float4(input.color, fade);
}
)";

// 纹理顶点着色器（位置 + UV）
inline const char* kVertexShaderTextured = R"(
struct VSInput {
    float3 position : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

cbuffer Uniforms : register(b0, space1) {
    float4x4 uMVP;
    float4x4 uModel;
    float4 uColor;
    float4 uHighlight;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(uMVP, float4(input.position, 1.0));
    output.uv = input.uv;
    output.worldPos = mul(uModel, float4(input.position, 1.0)).xyz;
    return output;
}
)";

// 纹理片段着色器
inline const char* kFragmentShaderTextured = R"(
Texture2D tex : register(t0, space2);
SamplerState texSampler : register(s0, space2);

cbuffer Uniforms : register(b0, space3) {
    float4x4 uMVP;
    float4x4 uModel;
    float4 uColor;
    float4 uHighlight;
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target0 {
    float4 texColor = tex.Sample(texSampler, input.uv);
    float4 result = texColor * uColor;
    
    // 悬停高亮边框
    if (uHighlight.x > 0.5) {
        float border = 0.02;
        if (input.uv.x < border || input.uv.x > 1.0 - border || 
            input.uv.y < border || input.uv.y > 1.0 - border) {
            result.rgb = lerp(result.rgb, float3(0.3, 0.6, 1.0), 0.7);
        }
    }
    
    // 选中高亮边框
    if (uHighlight.y > 0.5) {
        float border = 0.03;
        if (input.uv.x < border || input.uv.x > 1.0 - border || 
            input.uv.y < border || input.uv.y > 1.0 - border) {
            result.rgb = float3(1.0, 0.8, 0.2);
        }
    }
    
    return result;
}
)";

// ============================================================================
// 着色器编译
// ============================================================================

inline SDL_GPUShader* compileShader(SDL_GPUDevice* device, SDL_GPUShaderStage stage,
                                    const char* hlsl, const char* entry,
                                    int numSamplers = 0, int numUniformBuffers = 1) {
    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
    
    SDL_ShaderCross_HLSL_Info info{};
    info.source = hlsl;
    info.entrypoint = entry;
    info.shader_stage = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? 
                        SDL_SHADERCROSS_SHADERSTAGE_VERTEX : 
                        SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    
    auto createShader = [&](void* bytecode, size_t size, SDL_GPUShaderFormat format) -> SDL_GPUShader* {
        SDL_GPUShaderCreateInfo sci{};
        sci.code = (const Uint8*)bytecode;
        sci.code_size = size;
        sci.entrypoint = entry;
        sci.format = format;
        sci.stage = stage;
        sci.num_samplers = numSamplers;
        sci.num_uniform_buffers = numUniformBuffers;
        return SDL_CreateGPUShader(device, &sci);
    };
    
    if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        size_t size = 0;
        void* bytecode = SDL_ShaderCross_CompileSPIRVFromHLSL(&info, &size);
        if (!bytecode) {
            SDL_Log("Failed to compile HLSL to SPIRV: %s", SDL_GetError());
            return nullptr;
        }
        SDL_GPUShader* shader = createShader(bytecode, size, SDL_GPU_SHADERFORMAT_SPIRV);
        SDL_free(bytecode);
        return shader;
    }
    
    if (formats & SDL_GPU_SHADERFORMAT_DXIL) {
        size_t size = 0;
        void* bytecode = SDL_ShaderCross_CompileDXILFromHLSL(&info, &size);
        if (!bytecode) {
            SDL_Log("Failed to compile HLSL to DXIL: %s", SDL_GetError());
            return nullptr;
        }
        SDL_GPUShader* shader = createShader(bytecode, size, SDL_GPU_SHADERFORMAT_DXIL);
        SDL_free(bytecode);
        return shader;
    }
    
    SDL_Log("No supported shader format found");
    return nullptr;
}

// ============================================================================
// 几何体生成
// ============================================================================

// 生成立方体顶点（每个面不同颜色）
inline std::vector<Vertex3D> generateCube(float size = 1.0f) {
    std::vector<Vertex3D> verts;
    float s = size * 0.5f;
    
    struct Face { float nx, ny, nz; float r, g, b; };
    Face faces[6] = {
        { 0,  0,  1, 0.9f, 0.3f, 0.3f}, // 前 - 红
        { 0,  0, -1, 0.3f, 0.9f, 0.3f}, // 后 - 绿
        { 1,  0,  0, 0.3f, 0.3f, 0.9f}, // 右 - 蓝
        {-1,  0,  0, 0.9f, 0.9f, 0.3f}, // 左 - 黄
        { 0,  1,  0, 0.9f, 0.3f, 0.9f}, // 上 - 紫
        { 0, -1,  0, 0.3f, 0.9f, 0.9f}, // 下 - 青
    };
    
    float cubeVerts[8][3] = {
        {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s},
        {-s, -s, -s}, { s, -s, -s}, { s,  s, -s}, {-s,  s, -s},
    };
    
    int faceIndices[6][4] = {
        {0, 1, 2, 3}, {5, 4, 7, 6}, {1, 5, 6, 2},
        {4, 0, 3, 7}, {3, 2, 6, 7}, {4, 5, 1, 0},
    };
    
    for (int f = 0; f < 6; f++) {
        auto& face = faces[f];
        int* idx = faceIndices[f];
        auto addVert = [&](int i) {
            verts.push_back({
                cubeVerts[idx[i]][0], cubeVerts[idx[i]][1], cubeVerts[idx[i]][2],
                face.nx, face.ny, face.nz, face.r, face.g, face.b
            });
        };
        addVert(0); addVert(1); addVert(2);
        addVert(0); addVert(2); addVert(3);
    }
    return verts;
}

// 生成地面网格（LINE_LIST）
inline std::vector<Vertex3D> generateGrid(float size = 20.0f, int lines = 21, float color = 0.4f) {
    std::vector<Vertex3D> verts;
    for (int i = 0; i < lines; i++) {
        float t = -size / 2 + (size / (lines - 1)) * i;
        // X 方向线
        verts.push_back({-size / 2, 0, t, 0, 1, 0, color, color, color * 1.2f});
        verts.push_back({size / 2, 0, t, 0, 1, 0, color, color, color * 1.2f});
        // Z 方向线
        verts.push_back({t, 0, -size / 2, 0, 1, 0, color, color, color * 1.2f});
        verts.push_back({t, 0, size / 2, 0, 1, 0, color, color, color * 1.2f});
    }
    return verts;
}

// 生成四边形顶点（用于屏幕/面板）
inline std::vector<VertexUV> generateQuad(float width, float height, float z = 0) {
    float hw = width * 0.5f, hh = height * 0.5f;
    return {
        {-hw, -hh, z, 0, 1}, { hw, -hh, z, 1, 1}, { hw,  hh, z, 1, 0},
        {-hw, -hh, z, 0, 1}, { hw,  hh, z, 1, 0}, {-hw,  hh, z, 0, 0},
    };
}

// ============================================================================
// GPU 资源辅助
// ============================================================================

// 创建深度纹理
inline SDL_GPUTexture* createDepthTexture(SDL_GPUDevice* device, uint32_t w, uint32_t h) {
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    info.width = w;
    info.height = h;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    return SDL_CreateGPUTexture(device, &info);
}

// 创建渲染目标纹理
inline SDL_GPUTexture* createRenderTargetTexture(SDL_GPUDevice* device, uint32_t w, uint32_t h) {
    SDL_GPUTextureCreateInfo info{};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.width = w;
    info.height = h;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    return SDL_CreateGPUTexture(device, &info);
}

// 上传顶点数据到 GPU
template<typename T>
inline void uploadVertices(SDL_GPUDevice* device, SDL_GPUCopyPass* copyPass,
                          SDL_GPUBuffer* buffer, const std::vector<T>& data) {
    SDL_GPUTransferBufferCreateInfo tbInfo{};
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbInfo.size = (Uint32)(data.size() * sizeof(T));
    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbInfo);
    void* ptr = SDL_MapGPUTransferBuffer(device, tb, false);
    memcpy(ptr, data.data(), data.size() * sizeof(T));
    SDL_UnmapGPUTransferBuffer(device, tb);
    
    SDL_GPUTransferBufferLocation src{}; src.transfer_buffer = tb;
    SDL_GPUBufferRegion dst{}; dst.buffer = buffer; dst.size = tbInfo.size;
    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);
    SDL_ReleaseGPUTransferBuffer(device, tb);
}

} // namespace dong::utils
