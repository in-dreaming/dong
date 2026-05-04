#pragma once

#include "slug_types.hpp"
#include <vector>

namespace dong::render::slug {

// Result of encoding curves into texel data.
struct CurveEncodeResult {
    std::vector<CurveTexel> texels;     // 2 texels per curve (may share endpoints)
    uint32_t texels_used = 0;
    // Per-curve location in the texel array (index into texels[])
    std::vector<uint32_t> curve_texel_offsets;
};

// Encode a glyph's curves into CurveTexel array ready for GPU upload.
// Each curve occupies 2 texels: (p0.x, p0.y, p1.x, p1.y) and (p2.x, p2.y, 0, 0).
// Consecutive curves sharing endpoints reuse the last texel.
CurveEncodeResult encodeCurves(const std::vector<QuadraticBezier>& curves);

} // namespace dong::render::slug
