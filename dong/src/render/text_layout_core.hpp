#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include "text_shaper.hpp"

namespace dong::render {

struct PreparedSegment {
    std::string text;
    float width_px = 0.0f;
    bool is_space = false;
    bool is_break_after = false;
};

struct PreparedText {
    std::vector<PreparedSegment> segments;

    std::string font_family;
    std::string font_weight;
    std::string font_style;
    float font_size = 16.0f;
    float line_height_px = 0.0f;
    float ascent_px = 0.0f;
    float descent_px = 0.0f;
    float scale_to_pixels = 1.0f;
    std::string lang;
};

struct TextCursor {
    size_t segment_index = 0;
    size_t byte_offset = 0;

    bool at_end(const PreparedText& prepared) const {
        return segment_index >= prepared.segments.size();
    }
};

struct LayoutLine {
    TextCursor start;
    TextCursor end;
    float width = 0.0f;
    std::string text;
};

// Two-phase text layout inspired by pretext's architecture:
//   Phase 1: prepare() — segment text, measure each segment via HarfBuzz, cache widths.
//   Phase 2: layoutNextLine() — pure arithmetic on cached widths. Each call can use
//            a different max_width, enabling obstacle-aware and multi-column flow.
class TextLayoutCore {
public:
    TextLayoutCore();
    ~TextLayoutCore();

    PreparedText prepare(const std::string& text,
                         const std::string& font_family,
                         const std::string& font_weight,
                         const std::string& font_style,
                         float font_size,
                         float line_height,
                         const std::string& lang = "");

    bool layoutNextLine(const PreparedText& prepared,
                        TextCursor& cursor,
                        float max_width,
                        LayoutLine& out_line);

private:
    float measureSegment(const std::string& text, const PreparedText& prepared);
    TextShaper shaper_;
};

} // namespace dong::render
