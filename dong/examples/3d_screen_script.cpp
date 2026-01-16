/**
 * Dong Engine 3D Multi-Screen Script Demo
 * 
 * 演示多个 HTML 屏幕的自动排列和交互
 * 支持 n 个 HTML 文件，用数组声明文件路径和窗口大小，自动排列屏幕
 * 
 * 控制说明：
 * - 右键按住 + 鼠标移动：控制视角
 * - WASD：前后左右移动
 * - Q/E 或 空格/Ctrl：上下移动
 * - Shift：加速移动
 * - 左键点击屏幕：与 HTML 交互
 * - ESC：退出
 */

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <cstring>
#include <string>
#include <csignal>
#include <limits>


#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "utils/math3d.hpp"
#include "utils/camera.hpp"
#include "utils/gpu_utils.hpp"
#include "utils/dong_utils.hpp"
#include "core/profiler.h"

using namespace dong::utils;

// Global for signal handler and atexit
static std::string g_profile_output;

static void dumpProfilerAtExit() {
    if (!g_profile_output.empty()) {
        printf("[Profile] atexit: dumping profiler trace...\n");
        fflush(stdout);
        int result = dong_profiler_dump(g_profile_output.c_str());
        printf("[Profile] atexit: dump result=%d, file=%s\n", result, g_profile_output.c_str());
        fflush(stdout);
    }
}

static void signalHandler(int signum) {
    printf("[Profile] Signal %d received\n", signum);
    fflush(stdout);
    dumpProfilerAtExit();
    std::_Exit(signum);  // Use _Exit to avoid calling atexit handlers twice
}

// ============================================================================
// HUD 系统
// ============================================================================

struct HUD {
    HtmlScreen3D html;
    SDL_GPUBuffer* quadVB = nullptr;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    
    bool showHelp = false;
    float fps = 0.0f;
    float fpsAccum = 0.0f;
    int frameCount = 0;
    float fpsUpdateTimer = 0.0f;
    
    bool init(dong_context_t* ctx, SDL_GPUDevice* device, SDL_Window* window,
              const char* htmlContent, uint32_t w, uint32_t h,
              SDL_GPUShader* vsHUD, SDL_GPUShader* fsHUD,
              const char* resourceRoot = nullptr) {
        // 初始化 HTML 渲染
        if (!html.init(ctx, device, window, htmlContent, w, h, resourceRoot)) {
            return false;
        }

        
        // 创建 HUD 顶点缓冲区
        SDL_GPUBufferCreateInfo vbInfo{};
        vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        vbInfo.size = sizeof(VertexHUD) * 6;
        quadVB = SDL_CreateGPUBuffer(device, &vbInfo);
        if (!quadVB) return false;
        
        // 创建 HUD 管线
        SDL_GPUVertexBufferDescription vbDesc{};
        vbDesc.slot = 0;
        vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vbDesc.pitch = sizeof(VertexHUD);
        
        SDL_GPUVertexAttribute attrs[2] = {};
        attrs[0].buffer_slot = 0;
        attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attrs[0].location = 0;
        attrs[0].offset = 0;
        attrs[1].buffer_slot = 0;
        attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attrs[1].location = 1;
        attrs[1].offset = sizeof(float) * 2;
        
        SDL_GPUColorTargetDescription colorDesc{};
        colorDesc.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        colorDesc.blend_state.enable_blend = true;
        colorDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        colorDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        
        SDL_GPUGraphicsPipelineCreateInfo pipeInfo{};
        pipeInfo.vertex_shader = vsHUD;
        pipeInfo.fragment_shader = fsHUD;
        pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipeInfo.target_info.num_color_targets = 1;
        pipeInfo.target_info.color_target_descriptions = &colorDesc;
        pipeInfo.target_info.has_depth_stencil_target = false;
        pipeInfo.vertex_input_state.num_vertex_buffers = 1;
        pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
        pipeInfo.vertex_input_state.num_vertex_attributes = 2;
        pipeInfo.vertex_input_state.vertex_attributes = attrs;
        pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
        pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
        
        pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
        return pipeline != nullptr;
    }
    
    void update(SDL_GPUDevice* device, float dt) {
        // 更新 FPS
        frameCount++;
        fpsAccum += dt;
        fpsUpdateTimer += dt;
        
        // 每 0.5 秒更新一次 FPS 显示
        if (fpsUpdateTimer >= 0.5f) {
            fps = frameCount / fpsAccum;
            fpsAccum = 0;
            frameCount = 0;
            fpsUpdateTimer = 0;
            
            // 更新 HTML 中的 FPS 显示
            char js[64];
            snprintf(js, sizeof(js), "updateFPS(%.1f)", fps);
            html.eval(js);
        }
        
        // 渲染 HTML 到纹理
        html.update(device);
    }
    
    void toggleHelp() {
        showHelp = !showHelp;
        html.eval("toggleHelp()");
    }
    
    void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* cmd,
                SDL_GPUSampler* sampler) {
        if (!html.renderTexture || !pipeline) return;
        
        SDL_BindGPUGraphicsPipeline(pass, pipeline);
        
        SDL_GPUBufferBinding binding{quadVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
        
        SDL_GPUTextureSamplerBinding texBinding{html.renderTexture, sampler};
        SDL_BindGPUFragmentSamplers(pass, 0, &texBinding, 1);
        
        UniformsHUD uniforms{};
        uniforms.color[0] = 1.0f;
        uniforms.color[1] = 1.0f;
        uniforms.color[2] = 1.0f;
        uniforms.color[3] = 1.0f;
        
        SDL_PushGPUFragmentUniformData(cmd, 0, &uniforms, sizeof(uniforms));
        SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
    }
    
    void cleanup(SDL_GPUDevice* device) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
            pipeline = nullptr;
        }
        if (quadVB) {
            SDL_ReleaseGPUBuffer(device, quadVB);
            quadVB = nullptr;
        }
        html.cleanup(device);
    }
};

// HTML 屏幕配置
struct ScreenConfig {
    const char* htmlFile;
    uint32_t rtWidth;
    uint32_t rtHeight;
    float width;   // 3D 世界中的宽度
    float height;  // 3D 世界中的高度
};

// 屏幕信息（扩展 HtmlScreen3D）
struct Screen3D {
    HtmlScreen3D html;
    Vec3 position;
    float yaw = 0;
    float width = 3.0f;
    float height = 4.0f;
    uint32_t rtWidth = 800;
    uint32_t rtHeight = 1280;
    bool hovered = false;
    bool focused = false;
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

// 自动排列屏幕位置
// 从左下角往右上角分布，避免叠加
void arrangeScreens(Screen3D* screens, int numScreens, float spacingX = 4.0f, float spacingY = 2.8f) {
    if (numScreens <= 0) return;
    
    // 计算排列方式：优先横向排列，超过一定数量时分行
    int maxPerRow = 5;  // 每行最多5个屏幕
    int totalRows = (numScreens + maxPerRow - 1) / maxPerRow;
    
    // 基准 Y：让整个阵列垂直居中于相机高度附近
    // 相机初始高度 2.5f，让阵列中心大约在 2.5f
    float baseY = 2.5f + 3.0f - (totalRows - 1) * spacingY * 0.5f;
    
    for (int i = 0; i < numScreens; i++) {
        int row = i / maxPerRow;
        int col = i % maxPerRow;
        int screensInThisRow = std::min(maxPerRow, numScreens - row * maxPerRow);
        
        // 计算该行的起始X位置（居中对齐）
        float rowWidth = (screensInThisRow - 1) * spacingX;
        float startX = -rowWidth * 0.5f;
        
        // 设置位置：row 增加时 Y 增加（从下往上排）
        screens[i].position.x = startX + col * spacingX;
        screens[i].position.y = baseY + row * spacingY;
        screens[i].position.z = -3.0f;  // 稍微远一点，避免太近
        
        // 设置朝向（稍微向内倾斜）
        float centerX = 0.0f;
        float deltaX = screens[i].position.x - centerX;
        screens[i].yaw = -deltaX * 0.08f;  // 向中心倾斜
        
        // 限制倾斜角度
        screens[i].yaw = std::max(-0.4f, std::min(0.4f, screens[i].yaw));
    }
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    std::string profile_output;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--profile" && i + 1 < argc) {
            profile_output = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            printf("Usage: %s [--profile <trace.json>]\n", argv[0]);
            printf("Options:\n");
            printf("  --profile <file.json>  Dump profiler trace to file (Chrome Trace format)\n");
            return 0;
        }
    }

    // 初始化 profiler (before SDL_Init, use printf)
    if (!profile_output.empty()) {
        printf("[Profile] Profiler enabled, output: %s\n", profile_output.c_str());
        fflush(stdout);
        g_profile_output = profile_output;  // Save for atexit/signal handler
        dong_profiler_init();
        
        // Register atexit handler - will be called on any normal exit
        std::atexit(dumpProfilerAtExit);
        
        // Install signal handlers for Ctrl+C etc
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
    }

    SDL_Log("=== Dong 3D Multi-Screen Script Demo ===");
    SDL_Log("This demo supports multiple HTML screens with automatic arrangement");
    SDL_Log("Controls: RMB+Mouse=Look, WASD=Move, Q/E=Up/Down, Shift=Sprint, LMB=Interact, ESC=Exit");

    // 配置要显示的 HTML 屏幕
    ScreenConfig screenConfigs[] = {
        {"screen1_script.html", 800, 1280, 3.0f, 4.8f},
        {"screen2_script.html", 960, 640, 3.0f, 2.0f},
        {"feature_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/cursor_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/image_test.html", 960, 640, 3.0f, 2.0f},

        {"tests/background_attachment_fixed_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/background_clip_test.html",960, 640, 3.0f, 2.0f},
        {"tests/background_origin_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/cursor_test.html",960, 640, 3.0f, 2.0f},
        {"tests/font_style_test.html",960, 640, 3.0f, 2.0f},
        {"tests/getcomputedstyle_smoke_test.html",960, 640, 3.0f, 2.0f},
        {"tests/image_test.html",960, 640, 3.0f, 2.0f},
        {"tests/outline_test.html",960, 640, 3.0f, 2.0f},
        {"tests/queryselector_complex_test.html",960, 640, 3.0f, 2.0f},
        {"tests/stylesheets_deleterule_test.html",960, 640, 3.0f, 2.0f},
        {"tests/stylesheets_insertrule_test.html",960, 640, 3.0f, 2.0f},
        {"tests/text_decoration_test.html",960, 640, 3.0f, 2.0f},
        {"tests/text_shadow_test.html",960, 640, 3.0f, 2.0f},
        {"tests/text_wrap_test.html",960, 640, 3.0f, 2.0f},
        {"tests/transform_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_play_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_events_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_acceptance.html",960, 640, 3.0f, 2.0f},
        // 可以继续添加更多屏幕...
    };
    
    const int numScreens = sizeof(screenConfigs) / sizeof(screenConfigs[0]);
    SDL_Log("Configured %d screens", numScreens);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    
    if (!SDL_ShaderCross_Init()) {
        SDL_Log("SDL_ShaderCross_Init failed: %s", SDL_GetError());
        return 1;
    }

    const int WIN_W = 2560, WIN_H = 1440;
    SDL_Window* window = SDL_CreateWindow("Dong 3D Multi-Screen Script Demo", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
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

    // 创建 Dong 上下文
    dong_context_t* dongCtx = dong_create_context();
    if (!dongCtx) {
        SDL_Log("Failed to create dong context");
        return 1;
    }

    // 编译着色器
    SDL_GPUShader* vs3d = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShader3D, "main");
    SDL_GPUShader* fsCube = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShader3D, "main");
    SDL_GPUShader* fsGrid = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderGrid, "main");
    SDL_GPUShader* vsTextured = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShaderTextured, "main");
    SDL_GPUShader* fsTextured = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderTextured, "main", 1);
    SDL_GPUShader* vsHUD = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShaderHUD, "main", 0, 0);
    SDL_GPUShader* fsHUD = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderHUD, "main", 1);
    
    if (!vs3d || !fsCube || !fsGrid || !vsTextured || !fsTextured || !vsHUD || !fsHUD) {
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

    if (!cubePipeline || !gridPipeline || !screenPipeline) {
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

    // 读取 HTML 文件并创建屏幕
    std::vector<std::string> htmlContents(numScreens);
    std::vector<std::string> htmlPaths(numScreens);
    std::vector<std::string> htmlRoots(numScreens);
    std::vector<Screen3D> screens(numScreens);

    for (int i = 0; i < numScreens; i++) {
        htmlPaths[i] = getDataPath(screenConfigs[i].htmlFile);
        htmlContents[i] = readFile(htmlPaths[i].c_str());
        if (htmlContents[i].empty()) {
            SDL_Log("Warning: Failed to load %s, using fallback HTML", screenConfigs[i].htmlFile);
            htmlContents[i] = "<html><body><h1>File not found: " + std::string(screenConfigs[i].htmlFile) + "</h1></body></html>";
            htmlRoots[i] = getDataPath("");
        } else {
            SDL_Log("Loaded %s (%zu bytes)", screenConfigs[i].htmlFile, htmlContents[i].size());
            try {
                htmlRoots[i] = std::filesystem::path(htmlPaths[i]).parent_path().string();
            } catch (...) {
                htmlRoots[i] = getDataPath("");
            }
        }

        // 设置屏幕属性
        screens[i].width = screenConfigs[i].width;
        screens[i].height = screenConfigs[i].height;
        screens[i].rtWidth = screenConfigs[i].rtWidth;
        screens[i].rtHeight = screenConfigs[i].rtHeight;
    }

    
    // 自动排列屏幕
    arrangeScreens(screens.data(), numScreens);
    
    SDL_Log("Screen arrangement:");
    for (int i = 0; i < numScreens; i++) {
        SDL_Log("  Screen %d: pos=(%.1f,%.1f,%.1f) yaw=%.2f size=%.1fx%.1f rt=%dx%d", 
                i, screens[i].position.x, screens[i].position.y, screens[i].position.z,
                screens[i].yaw, screens[i].width, screens[i].height,
                screens[i].rtWidth, screens[i].rtHeight);
    }

    // 初始化 HTML 屏幕
    // 注意：JS 代码现在通过 HTML 中的 <script> 标签自动执行，无需手动 eval
    for (int i = 0; i < numScreens; i++) {
        if (!screens[i].html.init(dongCtx, device, window, htmlContents[i].c_str(),
                                   screens[i].rtWidth, screens[i].rtHeight,
                                   htmlRoots[i].c_str())) {

            SDL_Log("Failed to init HTML screen %d (%s)", i, screenConfigs[i].htmlFile);
            return 1;
        }
        
        // 设置 3D 属性
        screens[i].html.width = screens[i].width;
        screens[i].html.height = screens[i].height;
        
        // 创建顶点缓冲区
        vbInfo.size = sizeof(VertexUV) * 6;
        screens[i].html.quadVB = SDL_CreateGPUBuffer(device, &vbInfo);
    }

    // 无需手动执行 JS - <script> 标签会在 load_html 时自动执行
    SDL_Log("HTML screens initialized - <script> tags executed automatically");

    // 初始化 HUD
    HUD hud;
    const std::string hudPath = getDataPath("hud.html");
    std::string hudHtml = readFile(hudPath.c_str());
    std::string hudRoot;
    try {
        hudRoot = std::filesystem::path(hudPath).parent_path().string();
    } catch (...) {
        hudRoot = getDataPath("");
    }

    if (hudHtml.empty()) {
        SDL_Log("Warning: Failed to load hud.html");
        hudHtml = "<html><body style='background:transparent'><div style='color:white;position:absolute;top:10px;left:10px'>FPS: --</div></body></html>";
    }
    if (!hud.init(dongCtx, device, window, hudHtml.c_str(), WIN_W, WIN_H, vsHUD, fsHUD, hudRoot.c_str())) {

        SDL_Log("Failed to init HUD");
        return 1;
    }
    SDL_Log("HUD initialized");

    // 相机
    FPSCamera camera;
    camera.position = Vec3{0.0f, 2.5f, 8.0f};
    camera.yaw = -3.14159f;
    camera.pitch = -0.1f;

    InputState input;
    
    SDL_GPUTexture* depthTexture = nullptr;
    uint32_t depthW = 0, depthH = 0;
    
    float mainCubeRotation = 0.0f;
    int focusedScreen = -1;  // 当前聚焦的屏幕 (-1 表示无)

    // Track press target so mouse-up goes to the same screen even if cursor moves away.
    int pressedScreen = -1;
    int32_t pressedX = 0;
    int32_t pressedY = 0;

    // If press+release happen in the same frame, delay mouse-up by 1 frame so :active is visible.
    bool pendingMouseUp = false;
    int pendingMouseUpScreen = -1;
    int32_t pendingMouseUpX = 0;
    int32_t pendingMouseUpY = 0;

    // Track last hovered screen so we can clear :hover when cursor leaves all screens.
    int lastHoveredScreen = -1;

    // 只有鼠标坐标真的变化时才把 move 转发给 HTML（否则会导致不必要的 repaint/重建）。
    int lastSentMouseScreen = -1;
    int32_t lastSentMouseX = std::numeric_limits<int32_t>::min();
    int32_t lastSentMouseY = std::numeric_limits<int32_t>::min();


    auto lastTime = std::chrono::high_resolution_clock::now();

    bool running = true;
    int winW = WIN_W, winH = WIN_H;

    // 启用文本输入
    SDL_StartTextInput(window);

    // 帧率/卡顿排查：不要再强制每帧 SDL_Delay(16) 双重限速。
    // - 默认不 sleep（让 swapchain/vsync 自己决定节奏）
    // - 如需手动限帧：设置环境变量 DONG_FRAME_SLEEP_MS=N
    const int frame_sleep_ms = []() -> int {
        const char* v = std::getenv("DONG_FRAME_SLEEP_MS");
        if (!v || !*v) return 0;
        const int ms = std::atoi(v);
        return ms < 0 ? 0 : ms;
    }();

    // 自动化性能采样：预热后清空 profiler，只采样固定时长后自动退出。
    // - 默认仅在传了 --profile 时启用（避免影响日常交互使用）
    // - 可用环境变量覆盖：
    //   - DONG_BENCH_AUTOSTOP=0  关闭自动退出
    //   - DONG_BENCH_WARMUP_MS=N 预热 N ms（默认 2000）
    //   - DONG_BENCH_RUN_MS=N    采样 N ms（默认 5000）
    const bool bench_has_profile = !g_profile_output.empty();
    const bool bench_autostop = bench_has_profile && ([]() {
        const char* v = std::getenv("DONG_BENCH_AUTOSTOP");
        return !(v && std::strcmp(v, "0") == 0);
    })();
    const uint64_t bench_warmup_ms = []() -> uint64_t {
        const char* v = std::getenv("DONG_BENCH_WARMUP_MS");
        if (!v || !*v) return 2000;
        const long long ms = std::atoll(v);
        return ms <= 0 ? 0ull : static_cast<uint64_t>(ms);
    }();
    const uint64_t bench_run_ms = []() -> uint64_t {
        const char* v = std::getenv("DONG_BENCH_RUN_MS");
        if (!v || !*v) return 5000;
        const long long ms = std::atoll(v);
        return ms <= 0 ? 0ull : static_cast<uint64_t>(ms);
    }();

    const auto bench_t0 = std::chrono::steady_clock::now();
    bool bench_measuring = false;
    auto bench_t_measure = bench_t0;

    while (running) {
        // 先处理自动采样窗口（保证开始采样的那一帧也能被记录到）
        if (bench_autostop) {
            const auto tnow = std::chrono::steady_clock::now();
            const uint64_t elapsed_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(tnow - bench_t0).count();

            if (!bench_measuring && elapsed_ms >= bench_warmup_ms) {
                bench_measuring = true;
                bench_t_measure = tnow;

                // 清空并重置时间戳：把启动/预热阶段的长耗时从 trace 里剔除。
                dong_profiler_init();
                SDL_Log("[Bench] measuring started (warmup_ms=%llu run_ms=%llu)",
                        (unsigned long long)bench_warmup_ms, (unsigned long long)bench_run_ms);
            }

            if (bench_measuring) {
                const uint64_t measure_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(tnow - bench_t_measure).count();
                if (bench_run_ms > 0 && measure_ms >= bench_run_ms) {
                    SDL_Log("[Bench] measuring finished (%llu ms), exiting...", (unsigned long long)measure_ms);
                    running = false;
                }
            }
        }

        DONG_PROFILE_SCOPE_CAT("Frame", "frame");

        auto now = std::chrono::high_resolution_clock::now();

        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        
        mainCubeRotation += dt * 0.5f;

        input.resetFrameState();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            input.handleEvent(event);
            
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                SDL_Log("=== Quit event received (type=%d) ===", event.type);
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                SDL_Log("=== ESC pressed, exiting ===");
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && 
                       (event.key.scancode == SDL_SCANCODE_F1 || event.key.scancode == SDL_SCANCODE_H)) {
                // F1 或 H 键切换帮助面板
                hud.toggleHelp();
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &winW, &winH);
            } else if (event.type == SDL_EVENT_TEXT_INPUT) {
                // 转发文本输入到聚焦的屏幕
                if (focusedScreen >= 0) {
                    screens[focusedScreen].html.sendTextInput(event.text.text);
                }
            } else if (event.type == SDL_EVENT_KEY_DOWN && !input.right_mouse_down) {
                // 转发按键到聚焦的屏幕
                if (focusedScreen >= 0) {
                    screens[focusedScreen].html.sendKeyDown(event.key.scancode);
                }
            } else if (event.type == SDL_EVENT_KEY_UP && !input.right_mouse_down) {
                if (focusedScreen >= 0) {
                    screens[focusedScreen].html.sendKeyUp(event.key.scancode);
                }
            }
        }

        // 只有在右键按住且没有聚焦屏幕时才控制相机
        // 当屏幕聚焦时，键盘输入应该被屏幕劫持
        bool cameraControlEnabled = input.right_mouse_down || focusedScreen < 0;
        if (input.right_mouse_down) {
            camera.update(dt, input.keys, true, input.mouse_delta_x, input.mouse_delta_y);
        } else if (focusedScreen < 0) {
            // 只有没有聚焦屏幕时才允许 WASD 移动相机
            camera.update(dt, input.keys, false, 0, 0);
        }
        // 只有右键按住时才锁定鼠标
        SDL_SetWindowRelativeMouseMode(window, input.right_mouse_down);

        // 射线检测屏幕悬停和点击
        Ray hoverRay = camera.pixelToRay(input.mouse_x, input.mouse_y, winW, winH);
        
        int hoveredScreen = -1;
        Vec2 hoveredUV{-1, -1};
        
        int32_t hoveredScreenX = 0;
        int32_t hoveredScreenY = 0;

        for (int i = 0; i < numScreens; i++) {
            Quad3D quad = getScreenQuad(screens[i]);
            Vec2 uv = quad.intersect(hoverRay);
            screens[i].hovered = (uv.x >= 0);
            
            if (screens[i].hovered) {
                hoveredScreen = i;
                hoveredUV = uv;

                // 转换 UV 到屏幕像素坐标
                // 注意：UV 的 v 是从下到上（0=底部，1=顶部）
                // 但 HTML 坐标系 Y 是从上到下（0=顶部）
                // 所以需要翻转 Y 坐标
                hoveredScreenX = (int32_t)(uv.x * screens[i].rtWidth);
                hoveredScreenY = (int32_t)((1.0f - uv.y) * screens[i].rtHeight);
            }
        }

        // 只在坐标变化时才转发 move；否则会导致 HTML 侧每帧都置脏并触发昂贵的 rebuild。
        if (hoveredScreen >= 0) {
            if (hoveredScreen != lastSentMouseScreen || hoveredScreenX != lastSentMouseX || hoveredScreenY != lastSentMouseY) {
                screens[hoveredScreen].html.sendMouseMove(hoveredScreenX, hoveredScreenY);
                lastSentMouseScreen = hoveredScreen;
                lastSentMouseX = hoveredScreenX;
                lastSentMouseY = hoveredScreenY;
            }
        }

        // 当鼠标离开所有屏幕时，给上一块屏幕发一次“屏幕外”的 move 以清除 :hover。
        if (hoveredScreen < 0 && lastHoveredScreen >= 0) {
            screens[lastHoveredScreen].html.sendMouseMove(-1, -1);
            lastHoveredScreen = -1;
            lastSentMouseScreen = -1;
            lastSentMouseX = std::numeric_limits<int32_t>::min();
            lastSentMouseY = std::numeric_limits<int32_t>::min();
        } else if (hoveredScreen >= 0) {
            lastHoveredScreen = hoveredScreen;
        }


        // CSS cursor：由宿主读取 hit-test 位置的 computed cursor 并设置系统光标。
        // 注意：右键相机控制时启用 relative mouse mode，避免在此期间反复 Show/Set cursor。
        if (!input.right_mouse_down) {
            const char* css_cursor = "auto";
            if (hoveredScreen >= 0) {
                css_cursor = dong_view_get_cursor_at(screens[hoveredScreen].html.view, hoveredScreenX, hoveredScreenY);
            }
            applyCSSCursor(css_cursor);
        }

        // If we delayed mouse-up from last frame, deliver it now (after a frame rendered with :active).

        if (pendingMouseUp && pendingMouseUpScreen >= 0) {
            screens[pendingMouseUpScreen].html.sendMouseMove(pendingMouseUpX, pendingMouseUpY);
            screens[pendingMouseUpScreen].html.sendMouseUp(1);
            pendingMouseUp = false;
            pendingMouseUpScreen = -1;
        }

        // 处理鼠标按下
        if (input.left_mouse_pressed) {
            if (hoveredScreen >= 0) {
                // 点击屏幕，设置焦点
                focusedScreen = hoveredScreen;
                
                for (int i = 0; i < numScreens; i++) {
                    screens[i].focused = (i == focusedScreen);
                }

                // Capture press target so release goes to the same screen.
                pressedScreen = hoveredScreen;
                pressedX = hoveredScreenX;
                pressedY = hoveredScreenY;
                
                // 发送鼠标按下事件
                screens[hoveredScreen].html.sendMouseDown(1);
            } else {
                // 点击屏幕外，取消焦点
                focusedScreen = -1;
                for (int i = 0; i < numScreens; i++) {
                    screens[i].focused = false;
                }
                pressedScreen = -1;
            }
        }

        // 处理鼠标释放：始终发送给按下时命中的屏幕（否则会导致 :active 不稳定/卡住）。
        if (input.left_mouse_released) {
            const int targetScreen = (pressedScreen >= 0) ? pressedScreen : hoveredScreen;
            const int32_t upX = (pressedScreen >= 0) ? pressedX : hoveredScreenX;
            const int32_t upY = (pressedScreen >= 0) ? pressedY : hoveredScreenY;

            if (targetScreen >= 0) {
                if (input.left_mouse_pressed) {
                    // press+release in same frame: delay mouse-up by 1 frame for visible :active.
                    pendingMouseUp = true;
                    pendingMouseUpScreen = targetScreen;
                    pendingMouseUpX = upX;
                    pendingMouseUpY = upY;
                } else {
                    screens[targetScreen].html.sendMouseMove(upX, upY);
                    screens[targetScreen].html.sendMouseUp(1);
                }
            }

            pressedScreen = -1;
        }


        // 处理滚轮 - SDL 滚轮值通常是 -1 或 1
        // view.cpp 中已经有 kScrollSpeed = 20.0f，这里只需要小幅放大
        if (hoveredScreen >= 0 && (input.mouse_wheel_x != 0 || input.mouse_wheel_y != 0)) {
            constexpr float kWheelMultiplier = 3.0f;
            screens[hoveredScreen].html.sendMouseWheel(
                input.mouse_wheel_x * kWheelMultiplier, 
                input.mouse_wheel_y * kWheelMultiplier);
        }

        // 更新 HTML 屏幕（离屏渲染）
        for (int i = 0; i < numScreens; i++) {
            screens[i].html.update(device);
        }
        
        // 更新 HUD
        hud.update(device, dt);
        
        // 注意：不要每帧全局等待 GPU idle。
        // 提交顺序已经能保证后续对 HTML 纹理的采样依赖；每帧 WaitForGPUIdle 会把性能直接锁死。
        // 如需排查同步/资源竞争问题，可设置环境变量 DONG_DEBUG_WAIT_GPU_IDLE_EVERY_FRAME=1。
        static const bool kWaitGpuIdleEveryFrame = (std::getenv("DONG_DEBUG_WAIT_GPU_IDLE_EVERY_FRAME") != nullptr);
        if (kWaitGpuIdleEveryFrame) {
            SDL_WaitForGPUIdle(device);
        }


        SDL_GPUCommandBuffer* cmd = nullptr;
        {
            DONG_PROFILE_SCOPE_CAT("SDL_AcquireGPUCommandBuffer", "gpu");
            cmd = SDL_AcquireGPUCommandBuffer(device);
        }
        if (!cmd) {
            // cmd_buf 不可用时避免 busy-spin
            const int sleep_ms = (frame_sleep_ms > 0) ? frame_sleep_ms : 1;
            {
                DONG_PROFILE_SCOPE_CAT("SDL_Delay", "frame");
                SDL_Delay(sleep_ms);
            }
            continue;
        }



        // Copy pass - 上传顶点数据
        // 这些几何体顶点在 demo 中是静态的：只需要上传一次即可，避免每帧 copy/upload 造成不必要的 GPU submit 开销。
        static bool kUploadedStaticVBs = false;
        if (!kUploadedStaticVBs) {
            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            uploadVertices(device, copyPass, cubeVB, cubeVerts);
            uploadVertices(device, copyPass, gridVB, gridVerts);

            // 上传屏幕四边形顶点（静态）
            for (int i = 0; i < numScreens; i++) {
                auto quadVerts = generateQuad(screens[i].width, screens[i].height);
                uploadVertices(device, copyPass, screens[i].html.quadVB, quadVerts);
            }

            // 上传 HUD 四边形顶点（静态）
            auto hudQuadVerts = generateHUDQuad();
            uploadVertices(device, copyPass, hud.quadVB, hudQuadVerts);

            SDL_EndGPUCopyPass(copyPass);
            kUploadedStaticVBs = true;
        }


        // ========== 渲染主场景 ==========
        SDL_GPUTexture* swapchain = nullptr;
        Uint32 sw = 0, sh = 0;
        {
            // swapchain acquire：默认阻塞（稳定、接近 vsync 行为）。
            // 如需减少卡顿/等待，可用 NOWAIT（可能跳帧/撕裂）：DONG_GPU_SWAPCHAIN_NOWAIT=1
            static const bool kSwapchainNowait = (std::getenv("DONG_GPU_SWAPCHAIN_NOWAIT") != nullptr);
            bool ok = false;
            if (!kSwapchainNowait) {
                DONG_PROFILE_SCOPE_CAT("SDL_WaitAndAcquireGPUSwapchainTexture", "gpu");
                ok = SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain, &sw, &sh);
            } else {
                DONG_PROFILE_SCOPE_CAT("SDL_AcquireGPUSwapchainTexture", "gpu");
                ok = SDL_AcquireGPUSwapchainTexture(cmd, window, &swapchain, &sw, &sh);
            }

            if (!ok || !swapchain) {
                {
                    DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBuffer", "gpu");
                    SDL_SubmitGPUCommandBuffer(cmd);
                }
                // swapchain 不可用时避免 busy-spin
                {
                    const int sleep_ms = (frame_sleep_ms > 0) ? frame_sleep_ms : 1;
                    DONG_PROFILE_SCOPE_CAT("SDL_Delay", "frame");
                    SDL_Delay(sleep_ms);
                }
                continue;
            }

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

        // 绘制 HTML 屏幕
        SDL_BindGPUGraphicsPipeline(pass, screenPipeline);
        
        for (int i = 0; i < numScreens; i++) {
            if (!screens[i].html.renderTexture) continue;
            
            SDL_GPUBufferBinding screenBinding{screens[i].html.quadVB, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &screenBinding, 1);
            
            SDL_GPUTextureSamplerBinding texBinding{screens[i].html.renderTexture, sampler};
            SDL_BindGPUFragmentSamplers(pass, 0, &texBinding, 1);
            
            Mat4 screenModel = getScreenTransform(screens[i]);
            Mat4 screenMVP = vp * screenModel;
            
            UniformsTextured screenUniforms{};
            memcpy(screenUniforms.mvp, screenMVP.m, sizeof(float) * 16);
            memcpy(screenUniforms.model, screenModel.m, sizeof(float) * 16);
            screenUniforms.color[0] = 1.0f; screenUniforms.color[1] = 1.0f;
            screenUniforms.color[2] = 1.0f; screenUniforms.color[3] = 1.0f;
            screenUniforms.highlight[0] = screens[i].hovered ? 1.0f : 0.0f;
            screenUniforms.highlight[1] = screens[i].focused ? 1.0f : 0.0f;
            
            SDL_PushGPUVertexUniformData(cmd, 0, &screenUniforms, sizeof(screenUniforms));
            SDL_PushGPUFragmentUniformData(cmd, 0, &screenUniforms, sizeof(screenUniforms));
            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
        }

        SDL_EndGPURenderPass(pass);
        
        // ========== 渲染 HUD（2D 覆盖层，无深度测试）==========
        SDL_GPUColorTargetInfo hudColorTarget{};
        hudColorTarget.texture = swapchain;
        hudColorTarget.load_op = SDL_GPU_LOADOP_LOAD;  // 保留 3D 场景
        hudColorTarget.store_op = SDL_GPU_STOREOP_STORE;
        
        SDL_GPURenderPass* hudPass = SDL_BeginGPURenderPass(cmd, &hudColorTarget, 1, nullptr);
        hud.render(hudPass, cmd, sampler);
        SDL_EndGPURenderPass(hudPass);
        
        {
            DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBuffer", "gpu");
            SDL_SubmitGPUCommandBuffer(cmd);
        }

        if (frame_sleep_ms > 0) {
            DONG_PROFILE_SCOPE_CAT("SDL_Delay", "frame");
            SDL_Delay(frame_sleep_ms);
        }


    }

    SDL_Log("=== Exiting main loop ===");
    fflush(stdout);

    // 停止文本输入
    SDL_Log("[Cleanup] Stopping text input...");
    SDL_StopTextInput(window);

    // 清理
    SDL_Log("[Cleanup] Releasing depth texture...");
    if (depthTexture) SDL_ReleaseGPUTexture(device, depthTexture);
    
    SDL_Log("[Cleanup] Cleaning up HUD...");
    hud.cleanup(device);
    
    SDL_Log("[Cleanup] Cleaning up %d screens...", numScreens);
    for (int i = 0; i < numScreens; i++) {
        SDL_Log("[Cleanup] Screen %d...", i);
        screens[i].html.cleanup(device);
    }
    
    SDL_Log("[Cleanup] Releasing GPU buffers...");
    SDL_ReleaseGPUBuffer(device, cubeVB);
    SDL_ReleaseGPUBuffer(device, gridVB);
    SDL_ReleaseGPUSampler(device, sampler);
    SDL_Log("[Cleanup] Releasing pipelines...");
    SDL_ReleaseGPUGraphicsPipeline(device, cubePipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, gridPipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, screenPipeline);
    SDL_Log("[Cleanup] Releasing shaders...");
    SDL_ReleaseGPUShader(device, vs3d);
    SDL_ReleaseGPUShader(device, fsCube);
    SDL_ReleaseGPUShader(device, fsGrid);
    SDL_ReleaseGPUShader(device, vsTextured);
    SDL_ReleaseGPUShader(device, fsTextured);
    SDL_ReleaseGPUShader(device, vsHUD);
    SDL_ReleaseGPUShader(device, fsHUD);
    
    SDL_Log("[Cleanup] Destroying dong context...");
    dong_destroy_context(dongCtx);
    
    // Dump profiler BEFORE destroying GPU device (in case it crashes)
    if (!g_profile_output.empty()) {
        SDL_Log("[Profile] Dumping profiler trace before GPU cleanup...");
        int result = dong_profiler_dump(g_profile_output.c_str());
        SDL_Log("[Profile] Dump result=%d, file=%s", result, g_profile_output.c_str());
        g_profile_output.clear();  // Prevent atexit from dumping again
    }
    
    SDL_Log("[Cleanup] Destroying GPU device...");
    SDL_DestroyGPUDevice(device);
    SDL_Log("[Cleanup] Destroying window...");
    SDL_DestroyWindow(window);
    SDL_Log("[Cleanup] ShaderCross quit...");
    SDL_ShaderCross_Quit();

    SDL_Log("[Profile] Normal exit path complete");

    SDL_Log("=== Demo Complete ===");
    SDL_Quit();

    // atexit(dumpProfilerAtExit) will be called after this
    return 0;
}
