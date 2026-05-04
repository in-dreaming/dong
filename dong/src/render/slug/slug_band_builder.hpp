#pragma once

#include "slug_types.hpp"
#include "slug_curve_encoder.hpp"
#include <vector>

namespace dong::render::slug {

// Result of building band data for one glyph.
struct BandBuildResult {
    std::vector<BandTexel> texels;
    uint32_t texels_used = 0;
    uint16_t hband_count = 0;   // horizontal bands (Y direction)
    uint16_t vband_count = 0;   // vertical bands (X direction)
    float band_scale_x = 0.0f;
    float band_scale_y = 0.0f;
};

// Build horizontal and vertical band index data for a glyph.
// curves: the glyph's quadratic Bezier curves
// curve_encode: the encoded curve texel data (for texel locations)
// bbox: the glyph's bounding box in em-space
// curve_tex_base_x/y: where this glyph's curves start in the curve texture
BandBuildResult buildBands(const std::vector<QuadraticBezier>& curves,
                           const CurveEncodeResult& curve_encode,
                           const BBox& bbox,
                           uint32_t curve_tex_base_x,
                           uint32_t curve_tex_base_y);

} // namespace dong::render::slug
