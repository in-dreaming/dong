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
#include <cstdlib>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace dong::render {

namespace {

// 直接传递 sRGB 颜色，不做 gamma 转换
// 因为 CSS 颜色本身就是 sRGB 空间，且 GPU 混合也在 sRGB 空间进行
void writeLinearColor(const Color& color, float out_rgba[4]) {
    out_rgba[0] = color.r;
    out_rgba[1] = color.g;
    out_rgba[2] = color.b;
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

        // 释放图层离屏渲染缓存纹理
        for (auto& entry : layer_render_targets_) {
            if (entry.texture) {
                SDL_ReleaseGPUTexture(dev, entry.texture);
                entry.texture = nullptr;
            }
        }
        layer_render_targets_.clear();
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
    // 直接输出 sRGB 颜色，不做 gamma 转换
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
    // 启用 alpha 混合，支持半透明背景
    color_desc.blend_state.enable_blend = true;
    color_desc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_desc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color_desc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    color_desc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

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
    // 直接输出 sRGB 颜色，不做 gamma 转换
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
    // 启用 alpha 混合，支持半透明背景和抗锯齿边缘
    color_desc_rr.blend_state.enable_blend = true;
    color_desc_rr.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_desc_rr.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc_rr.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color_desc_rr.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    color_desc_rr.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc_rr.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

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

    // 阴影绘制着色器：使用 SDF + 模糊半径实现柔和阴影边缘
    const char* kShadowVS = R"(
struct VSOutput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    nointerpolation float blur : TEXCOORD3;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD4;
};

cbuffer ShadowUniforms : register(b0, space1) {
    float4 uRect;
    float4 uRadius;   // x=corner radius, y=blur radius
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
    
    float blur = uRadius.y;
    // 扩展绘制区域以容纳模糊
    float2 expanded_rect_pos = uRect.xy - blur;
    float2 expanded_rect_size = uRect.zw + blur * 2.0;
    
    float2 pos = expanded_rect_pos + local * expanded_rect_size;
    float2 ndc;
    ndc.x = (pos.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pos.y / uViewport.y) * 2.0;
    
    float radius = uRadius.x;
    radius = min(radius, min(uRect.z, uRect.w) * 0.5 - 0.5);
    radius = max(radius, 0.0);
    
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    // local 坐标相对于原始矩形（不是扩展后的）
    o.local = (local * expanded_rect_size - blur) / uRect.zw;
    o.size = uRect.zw;
    o.radius = radius;
    o.blur = blur;
    o.color = uColor;
    o.pixel = pos;
    return o;
}
)";

    const char* kShadowFS = R"(
struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    nointerpolation float blur : TEXCOORD3;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD4;
};

cbuffer ShadowUniforms : register(b0, space1) {
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
    float blur = input.blur;
    float2 halfSize = size * 0.5;
    
    float dist = sdRoundRect(p, halfSize, r);
    
    // 使用 smoothstep 实现模糊效果
    // blur 为 0 时退化为硬边缘
    float sigma = max(blur, 0.5);
    float alpha = 1.0 - smoothstep(-sigma, sigma, dist);
    
    float4 base = input.color;
    base.a *= alpha;
    // 直接输出 sRGB 颜色，不做 gamma 转换
    return base;
}
)";

    shadow_vs_ = shader_manager_->loadShaderFromHLSL(
        "dong_shadow_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        kShadowVS,
        "main"
    );
    shadow_fs_ = shader_manager_->loadShaderFromHLSL(
        "dong_shadow_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        kShadowFS,
        "main"
    );

    if (!shadow_vs_ || !shadow_fs_) {
        SDL_Log("GPUDriverSDL::initialize: failed to compile shadow shaders");
        return false;
    }

    SDL_GPUGraphicsPipelineCreateInfo shadow_ci{};
    SDL_GPUColorTargetDescription color_desc_shadow{};
    color_desc_shadow.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    // 启用 alpha 混合，支持半透明阴影
    color_desc_shadow.blend_state.enable_blend = true;
    color_desc_shadow.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_desc_shadow.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc_shadow.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color_desc_shadow.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    color_desc_shadow.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc_shadow.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

    shadow_ci.target_info.num_color_targets = 1;
    shadow_ci.target_info.color_target_descriptions = &color_desc_shadow;
    shadow_ci.target_info.has_depth_stencil_target = false;

    shadow_ci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
    shadow_ci.vertex_shader = shadow_vs_;
    shadow_ci.fragment_shader = shadow_fs_;

    shadow_ci.vertex_input_state.num_vertex_buffers = 0;
    shadow_ci.vertex_input_state.vertex_buffer_descriptions = nullptr;
    shadow_ci.vertex_input_state.num_vertex_attributes = 0;
    shadow_ci.vertex_input_state.vertex_attributes = nullptr;

    shadow_pipeline_ = SDL_CreateGPUGraphicsPipeline(dev, &shadow_ci);
    if (!shadow_pipeline_) {
        SDL_Log("GPUDriverSDL::initialize: failed to create shadow pipeline: %s", SDL_GetError());
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
    // 直接在 sRGB 空间做 tint 乘法（简化处理）
    float3 tinted = tex.rgb * input.tint.rgb;
    float alpha = tex.a * input.tint.a;
    return float4(tinted, alpha);
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
    // 启用 alpha 混合，支持半透明图片
    color_desc2.blend_state.enable_blend = true;
    color_desc2.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    color_desc2.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc2.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    color_desc2.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    color_desc2.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    color_desc2.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

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
struct GlyphInstanceData {
    float4 rect;
    float4 uvRect;
    float4 color;
    float4 params;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
    float4 params : TEXCOORD2;
};

cbuffer TextUniforms : register(b0, space1) {
    float4 uViewport;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
    GlyphInstanceData uGlyphs[64];
};

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }

    GlyphInstanceData glyph = uGlyphs[instanceID];

    float2 pos = glyph.rect.xy + local * glyph.rect.zw;
    float2 ndc;
    ndc.x = (pos.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pos.y / uViewport.y) * 2.0;

    float2 uv = float2(
        lerp(glyph.uvRect.x, glyph.uvRect.z, local.x),
        lerp(glyph.uvRect.y, glyph.uvRect.w, local.y)
    );

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = uv;
    o.color = glyph.color;
    o.pixel = pos;
    o.params = glyph.params;
    return o;
}
)";

    const char* kTextFS = R"(
Texture2D msdfTexture : register(t0);
SamplerState msdfSampler : register(s0);

struct GlyphInstanceData {
    float4 rect;
    float4 uvRect;
    float4 color;
    float4 params;
};

cbuffer TextUniforms : register(b0, space1) {
    float4 uViewport;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
    GlyphInstanceData uGlyphs[64];
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
    float4 params : TEXCOORD2;
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

    // 基于 msdfgen + fwidth 的 MSDF 解码：
    // 1. params.x 传入屏幕空间 pxRange（像素）
    // 2. median 取 signed distance（0.5 为轮廓）并按 pxRange 缩放
    // 3. 使用 fwidth(dist) * inv_device_pixel_ratio 估算当前缩放下 1 逻辑像素的距离变化
    // 4. 按 dist / width + 0.5 做归一化，保证不同缩放与 DPI 下边缘宽度一致
    float3 msdf = msdfTexture.Sample(msdfSampler, input.uv).rgb;
    float sd = median(msdf.r, msdf.g, msdf.b);

    float pxRange = max(input.params.x, 1.0f);
    float invDPR = input.params.y;

    // 将 signed distance 映射到屏幕像素距离，并用 fwidth 估算当前缩放下的边缘宽度
    // 使用 msdfgen 官方约定：distanceSignCorrection 已经保证填充区域对应 (sd > 0.5)
    // 即 glyph 内部 median(msdf) > 0.5，外部 < 0.5。
    float screenPxDistance = (sd - 0.5f) * pxRange;
    float screenPxRange = max(fwidth(screenPxDistance) * max(invDPR, 1.0f / 8.0f), 1.0f / 256.0f);

    float subpixel = input.params.z;

    float alpha;

    if (subpixel > 0.5f) {
        // 简单 subpixel 路径：对 RGB 三个通道分别计算 coverage
        float3 dist_rgb = (msdf - 0.5f) * pxRange;
        float3 range_rgb = max(fwidth(dist_rgb) * max(invDPR, 1.0f / 8.0f), 1.0f / 256.0f);
        // 使用标准 MSDF 公式：dist / range + 0.5，然后 saturate 到 [0,1]
        float3 alpha_rgb = saturate(dist_rgb / range_rgb + 0.5f);
        alpha = max(alpha_rgb.r, max(alpha_rgb.g, alpha_rgb.b));
        if (alpha <= 0.0f) {
            discard;
        }
        // 颜色已经是 sRGB 空间，直接使用
        float3 srgbColor = input.color.rgb * alpha_rgb;
        float4 color;
        color.rgb = srgbColor;
        color.a = input.color.a * alpha;
        return color;
    } else {
        // 灰度 MSDF 路径
        // 使用标准 MSDF 公式：screenPxDistance / screenPxRange + 0.5
        float alpha_gray = saturate(screenPxDistance / screenPxRange + 0.5f);
        if (alpha_gray <= 0.0f) {
            discard;
        }
        float4 color;
        color.rgb = input.color.rgb;
        color.a = input.color.a * alpha_gray;
        return color;
    }
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

    // 初始化多级字形 Atlas（根据字号挑选）。
    // bitmap_px 表示单个 glyph 的 MSDF 纹理分辨率，
    // 这里采用 (32, 48, 72, 96) 四档，覆盖从小字号到标题字号的主流范围。
    // 未来如果需要支持更大字号，只需在此新增配置项即可。
    glyph_atlas_tiers_.clear();
    struct GlyphTierConfig {
        uint32_t bitmap_px;
        float distance_range;
    };
    // Atlas 分级只区分 bitmap 尺寸，distance_range 根据 bitmap 尺寸调整
    // 更大的 distance_range 可以提供更好的抗锯齿效果，但会占用更多的字形空间
    // 一般建议 distance_range 约为 bitmap 尺寸的 1/8 到 1/4
    const GlyphTierConfig tier_configs[] = {
        {32u, 4.0f},    // 正文/小号文本主力档位 (range = 32/8 = 4)
        {48u, 6.0f},    // 中号/UI 控件文本 (range = 48/8 = 6)
        {72u, 8.0f},    // 大号标题 (range = 72/9 = 8)
        {96u, 10.0f},   // 特大号标题或放大预览 (range = 96/9.6 ≈ 10)
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

    ++frame_index_;
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

void GPUDriverSDL::beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height) {
    if (in_frame_) {
        SDL_Log("GPUDriverSDL::beginFrameOffscreen: already in frame");
        return;
    }
    if (!gpu_device_ || !gpu_device_->isInitialized() || !target) {
        SDL_Log("GPUDriverSDL::beginFrameOffscreen: invalid parameters");
        return;
    }

    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
    if (!current_cmd_buf_) {
        SDL_Log("GPUDriverSDL::beginFrameOffscreen: failed to acquire command buffer");
        return;
    }

    ++frame_index_;

    // 保存纹理尺寸
    offscreen_width_ = width;
    offscreen_height_ = height;
    
    // 开始离屏渲染通道（清屏）
    SDL_GPUColorTargetInfo color_target{};
    color_target.texture = target;
    color_target.mip_level = 0;
    color_target.layer_or_depth_plane = 0;
    color_target.clear_color = SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f};  // 白色背景
    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(current_cmd_buf_, &color_target, 1, nullptr);
    if (pass) {
        SDL_EndGPURenderPass(pass);  // 先结束清除通道，后续 execute() 会开启新的通道
    }

    offscreen_target_ = target;
    in_frame_ = true;
}

void GPUDriverSDL::endFrameOffscreen() {
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        return;
    }

    gpu_device_->submitCommandBuffer(current_cmd_buf_);
    SDL_WaitForGPUIdle(gpu_device_->getHandle());  // 等待离屏渲染完成
    current_cmd_buf_ = nullptr;
    offscreen_target_ = nullptr;
    offscreen_width_ = 0;
    offscreen_height_ = 0;
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
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        SDL_Log("GPUDriverSDL::execute: invalid state");
        return;
    }

    SDL_GPUTexture* swapchain_texture = nullptr;
    Uint32 w = 0;
    Uint32 h = 0;
    
    // 判断是离屏渲染还是窗口渲染
    if (offscreen_target_) {
        // 离屏渲染模式
        swapchain_texture = offscreen_target_;
        w = offscreen_width_;
        h = offscreen_height_;
        SDL_Log("[GPUDriverSDL::execute] Offscreen mode: viewport = %u x %u", w, h);
    } else {
        // 窗口渲染模式
        if (!window_) {
            SDL_Log("GPUDriverSDL::execute: no window for swapchain rendering");
            return;
        }
        if (!SDL_AcquireGPUSwapchainTexture(current_cmd_buf_, window_, &swapchain_texture, &w, &h)) {
            SDL_Log("GPUDriverSDL::execute: failed to acquire swapchain texture");
            return;
        }
        if (!swapchain_texture) {
            SDL_Log("GPUDriverSDL::execute: swapchain texture is null");
            return;
        }
    }

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
        uint64_t id = 0;
        bool dirty = true;
        LayerRenderTarget* cache_entry = nullptr;
        float transform[6] = {1.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f};
        float scroll[2] = {0.0f, 0.0f};
    };

    std::vector<RenderTargetState> render_target_stack;
    std::vector<IsolatedLayerState> isolated_layer_stack;
    int skip_draw_depth = 0; // 大于 0 时，跳过图层内部的绘制命令，仅在 EndIsolatedLayer 处做合成

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

    struct PipelineBindingState {
        enum class ActivePipeline : uint8_t {
            None,
            Rect,
            RoundRect,
            Shadow,
            Image,
            Text,
        } active = ActivePipeline::None;

        bool image_sampler_bound = false;
        bool text_sampler_bound = false;

        void reset() {
            active = ActivePipeline::None;
            image_sampler_bound = false;
            text_sampler_bound = false;
        }
    };

    std::vector<ClipStackEntry> clip_stack;
    PipelineBindingState pipeline_state;
    uint32_t debug_layer_cache_rasterized = 0;
    uint32_t debug_layer_cache_reused = 0;

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
        // 当在非脏隔离图层内部时，跳过除 Begin/EndIsolatedLayer 以外的命令
        if (skip_draw_depth > 0 && cmd.type != GPUCommandType::BeginIsolatedLayer &&
            cmd.type != GPUCommandType::EndIsolatedLayer) {
            continue;
        }

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

            // 离屏渲染模式：swapchain_texture已经在execute()开头设置为offscreen_target_
            // 窗口渲染模式：swapchain_texture已经在execute()开头从窗口获取
            // 这里不应该再重新获取
            
            render_target_stack.clear();
            isolated_layer_stack.clear();
            render_target_stack.push_back(RenderTargetState{swapchain_texture, w, h, offscreen_target_ != nullptr});

            color_target = {};
            color_target.texture = swapchain_texture;
            color_target.mip_level = 0;
            color_target.layer_or_depth_plane = 0;
            
            // 离屏渲染：beginFrameOffscreen已经CLEAR过了，这里用LOAD
            // 窗口渲染：需要CLEAR黑色背景
            if (offscreen_target_) {
                color_target.clear_color = SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f};  // 离屏：白色（虽然会被LOAD忽略）
                color_target.load_op = SDL_GPU_LOADOP_LOAD;  // 保留之前clear的内容
            } else {
                color_target.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};  // 窗口：黑色
                color_target.load_op = SDL_GPU_LOADOP_CLEAR;
            }
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
            pipeline_state.reset();

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
            if (render_target_stack.empty()) {
                SDL_Log("GPUDriverSDL::execute: BeginIsolatedLayer without active render target");
                break;
            }

            SDL_GPUDevice* dev = gpu_device_ ? gpu_device_->getHandle() : nullptr;
            if (!dev) {
                SDL_Log("GPUDriverSDL::execute: no GPU device for isolated layer");
                break;
            }

            const uint64_t layer_id = cmd.layer_id;
            const bool layer_dirty = cmd.layer_dirty;

            auto parent_state = render_target_stack.back();
            Uint32 target_w = parent_state.width > 0 ? parent_state.width : w;
            Uint32 target_h = parent_state.height > 0 ? parent_state.height : h;
            if (target_w == 0) target_w = 1;
            if (target_h == 0) target_h = 1;

            LayerRenderTarget* cache_entry = nullptr;

            // 如果标记为非脏图层，尝试直接复用已有缓存，不切换 render target
            if (!layer_dirty && layer_id != 0) {
                for (auto& entry : layer_render_targets_) {
                    if (entry.layer_id == layer_id && entry.texture &&
                        entry.valid_for_cache && entry.width == target_w && entry.height == target_h) {
                        cache_entry = &entry;
                        break;
                    }
                }
            }

            IsolatedLayerState layer_state{};
            layer_state.bounds = cmd.rect;
            layer_state.opacity = cmd.layer_opacity;
            layer_state.id = layer_id;
            layer_state.dirty = layer_dirty;
            layer_state.cache_entry = cache_entry;
            for (int i = 0; i < 6; ++i) {
                layer_state.transform[i] = cmd.layer_transform[i];
            }
            layer_state.scroll[0] = cmd.layer_scroll[0];
            layer_state.scroll[1] = cmd.layer_scroll[1];

            if (!layer_dirty && cache_entry && cache_entry->texture) {
                // 非脏图层且有有效缓存：不切换 render target，仅记录状态并开始跳过内部绘制
                if (debug_log_layer_cache_) {
                    ++debug_layer_cache_reused;
                    SDL_Log("[GPUDriverSDL layer-cache] frame=%llu reuse layer id=%llu size=%ux%u bounds=(%.1f,%.1f,%.1f,%.1f)",
                            frame_index_,
                            static_cast<unsigned long long>(layer_id),
                            static_cast<unsigned int>(cache_entry->width),
                            static_cast<unsigned int>(cache_entry->height),
                            layer_state.bounds.x,
                            layer_state.bounds.y,
                            layer_state.bounds.width,
                            layer_state.bounds.height);
                }
                layer_state.texture = cache_entry->texture;
                layer_state.width = cache_entry->width;
                layer_state.height = cache_entry->height;
                isolated_layer_stack.push_back(layer_state);
                ++skip_draw_depth;
                break;
            }

            // 脏图层：需要重新栅格，切换到离屏纹理
            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }

            SDL_GPUTexture* layer_texture = nullptr;

            // 优先复用同一 layer_id 的缓存纹理
            if (layer_id != 0) {
                for (auto& entry : layer_render_targets_) {
                    if (entry.layer_id == layer_id && entry.texture && !entry.in_use) {
                        if (entry.width != target_w || entry.height != target_h) {
                            SDL_ReleaseGPUTexture(dev, entry.texture);
                            entry.texture = nullptr;
                            entry.valid_for_cache = false;
                            entry.width = target_w;
                            entry.height = target_h;
                        }
                        if (!entry.texture) {
                            SDL_GPUTextureCreateInfo tex_info{};
                            tex_info.type = SDL_GPU_TEXTURETYPE_2D;
                            tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
                            tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
                            tex_info.width = target_w;
                            tex_info.height = target_h;
                            tex_info.layer_count_or_depth = 1;
                            tex_info.num_levels = 1;
                            tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
                            entry.texture = SDL_CreateGPUTexture(dev, &tex_info);
                            if (!entry.texture) {
                                SDL_Log("GPUDriverSDL::execute: failed to recreate layer texture: %s", SDL_GetError());
                                break;
                            }
                        }
                        entry.in_use = true;
                        entry.valid_for_cache = false;
                        layer_texture = entry.texture;
                        cache_entry = &entry;
                        break;
                    }
                }
            }

            // 其次复用未绑定 layer_id 的空闲纹理
            if (!layer_texture) {
                for (auto& entry : layer_render_targets_) {
                    if (entry.layer_id == 0 && !entry.in_use && entry.texture &&
                        entry.width == target_w && entry.height == target_h) {
                        entry.in_use = true;
                        entry.valid_for_cache = false;
                        entry.layer_id = layer_id;
                        layer_texture = entry.texture;
                        cache_entry = &entry;
                        break;
                    }
                }
            }

            // 都没有的话，新建一个并加入池
            if (!layer_texture) {
                SDL_GPUTextureCreateInfo tex_info{};
                tex_info.type = SDL_GPU_TEXTURETYPE_2D;
                tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
                tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
                tex_info.width = target_w;
                tex_info.height = target_h;
                tex_info.layer_count_or_depth = 1;
                tex_info.num_levels = 1;
                tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

                layer_texture = SDL_CreateGPUTexture(dev, &tex_info);
                if (!layer_texture) {
                    SDL_Log("GPUDriverSDL::execute: failed to create layer texture: %s", SDL_GetError());
                    break;
                }

                LayerRenderTarget entry{};
                entry.texture = layer_texture;
                entry.width = target_w;
                entry.height = target_h;
                entry.layer_id = layer_id;
                entry.in_use = true;
                entry.valid_for_cache = false;
                layer_render_targets_.push_back(entry);
                cache_entry = &layer_render_targets_.back();
            }

            if (debug_log_layer_cache_) {
                const Uint32 log_width = target_w;
                const Uint32 log_height = target_h;
                ++debug_layer_cache_rasterized;
                SDL_Log("[GPUDriverSDL layer-cache] frame=%llu rasterize layer id=%llu size=%ux%u bounds=(%.1f,%.1f,%.1f,%.1f)",
                        frame_index_,
                        static_cast<unsigned long long>(layer_id),
                        static_cast<unsigned int>(log_width),
                        static_cast<unsigned int>(log_height),
                        layer_state.bounds.x,
                        layer_state.bounds.y,
                        layer_state.bounds.width,
                        layer_state.bounds.height);
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
                if (cache_entry) {
                    cache_entry->texture = nullptr;
                    cache_entry->in_use = false;
                    cache_entry->valid_for_cache = false;
                    cache_entry->layer_id = 0;
                }
                break;
            }

            apply_scissor(pass);
            pipeline_state.reset();

            layer_state.texture = layer_texture;
            layer_state.width = target_w;
            layer_state.height = target_h;
            layer_state.cache_entry = cache_entry;
            isolated_layer_stack.push_back(layer_state);
            break;
        }
        case GPUCommandType::EndIsolatedLayer: {
            if (isolated_layer_stack.empty()) {
                SDL_Log("GPUDriverSDL::execute: EndIsolatedLayer without matching layer");
                break;
            }

            IsolatedLayerState layer_info = isolated_layer_stack.back();
            isolated_layer_stack.pop_back();

            SDL_GPUDevice* dev = gpu_device_ ? gpu_device_->getHandle() : nullptr;

            // 如果这是一个非脏图层，我们之前没有切换 render target，也不需要结束子 pass
            if (!layer_info.dirty) {
                if (skip_draw_depth > 0) {
                    --skip_draw_depth;
                }

                SDL_GPUTexture* layer_texture = layer_info.texture;
                if (!pass || !image_pipeline_ || !image_sampler_ || !layer_texture) {
                    break;
                }

                if (layer_info.bounds.width <= 0.0f || layer_info.bounds.height <= 0.0f) {
                    break;
                }

                struct LayerCompositeUniforms {
                    float rect[4];
                    float uv_rect[4];
                    float viewport[4];
                    float tint[4];
                    ClipUniformBlock clip;
                };

                LayerCompositeUniforms u{};
                float sx = layer_info.transform[0];
                float sy = layer_info.transform[4];
                if (sx == 0.0f) sx = 1.0f;
                if (sy == 0.0f) sy = 1.0f;
                float tx = layer_info.transform[2];
                float ty = layer_info.transform[5];
                float draw_x = layer_info.bounds.x + tx;
                float draw_y = layer_info.bounds.y + ty;
                float draw_w = layer_info.bounds.width * sx;
                float draw_h = layer_info.bounds.height * sy;
                u.rect[0] = draw_x;
                u.rect[1] = draw_y;
                u.rect[2] = draw_w;
                u.rect[3] = draw_h;

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

                // 非脏图层的缓存纹理保持在池中，valid_for_cache 标志不变
                break;
            }

            // 脏图层：需要结束子 pass 并合成到父 render target
            if (render_target_stack.size() <= 1) {
                SDL_Log("GPUDriverSDL::execute: EndIsolatedLayer without matching layer render target");
                break;
            }

            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }

            RenderTargetState layer_target_state = render_target_stack.back();
            render_target_stack.pop_back();

            SDL_GPUTexture* layer_texture = layer_target_state.texture;

            if (!dev || !layer_texture) {
                break;
            }

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
                break;
            }

            apply_scissor(pass);

            if (image_pipeline_ && image_sampler_ &&
                layer_info.bounds.width > 0.0f && layer_info.bounds.height > 0.0f) {
                struct LayerCompositeUniforms {
                    float rect[4];
                    float uv_rect[4];
                    float viewport[4];
                    float tint[4];
                    ClipUniformBlock clip;
                };

                LayerCompositeUniforms u{};
                float sx = layer_info.transform[0];
                float sy = layer_info.transform[4];
                if (sx == 0.0f) sx = 1.0f;
                if (sy == 0.0f) sy = 1.0f;
                float tx = layer_info.transform[2];
                float ty = layer_info.transform[5];
                float draw_x = layer_info.bounds.x + tx;
                float draw_y = layer_info.bounds.y + ty;
                float draw_w = layer_info.bounds.width * sx;
                float draw_h = layer_info.bounds.height * sy;
                u.rect[0] = draw_x;
                u.rect[1] = draw_y;
                u.rect[2] = draw_w;
                u.rect[3] = draw_h;

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

            // 将本次渲染结果标记为可缓存，下次若 layer_dirty=false 可直接复用
            if (layer_info.cache_entry) {
                layer_info.cache_entry->in_use = false;
                layer_info.cache_entry->valid_for_cache = true;
            } else {
                // 未能关联到缓存条目的纹理仍按旧逻辑归还到池中
                for (auto& entry : layer_render_targets_) {
                    if (entry.texture == layer_texture) {
                        entry.in_use = false;
                        entry.valid_for_cache = true;
                        if (entry.layer_id == 0) {
                            entry.layer_id = layer_info.id;
                        }
                        break;
                    }
                }
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

            SDL_Log("[RECT] rect=(%.2f,%.2f,%.2f,%.2f) color=(%.3f,%.3f,%.3f,%.3f)",
                    cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height,
                    cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);

            writeLinearColor(cmd.color, u.color);

            write_viewport(u.viewport);
            fill_clip_uniform(u.clip);

            SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

            if (pipeline_state.active != PipelineBindingState::ActivePipeline::Rect) {
                SDL_BindGPUGraphicsPipeline(pass, rect_pipeline_);
                pipeline_state.active = PipelineBindingState::ActivePipeline::Rect;
            }
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

            SDL_Log("[RRECT] rect=(%.2f,%.2f,%.2f,%.2f) radius=%.2f color=(%.3f,%.3f,%.3f,%.3f)",
                    cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height,
                    cmd.radius,
                    cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);

            write_viewport(u.viewport);

            writeLinearColor(cmd.color, u.color);
            fill_clip_uniform(u.clip);

            SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

            if (pipeline_state.active != PipelineBindingState::ActivePipeline::RoundRect) {
                SDL_BindGPUGraphicsPipeline(pass, round_rect_pipeline_);
                pipeline_state.active = PipelineBindingState::ActivePipeline::RoundRect;
            }
            SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            break;
        }
        case GPUCommandType::DrawShadowQuad: {
            if (!pass || !shadow_pipeline_) {
                break;
            }

            struct ShadowUniformData {
                float rect[4];
                float radius[4];   // x=corner radius, y=blur radius
                float viewport[4];
                float color[4];
                ClipUniformBlock clip;
            };

            ShadowUniformData u{};
            u.rect[0] = cmd.rect.x;
            u.rect[1] = cmd.rect.y;
            u.rect[2] = cmd.rect.width;
            u.rect[3] = cmd.rect.height;

            u.radius[0] = cmd.radius;
            u.radius[1] = cmd.blur;
            u.radius[2] = 0.0f;
            u.radius[3] = 0.0f;

            write_viewport(u.viewport);

            writeLinearColor(cmd.color, u.color);
            fill_clip_uniform(u.clip);

            SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &u, sizeof(u));

            if (pipeline_state.active != PipelineBindingState::ActivePipeline::Shadow) {
                SDL_BindGPUGraphicsPipeline(pass, shadow_pipeline_);
                pipeline_state.active = PipelineBindingState::ActivePipeline::Shadow;
            }
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

            if (pipeline_state.active != PipelineBindingState::ActivePipeline::Image) {
                SDL_BindGPUGraphicsPipeline(pass, image_pipeline_);
                pipeline_state.active = PipelineBindingState::ActivePipeline::Image;
            }

            if (!pipeline_state.image_sampler_bound) {
                SDL_GPUTextureSamplerBinding binding{};
                binding.texture = image_atlas_texture_;
                binding.sampler = image_sampler_;
                SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
                pipeline_state.image_sampler_bound = true;
            }

            SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
            break;
        }
        case GPUCommandType::DrawText: {
            if (!pass || !text_pipeline_ || glyph_atlas_tiers_.empty() || cmd.glyphs.empty()) {
                break;
            }

            std::string font_path = !cmd.font_path.empty()
                ? cmd.font_path
                : resolveFontPath(cmd.font_family, cmd.font_weight);
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
            if (!glyph_atlas || !text_sampler_) {
                SDL_Log("GPUDriverSDL: glyph atlas unavailable");
                break;
            }

            const float atlas_range = glyph_tier->distance_range;
            const float gamma_correction = -2.2f;  // sRGB gamma 值
            const float pixel_scale = cmd.scale_to_pixels;

            if (pipeline_state.active != PipelineBindingState::ActivePipeline::Text) {
                SDL_BindGPUGraphicsPipeline(pass, text_pipeline_);
                pipeline_state.active = PipelineBindingState::ActivePipeline::Text;
            }

            constexpr int kMaxGlyphsPerBatch = 64;

            struct GlyphInstanceUniform {
                float rect[4];
                float uv_rect[4];
                float color[4];
                float params[4];
            };

            struct TextBatchUniformData {
                float viewport[4];
                ClipUniformBlock clip;
                GlyphInstanceUniform glyphs[kMaxGlyphsPerBatch];
            };

            struct PreparedGlyph {
                GlyphInstanceUniform instance;
                uint32_t atlas_page;
            };

            TextBatchUniformData batch_uniform{};
            write_viewport(batch_uniform.viewport);
            fill_clip_uniform(batch_uniform.clip);

            std::vector<PreparedGlyph> prepared;
            prepared.reserve(cmd.glyphs.size());

            // 第一步：遍历所有 glyph，确保它们被放入 atlas，并记录每个 glyph 属于哪一页
            for (const auto& glyph : cmd.glyphs) {
                if (glyph.glyph_id == 0) {
                    continue;
                }

                const AtlasEntry* entry = glyph_atlas->addGlyph(glyph.glyph_id, font_path);
                if (!entry) {
                    continue;
                }

                if (entry->metrics.width_units <= 0.0f || entry->metrics.height_units <= 0.0f ||
                    entry->u1 <= entry->u0 || entry->v1 <= entry->v0) {
                    continue;
                }

                const float msdf_scale = (entry->metrics.msdf_scale > 0.0f)
                    ? entry->metrics.msdf_scale
                    : 1.0f;
                
                // MSDF 纹理包含字形 + padding (range 像素)
                // 渲染时需要考虑 padding 的影响
                // padding 在 design units 中的大小 = range / msdf_scale
                const float padding_units = atlas_range / msdf_scale;
                
                // 字形渲染尺寸 = (字形尺寸 + 2 * padding) * pixel_scale
                const float glyph_w = (entry->metrics.width_units + 2.0f * padding_units) * pixel_scale;
                const float glyph_h = (entry->metrics.height_units + 2.0f * padding_units) * pixel_scale;
                
                // bearing 需要调整，因为 MSDF 纹理左下角有 padding
                // 原始 bearing_x 是从 pen position 到字形左边缘的距离
                // 现在需要从 pen position 到 MSDF 纹理左边缘的距离 = bearing_x - padding
                const float bearing_x_px = (entry->metrics.bearing_x_units - padding_units) * pixel_scale;
                const float bearing_y_px = (entry->metrics.bearing_y_units + padding_units) * pixel_scale;

                if (glyph_w <= 0.0f || glyph_h <= 0.0f) {
                    continue;
                }

                const float pen_x_px = glyph.pen_x_units * pixel_scale + cmd.baseline_x;
                const float pen_y_px = glyph.pen_y_units * pixel_scale + cmd.baseline_y;

                float glyph_x = pen_x_px + bearing_x_px;
                float glyph_y = pen_y_px - bearing_y_px;

                GlyphInstanceUniform inst{};
                inst.rect[0] = glyph_x;
                inst.rect[1] = glyph_y;
                inst.rect[2] = glyph_w;
                inst.rect[3] = glyph_h;

                inst.uv_rect[0] = entry->u0;
                inst.uv_rect[1] = entry->v0;
                inst.uv_rect[2] = entry->u1;
                inst.uv_rect[3] = entry->v1;

                SDL_Log("[TEXT] glyph=%u font='%s' page=%u pen=(%.2f,%.2f) rect=(%.2f,%.2f,%.2f,%.2f) uv=(%.4f,%.4f,%.4f,%.4f) metrics_w=%.1f h=%.1f bx=%.1f by=%.1f padding_units=%.2f pixel_scale=%.4f",
                        glyph.glyph_id,
                        font_path.c_str(),
                        entry->atlas_page,
                        pen_x_px,
                        pen_y_px,
                        glyph_x,
                        glyph_y,
                        glyph_w,
                        glyph_h,
                        entry->u0,
                        entry->v0,
                        entry->u1,
                        entry->v1,
                        entry->metrics.width_units,
                        entry->metrics.height_units,
                        entry->metrics.bearing_x_units,
                        entry->metrics.bearing_y_units,
                        padding_units,
                        pixel_scale);

                writeLinearColor(cmd.color, inst.color);

                const float px_range_screen = atlas_range * (pixel_scale / msdf_scale);

                inst.params[0] = px_range_screen;
                inst.params[1] = inv_device_pixel_ratio;
                inst.params[2] = msdf_subpixel_enabled_ ? 1.0f : 0.0f;
                inst.params[3] = gamma_correction;

                PreparedGlyph pg{};
                pg.instance = inst;
                pg.atlas_page = entry->atlas_page;
                prepared.push_back(pg);
            }

            if (prepared.empty()) {
                break;
            }

            // 第二步：按 atlas_page 分批绘制，每个批次绑定对应页纹理
            uint32_t page_count = glyph_atlas->getPageCount();
            for (uint32_t page_index = 0; page_index < page_count; ++page_index) {
                SDL_GPUTexture* atlas_texture = glyph_atlas->getAtlasTextureForPage(page_index);
                if (!atlas_texture) {
                    continue;
                }

                SDL_GPUTextureSamplerBinding binding{};
                binding.texture = atlas_texture;
                binding.sampler = text_sampler_;
                SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
                pipeline_state.text_sampler_bound = true;

                int glyphs_in_batch = 0;

                auto flush_batch = [&]() {
                    if (glyphs_in_batch <= 0) {
                        return;
                    }
                    SDL_PushGPUVertexUniformData(current_cmd_buf_, 0, &batch_uniform, sizeof(batch_uniform));
                    SDL_DrawGPUPrimitives(pass, 4, static_cast<Uint32>(glyphs_in_batch), 0, 0);
                    glyphs_in_batch = 0;
                };

                for (const auto& pg : prepared) {
                    if (pg.atlas_page != page_index) {
                        continue;
                    }

                    if (glyphs_in_batch == 0) {
                        // 首个 glyph 时刷新 viewport/clip，确保在多次 flush 时仍保持正确
                        write_viewport(batch_uniform.viewport);
                        fill_clip_uniform(batch_uniform.clip);
                    }

                    batch_uniform.glyphs[glyphs_in_batch] = pg.instance;
                    ++glyphs_in_batch;

                    if (glyphs_in_batch == kMaxGlyphsPerBatch) {
                        flush_batch();
                    }
                }

                flush_batch();
            }

            break;
        }
        default:
            // 目前先忽略其他绘制类命令
            break;
        }
    }

    if (debug_log_layer_cache_ && (debug_layer_cache_rasterized > 0 || debug_layer_cache_reused > 0)) {
        SDL_Log("[GPUDriverSDL layer-cache] frame=%llu summary: rasterize=%u, reuse=%u",
                frame_index_,
                static_cast<unsigned int>(debug_layer_cache_rasterized),
                static_cast<unsigned int>(debug_layer_cache_reused));
    }

    // Debug: 按 GPUCommandList 中的 draw_batches 做一次批次遍历并输出日志
    if (debug_log_draw_batches_ && !commands.draw_batches.empty()) {
        for (size_t i = 0; i < commands.draw_batches.size(); ++i) {
            const DrawBatchRange& batch = commands.draw_batches[i];
            SDL_Log("[GPUDriverSDL debug] draw batch %zu: type=%d, sort_key=0x%llx, count=%u, start=%u", 
                    i,
                    static_cast<int>(batch.type),
                    static_cast<unsigned long long>(batch.sort_key),
                    batch.count,
                    batch.start);
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

    // 将字号映射到“期望的 MSDF 像素分辨率”，再在现有档位中选择最近的一档：
    //   target_msdf_px ≈ ceil(font_px_size / 1.5)
    // 这样可以：
    //   - 小字号不会浪费过高分辨率；
    //   - 大字号也不会因为 atlas 过小而出现过度放大模糊。
    const float clamped_font_size = std::max(font_size, 1.0f);
    const float target_msdf_px_f = std::ceil(clamped_font_size / 1.5f);
    const uint32_t target_msdf_px = target_msdf_px_f > 0.0f
        ? static_cast<uint32_t>(target_msdf_px_f)
        : 16u;

    GlyphAtlasTier* best = nullptr;
    uint32_t best_error = std::numeric_limits<uint32_t>::max();

    for (auto& tier : glyph_atlas_tiers_) {
        const uint32_t tier_px = tier.bitmap_px;
        const uint32_t error = (tier_px > target_msdf_px)
            ? (tier_px - target_msdf_px)
            : (target_msdf_px - tier_px);
        if (error < best_error) {
            best_error = error;
            best = &tier;
        } else if (error == best_error && best && tier_px > best->bitmap_px) {
            // 误差相同时，偏向更高分辨率的一档，保证质量优先
            best = &tier;
        }
    }

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

std::unique_ptr<GPUDriver> CreateGPUDriver(
    GPUBackendType backend,
    GPUDevice* device,
    SDL_Window* window,
    ShaderManager* shader_manager
) {
    if (backend == GPUBackendType::SDL_GPU) {
        if (!device || !window || !shader_manager) {
            return nullptr;
        }
        auto driver = std::make_unique<GPUDriverSDL>(device, window, shader_manager);

        // 通过环境变量控制调试日志：
        //   DONG_DEBUG_DRAW_BATCHES=1       → 打印 draw_batches 日志
        //   DONG_DEBUG_LAYER_CACHE=1        → 打印图层缓存（重栅格 / 复用）日志
        if (const char* env_batches = std::getenv("DONG_DEBUG_DRAW_BATCHES")) {
            if (env_batches[0] == '1') {
                driver->setDebugLogDrawBatches(true);
            }
        }
        if (const char* env_layer_cache = std::getenv("DONG_DEBUG_LAYER_CACHE")) {
            if (env_layer_cache[0] == '1') {
                driver->setDebugLogLayerCache(true);
            }
        }
        if (const char* env_subpixel = std::getenv("DONG_MSDF_SUBPIXEL")) {
            if (env_subpixel[0] == '1') {
                driver->setMsdfSubpixelEnabled(true);
            }
        }

        return driver;
    }
    return nullptr;
}

} // namespace dong::render
