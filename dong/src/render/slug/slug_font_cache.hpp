#pragma once

#include "slug_types.hpp"
#include "slug_curve_encoder.hpp"
#include "slug_band_builder.hpp"
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>

struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;

namespace dong::render::slug {

// Prepared glyph data ready for GPU upload.
struct PreparedGlyph {
    SlugGlyphData glyph_data;
    CurveEncodeResult curve_data;
    BandBuildResult band_data;
};

// Manages Slug glyph data preparation and caching.
// Thread-safe: all methods can be called from any thread.
class SlugFontCache {
public:
    SlugFontCache() = default;
    ~SlugFontCache() = default;

    // Prepare a glyph. Returns null if the glyph has no outline.
    // Results are cached by (font_path, glyph_id).
    const PreparedGlyph* prepareGlyph(const std::string& font_path,
                                       uint32_t glyph_id);

    // Batch prepare multiple glyphs. Returns number successfully prepared.
    uint32_t prepareGlyphBatch(const std::string& font_path,
                                const uint32_t* glyph_ids,
                                uint32_t count);

    // Check if a glyph is already cached.
    bool hasGlyph(const std::string& font_path, uint32_t glyph_id) const;

    // Get cached glyph data. Returns null if not prepared.
    const PreparedGlyph* getGlyph(const std::string& font_path,
                                   uint32_t glyph_id) const;

    // Get the consolidated curve texture data (all glyphs combined).
    const std::vector<CurveTexel>& getCurveTextureData() const { return curve_texture_; }
    uint32_t getCurveTextureWidth() const { return kMaxCurveTextureWidth; }
    uint32_t getCurveTextureHeight() const;

    // Get the consolidated band texture data (all glyphs combined).
    const std::vector<BandTexel>& getBandTextureData() const { return band_texture_; }
    uint32_t getBandTextureWidth() const { return kMaxBandTextureWidth; }
    uint32_t getBandTextureHeight() const;

    // Mark texture data as dirty (needs re-upload to GPU).
    bool isCurveTextureDirty() const { return curve_dirty_; }
    bool isBandTextureDirty() const { return band_dirty_; }
    void clearDirtyFlags();

    // Statistics
    uint32_t getCachedGlyphCount() const;
    uint32_t getTotalCurveTexels() const { return curve_write_x_ + curve_write_y_ * kMaxCurveTextureWidth; }
    uint32_t getTotalBandTexels() const { return band_write_x_ + band_write_y_ * kMaxBandTextureWidth; }

    // Clear all cached data.
    void clear();

private:
    // Open or retrieve a cached FreeType face for the given font path.
    FT_Face getOrOpenFace(const std::string& font_path);

    // Allocate space in curve texture and return base coordinates.
    bool allocateCurveSpace(uint32_t texels_needed,
                            uint32_t& out_x, uint32_t& out_y);

    // Allocate space in band texture and return base coordinates.
    bool allocateBandSpace(uint32_t texels_needed,
                           uint32_t& out_x, uint32_t& out_y);

    // Write encoded curve data into the consolidated curve texture.
    void writeCurveTexels(const CurveEncodeResult& encode,
                          uint32_t base_x, uint32_t base_y);

    // Write band data into the consolidated band texture.
    void writeBandTexels(const BandBuildResult& bands,
                         uint32_t base_x, uint32_t base_y);

    mutable std::mutex mutex_;

    // Glyph cache
    std::unordered_map<SlugCacheKey, PreparedGlyph, SlugCacheKeyHash> glyph_cache_;

    // Consolidated texture data (CPU side)
    std::vector<CurveTexel> curve_texture_;
    uint32_t curve_write_x_ = 0;
    uint32_t curve_write_y_ = 0;

    std::vector<BandTexel> band_texture_;
    uint32_t band_write_x_ = 0;
    uint32_t band_write_y_ = 0;

    bool curve_dirty_ = false;
    bool band_dirty_ = false;

    // FreeType face cache (font_path -> face)
    struct FaceEntry {
        FT_Face face = nullptr;
        void* ft_library = nullptr;  // Owns the library instance
    };
    std::unordered_map<std::string, FaceEntry> face_cache_;
};

} // namespace dong::render::slug
