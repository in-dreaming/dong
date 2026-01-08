/**
 * Dong Engine 3D Screen Demo
 * 
 * 在3D空间中渲染立方体和两个"屏幕"，屏幕内容来自实时渲染的RT
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
#include "utils/gpu_utils.hpp"

using namespace dong::utils;

// RT 场景片段着色器（动画颜色效果）
static const char* kRTFragmentShader = R"(
cbuffer Uniforms : register(b0, space3) {
    float4x4 uMVP;
    float4x4 uModel;
    float4 uLightDir;
    float4 uAmbient;  // w = time
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
    
    float time = uAmbient.w;
    float3 animColor = input.color;
    animColor.r *= 0.5 + 0.5 * sin(time * 2.0);
    animColor.g *= 0.5 + 0.5 * sin(time * 2.0 + 2.094);
    animColor.b *= 0.5 + 0.5 * sin(time * 2.0 + 4.189);
    
    float3 ambient = animColor * uAmbient.xyz;
    float3 result = ambient + animColor * diff * 0.8;
    return float4(result, 1.0);
}
)";

// 屏幕信息
struct Screen3D {
    SDL_GPUTexture* colorTexture = nullptr;
    SDL_GPUTexture* depthTexture = nullptr;
    SDL_GPUBuffer* quadVB = nullptr;
    Vec3 position;
    float yaw = 0;
    float width = 2.0f;
    float height = 1.5f;
    uint32_t rtWidth = 512;
    uint32_t rtHeight = 384;
    float cubeRotation = 0.0f;
    float cubeRotationSpeed = 1.0f;
    float timeOffset = 0.0f;
    bool hovered = false;
};

// 获取屏幕的世界变换矩阵
Mat4 getScreenTransform(const Screen3D& screen) {
    return Mat4::translate(screen.position) * Mat4::rotateY(screen.yaw);
}

// 获取屏幕的 Quad3D（用于射线检测）
Quad3D getScreenQuad(const Screen3D& screen) {
    Vec3 right = Vec3{std::cos(screen.yaw), 0, -std::sin(screen.yaw)};
    Vec3 up = Vec3{0, 1, 0};
    return Quad3D(screen.position, right, up, screen.width, screen.height);
}

int main() {
    SDL_Log("=== Dong 3D Screen Demo ===");
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
    SDL_Window* window = SDL_CreateWindow("Dong 3D Screen Demo", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
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
    SDL_GPUShader* vs3d = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShader3D, "main");
    SDL_GPUShader* fsCube = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShader3D, "main");
    SDL_GPUShader* fsGrid = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderGrid, "main");
    SDL_GPUShader* fsRT = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kRTFragmentShader, "main");
    SDL_GPUShader* vsTextured = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShaderTextured, "main");
    SDL_GPUShader* fsTextured = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderTextured, "main", 1);
    
    if (!vs3d || !fsCube || !fsGrid || !fsRT || !vsTextured || !fsTextured) {
        SDL_Log("Failed to compile shaders");
        return 1;
    }

    // 3D 顶点布局
    SDL_GPUVertexBufferDescription vbDesc3D{};
    vbDesc3D.slot = 0;
    vbDesc3D.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDesc3D.pitch = sizeof(Vertex3D);

    SDL_GPUVertexAttribute attrs3D[3] = {};
    attrs3D[0].buffer_slot = 0; attrs3D[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs3D[0].location = 0; attrs3D[0].offset = 0;
    attrs3D[1].buffer_slot = 0; attrs3D[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs3D[1].location = 1; attrs3D[1].offset = sizeof(float) * 3;
    attrs3D[2].buffer_slot = 0; attrs3D[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs3D[2].location = 2; attrs3D[2].offset = sizeof(float) * 6;

    // 纹理顶点布局
    SDL_GPUVertexBufferDescription vbDescUV{};
    vbDescUV.slot = 0;
    vbDescUV.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDescUV.pitch = sizeof(VertexUV);

    SDL_GPUVertexAttribute attrsUV[2] = {};
    attrsUV[0].buffer_slot = 0; attrsUV[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrsUV[0].location = 0; attrsUV[0].offset = 0;
    attrsUV[1].buffer_slot = 0; attrsUV[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; attrsUV[1].location = 1; attrsUV[1].offset = sizeof(float) * 3;

    // 立方体管线
    SDL_GPUColorTargetDescription colorDesc{};
    colorDesc.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    colorDesc.blend_state.enable_blend = false;

    SDL_GPUGraphicsPipelineCreateInfo pipeInfo{};
    pipeInfo.vertex_shader = vs3d;
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
    pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDesc3D;
    pipeInfo.vertex_input_state.num_vertex_attributes = 3;
    pipeInfo.vertex_input_state.vertex_attributes = attrs3D;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipeInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    SDL_GPUGraphicsPipeline* cubePipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
    
    // RT 场景管线
    pipeInfo.fragment_shader = fsRT;
    SDL_GPUGraphicsPipeline* rtCubePipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

    // 网格管线
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

    // 屏幕纹理管线
    colorDesc.blend_state.enable_blend = true;
    pipeInfo.vertex_shader = vsTextured;
    pipeInfo.fragment_shader = fsTextured;
    pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDescUV;
    pipeInfo.vertex_input_state.num_vertex_attributes = 2;
    pipeInfo.vertex_input_state.vertex_attributes = attrsUV;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    
    SDL_GPUGraphicsPipeline* screenPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

    if (!cubePipeline || !rtCubePipeline || !gridPipeline || !screenPipeline) {
        SDL_Log("Failed to create pipelines");
        return 1;
    }

    // 创建采样器
    SDL_GPUSamplerCreateInfo samplerInfo{};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &samplerInfo);

    // 生成几何体
    auto cubeVerts = generateCube();
    auto gridVerts = generateGrid();
    
    // 创建顶点缓冲区
    SDL_GPUBufferCreateInfo vbInfo{};
    vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    
    vbInfo.size = (Uint32)(cubeVerts.size() * sizeof(Vertex3D));
    SDL_GPUBuffer* cubeVB = SDL_CreateGPUBuffer(device, &vbInfo);
    
    vbInfo.size = (Uint32)(gridVerts.size() * sizeof(Vertex3D));
    SDL_GPUBuffer* gridVB = SDL_CreateGPUBuffer(device, &vbInfo);

    // 创建两个屏幕
    Screen3D screens[2];
    
    // 屏幕1：左侧
    screens[0].position = Vec3{-3.0f, 1.5f, -2.0f};
    screens[0].yaw = 0.4f;
    screens[0].width = 2.5f;
    screens[0].height = 1.875f;
    screens[0].rtWidth = 640;
    screens[0].rtHeight = 480;
    screens[0].cubeRotationSpeed = 1.2f;
    screens[0].timeOffset = 0.0f;
    
    // 屏幕2：右侧
    screens[1].position = Vec3{3.0f, 1.5f, -2.0f};
    screens[1].yaw = -0.4f;
    screens[1].width = 2.5f;
    screens[1].height = 1.875f;
    screens[1].rtWidth = 640;
    screens[1].rtHeight = 480;
    screens[1].cubeRotationSpeed = 0.8f;
    screens[1].timeOffset = 3.14159f;

    // 为每个屏幕创建 RT 和顶点缓冲
    for (int i = 0; i < 2; i++) {
        screens[i].colorTexture = createRenderTargetTexture(device, screens[i].rtWidth, screens[i].rtHeight);
        screens[i].depthTexture = createDepthTexture(device, screens[i].rtWidth, screens[i].rtHeight);
        
        vbInfo.size = sizeof(VertexUV) * 6;
        screens[i].quadVB = SDL_CreateGPUBuffer(device, &vbInfo);
        
        if (!screens[i].colorTexture || !screens[i].depthTexture || !screens[i].quadVB) {
            SDL_Log("Failed to create screen %d resources", i);
            return 1;
        }
    }

    // 相机
    FPSCamera camera;
    camera.position = Vec3{0.0f, 2.0f, 6.0f};
    camera.yaw = -3.14159f;
    camera.pitch = -0.2f;

    InputState input;
    
    SDL_GPUTexture* depthTexture = nullptr;
    uint32_t depthW = 0, depthH = 0;
    
    float mainCubeRotation = 0.0f;
    float totalTime = 0.0f;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;
    int winW = WIN_W, winH = WIN_H;

    while (running) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        
        mainCubeRotation += dt * 0.5f;
        totalTime += dt;
        
        for (int i = 0; i < 2; i++) {
            screens[i].cubeRotation += dt * screens[i].cubeRotationSpeed;
        }

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

        // 射线检测屏幕悬停
        Ray hoverRay = camera.pixelToRay(input.mouse_x, input.mouse_y, winW, winH);
        for (int i = 0; i < 2; i++) {
            Quad3D quad = getScreenQuad(screens[i]);
            Vec2 uv = quad.intersect(hoverRay);
            screens[i].hovered = (uv.x >= 0);
        }

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        if (!cmd) {
            SDL_Delay(16);
            continue;
        }

        // Copy pass - 上传顶点数据
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
        uploadVertices(device, copyPass, cubeVB, cubeVerts);
        uploadVertices(device, copyPass, gridVB, gridVerts);
        
        // 上传屏幕四边形顶点
        for (int i = 0; i < 2; i++) {
            auto quadVerts = generateQuad(screens[i].width, screens[i].height);
            uploadVertices(device, copyPass, screens[i].quadVB, quadVerts);
        }
        SDL_EndGPUCopyPass(copyPass);

        // ========== 渲染到 RT ==========
        for (int i = 0; i < 2; i++) {
            SDL_GPUColorTargetInfo rtColorTarget{};
            rtColorTarget.texture = screens[i].colorTexture;
            rtColorTarget.clear_color = SDL_FColor{0.05f, 0.05f, 0.1f, 1.0f};
            rtColorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
            rtColorTarget.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPUDepthStencilTargetInfo rtDepthTarget{};
            rtDepthTarget.texture = screens[i].depthTexture;
            rtDepthTarget.clear_depth = 1.0f;
            rtDepthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
            rtDepthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;

            SDL_GPURenderPass* rtPass = SDL_BeginGPURenderPass(cmd, &rtColorTarget, 1, &rtDepthTarget);
            
            // RT 相机（固定视角看立方体）
            float rtAspect = (float)screens[i].rtWidth / screens[i].rtHeight;
            Mat4 rtView = Mat4::lookAt(Vec3{0, 1.5f, 3.0f}, Vec3{0, 0, 0}, Vec3{0, 1, 0});
            Mat4 rtProj = Mat4::perspective(radians(60.0f), rtAspect, 0.1f, 100.0f);
            Mat4 rtVP = rtProj * rtView;
            
            // 绘制旋转立方体
            SDL_BindGPUGraphicsPipeline(rtPass, rtCubePipeline);
            SDL_GPUBufferBinding cubeBinding{cubeVB, 0};
            SDL_BindGPUVertexBuffers(rtPass, 0, &cubeBinding, 1);
            
            Mat4 cubeModel = Mat4::rotateY(screens[i].cubeRotation) * Mat4::rotateX(screens[i].cubeRotation * 0.7f);
            Mat4 cubeMVP = rtVP * cubeModel;
            
            Uniforms3D cubeUniforms{};
            memcpy(cubeUniforms.mvp, cubeMVP.m, sizeof(float) * 16);
            memcpy(cubeUniforms.model, cubeModel.m, sizeof(float) * 16);
            cubeUniforms.lightDir[0] = 0.5f; cubeUniforms.lightDir[1] = 1.0f; cubeUniforms.lightDir[2] = 0.3f;
            cubeUniforms.ambient[0] = 0.3f; cubeUniforms.ambient[1] = 0.3f; cubeUniforms.ambient[2] = 0.35f;
            cubeUniforms.ambient[3] = totalTime + screens[i].timeOffset;  // 时间传入 w 分量
            
            SDL_PushGPUVertexUniformData(cmd, 0, &cubeUniforms, sizeof(cubeUniforms));
            SDL_PushGPUFragmentUniformData(cmd, 0, &cubeUniforms, sizeof(cubeUniforms));
            SDL_DrawGPUPrimitives(rtPass, (Uint32)cubeVerts.size(), 1, 0, 0);
            
            SDL_EndGPURenderPass(rtPass);
        }

        // ========== 渲染主场景 ==========
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
            depthTexture = createDepthTexture(device, sw, sh);
            depthW = sw;
            depthH = sh;
        }

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
        
        Uniforms3D gridUniforms{};
        Mat4 identity = Mat4::identity();
        memcpy(gridUniforms.mvp, vp.m, sizeof(float) * 16);
        memcpy(gridUniforms.model, identity.m, sizeof(float) * 16);
        gridUniforms.lightDir[0] = 0.5f; gridUniforms.lightDir[1] = 1.0f; gridUniforms.lightDir[2] = 0.3f;
        gridUniforms.ambient[0] = 0.3f; gridUniforms.ambient[1] = 0.3f; gridUniforms.ambient[2] = 0.3f;
        
        SDL_PushGPUVertexUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        SDL_PushGPUFragmentUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        SDL_DrawGPUPrimitives(pass, (Uint32)gridVerts.size(), 1, 0, 0);

        // 绘制主立方体
        SDL_BindGPUGraphicsPipeline(pass, cubePipeline);
        SDL_GPUBufferBinding cubeBinding{cubeVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &cubeBinding, 1);
        
        Mat4 mainCubeModel = Mat4::translate(Vec3{0, 0.5f, 0}) * Mat4::rotateY(mainCubeRotation);
        Mat4 mainCubeMVP = vp * mainCubeModel;
        
        Uniforms3D mainCubeUniforms{};
        memcpy(mainCubeUniforms.mvp, mainCubeMVP.m, sizeof(float) * 16);
        memcpy(mainCubeUniforms.model, mainCubeModel.m, sizeof(float) * 16);
        mainCubeUniforms.lightDir[0] = 0.5f; mainCubeUniforms.lightDir[1] = 1.0f; mainCubeUniforms.lightDir[2] = 0.3f;
        mainCubeUniforms.ambient[0] = 0.2f; mainCubeUniforms.ambient[1] = 0.2f; mainCubeUniforms.ambient[2] = 0.25f;
        
        SDL_PushGPUVertexUniformData(cmd, 0, &mainCubeUniforms, sizeof(mainCubeUniforms));
        SDL_PushGPUFragmentUniformData(cmd, 0, &mainCubeUniforms, sizeof(mainCubeUniforms));
        SDL_DrawGPUPrimitives(pass, (Uint32)cubeVerts.size(), 1, 0, 0);

        // 绘制屏幕
        SDL_BindGPUGraphicsPipeline(pass, screenPipeline);
        
        for (int i = 0; i < 2; i++) {
            SDL_GPUBufferBinding screenBinding{screens[i].quadVB, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &screenBinding, 1);
            
            SDL_GPUTextureSamplerBinding texBinding{screens[i].colorTexture, sampler};
            SDL_BindGPUFragmentSamplers(pass, 0, &texBinding, 1);
            
            Mat4 screenModel = getScreenTransform(screens[i]);
            Mat4 screenMVP = vp * screenModel;
            
            UniformsTextured screenUniforms{};
            memcpy(screenUniforms.mvp, screenMVP.m, sizeof(float) * 16);
            memcpy(screenUniforms.model, screenModel.m, sizeof(float) * 16);
            screenUniforms.color[0] = 1.0f; screenUniforms.color[1] = 1.0f;
            screenUniforms.color[2] = 1.0f; screenUniforms.color[3] = 1.0f;
            screenUniforms.highlight[0] = screens[i].hovered ? 1.0f : 0.0f;
            screenUniforms.highlight[1] = 0.0f;
            
            SDL_PushGPUVertexUniformData(cmd, 0, &screenUniforms, sizeof(screenUniforms));
            SDL_PushGPUFragmentUniformData(cmd, 0, &screenUniforms, sizeof(screenUniforms));
            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
        }

        SDL_EndGPURenderPass(pass);
        SDL_SubmitGPUCommandBuffer(cmd);
        
        SDL_Delay(16);
    }

    // 清理
    if (depthTexture) SDL_ReleaseGPUTexture(device, depthTexture);
    
    for (int i = 0; i < 2; i++) {
        if (screens[i].colorTexture) SDL_ReleaseGPUTexture(device, screens[i].colorTexture);
        if (screens[i].depthTexture) SDL_ReleaseGPUTexture(device, screens[i].depthTexture);
        if (screens[i].quadVB) SDL_ReleaseGPUBuffer(device, screens[i].quadVB);
    }
    
    SDL_ReleaseGPUBuffer(device, cubeVB);
    SDL_ReleaseGPUBuffer(device, gridVB);
    SDL_ReleaseGPUSampler(device, sampler);
    SDL_ReleaseGPUGraphicsPipeline(device, cubePipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, rtCubePipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, gridPipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, screenPipeline);
    SDL_ReleaseGPUShader(device, vs3d);
    SDL_ReleaseGPUShader(device, fsCube);
    SDL_ReleaseGPUShader(device, fsGrid);
    SDL_ReleaseGPUShader(device, fsRT);
    SDL_ReleaseGPUShader(device, vsTextured);
    SDL_ReleaseGPUShader(device, fsTextured);
    
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_ShaderCross_Quit();
    SDL_Quit();

    SDL_Log("=== Demo Complete ===");
    return 0;
}
