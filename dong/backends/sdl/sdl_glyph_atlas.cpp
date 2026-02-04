#include "sdl_glyph_atlas.hpp"
#include "sdl_gpu_device.hpp"
#include "../../src/render/font_metrics.hpp"
#include "../../src/core/log.h"
#include "../../src/core/profiler.h"
#include <cstring>
#include <cstdio>
#include <algorithm>

#include <msdfgen/msdfgen.h>
#include <msdfgen/core/edge-coloring.h>
#include <msdfgen/core/rasterization.h>
#include <msdfgen/ext/import-font.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

namespace dong {
namespace render {

GlyphAtlas::GlyphAtlas(GPUDevice* gpu_device)
    : gpu_device_(gpu_device) {}

GlyphAtlas::~GlyphAtlas() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        return;
    }
    SDL_GPUDevice* dev = gpu_device_->getHandle();
    waitAllPendingUploads(dev);
    for (auto& page : pages_) {
        if (page.texture) {
            SDL_ReleaseGPUTexture(dev, page.texture);
            page.texture = nullptr;
        }
    }
    pages_.clear();
}

SDL_GPUTexture* GlyphAtlas::getAtlasTexture() const {
    if (pages_.empty()) return nullptr;
    return pages_.front().texture;
}

SDL_GPUTexture* GlyphAtlas::getAtlasTextureForPage(uint32_t page_index) const {
    if (page_index >= pages_.size()) return nullptr;
    return pages_[page_index].texture;
}

std::string GlyphAtlas::makeGlyphKey(uint32_t glyph_id, const std::string& font_path) const {
    std::string key = font_path;
    key.push_back('#');
    key.append(std::to_string(glyph_id));
    return key;
}

bool GlyphAtlas::createPage() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_ERROR("GlyphAtlas::createPage: GPU device not initialized");
        return false;
    }
    if (pages_.size() >= max_pages_) return false;

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.width = atlas_width_;
    tex_info.height = atlas_height_;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(dev, &tex_info);
    if (!texture) {
        DONG_LOG_ERROR("GlyphAtlas::createPage: failed to create atlas texture");
        return false;
    }

    // Initialize to black
    {
        uint32_t buffer_size = atlas_width_ * atlas_height_ * 4;
        SDL_GPUTransferBufferCreateInfo transfer_info{};
        transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_info.size = buffer_size;
        SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
        if (transfer_buf) {
            void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
            if (mapped) {
                std::memset(mapped, 0, buffer_size);
                SDL_UnmapGPUTransferBuffer(dev, transfer_buf);
                SDL_GPUCommandBuffer* cmd_buf = gpu_device_->acquireCommandBuffer();
                if (cmd_buf) {
                    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
                    if (copy_pass) {
                        SDL_GPUTextureTransferInfo tex_transfer{};
                        tex_transfer.transfer_buffer = transfer_buf;
                        tex_transfer.offset = 0;
                        tex_transfer.pixels_per_row = atlas_width_;
                        tex_transfer.rows_per_layer = atlas_height_;
                        SDL_GPUTextureRegion region{};
                        region.texture = texture;
                        region.mip_level = 0;
                        region.layer = 0;
                        region.x = 0; region.y = 0; region.z = 0;
                        region.w = atlas_width_; region.h = atlas_height_; region.d = 1;
                        SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
                        SDL_EndGPUCopyPass(copy_pass);
                    }
                    gpu_device_->submitCommandBuffer(cmd_buf);
                    gpu_device_->waitForGPU();
                }
            }
            SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        }
    }

    AtlasPage page{};
    page.texture = texture;
    page.width = atlas_width_;
    page.height = atlas_height_;
    page.cursor_x = 0;
    page.cursor_y = 0;
    page.row_height = 0;
    page.page_index = static_cast<uint32_t>(pages_.size());
    page.glyph_count = 0;
    page.last_used = 0;
    pages_.push_back(page);
    return true;
}

bool GlyphAtlas::initialize(uint32_t width, uint32_t height,
                            uint32_t glyph_bitmap_size,
                            float glyph_distance_range) {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        DONG_LOG_ERROR("GlyphAtlas::initialize: GPU device not initialized");
        return false;
    }
    atlas_width_ = width;
    atlas_height_ = height;
    glyph_bitmap_size_ = glyph_bitmap_size;
    glyph_distance_range_ = glyph_distance_range;
    cache_.clear();
    page_to_keys_.clear();
    pages_.clear();
    usage_counter_ = 0;
    if (!createPage()) {
        DONG_LOG_ERROR("GlyphAtlas::initialize: failed to create initial page");
        return false;
    }
    DONG_LOG_INFO("GlyphAtlas initialized: %u x %u, max_pages=%u", atlas_width_, atlas_height_, max_pages_);
    return true;
}

const AtlasEntry* GlyphAtlas::getGlyph(uint32_t glyph_id, const std::string& font_path) {
    if (font_path.empty()) return nullptr;
    auto it = cache_.find(makeGlyphKey(glyph_id, font_path));
    if (it != cache_.end()) {
        const AtlasEntry& entry = it->second;
        if (entry.atlas_page < pages_.size()) {
            pages_[entry.atlas_page].last_used = ++usage_counter_;
        }
        return &it->second;
    }
    return nullptr;
}

// Stub implementations for remaining methods
// Full implementation would be ~1000 lines, copied from src/render/glyph_atlas.cpp

void GlyphAtlas::reapPendingUploads(SDL_GPUDevice* dev) {
    if (!dev) return;
    for (size_t i = 0; i < pending_uploads_.size();) {
        PendingUpload& p = pending_uploads_[i];
        if (!p.fence || SDL_QueryGPUFence(dev, p.fence)) {
            for (auto* tb : p.buffers) {
                if (tb) SDL_ReleaseGPUTransferBuffer(dev, tb);
            }
            if (p.fence) SDL_ReleaseGPUFence(dev, p.fence);
            pending_uploads_.erase(pending_uploads_.begin() + (ptrdiff_t)i);
            continue;
        }
        ++i;
    }
}

void GlyphAtlas::waitAllPendingUploads(SDL_GPUDevice* dev) {
    if (!dev || pending_uploads_.empty()) return;
    if (gpu_device_) gpu_device_->waitForGPU();
    for (auto& p : pending_uploads_) {
        for (auto* tb : p.buffers) {
            if (tb) SDL_ReleaseGPUTransferBuffer(dev, tb);
        }
        if (p.fence) SDL_ReleaseGPUFence(dev, p.fence);
    }
    pending_uploads_.clear();
}

// Forward declaration for generateMSDF
extern FT_Face getOrCreateDesignUnitsFace(const std::string& font_path);

bool GlyphAtlas::generateMSDF(uint32_t glyph_id, const std::string& font_path,
                              std::vector<uint8_t>& out_bitmap,
                              uint32_t& out_width, uint32_t& out_height,
                              GlyphMetrics& out_metrics) {
    // Stub - full implementation requires FreeType + msdfgen integration
    // See src/render/glyph_atlas.cpp for complete implementation
    (void)glyph_id;
    (void)font_path;
    out_width = 0;
    out_height = 0;
    return false;
}

const AtlasEntry* GlyphAtlas::addGlyph(uint32_t glyph_id, const std::string& font_path) {
    // Stub - full implementation ~200 lines
    // See src/render/glyph_atlas.cpp
    (void)glyph_id;
    (void)font_path;
    return nullptr;
}

void GlyphAtlas::addGlyphsBatched(const std::vector<GlyphRequest>& requests) {
    // Stub - full implementation ~200 lines
    // See src/render/glyph_atlas.cpp
    (void)requests;
}

GlyphAtlas::AtlasPage* GlyphAtlas::evictAndRecyclePage() {
    // Stub
    return nullptr;
}

GlyphAtlas::AtlasPage* GlyphAtlas::selectPageForGlyph(uint32_t glyph_width, uint32_t glyph_height) {
    // Stub
    (void)glyph_width;
    (void)glyph_height;
    return nullptr;
}

} // namespace render
} // namespace dong
