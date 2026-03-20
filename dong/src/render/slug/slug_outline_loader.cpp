#include "slug_outline_loader.hpp"
#include "../../core/log.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

namespace dong::render::slug {

namespace {

// Convert a cubic Bezier to two quadratic approximations.
// This is the standard midpoint-splitting approach.
void cubicToQuadratics(Point2D p0, Point2D c0, Point2D c1, Point2D p1,
                       float scale,
                       std::vector<QuadraticBezier>& out) {
    // Simple midpoint approximation: split at t=0.5, approximate each half
    // as a quadratic by computing the midpoint of the cubic control segment.
    auto mid = [](Point2D a, Point2D b) -> Point2D {
        return {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
    };

    Point2D m01 = mid(p0, c0);
    Point2D m12 = mid(c0, c1);
    Point2D m23 = mid(c1, p1);
    Point2D m012 = mid(m01, m12);
    Point2D m123 = mid(m12, m23);
    Point2D m0123 = mid(m012, m123);

    // First half: p0 -> m0123 with control m01
    // Quadratic control point = 2*midpoint_of_cubic_controls - 0.5*(start+end)
    // Simplified: use m01 as approx control for first half
    QuadraticBezier q1;
    q1.p0 = {p0.x * scale, p0.y * scale};
    q1.p1 = {m01.x * scale, m01.y * scale};
    q1.p2 = {m0123.x * scale, m0123.y * scale};
    out.push_back(q1);

    // Second half: m0123 -> p1 with control m23
    QuadraticBezier q2;
    q2.p0 = {m0123.x * scale, m0123.y * scale};
    q2.p1 = {m23.x * scale, m23.y * scale};
    q2.p2 = {p1.x * scale, p1.y * scale};
    out.push_back(q2);
}

// Process a single contour from the outline.
void processContour(const FT_Outline& outline,
                    int contour_idx,
                    float scale,
                    std::vector<QuadraticBezier>& out) {
    int start = (contour_idx == 0) ? 0 : (outline.contours[contour_idx - 1] + 1);
    int end = outline.contours[contour_idx];
    int count = end - start + 1;
    if (count < 2) return;

    auto getPoint = [&](int idx) -> Point2D {
        int i = start + ((idx % count) + count) % count;
        return {static_cast<float>(outline.points[i].x),
                static_cast<float>(outline.points[i].y)};
    };

    auto isOnCurve = [&](int idx) -> bool {
        int i = start + ((idx % count) + count) % count;
        return (outline.tags[i] & FT_CURVE_TAG_ON) != 0;
    };

    auto isCubic = [&](int idx) -> bool {
        int i = start + ((idx % count) + count) % count;
        return (outline.tags[i] & FT_CURVE_TAG_CUBIC) != 0;
    };

    // Walk through the contour emitting quadratic Bezier curves.
    // TrueType outlines use on-curve and off-curve (conic) points.
    // CFF outlines may have cubic control points.
    int i = 0;
    while (i < count) {
        if (!isOnCurve(i)) {
            // Shouldn't normally start off-curve, but handle gracefully
            i++;
            continue;
        }

        Point2D p0 = getPoint(i);
        int next = (i + 1) % count;

        if (isOnCurve(next)) {
            // Line segment: emit as degenerate quadratic
            Point2D p1 = getPoint(next);
            QuadraticBezier q;
            q.p0 = {p0.x * scale, p0.y * scale};
            q.p1 = {(p0.x + p1.x) * 0.5f * scale, (p0.y + p1.y) * 0.5f * scale};
            q.p2 = {p1.x * scale, p1.y * scale};
            out.push_back(q);
            i++;
            continue;
        }

        if (isCubic(next)) {
            // Cubic Bezier: need two off-curve control points
            int next2 = (next + 1) % count;
            int next3 = (next2 + 1) % count;
            Point2D c0 = getPoint(next);
            Point2D c1 = getPoint(next2);
            Point2D endPt = getPoint(next3);
            cubicToQuadratics(p0, c0, c1, endPt, scale, out);
            i += 3;
            continue;
        }

        // Off-curve conic (TrueType quadratic)
        // Handle consecutive off-curve points by inserting implicit on-curve midpoints
        int j = next;
        Point2D prevOnCurve = p0;

        while (!isOnCurve(j)) {
            int jnext = (j + 1) % count;
            Point2D offCurve = getPoint(j);
            Point2D endPt;

            if (isOnCurve(jnext)) {
                endPt = getPoint(jnext);
            } else {
                // Implicit on-curve midpoint
                Point2D nextOff = getPoint(jnext);
                endPt = {(offCurve.x + nextOff.x) * 0.5f,
                         (offCurve.y + nextOff.y) * 0.5f};
            }

            QuadraticBezier q;
            q.p0 = {prevOnCurve.x * scale, prevOnCurve.y * scale};
            q.p1 = {offCurve.x * scale, offCurve.y * scale};
            q.p2 = {endPt.x * scale, endPt.y * scale};
            out.push_back(q);

            prevOnCurve = endPt;
            j = (j + 1) % count;

            if (isOnCurve(j)) break;
        }

        // Advance past the consumed points
        int consumed = ((j - next) % count + count) % count;
        i += consumed + 1;
    }
}

} // namespace

std::vector<QuadraticBezier> loadGlyphOutline(FT_Face face, uint32_t glyph_id) {
    std::vector<QuadraticBezier> curves;

    if (!face) return curves;

    FT_Error err = FT_Load_Glyph(face, glyph_id,
                                  FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP);
    if (err) {
        DONG_LOG_WARN("[SlugOutline] FT_Load_Glyph failed for glyph %u: error %d",
                      glyph_id, err);
        return curves;
    }

    if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        return curves;  // No outline (e.g. bitmap-only or space glyph)
    }

    const FT_Outline& outline = face->glyph->outline;
    if (outline.n_contours <= 0 || outline.n_points <= 0) {
        return curves;
    }

    // Normalize to em-space
    float units_per_em = static_cast<float>(face->units_per_EM);
    float scale = (units_per_em > 0.0f) ? (1.0f / units_per_em) : 1.0f;

    curves.reserve(static_cast<size_t>(outline.n_points));

    for (int c = 0; c < outline.n_contours; c++) {
        processContour(outline, c, scale, curves);
    }

    return curves;
}

} // namespace dong::render::slug
