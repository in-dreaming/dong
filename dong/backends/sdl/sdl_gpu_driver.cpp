// =============================================================================
// SDL GPU Driver Implementation (Backend)
// =============================================================================
// Phase 2A: Framework and CMake integration
// Phase 2B: Implementation migration (TODO)
//
// This file will contain the full implementation migrated from:
//   src/render/gpu_driver_sdl.cpp
//   src/render/sdl_render/gpu_driver_sdl_*.cpp
// =============================================================================

#include "sdl_gpu_driver.hpp"

// Core includes (temporary, to be decoupled in future phases)
#include "sdl_gpu_device.hpp"
#include "sdl_shader_manager.hpp"
#include "../../src/render/resource_manager.hpp"
#include "../../src/render/glyph_atlas.hpp"
#include "../../src/render/font_resolver.hpp"
#include "../../src/render/gpu_ir.hpp"
#include "../../src/core/log.h"
#include "../../src/core/profiler.h"
#include "../../src/core/global_shared.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
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

#include "dong_image_atlas.h"
#include "../../src/render/slug/slug_font_cache.hpp"

namespace dong {
namespace render {

namespace {

void writeLinearColor(const Color& color, float out_rgba[4]) {
    out_rgba[0] = color.r;
    out_rgba[1] = color.g;
    out_rgba[2] = color.b;
    out_rgba[3] = color.a;
}

} // namespace

// =============================================================================
// Construction / Destruction
// =============================================================================

SDLGPUDriver::SDLGPUDriver(sdl_backend::GPUDevice* device, SDL_Window* window, sdl_backend::ShaderManager* shader_manager)
    : gpu_device_(device), window_(window), shader_manager_(shader_manager) {}

SDLGPUDriver::~SDLGPUDriver() {
    // 确保命令缓冲已提交，避免泄漏
    if (in_frame_ && gpu_device_ && gpu_device_->isInitialized() && current_cmd_buf_) {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
    }
    current_cmd_buf_ = nullptr;
    in_frame_ = false;

    // 检查设备是否仍然有效
    if (gpu_device_ && gpu_device_->isInitialized()) {
        SDL_GPUDevice* dev = gpu_device_->getHandle();
        if (!dev) return;

        // 释放所有 pipelines 和 shaders
        if (rect_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, rect_pipeline_);
        if (rect_vs_) SDL_ReleaseGPUShader(dev, rect_vs_);
        if (rect_fs_) SDL_ReleaseGPUShader(dev, rect_fs_);

        if (round_rect_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, round_rect_pipeline_);
        if (round_rect_vs_) SDL_ReleaseGPUShader(dev, round_rect_vs_);
        if (round_rect_fs_) SDL_ReleaseGPUShader(dev, round_rect_fs_);

        if (shadow_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, shadow_pipeline_);
        if (shadow_vs_) SDL_ReleaseGPUShader(dev, shadow_vs_);
        if (shadow_fs_) SDL_ReleaseGPUShader(dev, shadow_fs_);

        if (gradient_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, gradient_pipeline_);
        if (gradient_vs_) SDL_ReleaseGPUShader(dev, gradient_vs_);
        if (gradient_fs_) SDL_ReleaseGPUShader(dev, gradient_fs_);

        if (conic_gradient_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, conic_gradient_pipeline_);
        if (mask_apply_conic_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, mask_apply_conic_pipeline_);
        if (conic_gradient_vs_) SDL_ReleaseGPUShader(dev, conic_gradient_vs_);
        if (conic_gradient_fs_) SDL_ReleaseGPUShader(dev, conic_gradient_fs_);

        if (uber_quad_instanced_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, uber_quad_instanced_pipeline_);
        if (uber_quad_instanced_vs_) SDL_ReleaseGPUShader(dev, uber_quad_instanced_vs_);
        if (uber_quad_instanced_fs_) SDL_ReleaseGPUShader(dev, uber_quad_instanced_fs_);

        if (uber_quad_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, uber_quad_pipeline_);
        if (uber_quad_vs_) SDL_ReleaseGPUShader(dev, uber_quad_vs_);
        if (uber_quad_fs_) SDL_ReleaseGPUShader(dev, uber_quad_fs_);

        if (uber_instance_buffer_) {
            SDL_ReleaseGPUBuffer(dev, uber_instance_buffer_);
            uber_instance_buffer_ = nullptr;
            uber_instance_buffer_instance_cap_ = 0;
        }

        if (image_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, image_pipeline_);
        if (nineslice_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, nineslice_pipeline_);
        if (video_yuv_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, video_yuv_pipeline_);
        if (image_vs_) SDL_ReleaseGPUShader(dev, image_vs_);
        if (image_fs_) SDL_ReleaseGPUShader(dev, image_fs_);
        if (nineslice_fs_) SDL_ReleaseGPUShader(dev, nineslice_fs_);
        if (video_yuv_fs_) SDL_ReleaseGPUShader(dev, video_yuv_fs_);
        if (image_sampler_) SDL_ReleaseGPUSampler(dev, image_sampler_);
        if (image_sampler_nearest_) SDL_ReleaseGPUSampler(dev, image_sampler_nearest_);


        if (image_atlas_) {
            dong_atlas_destroy(image_atlas_);
            image_atlas_ = nullptr;
        }

        // Release external textures
        for (auto& kv : external_images_) {
            if (kv.second.texture) SDL_ReleaseGPUTexture(dev, kv.second.texture);
            if (kv.second.texture_u) SDL_ReleaseGPUTexture(dev, kv.second.texture_u);
            if (kv.second.texture_v) SDL_ReleaseGPUTexture(dev, kv.second.texture_v);
        }
        external_images_.clear();

        // Release upload buffers
        for (auto& p : pending_upload_buffers_) {
            for (auto& b : p.buffers) {
                if (b.buf) SDL_ReleaseGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(b.buf));
            }
            if (p.fence) SDL_ReleaseGPUFence(dev, static_cast<SDL_GPUFence*>(p.fence));
        }
        pending_upload_buffers_.clear();

        for (auto& b : frame_upload_buffers_) {
            if (b.buf) SDL_ReleaseGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(b.buf));
        }
        frame_upload_buffers_.clear();

        for (auto& b : free_upload_buffers_) {
            if (b.buf) SDL_ReleaseGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(b.buf));
        }
        free_upload_buffers_.clear();

        // Release text resources
        if (text_pipeline_) SDL_ReleaseGPUGraphicsPipeline(dev, text_pipeline_);
        if (text_vs_) SDL_ReleaseGPUShader(dev, text_vs_);
        if (text_fs_) SDL_ReleaseGPUShader(dev, text_fs_);
        if (text_sampler_) SDL_ReleaseGPUSampler(dev, text_sampler_);

        // Release intermediate texture
        if (intermediate_texture_) SDL_ReleaseGPUTexture(dev, intermediate_texture_);

        // Release layer render targets
        for (auto& entry : layer_render_targets_) {
            if (entry.texture) SDL_ReleaseGPUTexture(dev, entry.texture);
        }
        layer_render_targets_.clear();
    }

    // 清理 FreeType 资源
    for (auto& entry : ft_face_cache_) {
        if (entry.second) FT_Done_Face(entry.second);
    }
    ft_face_cache_.clear();
    if (ft_library_) {
        FT_Done_FreeType(ft_library_);
        ft_library_ = nullptr;
    }

    // 释放 GlobalShared 引用（如果使用了共享 GlyphAtlas）
    if (use_global_shared_glyph_atlas_) {
        auto* global_shared = GlobalShared::instance();
        if (global_shared) {
            global_shared->release();
            DONG_LOG_DEBUG("SDLGPUDriver: Released GlobalShared reference");
        }
    }

    gpu_device_ = nullptr;
    window_ = nullptr;
    shader_manager_ = nullptr;
}

// =============================================================================
// Initialization
// =============================================================================

// initialize() implementation is in sdl_gpu_driver_init.cpp

// =============================================================================
// Frame Management
// =============================================================================

void SDLGPUDriver::beginFrame() {
    DONG_LOG_DEBUG("[SDLGPUDriver::beginFrame] START frame=%llu in_frame=%d", frame_index_ + 1, in_frame_ ? 1 : 0);
    if (in_frame_) {
        DONG_LOG_WARN("SDLGPUDriver::beginFrame: already in frame");
        return;
    }
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_ERROR("SDLGPUDriver::beginFrame: device not initialized");
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    if (dev) {
        reapUploadBuffers(dev);
    }

    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
    if (!current_cmd_buf_) {
        DONG_LOG_ERROR("SDLGPUDriver::beginFrame: failed to acquire command buffer");
        return;
    }

    ++frame_index_;
    in_frame_ = true;

    for (auto& entry : layer_render_targets_) {
        entry.in_use = false;
    }
}

void SDLGPUDriver::endFrame() {
    DONG_LOG_DEBUG("[SDLGPUDriver::endFrame] START frame=%llu in_frame=%d", frame_index_, in_frame_ ? 1 : 0);
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    if (dev && !frame_upload_buffers_.empty()) {
        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(current_cmd_buf_);
        if (fence) {
            PendingUploadBuffers pending;
            pending.fence = fence;
            pending.buffers = std::move(frame_upload_buffers_);
            pending_upload_buffers_.push_back(std::move(pending));
            frame_upload_buffers_.clear();
        } else {
            DONG_LOG_ERROR("SDLGPUDriver::endFrame: fence acquisition failed: %s", SDL_GetError());
            gpu_device_->submitCommandBuffer(current_cmd_buf_);
            gpu_device_->waitForGPU();
            for (auto& b : frame_upload_buffers_) {
                if (b.buf) free_upload_buffers_.push_back(b);
            }
            frame_upload_buffers_.clear();
        }
    } else {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
    }

    current_cmd_buf_ = nullptr;
    in_frame_ = false;
}

// =============================================================================
// Command Execution
// =============================================================================

// execute() implementation is in sdl_gpu_driver_execute.cpp
// prepareResources() implementation is in sdl_gpu_driver_resources.cpp

// =============================================================================
// Offscreen Rendering
// =============================================================================

void SDLGPUDriver::beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height) {
    if (in_frame_) {
        DONG_LOG_WARN("SDLGPUDriver::beginFrameOffscreen: already in frame");
        return;
    }
    if (!gpu_device_ || !gpu_device_->isInitialized() || !target) {
        DONG_LOG_ERROR("SDLGPUDriver::beginFrameOffscreen: invalid parameters");
        return;
    }

    current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
    if (!current_cmd_buf_) {
        DONG_LOG_ERROR("SDLGPUDriver::beginFrameOffscreen: failed to acquire command buffer");
        return;
    }

    ++frame_index_;

    for (auto& entry : layer_render_targets_) {
        entry.in_use = false;
    }

    offscreen_width_ = width;
    offscreen_height_ = height;
    offscreen_target_ = target;
    in_frame_ = true;
}

void SDLGPUDriver::endFrameOffscreen() {
    if (!in_frame_ || !current_cmd_buf_ || !gpu_device_) {
        return;
    }

    gpu_device_->submitCommandBuffer(current_cmd_buf_);

    current_cmd_buf_ = nullptr;
    offscreen_target_ = nullptr;
    offscreen_width_ = 0;
    offscreen_height_ = 0;
    in_frame_ = false;
}

// =============================================================================
// External Images (Video)
// =============================================================================

bool SDLGPUDriver::updateExternalImageRGBA(const std::string& key,
                                           const uint8_t* rgba,
                                           uint32_t width,
                                           uint32_t height,
                                           uint32_t stride_bytes) {
    if (key.empty() || !rgba || width == 0 || height == 0) return false;
    if (!gpu_device_ || !gpu_device_->isInitialized()) return false;

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    if (!dev) return false;

    // If not in a frame, acquire a temporary command buffer for the upload
    bool temp_frame = false;
    if (!current_cmd_buf_) {
        current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
        if (!current_cmd_buf_) {
            DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageRGBA: failed to acquire temp command buffer");
            return false;
        }
        temp_frame = true;
    }

    ExternalImage& ex = external_images_[key];
    if (ex.format != ExternalImageFormat::RGBA8) {
        if (ex.texture) SDL_ReleaseGPUTexture(dev, ex.texture);
        if (ex.texture_u) SDL_ReleaseGPUTexture(dev, ex.texture_u);
        if (ex.texture_v) SDL_ReleaseGPUTexture(dev, ex.texture_v);
        ex = ExternalImage{};
        ex.format = ExternalImageFormat::RGBA8;
    }

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
            DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageRGBA: failed to create texture");
            external_images_.erase(key);
            return false;
        }
        ex.width = width;
        ex.height = height;
    }

    const uint32_t dst_stride = width * 4;
    const uint32_t upload_size = dst_stride * height;

    UploadBuffer upload = acquireUploadBuffer(dev, upload_size);
    if (!upload.buf) {
        DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageRGBA: failed to acquire transfer buffer");
        return false;
    }

    void* mapped = SDL_MapGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(upload.buf), false);
    if (!mapped) {
        DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageRGBA: failed to map transfer buffer");
        SDL_ReleaseGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(upload.buf));
        return false;
    }

    uint8_t* dst = static_cast<uint8_t*>(mapped);
    uint32_t src_stride = stride_bytes == 0 ? dst_stride : stride_bytes;

    if (src_stride == dst_stride) {
        std::memcpy(dst, rgba, upload_size);
    } else {
        for (uint32_t y = 0; y < height; ++y) {
            const uint8_t* src_row = rgba + static_cast<size_t>(y) * src_stride;
            uint8_t* dst_row = dst + static_cast<size_t>(y) * dst_stride;
            std::memcpy(dst_row, src_row, dst_stride);
        }
    }

    SDL_UnmapGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(upload.buf));

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(current_cmd_buf_);
    if (!copy_pass) {
        free_upload_buffers_.push_back(upload);
        return false;
    }

    SDL_GPUTextureTransferInfo tex_transfer{};
    tex_transfer.transfer_buffer = static_cast<SDL_GPUTransferBuffer*>(upload.buf);
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

    frame_upload_buffers_.push_back(upload);

    // If we acquired a temporary command buffer, submit it now
    if (temp_frame) {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
        current_cmd_buf_ = nullptr;
    }

    return true;
}

bool SDLGPUDriver::updateExternalImageYUV420P(const std::string& key,
                                              const uint8_t* plane_y,
                                              uint32_t stride_y,
                                              const uint8_t* plane_u,
                                              uint32_t stride_u,
                                              const uint8_t* plane_v,
                                              uint32_t stride_v,
                                              uint32_t width,
                                              uint32_t height) {
    if (key.empty() || !plane_y || !plane_u || !plane_v || width == 0 || height == 0) return false;
    if (!gpu_device_ || !gpu_device_->isInitialized()) return false;

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    if (!dev) return false;

    // If not in a frame, acquire a temporary command buffer for the upload
    bool temp_frame = false;
    if (!current_cmd_buf_) {
        current_cmd_buf_ = gpu_device_->acquireCommandBuffer();
        if (!current_cmd_buf_) {
            DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageYUV420P: failed to acquire temp command buffer");
            return false;
        }
        temp_frame = true;
    }

    if (stride_y == 0) stride_y = width;
    const uint32_t cw = (width + 1) / 2;
    const uint32_t ch = (height + 1) / 2;
    if (stride_u == 0) stride_u = cw;
    if (stride_v == 0) stride_v = cw;

    ExternalImage& ex = external_images_[key];
    if (ex.format != ExternalImageFormat::YUV420P) {
        if (ex.texture) SDL_ReleaseGPUTexture(dev, ex.texture);
        if (ex.texture_u) SDL_ReleaseGPUTexture(dev, ex.texture_u);
        if (ex.texture_v) SDL_ReleaseGPUTexture(dev, ex.texture_v);
        ex = ExternalImage{};
        ex.format = ExternalImageFormat::YUV420P;
    }

    auto ensure_plane_tex = [&](SDL_GPUTexture*& tex, uint32_t w, uint32_t h) -> bool {
        if (tex) return true;
        SDL_GPUTextureCreateInfo tex_info{};
        tex_info.type = SDL_GPU_TEXTURETYPE_2D;
        tex_info.format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
        tex_info.width = w;
        tex_info.height = h;
        tex_info.layer_count_or_depth = 1;
        tex_info.num_levels = 1;
        tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
        tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

        tex = SDL_CreateGPUTexture(dev, &tex_info);
        if (!tex) {
            DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageYUV420P: failed to create plane texture");
            return false;
        }
        return true;
    };

    const bool need_recreate = (!ex.texture || !ex.texture_u || !ex.texture_v || ex.width != width || ex.height != height);
    if (need_recreate) {
        if (ex.texture) SDL_ReleaseGPUTexture(dev, ex.texture);
        if (ex.texture_u) SDL_ReleaseGPUTexture(dev, ex.texture_u);
        if (ex.texture_v) SDL_ReleaseGPUTexture(dev, ex.texture_v);
        ex.texture = nullptr;
        ex.texture_u = nullptr;
        ex.texture_v = nullptr;

        if (!ensure_plane_tex(ex.texture, width, height) ||
            !ensure_plane_tex(ex.texture_u, cw, ch) ||
            !ensure_plane_tex(ex.texture_v, cw, ch)) {
            external_images_.erase(key);
            return false;
        }
        ex.width = width;
        ex.height = height;
    }

    const uint32_t y_size = width * height;
    const uint32_t u_size = cw * ch;
    const uint32_t v_size = cw * ch;
    const uint32_t upload_size = y_size + u_size + v_size;

    UploadBuffer upload = acquireUploadBuffer(dev, upload_size);
    if (!upload.buf) {
        DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageYUV420P: failed to acquire transfer buffer");
        return false;
    }

    void* mapped = SDL_MapGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(upload.buf), false);
    if (!mapped) {
        DONG_LOG_ERROR("SDLGPUDriver::updateExternalImageYUV420P: failed to map transfer buffer");
        SDL_ReleaseGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(upload.buf));
        return false;
    }

    uint8_t* dst = static_cast<uint8_t*>(mapped);

    auto copy_plane = [&](uint8_t* dst_plane, const uint8_t* src_plane, uint32_t src_stride_val, uint32_t w, uint32_t h) {
        for (uint32_t y = 0; y < h; ++y) {
            const uint8_t* src_row = src_plane + static_cast<size_t>(y) * src_stride_val;
            uint8_t* dst_row = dst_plane + static_cast<size_t>(y) * w;
            std::memcpy(dst_row, src_row, w);
        }
    };

    uint8_t* dst_y = dst;
    uint8_t* dst_u = dst_y + y_size;
    uint8_t* dst_v = dst_u + u_size;

    copy_plane(dst_y, plane_y, stride_y, width, height);
    copy_plane(dst_u, plane_u, stride_u, cw, ch);
    copy_plane(dst_v, plane_v, stride_v, cw, ch);

    SDL_UnmapGPUTransferBuffer(dev, static_cast<SDL_GPUTransferBuffer*>(upload.buf));

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(current_cmd_buf_);
    if (!copy_pass) {
        free_upload_buffers_.push_back(upload);
        return false;
    }

    auto upload_plane = [&](SDL_GPUTexture* tex, uint32_t w, uint32_t h, uint32_t offset) {
        SDL_GPUTextureTransferInfo tex_transfer{};
        tex_transfer.transfer_buffer = static_cast<SDL_GPUTransferBuffer*>(upload.buf);
        tex_transfer.offset = offset;
        tex_transfer.pixels_per_row = w;
        tex_transfer.rows_per_layer = h;

        SDL_GPUTextureRegion region{};
        region.texture = tex;
        region.mip_level = 0;
        region.layer = 0;
        region.x = 0;
        region.y = 0;
        region.z = 0;
        region.w = w;
        region.h = h;
        region.d = 1;

        SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
    };

    upload_plane(ex.texture, width, height, 0);
    upload_plane(ex.texture_u, cw, ch, y_size);
    upload_plane(ex.texture_v, cw, ch, y_size + u_size);

    SDL_EndGPUCopyPass(copy_pass);

    frame_upload_buffers_.push_back(upload);

    // If we acquired a temporary command buffer, submit it now
    if (temp_frame) {
        gpu_device_->submitCommandBuffer(current_cmd_buf_);
        current_cmd_buf_ = nullptr;
    }

    return true;
}

// =============================================================================
// Resource Management
// =============================================================================
// reapUploadBuffers() / acquireUploadBuffer() / ensureImageInAtlas()
// implementations are in sdl_gpu_driver_resources.cpp

// =============================================================================
// Font/Glyph Management
// =============================================================================
// selectGlyphAtlasTier() / getOrCreateFace()
// implementations are in sdl_gpu_driver_fonts.cpp

// =============================================================================
// Execute Sub-functions
// =============================================================================
// ExecuteContext and all executeXXX() methods
// implementations are in sdl_gpu_driver_execute.cpp

// =============================================================================
// Factory
// =============================================================================

std::unique_ptr<SDLGPUDriver> CreateSDLGPUDriver(
    sdl_backend::GPUDevice* device,
    SDL_Window* window,
    sdl_backend::ShaderManager* shader_manager) {
    return std::make_unique<SDLGPUDriver>(device, window, shader_manager);
}

} // namespace render
} // namespace dong
