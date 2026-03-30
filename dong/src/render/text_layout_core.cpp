#include "text_layout_core.hpp"
#include <algorithm>
#include <cmath>

namespace dong::render {

TextLayoutCore::TextLayoutCore() = default;
TextLayoutCore::~TextLayoutCore() = default;

// --- UTF-8 helpers ---

static bool isCJK(uint32_t cp) {
    return (cp >= 0x4E00 && cp <= 0x9FFF) ||
           (cp >= 0x3400 && cp <= 0x4DBF) ||
           (cp >= 0x20000 && cp <= 0x2A6DF) ||
           (cp >= 0x2A700 && cp <= 0x2B73F) ||
           (cp >= 0xF900 && cp <= 0xFAFF) ||
           (cp >= 0x3000 && cp <= 0x303F) ||
           (cp >= 0xFF00 && cp <= 0xFFEF);
}

static size_t utf8CharLen(unsigned char c) {
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    return 4;
}

static uint32_t decodeUtf8(const char* s, size_t len, size_t& out_bytes) {
    if (len == 0) { out_bytes = 0; return 0; }
    unsigned char c = static_cast<unsigned char>(s[0]);
    if ((c & 0x80) == 0) { out_bytes = 1; return c; }
    if ((c & 0xE0) == 0xC0 && len >= 2) {
        out_bytes = 2;
        return ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[1]) & 0x3F);
    }
    if ((c & 0xF0) == 0xE0 && len >= 3) {
        out_bytes = 3;
        return ((c & 0x0F) << 12) |
               ((static_cast<unsigned char>(s[1]) & 0x3F) << 6) |
               (static_cast<unsigned char>(s[2]) & 0x3F);
    }
    if ((c & 0xF8) == 0xF0 && len >= 4) {
        out_bytes = 4;
        return ((c & 0x07) << 18) |
               ((static_cast<unsigned char>(s[1]) & 0x3F) << 12) |
               ((static_cast<unsigned char>(s[2]) & 0x3F) << 6) |
               (static_cast<unsigned char>(s[3]) & 0x3F);
    }
    out_bytes = 1;
    return 0xFFFD;
}

// --- Segmentation ---
// Segments text into words, spaces, and individual CJK characters.
// Each CJK character is its own breakable segment (per-character line breaking).
// Spaces are breakable. Latin words run until next space or CJK.

static std::vector<PreparedSegment> segmentText(const std::string& text) {
    std::vector<PreparedSegment> segs;
    if (text.empty()) return segs;

    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        if (c == ' ' || c == '\t') {
            size_t start = i;
            while (i < text.size() && (text[i] == ' ' || text[i] == '\t')) ++i;
            PreparedSegment seg;
            seg.text = text.substr(start, i - start);
            seg.is_space = true;
            seg.is_break_after = true;
            segs.push_back(std::move(seg));
            continue;
        }

        if (c == '\n' || c == '\r') {
            ++i;
            if (c == '\r' && i < text.size() && text[i] == '\n') ++i;
            continue;
        }

        size_t bytes = 0;
        uint32_t cp = decodeUtf8(text.data() + i, text.size() - i, bytes);

        if (isCJK(cp)) {
            PreparedSegment seg;
            seg.text = text.substr(i, bytes);
            seg.is_space = false;
            seg.is_break_after = true;
            segs.push_back(std::move(seg));
            i += bytes;
            continue;
        }

        size_t start = i;
        i += bytes;
        while (i < text.size()) {
            unsigned char nc = static_cast<unsigned char>(text[i]);
            if (nc == ' ' || nc == '\t' || nc == '\n' || nc == '\r') break;
            size_t nb = 0;
            uint32_t ncp = decodeUtf8(text.data() + i, text.size() - i, nb);
            if (isCJK(ncp)) break;
            i += nb;
        }
        PreparedSegment seg;
        seg.text = text.substr(start, i - start);
        seg.is_space = false;
        seg.is_break_after = false;
        segs.push_back(std::move(seg));
    }

    for (size_t s = 0; s + 1 < segs.size(); ++s) {
        if (!segs[s].is_space && !segs[s].is_break_after) {
            if (segs[s + 1].is_space || segs[s + 1].is_break_after) {
                segs[s].is_break_after = true;
            }
        }
    }
    if (!segs.empty()) segs.back().is_break_after = true;

    return segs;
}

// --- Measure a single segment via HarfBuzz ---

float TextLayoutCore::measureSegment(const std::string& text,
                                     const PreparedText& prepared) {
    if (text.empty()) return 0.0f;

    TextShapeRequest req;
    req.text = text;
    req.font_family = prepared.font_family;
    req.font_weight = prepared.font_weight;
    req.font_style = prepared.font_style;
    req.font_size = prepared.font_size;
    req.lang = prepared.lang;

    ShapedText shaped;
    if (!shaper_.shape(req, shaped) || shaped.glyphs.empty()) {
        return static_cast<float>(text.size()) * prepared.font_size * 0.6f;
    }
    return shaped.width_units * shaped.scale_to_pixels;
}

// --- Phase 1: prepare ---

PreparedText TextLayoutCore::prepare(
    const std::string& text,
    const std::string& font_family,
    const std::string& font_weight,
    const std::string& font_style,
    float font_size,
    float line_height,
    const std::string& lang) {

    PreparedText result;
    result.font_family = font_family;
    result.font_weight = font_weight;
    result.font_style = font_style;
    result.font_size = font_size;
    result.lang = lang;

    {
        TextShapeRequest req;
        req.text = "X";
        req.font_family = font_family;
        req.font_weight = font_weight;
        req.font_style = font_style;
        req.font_size = font_size;
        req.lang = lang;

        ShapedText shaped;
        if (shaper_.shape(req, shaped) && shaped.units_per_em > 0) {
            result.scale_to_pixels = shaped.scale_to_pixels;
            result.ascent_px = shaped.ascent_units * shaped.scale_to_pixels;
            result.descent_px = shaped.descent_units * shaped.scale_to_pixels;
            float natural_lh = shaped.line_height_units * shaped.scale_to_pixels;
            result.line_height_px = (line_height > 0.0f) ? line_height : natural_lh;
            if (result.line_height_px <= 0.0f) result.line_height_px = font_size * 1.2f;
        } else {
            result.scale_to_pixels = 1.0f;
            result.ascent_px = font_size * 0.8f;
            result.descent_px = font_size * 0.2f;
            result.line_height_px = (line_height > 0.0f) ? line_height : font_size * 1.2f;
        }
    }

    result.segments = segmentText(text);

    for (auto& seg : result.segments) {
        seg.width_px = measureSegment(seg.text, result);
    }

    return result;
}

// --- Phase 2: layoutNextLine ---
// Pure arithmetic on cached segment widths. No shaping calls on the normal path.

bool TextLayoutCore::layoutNextLine(
    const PreparedText& prepared,
    TextCursor& cursor,
    float max_width,
    LayoutLine& out_line) {

    if (cursor.at_end(prepared)) return false;

    out_line.start = cursor;
    out_line.text.clear();
    out_line.width = 0.0f;

    float remaining = max_width;
    bool first_on_line = true;

    while (!cursor.at_end(prepared)) {
        const auto& seg = prepared.segments[cursor.segment_index];

        if (first_on_line && seg.is_space) {
            cursor.segment_index++;
            cursor.byte_offset = 0;
            continue;
        }

        float seg_width = seg.width_px;
        std::string seg_text = seg.text;

        if (cursor.byte_offset > 0 && !seg.is_space) {
            seg_text = seg.text.substr(cursor.byte_offset);
            seg_width = measureSegment(seg_text, prepared);
        }

        if (seg.is_space) {
            if (!first_on_line) {
                if (remaining >= seg_width) {
                    remaining -= seg_width;
                    out_line.width += seg_width;
                    out_line.text += seg_text;
                } else {
                    cursor.segment_index++;
                    cursor.byte_offset = 0;
                    break;
                }
            }
            cursor.segment_index++;
            cursor.byte_offset = 0;
            continue;
        }

        if (seg_width <= remaining) {
            out_line.text += seg_text;
            out_line.width += seg_width;
            remaining -= seg_width;
            first_on_line = false;
            cursor.segment_index++;
            cursor.byte_offset = 0;
            continue;
        }

        if (!first_on_line) break;

        // Force-break: word wider than max_width on an empty line.
        // Binary search for the longest prefix that fits.
        size_t lo = 1, hi = seg_text.size(), best = 0;
        while (lo <= hi) {
            size_t mid = lo + (hi - lo) / 2;
            size_t actual = mid;
            while (actual < seg_text.size() &&
                   (static_cast<unsigned char>(seg_text[actual]) & 0xC0) == 0x80) {
                actual++;
            }
            if (actual > seg_text.size()) actual = seg_text.size();

            float w = measureSegment(seg_text.substr(0, actual), prepared);
            if (w <= max_width) {
                best = actual;
                lo = actual + 1;
            } else {
                if (actual <= 1) { best = actual; break; }
                hi = actual - 1;
            }
        }
        if (best == 0) {
            best = utf8CharLen(static_cast<unsigned char>(seg_text[0]));
        }

        out_line.text = seg_text.substr(0, best);
        out_line.width = measureSegment(out_line.text, prepared);
        cursor.byte_offset += best;
        if (cursor.byte_offset >= seg.text.size()) {
            cursor.segment_index++;
            cursor.byte_offset = 0;
        }
        first_on_line = false;
        break;
    }

    out_line.end = cursor;

    while (!out_line.text.empty() && out_line.text.back() == ' ') {
        out_line.text.pop_back();
    }

    return !out_line.text.empty() ||
           (out_line.start.segment_index != out_line.end.segment_index);
}

} // namespace dong::render
