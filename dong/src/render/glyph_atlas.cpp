#include "glyph_atlas.hpp"
#include "gpu_device.hpp"
#include "font_metrics.hpp"
#include <SDL3/SDL_log.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

// MSDF 生成库（仅核心）
#include <msdfgen/msdfgen.h>
#include <msdfgen/core/edge-coloring.h>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

namespace dong::render {

// FreeType outline 转换为 msdfgen::Shape 的辅助类
class FTContourConverter {
public:
    explicit FTContourConverter(msdfgen::Shape* shape) : shape_(shape) {}

    static int moveTo(const FT_Vector* to, void* user) {
        auto* self = static_cast<FTContourConverter*>(user);
        if (self->current_contour_) {
            self->shape_->contours.push_back(*self->current_contour_);
        }
        self->current_contour_ = std::make_unique<msdfgen::Contour>();
        // FT_LOAD_NO_SCALE 返回的是 design units，不需要除以 64
        self->last_point_ = msdfgen::Point2(to->x, to->y);
        return 0;
    }

    static int lineTo(const FT_Vector* to, void* user) {
        auto* self = static_cast<FTContourConverter*>(user);
        if (!self->current_contour_) return 1;
        
        msdfgen::Point2 p(to->x, to->y);
        self->current_contour_->addEdge(msdfgen::EdgeHolder(
            self->last_point_, p
        ));
        self->last_point_ = p;
        return 0;
    }

    static int conicTo(const FT_Vector* control, const FT_Vector* to, void* user) {
        auto* self = static_cast<FTContourConverter*>(user);
        if (!self->current_contour_) return 1;
        
        msdfgen::Point2 c(control->x, control->y);
        msdfgen::Point2 p(to->x, to->y);
        self->current_contour_->addEdge(msdfgen::EdgeHolder(
            self->last_point_, c, p
        ));
        self->last_point_ = p;
        return 0;
    }

    static int cubicTo(const FT_Vector* c1, const FT_Vector* c2, const FT_Vector* to, void* user) {
        auto* self = static_cast<FTContourConverter*>(user);
        if (!self->current_contour_) return 1;
        
        msdfgen::Point2 ctrl1(c1->x, c1->y);
        msdfgen::Point2 ctrl2(c2->x, c2->y);
        msdfgen::Point2 p(to->x, to->y);
        self->current_contour_->addEdge(msdfgen::EdgeHolder(
            self->last_point_, ctrl1, ctrl2, p
        ));
        self->last_point_ = p;
        return 0;
    }

    void finalize() {
        if (current_contour_) {
            shape_->contours.push_back(*current_contour_);
            current_contour_.reset();
        }
    }

private:
    msdfgen::Shape* shape_;
    std::unique_ptr<msdfgen::Contour> current_contour_;
    msdfgen::Point2 last_point_;
};

GlyphAtlas::GlyphAtlas(GPUDevice* gpu_device)
    : gpu_device_(gpu_device) {}

GlyphAtlas::~GlyphAtlas() {
    if (!gpu_device_ || !gpu_device_->isInitialized()) {
        return;
    }

    SDL_GPUDevice* dev = gpu_device_->getHandle();
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
    SDL_Log("GlyphAtlas: created page %u (%u x %u)", page.page_index, atlas_width_, atlas_height_);
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

    victim->width = atlas_width_;
    victim->height = atlas_height_;
    victim->cursor_x = 0;
    victim->cursor_y = 0;
    victim->row_height = 0;
    victim->glyph_count = 0;
    victim->last_used = 0;

    SDL_Log("GlyphAtlas: evicted and recycled page %u", victim->page_index);
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

    constexpr uint32_t kAtlasPadding = 1;

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
        SDL_Log("GlyphAtlas::initialize: failed to create initial page");
        return false;
    }

    SDL_Log("GlyphAtlas initialized: %u x %u, max_pages=%u", atlas_width_, atlas_height_, max_pages_);
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

    constexpr uint32_t kAtlasPadding = 1;

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
    for (uint32_t row = 0; row < glyph_height; ++row) {
        const uint32_t src_row = glyph_height - 1 - row;
        const uint8_t* src_ptr = bitmap.data() + src_row * stride;
        std::memcpy(dst_bytes + row * stride, src_ptr, stride);
    }
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

    gpu_device_->submitCommandBuffer(cmd_buf);
    gpu_device_->waitForGPU();
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

    SDL_Log("GlyphAtlas: added glyph %u (%s) at page %u (%u, %u), size (%u, %u)",
            glyph_id, font_path.c_str(), page->page_index, dst_x, dst_y, glyph_width, glyph_height);

    return &cache_[key];
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

    // 获取度量信息（design units，无需除以 64）
    out_metrics.advance_x_units = static_cast<float>(face->glyph->advance.x);
    out_metrics.bearing_x_units = static_cast<float>(face->glyph->metrics.horiBearingX);
    out_metrics.bearing_y_units = static_cast<float>(face->glyph->metrics.horiBearingY);
    out_metrics.width_units = static_cast<float>(face->glyph->metrics.width);
    out_metrics.height_units = static_cast<float>(face->glyph->metrics.height);

    // 如果是空字形（如空格），直接返回
    if (face->glyph->outline.n_points == 0) {
        out_width = 0;
        out_height = 0;
        return true;
    }

    // 将 FreeType outline 转换为 msdfgen::Shape
    msdfgen::Shape shape;
    FTContourConverter converter(&shape);
    
    FT_Outline_Funcs callbacks{};
    callbacks.move_to = &FTContourConverter::moveTo;
    callbacks.line_to = &FTContourConverter::lineTo;
    callbacks.conic_to = &FTContourConverter::conicTo;
    callbacks.cubic_to = &FTContourConverter::cubicTo;
    callbacks.shift = 0;
    callbacks.delta = 0;

    FT_Outline_Decompose(&face->glyph->outline, &callbacks, &converter);
    converter.finalize();

    if (shape.contours.empty()) {
        SDL_Log("GlyphAtlas::generateMSDF: empty contours for glyph %u", glyph_id);
        out_width = 0;
        out_height = 0;
        return true;
    }

    // 规范化并着色边缘
    shape.normalize();
    msdfgen::edgeColoringSimple(shape, 3.0);

    // 计算边界（design units），仅用于确定 MSDF 投影区域
    // 注意：真正用于排版/基线对齐的字形度量（bearing/width/height）
    // 已经从 FreeType/OS2 提取，保存在 out_metrics 中，后续不再被 bounds 覆盖。
    // 这样 GlyphAtlas / TextShaper / Painter 全部共享同一套设计单位度量，
    // 与浏览器的基线和行高定义保持一致，避免每个字形因为局部 bounds 差异产生“参差不齐”的视觉错位。
    msdfgen::Shape::Bounds bounds = shape.getBounds();

    // 生成 MSDF（固定尺寸，字号无关）
    const int msdf_size = static_cast<int>(glyph_bitmap_size_);
    const double range = static_cast<double>(glyph_distance_range_);
    
    msdfgen::Bitmap<float, 3> msdf(msdf_size, msdf_size);
    
    double width = bounds.r - bounds.l;
    double height = bounds.t - bounds.b;
    double safe_width = std::max(width, 1.0);
    double safe_height = std::max(height, 1.0);
    
    // 计算 scale：使字形 + padding 能完全放入 MSDF 纹理
    // 字形在 MSDF 纹理中占用 (msdf_size - 2*range) 像素
    double scale = std::min((msdf_size - range * 2) / safe_width,
                            (msdf_size - range * 2) / safe_height);
    
    // translate 的计算：
    // msdfgen 的 Projection 是 project(coord) = coord * scale + translate
    // 我们希望字形左下角在 MSDF 纹理中位于 (range, range)
    // 字形左下角在 shape 坐标系中是 (bounds.l, bounds.b)
    // 所以 bounds.l * scale + translate.x = range
    // translate.x = range - bounds.l * scale
    msdfgen::Vector2 translate(
        range - bounds.l * scale,
        range - bounds.b * scale
    );

    msdfgen::generateMSDF(msdf, shape, range, scale, translate);

    // 保存 MSDF 元数据（字号无关，design units → MSDF 像素的缩放和偏移）
    out_metrics.msdf_scale = static_cast<float>(scale);
    out_metrics.msdf_translate_x = static_cast<float>(translate.x);
    out_metrics.msdf_translate_y = static_cast<float>(translate.y);

    // 转换为 RGBA8 格式（RGB = MSDF 通道，A 恒为 1.0）
    out_width = msdf_size;
    out_height = msdf_size;
    out_bitmap.resize(out_width * out_height * 4);

    for (int y = 0; y < msdf_size; ++y) {
        for (int x = 0; x < msdf_size; ++x) {
            const float* pixel = msdf(x, y);
            float r = std::clamp(pixel[0], 0.0f, 1.0f);
            float g = std::clamp(pixel[1], 0.0f, 1.0f);
            float b = std::clamp(pixel[2], 0.0f, 1.0f);

            int idx = (y * msdf_size + x) * 4;
            out_bitmap[idx + 0] = static_cast<uint8_t>(r * 255.0f);
            out_bitmap[idx + 1] = static_cast<uint8_t>(g * 255.0f);
            out_bitmap[idx + 2] = static_cast<uint8_t>(b * 255.0f);
            out_bitmap[idx + 3] = 255;
        }
    }

    return true;
}

} // namespace dong::render
