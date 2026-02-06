// =============================================================================
// SDL GPU Driver - Initialization (Phase 2B)
// =============================================================================
// Migrated from: src/render/sdl_render/gpu_driver_sdl_init.cpp
// =============================================================================

#include "sdl_gpu_driver.hpp"

#include "sdl_gpu_device.hpp"
#include "sdl_shader_manager.hpp"
#include "../../src/render/glyph_atlas.hpp"
#include "../../src/render/font_resolver.hpp"
#include "../../src/core/log.h"

#include <SDL3/SDL_video.h>

#include "dong_image_atlas.h"
#include "dong_sdl_image_atlas.h"

#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <utility>
#include <string>

namespace dong {
namespace render {

bool SDLGPUDriver::initialize() {
    if (!gpu_device_ || !gpu_device_->isInitialized() || !window_ || !shader_manager_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: invalid device, window, or shader manager");
        return false;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // 获取 swapchain 的实际格式，用于创建兼容的 pipeline
    SDL_GPUTextureFormat swapchain_format = SDL_GetGPUSwapchainTextureFormat(dev, window_);
    DONG_LOG_INFO("SDLGPUDriver::initialize: swapchain format = %d", swapchain_format);

    // 对于离屏渲染，我们使用 R8G8B8A8_UNORM，因为它是最通用的格式
    SDL_GPUTextureFormat pipeline_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    (void)pipeline_format;

#ifndef DONG_SDL_SHADER_DIR
#define DONG_SDL_SHADER_DIR "src/render/sdl_render/shaders"
#endif

    auto shader_path = [](const char* filename) -> std::string {
        std::string base = DONG_SDL_SHADER_DIR;
        if (!base.empty() && (base.back() == '/' || base.back() == '\\')) {
            return base + filename;
        }
        return base + "/" + filename;
    };

    rect_vs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_rect_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        shader_path("rect_vs.hlsl").c_str(),
        "main"
    );
    rect_fs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_rect_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        shader_path("rect_fs.hlsl").c_str(),
        "main"
    );

    if (!rect_vs_ || !rect_fs_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to compile rect shaders");
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
    // IMPORTANT: alpha channel must blend with SRC_ALPHA, otherwise offscreen targets
    // will quickly become fully opaque (alpha forced towards 1) and break transparent HTML.
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
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create rect pipeline: %s", SDL_GetError());
        return false;
    }

    // 圆角矩形绘制：analytic SDF，在 fragment 阶段做抗锯齿边缘
    round_rect_vs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_round_rect_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        shader_path("round_rect_vs.hlsl").c_str(),
        "main"
    );
    round_rect_fs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_round_rect_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        shader_path("round_rect_fs.hlsl").c_str(),
        "main"
    );

    if (!round_rect_vs_ || !round_rect_fs_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to compile round-rect shaders");
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
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create round-rect pipeline: %s", SDL_GetError());
        return false;
    }

    // 阴影绘制着色器：使用 SDF + 模糊半径实现柔和阴影边缘
    shadow_vs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_shadow_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        shader_path("shadow_vs.hlsl").c_str(),
        "main"
    );
    shadow_fs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_shadow_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        shader_path("shadow_fs.hlsl").c_str(),
        "main"
    );

    if (!shadow_vs_ || !shadow_fs_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to compile shadow shaders");
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
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create shadow pipeline: %s", SDL_GetError());
        return false;
    }

    // 图片绘制着色器：使用 SV_VertexID 生成矩形，并根据 atlas UV 采样
    image_vs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_image_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        shader_path("image_vs.hlsl").c_str(),
        "main"
    );
    image_fs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_image_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        shader_path("image_fs.hlsl").c_str(),
        "main"
    );

    if (!image_vs_ || !image_fs_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to compile image shaders");
        return false;
    }

    // Video YUV420P fragment shader (same quad vertex shader)
    video_yuv_fs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_video_yuv_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        shader_path("video_yuv_fs.hlsl").c_str(),
        "main"
    );
    if (!video_yuv_fs_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to compile video_yuv_fs shader");
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
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create image pipeline: %s", SDL_GetError());
        return false;
    }

    // YUV pipeline: identical to image pipeline except fragment shader.
    SDL_GPUGraphicsPipelineCreateInfo yuv_pci = ipci;
    yuv_pci.fragment_shader = video_yuv_fs_;
    video_yuv_pipeline_ = SDL_CreateGPUGraphicsPipeline(dev, &yuv_pci);
    if (!video_yuv_pipeline_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create video_yuv pipeline: %s", SDL_GetError());
        return false;
    }

    // 创建 Image Atlas（使用新的 DongImageAtlas 接口）
    // 根据 GPU 能力选择最佳格式：
    // 1. 桌面平台优先使用 BC7（高质量，Windows/Linux/macOS 广泛支持）
    // 2. 移动平台优先使用 ASTC 4x4（iOS/Android 广泛支持）
    // 3. 降级到 RGBA8（无压缩，全平台支持）

    DongImageFormat selected_atlas_format = DONG_IMAGE_FORMAT_RGBA8;

    // 检测 GPU 支持的压缩格式
    auto test_format_support = [dev](SDL_GPUTextureFormat fmt) -> bool {
        SDL_GPUTextureCreateInfo test_info{};
        test_info.type = SDL_GPU_TEXTURETYPE_2D;
        test_info.format = fmt;
        test_info.width = 4;
        test_info.height = 4;
        test_info.layer_count_or_depth = 1;
        test_info.num_levels = 1;
        test_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

        SDL_GPUTexture* test_tex = SDL_CreateGPUTexture(dev, &test_info);
        if (test_tex) {
            SDL_ReleaseGPUTexture(dev, test_tex);
            return true;
        }
        return false;
    };

    // 检查环境变量是否强制使用压缩格式
    const char* atlas_format_env = std::getenv("DONG_ATLAS_FORMAT");
    if (atlas_format_env) {
        if (strcmp(atlas_format_env, "BC7") == 0) {
            if (test_format_support(SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM)) {
                selected_atlas_format = DONG_IMAGE_FORMAT_BC7;
                DONG_LOG_INFO("SDLGPUDriver: Using BC7 atlas format (via env)");
            } else {
                DONG_LOG_WARN("SDLGPUDriver: BC7 not supported, falling back to RGBA8");
            }
        } else if (strcmp(atlas_format_env, "ASTC") == 0) {
            if (test_format_support(SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM)) {
                selected_atlas_format = DONG_IMAGE_FORMAT_ASTC_4x4;
                DONG_LOG_INFO("SDLGPUDriver: Using ASTC 4x4 atlas format (via env)");
            } else {
                DONG_LOG_WARN("SDLGPUDriver: ASTC not supported, falling back to RGBA8");
            }
        }
    }

    DongAtlasConfig atlas_cfg = dong_atlas_config_default();
    atlas_cfg.width = 2048;
    atlas_cfg.height = 2048;
    atlas_cfg.format = selected_atlas_format;
    atlas_cfg.padding = 2;

    image_atlas_ = dong_sdl_image_atlas_create(dev, &atlas_cfg);
    if (!image_atlas_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create image atlas");
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
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create image sampler: %s", SDL_GetError());
        dong_atlas_destroy(image_atlas_);
        image_atlas_ = nullptr;
        return false;
    }

    // MSDF 文字渲染着色器和管线
    text_vs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_text_vs",
        SDL_GPU_SHADERSTAGE_VERTEX,
        shader_path("text_vs.hlsl").c_str(),
        "main"
    );
    text_fs_ = shader_manager_->loadShaderFromHLSLFile(
        "dong_text_fs",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        shader_path("text_fs.hlsl").c_str(),
        "main"
    );

    if (!text_vs_ || !text_fs_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to compile text shaders");
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
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create text pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_GPUSamplerCreateInfo text_sampler_info{};
    text_sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    text_sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    text_sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    // 关键：MSDF 必须使用 NEAREST 采样！
    // Linear 过滤会插值 MSDF 的距离值，导致边缘模糊
    text_sampler_info.mag_filter = SDL_GPU_FILTER_NEAREST;
    text_sampler_info.min_filter = SDL_GPU_FILTER_NEAREST;
    text_sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;

    text_sampler_ = SDL_CreateGPUSampler(dev, &text_sampler_info);
    if (!text_sampler_) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to create text sampler: %s", SDL_GetError());
        return false;
    }

    // 初始化多级字形 Atlas（根据字号挑选）。
    // bitmap_px 表示单个 glyph 的 MSDF 纹理分辨率，
    // 采用 (128, 192, 256, 384) 四档，优化小字体清晰度和大字体边缘质量。
    glyph_atlas_tiers_.clear();
    struct GlyphTierConfig {
        uint32_t bitmap_px;
        float distance_range;
    };

    // 字体渲染优化配置 - 确保小字体清晰
    const GlyphTierConfig tier_configs[] = {
        {128u,  7.0f},  // 9px-14px
        {192u,  9.0f},  // 14-22px
        {256u,  11.0f}, // 22-36px
        {384u,  12.0f}, // 36px+
    };

    for (const auto& cfg : tier_configs) {
        auto atlas = std::make_unique<GlyphAtlas>(dong_gpu_driver_);
        if (!atlas->initialize(2048, 2048, cfg.bitmap_px, cfg.distance_range)) {
            DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to initialize glyph atlas tier %u", cfg.bitmap_px);
            return false;
        }
        GlyphAtlasTier tier{};
        tier.bitmap_px = cfg.bitmap_px;
        tier.distance_range = cfg.distance_range;
        tier.atlas = std::move(atlas);
        glyph_atlas_tiers_.push_back(std::move(tier));
    }

    if (FT_Init_FreeType(&ft_library_) != 0) {
        DONG_LOG_ERROR("SDLGPUDriver::initialize: failed to initialize FreeType library for caching");
        ft_library_ = nullptr;
        return false;
    }
    ft_face_cache_.clear();

    // RenderTarget/图层合成调试日志默认关闭，可通过环境变量 DONG_DEBUG_RT=1 开启
    debug_rt_enabled_ = false;

    DONG_LOG_INFO("SDLGPUDriver initialized successfully");
    return true;
}

} // namespace render
} // namespace dong
