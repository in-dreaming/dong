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

} // namespace dong::render
