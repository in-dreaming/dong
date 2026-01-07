#include "gpu_driver_sdl.hpp"
#include "gpu_device.hpp"
#include "shader_manager.hpp"
#include "resource_manager.hpp"
#include "glyph_atlas.hpp"
#include "font_resolver.hpp"
#include "../core/log.h"
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
    // 注意：在析构时，外部设备可能已经被销毁，需要小心检查
    if (in_frame_ && gpu_device_ && gpu_device_->isInitialized() && current_cmd_buf_) {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
    }
    current_cmd_buf_ = nullptr;
    in_frame_ = false;

    // 检查设备是否仍然有效（可能已被外部销毁）
    if (gpu_device_ && gpu_device_->isInitialized()) {
        SDL_GPUDevice* dev = gpu_device_->getHandle();
        if (!dev) {
            // 设备已被外部销毁，跳过资源清理
            return;
        }
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

        // 释放中间渲染纹理
        if (intermediate_texture_) {
            SDL_ReleaseGPUTexture(dev, intermediate_texture_);
            intermediate_texture_ = nullptr;
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

    // 清理字形图集（不依赖 GPU 设备）
    glyph_atlas_tiers_.clear();

    // 清理 FreeType 资源（不依赖 GPU 设备）
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
    
    // 重置指针，避免悬空引用
    gpu_device_ = nullptr;
    window_ = nullptr;
    shader_manager_ = nullptr;
}

bool GPUDriverSDL::initialize() {
    if (!gpu_device_ || !gpu_device_->isInitialized() || !window_ || !shader_manager_) {
        SDL_Log("GPUDriverSDL::initialize: invalid device, window, or shader manager");
        return false;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    
    // 获取 swapchain 的实际格式，用于创建兼容的 pipeline
    // Windows D3D12/Vulkan 通常使用 B8G8R8A8，macOS Metal 使用 BGRA8 或 RGBA8
    SDL_GPUTextureFormat swapchain_format = SDL_GetGPUSwapchainTextureFormat(dev, window_);
    SDL_Log("GPUDriverSDL::initialize: swapchain format = %d", swapchain_format);
    
    // 对于离屏渲染，我们使用 R8G8B8A8_UNORM，因为它是最通用的格式
    // 但对于 swapchain 渲染，需要使用 swapchain 的实际格式
    // 为了简化，我们统一使用 R8G8B8A8_UNORM，因为：
    // 1. 离屏渲染纹理使用 R8G8B8A8_UNORM
    // 2. 中间纹理使用 R8G8B8A8_UNORM
    // 3. 最终 blit 到 swapchain 时会自动转换
    SDL_GPUTextureFormat pipeline_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

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

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer RectUniforms : register(b0, space3) {
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

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer RoundRectUniforms : register(b0, space3) {
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

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer ShadowUniforms : register(b0, space3) {
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
// 关键：Fragment shader 的纹理和采样器必须使用 space2
// Uniform buffer 必须使用 space3
Texture2D imageTexture : register(t0, space2);
SamplerState imageSampler : register(s0, space2);

cbuffer ImageUniforms : register(b0, space3) {
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
    float4 debug : TEXCOORD3;  // 调试信息
};

// SDL_GPU 规范：Vertex shader 的 uniform buffer 使用 space1
// 布局必须与 C++ TextBatchUniformData 完全一致
// 总大小 <= 4096 bytes 以兼容大多数 GPU
cbuffer TextUniforms : register(b0, space1) {
    float4 uViewport;           // offset 0, size 16
    float4 uClipRects[4];       // offset 16, size 64
    float4 uClipRadii;          // offset 80, size 16
    float4 uClipMeta;           // offset 96, size 16
    float4 uGlyphData[248];     // offset 112, size 3968 (62 glyphs * 4 float4)
};

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }

    // 每个 glyph 占用 4 个 float4 (rect, uvRect, color, params)
    // glyph 数据从 uGlyphData[0] 开始
    uint base = instanceID * 4;
    float4 rect = uGlyphData[base + 0];
    float4 uvRect = uGlyphData[base + 1];
    float4 color = uGlyphData[base + 2];
    float4 params = uGlyphData[base + 3];

    float2 pos = rect.xy + local * rect.zw;
    float2 ndc;
    ndc.x = (pos.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (pos.y / uViewport.y) * 2.0;

    float2 uv = float2(
        lerp(uvRect.x, uvRect.z, local.x),
        lerp(uvRect.y, uvRect.w, local.y)
    );

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = uv;
    o.color = color;
    o.pixel = pos;
    o.params = params;
    o.debug = rect;  // 调试用
    return o;
}
)";

    const char* kTextFS = R"(
// 关键：Fragment shader 的纹理和采样器必须使用 space2
// Uniform buffer 必须使用 space3
// 这是 SDL_GPU 对 SPIR-V/Vulkan 的要求
Texture2D msdfTexture : register(t0, space2);
SamplerState msdfSampler : register(s0, space2);

struct GlyphInstanceData {
    float4 rect;
    float4 uvRect;
    float4 color;
    float4 params;
};

// 布局必须与 Vertex Shader (space1) 和 C++ TextBatchUniformData 完全一致
cbuffer TextUniforms : register(b0, space3) {
    float4 uViewport;           // offset 0, size 16
    float4 uClipRects[4];       // offset 16, size 64
    float4 uClipRadii;          // offset 80, size 16
    float4 uClipMeta;           // offset 96, size 16
    float4 uGlyphData[248];     // offset 112, size 3968 (62 glyphs * 4 float4)
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
    float4 params : TEXCOORD2;
    float4 debug : TEXCOORD3;
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

// 使用 fwidth 动态计算 screenPxRange，实现更精确的抗锯齿
// params.x = 预计算的 screenPxRange（作为备用）
// params.y = distance_range / msdf_texture_size（用于 fwidth 计算）
float calcScreenPxRange(float2 uv, float precomputed, float unitRange) {
    // 使用 fwidth 计算 UV 在屏幕空间的变化率
    // fwidth(uv) 返回 uv 在相邻像素间的变化量
    float2 screenTexSize = float2(1.0, 1.0) / fwidth(uv);
    
    // unitRange = distance_range / texture_size
    // screenPxRange = unitRange * screenTexSize（取平均）
    float dynamicRange = 0.5 * (unitRange * screenTexSize.x + unitRange * screenTexSize.y);
    
    // 确保 screenPxRange 至少为 2.0，以获得良好的抗锯齿效果
    // msdfgen 官方建议 screenPxRange >= 2
    return max(max(dynamicRange, precomputed), 2.0);
}

// 计算 MSDF 的 opacity，使用改进的抗锯齿算法
float calcMSDFOpacity(float3 msdf, float screenPxRange) {
    float sd = median(msdf.r, msdf.g, msdf.b);
    float screenPxDistance = screenPxRange * (sd - 0.5);
    
    // 使用 smoothstep 实现更平滑的抗锯齿过渡
    // 扩展过渡范围以获得更柔和的边缘
    return smoothstep(-0.5, 0.5, screenPxDistance);
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    // 采样 MSDF 纹理
    float3 msdf = msdfTexture.Sample(msdfSampler, input.uv).rgb;
    float sd = median(msdf.r, msdf.g, msdf.b);
    
    // 使用 fwidth 动态计算 screenPxRange
    float precomputedRange = input.params.x;
    float unitRange = input.params.y;
    float screenPxRange = calcScreenPxRange(input.uv, precomputedRange, unitRange);
    
    // 计算 opacity
    float screenPxDistance = screenPxRange * (sd - 0.5);
    float opacity = smoothstep(-0.5, 0.5, screenPxDistance);
    
    // 输出颜色
    float4 result;
    result.rgb = input.color.rgb;
    result.a = input.color.a * opacity;
    
    return result;
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
    // 
    // 关键：screenPxRange = distance_range * (pixel_scale / msdf_scale)
    // msdfgen 官方要求 screenPxRange >= 1，最好 >= 2
    // 
    // 对于小字号（如 12px），pixel_scale 很小，所以需要更大的 distance_range
    // 来保证 screenPxRange >= 2
    //
    // 计算示例（Arial, units_per_em=2048, glyph_width≈1000）：
    // - 32px bitmap: msdf_scale ≈ (32 - 2*range) / 1000
    // - 12px 字号: pixel_scale = 12/2048 = 0.00586
    // - 要使 screenPxRange >= 2: range * (0.00586 / msdf_scale) >= 2
    // - 如果 range=8, msdf_scale=(32-16)/1000=0.016, screenPxRange=8*(0.00586/0.016)=2.93 ✓
    //
    // 增加 distance_range 会减少字形在 MSDF 纹理中的可用空间，
    // 但可以显著改善小字号的抗锯齿效果
    const GlyphTierConfig tier_configs[] = {
        {32u, 8.0f},    // 正文/小号文本：增大 range 以改善小字抗锯齿
        {48u, 8.0f},    // 中号/UI 控件文本
        {72u, 8.0f},    // 大号标题
        {96u, 10.0f},   // 特大号标题或放大预览
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
    
    // RenderTarget/图层合成调试日志默认关闭，可通过环境变量 DONG_DEBUG_RT=1 开启
    debug_rt_enabled_ = false;

    SDL_Log("GPUDriverSDL initialized successfully");
    return true;
}

void GPUDriverSDL::prepareResources(const GPUCommandList& commands) {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("[prepareResources] SKIP: gpu_device not ready");
        return;
    }

    DONG_LOG_VERBOSE("[prepareResources] START frame=%llu commands=%zu", frame_index_ + 1, commands.commands.size());

    // ========== 关键修复：在 beginFrame() 之前预处理所有 glyph ==========
    // 
    // 问题：在 Vulkan 中，当 render pass 正在进行时，不能上传纹理数据。
    // 纹理上传需要 copy pass，而 copy pass 和 render pass 不能同时进行。
    // 
    // 解决方案：在 beginFrame() 之前预先遍历所有 DrawText 命令，
    // 确保所有需要的 glyph 都已经上传到 atlas。这样纹理上传的 command buffer
    // 会在主渲染 command buffer 之前完成。
    //
    for (const auto& cmd : commands.commands) {
        if (cmd.type != GPUCommandType::DrawText) {
            continue;
        }
        if (cmd.glyphs.empty()) {
            continue;
        }

        // 确定字体路径
        std::string font_path = !cmd.font_path.empty()
            ? cmd.font_path
            : resolveFontPath(cmd.font_family, cmd.font_weight);
        if (font_path.empty()) {
            continue;
        }

        // 选择合适的 glyph atlas tier
        float font_size = cmd.font_size > 0.0f ? cmd.font_size : 16.0f;
        GlyphAtlasTier* glyph_tier = selectGlyphAtlasTier(font_size);
        if (!glyph_tier || !glyph_tier->atlas) {
            continue;
        }
        GlyphAtlas* glyph_atlas = glyph_tier->atlas.get();
        if (!glyph_atlas) {
            continue;
        }

        // 预先添加所有 glyph 到 atlas（这会触发 MSDF 生成和纹理上传）
        int glyph_count = 0;
        for (const auto& glyph : cmd.glyphs) {
            if (glyph.glyph_id == 0) {
                continue;
            }
            // 使用 glyph 自己的 font_path（支持字体回退），如果为空则使用默认字体
            // 这与 execute() 中的逻辑保持一致
            std::string glyph_font_path = !glyph.font_path.empty() 
                ? glyph.font_path 
                : font_path;
            // addGlyph 会检查缓存，如果已存在则直接返回
            glyph_atlas->addGlyph(glyph.glyph_id, glyph_font_path);
            glyph_count++;
        }
        DONG_LOG_DEBUG("[prepareResources] DrawText: font='%s' size=%.1f glyphs=%d tier=%upx", 
                font_path.c_str(), font_size, glyph_count, glyph_tier->bitmap_px);
    }
    DONG_LOG_DEBUG("[prepareResources] END frame=%llu", frame_index_ + 1);
}

void GPUDriverSDL::beginFrame() {
    DONG_LOG_DEBUG("[GPUDriverSDL::beginFrame] START frame=%llu in_frame=%d", frame_index_ + 1, in_frame_ ? 1 : 0);
    if (in_frame_) {
        DONG_LOG_WARN("GPUDriverSDL::beginFrame: already in frame");
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

    // 重置所有图层渲染目标的 in_use 标志，以便新帧可以复用缓存
    for (auto& entry : layer_render_targets_) {
        entry.in_use = false;
    }

    if (debug_rt_enabled_) {
        SDL_Log("[GPUDriverSDL::beginFrame] frame=%llu mode=window", frame_index_);
    }
}

void GPUDriverSDL::endFrame() {
    DONG_LOG_DEBUG("[GPUDriverSDL::endFrame] START frame=%llu in_frame=%d", frame_index_, in_frame_ ? 1 : 0);
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        return;
    }

    gpu_device_->submitCommandBuffer(current_cmd_buf_);
    DONG_LOG_DEBUG("[GPUDriverSDL::endFrame] command buffer submitted");
    current_cmd_buf_ = nullptr;
    in_frame_ = false;
}

void GPUDriverSDL::beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height) {
    if (in_frame_) {
        DONG_LOG_WARN("GPUDriverSDL::beginFrameOffscreen: already in frame");
        return;
    }
    if (!gpu_device_ || !gpu_device_->isInitialized() || !target) {
        DONG_LOG_ERROR("GPUDriverSDL::beginFrameOffscreen: invalid parameters");
        return;
    }

    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
    if (!current_cmd_buf_) {
        DONG_LOG_ERROR("GPUDriverSDL::beginFrameOffscreen: failed to acquire command buffer");
        return;
    }

    ++frame_index_;

    // 重置所有图层渲染目标的 in_use 标志，以便新帧可以复用缓存
    for (auto& entry : layer_render_targets_) {
        entry.in_use = false;
    }

    if (debug_rt_enabled_) {
        DONG_LOG_DEBUG("[GPUDriverSDL::beginFrameOffscreen] frame=%llu target=%p size=%ux%u", frame_index_, (void*)target, width, height);
    }

    // 保存纹理尺寸
    offscreen_width_ = width;
    offscreen_height_ = height;
    
    // 注意：不在这里清除纹理，而是在 execute() 的 BeginPass 中使用 LOADOP_CLEAR
    // 这样可以避免在某些驱动上 LOADOP_LOAD 的兼容性问题

    offscreen_target_ = target;
    in_frame_ = true;
}

void GPUDriverSDL::endFrameOffscreen() {
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        DONG_LOG_WARN("[GPUDriverSDL::endFrameOffscreen] Invalid state: in_frame=%d cmd_buf=%p gpu_device=%p",
                in_frame_, (void*)current_cmd_buf_, (void*)gpu_device_);
        return;
    }

    DONG_LOG_DEBUG("[GPUDriverSDL::endFrameOffscreen] Submitting command buffer %p", (void*)current_cmd_buf_);
    gpu_device_->submitCommandBuffer(current_cmd_buf_);
    SDL_WaitForGPUIdle(gpu_device_->getHandle());  // 等待离屏渲染完成
    DONG_LOG_DEBUG("[GPUDriverSDL::endFrameOffscreen] GPU idle, rendering complete");
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

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // 注意：glyph 预处理已移到 prepareResources()，必须在 beginFrame() 之前调用
    
    SDL_GPUTexture* real_swapchain_texture = nullptr;  // 真正的 swapchain 纹理，用于最后 blit（window 模式延后 acquire）
    SDL_GPUTexture* swapchain_texture = nullptr;       // 实际用于渲染的纹理（可能是中间纹理）
    Uint32 w = 0;
    Uint32 h = 0;
    bool use_intermediate = false;  // 是否使用中间纹理
    
    // 判断是离屏渲染还是窗口渲染
    if (offscreen_target_) {
        // 离屏渲染模式：直接使用目标纹理
        swapchain_texture = offscreen_target_;
        w = offscreen_width_;
        h = offscreen_height_;
        if (debug_rt_enabled_) {
            SDL_Log("[GPUDriverSDL::execute] frame=%llu mode=offscreen viewport=%ux%u", frame_index_, w, h);
        }
    } else {
        // 窗口渲染模式：使用中间纹理避免 swapchain 多次 render pass 问题
        if (!window_) {
            SDL_Log("GPUDriverSDL::execute: no window for swapchain rendering");
            return;
        }

        // 关键：不要在这里 AcquireGPUSwapchainTexture。
        // 因为隔离层切换可能会 split/submit command buffer，而 swapchain texture 只能在 acquire 它的 command buffer 上使用。
        // 我们会在 EndPass（blit 到 swapchain 之前）用 *当前* command buffer 再 acquire 一次。
        int win_w = 0;
        int win_h = 0;
        if (!SDL_GetWindowSizeInPixels(window_, &win_w, &win_h) || win_w <= 0 || win_h <= 0) {
            // fallback: 有些平台可能只支持逻辑尺寸
            if (!SDL_GetWindowSize(window_, &win_w, &win_h) || win_w <= 0 || win_h <= 0) {
                SDL_Log("GPUDriverSDL::execute: failed to query window size");
                return;
            }
        }
        w = static_cast<Uint32>(win_w);
        h = static_cast<Uint32>(win_h);
        
        // 创建或复用中间纹理
        if (!intermediate_texture_ || intermediate_width_ != w || intermediate_height_ != h) {
            if (intermediate_texture_) {
                SDL_ReleaseGPUTexture(dev, intermediate_texture_);
                intermediate_texture_ = nullptr;
            }
            
            SDL_GPUTextureCreateInfo tex_info{};
            tex_info.type = SDL_GPU_TEXTURETYPE_2D;
            tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
            tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
            tex_info.width = w;
            tex_info.height = h;
            tex_info.layer_count_or_depth = 1;
            tex_info.num_levels = 1;
            tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
            
            intermediate_texture_ = SDL_CreateGPUTexture(dev, &tex_info);
            if (!intermediate_texture_) {
                SDL_Log("GPUDriverSDL::execute: failed to create intermediate texture: %s", SDL_GetError());
                // 回退：直接渲染到 swapchain（此路径下不允许 split cmd buf）
                if (!SDL_AcquireGPUSwapchainTexture(current_cmd_buf_, window_, &real_swapchain_texture, &w, &h)) {
                    SDL_Log("GPUDriverSDL::execute: failed to acquire swapchain texture (fallback)");
                    return;
                }
                if (!real_swapchain_texture) {
                    SDL_Log("[GPUDriverSDL::execute] swapchain texture is null (fallback), skipping frame");
                    return;
                }
                swapchain_texture = real_swapchain_texture;
            } else {
                intermediate_width_ = w;
                intermediate_height_ = h;
                swapchain_texture = intermediate_texture_;
                use_intermediate = true;
                if (debug_rt_enabled_) {
                    SDL_Log("[GPUDriverSDL::execute] frame=%llu created intermediate texture %p size=%ux%u",
                            frame_index_, (void*)intermediate_texture_, w, h);
                }
            }
        } else {
            swapchain_texture = intermediate_texture_;
            use_intermediate = true;
        }
        
        if (debug_rt_enabled_) {
            SDL_Log("[GPUDriverSDL::execute] frame=%llu mode=window viewport=%ux%u swapchain=%p intermediate=%p use_intermediate=%d",
                    frame_index_, w, h, (void*)real_swapchain_texture, (void*)intermediate_texture_, use_intermediate ? 1 : 0);
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

    auto log_render_target_stack = [&](const char* prefix) {
        if (!debug_rt_enabled_) {
            return;
        }
        if (render_target_stack.empty()) {
            SDL_Log("%s frame=%llu rt_depth=0 (swapchain=%p)", prefix, frame_index_, (void*)swapchain_texture);
            return;
        }
        const RenderTargetState& top = render_target_stack.back();
        SDL_Log("%s frame=%llu rt_depth=%zu top_rt=%p size=%ux%u is_swapchain=%d", prefix,
                frame_index_, render_target_stack.size(), (void*)top.texture,
                top.width, top.height, top.is_swapchain ? 1 : 0);
    };

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
            // SDL_SetGPUScissor 不接受 nullptr，需要显式设置全屏 scissor
            // 使用当前 render target 的尺寸
            auto [cw, ch] = current_target_dimensions();
            SDL_Rect full_scissor{};
            full_scissor.x = 0;
            full_scissor.y = 0;
            full_scissor.w = static_cast<int>(cw);
            full_scissor.h = static_cast<int>(ch);
            if (debug_rt_enabled_) {
                SDL_Log("[apply_scissor] full_scissor: x=%d y=%d w=%d h=%d", 
                        full_scissor.x, full_scissor.y, full_scissor.w, full_scissor.h);
            }
            SDL_SetGPUScissor(target_pass, &full_scissor);
        } else {
            if (debug_rt_enabled_) {
                SDL_Log("[apply_scissor] clip_stack scissor: x=%d y=%d w=%d h=%d",
                        clip_stack.back().scissor.x, clip_stack.back().scissor.y,
                        clip_stack.back().scissor.w, clip_stack.back().scissor.h);
            }
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

    int cmd_index = 0;
    for (const auto& cmd : commands.commands) {
        // 调试：只输出 clip 相关命令
        if (cmd.type == GPUCommandType::PushClipRect) {
            DONG_LOG_DEBUG("[GPU Execute] cmd[%d] PushClipRect: y=%.1f h=%.1f skip_depth=%d", 
                    cmd_index, cmd.rect.y, cmd.rect.height, skip_draw_depth);
        } else if (cmd.type == GPUCommandType::PopClip) {
            DONG_LOG_DEBUG("[GPU Execute] cmd[%d] PopClip skip_depth=%d", cmd_index, skip_draw_depth);
        }
        cmd_index++;
        if (debug_rt_enabled_ && offscreen_target_) {
            SDL_Log("[GPUDriverSDL::execute] frame=%llu processing cmd type=%d skip_depth=%d",
                    frame_index_, static_cast<int>(cmd.type), skip_draw_depth);
        }
        
        // 当在非脏隔离图层内部时，跳过除 Begin/EndIsolatedLayer 和 BeginPass/EndPass 以外的命令
        // 注意：BeginPass/EndPass 必须始终执行，否则会导致 blit 操作被跳过
        if (skip_draw_depth > 0 && cmd.type != GPUCommandType::BeginIsolatedLayer &&
            cmd.type != GPUCommandType::EndIsolatedLayer &&
            cmd.type != GPUCommandType::BeginPass &&
            cmd.type != GPUCommandType::EndPass) {
            if (debug_rt_enabled_) {
                SDL_Log("[GPUDriverSDL::execute] frame=%llu skip cmd type=%d due_to_non_dirty_layer depth=%d",
                        frame_index_, static_cast<int>(cmd.type), skip_draw_depth);
            }
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
            clip_stack.clear();  // 修复：清空 clip_stack，避免残留的 clip rect 影响新帧
            // 修复 is_swapchain 的逻辑：当没有离屏目标时，就是 swapchain
            render_target_stack.push_back(RenderTargetState{swapchain_texture, w, h, offscreen_target_ == nullptr});

            if (debug_rt_enabled_) {
                SDL_Log("[GPUDriverSDL::execute] BeginPass frame=%llu swapchain_texture=%p is_swapchain=%d viewport=%ux%u",
                        frame_index_, (void*)swapchain_texture, offscreen_target_ == nullptr ? 1 : 0, w, h);
            }

            color_target = {};
            color_target.texture = swapchain_texture;
            color_target.mip_level = 0;
            color_target.layer_or_depth_plane = 0;
            
            // 统一使用 LOADOP_CLEAR，避免 LOADOP_LOAD 在某些驱动上的兼容性问题
            // 离屏渲染：白色背景
            // 窗口渲染：黑色背景（通过中间纹理）
            if (offscreen_target_) {
                color_target.clear_color = SDL_FColor{1.0f, 1.0f, 1.0f, 1.0f};  // 离屏：白色
            } else {
                color_target.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f};  // 窗口：黑色
            }
            color_target.load_op = SDL_GPU_LOADOP_CLEAR;  // 始终使用 CLEAR
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

            // 设置 viewport - 在某些后端（如 Vulkan）上必须显式设置
            SDL_GPUViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.w = static_cast<float>(w);
            viewport.h = static_cast<float>(h);
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;
            SDL_SetGPUViewport(pass, &viewport);
            
            apply_scissor(pass);
            pipeline_state.reset();

            break;
        }
        case GPUCommandType::EndPass:
            if (debug_rt_enabled_) {
                SDL_Log("[GPUDriverSDL::execute] frame=%llu EndPass: pass=%p offscreen=%d use_intermediate=%d",
                        frame_index_, (void*)pass, offscreen_target_ != nullptr ? 1 : 0, use_intermediate ? 1 : 0);
            }
            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }
            
            // 如果使用了中间纹理，将其 blit 到真正的 swapchain
            if (use_intermediate && intermediate_texture_) {
                // window 模式：此时才 acquire swapchain texture（必须用当前 command buffer）
                if (!offscreen_target_) {
                    Uint32 sw = 0, sh = 0;
                    DONG_LOG_DEBUG("[GPUDriverSDL] EndPass acquiring swapchain (window mode) frame=%llu cmd_buf=%p win_size=%ux%u",
                            frame_index_, (void*)current_cmd_buf_, w, h);
                    if (!SDL_AcquireGPUSwapchainTexture(current_cmd_buf_, window_, &real_swapchain_texture, &sw, &sh)) {
                        SDL_Log("GPUDriverSDL::execute: failed to acquire swapchain texture at EndPass");
                        break;
                    }
                    if (!real_swapchain_texture) {
                        SDL_Log("[GPUDriverSDL::execute] swapchain texture is null at EndPass, skipping blit");
                        break;
                    }
                    DONG_LOG_DEBUG("[GPUDriverSDL] EndPass acquired swapchain (window mode) frame=%llu swapchain_tex=%p swapchain_size=%ux%u",
                            frame_index_, (void*)real_swapchain_texture, sw, sh);
                    // 如果尺寸不一致，取交集尺寸，避免越界
                    if (sw != w || sh != h) {
                        const Uint32 new_w = (sw < w) ? sw : w;
                        const Uint32 new_h = (sh < h) ? sh : h;
                        if (debug_rt_enabled_) {
                            SDL_Log("[GPUDriverSDL::execute] EndPass: swapchain size mismatch win=%ux%u swapchain=%ux%u using=%ux%u",
                                    w, h, sw, sh, new_w, new_h);
                        }
                        w = new_w;
                        h = new_h;
                    }
                }

                if (!real_swapchain_texture) {
                    break;
                }
                if (debug_rt_enabled_) {
                    SDL_Log("[GPUDriverSDL::execute] frame=%llu EndPass: blit intermediate=%p to swapchain=%p",
                            frame_index_, (void*)intermediate_texture_, (void*)real_swapchain_texture);
                }
                
                // 使用 blit 将中间纹理复制到 swapchain
                SDL_GPUBlitInfo blit_info{};
                blit_info.source.texture = intermediate_texture_;
                blit_info.source.mip_level = 0;
                blit_info.source.layer_or_depth_plane = 0;
                blit_info.source.x = 0;
                blit_info.source.y = 0;
                blit_info.source.w = w;
                blit_info.source.h = h;
                
                blit_info.destination.texture = real_swapchain_texture;
                blit_info.destination.mip_level = 0;
                blit_info.destination.layer_or_depth_plane = 0;
                blit_info.destination.x = 0;
                blit_info.destination.y = 0;
                blit_info.destination.w = w;
                blit_info.destination.h = h;
                
                blit_info.load_op = SDL_GPU_LOADOP_DONT_CARE;
                blit_info.filter = SDL_GPU_FILTER_NEAREST;
                blit_info.cycle = false;
                
                SDL_BlitGPUTexture(current_cmd_buf_, &blit_info);
            }
            break;
        case GPUCommandType::PushClipRect: {
            DONG_LOG_DEBUG("[GPU Execute] ENTER PushClipRect case");
            SDL_Rect clip = to_sdl_rect(cmd.rect);
            DONG_LOG_DEBUG("[GPU] PushClipRect: x=%.1f y=%.1f w=%.1f h=%.1f -> SDL x=%d y=%d w=%d h=%d",
                    cmd.rect.x, cmd.rect.y, cmd.rect.width, cmd.rect.height,
                    clip.x, clip.y, clip.w, clip.h);
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

            // 始终打印 BeginIsolatedLayer 日志
            // SDL_Log("[GPU Execute] BeginIsolatedLayer: layer_id=%llu layer_dirty=%d skip_depth=%d",
            //         static_cast<unsigned long long>(cmd.layer_id), cmd.layer_dirty ? 1 : 0, skip_draw_depth);

            if (debug_rt_enabled_) {
                SDL_Log("[GPUDriverSDL::execute] BeginIsolatedLayer frame=%llu layer_id=%llu layer_dirty=%d skip_depth=%d",
                        frame_index_, static_cast<unsigned long long>(cmd.layer_id), cmd.layer_dirty ? 1 : 0,
                        skip_draw_depth);
                log_render_target_stack("  RT before BeginIsolatedLayer");
            }
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

            // 优先检查是否有有效的缓存可以复用（无论 layer_dirty 标志如何）
            // 这样可以解决缓存的命令列表中 layer_dirty 标志不更新的问题
            if (layer_id != 0) {
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

            // 只有当图层非脏且有有效缓存时才复用缓存
            // 当 layer_dirty=true 时，必须重新绘制（例如滚动容器内容变化）
            if (layer_cache_enabled_ && !layer_dirty && cache_entry && cache_entry->texture) {
                // 有有效缓存：不切换 render target，仅记录状态并开始跳过内部绘制
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
                // 复用缓存时，标记为非脏，这样 EndIsolatedLayer 会走缓存合成路径
                layer_state.dirty = false;
                isolated_layer_stack.push_back(layer_state);
                ++skip_draw_depth;
                break;
            }

            // 脏图层：需要重新栅格，切换到离屏纹理
            if (pass) {
                SDL_EndGPURenderPass(pass);
                pass = nullptr;
            }

            // Workaround: 在隔离层切换时拆分 command buffer，避免同一纹理在同一 command buffer 内多次 Begin/End render pass 导致内容丢失
            // 目前仅在离屏渲染模式启用（窗口 swapchain texture 与 command buffer 绑定，拆分需要额外处理 AcquireGPUSwapchainTexture）。
            if (split_cmd_buf_for_isolated_layers_) {
                DONG_LOG_DEBUG("[GPUDriverSDL] Split command buffer for isolated layer frame=%llu where=BeginIsolatedLayer(before_layer_pass) offscreen=%d cmd_buf_old=%p",
                        frame_index_, offscreen_target_ ? 1 : 0, (void*)current_cmd_buf_);

                if ((offscreen_target_ || use_intermediate) && gpu_device_ && current_cmd_buf_) {
                    gpu_device_->submitCommandBuffer(current_cmd_buf_);
                    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
                    if (!current_cmd_buf_) {
                        SDL_Log("GPUDriverSDL::execute: failed to acquire command buffer after split");
                        return;
                    }
                }

                DONG_LOG_DEBUG("[GPUDriverSDL] Split command buffer done frame=%llu where=BeginIsolatedLayer(before_layer_pass) cmd_buf_new=%p",
                        frame_index_, (void*)current_cmd_buf_);
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

            // 设置 viewport - 在某些后端（如 Vulkan）上必须显式设置
            {
                SDL_GPUViewport layer_viewport{};
                layer_viewport.x = 0.0f;
                layer_viewport.y = 0.0f;
                layer_viewport.w = static_cast<float>(target_w);
                layer_viewport.h = static_cast<float>(target_h);
                layer_viewport.min_depth = 0.0f;
                layer_viewport.max_depth = 1.0f;
                SDL_SetGPUViewport(pass, &layer_viewport);
            }
            
            apply_scissor(pass);
            pipeline_state.reset();

            // 本帧已经为该图层开启了单独的 render pass，视为"脏图层"
            // 确保 EndIsolatedLayer 走重栅格路径，而不是复用缓存路径
            layer_state.dirty = true;
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

            // Workaround: 在隔离层切换时拆分 command buffer，避免同一纹理在同一 command buffer 内多次 Begin/End render pass 导致内容丢失
            // 目前仅在离屏渲染模式启用（窗口 swapchain texture 与 command buffer 绑定，拆分需要额外处理 AcquireGPUSwapchainTexture）。
            if (split_cmd_buf_for_isolated_layers_) {
                DONG_LOG_DEBUG("[GPUDriverSDL] Split command buffer for isolated layer frame=%llu where=EndIsolatedLayer(before_parent_pass) offscreen=%d cmd_buf_old=%p",
                        frame_index_, offscreen_target_ ? 1 : 0, (void*)current_cmd_buf_);

                if ((offscreen_target_ || use_intermediate) && gpu_device_ && current_cmd_buf_) {
                    gpu_device_->submitCommandBuffer(current_cmd_buf_);
                    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
                    if (!current_cmd_buf_) {
                        SDL_Log("GPUDriverSDL::execute: failed to acquire command buffer after split");
                        return;
                    }
                }

                DONG_LOG_DEBUG("[GPUDriverSDL] Split command buffer done frame=%llu where=EndIsolatedLayer(before_parent_pass) cmd_buf_new=%p",
                        frame_index_, (void*)current_cmd_buf_);
            }

            RenderTargetState child_target = render_target_stack.back();
            render_target_stack.pop_back();

            RenderTargetState& parent_target = render_target_stack.back();

            if (debug_rt_enabled_) {
                SDL_Log("[GPUDriverSDL::execute] EndIsolatedLayer frame=%llu layer_id=%llu dirty=%d compositing child_rt=%p -> parent_rt=%p",
                        frame_index_, static_cast<unsigned long long>(layer_info.id), layer_info.dirty ? 1 : 0,
                        (void*)child_target.texture, (void*)parent_target.texture);
                log_render_target_stack("  RT at EndIsolatedLayer");
            }

            // 重新开启父 render target 的 pass，用于合成子图层纹理
            SDL_GPUColorTargetInfo color_target{};
            color_target.texture = parent_target.texture;
            color_target.mip_level = 0;
            color_target.layer_or_depth_plane = 0;
            color_target.load_op = SDL_GPU_LOADOP_LOAD;
            color_target.store_op = parent_target.is_swapchain ? SDL_GPU_STOREOP_STORE : SDL_GPU_STOREOP_STORE;

            pass = SDL_BeginGPURenderPass(current_cmd_buf_, &color_target, 1, nullptr);
            if (!pass) {
                SDL_Log("GPUDriverSDL::execute: failed to begin parent render pass for EndIsolatedLayer");
                break;
            }

            // 设置 viewport - 在某些后端（如 Vulkan）上必须显式设置
            {
                SDL_GPUViewport parent_viewport{};
                parent_viewport.x = 0.0f;
                parent_viewport.y = 0.0f;
                parent_viewport.w = static_cast<float>(parent_target.width);
                parent_viewport.h = static_cast<float>(parent_target.height);
                parent_viewport.min_depth = 0.0f;
                parent_viewport.max_depth = 1.0f;
                SDL_SetGPUViewport(pass, &parent_viewport);
            }
            
            apply_scissor(pass);
            pipeline_state.reset();

            if (!image_pipeline_ || !image_sampler_ || !child_target.texture) {
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

            float tex_w = static_cast<float>(child_target.width);
            float tex_h = static_cast<float>(child_target.height);
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
            binding.texture = child_target.texture;
            binding.sampler = image_sampler_;
            SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

            SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);

            // 合成完成后，子 render target 保留在缓存池以便复用
            if (layer_info.cache_entry) {
                layer_info.cache_entry->valid_for_cache = true;
            }

            break;
        }
        case GPUCommandType::DrawInstancedQuads: {
            if (!pass || !rect_pipeline_) {
                SDL_Log("[RECT] SKIP: pass=%p rect_pipeline_=%p", (void*)pass, (void*)rect_pipeline_);
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
                DONG_LOG_WARN("[DrawText] EARLY EXIT: pass=%p text_pipeline=%p tiers_empty=%d glyphs_empty=%d",
                        (void*)pass, (void*)text_pipeline_, glyph_atlas_tiers_.empty() ? 1 : 0, cmd.glyphs.empty() ? 1 : 0);
                break;
            }

            DONG_LOG_DEBUG("[DrawText] frame=%llu glyphs_count=%zu baseline=(%.2f,%.2f) font_size=%.1f",
                    frame_index_, cmd.glyphs.size(), cmd.baseline_x, cmd.baseline_y, cmd.font_size);

            // 默认字体路径（用于没有指定 font_path 的 glyph）
            std::string default_font_path = !cmd.font_path.empty()
                ? cmd.font_path
                : resolveFontPath(cmd.font_family, cmd.font_weight);
            if (default_font_path.empty()) {
                DONG_LOG_WARN("GPUDriverSDL: no valid font found for family '%s'", cmd.font_family.c_str());
                break;
            }

            float font_size = cmd.font_size > 0.0f ? cmd.font_size : 16.0f;

            GlyphAtlasTier* glyph_tier = selectGlyphAtlasTier(font_size);
            if (!glyph_tier || !glyph_tier->atlas) {
                DONG_LOG_WARN("GPUDriverSDL: no glyph atlas tier available for font_size=%.1f", font_size);
                break;
            }
            GlyphAtlas* glyph_atlas = glyph_tier->atlas.get();
            if (!glyph_atlas || !text_sampler_) {
                DONG_LOG_WARN("GPUDriverSDL: glyph atlas unavailable");
                break;
            }

            DONG_LOG_DEBUG("[DrawText] using tier=%upx atlas=%p font=%s", 
                    glyph_tier->bitmap_px, (void*)glyph_atlas, default_font_path.c_str());

            const float atlas_range = glyph_tier->distance_range;
            const float gamma_correction = -2.2f;  // sRGB gamma 值
            const float pixel_scale = cmd.scale_to_pixels;

            if (pipeline_state.active != PipelineBindingState::ActivePipeline::Text) {
                SDL_BindGPUGraphicsPipeline(pass, text_pipeline_);
                pipeline_state.active = PipelineBindingState::ActivePipeline::Text;
            }

            // 重要：SDL_GPU uniform buffer 通常有 4096 字节限制
            // 布局：viewport(16) + clip(96) = 112 bytes header
            // 剩余：4096 - 112 = 3984 bytes，可容纳 62 个 glyph (每个 64 bytes)
            constexpr int kMaxGlyphsPerBatch = 62;

            struct GlyphInstanceUniform {
                float rect[4];
                float uv_rect[4];
                float color[4];
                float params[4];
            };

            // viewport 和 clip 在前，glyphs 在后，总大小 <= 4096 bytes
            struct TextBatchUniformData {
                float viewport[4];                                 // offset 0, size 16
                ClipUniformBlock clip;                             // offset 16, size 96
                GlyphInstanceUniform glyphs[kMaxGlyphsPerBatch];   // offset 112, size 3968
            };  // total: 4080 bytes

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
            for (size_t glyph_idx = 0; glyph_idx < cmd.glyphs.size(); ++glyph_idx) {
                const auto& glyph = cmd.glyphs[glyph_idx];
                if (glyph.glyph_id == 0) {
                    continue;
                }

                // 使用 glyph 自己的 font_path（支持字体回退），如果为空则使用默认字体
                std::string glyph_font_path = !glyph.font_path.empty() 
                    ? glyph.font_path 
                    : default_font_path;
                
                // 计算该 glyph 的 pixel_scale（可能使用不同字体的 units_per_em）
                float glyph_pixel_scale = pixel_scale;
                if (glyph.units_per_em > 0 && glyph.units_per_em != cmd.units_per_em) {
                    glyph_pixel_scale = font_size / static_cast<float>(glyph.units_per_em);
                }

                const AtlasEntry* entry = glyph_atlas->addGlyph(glyph.glyph_id, glyph_font_path);
                if (!entry) {
                    DONG_LOG_VERBOSE("[DrawText] SKIP glyph[%zu]: glyph_id=%u no_entry (font=%s)", 
                            glyph_idx, glyph.glyph_id, glyph_font_path.c_str());
                    continue;
                }

                if (entry->metrics.width_units <= 0.0f || entry->metrics.height_units <= 0.0f ||
                    entry->u1 <= entry->u0 || entry->v1 <= entry->v0) {
                    DONG_LOG_VERBOSE("[DrawText] SKIP glyph[%zu]: glyph_id=%u invalid_metrics (w=%.1f h=%.1f u0=%.4f u1=%.4f v0=%.4f v1=%.4f)", 
                            glyph_idx, glyph.glyph_id, 
                            entry->metrics.width_units, entry->metrics.height_units,
                            entry->u0, entry->u1, entry->v0, entry->v1);
                    continue;
                }

                const float msdf_scale = (entry->metrics.msdf_scale > 0.0f)
                    ? entry->metrics.msdf_scale
                    : 1.0f;
                
                // ========== MSDF 纹理渲染方案 ==========
                //
                // MSDF 纹理结构：
                // - 完整尺寸：msdf_size x msdf_size（如 32x32）
                // - 字形位置：从 (range, range) 开始（通过 translate 实现）
                // - 字形尺寸：bounds_width * msdf_scale x bounds_height * msdf_scale
                //
                // 渲染策略：
                // - 渲染矩形尺寸 = msdf_size * (glyph_pixel_scale / msdf_scale)
                // - 这样可以保证 MSDF 纹理中的每个像素都正确映射到屏幕
                // - 字形在渲染矩形中的位置由 range * (glyph_pixel_scale / msdf_scale) 确定
                //
                // 注意：这种方案会导致渲染矩形比实际字形大（包含 padding 和可能的空白区域）
                // 但这是正确的，因为 MSDF 需要 padding 来实现抗锯齿
                
                // 获取 MSDF 纹理的实际尺寸（从 tier 配置获取）
                const float msdf_size = static_cast<float>(glyph_tier->bitmap_px);
                
                // 渲染矩形尺寸 = msdf_size * (glyph_pixel_scale / msdf_scale)
                // 这是 MSDF 纹理在屏幕上应该占用的像素数
                const float render_scale = glyph_pixel_scale / msdf_scale;
                const float glyph_w = msdf_size * render_scale;
                const float glyph_h = msdf_size * render_scale;

                if (glyph_w <= 0.0f || glyph_h <= 0.0f) {
                    DONG_LOG_VERBOSE("[DrawText] SKIP glyph[%zu]: glyph_id=%u zero_size (w=%.1f h=%.1f render_scale=%.4f msdf_size=%.0f)",
                            glyph_idx, glyph.glyph_id, glyph_w, glyph_h, render_scale, msdf_size);
                    continue;
                }

                // pen position 是 baseline 上的当前绘制位置
                const float pen_x_px = glyph.pen_x_units * glyph_pixel_scale + cmd.baseline_x;
                const float pen_y_px = glyph.pen_y_units * glyph_pixel_scale + cmd.baseline_y;

                // ========== 字形位置计算（考虑 UV 翻转）==========
                //
                // 坐标系统：
                // - msdfgen：Y 向上，y=0 是底部
                // - GPU 纹理：Y 向下，y=0 是顶部
                // - 屏幕：Y 向下，y=0 是顶部
                //
                // MSDF 纹理结构（msdfgen 坐标）：
                // - 纹理范围：(0,0) 到 (msdf_size, msdf_size)
                // - 字形底部在 y = range（通过 translate 实现）
                // - 字形顶部在 y = range + glyph_h_msdf
                //
                // GPU 纹理：
                // - 直接复制 msdfgen 输出，所以 GPU y=0 对应 msdfgen y=0
                // - 但 GPU y=0 是顶部，msdfgen y=0 是底部
                //
                // UV 翻转：
                // - 交换 v0 和 v1，使渲染矩形顶部对应 msdfgen 纹理顶部
                // - 渲染矩形顶部 → GPU 纹理底部 → msdfgen 纹理顶部
                // - 渲染矩形底部 → GPU 纹理顶部 → msdfgen 纹理底部
                //
                // 位置计算：
                // - 渲染矩形覆盖整个 msdf_size x msdf_size 纹理
                // - 渲染矩形顶部对应 msdfgen 纹理顶部（y = msdf_size）
                // - 字形顶部在 msdfgen y = range + glyph_h_msdf
                // - 从 msdfgen 纹理顶部到字形顶部的距离 = msdf_size - (range + glyph_h_msdf)
                // - 在屏幕上，这个距离 = (msdf_size - range - glyph_h_msdf) * render_scale
                //
                // glyph_y = (字形顶部屏幕位置) - (渲染矩形顶部到字形顶部的距离)
                //         = (pen_y - bounds_top * glyph_pixel_scale) - (msdf_size - range - glyph_h_msdf) * render_scale
                //
                // 其中 glyph_h_msdf = bounds_height * msdf_scale
                //      bounds_height = bounds_top - bounds_bottom
                //
                // 展开：
                // glyph_y = pen_y - bounds_top * glyph_pixel_scale - (msdf_size - range) * render_scale + bounds_height * msdf_scale * render_scale
                //         = pen_y - bounds_top * glyph_pixel_scale - (msdf_size - range) * render_scale + bounds_height * glyph_pixel_scale
                //         = pen_y - bounds_top * glyph_pixel_scale + bounds_height * glyph_pixel_scale - (msdf_size - range) * render_scale
                //         = pen_y + (bounds_height - bounds_top) * glyph_pixel_scale - (msdf_size - range) * render_scale
                //         = pen_y - bounds_bottom * glyph_pixel_scale - (msdf_size - range) * render_scale
                //
                // 类似地，对于 glyph_x：
                // - 渲染矩形左边对应 msdfgen 纹理左边（x = 0）
                // - 字形左边在 msdfgen x = range
                // - 从 msdfgen 纹理左边到字形左边的距离 = range
                // - 在屏幕上，这个距离 = range * render_scale
                //
                // glyph_x = (字形左边屏幕位置) - (渲染矩形左边到字形左边的距离)
                //         = (pen_x + bounds_left * glyph_pixel_scale) - range * render_scale
                
                const float range_px = atlas_range * render_scale;
                const float bounds_left_px = entry->metrics.bounds_left * glyph_pixel_scale;
                const float bounds_bottom_px = entry->metrics.bounds_bottom * glyph_pixel_scale;

                // 字形左边缘位置 - range 偏移
                float glyph_x = pen_x_px + bounds_left_px - range_px;
                
                // 字形底部位置 - (msdf_size - range) 偏移
                // 注意：这里使用 bounds_bottom 而不是 bounds_top
                const float top_offset_px = (msdf_size - atlas_range) * render_scale;
                float glyph_y = pen_y_px - bounds_bottom_px - top_offset_px;

                GlyphInstanceUniform inst{};
                inst.rect[0] = glyph_x;
                inst.rect[1] = glyph_y;
                inst.rect[2] = glyph_w;
                inst.rect[3] = glyph_h;

                inst.uv_rect[0] = entry->u0;
                // Y 轴已经在 MSDF 生成时翻转，UV 不需要再翻转
                inst.uv_rect[1] = entry->v0;
                inst.uv_rect[2] = entry->u1;
                inst.uv_rect[3] = entry->v1;

                DONG_LOG_VERBOSE("[TEXT] glyph=%u page=%u pen=(%.2f,%.2f) rect=(%.2f,%.2f,%.2f,%.2f) bounds=(%.1f,%.1f,%.1f,%.1f) msdf_size=%.0f render_scale=%.4f range_px=%.2f",
                        glyph.glyph_id,
                        entry->atlas_page,
                        pen_x_px,
                        pen_y_px,
                        glyph_x,
                        glyph_y,
                        glyph_w,
                        glyph_h,
                        entry->metrics.bounds_left,
                        entry->metrics.bounds_bottom,
                        entry->metrics.bounds_right,
                        entry->metrics.bounds_top,
                        msdf_size,
                        render_scale,
                        range_px);

                writeLinearColor(cmd.color, inst.color);

                // 计算 screenPxRange：MSDF 距离场范围在屏幕空间的像素数
                // 公式：screenPxRange = atlas_range * (glyph_pixel_scale / msdf_scale)
                // 
                // 验证：
                // - atlas_range = MSDF 纹理中的 range（像素），例如 8.0
                // - msdf_scale = design units → MSDF 像素的缩放因子
                // - glyph_pixel_scale = design units → 屏幕像素的缩放因子
                // - glyph_pixel_scale / msdf_scale = 屏幕像素 / MSDF 像素 的比例
                // - 所以 atlas_range * (glyph_pixel_scale / msdf_scale) = MSDF 中的 range 在屏幕上对应多少像素
                //
                // msdfgen 官方要求：screenPxRange >= 1，最好 >= 2
                const float px_range_screen = atlas_range * (glyph_pixel_scale / msdf_scale);
                
                // unitRange = distance_range / msdf_texture_size
                // 用于着色器中 fwidth 动态计算 screenPxRange
                const float unit_range = atlas_range / msdf_size;

                inst.params[0] = px_range_screen;
                inst.params[1] = unit_range;  // 传递 unitRange 给着色器用于 fwidth 计算
                inst.params[2] = msdf_subpixel_enabled_ ? 1.0f : 0.0f;
                inst.params[3] = gamma_correction;  // 正常模式

                DONG_LOG_VERBOSE("[TEXT] glyph=%u screenPxRange=%.2f unitRange=%.4f params=(%.2f,%.4f,%.1f,%.2f) uv=(%.4f,%.4f,%.4f,%.4f)",
                        glyph.glyph_id, px_range_screen, unit_range,
                        inst.params[0], inst.params[1], inst.params[2], inst.params[3],
                        inst.uv_rect[0], inst.uv_rect[1], inst.uv_rect[2], inst.uv_rect[3]);

                PreparedGlyph pg{};
                pg.instance = inst;
                pg.atlas_page = entry->atlas_page;
                prepared.push_back(pg);
            }

            DONG_LOG_DEBUG("[DrawText] frame=%llu prepared=%zu glyphs for rendering", frame_index_, prepared.size());
            
            if (prepared.empty()) {
                DONG_LOG_WARN("[DrawText] frame=%llu ABORT: no glyphs prepared!", frame_index_);
                break;
            }

            // 第二步：按 atlas_page 分批绘制，每个批次绑定对应页纹理
            uint32_t page_count = glyph_atlas->getPageCount();
            DONG_LOG_DEBUG("[DrawText] frame=%llu rendering %zu glyphs across %u pages", frame_index_, prepared.size(), page_count);
            
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

    // 将字号映射到"期望的 MSDF 像素分辨率"，再在现有档位中选择最近的一档。
    // 使用较高的 MSDF 分辨率以保证小字号文本的清晰度：
    //   target_msdf_px ≈ font_px_size * 2.5
    // 这样对于 13px 字号会选择 32px tier，而不是更低的分辨率
    const float clamped_font_size = std::max(font_size, 1.0f);
    const float target_msdf_px_f = std::ceil(clamped_font_size * 2.5f);
    const uint32_t target_msdf_px = target_msdf_px_f > 0.0f
        ? static_cast<uint32_t>(target_msdf_px_f)
        : 32u;

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

    // DEBUG: 输出选择的 tier
    static bool first_log = true;
    if (first_log) {
        SDL_Log("[TIER SELECT] font_size=%.1f target_msdf=%u -> selected tier=%upx atlas=%p",
                font_size, target_msdf_px, best->bitmap_px, (void*)best->atlas.get());
        first_log = false;
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
            if (env_layer_cache[0] == '0') {
                driver->setDebugLogLayerCache(false);
            }
        }
        if (const char* env_subpixel = std::getenv("DONG_MSDF_SUBPIXEL")) {
            if (env_subpixel[0] == '1') {
                driver->setMsdfSubpixelEnabled(true);
            }
        }
        // 可选启用图层缓存（默认关闭，避免影响渲染正确性）。
        // 通过环境变量 DONG_LAYER_CACHE=1 显式开启。
        if (const char* env_layer_cache_enable = std::getenv("DONG_LAYER_CACHE")) {
            if (env_layer_cache_enable[0] == '0') {
                driver->setLayerCacheEnabled(false);
            }
        }

        return driver;
    }
    return nullptr;
}

} // namespace dong::render
