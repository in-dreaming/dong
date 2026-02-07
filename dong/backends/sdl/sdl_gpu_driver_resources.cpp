// =============================================================================
// SDL GPU Driver - Resource Management (Phase 2B)
// =============================================================================
// Migrated from: src/render/sdl_render/gpu_driver_sdl_resources.cpp
// =============================================================================

#include "sdl_gpu_driver.hpp"

#include "sdl_gpu_device.hpp"
#include "gpu_texture_compressor.hpp"
#include "../../src/render/gpu_ir.hpp"
#include "../../src/render/resource_manager.hpp"
#include "../../src/render/glyph_atlas.hpp"
#include "../../src/render/font_resolver.hpp"
#include "../../src/core/log.h"
#include "../../src/core/profiler.h"

#include "dong_image_atlas.h"
#include "dong_image_decoder.h"
#include "dong_platform.h"

#include <vector>
#include <algorithm>
#include <cstring>
#include <utility>
#include <unordered_map>
#include <unordered_set>

namespace dong {
namespace render {

void SDLGPUDriver::prepareResources(const GPUCommandList& commands) {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_WARN("[prepareResources] SKIP: gpu_device not ready");
        return;
    }

    DONG_LOG_VERBOSE("[prepareResources] START frame=%llu commands=%zu", frame_index_ + 1, commands.commands.size());

    // ========== 批量收集所有 glyph，按 atlas tier 分组后一次性上传 ==========

    // 按 tier (bitmap_px) -> glyph requests 分组
    std::unordered_map<uint32_t, std::vector<GlyphAtlas::GlyphRequest>> tier_requests;
    // 用于去重
    std::unordered_set<std::string> seen_keys;

    for (const auto& cmd : commands.commands) {
        if (cmd.type != GPUCommandType::DrawText) {
            continue;
        }
        if (cmd.glyphs.empty()) {
            continue;
        }

        std::string resolved_primary_font_path;
        const std::string* font_path = nullptr;

        if (!cmd.font_paths.empty()) {
            font_path = &cmd.font_paths[0];
        } else if (!cmd.font_path.empty()) {
            font_path = &cmd.font_path;
        } else {
            resolved_primary_font_path = resolveFontPath(cmd.font_family, cmd.font_weight, cmd.font_style);
            font_path = &resolved_primary_font_path;
        }

        if (!font_path || font_path->empty()) {
            continue;
        }

        // 选择合适的 glyph atlas tier
        float font_size = cmd.font_size > 0.0f ? cmd.font_size : 16.0f;
        GlyphAtlas* glyph_atlas = getGlyphAtlasForFontSize(font_size);
        if (!glyph_atlas) {
            continue;
        }

        uint32_t tier_key = glyph_atlas->getGlyphBitmapSize();

        // 收集 glyph 请求
        for (const auto& glyph : cmd.glyphs) {
            if (glyph.glyph_id == 0) {
                continue;
            }
            const std::string* glyph_font_path_ptr = font_path;
            if (!cmd.font_paths.empty()) {
                const uint16_t idx = glyph.font_path_index;
                if (idx < cmd.font_paths.size()) {
                    glyph_font_path_ptr = &cmd.font_paths[idx];
                }
            }
            const std::string& glyph_font_path = glyph_font_path_ptr ? *glyph_font_path_ptr : *font_path;

            // 去重：同一个 glyph+font 组合只请求一次
            std::string dedup_key = glyph_font_path + "#" + std::to_string(glyph.glyph_id) + "#" + std::to_string(tier_key);
            if (seen_keys.find(dedup_key) != seen_keys.end()) {
                continue;
            }
            seen_keys.insert(dedup_key);

            GlyphAtlas::GlyphRequest req;
            req.glyph_id = glyph.glyph_id;
            req.font_path = glyph_font_path;

            tier_requests[tier_key].push_back(std::move(req));
        }
    }

    // 批量添加每个 tier 的 glyphs
    for (auto& [tier_key, requests] : tier_requests) {
        if (requests.empty()) continue;

        // 获取对应的 atlas（支持 GlobalShared 模式）
        GlyphAtlas* atlas = getGlyphAtlasForBitmapPx(tier_key);
        if (!atlas) continue;

        DONG_LOG_DEBUG("[prepareResources] tier %upx: batching %zu glyph requests", tier_key, requests.size());

        // 使用批量接口添加
        atlas->addGlyphsBatched(requests);
    }

    DONG_LOG_DEBUG("[prepareResources] END frame=%llu", frame_index_ + 1);
}

bool SDLGPUDriver::ensureImageInAtlas(const std::string& src, ImageAtlasEntry& out_entry) {
    if (!gpu_device_ || !gpu_device_->isInitialized() || !image_atlas_ || !current_cmd_buf_) {
        return false;
    }
    if (src.empty()) {
        return false;
    }

    // Check cache first
    auto it = image_atlas_entries_.find(src);
    if (it != image_atlas_entries_.end()) {
        out_entry = it->second;
        return true;
    }

    if (!image_resource_manager_) {
        DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: no image resource manager bound");
        return false;
    }

    // Load image pixels via ResourceManager (uses IImageDecoder)
    std::vector<uint8_t> pixels;
    uint32_t img_w = 0;
    uint32_t img_h = 0;
    if (!image_resource_manager_->getImagePixelsRGBA(src, pixels, img_w, img_h)) {
        DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: failed to get pixels for '%s'", src.c_str());
        return false;
    }

    if (img_w == 0 || img_h == 0 || pixels.empty()) {
        return false;
    }

    // Get atlas dimensions from config
    uint32_t atlas_w = image_atlas_->config.width;
    uint32_t atlas_h = image_atlas_->config.height;

    // Downscale if image is too large for atlas
    if (img_w > atlas_w || img_h > atlas_h) {
        const uint32_t src_w = img_w;
        const uint32_t src_h = img_h;

        uint32_t new_w = img_w;
        uint32_t new_h = img_h;

        if (new_w > atlas_w) {
            new_h = static_cast<uint32_t>((static_cast<uint64_t>(new_h) * atlas_w) / new_w);
            new_w = atlas_w;
        }
        if (new_h > atlas_h) {
            new_w = static_cast<uint32_t>((static_cast<uint64_t>(new_w) * atlas_h) / new_h);
            new_h = atlas_h;
        }

        new_w = std::max(1u, new_w);
        new_h = std::max(1u, new_h);

        DONG_LOG_INFO("SDLGPUDriver::ensureImageInAtlas: downscaling '%s' from %ux%u to %ux%u to fit atlas",
                src.c_str(), src_w, src_h, new_w, new_h);

        std::vector<uint8_t> scaled;
        scaled.resize(static_cast<size_t>(new_w) * static_cast<size_t>(new_h) * 4);

        for (uint32_t y = 0; y < new_h; ++y) {
            const uint32_t sy = static_cast<uint32_t>((static_cast<uint64_t>(y) * src_h) / new_h);
            for (uint32_t x = 0; x < new_w; ++x) {
                const uint32_t sx = static_cast<uint32_t>((static_cast<uint64_t>(x) * src_w) / new_w);
                const size_t src_idx = (static_cast<size_t>(sy) * src_w + sx) * 4;
                const size_t dst_idx = (static_cast<size_t>(y) * new_w + x) * 4;
                scaled[dst_idx + 0] = pixels[src_idx + 0];
                scaled[dst_idx + 1] = pixels[src_idx + 1];
                scaled[dst_idx + 2] = pixels[src_idx + 2];
                scaled[dst_idx + 3] = pixels[src_idx + 3];
            }
        }

        pixels = std::move(scaled);
        img_w = new_w;
        img_h = new_h;
    }

    // Allocate region in atlas
    DongAtlasEntry atlas_entry = {0};
    DongAtlasResult result = dong_atlas_alloc(image_atlas_, img_w, img_h, &atlas_entry);
    if (result != DONG_ATLAS_OK) {
        DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: atlas allocation failed for '%s' (%ux%u), error=%d",
                src.c_str(), img_w, img_h, (int)result);
        return false;
    }

    // Get atlas texture for direct GPU compression
    SDL_GPUTexture* atlas_texture = static_cast<SDL_GPUTexture*>(
        dong_atlas_get_texture(image_atlas_, atlas_entry.atlas_page));

    DongImageFormat atlas_format = image_atlas_->config.format;

    // If atlas uses compressed format, try GPU compression directly to atlas texture
    if (dong_image_format_is_compressed(atlas_format)) {
        bool gpu_compressed = false;

        // Determine GPU compress format
        sdl_backend::GPUCompressFormat gpu_format = sdl_backend::GPUCompressFormat::BC7;
        bool format_supported = false;

        switch (atlas_format) {
            case DONG_IMAGE_FORMAT_BC7:
                gpu_format = sdl_backend::GPUCompressFormat::BC7;
                format_supported = gpu_compressor_ && gpu_compressor_->isFormatSupported(gpu_format);
                break;
            case DONG_IMAGE_FORMAT_ASTC_4x4:
                gpu_format = sdl_backend::GPUCompressFormat::ASTC_4x4;
                format_supported = gpu_compressor_ && gpu_compressor_->isFormatSupported(gpu_format);
                break;
            case DONG_IMAGE_FORMAT_ASTC_5x5:
                gpu_format = sdl_backend::GPUCompressFormat::ASTC_5x5;
                format_supported = gpu_compressor_ && gpu_compressor_->isFormatSupported(gpu_format);
                break;
            case DONG_IMAGE_FORMAT_ASTC_6x6:
                gpu_format = sdl_backend::GPUCompressFormat::ASTC_6x6;
                format_supported = gpu_compressor_ && gpu_compressor_->isFormatSupported(gpu_format);
                break;
            case DONG_IMAGE_FORMAT_ASTC_8x8:
                gpu_format = sdl_backend::GPUCompressFormat::ASTC_8x8;
                format_supported = gpu_compressor_ && gpu_compressor_->isFormatSupported(gpu_format);
                break;
            default:
                break;
        }

        // Try GPU compression directly to atlas texture region
        if (format_supported && atlas_texture) {
            if (gpu_compressor_->compressToTextureRegion(
                    pixels.data(), img_w, img_h, gpu_format,
                    atlas_texture, atlas_entry.x, atlas_entry.y)) {
                gpu_compressed = true;
                DONG_LOG_DEBUG("SDLGPUDriver::ensureImageInAtlas: GPU compressed '%s' %ux%u directly to atlas (%u,%u) (%s)",
                        src.c_str(), img_w, img_h, atlas_entry.x, atlas_entry.y,
                        gpu_format == sdl_backend::GPUCompressFormat::BC7 ? "BC7" :
                        gpu_format == sdl_backend::GPUCompressFormat::ASTC_4x4 ? "ASTC_4x4" :
                        gpu_format == sdl_backend::GPUCompressFormat::ASTC_5x5 ? "ASTC_5x5" :
                        gpu_format == sdl_backend::GPUCompressFormat::ASTC_6x6 ? "ASTC_6x6" : "ASTC_8x8");
            } else {
                DONG_LOG_DEBUG("SDLGPUDriver::ensureImageInAtlas: GPU direct compression failed for '%s', falling back to CPU",
                        src.c_str());
            }
        }

        // Fall back to CPU compression if GPU failed or not available
        if (!gpu_compressed) {
            DongPlatform* platform = dong_platform_get();
            DongImageDecoder* encoder = dong_platform_get_image_decoder(platform);

            if (!encoder) {
                DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: no image encoder available for compression");
                return false;
            }

            if (!dong_image_can_encode(encoder, DONG_IMAGE_FORMAT_RGBA8, atlas_format)) {
                DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: encoding from RGBA8 to %s not supported",
                        dong_image_format_name(atlas_format));
                return false;
            }

            DongDecodedImage src_img = {0};
            src_img.data = const_cast<void*>(static_cast<const void*>(pixels.data()));
            src_img.data_size = pixels.size();
            src_img.width = img_w;
            src_img.height = img_h;
            src_img.row_bytes = img_w * 4;
            src_img.format = DONG_IMAGE_FORMAT_RGBA8;
            src_img.mip_levels = 1;

            DongDecodedImage encoded_img = {0};
            DongEncodeOptions encode_opts = dong_encode_options_default();

            DongImageDecoderResult encode_result = dong_image_encode(encoder, &src_img, atlas_format, &encode_opts, &encoded_img);
            if (encode_result != DONG_IMAGE_OK) {
                DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: encoding to %s failed for '%s', error=%d",
                        dong_image_format_name(atlas_format), src.c_str(), (int)encode_result);
                return false;
            }

            // Upload CPU compressed data to atlas
            result = dong_atlas_upload(image_atlas_, &atlas_entry, encoded_img.data, encoded_img.data_size);
            dong_image_free(encoder, &encoded_img);

            if (result != DONG_ATLAS_OK) {
                DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: atlas upload failed for '%s', error=%d",
                        src.c_str(), (int)result);
                return false;
            }

            DONG_LOG_DEBUG("SDLGPUDriver::ensureImageInAtlas: CPU compressed '%s' %ux%u (%s)",
                    src.c_str(), img_w, img_h, dong_image_format_name(atlas_format));
        }
    } else {
        // Uncompressed format - direct upload
        result = dong_atlas_upload(image_atlas_, &atlas_entry, pixels.data(), pixels.size());
        if (result != DONG_ATLAS_OK) {
            DONG_LOG_ERROR("SDLGPUDriver::ensureImageInAtlas: atlas upload failed for '%s', error=%d",
                    src.c_str(), (int)result);
            return false;
        }
    }

    // Create cache entry
    ImageAtlasEntry entry{};
    entry.width = img_w;
    entry.height = img_h;
    entry.u0 = atlas_entry.u0;
    entry.v0 = atlas_entry.v0;
    entry.u1 = atlas_entry.u1;
    entry.v1 = atlas_entry.v1;

    image_atlas_entries_[src] = entry;
    out_entry = entry;
    return true;
}

void SDLGPUDriver::reapUploadBuffers(SDL_GPUDevice* dev) {
    if (!dev) return;

    // Move any finished frame upload buffers back to the free list.
    for (size_t i = 0; i < pending_upload_buffers_.size();) {
        PendingUploadBuffers& p = pending_upload_buffers_[i];
        if (!p.fence) {
            for (auto& b : p.buffers) {
                if (b.buf) free_upload_buffers_.push_back(b);
            }
            pending_upload_buffers_.erase(pending_upload_buffers_.begin() + (ptrdiff_t)i);
            continue;
        }

        if (SDL_QueryGPUFence(dev, static_cast<SDL_GPUFence*>(p.fence))) {
            for (auto& b : p.buffers) {
                if (b.buf) free_upload_buffers_.push_back(b);
            }
            SDL_ReleaseGPUFence(dev, static_cast<SDL_GPUFence*>(p.fence));
            pending_upload_buffers_.erase(pending_upload_buffers_.begin() + (ptrdiff_t)i);
            continue;
        }

        ++i;
    }
}

SDLGPUDriver::UploadBuffer SDLGPUDriver::acquireUploadBuffer(SDL_GPUDevice* dev, uint32_t size) {
    UploadBuffer out;
    if (!dev || size == 0) return out;

    // Best-fit from free list.
    size_t best = (size_t)-1;
    uint32_t best_size = 0;
    for (size_t i = 0; i < free_upload_buffers_.size(); ++i) {
        const UploadBuffer& b = free_upload_buffers_[i];
        if (!b.buf || b.size < size) continue;
        if (best == (size_t)-1 || b.size < best_size) {
            best = i;
            best_size = b.size;
            if (best_size == size) break;
        }
    }

    if (best != (size_t)-1) {
        out = free_upload_buffers_[best];
        free_upload_buffers_.erase(free_upload_buffers_.begin() + (ptrdiff_t)best);
        return out;
    }

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = size;

    out.buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
    out.size = out.buf ? size : 0;
    return out;
}

SDL_GPUCommandBuffer* SDLGPUDriver::acquireCommandBufferForUploads(SDL_GPUDevice* dev, bool& out_temp) {
    out_temp = false;
    if (current_cmd_buf_) {
        return current_cmd_buf_;
    }
    if (!gpu_device_ || !gpu_device_->isInitialized() || !dev) {
        return nullptr;
    }

    reapUploadBuffers(dev);

    SDL_GPUCommandBuffer* cmd = gpu_device_->acquireCommandBuffer();
    if (!cmd) {
        return nullptr;
    }

    out_temp = true;
    return cmd;
}

void SDLGPUDriver::submitStandaloneUploadCommandBuffer(SDL_GPUDevice* dev, SDL_GPUCommandBuffer* cmd_buf) {
    if (!gpu_device_ || !gpu_device_->isInitialized() || !cmd_buf) {
        return;
    }

    if (dev && !frame_upload_buffers_.empty()) {
        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
        if (fence) {
            PendingUploadBuffers pending;
            pending.fence = fence;
            pending.buffers = std::move(frame_upload_buffers_);
            pending_upload_buffers_.push_back(std::move(pending));
            frame_upload_buffers_.clear();
            return;
        }

        DONG_LOG_ERROR("SDLGPUDriver: fence acquisition failed for standalone upload: %s", SDL_GetError());
        gpu_device_->submitCommandBuffer(cmd_buf);
        gpu_device_->waitForGPU();

        for (auto& b : frame_upload_buffers_) {
            if (b.buf) free_upload_buffers_.push_back(b);
        }
        frame_upload_buffers_.clear();
        return;
    }

    gpu_device_->submitCommandBuffer(cmd_buf);
}

} // namespace render
} // namespace dong

