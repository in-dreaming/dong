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

        // Release external textures (e.g. video frames)
        for (auto& kv : external_images_) {
            if (kv.second.texture) {
                SDL_ReleaseGPUTexture(dev, kv.second.texture);
                kv.second.texture = nullptr;
            }
        }
        external_images_.clear();

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

bool GPUDriverSDL::updateExternalImageRGBA(const std::string& key,
                                          const uint8_t* rgba,
                                          uint32_t width,
                                          uint32_t height,
                                          uint32_t stride_bytes) {
    if (key.empty() || !rgba || width == 0 || height == 0) {
        return false;
    }
    if (!gpu_device_ || !gpu_device_->isInitialized() || !current_cmd_buf_) {
        return false;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    if (!dev) {
        return false;
    }

    ExternalImage& ex = external_images_[key];
    if (!ex.texture || ex.width != width || ex.height != height) {
        if (ex.texture) {
            SDL_ReleaseGPUTexture(dev, ex.texture);
            ex.texture = nullptr;
        }

        SDL_GPUTextureCreateInfo tex_info{};
        tex_info.type = SDL_GPU_TEXTURETYPE_2D;
        tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        tex_info.width = width;
        tex_info.height = height;
        tex_info.layer_count_or_depth = 1;
        tex_info.num_levels = 1;
        tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
        tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

        ex.texture = SDL_CreateGPUTexture(dev, &tex_info);
        if (!ex.texture) {
            SDL_Log("GPUDriverSDL::updateExternalImageRGBA: failed to create texture: %s", SDL_GetError());
            external_images_.erase(key);
            return false;
        }
        ex.width = width;
        ex.height = height;
    }

    const uint32_t dst_stride = width * 4;
    const uint32_t upload_size = dst_stride * height;

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = upload_size;

    SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
    if (!transfer_buf) {
        SDL_Log("GPUDriverSDL::updateExternalImageRGBA: failed to create transfer buffer: %s", SDL_GetError());
        return false;
    }

    void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
    if (!mapped) {
        SDL_Log("GPUDriverSDL::updateExternalImageRGBA: failed to map transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        return false;
    }

    uint8_t* dst = static_cast<uint8_t*>(mapped);
    if (stride_bytes == 0) {
        stride_bytes = dst_stride;
    }

    if (stride_bytes == dst_stride) {
        std::memcpy(dst, rgba, upload_size);
    } else {
        for (uint32_t y = 0; y < height; ++y) {
            const uint8_t* src_row = rgba + static_cast<size_t>(y) * stride_bytes;
            uint8_t* dst_row = dst + static_cast<size_t>(y) * dst_stride;
            std::memcpy(dst_row, src_row, dst_stride);
        }
    }

    SDL_UnmapGPUTransferBuffer(dev, transfer_buf);

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(current_cmd_buf_);
    if (!copy_pass) {
        SDL_Log("GPUDriverSDL::updateExternalImageRGBA: failed to begin copy pass: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        return false;
    }

    SDL_GPUTextureTransferInfo tex_transfer{};
    tex_transfer.transfer_buffer = transfer_buf;
    tex_transfer.offset = 0;
    tex_transfer.pixels_per_row = width;
    tex_transfer.rows_per_layer = height;

    SDL_GPUTextureRegion region{};
    region.texture = ex.texture;
    region.mip_level = 0;
    region.layer = 0;
    region.x = 0;
    region.y = 0;
    region.z = 0;
    region.w = width;
    region.h = height;
    region.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
    return true;
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

    // A1: 不在每次离屏渲染后强制等待 GPU idle。
    // 依赖同一 SDL_GPUDevice 上 command buffer 的提交顺序来保证后续采样/拷贝的正确性。
    // 若需要 CPU 侧立刻读回，请在读回路径使用 fence 精确等待（见 View::renderOffscreen）。

    current_cmd_buf_ = nullptr;
    offscreen_target_ = nullptr;
    offscreen_width_ = 0;
    offscreen_height_ = 0;
    in_frame_ = false;
}

} // namespace dong::render
