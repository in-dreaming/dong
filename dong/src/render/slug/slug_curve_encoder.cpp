#include "slug_curve_encoder.hpp"
#include <cmath>

namespace dong::render::slug {

namespace {

bool pointsEqual(const Point2D& a, const Point2D& b) {
    constexpr float eps = 1e-6f;
    return std::fabs(a.x - b.x) < eps && std::fabs(a.y - b.y) < eps;
}

CurveTexel makeTexel(float a, float b, float c, float d) {
    return CurveTexel{{a, b, c, d}};
}

} // namespace

CurveEncodeResult encodeCurves(const std::vector<QuadraticBezier>& curves) {
    CurveEncodeResult result;
    if (curves.empty()) return result;

    result.texels.reserve(curves.size() * 2);
    result.curve_texel_offsets.reserve(curves.size());

    for (size_t i = 0; i < curves.size(); i++) {
        const auto& c = curves[i];
        bool can_share = (i > 0) && pointsEqual(curves[i - 1].p2, c.p0);

        if (can_share) {
            // The previous curve's p2 texel already has (p2.x, p2.y, 0, 0).
            // Overwrite it with (p0.x, p0.y, p1.x, p1.y) = (prev.p2.x, prev.p2.y, c.p1.x, c.p1.y)
            // But since p0 == prev.p2, we just need to update the last texel
            uint32_t shared_idx = static_cast<uint32_t>(result.texels.size() - 1);
            result.texels[shared_idx] = makeTexel(c.p0.x, c.p0.y, c.p1.x, c.p1.y);
            result.curve_texel_offsets.push_back(shared_idx);
            result.texels.push_back(makeTexel(c.p2.x, c.p2.y, 0.0f, 0.0f));
        } else {
            uint32_t offset = static_cast<uint32_t>(result.texels.size());
            result.curve_texel_offsets.push_back(offset);
            result.texels.push_back(makeTexel(c.p0.x, c.p0.y, c.p1.x, c.p1.y));
            result.texels.push_back(makeTexel(c.p2.x, c.p2.y, 0.0f, 0.0f));
        }
    }

    result.texels_used = static_cast<uint32_t>(result.texels.size());
    return result;
}

} // namespace dong::render::slug
