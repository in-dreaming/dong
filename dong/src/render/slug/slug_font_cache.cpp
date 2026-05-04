#include "slug_font_cache.hpp"
#include "slug_outline_loader.hpp"
#include "../../core/log.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace dong::render::slug {

namespace {

BBox computeGlyphBBox(const std::vector<QuadraticBezier>& curves) {
    BBox bbox;
    for (const auto& c : curves) {
        BBox cb = computeCurveBBox(c);
        bbox.expand(cb.min_x, cb.min_y);
        bbox.expand(cb.max_x, cb.max_y);
    }
    return bbox;
}

} // namespace

FT_Face SlugFontCache::getOrOpenFace(const std::string& font_path) {
    auto it = face_cache_.find(font_path);
    if (it != face_cache_.end()) return it->second.face;

    FT_Library lib = nullptr;
    FT_Error err = FT_Init_FreeType(&lib);
    if (err) {
        DONG_LOG_ERROR("[SlugFontCache] FT_Init_FreeType failed: %d", err);
        return nullptr;
    }

    FT_Face face = nullptr;
    err = FT_New_Face(lib, font_path.c_str(), 0, &face);
    if (err) {
        DONG_LOG_WARN("[SlugFontCache] FT_New_Face failed for '%s': %d",
                      font_path.c_str(), err);
        FT_Done_FreeType(lib);
        return nullptr;
    }

    FaceEntry entry;
    entry.face = face;
    entry.ft_library = lib;
    face_cache_[font_path] = entry;
    return face;
}

bool SlugFontCache::allocateCurveSpace(uint32_t texels_needed,
                                        uint32_t& out_x, uint32_t& out_y) {
    if (curve_write_x_ + texels_needed > kMaxCurveTextureWidth) {
        curve_write_x_ = 0;
        curve_write_y_++;
    }

    if (curve_write_y_ >= kMaxCurveTextureHeight) {
        DONG_LOG_ERROR("[SlugFontCache] Curve texture full (%u rows)", kMaxCurveTextureHeight);
        return false;
    }

    out_x = curve_write_x_;
    out_y = curve_write_y_;

    uint32_t row_end = (curve_write_y_ + 1) * kMaxCurveTextureWidth;
    if (curve_texture_.size() < row_end) {
        curve_texture_.resize(row_end, CurveTexel{{0, 0, 0, 0}});
    }

    curve_write_x_ += texels_needed;
    return true;
}

bool SlugFontCache::allocateBandSpace(uint32_t texels_needed,
                                       uint32_t& out_x, uint32_t& out_y) {
    if (band_write_x_ + texels_needed > kMaxBandTextureWidth) {
        band_write_x_ = 0;
        band_write_y_++;
    }

    if (band_write_y_ >= kMaxBandTextureHeight) {
        DONG_LOG_ERROR("[SlugFontCache] Band texture full (%u rows)", kMaxBandTextureHeight);
        return false;
    }

    out_x = band_write_x_;
    out_y = band_write_y_;

    uint32_t row_end = (band_write_y_ + 1) * kMaxBandTextureWidth;
    if (band_texture_.size() < row_end) {
        band_texture_.resize(row_end, BandTexel{{0, 0}});
    }

    band_write_x_ += texels_needed;
    return true;
}

void SlugFontCache::writeCurveTexels(const CurveEncodeResult& encode,
                                      uint32_t base_x, uint32_t base_y) {
    for (uint32_t i = 0; i < encode.texels_used; i++) {
        uint32_t x = base_x + i;
        uint32_t y = base_y;
        // Handle row wrapping
        y += x / kMaxCurveTextureWidth;
        x = x % kMaxCurveTextureWidth;

        uint32_t idx = y * kMaxCurveTextureWidth + x;
        if (idx < curve_texture_.size()) {
            curve_texture_[idx] = encode.texels[i];
        }
    }
}

void SlugFontCache::writeBandTexels(const BandBuildResult& bands,
                                     uint32_t base_x, uint32_t base_y) {
    for (uint32_t i = 0; i < bands.texels_used; i++) {
        uint32_t x = base_x + i;
        uint32_t y = base_y;
        y += x / kMaxBandTextureWidth;
        x = x % kMaxBandTextureWidth;

        uint32_t idx = y * kMaxBandTextureWidth + x;
        if (idx < band_texture_.size()) {
            band_texture_[idx] = bands.texels[i];
        }
    }
}

const PreparedGlyph* SlugFontCache::prepareGlyph(const std::string& font_path,
                                                   uint32_t glyph_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    SlugCacheKey key{font_path, glyph_id};
    auto it = glyph_cache_.find(key);
    if (it != glyph_cache_.end()) {
        return it->second.glyph_data.valid ? &it->second : nullptr;
    }

    FT_Face face = getOrOpenFace(font_path);
    if (!face) {
        glyph_cache_[key] = PreparedGlyph{};
        return nullptr;
    }

    auto curves = loadGlyphOutline(face, glyph_id);
    if (curves.empty()) {
        glyph_cache_[key] = PreparedGlyph{};
        return nullptr;
    }

    BBox bbox = computeGlyphBBox(curves);
    if (!bbox.valid()) {
        glyph_cache_[key] = PreparedGlyph{};
        return nullptr;
    }

    CurveEncodeResult curve_encode = encodeCurves(curves);

    uint32_t curve_base_x = 0, curve_base_y = 0;
    if (!allocateCurveSpace(curve_encode.texels_used, curve_base_x, curve_base_y)) {
        glyph_cache_[key] = PreparedGlyph{};
        return nullptr;
    }

    writeCurveTexels(curve_encode, curve_base_x, curve_base_y);

    BandBuildResult band_result = buildBands(curves, curve_encode, bbox,
                                              curve_base_x, curve_base_y);

    uint32_t band_base_x = 0, band_base_y = 0;
    if (!allocateBandSpace(band_result.texels_used, band_base_x, band_base_y)) {
        glyph_cache_[key] = PreparedGlyph{};
        return nullptr;
    }

    writeBandTexels(band_result, band_base_x, band_base_y);

    PreparedGlyph prepared;
    prepared.glyph_data.bbox = bbox;
    prepared.glyph_data.band_loc_x = static_cast<uint16_t>(band_base_x);
    prepared.glyph_data.band_loc_y = static_cast<uint16_t>(band_base_y);
    prepared.glyph_data.hband_count = band_result.hband_count;
    prepared.glyph_data.vband_count = band_result.vband_count;
    prepared.glyph_data.band_scale_x = band_result.band_scale_x;
    prepared.glyph_data.band_scale_y = band_result.band_scale_y;
    prepared.glyph_data.curve_count = static_cast<uint32_t>(curves.size());
    prepared.glyph_data.valid = true;
    prepared.curve_data = std::move(curve_encode);
    prepared.band_data = std::move(band_result);

    curve_dirty_ = true;
    band_dirty_ = true;

    auto [inserted_it, _] = glyph_cache_.emplace(key, std::move(prepared));
    return &inserted_it->second;
}

uint32_t SlugFontCache::prepareGlyphBatch(const std::string& font_path,
                                            const uint32_t* glyph_ids,
                                            uint32_t count) {
    uint32_t prepared = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (prepareGlyph(font_path, glyph_ids[i])) {
            prepared++;
        }
    }
    return prepared;
}

bool SlugFontCache::hasGlyph(const std::string& font_path, uint32_t glyph_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    SlugCacheKey key{font_path, glyph_id};
    auto it = glyph_cache_.find(key);
    return it != glyph_cache_.end() && it->second.glyph_data.valid;
}

const PreparedGlyph* SlugFontCache::getGlyph(const std::string& font_path,
                                               uint32_t glyph_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    SlugCacheKey key{font_path, glyph_id};
    auto it = glyph_cache_.find(key);
    if (it != glyph_cache_.end() && it->second.glyph_data.valid) {
        return &it->second;
    }
    return nullptr;
}

uint32_t SlugFontCache::getCurveTextureHeight() const {
    return curve_write_y_ + (curve_write_x_ > 0 ? 1 : 0);
}

uint32_t SlugFontCache::getBandTextureHeight() const {
    return band_write_y_ + (band_write_x_ > 0 ? 1 : 0);
}

void SlugFontCache::clearDirtyFlags() {
    curve_dirty_ = false;
    band_dirty_ = false;
}

uint32_t SlugFontCache::getCachedGlyphCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t count = 0;
    for (const auto& [k, v] : glyph_cache_) {
        if (v.glyph_data.valid) count++;
    }
    return count;
}

void SlugFontCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    glyph_cache_.clear();
    curve_texture_.clear();
    curve_write_x_ = 0;
    curve_write_y_ = 0;
    band_texture_.clear();
    band_write_x_ = 0;
    band_write_y_ = 0;
    curve_dirty_ = false;
    band_dirty_ = false;

    for (auto& [path, entry] : face_cache_) {
        if (entry.face) FT_Done_Face(entry.face);
        if (entry.ft_library) FT_Done_FreeType(static_cast<FT_Library>(entry.ft_library));
    }
    face_cache_.clear();
}

} // namespace dong::render::slug
