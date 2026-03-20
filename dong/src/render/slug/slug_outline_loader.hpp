#pragma once

#include "slug_types.hpp"
#include <string>
#include <vector>

struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;

namespace dong::render::slug {

// Extract quadratic Bezier curves from a FreeType glyph outline.
// All coordinates are normalized to em-space (divided by units_per_em).
// Cubic curves (CFF/OTF) are approximated as quadratics.
//
// Returns empty vector if the glyph has no outline (e.g. space).
std::vector<QuadraticBezier> loadGlyphOutline(FT_Face face, uint32_t glyph_id);

} // namespace dong::render::slug
