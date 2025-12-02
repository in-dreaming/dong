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
    if (atlas_texture_ && gpu_device_ && gpu_device_->isInitialized()) {
        SDL_ReleaseGPUTexture(gpu_device_->getHandle(), atlas_texture_);
        atlas_texture_ = nullptr;
    }
}

std::string GlyphAtlas::makeGlyphKey(uint32_t glyph_id, const std::string& font_path) const {
    std::string key = font_path;
    key.push_back('#');
    key.append(std::to_string(glyph_id));
    return key;
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
    cursor_x_ = 0;
    cursor_y_ = 0;
    row_height_ = 0;
    cache_.clear();

    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.width = atlas_width_;
    tex_info.height = atlas_height_;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    atlas_texture_ = SDL_CreateGPUTexture(gpu_device_->getHandle(), &tex_info);
    if (!atlas_texture_) {
        SDL_Log("GlyphAtlas::initialize: failed to create atlas texture: %s", SDL_GetError());
        return false;
    }

    SDL_Log("GlyphAtlas initialized: %u x %u", atlas_width_, atlas_height_);
    return true;
}

const AtlasEntry* GlyphAtlas::getGlyph(uint32_t glyph_id, const std::string& font_path) {
    if (font_path.empty()) {
        return nullptr;
    }
    auto it = cache_.find(makeGlyphKey(glyph_id, font_path));
    if (it != cache_.end()) {
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
        cache_[key] = entry;
        return &cache_[key];
    }

    // 简单行优先装箱，加入统一的 atlas padding，避免 MSDF 线性采样时跨到相邻字形
    constexpr uint32_t kAtlasPadding = 1;

    if (cursor_x_ + glyph_width + kAtlasPadding > atlas_width_) {
        cursor_x_ = 0;
        cursor_y_ += row_height_ + kAtlasPadding;
        row_height_ = 0;
    }

    if (cursor_y_ + glyph_height > atlas_height_) {
        SDL_Log("GlyphAtlas::addGlyph: atlas is full");
        return nullptr;
    }

    uint32_t dst_x = cursor_x_;
    uint32_t dst_y = cursor_y_;
    cursor_x_ += glyph_width + kAtlasPadding;
    row_height_ = std::max(row_height_, glyph_height);

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
    region.texture = atlas_texture_;
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
    entry.atlas_page = 0;
    entry.u0 = static_cast<float>(dst_x) / static_cast<float>(atlas_width_);
    entry.v0 = static_cast<float>(dst_y) / static_cast<float>(atlas_height_);
    entry.u1 = static_cast<float>(dst_x + glyph_width) / static_cast<float>(atlas_width_);
    entry.v1 = static_cast<float>(dst_y + glyph_height) / static_cast<float>(atlas_height_);
    entry.metrics = metrics;

    cache_[key] = entry;
    SDL_Log("GlyphAtlas: added glyph %u (%s) at (%u, %u), size (%u, %u)",
            glyph_id, font_path.c_str(), dst_x, dst_y, glyph_width, glyph_height);

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

    // 计算边界（design units）
    msdfgen::Shape::Bounds bounds = shape.getBounds();

    // 使用 msdfgen 的 bounds 统一几何度量（与 MSDF 纹理坐标系对齐）
    out_metrics.bearing_x_units = static_cast<float>(bounds.l);
    out_metrics.bearing_y_units = static_cast<float>(bounds.t);
    out_metrics.width_units = static_cast<float>(bounds.r - bounds.l);
    out_metrics.height_units = static_cast<float>(bounds.t - bounds.b);

    // 生成 MSDF（固定尺寸，字号无关）
    const int msdf_size = static_cast<int>(glyph_bitmap_size_);
    const double range = static_cast<double>(glyph_distance_range_);
    
    msdfgen::Bitmap<float, 3> msdf(msdf_size, msdf_size);
    
    double width = bounds.r - bounds.l;
    double height = bounds.t - bounds.b;
    double safe_width = std::max(width, 1.0);
    double safe_height = std::max(height, 1.0);
    double scale = std::min((msdf_size - range * 2) / safe_width,
                            (msdf_size - range * 2) / safe_height);
    
    // translate的作用：将字形边界框的左下角移到原点，然后再平移使其居中
    // 关键：translate 操作在缩放之前，单位是 design units
    // 最简单的方式：将字形移到原点，让 msdfgen 自然居中
    msdfgen::Vector2 translate(
        -bounds.l,
        -bounds.b
    );

    SDL_Log("MSDF Gen: glyph=%u bounds=(%.1f,%.1f)-(%.1f,%.1f) size=%d range=%.1f scale=%.3f translate=(%.1f,%.1f)",
            glyph_id, bounds.l, bounds.b, bounds.r, bounds.t, msdf_size, range, scale, translate.x, translate.y);

    msdfgen::generateMSDF(msdf, shape, range, scale, translate);

    // 保存 MSDF 元数据（字号无关）
    out_metrics.msdf_scale = static_cast<float>(scale);
    out_metrics.msdf_translate_x = static_cast<float>(translate.x);
    out_metrics.msdf_translate_y = static_cast<float>(translate.y);

    // 转换为 RGBA8 格式，并做简单的数值统计 + 调试导出
    out_width = msdf_size;
    out_height = msdf_size;
    out_bitmap.resize(out_width * out_height * 4);

    float min_r = 1.0f;
    float max_r = 0.0f;
    float sum_r = 0.0f;
    int inside_count = 0;
    int outside_count = 0;

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

            if (r < min_r) min_r = r;
            if (r > max_r) max_r = r;
            sum_r += r;
            
            if (r > 0.5f) inside_count++;
            else outside_count++;
        }
    }

    float avg_r = sum_r / (msdf_size * msdf_size);
    SDL_Log("MSDF Data: glyph=%u R[min=%.3f avg=%.3f max=%.3f] inside=%d outside=%d total=%d",
            glyph_id, min_r, avg_r, max_r, inside_count, outside_count, msdf_size * msdf_size);

    // 调试：导出前几个字形的MSDF到文件
    static int debug_glyph_count = 0;
    if (debug_glyph_count < 3) {
        char filename[256];
        snprintf(filename, sizeof(filename), "msdf_debug_glyph_%u.bmp", glyph_id);
        
        FILE* f = fopen(filename, "wb");
        if (f) {
            // BMP header
            uint32_t filesize = 54 + msdf_size * msdf_size * 3;
            uint8_t header[54] = {0};
            header[0] = 'B'; header[1] = 'M';
            memcpy(header + 2, &filesize, 4);
            header[10] = 54;
            header[14] = 40;
            memcpy(header + 18, &msdf_size, 4);
            memcpy(header + 22, &msdf_size, 4);
            *(uint16_t*)(header + 26) = 1;
            *(uint16_t*)(header + 28) = 24;
            fwrite(header, 1, 54, f);
            
            // Write pixels (BMP is bottom-up)
            for (int y = msdf_size - 1; y >= 0; --y) {
                for (int x = 0; x < msdf_size; ++x) {
                    int idx = (y * msdf_size + x) * 4;
                    uint8_t bgr[3] = {out_bitmap[idx + 2], out_bitmap[idx + 1], out_bitmap[idx + 0]};
                    fwrite(bgr, 3, 1, f);
                }
            }
            fclose(f);
            SDL_Log("MSDF Debug: Exported %s", filename);
        }
        ++debug_glyph_count;
    }

    return true;
}

} // namespace dong::render
