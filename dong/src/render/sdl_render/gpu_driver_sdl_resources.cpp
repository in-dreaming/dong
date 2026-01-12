#include "../gpu_driver_sdl.hpp"


#include "../gpu_device.hpp"
#include "../resource_manager.hpp"
#include "../glyph_atlas.hpp"
#include "../font_resolver.hpp"
#include "../../core/log.h"

#include <SDL3/SDL_log.h>

#include <vector>
#include <algorithm>
#include <cstring>
#include <utility>

namespace dong::render {

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
        // Fallback: downscale (nearest) to fit into atlas.
        // This keeps the rendering path simple (single atlas) while ensuring correctness.
        const uint32_t src_w = img_w;
        const uint32_t src_h = img_h;

        uint32_t new_w = img_w;
        uint32_t new_h = img_h;

        if (new_w > image_atlas_width_) {
            new_h = static_cast<uint32_t>((static_cast<uint64_t>(new_h) * image_atlas_width_) / new_w);
            new_w = image_atlas_width_;
        }
        if (new_h > image_atlas_height_) {
            new_w = static_cast<uint32_t>((static_cast<uint64_t>(new_w) * image_atlas_height_) / new_h);
            new_h = image_atlas_height_;
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
    tex_transfer.pixels_per_row = img_w;
    tex_transfer.rows_per_layer = img_h;

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

} // namespace dong::render
