#pragma once

#include <cstdint>
#include <cmath>
#include <vector>
#include <string>

namespace dong::render::slug {

// A 2D point in em-space (1.0 = 1 em).
struct Point2D {
    float x = 0.0f;
    float y = 0.0f;
};

// A quadratic Bezier curve with 3 control points in em-space.
struct QuadraticBezier {
    Point2D p0;  // start
    Point2D p1;  // control
    Point2D p2;  // end
};

// Axis-aligned bounding box.
struct BBox {
    float min_x = 1e30f;
    float min_y = 1e30f;
    float max_x = -1e30f;
    float max_y = -1e30f;

    bool valid() const { return min_x <= max_x && min_y <= max_y; }
    float width() const { return max_x - min_x; }
    float height() const { return max_y - min_y; }

    void expand(float x, float y) {
        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
        if (x > max_x) max_x = x;
        if (y > max_y) max_y = y;
    }
};

// Compute the tight bounding box of a quadratic Bezier curve
// (includes extrema, not just control point hull).
BBox computeCurveBBox(const QuadraticBezier& curve);

// Per-curve texel: 2 texels of 4×float16 each.
// texel0 = (p0.x, p0.y, p1.x, p1.y)
// texel1 = (p2.x, p2.y, 0, 0)
struct CurveTexel {
    float data[4];  // 4 floats (will be converted to half on upload)
};

// Band texel: 2×uint16.
// For header: u[0] = curve_count, u[1] = offset_to_curve_list
// For data:   u[0] = curve_location_x, u[1] = curve_location_y
struct BandTexel {
    uint16_t u[2];
};

// Result of building curve + band data for one glyph.
struct SlugGlyphData {
    BBox bbox;                      // em-space bounding box
    uint16_t band_loc_x = 0;       // band texture start x
    uint16_t band_loc_y = 0;       // band texture start y
    uint16_t hband_count = 0;      // horizontal band count (Y direction)
    uint16_t vband_count = 0;      // vertical band count (X direction)
    float band_scale_x = 0.0f;     // vband_count / bbox.width
    float band_scale_y = 0.0f;     // hband_count / bbox.height
    uint32_t curve_count = 0;      // total curves
    bool valid = false;
};

// Cache key for looking up a prepared glyph.
struct SlugCacheKey {
    std::string font_path;
    uint32_t glyph_id = 0;

    bool operator==(const SlugCacheKey& o) const {
        return font_path == o.font_path && glyph_id == o.glyph_id;
    }
};

struct SlugCacheKeyHash {
    size_t operator()(const SlugCacheKey& k) const {
        size_t h = std::hash<std::string>{}(k.font_path);
        h ^= std::hash<uint32_t>{}(k.glyph_id) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

// Limits
static constexpr uint32_t kMaxCurveTextureWidth = 4096;
static constexpr uint32_t kMaxCurveTextureHeight = 2048;
static constexpr uint32_t kMaxBandTextureWidth = 4096;
static constexpr uint32_t kMaxBandTextureHeight = 8192;
static constexpr uint32_t kLogBandTextureWidth = 12;
static constexpr uint32_t kMaxBandCount = 32;

} // namespace dong::render::slug
