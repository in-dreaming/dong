#include "slug_band_builder.hpp"
#include <algorithm>
#include <cmath>

namespace dong::render::slug {

namespace {

// Compute bounding box of a quadratic Bezier curve (tight, including extrema).
BBox curveBBox(const QuadraticBezier& c) {
    BBox box;
    box.expand(c.p0.x, c.p0.y);
    box.expand(c.p2.x, c.p2.y);

    // Check for extrema along each axis
    for (int axis = 0; axis < 2; axis++) {
        float a0 = (axis == 0) ? c.p0.x : c.p0.y;
        float a1 = (axis == 0) ? c.p1.x : c.p1.y;
        float a2 = (axis == 0) ? c.p2.x : c.p2.y;
        float denom = a0 - 2.0f * a1 + a2;
        if (std::fabs(denom) > 1e-10f) {
            float t = (a0 - a1) / denom;
            if (t > 0.0f && t < 1.0f) {
                float v = (1 - t) * (1 - t) * a0 + 2 * (1 - t) * t * a1 + t * t * a2;
                if (axis == 0) box.expand(v, box.min_y);
                else box.expand(box.min_x, v);
            }
        }
    }
    return box;
}

// Check if a curve's bounding box overlaps a band range along the given axis.
bool curveOverlapsBand(const BBox& cb, float band_min, float band_max, bool horizontal) {
    if (horizontal) {
        return cb.min_y < band_max && cb.max_y > band_min;
    } else {
        return cb.min_x < band_max && cb.max_x > band_min;
    }
}

// Find optimal band count for one axis.
// Returns the count that minimizes the maximum number of curves per band.
uint16_t findOptimalBandCount(const std::vector<BBox>& curve_boxes,
                              float axis_min, float axis_max,
                              bool horizontal) {
    float range = axis_max - axis_min;
    if (range <= 0.0f) return 1;

    uint16_t best_count = 1;
    uint32_t best_max_curves = UINT32_MAX;

    for (uint16_t k = 1; k <= kMaxBandCount; k++) {
        // Step size optimization: k < 8 step 1, 8-15 step 2, >=16 step 4
        if (k >= 16 && (k % 4) != 0) continue;
        if (k >= 8 && k < 16 && (k % 2) != 0) continue;

        float band_size = range / static_cast<float>(k);
        uint32_t max_in_band = 0;

        for (uint16_t b = 0; b < k; b++) {
            float bmin = axis_min + band_size * b;
            float bmax = (b == k - 1) ? axis_max : bmin + band_size;
            uint32_t count = 0;
            for (const auto& cb : curve_boxes) {
                if (curveOverlapsBand(cb, bmin, bmax, horizontal)) {
                    count++;
                }
            }
            if (count > max_in_band) max_in_band = count;
        }

        if (max_in_band < best_max_curves) {
            best_max_curves = max_in_band;
            best_count = k;
        }
    }

    return best_count;
}

// Build curve index lists for bands.
struct BandCurveList {
    uint32_t curve_count = 0;
    std::vector<uint32_t> curve_indices;
};

std::vector<BandCurveList> buildBandCurveLists(
    const std::vector<BBox>& curve_boxes,
    uint16_t band_count,
    float axis_min, float axis_max,
    bool horizontal) {
    std::vector<BandCurveList> bands(band_count);
    float range = axis_max - axis_min;
    float band_size = range / static_cast<float>(band_count);

    for (uint16_t b = 0; b < band_count; b++) {
        float bmin = axis_min + band_size * b;
        float bmax = (b == band_count - 1) ? axis_max : bmin + band_size;

        for (uint32_t ci = 0; ci < curve_boxes.size(); ci++) {
            if (curveOverlapsBand(curve_boxes[ci], bmin, bmax, horizontal)) {
                bands[b].curve_indices.push_back(ci);
            }
        }
        bands[b].curve_count = static_cast<uint32_t>(bands[b].curve_indices.size());
    }

    return bands;
}

// Sort curves within a band for early-exit optimization.
// Horizontal bands: sort by descending max.x
// Vertical bands: sort by descending max.y
void sortBandCurves(std::vector<BandCurveList>& bands,
                    const std::vector<BBox>& curve_boxes,
                    bool horizontal) {
    for (auto& band : bands) {
        std::sort(band.curve_indices.begin(), band.curve_indices.end(),
                  [&](uint32_t a, uint32_t b) {
            if (horizontal) {
                return curve_boxes[a].max_x > curve_boxes[b].max_x;
            } else {
                return curve_boxes[a].max_y > curve_boxes[b].max_y;
            }
        });
    }
}

} // namespace

BBox computeCurveBBox(const QuadraticBezier& curve) {
    return curveBBox(curve);
}

BandBuildResult buildBands(const std::vector<QuadraticBezier>& curves,
                           const CurveEncodeResult& curve_encode,
                           const BBox& bbox,
                           uint32_t curve_tex_base_x,
                           uint32_t curve_tex_base_y) {
    BandBuildResult result;
    if (curves.empty() || !bbox.valid()) return result;

    // Compute per-curve bounding boxes
    std::vector<BBox> curve_boxes;
    curve_boxes.reserve(curves.size());
    for (const auto& c : curves) {
        curve_boxes.push_back(curveBBox(c));
    }

    // Find optimal band counts
    result.hband_count = findOptimalBandCount(curve_boxes, bbox.min_y, bbox.max_y, true);
    result.vband_count = findOptimalBandCount(curve_boxes, bbox.min_x, bbox.max_x, false);

    // Compute band scales
    float w = bbox.width();
    float h = bbox.height();
    result.band_scale_x = (w > 0.0f) ? result.vband_count / w : 0.0f;
    result.band_scale_y = (h > 0.0f) ? result.hband_count / h : 0.0f;

    // Build curve lists per band
    auto hbands = buildBandCurveLists(curve_boxes, result.hband_count,
                                       bbox.min_y, bbox.max_y, true);
    auto vbands = buildBandCurveLists(curve_boxes, result.vband_count,
                                       bbox.min_x, bbox.max_x, false);

    sortBandCurves(hbands, curve_boxes, true);
    sortBandCurves(vbands, curve_boxes, false);

    // Build band texels: [hband_headers][vband_headers][hband_data...][vband_data...]
    uint32_t header_count = result.hband_count + result.vband_count;
    uint32_t total_data = 0;
    for (const auto& b : hbands) total_data += b.curve_count;
    for (const auto& b : vbands) total_data += b.curve_count;

    result.texels.resize(header_count + total_data);

    // Write headers and data
    uint32_t data_offset = header_count;

    // Horizontal bands (header at indices 0..hband_count-1)
    for (uint16_t b = 0; b < result.hband_count; b++) {
        result.texels[b].u[0] = static_cast<uint16_t>(hbands[b].curve_count);
        result.texels[b].u[1] = static_cast<uint16_t>(data_offset);

        for (uint32_t ci : hbands[b].curve_indices) {
            uint32_t texel_offset = curve_encode.curve_texel_offsets[ci];
            // Convert local texel offset to global curve texture coordinates
            uint32_t gx = curve_tex_base_x + texel_offset;
            uint32_t gy = curve_tex_base_y;
            // Handle row wrapping
            gy += gx / kMaxCurveTextureWidth;
            gx = gx % kMaxCurveTextureWidth;

            result.texels[data_offset].u[0] = static_cast<uint16_t>(gx);
            result.texels[data_offset].u[1] = static_cast<uint16_t>(gy);
            data_offset++;
        }
    }

    // Vertical bands (header at indices hband_count..header_count-1)
    for (uint16_t b = 0; b < result.vband_count; b++) {
        uint32_t header_idx = result.hband_count + b;
        result.texels[header_idx].u[0] = static_cast<uint16_t>(vbands[b].curve_count);
        result.texels[header_idx].u[1] = static_cast<uint16_t>(data_offset);

        for (uint32_t ci : vbands[b].curve_indices) {
            uint32_t texel_offset = curve_encode.curve_texel_offsets[ci];
            uint32_t gx = curve_tex_base_x + texel_offset;
            uint32_t gy = curve_tex_base_y;
            gy += gx / kMaxCurveTextureWidth;
            gx = gx % kMaxCurveTextureWidth;

            result.texels[data_offset].u[0] = static_cast<uint16_t>(gx);
            result.texels[data_offset].u[1] = static_cast<uint16_t>(gy);
            data_offset++;
        }
    }

    result.texels_used = data_offset;
    return result;
}

} // namespace dong::render::slug
