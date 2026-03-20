#pragma once

#include "slug_types.hpp"
#include "slug_font_cache.hpp"
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace dong::render::slug {

// =============================================================================
// Slug Vertex Builder
// =============================================================================
// Constructs per-glyph vertex/index buffers following the canonical Slug
// vertex buffer architecture (68 bytes per vertex, 4 vertices per glyph quad).
//
// Vertex layout (5 × float4):
//   attrib[0] = pos: (pos.x, pos.y, normal.x, normal.y)
//   attrib[1] = tex: (em.x, em.y, bandLocPacked, bandMaxPacked)
//   attrib[2] = jac: (M^-1_00, M^-1_01, M^-1_10, M^-1_11)
//   attrib[3] = bnd: (bandScale.x, bandScale.y, bandOffset.x, bandOffset.y)
//   attrib[4] = col: (r, g, b, a)
// =============================================================================

// Single vertex: 5 × float4 = 80 bytes
struct SlugVertex {
    float pos[4];  // object-space position + miter normal
    float tex[4];  // em-space coords + packed band data
    float jac[4];  // inverse Jacobian
    float bnd[4];  // band scale and offset
    float col[4];  // RGBA color
};

static_assert(sizeof(SlugVertex) == 80, "SlugVertex must be 80 bytes");

// Result of building vertex/index data for a batch of glyphs.
struct SlugMeshData {
    std::vector<SlugVertex> vertices;
    std::vector<uint16_t> indices;
    uint32_t glyph_count = 0;
};

// Per-glyph rendering parameters (passed from the execute path).
struct SlugGlyphParams {
    const PreparedGlyph* prepared = nullptr;
    float pos_x = 0.0f;      // screen-space glyph origin X
    float pos_y = 0.0f;      // screen-space glyph origin Y
    float scale = 1.0f;      // em-to-pixel scale factor
    float color_r = 0.0f;
    float color_g = 0.0f;
    float color_b = 0.0f;
    float color_a = 1.0f;
};

// Pack a uint32 as a float (bit-preserving reinterpret).
inline float packUintAsFloat(uint32_t v) {
    float f;
    std::memcpy(&f, &v, sizeof(float));
    return f;
}

// Compute miter normal for a quad corner.
// For an axis-aligned quad, the normals point outward diagonally.
// corner: 0=TL, 1=TR, 2=BL, 3=BR
inline void computeCornerNormal(int corner, float& nx, float& ny) {
    // Normalized diagonal directions for quad corners
    static const float kInvSqrt2 = 0.70710678118f;
    switch (corner) {
        case 0: nx = -kInvSqrt2; ny = -kInvSqrt2; break; // top-left
        case 1: nx =  kInvSqrt2; ny = -kInvSqrt2; break; // top-right
        case 2: nx = -kInvSqrt2; ny =  kInvSqrt2; break; // bottom-left
        case 3: nx =  kInvSqrt2; ny =  kInvSqrt2; break; // bottom-right
        default: nx = 0.0f; ny = 0.0f; break;
    }
}

// Build vertex/index data for a single glyph quad.
inline void buildGlyphQuad(const SlugGlyphParams& params,
                            SlugMeshData& mesh) {
    if (!params.prepared || !params.prepared->glyph_data.valid) return;

    const auto& gd = params.prepared->glyph_data;
    const float scale = params.scale;

    // Glyph bounding box in screen space, expanded by 1 pixel for edge coverage.
    // Without SlugDilate (disabled for 2D orthographic), we need manual padding
    // so that edge pixels have correct analytical coverage.
    //
    // FreeType y-axis points UP (math coords) but screen y-axis points DOWN.
    // So screen_y = pos_y - ft_y * scale (negate y).
    const float kPadPx = 1.0f;
    const float pad_em = kPadPx * ((scale > 1e-10f) ? (1.0f / scale) : 0.0f);

    const float x0 = params.pos_x + gd.bbox.min_x * scale - kPadPx;
    const float y0 = params.pos_y - gd.bbox.max_y * scale - kPadPx; // FT top -> screen top
    const float x1 = params.pos_x + gd.bbox.max_x * scale + kPadPx;
    const float y1 = params.pos_y - gd.bbox.min_y * scale + kPadPx; // FT bottom -> screen bottom

    // Em-space coordinates at quad corners (also padded).
    // Must stay in FreeType coord space (y-up) to match curve data.
    // Screen TL (y0) maps to em max_y (FT top), screen BL (y1) maps to em min_y (FT bottom).
    const float em_x0 = gd.bbox.min_x - pad_em;
    const float em_y_top = gd.bbox.max_y + pad_em;    // screen top -> FT top
    const float em_x1 = gd.bbox.max_x + pad_em;
    const float em_y_bottom = gd.bbox.min_y - pad_em;  // screen bottom -> FT bottom

    // Pack band location: (y << 16) | x
    const uint32_t band_loc_packed = (static_cast<uint32_t>(gd.band_loc_y) << 16u)
                                   | static_cast<uint32_t>(gd.band_loc_x);

    // Pack band max: (flags << 24) | (hband_max << 16) | (0 << 8) | vband_max
    // hband_max = hband_count - 1, vband_max = vband_count - 1
    const uint32_t hband_max = (gd.hband_count > 0) ? (gd.hband_count - 1u) : 0u;
    const uint32_t vband_max = (gd.vband_count > 0) ? (gd.vband_count - 1u) : 0u;
    const uint32_t band_max_packed = (hband_max << 16u) | vband_max;

    // Inverse Jacobian: maps object-space displacement to em-space
    // For uniform scaling: jac = (1/scale, 0, 0, 1/scale)
    const float inv_scale = (scale > 1e-10f) ? (1.0f / scale) : 0.0f;
    const float jac00 = inv_scale;
    const float jac01 = 0.0f;
    const float jac10 = 0.0f;
    const float jac11 = inv_scale;

    // Band scale and offset
    const float bbox_w = gd.bbox.width();
    const float bbox_h = gd.bbox.height();
    const float band_scale_x = gd.band_scale_x;
    const float band_scale_y = gd.band_scale_y;
    const float band_offset_x = -gd.bbox.min_x * band_scale_x;
    const float band_offset_y = -gd.bbox.min_y * band_scale_y;

    // Quad corner positions and em coords
    // Order: 0=TL, 1=TR, 2=BL, 3=BR
    const float cx[4] = { x0, x1, x0, x1 };
    const float cy[4] = { y0, y0, y1, y1 };
    const float eu[4] = { em_x0, em_x1, em_x0, em_x1 };
    const float ev[4] = { em_y_top, em_y_top, em_y_bottom, em_y_bottom };

    const uint16_t base = static_cast<uint16_t>(mesh.vertices.size());

    for (int i = 0; i < 4; i++) {
        SlugVertex vtx{};

        float nx, ny;
        computeCornerNormal(i, nx, ny);

        vtx.pos[0] = cx[i];
        vtx.pos[1] = cy[i];
        vtx.pos[2] = nx;
        vtx.pos[3] = ny;

        vtx.tex[0] = eu[i];
        vtx.tex[1] = ev[i];
        vtx.tex[2] = packUintAsFloat(band_loc_packed);
        vtx.tex[3] = packUintAsFloat(band_max_packed);

        vtx.jac[0] = jac00;
        vtx.jac[1] = jac01;
        vtx.jac[2] = jac10;
        vtx.jac[3] = jac11;

        vtx.bnd[0] = band_scale_x;
        vtx.bnd[1] = band_scale_y;
        vtx.bnd[2] = band_offset_x;
        vtx.bnd[3] = band_offset_y;

        vtx.col[0] = params.color_r;
        vtx.col[1] = params.color_g;
        vtx.col[2] = params.color_b;
        vtx.col[3] = params.color_a;

        mesh.vertices.push_back(vtx);
    }

    // Two triangles: [0,1,2], [1,3,2]
    mesh.indices.push_back(base + 0);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base + 3);
    mesh.indices.push_back(base + 2);

    mesh.glyph_count++;
}

// Build mesh data for a batch of glyphs.
inline SlugMeshData buildSlugMesh(const SlugGlyphParams* glyphs,
                                   uint32_t count) {
    SlugMeshData mesh;
    mesh.vertices.reserve(count * 4);
    mesh.indices.reserve(count * 6);

    for (uint32_t i = 0; i < count; i++) {
        buildGlyphQuad(glyphs[i], mesh);
    }
    return mesh;
}

} // namespace dong::render::slug
