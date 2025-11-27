#include "gpu_driver_sdl.hpp"
#include "gpu_device.hpp"
#include "shader_manager.hpp"
#include "resource_manager.hpp"
#include "glyph_atlas.hpp"
#include "font_resolver.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace {

constexpr float kSRGBGamma = 2.2f;

float srgbChannelToLinear(float value) {
    if (value <= 0.04045f) {
        return value / 12.92f;
    }
    return std::pow((value + 0.055f) / 1.055f, 2.4f);
}

} // namespace

namespace dong::render {

namespace {

void writeLinearColor(const Color& color, float out_rgba[4]) {
    out_rgba[0] = ::srgbChannelToLinear(color.r);
    out_rgba[1] = ::srgbChannelToLinear(color.g);
    out_rgba[2] = ::srgbChannelToLinear(color.b);
    out_rgba[3] = color.a;
}

} // namespace

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

        // 释放 MSDF 文字渲染资源
        if (text_pipeline_) {
            SDL_ReleaseGPUGraphicsPipeline(dev, text_pipeline_);
            text_pipeline_ = nullptr;
        }
        if (text_vs_) {
            SDL_ReleaseGPUShader(dev, text_vs_);
            text_vs_ = nullptr;
        }
        if (text_fs_) {
            SDL_ReleaseGPUShader(dev, text_fs_);
            text_fs_ = nullptr;
        }
        if (text_sampler_) {
            SDL_ReleaseGPUSampler(dev, text_sampler_);
            text_sampler_ = nullptr;
        }
    }

    glyph_atlas_tiers_.clear();

    for (auto& entry : ft_face_cache_) {
        if (entry.second) {
            FT_Done_Face(entry.second);
        }
    }
    ft_face_cache_.clear();
    if (ft_library_) {
        FT_Done_FreeType(ft_library_);
        ft_library_ = nullptr;
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
    float2 pixel : TEXCOORD0;
};

cbuffer RectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
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
    o.pixel = pos;
    return o;
}
)";

    const char* kRectFS = R"(
struct PSInput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD0;
};

cbuffer RectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

float3 linearToSRGB(float3 col) {
    return pow(saturate(col), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    float2 q = abs(local) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

bool discardByClip(float2 px) {
    uint clipCount = (uint)uClipMeta.x;
    for (uint i = 0; i < clipCount; ++i) {
        float rad = uClipRadii[i];
        float4 rc = uClipRects[i];
        if (rad <= 0.0f) {
            if (px.x < rc.x || px.x > rc.z || px.y < rc.y || px.y > rc.w) {
                return true;
            }
            continue;
        }
        if (sdRoundedClip(px, rc, rad) > 0.0f) {
            return true;
        }
    }
    return false;
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    float3 srgb = linearToSRGB(input.color.rgb);
    return float4(srgb, input.color.a);
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
    float2 pixel : TEXCOORD3;
};

cbuffer RoundRectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
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
    o.pixel = pos;
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
    float2 pixel : TEXCOORD3;
};

cbuffer RoundRectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

float sdRoundRect(float2 p, float2 halfSize, float rad) {
    float2 q = abs(p) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    return sdRoundRect(local, halfSize, rad);
}

bool discardByClip(float2 px) {
    uint clipCount = (uint)uClipMeta.x;
    for (uint i = 0; i < clipCount; ++i) {
        float rad = uClipRadii[i];
        float4 rc = uClipRects[i];
        if (rad <= 0.0f) {
            if (px.x < rc.x || px.x > rc.z || px.y < rc.y || px.y > rc.w) {
                return true;
            }
            continue;
        }
        if (sdRoundedClip(px, rc, rad) > 0.0f) {
            return true;
        }
    }
    return false;
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    float2 size = input.size;
    float2 p = (input.local - 0.5) * size;
    float r = input.radius;
    float2 halfSize = size * 0.5;
    float dist = sdRoundRect(p, halfSize, r);
    const float aa = 1.0;
    float alpha = saturate(0.5 - dist / aa);
    float4 base = input.color;
    base.a *= alpha;
    base.rgb = pow(saturate(base.rgb), 1.0 / 2.2);
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
    float2 pixel : TEXCOORD1;
};

cbuffer ImageUniforms : register(b0, space1) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTint;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
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
    o.pixel = pos;
    return o;
}
)";

    const char* kImageFS = R"(
Texture2D imageTexture : register(t0);
SamplerState imageSampler : register(s0);

cbuffer ImageUniforms : register(b0, space1) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTint;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
    float2 pixel : TEXCOORD1;
};

float3 srgbToLinear(float3 col) {
    return pow(saturate(col), float3(2.2, 2.2, 2.2));
}

float3 linearToSRGB(float3 col) {
    return pow(saturate(col), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    float2 q = abs(local) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

bool discardByClip(float2 px) {
    uint clipCount = (uint)uClipMeta.x;
    for (uint i = 0; i < clipCount; ++i) {
        float rad = uClipRadii[i];
        float4 rc = uClipRects[i];
        if (rad <= 0.0f) {
            if (px.x < rc.x || px.x > rc.z || px.y < rc.y || px.y > rc.w) {
                return true;
            }
            continue;
        }
        if (sdRoundedClip(px, rc, rad) > 0.0f) {
            return true;
        }
    }
    return false;
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    float4 tex = imageTexture.Sample(imageSampler, input.uv);
    float3 linearSample = srgbToLinear(tex.rgb);
    float3 tinted = linearSample * input.tint.rgb;
    float3 srgbColor = linearToSRGB(tinted);
    float alpha = tex.a * input.tint.a;
    return float4(srgbColor, alpha);
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

    // MSDF 文字渲染着色器和管线
    const char* kTextVS = R"(
struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
};

cbuffer TextUniforms : register(b0, space1) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uColor;
    float4 uParams;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
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
    o.color = uColor;
    o.pixel = pos;
    return o;
}
)";

    const char* kTextFS = R"(
Texture2D msdfTexture : register(t0);
SamplerState msdfSampler : register(s0);

cbuffer TextUniforms : register(b0, space1) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uColor;
    float4 uParams;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
};

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float3 toLinear(float3 col, float gam) {
    return pow(saturate(col), float3(gam, gam, gam));
}

float3 toSRGB(float3 col, float gam) {
    float invGamma = 1.0 / gam;
    return pow(saturate(col), float3(invGamma, invGamma, invGamma));
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    float2 q = abs(local) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

bool discardByClip(float2 px) {
    uint clipCount = (uint)uClipMeta.x;
    for (uint i = 0; i < clipCount; ++i) {
        float rad = uClipRadii[i];
        float4 rc = uClipRects[i];
        if (rad <= 0.0f) {
            if (px.x < rc.x || px.x > rc.z || px.y < rc.y || px.y > rc.w) {
                return true;
            }
            continue;
        }
        if (sdRoundedClip(px, rc, rad) > 0.0f) {
            return true;
        }
    }
    return false;
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    // 最简单、最常用的 MSDF 解码：只依赖 sigDist 和 fwidth(sigDist)。
    float3 msdf = msdfTexture.Sample(msdfSampler, input.uv).rgb;
    float sd = median(msdf.r, msdf.g, msdf.b);
    float sigDist = sd - 0.5; // sigDist = 0 在轮廓线上

    float pxRange = uParams.x;   // 目前先保留参数，不直接参与计算
    float glyphScale = uParams.y;
    float subpixelMask = uParams.z;
    float gammaSigned = uParams.w;
    float gammaMagnitude = max(abs(gammaSigned), 1e-3);
    bool inputIsLinear = gammaSigned < 0.0;

    // 基于 sigDist 自身的导数估计当前缩放下的像素宽度
    float width = max(fwidth(sigDist), 1e-4);
    float opacity = saturate(sigDist / width + 0.5);

    float3 linearColor = inputIsLinear ? input.color.rgb : toLinear(input.color.rgb, gammaMagnitude);
    linearColor *= opacity;
    float3 srgbColor = toSRGB(linearColor, gammaMagnitude);

    float4 color = input.color;
    color.rgb = srgbColor;
    color.a *= opacity;
    return color;
}
)";

    text_vs_ = shader_manager_->loadShaderFromHLSL(
        "dong_text_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        kTextVS,
        "main"
    );
    text_fs_ = shader_manager_->loadShaderFromHLSL(
        "dong_text_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        kTextFS,
        "main"
    );

    if (!text_vs_ || !text_fs_) {
        SDL_Log("GPUDriverSDL::initialize: failed to compile text shaders");
        return false;
    }

    SDL_GPUGraphicsPipelineCreateInfo tpci{};
    SDL_GPUColorTargetDescription color_desc_text{};
    color_desc_text.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    color_desc_text.blend_state.enable_blend = true;
    color_desc_text.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_desc_text.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc_text.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color_desc_text.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    color_desc_text.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc_text.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

    tpci.target_info.num_color_targets = 1;
    tpci.target_info.color_target_descriptions = &color_desc_text;
    tpci.target_info.has_depth_stencil_target = false;

    tpci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    tpci.vertex_shader = text_vs_;
    tpci.fragment_shader = text_fs_;

    tpci.vertex_input_state.num_vertex_buffers = 0;
    tpci.vertex_input_state.vertex_buffer_descriptions = nullptr;
    tpci.vertex_input_state.num_vertex_attributes = 0;
    tpci.vertex_input_state.vertex_attributes = nullptr;

    text_pipeline_ = SDL_CreateGPUGraphicsPipeline(dev, &tpci);
    if (!text_pipeline_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create text pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_GPUSamplerCreateInfo text_sampler_info{};
    text_sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    text_sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    text_sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    text_sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
    text_sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
    text_sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;

    text_sampler_ = SDL_CreateGPUSampler(dev, &text_sampler_info);
    if (!text_sampler_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create text sampler: %s", SDL_GetError());
        return false;
    }

    // 初始化多级字形 Atlas（根据字号挑选）
    glyph_atlas_tiers_.clear();
    struct GlyphTierConfig {
        uint32_t bitmap_px;
        float distance_range;
    };
    // 更细粒度的atlas分级，range增大以提高画质
    const GlyphTierConfig tier_configs[] = {
        {16u, 4.0f},    // 小字号 (8-16px)
        {24u, 6.0f},    // 中字号 (17-24px)
        {32u, 8.0f},    // 大字号 (25-32px)  
        {48u, 10.0f},   // 特大字号 (33-48px)
    };

    for (const auto& cfg : tier_configs) {
        auto atlas = std::make_unique<GlyphAtlas>(gpu_device_);
        if (!atlas->initialize(2048, 2048, cfg.bitmap_px, cfg.distance_range)) {
            SDL_Log("GPUDriverSDL::initialize: failed to initialize glyph atlas tier %u", cfg.bitmap_px);
            return false;
        }
        GlyphAtlasTier tier{};
        tier.bitmap_px = cfg.bitmap_px;
        tier.distance_range = cfg.distance_range;
        tier.atlas = std::move(atlas);
        glyph_atlas_tiers_.push_back(std::move(tier));
    }

    if (FT_Init_FreeType(&ft_library_) != 0) {
        SDL_Log("GPUDriverSDL::initialize: failed to initialize FreeType library for caching");
        ft_library_ = nullptr;
        return false;
    }
    ft_face_cache_.clear();

    SDL_Log("GPUDriverSDL initialized successfully");
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

    struct RenderTargetState {
        SDL_GPUTexture* texture = nullptr;
        Uint32 width = 0;
        Uint32 height = 0;
        bool is_swapchain = false;
    };

    struct IsolatedLayerState {
        SDL_GPUTexture* texture = nullptr;
        Uint32 width = 0;
        Uint32 height = 0;
        Rect bounds;
        float opacity = 1.0f;
    };

    std::vector<RenderTargetState> render_target_stack;
    std::vector<IsolatedLayerState> isolated_layer_stack;

    auto current_target_dimensions = [&render_target_stack, &w, &h]() -> std::pair<Uint32, Uint32> {
        if (render_target_stack.empty()) {
            return {w, h};
        }
        return {render_target_stack.back().width, render_target_stack.back().height};
    };

    auto current_viewport = [&current_target_dimensions]() {
        auto [cw, ch] = current_target_dimensions();
        if (cw == 0) {
            cw = 1;
        }
        if (ch == 0) {
            ch = 1;
        }
        return std::pair<float, float>{static_cast<float>(cw), static_cast<float>(ch)};
    };

    auto write_viewport = [&current_viewport](float (&out)[4]) {
        auto [vw, vh] = current_viewport();
        out[0] = vw;
        out[1] = vh;
        out[2] = 0.0f;
        out[3] = 0.0f;
    };

    float device_pixel_ratio = 1.0f;
    if (window_) {
        int logical_w = 0;
        int logical_h = 0;
        int drawable_w = 0;
        int drawable_h = 0;
        SDL_GetWindowSize(window_, &logical_w, &logical_h);
        SDL_GetWindowSizeInPixels(window_, &drawable_w, &drawable_h);
        if (logical_w > 0 && drawable_w > 0) {
            device_pixel_ratio = static_cast<float>(drawable_w) / static_cast<float>(logical_w);
        }
    }
    const float inv_device_pixel_ratio = device_pixel_ratio > 0.0f ? (1.0f / device_pixel_ratio) : 1.0f;

    SDL_GPUColorTargetInfo color_target{};
    SDL_GPURenderPass* pass = nullptr;

    constexpr int kMaxRoundedClipUniforms = 4;

    struct ClipStackEntry {
        SDL_Rect scissor;
        bool has_rounded;
        Rect rounded_rect;
        float rounded_radius;
    };

    struct ClipUniformBlock {
        float clip_rects[kMaxRoundedClipUniforms][4];
        float clip_radii[kMaxRoundedClipUniforms];
        float clip_meta[4];
    };

    std::vector<ClipStackEntry> clip_stack;

    auto apply_scissor = [&](SDL_GPURenderPass* target_pass) {
        if (!target_pass) {
            return;
        }
        if (clip_stack.empty()) {
            SDL_SetGPUScissor(target_pass, nullptr);
        } else {
            SDL_SetGPUScissor(target_pass, &clip_stack.back().scissor);
        }
    };

    auto to_sdl_rect = [](const Rect& rect) {
        const float x0 = rect.x;
        const float y0 = rect.y;
        const float x1 = rect.x + rect.width;
        const float y1 = rect.y + rect.height;
        SDL_Rect result{};
        result.x = static_cast<int>(std::floor(x0));
        result.y = static_cast<int>(std::floor(y0));
        const int right = static_cast<int>(std::ceil(x1));
        const int bottom = static_cast<int>(std::ceil(y1));
        result.w = std::max(0, right - result.x);
        result.h = std::max(0, bottom - result.y);
        return result;
    };

    auto intersect_rect = [](const SDL_Rect& a, const SDL_Rect& b) {
        SDL_Rect out{};
        const int x = std::max(a.x, b.x);
        const int y = std::max(a.y, b.y);
        const int right = std::min(a.x + a.w, b.x + b.w);
        const int bottom = std::min(a.y + a.h, b.y + b.h);
        out.x = x;
        out.y = y;
        out.w = std::max(0, right - x);
        out.h = std::max(0, bottom - y);
        return out;
    };

    auto fill_clip_uniform = [&](ClipUniformBlock& block) {
        for (int i = 0; i < kMaxRoundedClipUniforms; ++i) {
            block.clip_rects[i][0] = 0.0f;
            block.clip_rects[i][1] = 0.0f;
            block.clip_rects[i][2] = 0.0f;
            block.clip_rects[i][3] = 0.0f;
            block.clip_radii[i] = 0.0f;
        }
        block.clip_meta[0] = 0.0f;
        block.clip_meta[1] = 0.0f;
        block.clip_meta[2] = 0.0f;
        block.clip_meta[3] = 0.0f;

        int clip_index = 0;
        for (const auto& entry : clip_stack) {
            if (!entry.has_rounded) {
                continue;
            }
            block.clip_rects[clip_index][0] = entry.rounded_rect.x;
            block.clip_rects[clip_index][1] = entry.rounded_rect.y;
            block.clip_rects[clip_index][2] = entry.rounded_rect.x + entry.rounded_rect.width;
            block.clip_rects[clip_index][3] = entry.rounded_rect.y + entry.rounded_rect.height;
            block.clip_radii[clip_index] = entry.rounded_radius;
            ++clip_index;
            if (clip_index >= kMaxRoundedClipUniforms) {
                break;
            }
        }

        block.clip_meta[0] = static_cast<float>(clip_index);
    };

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

            render_target_stack.clear();
            isolated_layer_stack.clear();
            render_target_stack.push_back(RenderTargetState{swapchain_texture, w, h, true});

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

            apply_scissor(pass);

            break;
        }
        case GPUCommandType::EndPass:
            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }
            break;
        case GPUCommandType::PushClipRect: {
            SDL_Rect clip = to_sdl_rect(cmd.rect);
            if (!clip_stack.empty()) {
                clip = intersect_rect(clip_stack.back().scissor, clip);
            }
            ClipStackEntry entry{};
            entry.scissor = clip;
            if (cmd.radius > 0.0f) {
                entry.has_rounded = true;
                entry.rounded_rect = cmd.rect;
                entry.rounded_radius = cmd.radius;
            }
            clip_stack.push_back(entry);
            apply_scissor(pass);
            break;
        }
        case GPUCommandType::PopClip: {
            if (!clip_stack.empty()) {
                clip_stack.pop_back();
            }
            apply_scissor(pass);
            break;
        }
        case GPUCommandType::BeginIsolatedLayer: {
            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }
            if (render_target_stack.empty()) {
                SDL_Log("GPUDriverSDL::execute: BeginIsolatedLayer without active render target");
                break;
            }

            auto parent_state = render_target_stack.back();
            Uint32 target_w = parent_state.width > 0 ? parent_state.width : w;
            Uint32 target_h = parent_state.height > 0 ? parent_state.height : h;
            if (target_w == 0) target_w = 1;
            if (target_h == 0) target_h = 1;

            SDL_GPUTextureCreateInfo tex_info{};
            tex_info.type = SDL_GPU_TEXTURETYPE_2D;
            tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
            tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
            tex_info.width = target_w;
            tex_info.height = target_h;
            tex_info.layer_count_or_depth = 1;
            tex_info.num_levels = 1;
            tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

            SDL_GPUDevice* dev = gpu_device_->getHandle();
            SDL_GPUTexture* layer_texture = dev ? SDL_CreateGPUTexture(dev, &tex_info) : nullptr;
            if (!layer_texture) {
                SDL_Log("GPUDriverSDL::execute: failed to create layer texture: %s", SDL_GetError());
                break;
            }

            render_target_stack.push_back(RenderTargetState{layer_texture, target_w, target_h, false});

            SDL_GPUColorTargetInfo layer_target{};
            layer_target.texture = layer_texture;
            layer_target.mip_level = 0;
            layer_target.layer_or_depth_plane = 0;
            layer_target.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f};
            layer_target.load_op = SDL_GPU_LOADOP_CLEAR;
            layer_target.store_op = SDL_GPU_STOREOP_STORE;

            pass = SDL_BeginGPURenderPass(current_cmd_buf_, &layer_target, 1, nullptr);
            if (!pass) {
                SDL_Log("GPUDriverSDL::execute: failed to begin layer pass: %s", SDL_GetError());
                SDL_ReleaseGPUTexture(dev, layer_texture);
                render_target_stack.pop_back();
                break;
            }

            apply_scissor(pass);

            IsolatedLayerState layer_state{};
            layer_state.texture = layer_texture;
            layer_state.width = target_w;
            layer_state.height = target_h;
            layer_state.bounds = cmd.rect;
            layer_state.opacity = cmd.layer_opacity;
            isolated_layer_stack.push_back(layer_state);
            break;
        }
        case GPUCommandType::EndIsolatedLayer: {
            if (render_target_stack.size() <= 1) {
                SDL_Log("GPUDriverSDL::execute: EndIsolatedLayer without matching layer");
                break;
            }

            SDL_GPUDevice* dev = gpu_device_->getHandle();

            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }

            RenderTargetState layer_target_state = render_target_stack.back();
            render_target_stack.pop_back();

            SDL_GPUTexture* layer_texture = layer_target_state.texture;

            if (isolated_layer_stack.empty()) {
                SDL_Log("GPUDriverSDL::execute: layer stack underflow");
                if (dev && layer_texture) {
                    SDL_ReleaseGPUTexture(dev, layer_texture);
                }
                break;
            }

            IsolatedLayerState layer_info = isolated_layer_stack.back();
            isolated_layer_stack.pop_back();

            RenderTargetState parent_state = render_target_stack.back();

            SDL_GPUColorTargetInfo parent_target{};
            parent_target.texture = parent_state.texture;
            parent_target.mip_level = 0;
            parent_target.layer_or_depth_plane = 0;
            parent_target.load_op = SDL_GPU_LOADOP_LOAD;
            parent_target.store_op = SDL_GPU_STOREOP_STORE;

            pass = SDL_BeginGPURenderPass(current_cmd_buf_, &parent_target, 1, nullptr);
            if (!pass) {
                SDL_Log("GPUDriverSDL::execute: failed to resume parent pass: %s", SDL_GetError());
                if (dev && layer_texture) {
                    SDL_ReleaseGPUTexture(dev, layer_texture);
                }
                break;
            }

            apply_scissor(pass);

            if (image_pipeline_ && image_sampler_ && layer_texture &&
                layer_info.bounds.width > 0.0f && layer_info.bounds.height > 0.0f) {
                struct LayerCompositeUniforms {
                    float rect[4];
                    float uv_rect[4];
                    float viewport[4];
                    float tint[4];
                    ClipUniformBlock clip;
                };

                LayerCompositeUniforms u{};
                u.rect[0] = layer_info.bounds.x;
                u.rect[1] = layer_info.bounds.y;
                u.rect[2] = layer_info.bounds.width;
                u.rect[3] = layer_info.bounds.height;

                float tex_w = static_cast<float>(layer_info.width);
                float tex_h = static_cast<float>(layer_info.height);
                if (tex_w <= 0.0f) tex_w = 1.0f;
                if (tex_h <= 0.0f) tex_h = 1.0f;
                u.uv_rect[0] = layer_info.bounds.x / tex_w;
                u.uv_rect[1] = layer_info.bounds.y / tex_h;
                u.uv_rect[2] = (layer_info.bounds.x + layer_info.bounds.width) / tex_w;
                u.uv_rect[3] = (layer_info.bounds.y + layer_info.bounds.height) / tex_h;

                write_viewport(u.viewport);

                u.tint[0] = 1.0f;
                u.tint[1] = 1.0f;
                u.tint[2] = 1.0f;
                u.tint[3] = layer_info.opacity;
                fill_clip_uniform(u.clip);

                SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

                SDL_BindGPUGraphicsPipeline(pass, image_pipeline_);

                SDL_GPUTextureSamplerBinding binding{};
                binding.texture = layer_texture;
                binding.sampler = image_sampler_;
                SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

                SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            }

            if (dev && layer_texture) {
                SDL_ReleaseGPUTexture(dev, layer_texture);
            }

            break;
        }
        case GPUCommandType::DrawInstancedQuads: {
            if (!pass || !rect_pipeline_) {
                break;
            }

            struct RectUniformData {
                float rect[4];
                float color[4];
                float viewport[4];
                ClipUniformBlock clip;
            };

            RectUniformData u{};
            u.rect[0] = cmd.rect.x;
            u.rect[1] = cmd.rect.y;
            u.rect[2] = cmd.rect.width;
            u.rect[3] = cmd.rect.height;

            writeLinearColor(cmd.color, u.color);

            write_viewport(u.viewport);
            fill_clip_uniform(u.clip);

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
                ClipUniformBlock clip;
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

            write_viewport(u.viewport);

            writeLinearColor(cmd.color, u.color);
            fill_clip_uniform(u.clip);

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
                ClipUniformBlock clip;
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

            write_viewport(u.viewport);

            u.tint[0] = 1.0f;
            u.tint[1] = 1.0f;
            u.tint[2] = 1.0f;
            u.tint[3] = cmd.opacity;
            fill_clip_uniform(u.clip);

            SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

            SDL_BindGPUGraphicsPipeline(pass, image_pipeline_);

            SDL_GPUTextureSamplerBinding binding{};
            binding.texture = image_atlas_texture_;
            binding.sampler = image_sampler_;
            SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

            SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            break;
        }
        case GPUCommandType::DrawText: {
            if (!pass || !text_pipeline_ || glyph_atlas_tiers_.empty() || cmd.glyphs.empty()) {
                break;
            }

            std::string font_path = !cmd.font_path.empty() ? cmd.font_path : resolveFontPath(cmd.font_family);
            if (font_path.empty()) {
                SDL_Log("GPUDriverSDL: no valid font found for family '%s'", cmd.font_family.c_str());
                break;
            }

            float font_size = cmd.font_size > 0.0f ? cmd.font_size : 16.0f;

            GlyphAtlasTier* glyph_tier = selectGlyphAtlasTier(font_size);
            if (!glyph_tier || !glyph_tier->atlas) {
                SDL_Log("GPUDriverSDL: no glyph atlas tier available");
                break;
            }
            GlyphAtlas* glyph_atlas = glyph_tier->atlas.get();
            SDL_GPUTexture* atlas_texture = glyph_atlas ? glyph_atlas->getAtlasTexture() : nullptr;
            if (!atlas_texture || !text_sampler_) {
                SDL_Log("GPUDriverSDL: glyph atlas texture unavailable");
                break;
            }
            float base_bitmap = static_cast<float>(glyph_tier->bitmap_px);
            float glyph_scale = (base_bitmap > 0.0f) ? (font_size / base_bitmap) : 1.0f;

            // 与 demo 对齐：range 仅作为 pxRange 传入 shader，
            // 由 shader 使用纹理导数和 fwidth(uv) 自动换算到屏幕像素距离。
            const float px_range = glyph_tier->distance_range;

            const float subpixel_flag = (font_size <= 18.0f) ? 1.0f : 0.0f;
            const float gamma_correction = -kSRGBGamma;

            SDL_GPUTextureSamplerBinding binding{};
            binding.texture = atlas_texture;
            binding.sampler = text_sampler_;
            SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
            SDL_BindGPUGraphicsPipeline(pass, text_pipeline_);

            struct TextUniformData {
                float rect[4];
                float uv_rect[4];
                float viewport[4];
                float color[4];
                float params[4];
                ClipUniformBlock clip;
            };

            int debug_count = 0;
            for (const auto& glyph : cmd.glyphs) {
                if (glyph.glyph_id == 0) {
                    continue;
                }

                const AtlasEntry* entry = glyph_atlas->addGlyph(glyph.glyph_id, font_path);
                if (!entry) {
                    continue;
                }

                if (entry->metrics.width <= 0.0f || entry->metrics.height <= 0.0f ||
                    entry->u1 <= entry->u0 || entry->v1 <= entry->v0) {
                    continue;
                }

                float glyph_x = glyph.pen_x + entry->metrics.bearing_x * glyph_scale;
                float glyph_y = glyph.pen_y - entry->metrics.bearing_y * glyph_scale;
                float glyph_w = std::max(entry->metrics.width * glyph_scale, 0.0f);
                float glyph_h = std::max(entry->metrics.height * glyph_scale, 0.0f);

                if (glyph_w <= 0.0f || glyph_h <= 0.0f) {
                    continue;
                }

                TextUniformData u{};
                u.rect[0] = glyph_x;
                u.rect[1] = glyph_y;
                u.rect[2] = glyph_w;
                u.rect[3] = glyph_h;

                u.uv_rect[0] = entry->u0;
                u.uv_rect[1] = entry->v0;
                u.uv_rect[2] = entry->u1;
                u.uv_rect[3] = entry->v1;

                write_viewport(u.viewport);

            writeLinearColor(cmd.color, u.color);

                // uParams.x = pxRange (msdfgen glyph_distance_range)
                // uParams.y = glyphScale = font_size / bitmap_px
                // 其余分量保留，以后可用于 subpixel/gamma 等扩展
                u.params[0] = px_range;
                u.params[1] = glyph_scale;
                u.params[2] = subpixel_flag;
                u.params[3] = gamma_correction;
                fill_clip_uniform(u.clip);

                SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));
                SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            }

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

GPUDriverSDL::GlyphAtlasTier* GPUDriverSDL::selectGlyphAtlasTier(float font_size) {
    if (glyph_atlas_tiers_.empty()) {
        return nullptr;
    }
    
    // 选择 <= font_size 的最大 atlas (放大渲染，glyph_scale >= 1.0)
    // 这样可以保留MSDF细节并通过放大获得清晰边缘
    GlyphAtlasTier* best = nullptr;
    uint32_t max_smaller_px = 0;
    
    for (auto& tier : glyph_atlas_tiers_) {
        if (tier.bitmap_px <= static_cast<uint32_t>(std::ceil(font_size))) {
            if (tier.bitmap_px > max_smaller_px) {
                max_smaller_px = tier.bitmap_px;
                best = &tier;
            }
        }
    }
    
    // 如果没有找到，使用最小的 atlas (会有明显放大)
    if (!best) {
        best = &glyph_atlas_tiers_.front();
    }
    
    return best;
}

FT_Face GPUDriverSDL::getOrCreateFace(const std::string& font_path, uint32_t pixel_size) {
    if (!ft_library_ || font_path.empty()) {
        return nullptr;
    }
    auto it = ft_face_cache_.find(font_path);
    if (it == ft_face_cache_.end()) {
        FT_Face face = nullptr;
        if (FT_New_Face(ft_library_, font_path.c_str(), 0, &face) != 0) {
            SDL_Log("GPUDriverSDL: failed to load font face '%s'", font_path.c_str());
            return nullptr;
        }
        ft_face_cache_[font_path] = face;
        it = ft_face_cache_.find(font_path);
    }
    FT_Face face = it->second;
    if (face) {
        FT_Set_Pixel_Sizes(face, 0, pixel_size);
    }
    return face;
}

} // namespace dong::render
