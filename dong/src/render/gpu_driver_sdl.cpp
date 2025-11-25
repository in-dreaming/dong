#include "gpu_driver_sdl.hpp"
#include "gpu_device.hpp"
#include "shader_manager.hpp"
#include "resource_manager.hpp"
#include <SDL3/SDL_log.h>
#include <vector>
#include <algorithm>
#include <cstring>

namespace dong::render {

GPUDriverSDL::GPUDriverSDL(GPUDevice* device, SDL_Window* window, ShaderManager* shader_manager)
    : gpu_device_(device), window_(window), shader_manager_(shader_manager) {}

GPUDriverSDL::~GPUDriverSDL() {
    // 确保命令缓冲已提交，避免泄漏
    if (in_frame_ && gpu_device_ && current_cmd_buf_) {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
    }
    current_cmd_buf_ = nullptr;
    in_frame_ = false;

    if (gpu_device_ && gpu_device_->isInitialized()) {
        SDL_GPUDevice* dev = gpu_device_->getHandle();
        if (rect_pipeline_) {
            SDL_ReleaseGPUGraphicsPipeline(dev, rect_pipeline_);
            rect_pipeline_ = nullptr;
        }
        if (rect_vs_) {
            SDL_ReleaseGPUShader(dev, rect_vs_);
            rect_vs_ = nullptr;
        }
        if (rect_fs_) {
            SDL_ReleaseGPUShader(dev, rect_fs_);
            rect_fs_ = nullptr;
        }

        if (round_rect_pipeline_) {
            SDL_ReleaseGPUGraphicsPipeline(dev, round_rect_pipeline_);
            round_rect_pipeline_ = nullptr;
        }
        if (round_rect_vs_) {
            SDL_ReleaseGPUShader(dev, round_rect_vs_);
            round_rect_vs_ = nullptr;
        }
        if (round_rect_fs_) {
            SDL_ReleaseGPUShader(dev, round_rect_fs_);
            round_rect_fs_ = nullptr;
        }

        if (image_pipeline_) {
            SDL_ReleaseGPUGraphicsPipeline(dev, image_pipeline_);
            image_pipeline_ = nullptr;
        }
        if (image_vs_) {
            SDL_ReleaseGPUShader(dev, image_vs_);
            image_vs_ = nullptr;
        }
        if (image_fs_) {
            SDL_ReleaseGPUShader(dev, image_fs_);
            image_fs_ = nullptr;
        }
        if (image_sampler_) {
            SDL_ReleaseGPUSampler(dev, image_sampler_);
            image_sampler_ = nullptr;
        }
        if (image_atlas_texture_) {
            SDL_ReleaseGPUTexture(dev, image_atlas_texture_);
            image_atlas_texture_ = nullptr;
        }
    }
}

bool GPUDriverSDL::initialize() {
    if (!gpu_device_ || !gpu_device_->isInitialized() || !window_ || !shader_manager_) {
        SDL_Log("GPUDriverSDL::initialize: invalid device, window, or shader manager");
        return false;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // 简单的纯色矩形着色器：用 SV_VertexID 生成一个屏幕空间矩形
    const char* kRectVS = R"(
struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR0;
};

[[vk::binding(0, 1)]] cbuffer RectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    float2 pos = uRect.xy + local * uRect.zw;
    float2 ndc;
    ndc.x = (pos.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pos.y / uViewport.y) * 2.0;
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.color = uColor;
    return o;
}
)";

    const char* kRectFS = R"(
struct PSInput {
    float4 position : SV_Position;
    float4 color : COLOR0;
};

float4 main(PSInput input) : SV_Target0 {
    return input.color;
}
)";

    rect_vs_ = shader_manager_->loadShaderFromHLSL(
        "dong_rect_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        kRectVS,
        "main"
    );
    rect_fs_ = shader_manager_->loadShaderFromHLSL(
        "dong_rect_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        kRectFS,
        "main"
    );

    if (!rect_vs_ || !rect_fs_) {
        SDL_Log("GPUDriverSDL::initialize: failed to compile rect shaders");
        return false;
    }

    SDL_GPUGraphicsPipelineCreateInfo pci{};
    SDL_GPUColorTargetDescription color_desc{};
    color_desc.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    pci.target_info.num_color_targets = 1;
    pci.target_info.color_target_descriptions = &color_desc;
    pci.target_info.has_depth_stencil_target = false;

    pci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    pci.vertex_shader = rect_vs_;
    pci.fragment_shader = rect_fs_;

    pci.vertex_input_state.num_vertex_buffers = 0;
    pci.vertex_input_state.vertex_buffer_descriptions = nullptr;
    pci.vertex_input_state.num_vertex_attributes = 0;
    pci.vertex_input_state.vertex_attributes = nullptr;

    rect_pipeline_ = SDL_CreateGPUGraphicsPipeline(dev, &pci);
    if (!rect_pipeline_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create rect pipeline: %s", SDL_GetError());
        return false;
    }

    // 圆角矩形绘制：analytic SDF，在 fragment 阶段做抗锯齿边缘
    const char* kRoundRectVS = R"(
struct VSOutput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    float4 color : COLOR0;
};

[[vk::binding(0, 1)]] cbuffer RoundRectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uColor;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    float2 pos = uRect.xy + local * uRect.zw;
    float2 ndc;
    ndc.x = (pos.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pos.y / uViewport.y) * 2.0;
    float radius = uRadius.x;
    radius = min(radius, min(uRect.z, uRect.w) * 0.5 - 0.5);
    radius = max(radius, 0.0);
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.local = local;
    o.size = uRect.zw;
    o.radius = radius;
    o.color = uColor;
    return o;
}
)";

    const char* kRoundRectFS = R"(
struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    float4 color : COLOR0;
};

float sdRoundRect(float2 p, float2 halfSize, float radius) {
    float2 q = abs(p) - (halfSize - radius);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - radius;
}

float4 main(PSInput input) : SV_Target0 {
    float2 size = input.size;
    float2 p = (input.local - 0.5) * size;
    float r = input.radius;
    float2 halfSize = size * 0.5;
    float dist = sdRoundRect(p, halfSize, r);
    const float aa = 1.0;
    float alpha = saturate(0.5 - dist / aa);
    float4 base = input.color;
    base.a *= alpha;
    return base;
}
)";

    round_rect_vs_ = shader_manager_->loadShaderFromHLSL(
        "dong_round_rect_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        kRoundRectVS,
        "main"
    );
    round_rect_fs_ = shader_manager_->loadShaderFromHLSL(
        "dong_round_rect_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        kRoundRectFS,
        "main"
    );

    if (!round_rect_vs_ || !round_rect_fs_) {
        SDL_Log("GPUDriverSDL::initialize: failed to compile round-rect shaders");
        return false;
    }

    SDL_GPUGraphicsPipelineCreateInfo rrci{};
    SDL_GPUColorTargetDescription color_desc_rr{};
    color_desc_rr.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    rrci.target_info.num_color_targets = 1;
    rrci.target_info.color_target_descriptions = &color_desc_rr;
    rrci.target_info.has_depth_stencil_target = false;

    rrci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    rrci.vertex_shader = round_rect_vs_;
    rrci.fragment_shader = round_rect_fs_;

    rrci.vertex_input_state.num_vertex_buffers = 0;
    rrci.vertex_input_state.vertex_buffer_descriptions = nullptr;
    rrci.vertex_input_state.num_vertex_attributes = 0;
    rrci.vertex_input_state.vertex_attributes = nullptr;

    round_rect_pipeline_ = SDL_CreateGPUGraphicsPipeline(dev, &rrci);
    if (!round_rect_pipeline_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create round-rect pipeline: %s", SDL_GetError());
        return false;
    }

    // 图片绘制着色器：使用 SV_VertexID 生成矩形，并根据 atlas UV 采样
    const char* kImageVS = R"(
struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
};

[[vk::binding(0, 1)]] cbuffer ImageUniforms : register(b0, space1) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTint;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    float2 pos = uRect.xy + local * uRect.zw;
    float2 ndc;
    ndc.x = (pos.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pos.y / uViewport.y) * 2.0;
    float2 uv = float2(lerp(uUVRect.x, uUVRect.z, local.x), lerp(uUVRect.y, uUVRect.w, local.y));
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = uv;
    o.tint = uTint;
    return o;
}
)";

    const char* kImageFS = R"(
Texture2D imageTexture : register(t0);
SamplerState imageSampler : register(s0);

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
};

float4 main(PSInput input) : SV_Target0 {
    float4 c = imageTexture.Sample(imageSampler, input.uv);
    return c * input.tint;
}
)";

    image_vs_ = shader_manager_->loadShaderFromHLSL(
        "dong_image_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        kImageVS,
        "main"
    );
    image_fs_ = shader_manager_->loadShaderFromHLSL(
        "dong_image_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        kImageFS,
        "main"
    );

    if (!image_vs_ || !image_fs_) {
        SDL_Log("GPUDriverSDL::initialize: failed to compile image shaders");
        return false;
    }

    SDL_GPUGraphicsPipelineCreateInfo ipci{};
    SDL_GPUColorTargetDescription color_desc2{};
    color_desc2.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    ipci.target_info.num_color_targets = 1;
    ipci.target_info.color_target_descriptions = &color_desc2;
    ipci.target_info.has_depth_stencil_target = false;

    ipci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    ipci.vertex_shader = image_vs_;
    ipci.fragment_shader = image_fs_;

    ipci.vertex_input_state.num_vertex_buffers = 0;
    ipci.vertex_input_state.vertex_buffer_descriptions = nullptr;
    ipci.vertex_input_state.num_vertex_attributes = 0;
    ipci.vertex_input_state.vertex_attributes = nullptr;

    image_pipeline_ = SDL_CreateGPUGraphicsPipeline(dev, &ipci);
    if (!image_pipeline_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create image pipeline: %s", SDL_GetError());
        return false;
    }

    // 创建一张固定大小的 atlas 纹理与采样器
    image_atlas_width_ = 2048;
    image_atlas_height_ = 2048;
    atlas_cursor_x_ = 0;
    atlas_cursor_y_ = 0;
    atlas_row_height_ = 0;

    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.width = image_atlas_width_;
    tex_info.height = image_atlas_height_;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    image_atlas_texture_ = SDL_CreateGPUTexture(dev, &tex_info);
    if (!image_atlas_texture_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create image atlas texture: %s", SDL_GetError());
        return false;
    }

    SDL_GPUSamplerCreateInfo sampler_info{};
    sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;

    image_sampler_ = SDL_CreateGPUSampler(dev, &sampler_info);
    if (!image_sampler_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create image sampler: %s", SDL_GetError());
        SDL_ReleaseGPUTexture(dev, image_atlas_texture_);
        image_atlas_texture_ = nullptr;
        return false;
    }

    return true;
}

void GPUDriverSDL::beginFrame() {
    if (in_frame_) {
        SDL_Log("GPUDriverSDL::beginFrame: already in frame");
        return;
    }
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GPUDriverSDL::beginFrame: device not initialized");
        return;
    }

    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
    if (!current_cmd_buf_) {
        SDL_Log("GPUDriverSDL::beginFrame: failed to acquire command buffer");
        return;
    }

    in_frame_ = true;
}

void GPUDriverSDL::endFrame() {
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        return;
    }

    gpu_device_->submitCommandBuffer(current_cmd_buf_);
    current_cmd_buf_ = nullptr;
    in_frame_ = false;
}

bool GPUDriverSDL::ensureImageInAtlas(const std::string& src, ImageAtlasEntry& out_entry) {
    if (!gpu_device_ || !gpu_device_->isInitialized() || !image_atlas_texture_ || !current_cmd_buf_) {
        return false;
    }
    if (src.empty()) {
        return false;
    }

    auto it = image_atlas_entries_.find(src);
    if (it != image_atlas_entries_.end()) {
        out_entry = it->second;
        return true;
    }

    if (!image_resource_manager_) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: no image resource manager bound");
        return false;
    }

    std::vector<uint8_t> pixels;
    uint32_t img_w = 0;
    uint32_t img_h = 0;
    if (!image_resource_manager_->getImagePixelsRGBA(src, pixels, img_w, img_h)) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: failed to get pixels for '%s'", src.c_str());
        return false;
    }

    if (img_w == 0 || img_h == 0 || pixels.empty()) {
        return false;
    }

    if (img_w > image_atlas_width_ || img_h > image_atlas_height_) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: image too large for atlas (%u x %u)", img_w, img_h);
        return false;
    }

    // 简单行优先打包：从左到右填充，不够则换行
    if (atlas_cursor_x_ + img_w > image_atlas_width_) {
        atlas_cursor_x_ = 0;
        atlas_cursor_y_ += atlas_row_height_;
        atlas_row_height_ = 0;
    }

    if (atlas_cursor_y_ + img_h > image_atlas_height_) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: atlas is full, cannot place '%s'", src.c_str());
        return false;
    }

    Uint32 dst_x = atlas_cursor_x_;
    Uint32 dst_y = atlas_cursor_y_;
    atlas_cursor_x_ += img_w;
    atlas_row_height_ = std::max(atlas_row_height_, img_h);

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // 创建上传缓冲
    uint32_t stride = img_w * 4;
    uint32_t buffer_size = stride * img_h;

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = buffer_size;

    SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
    if (!transfer_buf) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: failed to create transfer buffer: %s", SDL_GetError());
        return false;
    }

    void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
    if (!mapped) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: failed to map transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        return false;
    }

    std::memcpy(mapped, pixels.data(), buffer_size);
    SDL_UnmapGPUTransferBuffer(dev, transfer_buf);

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(current_cmd_buf_);
    if (!copy_pass) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: failed to begin copy pass: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        return false;
    }

    SDL_GPUTextureTransferInfo tex_transfer{};
    tex_transfer.transfer_buffer = transfer_buf;
    tex_transfer.offset = 0;

    SDL_GPUTextureRegion region{};
    region.texture = image_atlas_texture_;
    region.mip_level = 0;
    region.layer = 0;
    region.x = dst_x;
    region.y = dst_y;
    region.z = 0;
    region.w = img_w;
    region.h = img_h;
    region.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);

    ImageAtlasEntry entry{};
    entry.width = img_w;
    entry.height = img_h;
    entry.u0 = static_cast<float>(dst_x) / static_cast<float>(image_atlas_width_);
    entry.v0 = static_cast<float>(dst_y) / static_cast<float>(image_atlas_height_);
    entry.u1 = static_cast<float>(dst_x + img_w) / static_cast<float>(image_atlas_width_);
    entry.v1 = static_cast<float>(dst_y + img_h) / static_cast<float>(image_atlas_height_);

    image_atlas_entries_[src] = entry;
    out_entry = entry;
    return true;
}

void GPUDriverSDL::execute(const GPUCommandList& commands) {
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_ || !window_) {
        SDL_Log("GPUDriverSDL::execute: invalid state");
        return;
    }

    SDL_GPUTexture* swapchain_texture = nullptr;
    Uint32 w = 0;
    Uint32 h = 0;

    SDL_GPUColorTargetInfo color_target{};
    SDL_GPURenderPass* pass = nullptr;

    for (const auto& cmd : commands.commands) {
        switch (cmd.type) {
        case GPUCommandType::BeginFrame:
            // beginFrame 已经处理命令缓冲了，这里忽略
            break;
        case GPUCommandType::EndFrame:
            // endFrame 由调用方负责，这里忽略
            break;
        case GPUCommandType::BeginPass: {
            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }

            if (!SDL_WaitAndAcquireGPUSwapchainTexture(
                    current_cmd_buf_,
                    window_,
                    &swapchain_texture,
                    &w,
                    &h)) {
                SDL_Log("GPUDriverSDL::execute: failed to acquire swapchain texture: %s", SDL_GetError());
                return;
            }

            color_target = {};
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

            pass = SDL_BeginGPURenderPass(
                current_cmd_buf_,
                &color_target,
                1,
                nullptr
            );

            if (!pass) {
                SDL_Log("GPUDriverSDL::execute: failed to begin render pass: %s", SDL_GetError());
                return;
            }
            break;
        }
        case GPUCommandType::EndPass:
            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }
            break;
        case GPUCommandType::DrawInstancedQuads: {
            if (!pass || !rect_pipeline_) {
                break;
            }

            struct RectUniformData {
                float rect[4];
                float color[4];
                float viewport[4];
            };

            RectUniformData u{};
            u.rect[0] = cmd.rect.x;
            u.rect[1] = cmd.rect.y;
            u.rect[2] = cmd.rect.width;
            u.rect[3] = cmd.rect.height;

            u.color[0] = cmd.color.r;
            u.color[1] = cmd.color.g;
            u.color[2] = cmd.color.b;
            u.color[3] = cmd.color.a;

            u.viewport[0] = static_cast<float>(w);
            u.viewport[1] = static_cast<float>(h);
            u.viewport[2] = 0.0f;
            u.viewport[3] = 0.0f;

            SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

            SDL_BindGPUGraphicsPipeline(pass, rect_pipeline_);
            SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            break;
        }
        case GPUCommandType::DrawRoundedRectQuad: {
            if (!pass || !round_rect_pipeline_) {
                break;
            }

            struct RoundRectUniformData {
                float rect[4];
                float radius[4];
                float viewport[4];
                float color[4];
            };

            RoundRectUniformData u{};
            u.rect[0] = cmd.rect.x;
            u.rect[1] = cmd.rect.y;
            u.rect[2] = cmd.rect.width;
            u.rect[3] = cmd.rect.height;

            u.radius[0] = cmd.radius;
            u.radius[1] = cmd.radius;
            u.radius[2] = cmd.radius;
            u.radius[3] = cmd.radius;

            u.viewport[0] = static_cast<float>(w);
            u.viewport[1] = static_cast<float>(h);
            u.viewport[2] = 0.0f;
            u.viewport[3] = 0.0f;

            u.color[0] = cmd.color.r;
            u.color[1] = cmd.color.g;
            u.color[2] = cmd.color.b;
            u.color[3] = cmd.color.a;

            SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

            SDL_BindGPUGraphicsPipeline(pass, round_rect_pipeline_);
            SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            break;
        }
        case GPUCommandType::DrawImageQuad: {
            if (!pass || !image_pipeline_ || !image_atlas_texture_ || !image_sampler_) {
                break;
            }
            if (cmd.image_src.empty()) {
                break;
            }

            ImageAtlasEntry entry{};
            if (!ensureImageInAtlas(cmd.image_src, entry)) {
                break;
            }

            struct ImageUniformData {
                float rect[4];
                float uv_rect[4];
                float viewport[4];
                float tint[4];
            };

            ImageUniformData u{};

            // 保持等比缩放（类似 object-fit: contain）：
            // 在 cmd.rect 指定的盒子内，按图片原始宽高比缩放，并居中显示
            const float img_w = static_cast<float>(entry.width);
            const float img_h = static_cast<float>(entry.height);
            const float dst_w = cmd.rect.width;
            const float dst_h = cmd.rect.height;

            float draw_x = cmd.rect.x;
            float draw_y = cmd.rect.y;
            float draw_w = dst_w;
            float draw_h = dst_h;

            if (img_w > 0.0f && img_h > 0.0f && dst_w > 0.0f && dst_h > 0.0f) {
                const float scale_x = dst_w / img_w;
                const float scale_y = dst_h / img_h;
                const float scale = (scale_x < scale_y) ? scale_x : scale_y;
                draw_w = img_w * scale;
                draw_h = img_h * scale;
                const float offset_x = (dst_w - draw_w) * 0.5f;
                const float offset_y = (dst_h - draw_h) * 0.5f;
                draw_x = cmd.rect.x + offset_x;
                draw_y = cmd.rect.y + offset_y;
            }

            u.rect[0] = draw_x;
            u.rect[1] = draw_y;
            u.rect[2] = draw_w;
            u.rect[3] = draw_h;

            u.uv_rect[0] = entry.u0;
            u.uv_rect[1] = entry.v0;
            u.uv_rect[2] = entry.u1;
            u.uv_rect[3] = entry.v1;

            u.viewport[0] = static_cast<float>(w);
            u.viewport[1] = static_cast<float>(h);
            u.viewport[2] = 0.0f;
            u.viewport[3] = 0.0f;

            u.tint[0] = 1.0f;
            u.tint[1] = 1.0f;
            u.tint[2] = 1.0f;
            u.tint[3] = cmd.opacity;

            SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

            SDL_BindGPUGraphicsPipeline(pass, image_pipeline_);

            SDL_GPUTextureSamplerBinding binding{};
            binding.texture = image_atlas_texture_;
            binding.sampler = image_sampler_;
            SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

            SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            break;
        }
        default:
            // 目前先忽略其他绘制类命令
            break;
        }
    }

    if (pass) {
        SDL_EndGPURenderPass(pass);
        pass = nullptr;
    }
}

} // namespace dong::render
