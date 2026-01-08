/**
 * Dong Engine 3D Cube Demo
 * 
 * 在3D空间中渲染一个旋转的立方体
 * 
 * 控制说明：
 * - 右键按住 + 鼠标移动：控制视角
 * - WASD：前后左右移动
 * - Q/E 或 空格/Ctrl：上下移动
 * - Shift：加速移动
 * - ESC：退出
 */

#include <cstdio>
#include <cstdint>
#include <vector>
#include <chrono>
#include <cstring>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "utils/math3d.hpp"
#include "utils/camera.hpp"

using namespace dong::utils;

// 顶点着色器 (HLSL)
static const char* kVertexShader = R"(
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

// 片段着色器 (HLSL)
static const char* kFragmentShader = R"(
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
    
    // 漫反射光照
    float diff = max(dot(N, L), 0.0);
    float3 diffuse = input.color * diff;
    
    // 环境光
    float3 ambient = input.color * uAmbient.xyz;
    
    // 最终颜色
    float3 result = ambient + diffuse * 0.8;
    return float4(result, 1.0);
}
)";

// 地面网格着色器
static const char* kGridFragShader = R"(
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
    // 基于距离的淡出效果
    float dist = length(input.worldPos.xz);
    float fade = 1.0 - smoothstep(8.0, 15.0, dist);
    return float4(input.color, fade);
}
)";

// 顶点结构
struct Vertex {
    float x, y, z;    // 位置
    float nx, ny, nz; // 法线
    float r, g, b;    // 颜色
};

// Uniform 数据
struct Uniforms {
    float mvp[16];
    float model[16];
    float lightDir[4];
    float ambient[4];
};

// 编译着色器
static SDL_GPUShader* compileShader(SDL_GPUDevice* device, SDL_GPUShaderStage stage,
                                    const char* hlsl, const char* entry) {
    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
    
    SDL_ShaderCross_HLSL_Info info{};
    info.source = hlsl;
    info.entrypoint = entry;
    info.shader_stage = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? 
                        SDL_SHADERCROSS_SHADERSTAGE_VERTEX : 
                        SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    
    if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        size_t bytecode_size = 0;
        void* bytecode = SDL_ShaderCross_CompileSPIRVFromHLSL(&info, &bytecode_size);
        if (!bytecode) {
            SDL_Log("Failed to compile HLSL to SPIRV: %s", SDL_GetError());
            return nullptr;
        }
        
        SDL_GPUShaderCreateInfo sci{};
        sci.code = (const Uint8*)bytecode;
        sci.code_size = bytecode_size;
        sci.entrypoint = entry;
        sci.format = SDL_GPU_SHADERFORMAT_SPIRV;
        sci.stage = stage;
        sci.num_uniform_buffers = 1;
        
        SDL_GPUShader* shader = SDL_CreateGPUShader(device, &sci);
        SDL_free(bytecode);
        return shader;
    }
    
    if (formats & SDL_GPU_SHADERFORMAT_DXIL) {
        size_t bytecode_size = 0;
        void* bytecode = SDL_ShaderCross_CompileDXILFromHLSL(&info, &bytecode_size);
        if (!bytecode) {
            SDL_Log("Failed to compile HLSL to DXIL: %s", SDL_GetError());
            return nullptr;
        }
        
        SDL_GPUShaderCreateInfo sci{};
        sci.code = (const Uint8*)bytecode;
        sci.code_size = bytecode_size;
        sci.entrypoint = entry;
        sci.format = SDL_GPU_SHADERFORMAT_DXIL;
        sci.stage = stage;
        sci.num_uniform_buffers = 1;
        
        SDL_GPUShader* shader = SDL_CreateGPUShader(device, &sci);
        SDL_free(bytecode);
        return shader;
    }
    
    SDL_Log("No supported shader format found");
    return nullptr;
}

// 生成立方体顶点（每个面不同颜色）
static std::vector<Vertex> generateCube() {
    std::vector<Vertex> verts;
    
    // 面颜色
    struct Face { float nx, ny, nz; float r, g, b; };
    Face faces[6] = {
        { 0,  0,  1, 0.9f, 0.3f, 0.3f}, // 前面 - 红
        { 0,  0, -1, 0.3f, 0.9f, 0.3f}, // 后面 - 绿
        { 1,  0,  0, 0.3f, 0.3f, 0.9f}, // 右面 - 蓝
        {-1,  0,  0, 0.9f, 0.9f, 0.3f}, // 左面 - 黄
        { 0,  1,  0, 0.9f, 0.3f, 0.9f}, // 上面 - 紫
        { 0, -1,  0, 0.3f, 0.9f, 0.9f}, // 下面 - 青
    };
    
    // 立方体顶点位置（中心在原点，边长为1）
    float s = 0.5f;
    float cubeVerts[8][3] = {
        {-s, -s,  s}, { s, -s,  s}, { s,  s,  s}, {-s,  s,  s}, // 前面
        {-s, -s, -s}, { s, -s, -s}, { s,  s, -s}, {-s,  s, -s}, // 后面
    };
    
    // 面的顶点索引
    int faceIndices[6][4] = {
        {0, 1, 2, 3}, // 前
        {5, 4, 7, 6}, // 后
        {1, 5, 6, 2}, // 右
        {4, 0, 3, 7}, // 左
        {3, 2, 6, 7}, // 上
        {4, 5, 1, 0}, // 下
    };
    
    for (int f = 0; f < 6; f++) {
        auto& face = faces[f];
        int* idx = faceIndices[f];
        
        // 两个三角形
        auto addVert = [&](int i) {
            verts.push_back({
                cubeVerts[idx[i]][0], cubeVerts[idx[i]][1], cubeVerts[idx[i]][2],
                face.nx, face.ny, face.nz,
                face.r, face.g, face.b
            });
        };
        
        addVert(0); addVert(1); addVert(2);
        addVert(0); addVert(2); addVert(3);
    }
    
    return verts;
}

// 生成地面网格
static std::vector<Vertex> generateGrid() {
    std::vector<Vertex> verts;
    const float gridSize = 20.0f;
    const int gridLines = 21;
    float color = 0.4f;
    
    for (int i = 0; i < gridLines; i++) {
        float t = -gridSize / 2 + (gridSize / (gridLines - 1)) * i;
        // X 方向线
        verts.push_back({-gridSize / 2, 0, t, 0, 1, 0, color, color, color * 1.2f});
        verts.push_back({gridSize / 2, 0, t, 0, 1, 0, color, color, color * 1.2f});
        // Z 方向线
        verts.push_back({t, 0, -gridSize / 2, 0, 1, 0, color, color, color * 1.2f});
        verts.push_back({t, 0, gridSize / 2, 0, 1, 0, color, color, color * 1.2f});
    }
    
    return verts;
}

int main() {
    SDL_Log("=== Dong 3D Cube Demo ===");
    SDL_Log("Controls: RMB+Mouse=Look, WASD=Move, Q/E=Up/Down, Shift=Sprint, ESC=Exit");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    
    if (!SDL_ShaderCross_Init()) {
        SDL_Log("SDL_ShaderCross_Init failed: %s", SDL_GetError());
        return 1;
    }

    const int WIN_W = 1280, WIN_H = 720;
    SDL_Window* window = SDL_CreateWindow("Dong 3D Cube Demo", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        false, nullptr);
    if (!device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        return 1;
    }

    // 编译着色器
    SDL_GPUShader* vs = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShader, "main");
    SDL_GPUShader* fsCube = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShader, "main");
    SDL_GPUShader* fsGrid = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kGridFragShader, "main");
    
    if (!vs || !fsCube || !fsGrid) {
        SDL_Log("Failed to compile shaders");
        return 1;
    }

    // 顶点布局
    SDL_GPUVertexBufferDescription vbDesc{};
    vbDesc.slot = 0;
    vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDesc.pitch = sizeof(Vertex);

    SDL_GPUVertexAttribute attrs[3] = {};
    attrs[0].buffer_slot = 0; attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs[0].location = 0; attrs[0].offset = 0;
    attrs[1].buffer_slot = 0; attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs[1].location = 1; attrs[1].offset = sizeof(float) * 3;
    attrs[2].buffer_slot = 0; attrs[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs[2].location = 2; attrs[2].offset = sizeof(float) * 6;

    // 立方体管线
    SDL_GPUColorTargetDescription colorDesc{};
    colorDesc.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    colorDesc.blend_state.enable_blend = false;

    SDL_GPUGraphicsPipelineCreateInfo pipeInfo{};
    pipeInfo.vertex_shader = vs;
    pipeInfo.fragment_shader = fsCube;
    pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeInfo.target_info.num_color_targets = 1;
    pipeInfo.target_info.color_target_descriptions = &colorDesc;
    pipeInfo.target_info.has_depth_stencil_target = true;
    pipeInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    pipeInfo.depth_stencil_state.enable_depth_test = true;
    pipeInfo.depth_stencil_state.enable_depth_write = true;
    pipeInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    pipeInfo.vertex_input_state.num_vertex_buffers = 1;
    pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
    pipeInfo.vertex_input_state.num_vertex_attributes = 3;
    pipeInfo.vertex_input_state.vertex_attributes = attrs;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipeInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    SDL_GPUGraphicsPipeline* cubePipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
    if (!cubePipeline) {
        SDL_Log("Failed to create cube pipeline: %s", SDL_GetError());
        return 1;
    }

    // 网格管线（LINE_LIST，带混合）
    colorDesc.blend_state.enable_blend = true;
    colorDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    colorDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    colorDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    colorDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    
    pipeInfo.fragment_shader = fsGrid;
    pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    
    SDL_GPUGraphicsPipeline* gridPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
    if (!gridPipeline) {
        SDL_Log("Failed to create grid pipeline: %s", SDL_GetError());
        return 1;
    }

    // 生成几何体
    auto cubeVerts = generateCube();
    auto gridVerts = generateGrid();
    
    // 创建顶点缓冲区
    SDL_GPUBufferCreateInfo vbInfo{};
    vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    
    vbInfo.size = (Uint32)(cubeVerts.size() * sizeof(Vertex));
    SDL_GPUBuffer* cubeVB = SDL_CreateGPUBuffer(device, &vbInfo);
    
    vbInfo.size = (Uint32)(gridVerts.size() * sizeof(Vertex));
    SDL_GPUBuffer* gridVB = SDL_CreateGPUBuffer(device, &vbInfo);

    // 相机
    FPSCamera camera;
    camera.position = Vec3{3.0f, 2.0f, 5.0f};
    camera.yaw = -2.5f;
    camera.pitch = -0.3f;

    InputState input;
    
    SDL_GPUTexture* depthTexture = nullptr;
    uint32_t depthW = 0, depthH = 0;
    
    float cubeRotation = 0.0f;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;
    int winW = WIN_W, winH = WIN_H;

    while (running) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        
        cubeRotation += dt * 0.5f;

        input.resetFrameState();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            input.handleEvent(event);
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                running = false;
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &winW, &winH);
            }
        }

        camera.update(dt, input.keys, input.right_mouse_down, input.mouse_delta_x, input.mouse_delta_y);
        SDL_SetWindowRelativeMouseMode(window, input.right_mouse_down);

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        if (!cmd) {
            SDL_Delay(16);
            continue;
        }

        // Copy pass - 上传顶点数据
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
        
        // 上传立方体顶点
        {
            SDL_GPUTransferBufferCreateInfo tbInfo{};
            tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            tbInfo.size = (Uint32)(cubeVerts.size() * sizeof(Vertex));
            SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbInfo);
            void* ptr = SDL_MapGPUTransferBuffer(device, tb, false);
            memcpy(ptr, cubeVerts.data(), cubeVerts.size() * sizeof(Vertex));
            SDL_UnmapGPUTransferBuffer(device, tb);
            
            SDL_GPUTransferBufferLocation src{}; src.transfer_buffer = tb;
            SDL_GPUBufferRegion dst{}; dst.buffer = cubeVB; dst.size = tbInfo.size;
            SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);
            SDL_ReleaseGPUTransferBuffer(device, tb);
        }
        
        // 上传网格顶点
        {
            SDL_GPUTransferBufferCreateInfo tbInfo{};
            tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            tbInfo.size = (Uint32)(gridVerts.size() * sizeof(Vertex));
            SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbInfo);
            void* ptr = SDL_MapGPUTransferBuffer(device, tb, false);
            memcpy(ptr, gridVerts.data(), gridVerts.size() * sizeof(Vertex));
            SDL_UnmapGPUTransferBuffer(device, tb);
            
            SDL_GPUTransferBufferLocation src{}; src.transfer_buffer = tb;
            SDL_GPUBufferRegion dst{}; dst.buffer = gridVB; dst.size = tbInfo.size;
            SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);
            SDL_ReleaseGPUTransferBuffer(device, tb);
        }
        
        SDL_EndGPUCopyPass(copyPass);

        // 获取 swapchain
        SDL_GPUTexture* swapchain = nullptr;
        Uint32 sw, sh;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain, &sw, &sh) || !swapchain) {
            SDL_SubmitGPUCommandBuffer(cmd);
            SDL_Delay(16);
            continue;
        }

        // 深度纹理
        if (!depthTexture || depthW != sw || depthH != sh) {
            if (depthTexture) SDL_ReleaseGPUTexture(device, depthTexture);
            SDL_GPUTextureCreateInfo depthInfo{};
            depthInfo.type = SDL_GPU_TEXTURETYPE_2D;
            depthInfo.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
            depthInfo.width = sw;
            depthInfo.height = sh;
            depthInfo.layer_count_or_depth = 1;
            depthInfo.num_levels = 1;
            depthInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
            depthInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
            depthTexture = SDL_CreateGPUTexture(device, &depthInfo);
            depthW = sw;
            depthH = sh;
        }

        // 渲染
        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = swapchain;
        colorTarget.clear_color = SDL_FColor{0.1f, 0.1f, 0.15f, 1.0f};
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPUDepthStencilTargetInfo depthTarget{};
        depthTarget.texture = depthTexture;
        depthTarget.clear_depth = 1.0f;
        depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, &depthTarget);

        float aspect = (float)sw / (float)sh;
        Mat4 view = camera.getViewMatrix();
        Mat4 proj = camera.getProjectionMatrix(aspect);
        Mat4 vp = proj * view;

        // 绘制网格
        SDL_BindGPUGraphicsPipeline(pass, gridPipeline);
        SDL_GPUBufferBinding gridBinding{gridVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &gridBinding, 1);
        
        Uniforms gridUniforms{};
        Mat4 identity = Mat4::identity();
        memcpy(gridUniforms.mvp, vp.m, sizeof(float) * 16);
        memcpy(gridUniforms.model, identity.m, sizeof(float) * 16);
        gridUniforms.lightDir[0] = 0.5f; gridUniforms.lightDir[1] = 1.0f; gridUniforms.lightDir[2] = 0.3f;
        gridUniforms.ambient[0] = 0.3f; gridUniforms.ambient[1] = 0.3f; gridUniforms.ambient[2] = 0.3f;
        
        SDL_PushGPUVertexUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        SDL_PushGPUFragmentUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        SDL_DrawGPUPrimitives(pass, (Uint32)gridVerts.size(), 1, 0, 0);

        // 绘制立方体
        SDL_BindGPUGraphicsPipeline(pass, cubePipeline);
        SDL_GPUBufferBinding cubeBinding{cubeVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &cubeBinding, 1);
        
        // 立方体变换：位置 + 旋转
        Mat4 cubeModel = Mat4::translate(Vec3{0, 0.5f, 0}) * Mat4::rotateY(cubeRotation);
        Mat4 cubeMVP = vp * cubeModel;
        
        Uniforms cubeUniforms{};
        memcpy(cubeUniforms.mvp, cubeMVP.m, sizeof(float) * 16);
        memcpy(cubeUniforms.model, cubeModel.m, sizeof(float) * 16);
        cubeUniforms.lightDir[0] = 0.5f; cubeUniforms.lightDir[1] = 1.0f; cubeUniforms.lightDir[2] = 0.3f;
        cubeUniforms.ambient[0] = 0.2f; cubeUniforms.ambient[1] = 0.2f; cubeUniforms.ambient[2] = 0.25f;
        
        SDL_PushGPUVertexUniformData(cmd, 0, &cubeUniforms, sizeof(cubeUniforms));
        SDL_PushGPUFragmentUniformData(cmd, 0, &cubeUniforms, sizeof(cubeUniforms));
        SDL_DrawGPUPrimitives(pass, (Uint32)cubeVerts.size(), 1, 0, 0);

        SDL_EndGPURenderPass(pass);
        SDL_SubmitGPUCommandBuffer(cmd);
        
        SDL_Delay(16);
    }

    // 清理
    if (depthTexture) SDL_ReleaseGPUTexture(device, depthTexture);
    SDL_ReleaseGPUBuffer(device, cubeVB);
    SDL_ReleaseGPUBuffer(device, gridVB);
    SDL_ReleaseGPUGraphicsPipeline(device, cubePipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, gridPipeline);
    SDL_ReleaseGPUShader(device, vs);
    SDL_ReleaseGPUShader(device, fsCube);
    SDL_ReleaseGPUShader(device, fsGrid);
    
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_ShaderCross_Quit();
    SDL_Quit();

    SDL_Log("=== Demo Complete ===");
    return 0;
}
