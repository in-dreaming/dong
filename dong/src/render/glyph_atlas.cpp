#include "glyph_atlas.hpp"
#include "gpu_device.hpp"
#include "font_metrics.hpp"
#include "../core/log.h"
#include "../core/profiler.h"
#include <SDL3/SDL_log.h>
#include <cstring>

#include <cstdio>
#include <algorithm>

// MSDF 生成库
#include <msdfgen/msdfgen.h>
#include <msdfgen/core/edge-coloring.h>
#include <msdfgen/core/rasterization.h>
#include <msdfgen/ext/import-font.h>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

namespace dong::render {

GlyphAtlas::GlyphAtlas(GPUDevice* gpu_device)
    : gpu_device_(gpu_device) {}

GlyphAtlas::~GlyphAtlas() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();

    // 尽量确保所有异步上传完成后再释放 transfer buffer。
    // 这里选择等待是因为 GlyphAtlas 生命周期结束后，我们无法再安全地延后释放。
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
    if (pages_.empty()) {
        return nullptr;
    }
    return pages_.front().texture;
}

SDL_GPUTexture* GlyphAtlas::getAtlasTextureForPage(uint32_t page_index) const {
    if (page_index >= pages_.size()) {
        return nullptr;
    }
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
        SDL_Log("GlyphAtlas::createPage: GPU device not initialized");
        return false;
    }

    if (pages_.size() >= max_pages_) {
        return false;
    }

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
        SDL_Log("GlyphAtlas::createPage: failed to create atlas texture: %s", SDL_GetError());
        return false;
    }

    // 关键：GPU 纹理创建后内容是未定义的，必须初始化为全黑（MSDF 外部值 = 0）
    // 否则 glyph 之间的 padding 区域会包含垃圾数据，导致渲染时出现细线
    {
        uint32_t buffer_size = atlas_width_ * atlas_height_ * 4;
        
        SDL_GPUTransferBufferCreateInfo transfer_info{};
        transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transfer_info.size = buffer_size;
        
        SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
        if (transfer_buf) {
            void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
            if (mapped) {
                // 全黑 = RGBA(0,0,0,255)，MSDF 中 0 表示字形外部（距离场负值）
                std::memset(mapped, 0, buffer_size);
                SDL_UnmapGPUTransferBuffer(dev, transfer_buf);
                
                SDL_GPUCommandBuffer* cmd_buf = gpu_device_->acquireCommandBuffer();
                if (cmd_buf) {
                    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
                    if (copy_pass) {
                        SDL_GPUTextureTransferInfo tex_transfer{};
                        tex_transfer.transfer_buffer = transfer_buf;
                        tex_transfer.offset = 0;
                        tex_transfer.pixels_per_row = atlas_width_;   // 关键：设置每行像素数
                        tex_transfer.rows_per_layer = atlas_height_;  // 关键：设置每层行数
                        
                        SDL_GPUTextureRegion region{};
                        region.texture = texture;
                        region.mip_level = 0;
                        region.layer = 0;
                        region.x = 0;
                        region.y = 0;
                        region.z = 0;
                        region.w = atlas_width_;
                        region.h = atlas_height_;
                        region.d = 1;
                        
                        SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
                        SDL_EndGPUCopyPass(copy_pass);
                    }

                    // 使用 fence 精确等待该 command buffer 完成，避免某些后端 copy/transfer 队列
                    // 在 WaitForGPUIdle 下仍存在时序差异，导致首帧采样到未完成上传的 atlas。
                    SDL_GPUFence* fence = nullptr;
                    {
                        DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBufferAndAcquireFence", "gpu");
                        fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
                    }
                    if (fence) {
                        SDL_GPUFence* fences[] = { fence };
                        {
                            DONG_PROFILE_SCOPE_CAT("SDL_WaitForGPUFences", "gpu");
                            if (!SDL_WaitForGPUFences(dev, true, fences, 1)) {
                                SDL_Log("GlyphAtlas::evictAndRecyclePage: SDL_WaitForGPUFences failed: %s", SDL_GetError());
                                gpu_device_->waitForGPU();
                            }
                        }
                        SDL_ReleaseGPUFence(dev, fence);
                    } else {

                        SDL_Log("GlyphAtlas::evictAndRecyclePage: SDL_SubmitGPUCommandBufferAndAcquireFence failed: %s", SDL_GetError());

                        gpu_device_->submitCommandBuffer(cmd_buf);
                        gpu_device_->waitForGPU();
                    }


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
    DONG_LOG_DEBUG("GlyphAtlas: created page %u (%u x %u), initialized to black", page.page_index, atlas_width_, atlas_height_);
    return true;
}

GlyphAtlas::AtlasPage* GlyphAtlas::evictAndRecyclePage() {
    if (pages_.empty()) {
        return nullptr;
    }

    // 选择最久未使用且承载了 glyph 的页
    uint64_t oldest_use = UINT64_MAX;
    AtlasPage* victim = nullptr;
    for (auto& page : pages_) {
        if (page.glyph_count == 0) {
            continue;
        }
        if (page.last_used < oldest_use) {
            oldest_use = page.last_used;
            victim = &page;
        }
    }

    if (!victim) {
        // 所有页都没有 glyph，直接复用第 0 页
        victim = &pages_.front();
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    if (victim->texture) {
        SDL_ReleaseGPUTexture(dev, victim->texture);
        victim->texture = nullptr;
    }

    // 从缓存中删除该页上的所有 glyph
    auto it = page_to_keys_.find(victim->page_index);
    if (it != page_to_keys_.end()) {
        for (const auto& key : it->second) {
            cache_.erase(key);
        }
        it->second.clear();
    }

    // 重新创建纹理并重置装箱状态
    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.width = atlas_width_;
    tex_info.height = atlas_height_;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    victim->texture = SDL_CreateGPUTexture(dev, &tex_info);
    if (!victim->texture) {
        SDL_Log("GlyphAtlas::evictAndRecyclePage: failed to recreate atlas texture: %s", SDL_GetError());
        victim->width = 0;
        victim->height = 0;
        victim->cursor_x = victim->cursor_y = victim->row_height = 0;
        victim->glyph_count = 0;
        victim->last_used = 0;
        return nullptr;
    }

    // 关键：初始化纹理为全黑，避免垃圾数据导致的细线问题
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
                        
                        SDL_GPUTextureRegion region{};
                        region.texture = victim->texture;
                        region.mip_level = 0;
                        region.layer = 0;
                        region.x = 0;
                        region.y = 0;
                        region.z = 0;
                        region.w = atlas_width_;
                        region.h = atlas_height_;
                        region.d = 1;
                        
                        SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
                        SDL_EndGPUCopyPass(copy_pass);
                    }

                    SDL_GPUFence* fence = nullptr;
                    {
                        DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBufferAndAcquireFence", "gpu");
                        fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
                    }
                    if (fence) {
                        SDL_GPUFence* fences[] = { fence };
                        {
                            DONG_PROFILE_SCOPE_CAT("SDL_WaitForGPUFences", "gpu");
                            if (!SDL_WaitForGPUFences(dev, true, fences, 1)) {
                                SDL_Log("GlyphAtlas::evictAndRecyclePage: SDL_WaitForGPUFences failed: %s", SDL_GetError());
                                gpu_device_->waitForGPU();
                            }
                        }
                        SDL_ReleaseGPUFence(dev, fence);
                    } else {

                        SDL_Log("GlyphAtlas::evictAndRecyclePage: SDL_SubmitGPUCommandBufferAndAcquireFence failed: %s", SDL_GetError());

                        gpu_device_->submitCommandBuffer(cmd_buf);
                        gpu_device_->waitForGPU();
                    }


                }
            }
            SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        }
    }

    victim->width = atlas_width_;
    victim->height = atlas_height_;
    victim->cursor_x = 0;
    victim->cursor_y = 0;
    victim->row_height = 0;
    victim->glyph_count = 0;
    victim->last_used = 0;

    DONG_LOG_DEBUG("GlyphAtlas: evicted and recycled page %u", victim->page_index);
    return victim;
}

GlyphAtlas::AtlasPage* GlyphAtlas::selectPageForGlyph(uint32_t glyph_width, uint32_t glyph_height) {
    if (glyph_width == 0 || glyph_height == 0) {
        return nullptr;
    }
    if (glyph_width > atlas_width_ || glyph_height > atlas_height_) {
        SDL_Log("GlyphAtlas::selectPageForGlyph: glyph too large for atlas page (%u x %u)",
                glyph_width, glyph_height);
        return nullptr;
    }

    // Atlas 中 glyph 之间的 padding，防止线性采样时采样到相邻 glyph
    // 2 像素足以防止双线性插值时的边界泄漏
    constexpr uint32_t kAtlasPadding = 2;

    // 先尝试在现有页中找到能容纳该 glyph 的页
    for (auto& page : pages_) {
        uint32_t test_cursor_x = page.cursor_x;
        uint32_t test_cursor_y = page.cursor_y;
        uint32_t test_row_height = page.row_height;

        if (test_cursor_x + glyph_width + kAtlasPadding > page.width) {
            test_cursor_x = 0;
            test_cursor_y += test_row_height + kAtlasPadding;
            test_row_height = 0;
        }

        if (test_cursor_y + glyph_height <= page.height) {
            return &page;
        }
    }

    // 没有合适页：尝试新建一页
    if (pages_.size() < max_pages_) {
        if (!createPage()) {
            return nullptr;
        }
        return &pages_.back();
    }

    // 超过最大页数：进行页级 LRU 淘汰并复用
    return evictAndRecyclePage();
}

bool GlyphAtlas::initialize(uint32_t width,
                             uint32_t height,
                             uint32_t glyph_bitmap_size,
                             float glyph_distance_range) {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        SDL_Log("GlyphAtlas::initialize: GPU device not initialized");
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
    if (font_path.empty()) {
        return nullptr;
    }
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

const AtlasEntry* GlyphAtlas::addGlyph(uint32_t glyph_id, const std::string& font_path) {
    if (font_path.empty()) {
        SDL_Log("GlyphAtlas::addGlyph: font path is empty for glyph %u", glyph_id);
        return nullptr;
    }

    // 检查是否已缓存
    std::string key = makeGlyphKey(glyph_id, font_path);
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        const AtlasEntry& entry = it->second;
        if (entry.atlas_page < pages_.size()) {
            pages_[entry.atlas_page].last_used = ++usage_counter_;
        }
        return &it->second;
    }

    // 生成 MSDF 位图
    std::vector<uint8_t> bitmap;
    uint32_t glyph_width = 0;
    uint32_t glyph_height = 0;
    GlyphMetrics metrics;

    if (!generateMSDF(glyph_id, font_path, bitmap, glyph_width, glyph_height, metrics)) {
        SDL_Log("GlyphAtlas::addGlyph: failed to generate MSDF for glyph %u", glyph_id);
        return nullptr;
    }

    if (glyph_width == 0 || glyph_height == 0 || bitmap.empty()) {
        // 空字形（如空格）
        AtlasEntry entry{};
        entry.metrics = metrics;
        entry.atlas_page = 0;
        cache_[key] = entry;
        return &cache_[key];
    }

    AtlasPage* page = selectPageForGlyph(glyph_width, glyph_height);
    if (!page || !page->texture) {
        SDL_Log("GlyphAtlas::addGlyph: no available atlas page for glyph %u", glyph_id);
        return nullptr;
    }

    // Atlas 中 glyph 之间的 padding，防止线性采样时采样到相邻 glyph
    constexpr uint32_t kAtlasPadding = 2;

    if (page->cursor_x + glyph_width + kAtlasPadding > page->width) {
        page->cursor_x = 0;
        page->cursor_y += page->row_height + kAtlasPadding;
        page->row_height = 0;
    }

    if (page->cursor_y + glyph_height > page->height) {
        SDL_Log("GlyphAtlas::addGlyph: selected page overflows for glyph %u", glyph_id);
        return nullptr;
    }

    uint32_t dst_x = page->cursor_x;
    uint32_t dst_y = page->cursor_y;
    page->cursor_x += glyph_width + kAtlasPadding;
    page->row_height = std::max(page->row_height, glyph_height);

    // 上传到 GPU（通过 transfer buffer）
    SDL_GPUDevice* dev = gpu_device_->getHandle();
    SDL_GPUCommandBuffer* cmd_buf = gpu_device_->acquireCommandBuffer();
    if (!cmd_buf) {
        SDL_Log("GlyphAtlas::addGlyph: failed to acquire command buffer");
        return nullptr;
    }

    uint32_t stride = glyph_width * 4;
    uint32_t buffer_size = stride * glyph_height;

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = buffer_size;

    SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
    if (!transfer_buf) {
        SDL_Log("GlyphAtlas::addGlyph: failed to create transfer buffer");
        gpu_device_->submitCommandBuffer(cmd_buf);
        return nullptr;
    }

    void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
    if (!mapped) {
        SDL_Log("GlyphAtlas::addGlyph: failed to map transfer buffer");
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        gpu_device_->submitCommandBuffer(cmd_buf);
        return nullptr;
    }

    uint8_t* dst_bytes = static_cast<uint8_t*>(mapped);
    // 直接复制，Y 轴翻转已经在 generateMSDF 中完成
    std::memcpy(dst_bytes, bitmap.data(), bitmap.size());
    
    // DEBUG: 验证上传前的数据
    int upload_non_zero = 0;
    for (size_t i = 0; i < bitmap.size(); i += 4) {
        if (dst_bytes[i] > 0 || dst_bytes[i+1] > 0 || dst_bytes[i+2] > 0) {
            upload_non_zero++;
        }
    }
    DONG_LOG_DEBUG("[ATLAS UPLOAD] glyph=%u bitmap_size=%zu non_zero_pixels=%d dst=(%u,%u) size=(%u,%u) texture=%p",
            glyph_id, bitmap.size(), upload_non_zero, dst_x, dst_y, glyph_width, glyph_height, (void*)page->texture);
    
    SDL_UnmapGPUTransferBuffer(dev, transfer_buf);

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
    if (!copy_pass) {
        SDL_Log("GlyphAtlas::addGlyph: failed to begin copy pass");
        SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);
        gpu_device_->submitCommandBuffer(cmd_buf);
        return nullptr;
    }

    SDL_GPUTextureTransferInfo tex_transfer{};
    tex_transfer.transfer_buffer = transfer_buf;
    tex_transfer.offset = 0;
    tex_transfer.pixels_per_row = glyph_width;  // 关键：设置每行像素数
    tex_transfer.rows_per_layer = glyph_height; // 关键：设置每层行数

    SDL_GPUTextureRegion region{};
    region.texture = page->texture;
    region.mip_level = 0;
    region.layer = 0;
    region.x = dst_x;
    region.y = dst_y;
    region.z = 0;
    region.w = glyph_width;
    region.h = glyph_height;
    region.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_GPUFence* fence = nullptr;
    {
        DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBufferAndAcquireFence", "gpu");
        fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
    }
    if (fence) {
        SDL_GPUFence* fences[] = { fence };
        {
            DONG_PROFILE_SCOPE_CAT("SDL_WaitForGPUFences", "gpu");
            if (!SDL_WaitForGPUFences(dev, true, fences, 1)) {
                SDL_Log("GlyphAtlas::addGlyph: SDL_WaitForGPUFences failed: %s", SDL_GetError());
                gpu_device_->waitForGPU();
            }
        }
        SDL_ReleaseGPUFence(dev, fence);
    } else {

        SDL_Log("GlyphAtlas::addGlyph: SDL_SubmitGPUCommandBufferAndAcquireFence failed: %s", SDL_GetError());
        gpu_device_->submitCommandBuffer(cmd_buf);
        gpu_device_->waitForGPU();
    }


    SDL_ReleaseGPUTransferBuffer(dev, transfer_buf);


    // 创建 AtlasEntry
    AtlasEntry entry{};
    entry.atlas_page = page->page_index;
    entry.u0 = static_cast<float>(dst_x) / static_cast<float>(page->width);
    entry.v0 = static_cast<float>(dst_y) / static_cast<float>(page->height);
    entry.u1 = static_cast<float>(dst_x + glyph_width) / static_cast<float>(page->width);
    entry.v1 = static_cast<float>(dst_y + glyph_height) / static_cast<float>(page->height);
    entry.metrics = metrics;

    cache_[key] = entry;
    page_to_keys_[page->page_index].push_back(key);
    page->glyph_count += 1;
    page->last_used = ++usage_counter_;

    DONG_LOG_DEBUG("GlyphAtlas: added glyph %u (%s) at page %u (%u, %u), size (%u, %u)",
            glyph_id, font_path.c_str(), page->page_index, dst_x, dst_y, glyph_width, glyph_height);

    return &cache_[key];
}

// ============================================================================
// 批量字形添加：减少 GPU 同步开销
// ============================================================================

void GlyphAtlas::addGlyphsBatched(const std::vector<GlyphRequest>& requests) {
    DONG_PROFILE_SCOPE_CAT("addGlyphsBatched", "glyph_atlas");

    if (requests.empty() || !gpu_device_ || !gpu_device_->isInitialized()) {
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
    reapPendingUploads(dev);

    // 阶段1：收集需要添加的字形（过滤已缓存的）
    struct PendingGlyph {
        std::string key;
        uint32_t glyph_id;
        std::string font_path;
        std::vector<uint8_t> bitmap;
        uint32_t width = 0;
        uint32_t height = 0;
        GlyphMetrics metrics;
        AtlasPage* page = nullptr;
        uint32_t dst_x = 0;
        uint32_t dst_y = 0;
    };

    std::vector<PendingGlyph> pending;
    pending.reserve(requests.size());


    {
        DONG_PROFILE_SCOPE_CAT("addGlyphsBatched:generateMSDF", "glyph_atlas");
        for (const auto& req : requests) {
            if (req.font_path.empty()) continue;

            std::string key = makeGlyphKey(req.glyph_id, req.font_path);
            if (cache_.find(key) != cache_.end()) {
                // 已缓存，跳过
                continue;
            }

            PendingGlyph pg;
            pg.key = std::move(key);
            pg.glyph_id = req.glyph_id;
            pg.font_path = req.font_path;

            // 生成 MSDF（CPU-bound）
            if (!generateMSDF(req.glyph_id, req.font_path, pg.bitmap, pg.width, pg.height, pg.metrics)) {
                DONG_LOG_DEBUG("addGlyphsBatched: failed to generate MSDF for glyph %u", req.glyph_id);
                continue;
            }

            // 空字形（如空格）直接缓存
            if (pg.width == 0 || pg.height == 0 || pg.bitmap.empty()) {
                AtlasEntry entry{};
                entry.metrics = pg.metrics;
                entry.atlas_page = 0;
                cache_[pg.key] = entry;
                continue;
            }

            // 分配 atlas 位置
            constexpr uint32_t kAtlasPadding = 2;
            pg.page = selectPageForGlyph(pg.width, pg.height);
            if (!pg.page || !pg.page->texture) {
                DONG_LOG_DEBUG("addGlyphsBatched: no available atlas page for glyph %u", req.glyph_id);
                continue;
            }

            if (pg.page->cursor_x + pg.width + kAtlasPadding > pg.page->width) {
                pg.page->cursor_x = 0;
                pg.page->cursor_y += pg.page->row_height + kAtlasPadding;
                pg.page->row_height = 0;
            }

            if (pg.page->cursor_y + pg.height > pg.page->height) {
                DONG_LOG_DEBUG("addGlyphsBatched: selected page overflows for glyph %u", req.glyph_id);
                continue;
            }

            pg.dst_x = pg.page->cursor_x;
            pg.dst_y = pg.page->cursor_y;
            pg.page->cursor_x += pg.width + kAtlasPadding;
            pg.page->row_height = std::max(pg.page->row_height, pg.height);

            pending.push_back(std::move(pg));
        }
    }

    if (pending.empty()) {
        return;
    }

    DONG_LOG_DEBUG("addGlyphsBatched: processing %zu glyphs in single batch", pending.size());

    // 阶段2：批量上传到 GPU（单个命令缓冲区）。
    // 关键优化：不再在 CPU 侧等待 fence；依赖 command buffer 的提交顺序保证 upload 先于后续 draw。
    SDL_GPUCommandBuffer* cmd_buf = gpu_device_->acquireCommandBuffer();
    if (!cmd_buf) {
        SDL_Log("addGlyphsBatched: failed to acquire command buffer");
        return;
    }


    // 存储需要释放的 transfer buffers
    std::vector<SDL_GPUTransferBuffer*> transfer_buffers;
    transfer_buffers.reserve(pending.size());

    {
        DONG_PROFILE_SCOPE_CAT("addGlyphsBatched:upload", "gpu");

        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
        if (!copy_pass) {
            SDL_Log("addGlyphsBatched: failed to begin copy pass");
            gpu_device_->submitCommandBuffer(cmd_buf);
            return;
        }

        for (auto& pg : pending) {
            uint32_t stride = pg.width * 4;
            uint32_t buffer_size = stride * pg.height;

            SDL_GPUTransferBufferCreateInfo transfer_info{};
            transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
            transfer_info.size = buffer_size;

            SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(dev, &transfer_info);
            if (!transfer_buf) {
                DONG_LOG_DEBUG("addGlyphsBatched: failed to create transfer buffer for glyph %u", pg.glyph_id);
                continue;
            }
            transfer_buffers.push_back(transfer_buf);

            void* mapped = SDL_MapGPUTransferBuffer(dev, transfer_buf, false);
            if (!mapped) {
                DONG_LOG_DEBUG("addGlyphsBatched: failed to map transfer buffer for glyph %u", pg.glyph_id);
                continue;
            }

            std::memcpy(mapped, pg.bitmap.data(), pg.bitmap.size());
            SDL_UnmapGPUTransferBuffer(dev, transfer_buf);

            SDL_GPUTextureTransferInfo tex_transfer{};
            tex_transfer.transfer_buffer = transfer_buf;
            tex_transfer.offset = 0;
            tex_transfer.pixels_per_row = pg.width;
            tex_transfer.rows_per_layer = pg.height;

            SDL_GPUTextureRegion region{};
            region.texture = pg.page->texture;
            region.mip_level = 0;
            region.layer = 0;
            region.x = pg.dst_x;
            region.y = pg.dst_y;
            region.z = 0;
            region.w = pg.width;
            region.h = pg.height;
            region.d = 1;

            SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
        }

        SDL_EndGPUCopyPass(copy_pass);
    }

    // 提交 upload command buffer：优先拿 fence 做异步回收；若拿不到 fence，则退化为同步等待。
    SDL_GPUFence* fence = nullptr;
    {
        DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBufferAndAcquireFence", "gpu");
        fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
    }

    if (fence) {
        PendingUpload pu;
        pu.fence = fence;
        pu.buffers = std::move(transfer_buffers);
        pending_uploads_.push_back(std::move(pu));
    } else {
        SDL_Log("addGlyphsBatched: SDL_SubmitGPUCommandBufferAndAcquireFence failed: %s", SDL_GetError());
        gpu_device_->submitCommandBuffer(cmd_buf);
        gpu_device_->waitForGPU();

        // 退化路径：GPU 已 idle，立即释放 transfer buffers。
        for (auto* tb : transfer_buffers) {
            SDL_ReleaseGPUTransferBuffer(dev, tb);
        }
        transfer_buffers.clear();
    }


    // 阶段3：更新缓存
    for (const auto& pg : pending) {
        if (!pg.page) continue;

        AtlasEntry entry{};
        entry.atlas_page = pg.page->page_index;
        entry.u0 = static_cast<float>(pg.dst_x) / static_cast<float>(pg.page->width);
        entry.v0 = static_cast<float>(pg.dst_y) / static_cast<float>(pg.page->height);
        entry.u1 = static_cast<float>(pg.dst_x + pg.width) / static_cast<float>(pg.page->width);
        entry.v1 = static_cast<float>(pg.dst_y + pg.height) / static_cast<float>(pg.page->height);
        entry.metrics = pg.metrics;

        cache_[pg.key] = entry;
        page_to_keys_[pg.page->page_index].push_back(pg.key);
        pg.page->glyph_count += 1;
        pg.page->last_used = ++usage_counter_;
    }

    DONG_LOG_DEBUG("addGlyphsBatched: completed batch of %zu glyphs", pending.size());
}

void GlyphAtlas::reapPendingUploads(SDL_GPUDevice* dev) {
    if (!dev) return;

    for (size_t i = 0; i < pending_uploads_.size();) {
        PendingUpload& p = pending_uploads_[i];
        if (!p.fence) {
            // 容错：无 fence 直接释放。
            for (auto* tb : p.buffers) {
                if (tb) SDL_ReleaseGPUTransferBuffer(dev, tb);
            }
            pending_uploads_.erase(pending_uploads_.begin() + (ptrdiff_t)i);
            continue;
        }

        if (SDL_QueryGPUFence(dev, p.fence)) {
            for (auto* tb : p.buffers) {
                if (tb) SDL_ReleaseGPUTransferBuffer(dev, tb);
            }
            SDL_ReleaseGPUFence(dev, p.fence);
            pending_uploads_.erase(pending_uploads_.begin() + (ptrdiff_t)i);
            continue;
        }

        ++i;
    }
}

void GlyphAtlas::waitAllPendingUploads(SDL_GPUDevice* dev) {
    if (!dev) return;
    if (pending_uploads_.empty()) return;

    // 更稳妥的 shutdown 路径：直接等待 device idle，然后释放所有 fence/transfer buffers。
    // 相比逐 fence SDL_WaitForGPUFences，这个路径在某些后端/销毁阶段更不容易触发崩溃。
    if (gpu_device_) {
        gpu_device_->waitForGPU();
    }

    for (auto& p : pending_uploads_) {
        for (auto* tb : p.buffers) {
            if (tb) SDL_ReleaseGPUTransferBuffer(dev, tb);
        }
        p.buffers.clear();

        if (p.fence) {
            SDL_ReleaseGPUFence(dev, p.fence);
            p.fence = nullptr;
        }
    }

    pending_uploads_.clear();
}



bool GlyphAtlas::generateMSDF(uint32_t glyph_id, const std::string& font_path,
                               std::vector<uint8_t>& out_bitmap,
                               uint32_t& out_width, uint32_t& out_height,
                               GlyphMetrics& out_metrics) {
    if (font_path.empty()) {
        return false;
    }

    // 使用 design units face（无像素缩放）
    FT_Face face = getOrCreateDesignUnitsFace(font_path);
    if (!face) {
        SDL_Log("GlyphAtlas::generateMSDF: failed to get design units face for '%s'", font_path.c_str());
        return false;
    }

    out_metrics.units_per_em = face->units_per_EM;
    if (out_metrics.units_per_em == 0) {
        SDL_Log("GlyphAtlas::generateMSDF: invalid units_per_em for '%s'", font_path.c_str());
        return false;
    }

    // 加载字形：FT_LOAD_NO_SCALE 获取原始 design units 轮廓
    if (FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_SCALE) != 0) {
        SDL_Log("GlyphAtlas::generateMSDF: failed to load glyph index %u", glyph_id);
        return false;
    }

    // 获取度量信息（design units）
    // FreeType 的 advance/metrics 是 26.6 fixed point，即使 FT_LOAD_NO_SCALE 也一样。
    constexpr float kFtFixedToUnits = 1.0f / 64.0f;
    out_metrics.advance_x_units = static_cast<float>(face->glyph->advance.x) * kFtFixedToUnits;
    out_metrics.bearing_x_units = static_cast<float>(face->glyph->metrics.horiBearingX) * kFtFixedToUnits;
    out_metrics.bearing_y_units = static_cast<float>(face->glyph->metrics.horiBearingY) * kFtFixedToUnits;
    out_metrics.width_units = static_cast<float>(face->glyph->metrics.width) * kFtFixedToUnits;
    out_metrics.height_units = static_cast<float>(face->glyph->metrics.height) * kFtFixedToUnits;


    DONG_LOG_DEBUG("[MSDF] glyph=%u font='%s' units_per_em=%u metrics: adv=%.1f bx=%.1f by=%.1f w=%.1f h=%.1f",
            glyph_id,
            font_path.c_str(),
            out_metrics.units_per_em,
            out_metrics.advance_x_units,
            out_metrics.bearing_x_units,
            out_metrics.bearing_y_units,
            out_metrics.width_units,
            out_metrics.height_units);

    // 计算逻辑 bbox（基线坐标系）
    // 这是用于排版和渲染的统一坐标系：
    // - baseline 在 y=0
    // - logical_top = bearing_y（字形顶部到基线的距离，正值）
    // - logical_bottom = bearing_y - height（字形底部到基线的距离，descender 为负值）
    // - logical_left = bearing_x（字形左边缘相对于 pen position）
    // - logical_right = bearing_x + width
    //
    // 注意：FreeType 的 metrics.horiBearingY 是字形顶部到基线的距离（正值）
    // metrics.height 是字形的总高度
    // 所以 logical_bottom = horiBearingY - height
    // 对于有 descender 的字符（如 y, p, q），logical_bottom 会是负值
    out_metrics.logical_left = out_metrics.bearing_x_units;
    out_metrics.logical_right = out_metrics.bearing_x_units + out_metrics.width_units;
    out_metrics.logical_top = out_metrics.bearing_y_units;
    out_metrics.logical_bottom = out_metrics.bearing_y_units - out_metrics.height_units;

    DONG_LOG_DEBUG("[MSDF] glyph=%u logical: l=%.1f b=%.1f r=%.1f t=%.1f",
            glyph_id,
            out_metrics.logical_left,
            out_metrics.logical_bottom,
            out_metrics.logical_right,
            out_metrics.logical_top);

    // 如果是空字形（如空格），直接返回
    if (face->glyph->outline.n_points == 0) {
        out_width = 0;
        out_height = 0;
        // 空字形没有边界，初始化为 0
        out_metrics.bounds_left = 0.0f;
        out_metrics.bounds_bottom = 0.0f;
        out_metrics.bounds_right = 0.0f;
        out_metrics.bounds_top = 0.0f;
        return true;
    }

    // 将 FreeType outline 转换为 msdfgen::Shape
    // 注意：在 FT_LOAD_NO_SCALE 模式下，outline 点坐标已经是 font units（不是 26.6 fixed point），不需要再做 1/64 缩放。
    msdfgen::Shape shape;
    FT_Error error = msdfgen::readFreetypeOutline(shape, &face->glyph->outline, 1.0);


    if (error != 0) {
        SDL_Log("GlyphAtlas::generateMSDF: failed to convert outline: %d", error);
        out_width = 0;
        out_height = 0;
        out_metrics.bounds_left = 0.0f;
        out_metrics.bounds_bottom = 0.0f;
        out_metrics.bounds_right = 0.0f;
        out_metrics.bounds_top = 0.0f;
        return false;
    }

    if (shape.contours.empty()) {
        SDL_Log("GlyphAtlas::generateMSDF: empty contours for glyph %u", glyph_id);
        out_width = 0;
        out_height = 0;
        // 空字形没有边界，初始化为 0
        out_metrics.bounds_left = 0.0f;
        out_metrics.bounds_bottom = 0.0f;
        out_metrics.bounds_right = 0.0f;
        out_metrics.bounds_top = 0.0f;
        return true;
    }

    // 规范化并着色边缘
    shape.normalize();
    msdfgen::edgeColoringSimple(shape, 3.0);

    // 计算边界（design units），仅用于确定 MSDF 投影区域
    // 注意：真正用于排版/基线对齐的字形度量（bearing/width/height）
    // 已经从 FreeType/OS2 提取，保存在 out_metrics 中，后续不再被 bounds 覆盖。
    // 这样 GlyphAtlas / TextShaper / Painter 全部共享同一套设计单位度量，
    // 与浏览器的基线和行高定义保持一致，避免每个字形因为局部 bounds 差异产生"参差不齐"的视觉错位。
    msdfgen::Shape::Bounds bounds = shape.getBounds();
    
    // 保存实际边界信息，用于精确的 glyph 定位
    out_metrics.bounds_left = static_cast<float>(bounds.l);
    out_metrics.bounds_bottom = static_cast<float>(bounds.b);
    out_metrics.bounds_right = static_cast<float>(bounds.r);
    out_metrics.bounds_top = static_cast<float>(bounds.t);

    DONG_LOG_DEBUG("[MSDF] glyph=%u font='%s' bounds: l=%.1f b=%.1f r=%.1f t=%.1f",
            glyph_id,
            font_path.c_str(),
            bounds.l, bounds.b, bounds.r, bounds.t);

    // 生成 MSDF（固定尺寸，字号无关）
    const int msdf_size = static_cast<int>(glyph_bitmap_size_);
    const double range = static_cast<double>(glyph_distance_range_);
    
    msdfgen::Bitmap<float, 3> msdf(msdf_size, msdf_size);
    // 关键：msdfgen::Bitmap 构造函数不初始化内存，必须手动清零
    // 否则字形边界外的区域会包含垃圾数据，导致渲染时出现细线/噪点
    // 使用 operator float*() 获取像素指针
    std::memset(static_cast<float*>(msdf), 0, sizeof(float) * 3 * msdf_size * msdf_size);
    
    double width = bounds.r - bounds.l;
    double height = bounds.t - bounds.b;
    
    // ========== 关键修复：使用统一的 scale ==========
    //
    // 问题：之前的逻辑让每个字形填满 MSDF 纹理，导致小字形（如冒号、句号）
    // 的 msdf_scale 非常大，渲染时 screenPxRange 太小（< 1），导致模糊。
    //
    // 解决方案：使用基于 units_per_em 的统一 scale，而不是基于字形大小。
    // 这样所有字形使用相同的 msdf_scale，确保 screenPxRange 一致。
    //
    // 计算方式：
    // - 假设典型字形高度约为 units_per_em（实际上大多数字形高度在 0.7~1.0 em）
    // - scale = (msdf_size - 2*range) / units_per_em
    // - 这样 1 em 高度的字形正好填满 MSDF 纹理的可用区域
    //
    // 优点：
    // 1. 所有字形使用相同的 msdf_scale，screenPxRange 一致
    // 2. 小字形（如冒号）在 MSDF 纹理中占用较少空间，但抗锯齿效果正常
    // 3. 大字形（如 "國"）可能超出 MSDF 纹理，需要裁剪或使用更大的 tier
    //
    // 缺点：
    // - 小字形的 MSDF 纹理利用率较低
    // - 大字形可能需要更大的 tier
    //
    // 为了处理超大字形，我们仍然检查字形是否能放入纹理，
    // 如果不能，则回退到基于字形大小的 scale（但设置最小值）
    
    const double units_per_em = static_cast<double>(out_metrics.units_per_em);
    const double available_size = msdf_size - range * 2;
    
    // 基于 units_per_em 的统一 scale
    double uniform_scale = available_size / units_per_em;
    
    // 检查字形是否能放入纹理
    double glyph_width_msdf = width * uniform_scale;
    double glyph_height_msdf = height * uniform_scale;
    
    double scale;
    if (glyph_width_msdf <= available_size && glyph_height_msdf <= available_size) {
        // 字形能放入纹理，使用统一 scale
        scale = uniform_scale;
    } else {
        // 字形太大，需要缩小以适应纹理
        // 但设置最小 scale 为 uniform_scale 的一半，避免 screenPxRange 过大
        double fit_scale = std::min(available_size / std::max(width, 1.0),
                                    available_size / std::max(height, 1.0));
        scale = std::max(fit_scale, uniform_scale * 0.5);
        
        DONG_LOG_DEBUG("[MSDF] glyph=%u WARNING: glyph too large (%.1fx%.1f), using fit_scale=%.4f",
                glyph_id, width, height, scale);
    }

    DONG_LOG_DEBUG("[MSDF] glyph=%u msdf_size=%d range=%.2f width=%.2f height=%.2f scale=%.4f (uniform=%.4f)",
            glyph_id,
            msdf_size,
            range,
            width, height,
            scale,
            uniform_scale);
    
    // 正确的 translate 计算
    // msdfgen 的 Projection 公式是: msdf_coord = scale * (shape_coord + translate)
    // 我们希望字形左下角 (bounds.l, bounds.b) 在 MSDF 纹理中位于 (range, range)
    // 所以: range = scale * (bounds.l + translate.x)
    // translate.x = range / scale - bounds.l
    msdfgen::Vector2 translate(
        range / scale - bounds.l,
        range / scale - bounds.b
    );

    DONG_LOG_DEBUG("[MSDF] glyph=%u translate: tx=%.2f ty=%.2f", glyph_id, translate.x, translate.y);

    msdfgen::generateMSDF(msdf, shape, range, scale, translate);
    // 使用官方的 distanceSignCorrection 统一 MSDF 的符号约定，使填充区域
    // 在所有字体/字重下都保持一致，避免粗体等 glyph 出现"内部/外部符号反转"。
    msdfgen::distanceSignCorrection(msdf, shape, msdfgen::Vector2(scale, scale), translate);

    // 调试：检查 MSDF 纹理中特定位置的距离值（仅在需要时启用）
    // 注意：msdfgen 使用数学坐标系，y=0 在底部

    // 保存 MSDF 元数据（字号无关，design units → MSDF 像素的缩放和偏移）
    out_metrics.msdf_scale = static_cast<float>(scale);
    out_metrics.msdf_translate_x = static_cast<float>(translate.x);
    out_metrics.msdf_translate_y = static_cast<float>(translate.y);

    // ========== MSDF 纹理输出 ==========
    //
    // 重要：不裁剪 MSDF 纹理，始终输出完整的 msdf_size x msdf_size
    //
    // 原因：
    // 1. MSDF 生成时，字形被放置在 (range, range) 位置（通过 translate）
    // 2. 如果裁剪纹理，需要同时调整 UV 坐标，这很容易出错
    // 3. 保持完整纹理可以简化 UV 映射：UV 直接对应 [0,1] 范围
    //
    // 缺点是浪费一些 Atlas 空间，但对于小字号文本来说，
    // 32x32 或 48x48 的纹理大小是可以接受的。
    //
    // 如果将来需要优化空间，可以：
    // 1. 正确计算裁剪区域（从 (0,0) 到 (width*scale + 2*range, height*scale + 2*range)）
    // 2. 在 GlyphMetrics 中存储裁剪偏移
    // 3. 在 GPU 端正确计算 UV 映射
    
    out_width = static_cast<uint32_t>(msdf_size);
    out_height = static_cast<uint32_t>(msdf_size);
    out_bitmap.resize(out_width * out_height * 4);

    DONG_LOG_DEBUG("[MSDF] glyph=%u output_size=(%u, %u) glyph_in_msdf: pos=(%.1f,%.1f) size=(%.1f,%.1f)",
            glyph_id, out_width, out_height,
            range, range,
            width * scale, height * scale);

    // 转换为 RGBA8 格式（RGB = MSDF 通道，A 恒为 1.0）
    // 
    // 重要：Y 轴翻转
    // - msdfgen 使用数学坐标系（Y 向上），y=0 是底部
    // - GPU 纹理使用屏幕坐标系（Y 向下），y=0 是顶部
    // - 在这里翻转 Y 轴，这样 GPU 端不需要翻转 UV
    // 
    // 翻转后的坐标映射：
    // - 输出 bitmap 的 y=0 对应 msdfgen 的 y=msdf_size-1（纹理顶部）
    // - 输出 bitmap 的 y=msdf_size-1 对应 msdfgen 的 y=0（纹理底部）
    // 
    // 这样，字形在 GPU 纹理中的位置：
    // - 字形顶部在 GPU y = msdf_size - 1 - (range + glyph_height_msdf) + 1 = msdf_size - range - glyph_height_msdf
    // - 简化：字形顶部在 GPU y = range（因为 msdf_size - range - glyph_height_msdf ≈ range 对于典型字形）
    // - 实际上，字形顶部在 GPU y = msdf_size - (range + glyph_height_msdf)
    //
    // 更准确的分析：
    // - msdfgen 中，字形底部在 y = range，字形顶部在 y = range + glyph_height_msdf
    // - 翻转后，字形底部在 GPU y = msdf_size - 1 - range
    // - 翻转后，字形顶部在 GPU y = msdf_size - 1 - (range + glyph_height_msdf)
    //
    // 但这对于渲染位置计算来说太复杂了。
    // 更简单的方法：保持 msdfgen 的坐标系，在 GPU 端翻转 UV。
    // 
    // 实际上，当前的 UV 翻转方案是正确的，问题在于位置计算。
    // 让我们保持当前的 Y 轴方向，但修正位置计算。
    
    // DEBUG: 检查 MSDF 数据是否正确生成
    float min_r = 1.0f, max_r = 0.0f;
    float min_g = 1.0f, max_g = 0.0f;
    float min_b = 1.0f, max_b = 0.0f;
    int non_zero_count = 0;
    
    for (int y = 0; y < msdf_size; ++y) {
        // Y 轴翻转：msdfgen 使用数学坐标系（Y 向上），GPU 纹理使用屏幕坐标系（Y 向下）
        // 从 msdfgen 的 (msdf_size - 1 - y) 行读取，写入输出的 y 行
        const int src_y = msdf_size - 1 - y;
        for (int x = 0; x < msdf_size; ++x) {
            const float* pixel = msdf(x, src_y);

            // msdfgen 输出的是"有符号距离"（以像素为单位的距离范围由 `range` 控制），
            // 需要按官方编码方式映射到 [0,1]：encoded = distance / range + 0.5。
            const float inv_range = static_cast<float>(1.0 / range);
            float r = std::clamp(pixel[0] * inv_range + 0.5f, 0.0f, 1.0f);
            float g = std::clamp(pixel[1] * inv_range + 0.5f, 0.0f, 1.0f);
            float b = std::clamp(pixel[2] * inv_range + 0.5f, 0.0f, 1.0f);

            min_r = std::min(min_r, r);
            max_r = std::max(max_r, r);
            min_g = std::min(min_g, g);
            max_g = std::max(max_g, g);
            min_b = std::min(min_b, b);
            max_b = std::max(max_b, b);

            if (r > 0.01f || g > 0.01f || b > 0.01f) {
                non_zero_count++;
            }

            int idx = (y * msdf_size + x) * 4;
            out_bitmap[idx + 0] = static_cast<uint8_t>(r * 255.0f);
            out_bitmap[idx + 1] = static_cast<uint8_t>(g * 255.0f);
            out_bitmap[idx + 2] = static_cast<uint8_t>(b * 255.0f);
            out_bitmap[idx + 3] = 255;
        }
    }

    
    DONG_LOG_DEBUG("[MSDF DEBUG] glyph=%u msdf_size=%d non_zero_pixels=%d (%.1f%%) r=[%.3f,%.3f] g=[%.3f,%.3f] b=[%.3f,%.3f]",
            glyph_id, msdf_size, non_zero_count, 
            100.0f * non_zero_count / (msdf_size * msdf_size),
            min_r, max_r, min_g, max_g, min_b, max_b);

    return true;
}

} // namespace dong::render
