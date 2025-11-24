#include "gpu_painter.hpp"
#include "gpu_device.hpp"
#include "shader_manager.hpp"
#include "gpu_surface.hpp"
#include "../dom/dom_manager.hpp"
#include "../layout/layout_engine.hpp"
#include <SDL3/SDL_log.h>
#include <cstring>

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

    setupContentTexture();
    setupPipelines();
    if (!fullscreen_pipeline_) {
        SDL_Log("Failed to initialize GPU pipelines");
        return false;
    }

    if (!content_texture_) {
        SDL_Log("Failed to initialize content texture");
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

    // 注意：CPU 像素应该在 uploadCPUPixelsToGPU 中上传
    // 这里只需要从纹理渲染到 swapchain

    renderInternal();

    endFrame();
}

void GPUPainter::renderInternal() {
    if (!is_rendering_ || !current_cmd_buf_) {
        return;
    }

    // 获取 swapchain 纹理以渲染到屏幕
    SDL_GPUTexture* swapchain_texture = nullptr;
    Uint32 w = 0;
    Uint32 h = 0;

    SDL_Window* window = gpu_surface_->getWindow();
    if (!window) {
        SDL_Log("GPUPainter::renderInternal: GPU surface has no window bound");
        return;
    }

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            current_cmd_buf_,
            window,
            &swapchain_texture,
            &w,
            &h)) {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
        return;
    }

    SDL_GPUColorTargetInfo color_target{};
    color_target.texture = swapchain_texture;
    color_target.mip_level = 0;
    color_target.layer_or_depth_plane = 0;
    color_target.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};
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
        return;
    }

    // 设置 viewport：
    // - 内容逻辑尺寸固定为 960x600
    // - 窗口比内容大时，不放大，1:1 居中显示（保证清晰、不拉伸）
    // - 窗口比内容小时，等比缩小整帧
    {
        const float content_width = 960.0f;
        const float content_height = 600.0f;

        const float win_w = static_cast<float>(w);
        const float win_h = static_cast<float>(h);

        float scale = 1.0f;
        if (win_w < content_width || win_h < content_height) {
            const float sx = win_w / content_width;
            const float sy = win_h / content_height;
            scale = sx < sy ? sx : sy;
        }

        SDL_GPUViewport viewport{};
        viewport.w = content_width * scale;
        viewport.h = content_height * scale;
        viewport.x = (win_w - viewport.w) * 0.5f;
        viewport.y = (win_h - viewport.h) * 0.5f;
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;

        SDL_SetGPUViewport(pass, &viewport);
    }

    if (fullscreen_pipeline_ && content_texture_ && content_sampler_) {
        SDL_BindGPUGraphicsPipeline(pass, fullscreen_pipeline_);

        // 绑定采样的纹理和采样器
        SDL_GPUTextureSamplerBinding bindings{};
        bindings.texture = content_texture_;
        bindings.sampler = content_sampler_;

        SDL_BindGPUFragmentSamplers(pass, 0, &bindings, 1);

        // 绘制全屏三角形
        SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
        SDL_Log("Rendered 3 vertices with content texture");
    } else {
        SDL_Log("RenderInternal: missing pipeline=%p texture=%p sampler=%p", 
            (void*)fullscreen_pipeline_, (void*)content_texture_, (void*)content_sampler_);
    }

    SDL_EndGPURenderPass(pass);
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
    SDL_Log("beginFrame: acquired command buffer");
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

    // 简单全屏三角形 HLSL：使用 SV_VertexID 生成裁剪空间顶点 + UV
    const char* kFullscreenVS = 
        "struct VSOutput {"
        "    float4 position : SV_Position;"
        "    float2 uv : TEXCOORD0;"
        "};"
        "VSOutput main(uint vertexID : SV_VertexID) {"
        "    float2 pos;"
        "    float2 uv;"
        "    if (vertexID == 0) {"
        "        pos = float2(-1.0, -1.0);"
        "        uv = float2(0.0, 1.0);"
        "    } else if (vertexID == 1) {"
        "        pos = float2(3.0, -1.0);"
        "        uv = float2(1.0, 1.0);"
        "    } else {"
        "        pos = float2(-1.0, 3.0);"
        "        uv = float2(0.0, 0.0);"
        "    }"
        "    VSOutput o;"
        "    o.position = float4(pos, 0.0, 1.0);"
        "    o.uv = uv;"
        "    return o;"
        "}";

    const char* kFullscreenPS =
        "Texture2D contentTexture : register(t0);"
        "SamplerState contentSampler : register(s0);"
        "float4 main(float2 uv : TEXCOORD0) : SV_Target0 {"
        "    return contentTexture.Sample(contentSampler, uv);"
        "}";

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

void GPUPainter::setupContentTexture() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // 创建内容纹理
    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.type = SDL_GPU_TEXTURETYPE_2D;
    texture_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texture_info.width = 960;
    texture_info.height = 600;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    content_texture_ = SDL_CreateGPUTexture(dev, &texture_info);
    if (!content_texture_) {
        SDL_Log("Failed to create content texture: %s", SDL_GetError());
        return;
    }

    // 创建采样器
    SDL_GPUSamplerCreateInfo sampler_info{};
    sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;

    content_sampler_ = SDL_CreateGPUSampler(dev, &sampler_info);
    if (!content_sampler_) {
        SDL_Log("Failed to create content sampler: %s", SDL_GetError());
        SDL_ReleaseGPUTexture(dev, content_texture_);
        content_texture_ = nullptr;
        return;
    }

    SDL_Log("Content texture and sampler created");
}

void GPUPainter::uploadCPUPixelsToGPU(const void* cpu_buffer, uint32_t width, uint32_t height) {
    SDL_Log("uploadCPUPixelsToGPU called: buffer=%p w=%u h=%u", cpu_buffer, width, height);
    
    if (!cpu_buffer || !gpu_device_ || !gpu_device_->isInitialized() || !content_texture_) {
        SDL_Log("uploadCPUPixelsToGPU: invalid params");
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    
    if (!current_cmd_buf_) {
        SDL_Log("uploadCPUPixelsToGPU: no active command buffer");
        return;
    }
    
    SDL_Log("uploadCPUPixelsToGPU: proceeding with upload");

    // 计算缓冲大小
    uint32_t stride = width * 4;  // RGBA
    uint32_t buffer_size = stride * height;

    // 创建临时上传缓冲
    SDL_GPUTransferBufferCreateInfo transfer_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = buffer_size,
    };

    SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
    if (!transfer_buf) {
        SDL_Log("Failed to create transfer buffer: %s", SDL_GetError());
        return;
    }

    // 映射并复制数据
    void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
    if (!mapped) {
        SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        return;
    }

    std::memcpy(mapped, cpu_buffer, buffer_size);
    SDL_UnmapGPUTransferBuffer(dev, transfer_buf);

    // 开始复制传递
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(current_cmd_buf_);
    if (!copy_pass) {
        SDL_Log("Failed to begin GPU copy pass: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        return;
    }

    // 上传到纹理
    SDL_GPUTextureTransferInfo texture_transfer = {
        .transfer_buffer = transfer_buf,
        .offset = 0,
    };

    SDL_GPUTextureRegion texture_region = {
        .texture = content_texture_,
        .mip_level = 0,
        .layer = 0,
        .x = 0,
        .y = 0,
        .z = 0,
        .w = width,
        .h = height,
        .d = 1,
    };

    SDL_UploadToGPUTexture(copy_pass, &texture_transfer, &texture_region, false);
    SDL_EndGPUCopyPass(copy_pass);

    // 清理临时缓冲
    SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);

    SDL_Log("Uploaded %u x %u pixels to GPU texture", width, height);
}

} // namespace dong::render
