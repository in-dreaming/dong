/**
 * Dong Engine 3D RT Demo
 * 
 * 真正的 3D 空间 UI 演示：
 * - 第一人称相机控制（右键按住控制视角，WASD 漫游）
 * - 多个 3D 空间中的 UI 面板（渲染到纹理后映射到 3D 矩形）
 * - 射线检测实现 3D 空间中的 UI 交互
 * - 面板可配置为始终面向相机（Billboard 模式）
 * - 悬停高亮和选中效果
 * - 简单的地面网格
 * 
 * 控制说明：
 * - 右键按住 + 鼠标移动：控制视角
 * - WASD：前后左右移动
 * - Q/E 或 空格/Ctrl：上下移动
 * - Shift：加速移动
 * - 左键点击：与 3D 空间中的 UI 面板交互
 * - Tab：切换 Billboard 模式
 * - ESC：退出
 */

#include <cstdio>
#include <cstdint>
#include <vector>
#include <chrono>
#include <cstring>
#include <array>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "utils/math3d.hpp"
#include "utils/camera.hpp"

using namespace dong::utils;

// 面板数量
constexpr int NUM_PANELS = 3;

// 3D 顶点着色器 (HLSL)
static const char* k3DVertexShader = R"(
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
    float4 uHighlight; // x=hovered, y=selected, z=billboard, w=unused
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(uMVP, float4(input.position, 1.0));
    output.uv = input.uv;
    output.worldPos = mul(uModel, float4(input.position, 1.0)).xyz;
    return output;
}
)";

// 3D 纹理片段着色器 (HLSL) - 带高亮效果
static const char* k3DTextureFragShader = R"(
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
    
    // 悬停高亮 - 边缘发光效果
    if (uHighlight.x > 0.5) {
        float2 center = float2(0.5, 0.5);
        float dist = length(input.uv - center);
        float edge = smoothstep(0.4, 0.5, dist);
        float3 glowColor = float3(0.3, 0.6, 1.0);
        result.rgb = lerp(result.rgb, result.rgb + glowColor * 0.3, edge * 0.5);
        // 边框
        float border = 0.02;
        if (input.uv.x < border || input.uv.x > 1.0 - border || 
            input.uv.y < border || input.uv.y > 1.0 - border) {
            result.rgb = lerp(result.rgb, glowColor, 0.7);
        }
    }
    
    // 选中效果 - 更亮的边框
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

// 3D 纯色片段着色器 (HLSL) - 用于地面网格
static const char* k3DColorFragShader = R"(
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
    // 基于距离的淡出效果
    float dist = length(input.worldPos.xz);
    float fade = 1.0 - smoothstep(8.0, 15.0, dist);
    float4 result = uColor;
    result.a *= fade;
    return result;
}
)";

// 3D 顶点结构
struct Vertex3D {
    float x, y, z;
    float u, v;
};

// Uniform 数据结构（必须 16 字节对齐）
struct Uniforms3D {
    float mvp[16];
    float model[16];
    float color[4];
    float highlight[4]; // x=hovered, y=selected, z=billboard, w=unused
};

// UI 面板信息
struct UIPanel {
    dong_view_t* view = nullptr;
    SDL_GPUTexture* render_texture = nullptr;
    SDL_GPUBuffer* vertex_buffer = nullptr;
    uint32_t width = 400;
    uint32_t height = 500;
    Vec3 base_position;    // 基础位置
    Vec3 base_right;       // 基础右方向
    Vec3 base_up;          // 基础上方向
    float panel_width = 2.0f;
    float panel_height = 2.5f;
    Quad3D quad;           // 当前帧的实际 quad（可能是 billboard）
    bool hovered = false;
    bool selected = false;
    bool billboard = false; // 是否始终面向相机
    int panel_id = 0;
};

// 更新面板的 quad（处理 billboard 模式）
static void updatePanelQuad(UIPanel& panel, const Vec3& cameraPos) {
    if (panel.billboard) {
        // Billboard 模式：面向相机
        Vec3 toCamera = (cameraPos - panel.base_position);
        toCamera.y = 0; // 只在水平面上旋转
        toCamera = toCamera.normalized();
        
        Vec3 right = Vec3{0, 1, 0}.cross(toCamera).normalized();
        if (right.lengthSquared() < 0.001f) {
            right = Vec3{1, 0, 0};
        }
        Vec3 up = Vec3{0, 1, 0};
        
        panel.quad = Quad3D(panel.base_position, right, up, panel.panel_width, panel.panel_height);
    } else {
        // 固定方向
        panel.quad = Quad3D(panel.base_position, panel.base_right, panel.base_up, 
                           panel.panel_width, panel.panel_height);
    }
}

// 创建 UI HTML（支持不同主题）
static std::string createPanelHTML(int id, const char* bg, const char* accent) {
    char buf[4096];
    snprintf(buf, sizeof(buf), R"HTML(
<!DOCTYPE html>
<html>
<head><style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:linear-gradient(135deg,%s 0%%,#1a1a2e 100%%);font-family:Arial;padding:16px;color:#f0f0f0}
h2{font-size:24px;color:%s;margin-bottom:12px}
.section{background:rgba(0,0,0,0.3);border-radius:10px;padding:14px;margin-bottom:12px}
.section-title{font-size:14px;color:%s;margin-bottom:10px;font-weight:600}
.button-row{display:flex;gap:8px}
button{background:linear-gradient(145deg,%s,#0a1628);color:white;padding:10px 16px;border-radius:6px;font-size:13px}
#counter{font-size:16px;color:%s;margin-top:10px;font-weight:bold}
.scroll-container{overflow:scroll;height:100px;background:rgba(0,0,0,0.2);border-radius:6px;padding:6px}
.scroll-item{padding:8px 10px;margin-bottom:4px;background:rgba(255,255,255,0.1);border-radius:4px;font-size:13px}
input{width:100%%;padding:10px;background:rgba(0,0,0,0.3);border:2px solid transparent;border-radius:6px;color:white;font-size:13px;margin-bottom:8px}
input:focus{border-color:%s;outline:none}
#preview{font-size:12px;color:#94a3b8;padding:8px;background:rgba(0,0,0,0.2);border-radius:4px}
.status{position:fixed;bottom:8px;left:8px;right:8px;padding:6px 10px;background:rgba(0,0,0,0.5);border-radius:4px;font-size:11px;color:#64748b}
</style></head>
<body>
<h2>3D Panel %d</h2>
<div class="section">
<div class="section-title">Buttons</div>
<div class="button-row">
<button id="btn-add">Add +1</button>
<button id="btn-sub">Sub -1</button>
<button id="btn-reset">Reset</button>
</div>
<div id="counter">Count: 0</div>
</div>
<div class="section">
<div class="section-title">Scroll List</div>
<div class="scroll-container" id="scroll-area">
<div class="scroll-item">Item 1 - Scroll to see more</div>
<div class="scroll-item">Item 2 - 3D Space UI</div>
<div class="scroll-item">Item 3 - Ray Casting</div>
<div class="scroll-item">Item 4 - Texture Mapping</div>
<div class="scroll-item">Item 5 - First Person Camera</div>
<div class="scroll-item">Item 6 - WASD Movement</div>
</div>
</div>
<div class="section">
<div class="section-title">Text Input</div>
<input type="text" id="text-input" placeholder="Type something..." />
<div id="preview">Preview: (empty)</div>
</div>
<div class="status" id="status">Panel %d in 3D space</div>
</body></html>
)HTML", bg, accent, accent, accent, accent, accent, id, id);
    return std::string(buf);
}

static std::string createPanelJS(int id) {
    char buf[2048];
    snprintf(buf, sizeof(buf), R"JS(
(function(){
var count=0;
var btnAdd=document.getElementById('btn-add');
var btnSub=document.getElementById('btn-sub');
var btnReset=document.getElementById('btn-reset');
var counter=document.getElementById('counter');
var status=document.getElementById('status');
var textInput=document.getElementById('text-input');
var preview=document.getElementById('preview');
function updateCounter(){if(counter)counter.textContent='Count: '+count;}
function setStatus(msg){if(status)status.textContent=msg;}
if(btnAdd)btnAdd.addEventListener('click',function(){count++;updateCounter();setStatus('Added! Count: '+count);});
if(btnSub)btnSub.addEventListener('click',function(){count--;updateCounter();setStatus('Subtracted! Count: '+count);});
if(btnReset)btnReset.addEventListener('click',function(){count=0;updateCounter();setStatus('Reset!');});
if(textInput){textInput.addEventListener('input',function(){if(preview)preview.textContent='Preview: '+(textInput.value||'(empty)');setStatus('Typing: '+textInput.value);});}
console.log('[Panel %d] JS initialized');
})();
)JS", id);
    return std::string(buf);
}

// 编译着色器
static SDL_GPUShader* compileShader(SDL_GPUDevice* device, SDL_GPUShaderStage stage,
                                    const char* hlsl, const char* entry) {
    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
    
    SDL_ShaderCross_HLSL_Info info{};
    info.source = hlsl;
    info.entrypoint = entry;
    info.include_dir = nullptr;
    info.defines = nullptr;
    info.shader_stage = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? 
                        SDL_SHADERCROSS_SHADERSTAGE_VERTEX : 
                        SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    info.props = 0;
    
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
        sci.num_samplers = (stage == SDL_GPU_SHADERSTAGE_FRAGMENT) ? 1 : 0;
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
        sci.num_samplers = (stage == SDL_GPU_SHADERSTAGE_FRAGMENT) ? 1 : 0;
        sci.num_uniform_buffers = 1;
        
        SDL_GPUShader* shader = SDL_CreateGPUShader(device, &sci);
        SDL_free(bytecode);
        return shader;
    }
    
    SDL_Log("No supported shader format found");
    return nullptr;
}

int main() {
    SDL_Log("=== Dong 3D RT Demo ===");
    SDL_Log("Controls: RMB+Mouse=Look, WASD=Move, Q/E=Up/Down, LMB=Interact, ESC=Exit");

    // 初始化 SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    
    if (!SDL_ShaderCross_Init()) {
        SDL_Log("SDL_ShaderCross_Init failed: %s", SDL_GetError());
        return 1;
    }

    // 创建窗口
    const int WIN_W = 1280, WIN_H = 720;
    SDL_Window* window = SDL_CreateWindow("Dong 3D RT Demo", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    // 创建 GPU 设备
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
    SDL_GPUShader* vs3d = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, k3DVertexShader, "main");
    SDL_GPUShader* fsTexture = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, k3DTextureFragShader, "main");
    SDL_GPUShader* fsColor = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, k3DColorFragShader, "main");
    
    if (!vs3d || !fsTexture || !fsColor) {
        SDL_Log("Failed to compile shaders");
        return 1;
    }

    // 创建顶点缓冲区描述
    SDL_GPUVertexBufferDescription vbDesc{};
    vbDesc.slot = 0;
    vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDesc.instance_step_rate = 0;
    vbDesc.pitch = sizeof(Vertex3D);

    SDL_GPUVertexAttribute attrs[2] = {};
    attrs[0].buffer_slot = 0;
    attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attrs[0].location = 0;
    attrs[0].offset = 0;
    attrs[1].buffer_slot = 0;
    attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrs[1].location = 1;
    attrs[1].offset = sizeof(float) * 3;

    // 创建纹理管线
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
    pipeInfo.vertex_shader = vs3d;
    pipeInfo.fragment_shader = fsTexture;
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
    pipeInfo.vertex_input_state.num_vertex_attributes = 2;
    pipeInfo.vertex_input_state.vertex_attributes = attrs;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipeInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    SDL_GPUGraphicsPipeline* texturePipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
    if (!texturePipeline) {
        SDL_Log("Failed to create texture pipeline: %s", SDL_GetError());
        return 1;
    }

    // 创建纯色管线
    pipeInfo.fragment_shader = fsColor;
    SDL_GPUGraphicsPipeline* colorPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
    if (!colorPipeline) {
        SDL_Log("Failed to create color pipeline: %s", SDL_GetError());
        return 1;
    }

    // 创建采样器
    SDL_GPUSamplerCreateInfo samplerInfo{};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &samplerInfo);

    // 创建地面网格顶点
    std::vector<Vertex3D> gridVerts;
    const float gridSize = 20.0f;
    const int gridLines = 21;
    for (int i = 0; i < gridLines; i++) {
        float t = -gridSize / 2 + (gridSize / (gridLines - 1)) * i;
        // X 方向线
        gridVerts.push_back({-gridSize / 2, 0, t, 0, 0});
        gridVerts.push_back({gridSize / 2, 0, t, 1, 0});
        // Z 方向线
        gridVerts.push_back({t, 0, -gridSize / 2, 0, 0});
        gridVerts.push_back({t, 0, gridSize / 2, 0, 1});
    }

    // 创建顶点缓冲区
    SDL_GPUBufferCreateInfo vbInfo{};
    vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vbInfo.size = (Uint32)(gridVerts.size() * sizeof(Vertex3D));
    SDL_GPUBuffer* gridVB = SDL_CreateGPUBuffer(device, &vbInfo);
    
    // 创建 Dong 上下文
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        SDL_Log("Failed to create Dong context");
        return 1;
    }

    // 创建 UI 面板
    std::array<UIPanel, NUM_PANELS> panels;
    const uint32_t PANEL_W = 400, PANEL_H = 500;
    
    // 面板配置：位置、颜色主题、是否 billboard
    struct PanelConfig {
        Vec3 position;
        Vec3 right;
        Vec3 up;
        const char* bg;
        const char* accent;
        bool billboard;
    };
    
    std::array<PanelConfig, NUM_PANELS> configs = {{
        {{-3.0f, 1.5f, -2.0f}, {0.7f, 0, 0.7f}, {0, 1, 0}, "#0f2847", "#00d4ff", false},
        {{0.0f, 2.0f, -4.0f}, {1, 0, 0}, {0, 1, 0}, "#2d1b4e", "#b388ff", true},
        {{3.0f, 1.5f, -2.0f}, {-0.7f, 0, 0.7f}, {0, 1, 0}, "#1b3d2f", "#4ade80", false},
    }};
    
    for (int i = 0; i < NUM_PANELS; i++) {
        panels[i].panel_id = i + 1;
        panels[i].width = PANEL_W;
        panels[i].height = PANEL_H;
        panels[i].base_position = configs[i].position;
        panels[i].base_right = configs[i].right;
        panels[i].base_up = configs[i].up;
        panels[i].billboard = configs[i].billboard;
        panels[i].panel_width = 2.0f;
        panels[i].panel_height = 2.5f;
        
        panels[i].view = dong_view_create(ctx, PANEL_W, PANEL_H);
        if (!panels[i].view) {
            SDL_Log("Failed to create panel %d", i);
            return 1;
        }
        
        // 使用 GPU 渲染模式（必须设置 external GPU device）
        dong_view_set_external_gpu_device(panels[i].view, device, window);
        
        dong_view_load_html(panels[i].view, createPanelHTML(i + 1, configs[i].bg, configs[i].accent).c_str());
        dong_view_update(panels[i].view);
        dong_view_eval(panels[i].view, createPanelJS(i + 1).c_str());
        
        // 初始渲染到纹理
        panels[i].render_texture = (SDL_GPUTexture*)dong_view_render_to_gpu_texture(
            panels[i].view, device, PANEL_W, PANEL_H);
        
        if (!panels[i].render_texture) {
            SDL_Log("ERROR: Failed to render initial texture for panel %d: %s", i, SDL_GetError());
            return 1;
        }
        
        // 创建面板顶点缓冲区（预分配）
        SDL_GPUBufferCreateInfo panelVbInfo{};
        panelVbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        panelVbInfo.size = sizeof(Vertex3D) * 6;
        panels[i].vertex_buffer = SDL_CreateGPUBuffer(device, &panelVbInfo);
        
        if (!panels[i].vertex_buffer) {
            SDL_Log("ERROR: Failed to create vertex buffer for panel %d: %s", i, SDL_GetError());
            return 1;
        }
        
        // 初始化 quad
        updatePanelQuad(panels[i], Vec3{0, 2, 8});
    }

    // 创建相机
    FPSCamera camera;
    camera.position = Vec3{0, 2.0f, 8.0f};
    camera.yaw = 3.14159f;
    camera.pitch = -0.1f;

    // 输入状态
    InputState input;
    
    // 深度纹理（延迟创建）
    SDL_GPUTexture* depthTexture = nullptr;
    uint32_t depthW = 0, depthH = 0;
    
    // 全局 billboard 模式开关
    bool globalBillboard = false;
    int selectedPanel = -1;
    
    // 帧计数器（用于限制 UI 渲染频率）
    uint32_t frameCount = 0;

    // 主循环
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;
    int winW = WIN_W, winH = WIN_H;

    SDL_Log("Entering main loop...");
    SDL_Log("Press TAB to toggle billboard mode, 1-3 to select panels");

    while (running) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        input.resetFrameState();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            input.handleEvent(event);

            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_TAB) {
                // 切换全局 billboard 模式
                globalBillboard = !globalBillboard;
                for (int i = 0; i < NUM_PANELS; i++) {
                    panels[i].billboard = globalBillboard;
                }
                SDL_Log("Billboard mode: %s", globalBillboard ? "ON" : "OFF");
            } else if (event.type == SDL_EVENT_KEY_DOWN && 
                       event.key.scancode >= SDL_SCANCODE_1 && 
                       event.key.scancode <= SDL_SCANCODE_3) {
                // 选择面板
                int idx = event.key.scancode - SDL_SCANCODE_1;
                if (idx < NUM_PANELS) {
                    for (int i = 0; i < NUM_PANELS; i++) panels[i].selected = false;
                    panels[idx].selected = true;
                    selectedPanel = idx;
                    SDL_Log("Selected Panel %d", idx + 1);
                }
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &winW, &winH);
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
                // 射线检测
                Ray ray = camera.pixelToRay((int)event.button.x, (int)event.button.y, winW, winH);
                
                int hitPanel = -1;
                float closestT = 1e30f;
                Vec2 hitUV;
                
                for (int i = 0; i < NUM_PANELS; i++) {
                    float t;
                    Vec2 uv = panels[i].quad.intersect(ray, &t);
                    if (uv.x >= 0 && t < closestT) {
                        closestT = t;
                        hitPanel = i;
                        hitUV = uv;
                    }
                }
                
                // 更新选中状态
                for (int i = 0; i < NUM_PANELS; i++) panels[i].selected = false;
                
                if (hitPanel >= 0) {
                    panels[hitPanel].selected = true;
                    selectedPanel = hitPanel;
                    int px = (int)(hitUV.x * panels[hitPanel].width);
                    int py = (int)((1.0f - hitUV.y) * panels[hitPanel].height);
                    dong_view_send_mouse_move(panels[hitPanel].view, px, py);
                    dong_view_send_mouse_down(panels[hitPanel].view, 0);
                }
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
                for (int i = 0; i < NUM_PANELS; i++) {
                    dong_view_send_mouse_up(panels[i].view, 0);
                }
            } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                // 滚轮事件传递到悬停的面板
                for (int i = 0; i < NUM_PANELS; i++) {
                    if (panels[i].hovered) {
                        dong_view_send_mouse_wheel(panels[i].view, (int)(event.wheel.x * 30), (int)(event.wheel.y * 30));
                    }
                }
            } else if (event.type == SDL_EVENT_KEY_DOWN && !input.right_mouse_down) {
                // 按键传递到选中的面板（非相机控制时）
                if (selectedPanel >= 0) {
                    dong_view_send_key_down(panels[selectedPanel].view, event.key.key);
                }
            } else if (event.type == SDL_EVENT_KEY_UP) {
                if (selectedPanel >= 0) {
                    dong_view_send_key_up(panels[selectedPanel].view, event.key.key);
                }
            } else if (event.type == SDL_EVENT_TEXT_INPUT) {
                if (selectedPanel >= 0) {
                    dong_view_send_text_input(panels[selectedPanel].view, event.text.text);
                }
            }
        }

        // 相机更新
        camera.update(dt, input.keys, input.right_mouse_down, input.mouse_delta_x, input.mouse_delta_y);
        SDL_SetWindowRelativeMouseMode(window, input.right_mouse_down);

        // 更新面板 quad（处理 billboard）
        for (int i = 0; i < NUM_PANELS; i++) {
            updatePanelQuad(panels[i], camera.position);
        }

        // 射线检测悬停
        Ray hoverRay = camera.pixelToRay(input.mouse_x, input.mouse_y, winW, winH);
        for (int i = 0; i < NUM_PANELS; i++) {
            Vec2 uv = panels[i].quad.intersect(hoverRay);
            panels[i].hovered = (uv.x >= 0);
            
            if (panels[i].hovered) {
                int px = (int)(uv.x * panels[i].width);
                int py = (int)((1.0f - uv.y) * panels[i].height);
                dong_view_send_mouse_move(panels[i].view, px, py);
            }
        }

        // 更新 UI（这会标记内容是否变化）
        for (int i = 0; i < NUM_PANELS; i++) {
            dong_view_update(panels[i].view);
        }
        
        // 渲染 UI 到纹理（每帧都更新，因为 UI 可能变化）
        // 注意：dong_view_render_to_gpu_texture 内部会创建并提交 command buffer
        // 它使用独立的 offscreen 渲染路径，不会与主循环的 command buffer 冲突
        for (int i = 0; i < NUM_PANELS; i++) {
            // 释放旧纹理
            if (panels[i].render_texture) {
                SDL_ReleaseGPUTexture(device, panels[i].render_texture);
                panels[i].render_texture = nullptr;
            }
            
            // 渲染到 GPU 纹理（内部会等待 GPU 空闲并提交）
            panels[i].render_texture = (SDL_GPUTexture*)dong_view_render_to_gpu_texture(
                panels[i].view, device, panels[i].width, panels[i].height);
            
            if (!panels[i].render_texture) {
                SDL_Log("ERROR: Failed to render panel %d to texture (frame %u): %s", i, frameCount, SDL_GetError());
                // 如果渲染失败，标记并退出循环
                running = false;
                break;
            }
        }
        
        if (!running) {
            SDL_Log("ERROR: UI rendering failed, exiting main loop");
            break;  // 如果渲染失败，退出主循环
        }
        
        frameCount++;

        // 获取命令缓冲区（用于主场景渲染）
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        if (!cmd) {
            SDL_Log("Warning: Failed to acquire command buffer: %s", SDL_GetError());
            SDL_Delay(16);
            continue;
        }

        // 开始 copy pass - 上传所有数据
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

        // 上传地面网格顶点
        {
            SDL_GPUTransferBufferCreateInfo tbInfo{};
            tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            tbInfo.size = (Uint32)(gridVerts.size() * sizeof(Vertex3D));
            SDL_GPUTransferBuffer* transferBuf = SDL_CreateGPUTransferBuffer(device, &tbInfo);
            void* mapPtr = SDL_MapGPUTransferBuffer(device, transferBuf, false);
            memcpy(mapPtr, gridVerts.data(), gridVerts.size() * sizeof(Vertex3D));
            SDL_UnmapGPUTransferBuffer(device, transferBuf);
            
            SDL_GPUTransferBufferLocation srcLoc{};
            srcLoc.transfer_buffer = transferBuf;
            srcLoc.offset = 0;
            SDL_GPUBufferRegion dstRegion{};
            dstRegion.buffer = gridVB;
            dstRegion.offset = 0;
            dstRegion.size = (Uint32)(gridVerts.size() * sizeof(Vertex3D));
            SDL_UploadToGPUBuffer(copyPass, &srcLoc, &dstRegion, false);
            SDL_ReleaseGPUTransferBuffer(device, transferBuf);
        }

        // 上传面板顶点
        for (int i = 0; i < NUM_PANELS; i++) {
            if (!panels[i].render_texture) continue;  // 跳过未成功渲染的面板
            
            // 生成面板顶点
            const auto& q = panels[i].quad;
            Vertex3D panelVerts[6] = {
                {q.corners[0].x, q.corners[0].y, q.corners[0].z, 0, 1},
                {q.corners[1].x, q.corners[1].y, q.corners[1].z, 1, 1},
                {q.corners[3].x, q.corners[3].y, q.corners[3].z, 0, 0},
                {q.corners[1].x, q.corners[1].y, q.corners[1].z, 1, 1},
                {q.corners[2].x, q.corners[2].y, q.corners[2].z, 1, 0},
                {q.corners[3].x, q.corners[3].y, q.corners[3].z, 0, 0},
            };

            // 上传顶点
            SDL_GPUTransferBufferCreateInfo panelTbInfo{};
            panelTbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            panelTbInfo.size = sizeof(panelVerts);
            SDL_GPUTransferBuffer* panelTransfer = SDL_CreateGPUTransferBuffer(device, &panelTbInfo);
            void* panelMap = SDL_MapGPUTransferBuffer(device, panelTransfer, false);
            memcpy(panelMap, panelVerts, sizeof(panelVerts));
            SDL_UnmapGPUTransferBuffer(device, panelTransfer);

            SDL_GPUTransferBufferLocation panelSrcLoc{};
            panelSrcLoc.transfer_buffer = panelTransfer;
            SDL_GPUBufferRegion panelDstRegion{};
            panelDstRegion.buffer = panels[i].vertex_buffer;
            panelDstRegion.size = sizeof(panelVerts);
            SDL_UploadToGPUBuffer(copyPass, &panelSrcLoc, &panelDstRegion, false);
            SDL_ReleaseGPUTransferBuffer(device, panelTransfer);
        }
        SDL_EndGPUCopyPass(copyPass);

        // 获取 swapchain
        SDL_GPUTexture* swapchain = nullptr;
        Uint32 sw, sh;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain, &sw, &sh) || !swapchain) {
            SDL_Log("Warning: Failed to acquire swapchain texture: %s", SDL_GetError());
            SDL_SubmitGPUCommandBuffer(cmd);
            SDL_Delay(16);
            continue;
        }

        // 创建/重建深度纹理
        if (!depthTexture || depthW != sw || depthH != sh) {
            if (depthTexture) {
                SDL_ReleaseGPUTexture(device, depthTexture);
                depthTexture = nullptr;
            }
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
            if (!depthTexture) {
                SDL_Log("ERROR: Failed to create depth texture: %s", SDL_GetError());
                SDL_ReleaseGPUTexture(device, swapchain);
                SDL_SubmitGPUCommandBuffer(cmd);
                running = false;
                break;
            }
            depthW = sw;
            depthH = sh;
        }

        // 开始渲染 pass
        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = swapchain;
        colorTarget.clear_color = SDL_FColor{0.08f, 0.08f, 0.12f, 1.0f};
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPUDepthStencilTargetInfo depthTarget{};
        depthTarget.texture = depthTexture;
        depthTarget.clear_depth = 1.0f;
        depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, &depthTarget);

        float aspect = (float)sw / (float)sh;
        Mat4 vp = camera.getViewProjectionMatrix(aspect);
        Mat4 identity = Mat4::identity();

        // 绘制地面网格
        SDL_BindGPUGraphicsPipeline(pass, colorPipeline);
        SDL_GPUBufferBinding vbBinding{gridVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &vbBinding, 1);

        Uniforms3D gridUniforms{};
        memcpy(gridUniforms.mvp, vp.m, sizeof(float) * 16);
        memcpy(gridUniforms.model, identity.m, sizeof(float) * 16);
        gridUniforms.color[0] = 0.3f;
        gridUniforms.color[1] = 0.3f;
        gridUniforms.color[2] = 0.4f;
        gridUniforms.color[3] = 1.0f;
        gridUniforms.highlight[0] = 0;
        gridUniforms.highlight[1] = 0;
        gridUniforms.highlight[2] = 0;
        gridUniforms.highlight[3] = 0;
        SDL_PushGPUVertexUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        SDL_PushGPUFragmentUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        SDL_DrawGPUPrimitives(pass, (Uint32)gridVerts.size(), 1, 0, 0);

        // 绘制 UI 面板
        SDL_BindGPUGraphicsPipeline(pass, texturePipeline);
        
        for (int i = 0; i < NUM_PANELS; i++) {
            if (!panels[i].render_texture) continue;  // 跳过未成功渲染的面板
            
            SDL_GPUBufferBinding panelBinding{panels[i].vertex_buffer, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &panelBinding, 1);

            SDL_GPUTextureSamplerBinding texBinding{panels[i].render_texture, sampler};
            SDL_BindGPUFragmentSamplers(pass, 0, &texBinding, 1);

            Uniforms3D panelUniforms{};
            memcpy(panelUniforms.mvp, vp.m, sizeof(float) * 16);
            memcpy(panelUniforms.model, identity.m, sizeof(float) * 16);
            panelUniforms.color[0] = 1.0f;
            panelUniforms.color[1] = 1.0f;
            panelUniforms.color[2] = 1.0f;
            panelUniforms.color[3] = 1.0f;
            panelUniforms.highlight[0] = panels[i].hovered ? 1.0f : 0.0f;
            panelUniforms.highlight[1] = panels[i].selected ? 1.0f : 0.0f;
            panelUniforms.highlight[2] = panels[i].billboard ? 1.0f : 0.0f;
            panelUniforms.highlight[3] = 0.0f;
            SDL_PushGPUVertexUniformData(cmd, 0, &panelUniforms, sizeof(panelUniforms));
            SDL_PushGPUFragmentUniformData(cmd, 0, &panelUniforms, sizeof(panelUniforms));

            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
        }

        SDL_EndGPURenderPass(pass);
        
        // 提交命令缓冲区（提交后 swapchain 会自动 present）
        if (!SDL_SubmitGPUCommandBuffer(cmd)) {
            SDL_Log("ERROR: Failed to submit command buffer (frame %u): %s", frameCount, SDL_GetError());
            // swapchain 会在提交失败时自动释放，不需要手动释放
            running = false;
            break;
        }
        
        // 注意：swapchain 纹理会在下次 acquire 时自动释放，不需要手动释放

        SDL_Delay(16);
    }

    // 清理
    SDL_Log("Cleaning up...");
    
    for (int i = 0; i < NUM_PANELS; i++) {
        if (panels[i].vertex_buffer) SDL_ReleaseGPUBuffer(device, panels[i].vertex_buffer);
        if (panels[i].render_texture) SDL_ReleaseGPUTexture(device, panels[i].render_texture);
        if (panels[i].view) dong_view_free(panels[i].view);
    }
    
    if (depthTexture) SDL_ReleaseGPUTexture(device, depthTexture);
    SDL_ReleaseGPUBuffer(device, gridVB);
    SDL_ReleaseGPUSampler(device, sampler);
    SDL_ReleaseGPUGraphicsPipeline(device, texturePipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, colorPipeline);
    SDL_ReleaseGPUShader(device, vs3d);
    SDL_ReleaseGPUShader(device, fsTexture);
    SDL_ReleaseGPUShader(device, fsColor);
    
    dong_destroy_context(ctx);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_ShaderCross_Quit();
    SDL_Quit();

    SDL_Log("=== Demo Complete ===");
    return 0;
}
