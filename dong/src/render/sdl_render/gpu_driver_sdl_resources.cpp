#include "../gpu_driver_sdl.hpp"


#include "../gpu_device.hpp"
#include "../resource_manager.hpp"
#include "../glyph_atlas.hpp"
#include "../font_resolver.hpp"
#include "../../core/log.h"
#include "../../core/profiler.h"

#include <SDL3/SDL_log.h>

// ImageAtlas and ImageDecoder for compression support
#include "dong_image_atlas.h"
#include "dong_image_decoder.h"
#include "dong_platform.h"

#include <vector>
#include <algorithm>
#include <cstring>
#include <utility>
#include <unordered_map>
#include <unordered_set>

namespace dong::render {

void GPUDriverSDL::prepareResources(const GPUCommandList& commands) {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("[prepareResources] SKIP: gpu_device not ready");
        return;
    }

    DONG_LOG_VERBOSE("[prepareResources] START frame=%llu commands=%zu", frame_index_ + 1, commands.commands.size());

    // ========== 优化：批量收集所有 glyph，按 atlas tier 分组后一次性上传 ==========
    //
    // 之前的实现为每个 glyph 单独调用 addGlyph()，导致每个 glyph 都有一次 GPU sync。
    // 新实现：
    // 1. 遍历所有 DrawText 命令，收集需要添加的 glyph 请求
    // 2. 按 atlas tier 分组
    // 3. 每个 tier 调用 addGlyphsBatched() 一次性处理
    //

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
        GlyphAtlasTier* glyph_tier = selectGlyphAtlasTier(font_size);
        if (!glyph_tier || !glyph_tier->atlas) {
            continue;
        }

        uint32_t tier_key = glyph_tier->bitmap_px;

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

        // 找到对应的 tier
        GlyphAtlasTier* tier = nullptr;
        for (auto& t : glyph_atlas_tiers_) {
            if (t.bitmap_px == tier_key) {
                tier = &t;
                break;
            }
        }

        if (!tier || !tier->atlas) continue;

        DONG_LOG_DEBUG("[prepareResources] tier %upx: batching %zu glyph requests", tier_key, requests.size());

        // 使用批量接口添加
        tier->atlas->addGlyphsBatched(requests);
    }

    DONG_LOG_DEBUG("[prepareResources] END frame=%llu", frame_index_ + 1);
}

bool GPUDriverSDL::ensureImageInAtlas(const std::string& src, ImageAtlasEntry& out_entry) {
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
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: no image resource manager bound");
        return false;
    }

    // Load image pixels via ResourceManager (uses IImageDecoder)
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

        SDL_Log("GPUDriverSDL::ensureImageInAtlas: downscaling '%s' from %ux%u to %ux%u to fit atlas",
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
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: atlas allocation failed for '%s' (%ux%u), error=%d",
                src.c_str(), img_w, img_h, (int)result);
        return false;
    }

    // Prepare upload data
    // If atlas uses compressed format, encode the image first
    const void* upload_data = pixels.data();
    size_t upload_size = pixels.size();
    std::vector<uint8_t> compressed_data;  // Holds encoded data if compression is used

    DongImageFormat atlas_format = image_atlas_->config.format;
    if (dong_image_format_is_compressed(atlas_format)) {
        // Get ImageDecoder from platform
        DongPlatform* platform = dong_platform_get();
        DongImageDecoder* encoder = dong_platform_get_image_decoder(platform);

        if (!encoder) {
            SDL_Log("GPUDriverSDL::ensureImageInAtlas: no image encoder available for compression");
            return false;
        }

        // Check if encoding is supported
        if (!dong_image_can_encode(encoder, DONG_IMAGE_FORMAT_RGBA8, atlas_format)) {
            SDL_Log("GPUDriverSDL::ensureImageInAtlas: encoding from RGBA8 to %s not supported",
                    dong_image_format_name(atlas_format));
            return false;
        }

        // Prepare source image
        DongDecodedImage src_img = {0};
        src_img.data = const_cast<void*>(static_cast<const void*>(pixels.data()));
        src_img.data_size = pixels.size();
        src_img.width = img_w;
        src_img.height = img_h;
        src_img.row_bytes = img_w * 4;
        src_img.format = DONG_IMAGE_FORMAT_RGBA8;
        src_img.mip_levels = 1;

        // Encode to compressed format
        DongDecodedImage encoded_img = {0};
        DongEncodeOptions encode_opts = dong_encode_options_default();

        DongImageDecoderResult encode_result = dong_image_encode(encoder, &src_img, atlas_format, &encode_opts, &encoded_img);
        if (encode_result != DONG_IMAGE_OK) {
            SDL_Log("GPUDriverSDL::ensureImageInAtlas: encoding to %s failed for '%s', error=%d",
                    dong_image_format_name(atlas_format), src.c_str(), (int)encode_result);
            return false;
        }

        // Copy encoded data to our buffer (so we can free the encoder's allocation)
        compressed_data.resize(encoded_img.data_size);
        std::memcpy(compressed_data.data(), encoded_img.data, encoded_img.data_size);

        // Free encoder's allocation
        dong_image_free(encoder, &encoded_img);

        upload_data = compressed_data.data();
        upload_size = compressed_data.size();

        DONG_LOG_DEBUG("GPUDriverSDL::ensureImageInAtlas: compressed '%s' from %zu to %zu bytes (%s)",
                src.c_str(), pixels.size(), upload_size, dong_image_format_name(atlas_format));
    }

    // Upload pixel data to atlas
    result = dong_atlas_upload(image_atlas_, &atlas_entry, upload_data, upload_size);
    if (result != DONG_ATLAS_OK) {
        SDL_Log("GPUDriverSDL::ensureImageInAtlas: atlas upload failed for '%s', error=%d",
                src.c_str(), (int)result);
        return false;
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

} // namespace dong::render
