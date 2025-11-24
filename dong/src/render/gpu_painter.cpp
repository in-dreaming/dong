#include "gpu_painter.hpp"
#include "gpu_device.hpp"
#include "shader_manager.hpp"
#include "gpu_surface.hpp"
#include "../dom/dom_manager.hpp"
#include "../layout/layout_engine.hpp"
#include <SDL3/SDL_log.h>

namespace dong::render {

GPUPainter::GPUPainter(
    GPUTextureSurfaceImpl* gpu_surface,
    GPUDevice* gpu_device,
    ShaderManager* shader_manager
) : gpu_surface_(gpu_surface),
    gpu_device_(gpu_device),
    shader_manager_(shader_manager),
    current_cmd_buf_(nullptr),
    is_rendering_(false) {
}

GPUPainter::~GPUPainter() {
    if (fullscreen_pipeline_ && gpu_device_ && gpu_device_->isInitialized()) {
        SDL_ReleaseGPUGraphicsPipeline(gpu_device_->getHandle(), fullscreen_pipeline_);
        fullscreen_pipeline_ = nullptr;
    }
    if (fullscreen_vs_ && gpu_device_ && gpu_device_->isInitialized()) {
        SDL_ReleaseGPUShader(gpu_device_->getHandle(), fullscreen_vs_);
        fullscreen_vs_ = nullptr;
    }
    if (fullscreen_fs_ && gpu_device_ && gpu_device_->isInitialized()) {
        SDL_ReleaseGPUShader(gpu_device_->getHandle(), fullscreen_fs_);
        fullscreen_fs_ = nullptr;
    }
}

bool GPUPainter::initialize() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPU device not initialized");
        return false;
    }

    if (!gpu_surface_) {
        SDL_Log("GPU surface not initialized");
        return false;
    }

    setupPipelines();
    if (!fullscreen_pipeline_) {
        SDL_Log("Failed to initialize GPU pipelines");
        return false;
    }

    SDL_Log("GPU painter initialized");

    return true;
}

void GPUPainter::render(dom::Manager* dom_manager, layout::Engine* layout_engine) {
    (void)dom_manager;
    (void)layout_engine;

    if (!gpu_surface_ || !gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPUPainter::render: invalid GPU surface or device");
        return;
    }

    beginFrame();
    if (!is_rendering_ || !current_cmd_buf_) {
        return;
    }

    // 使用 GPU render pass 对当前 render target 进行绘制
    SDL_GPUColorTargetInfo color_target{};
    color_target.texture = gpu_surface_->getTexture();
    color_target.mip_level = 0;
    color_target.layer_or_depth_plane = 0;
    color_target.clear_color = SDL_FColor{0.1f, 0.2f, 0.4f, 1.0f};
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;
    color_target.resolve_texture = nullptr;
    color_target.resolve_mip_level = 0;
    color_target.resolve_layer = 0;
    color_target.cycle = false;
    color_target.cycle_resolve_texture = false;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(
        current_cmd_buf_,
        &color_target,
        1,
        nullptr
    );

    if (!pass) {
        SDL_Log("Failed to begin GPU render pass: %s", SDL_GetError());
        endFrame();
        return;
    }

    if (fullscreen_pipeline_) {
        SDL_BindGPUGraphicsPipeline(pass, fullscreen_pipeline_);
        // 利用 SV_VertexID 的全屏三角形，无需绑定顶点缓冲
        SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    }

    SDL_EndGPURenderPass(pass);

    endFrame();
}

void GPUPainter::beginFrame() {
    if (is_rendering_) {
        SDL_Log("Already rendering");
        return;
    }

    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
    if (!current_cmd_buf_) {
        SDL_Log("Failed to acquire command buffer");
        return;
    }

    is_rendering_ = true;
}

void GPUPainter::endFrame() {
    if (!is_rendering_ || !current_cmd_buf_) {
        return;
    }

    gpu_device_->submitCommandBuffer(current_cmd_buf_);
    current_cmd_buf_ = nullptr;
    is_rendering_ = false;
}

void GPUPainter::setupPipelines() {
    if (!gpu_device_ || !gpu_device_->isInitialized() || !shader_manager_) {
        SDL_Log("GPUPainter::setupPipelines: device or shader manager not ready");
        return;
    }

    // 简单全屏三角形 HLSL：使用 SV_VertexID 生成裁剪空间顶点
    static const char* kFullscreenVS = R"(
struct VSOutput {
    float4 position : SV_Position;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 pos;
    if (vertexID == 0) {
        pos = float2(-1.0, -1.0);
    } else if (vertexID == 1) {
        pos = float2(3.0, -1.0);
    } else {
        pos = float2(-1.0, 3.0);
    }

    VSOutput o;
    o.position = float4(pos, 0.0, 1.0);
    return o;
}
)";

    static const char* kFullscreenPS = R"(
float4 main() : SV_Target0 {
    // 颜色最终由 ClearColor 控制，这里返回白色以保证合法输出
    return float4(1.0, 1.0, 1.0, 1.0);
}
)";

    fullscreen_vs_ = shader_manager_->loadShaderFromHLSL(
        "dong_fullscreen_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        kFullscreenVS,
        "main"
    );
    fullscreen_fs_ = shader_manager_->loadShaderFromHLSL(
        "dong_fullscreen_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        kFullscreenPS,
        "main"
    );

    if (!fullscreen_vs_ || !fullscreen_fs_) {
        SDL_Log("GPUPainter::setupPipelines: failed to compile fullscreen shaders");
        return;
    }

    SDL_GPUGraphicsPipelineCreateInfo pci{};

    SDL_GPUColorTargetDescription color_desc{};
    color_desc.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    pci.target_info.num_color_targets = 1;
    pci.target_info.color_target_descriptions = &color_desc;
    pci.target_info.has_depth_stencil_target = false;

    pci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pci.vertex_shader = fullscreen_vs_;
    pci.fragment_shader = fullscreen_fs_;

    // 无顶点缓冲，全部由 SV_VertexID 驱动
    pci.vertex_input_state.num_vertex_buffers = 0;
    pci.vertex_input_state.vertex_buffer_descriptions = nullptr;
    pci.vertex_input_state.num_vertex_attributes = 0;
    pci.vertex_input_state.vertex_attributes = nullptr;

    fullscreen_pipeline_ = SDL_CreateGPUGraphicsPipeline(
        gpu_device_->getHandle(),
        &pci
    );

    if (!fullscreen_pipeline_) {
        SDL_Log("GPUPainter::setupPipelines: failed to create fullscreen pipeline: %s", SDL_GetError());
    }
}

void GPUPainter::drawRect(float x, float y, float width, float height, 
                          uint32_t color, float radius) {
    // 后续实现使用 GPU shader 绘制矩形
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)color;
    (void)radius;
}

void GPUPainter::drawText(const std::string& text, float x, float y, 
                          uint32_t color, float font_size) {
    // 后续实现 GPU 文字渲染
    (void)text;
    (void)x;
    (void)y;
    (void)color;
    (void)font_size;
}

void GPUPainter::drawImage(const std::string& src, float x, float y, 
                           float width, float height, uint8_t alpha) {
    // 后续实现 GPU 图片渲染
    (void)src;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)alpha;
}

} // namespace dong::render
