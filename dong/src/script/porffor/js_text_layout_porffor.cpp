#include "js_text_layout_porffor.hpp"

#include "js_bindings_porffor.hpp"
#include "porffor_mini_json.hpp"
#include "../../core/log.h"
#include "../../render/text_layout_core.hpp"
#include "../../render/overlay_draw.hpp"
#include "../../render/text_shaper.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace dong::script {
namespace {

namespace pj = porffor_json;

struct Slot {
    float left;
    float right;
};

struct CircleDef {
    float cx, cy, r, h_pad, v_pad;
};

struct RectDef {
    float x, y, w, h, h_pad, v_pad;
};

static render::TextLayoutCore s_layout_core;
static size_t s_cached_key = 0;
static render::PreparedText s_cached_prepared;

static bool pretextTimingEnabled() {
    static const bool enabled = std::getenv("DONG_DEBUG_PRETEXT_TIMING") != nullptr;
    return enabled;
}

static double elapsedMs(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
        .count();
}

static size_t makeKey(const std::string& text, const std::string& family, const std::string& weight,
                      const std::string& style, float font_size, float line_height) {
    size_t h = std::hash<std::string>{}(text);
    h ^= std::hash<std::string>{}(family) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<std::string>{}(weight) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<std::string>{}(style) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<float>{}(font_size) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<float>{}(line_height) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
}

static std::vector<Slot> carveSlots(Slot base, const std::vector<Slot>& blocked) {
    std::vector<Slot> free = {base};
    for (const auto& b : blocked) {
        std::vector<Slot> next;
        for (const auto& f : free) {
            if (b.right <= f.left || b.left >= f.right) {
                next.push_back(f);
                continue;
            }
            if (b.left > f.left) {
                next.push_back({f.left, b.left});
            }
            if (b.right < f.right) {
                next.push_back({b.right, f.right});
            }
        }
        free = std::move(next);
    }
    std::vector<Slot> result;
    for (const auto& s : free) {
        if (s.right - s.left > 1.0f) {
            result.push_back(s);
        }
    }
    return result;
}

static bool circleBlockedInterval(float cx, float cy, float r, float h_pad, float v_pad, float band_top,
                                  float band_bottom, Slot& out) {
    float ry = r + v_pad;
    if (band_bottom <= cy - ry || band_top >= cy + ry) {
        return false;
    }
    float dy = 0;
    if (band_top > cy) {
        dy = band_top - cy;
    } else if (band_bottom < cy) {
        dy = cy - band_bottom;
    }
    if (dy >= r) {
        return false;
    }
    float half = std::sqrt(r * r - dy * dy);
    out = {cx - half - h_pad, cx + half + h_pad};
    return out.right > out.left;
}

static bool rectBlockedInterval(float rx, float ry, float rw, float rh, float h_pad, float v_pad,
                                float band_top, float band_bottom, Slot& out) {
    if (band_bottom <= ry - v_pad || band_top >= ry + rh + v_pad) {
        return false;
    }
    out = {rx - h_pad, rx + rw + h_pad};
    return out.right > out.left;
}

static render::Color parseHexColor(const std::string& s, render::Color fallback = {0.2f, 0.2f, 0.2f, 1.0f}) {
    if (s.size() < 7 || s[0] != '#') {
        return fallback;
    }
    auto hex2f = [](const char* p) -> float {
        int v = 0;
        for (int i = 0; i < 2; i++) {
            char c = p[i];
            v = v * 16 + (c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c - '0');
        }
        return v / 255.0f;
    };
    render::Color c;
    c.r = hex2f(s.c_str() + 1);
    c.g = hex2f(s.c_str() + 3);
    c.b = hex2f(s.c_str() + 5);
    c.a = s.size() >= 9 ? hex2f(s.c_str() + 7) : 1.0f;
    return c;
}

static void parseObstacles(const std::string& json, std::vector<CircleDef>& circles,
                           std::vector<RectDef>& rects) {
    size_t obs = json.find("\"obstacles\"");
    if (obs == std::string::npos) {
        return;
    }
    size_t circles_pos = json.find("\"circles\"", obs);
    if (circles_pos != std::string::npos) {
        size_t arr_start = json.find('[', circles_pos);
        size_t arr_end = json.find(']', arr_start);
        if (arr_start != std::string::npos && arr_end != std::string::npos) {
            std::string arr = json.substr(arr_start, arr_end - arr_start + 1);
            size_t pos = 0;
            while ((pos = arr.find('{', pos)) != std::string::npos) {
                size_t end = arr.find('}', pos);
                if (end == std::string::npos) {
                    break;
                }
                std::string item = arr.substr(pos, end - pos + 1);
                CircleDef c{};
                double d = 0;
                if (pj::extractNumber(item, "cx", d)) {
                    c.cx = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "cy", d)) {
                    c.cy = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "r", d)) {
                    c.r = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "hPad", d)) {
                    c.h_pad = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "vPad", d)) {
                    c.v_pad = static_cast<float>(d);
                }
                circles.push_back(c);
                pos = end + 1;
            }
        }
    }
    size_t rects_pos = json.find("\"rects\"", obs);
    if (rects_pos != std::string::npos) {
        size_t arr_start = json.find('[', rects_pos);
        size_t arr_end = json.find(']', arr_start);
        if (arr_start != std::string::npos && arr_end != std::string::npos) {
            std::string arr = json.substr(arr_start, arr_end - arr_start + 1);
            size_t pos = 0;
            while ((pos = arr.find('{', pos)) != std::string::npos) {
                size_t end = arr.find('}', pos);
                if (end == std::string::npos) {
                    break;
                }
                std::string item = arr.substr(pos, end - pos + 1);
                RectDef r{};
                double d = 0;
                if (pj::extractNumber(item, "x", d)) {
                    r.x = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "y", d)) {
                    r.y = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "w", d)) {
                    r.w = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "h", d)) {
                    r.h = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "hPad", d)) {
                    r.h_pad = static_cast<float>(d);
                }
                if (pj::extractNumber(item, "vPad", d)) {
                    r.v_pad = static_cast<float>(d);
                }
                rects.push_back(r);
                pos = end + 1;
            }
        }
    }
}

} // namespace

std::string porfforTextLayout(const std::string& config_json) {
    const auto total_start = std::chrono::steady_clock::now();
    std::string text;
    if (!pj::extractString(config_json, "text", text)) {
        return "{\"columns\":[]}";
    }

    std::string family = "sans-serif";
    std::string weight = "normal";
    std::string style = "normal";
    float font_size = 16;
    float line_height = 0;

    size_t font_pos = config_json.find("\"font\"");
    if (font_pos != std::string::npos) {
        size_t obj_start = config_json.find('{', font_pos);
        size_t obj_end = config_json.find('}', obj_start);
        if (obj_start != std::string::npos && obj_end != std::string::npos) {
            std::string font_obj = config_json.substr(obj_start, obj_end - obj_start + 1);
            std::string s;
            if (pj::extractString(font_obj, "family", s)) {
                family = s;
            }
            if (pj::extractString(font_obj, "weight", s)) {
                weight = s;
            }
            if (pj::extractString(font_obj, "style", s)) {
                style = s;
            }
            double d = 0;
            if (pj::extractNumber(font_obj, "size", d)) {
                font_size = static_cast<float>(d);
            }
        }
    }

    double d = 0;
    if (pj::extractNumber(config_json, "lineHeight", d)) {
        line_height = static_cast<float>(d);
    }

    size_t key = makeKey(text, family, weight, style, font_size, line_height);
    double prepare_ms = 0.0;
    if (key != s_cached_key) {
        const auto prepare_start = std::chrono::steady_clock::now();
        s_cached_prepared = s_layout_core.prepare(text, family, weight, style, font_size, line_height);
        prepare_ms = elapsedMs(prepare_start);
        s_cached_key = key;
    }
    const auto& prepared = s_cached_prepared;

    std::vector<CircleDef> circles;
    std::vector<RectDef> rects;
    parseObstacles(config_json, circles, rects);

    std::ostringstream out;
    out << "{\"columns\":[";
    bool first_col = true;

    size_t cols_pos = config_json.find("\"columns\"");
    if (cols_pos != std::string::npos) {
        size_t arr_start = config_json.find('[', cols_pos);
        size_t arr_end = config_json.rfind(']');
        if (arr_start != std::string::npos && arr_end != std::string::npos && arr_end > arr_start) {
            std::string cols_arr = config_json.substr(arr_start, arr_end - arr_start + 1);
            render::TextCursor cursor{};
            float lh = prepared.line_height_px;

            size_t pos = 0;
            while ((pos = cols_arr.find('{', pos)) != std::string::npos) {
                size_t end = cols_arr.find('}', pos);
                if (end == std::string::npos) {
                    break;
                }
                std::string col_cfg = cols_arr.substr(pos, end - pos + 1);
                float rx = 0, ry = 0, rw = 300, rh_val = 600;
                if (pj::extractNumber(col_cfg, "x", d)) {
                    rx = static_cast<float>(d);
                }
                if (pj::extractNumber(col_cfg, "y", d)) {
                    ry = static_cast<float>(d);
                }
                if (pj::extractNumber(col_cfg, "width", d)) {
                    rw = static_cast<float>(d);
                }
                if (pj::extractNumber(col_cfg, "height", d)) {
                    rh_val = static_cast<float>(d);
                }

                if (!first_col) {
                    out << ',';
                }
                first_col = false;
                out << "{\"lines\":[";
                bool first_line = true;
                float y = ry;

                while (!cursor.at_end(prepared) && y + lh <= ry + rh_val + 0.5f) {
                    std::vector<Slot> blocked;
                    Slot interval;
                    for (const auto& c : circles) {
                        if (circleBlockedInterval(c.cx, c.cy, c.r, c.h_pad, c.v_pad, y, y + lh, interval)) {
                            blocked.push_back(interval);
                        }
                    }
                    for (const auto& r : rects) {
                        if (rectBlockedInterval(r.x, r.y, r.w, r.h, r.h_pad, r.v_pad, y, y + lh, interval)) {
                            blocked.push_back(interval);
                        }
                    }

                    auto slots = carveSlots({rx, rx + rw}, blocked);
                    if (slots.empty()) {
                        y += lh;
                        continue;
                    }

                    for (const auto& slot : slots) {
                        if (cursor.at_end(prepared)) {
                            break;
                        }
                        float sw = slot.right - slot.left;
                        if (sw < prepared.font_size) {
                            continue;
                        }

                        render::LayoutLine line;
                        if (s_layout_core.layoutNextLine(prepared, cursor, sw, line)) {
                            if (!first_line) {
                                out << ',';
                            }
                            first_line = false;
                            out << "{\"x\":" << slot.left << ",\"y\":" << y << ",\"width\":" << line.width
                                << ",\"text\":\"" << pj::escapeJson(line.text) << "\"}";
                        }
                    }
                    y += lh;
                }
                out << "]}";
                pos = end + 1;
            }

            out << "],\"lineHeight\":" << lh << ",\"ascent\":" << prepared.ascent_px << "}";
            std::string result = out.str();
            if (pretextTimingEnabled()) {
                DONG_LOG_INFO("[pretext timing] textLayout total=%.3f prepare=%.3f bytes=%zu",
                              elapsedMs(total_start), prepare_ms, result.size());
            }
            return result;
        }
    }

    out << "],\"lineHeight\":0,\"ascent\":0}";
    std::string result = out.str();
    if (pretextTimingEnabled()) {
        DONG_LOG_INFO("[pretext timing] textLayout total=%.3f prepare=%.3f bytes=%zu",
                      elapsedMs(total_start), prepare_ms, result.size());
    }
    return result;
}

void porfforClearOverlay(dong::render::OverlayDraw* overlay) {
    if (overlay) {
        overlay->clear();
    }
}

void porfforRenderText(const std::string& config_json, dong::render::OverlayDraw* overlay) {
    if (!overlay) {
        return;
    }

    std::string family = "sans-serif";
    std::string weight = "normal";
    std::string style = "normal";
    float font_size = 16;

    size_t font_pos = config_json.find("\"font\"");
    if (font_pos != std::string::npos) {
        size_t obj_start = config_json.find('{', font_pos);
        size_t obj_end = config_json.find('}', obj_start);
        if (obj_start != std::string::npos && obj_end != std::string::npos) {
            std::string font_obj = config_json.substr(obj_start, obj_end - obj_start + 1);
            std::string s;
            if (pj::extractString(font_obj, "family", s)) {
                family = s;
            }
            if (pj::extractString(font_obj, "weight", s)) {
                weight = s;
            }
            if (pj::extractString(font_obj, "style", s)) {
                style = s;
            }
            double d = 0;
            if (pj::extractNumber(font_obj, "size", d)) {
                font_size = static_cast<float>(d);
            }
        }
    }

    std::string color_str = "#333333";
    pj::extractString(config_json, "color", color_str);
    render::Color color = parseHexColor(color_str);

    size_t lines_pos = config_json.find("\"lines\"");
    if (lines_pos == std::string::npos) {
        return;
    }
    size_t arr_start = config_json.find('[', lines_pos);
    size_t arr_end = config_json.find(']', arr_start);
    if (arr_start == std::string::npos || arr_end == std::string::npos) {
        return;
    }
    std::string lines_arr = config_json.substr(arr_start, arr_end - arr_start + 1);

    render::DrawGlyphRunData merged;
    merged.color = color;
    merged.font_size = font_size;
    merged.font_family = family;
    merged.font_weight = weight;
    merged.font_style = style;

    float min_x = 1e9f, min_y = 1e9f, max_x = -1e9f, max_y = -1e9f;
    bool first_shape = true;
    render::TextShaper shaper;

    size_t pos = 0;
    while ((pos = lines_arr.find('{', pos)) != std::string::npos) {
        size_t end = lines_arr.find('}', pos);
        if (end == std::string::npos) {
            break;
        }
        std::string line_obj = lines_arr.substr(pos, end - pos + 1);
        double d = 0;
        float x = 0, y = 0;
        if (pj::extractNumber(line_obj, "x", d)) {
            x = static_cast<float>(d);
        }
        if (pj::extractNumber(line_obj, "y", d)) {
            y = static_cast<float>(d);
        }
        std::string text;
        pj::extractString(line_obj, "text", text);
        if (!text.empty()) {
            render::TextShapeRequest req;
            req.text = text;
            req.font_family = family;
            req.font_weight = weight;
            req.font_style = style;
            req.font_size = font_size;

            render::ShapedText shaped;
            if (shaper.shape(req, shaped) && !shaped.glyphs.empty()) {
                if (first_shape) {
                    merged.font_paths = shaped.font_paths;
                    merged.font_path = shaped.font_path;
                    merged.units_per_em = shaped.units_per_em;
                    merged.scale_to_pixels = shaped.scale_to_pixels;
                    first_shape = false;
                }

                float ascent = shaped.ascent_units * shaped.scale_to_pixels;
                float baseline_y = y + ascent;
                float inv_scale = merged.scale_to_pixels > 0 ? (1.0f / merged.scale_to_pixels) : 1.0f;
                float origin_x_units = x * inv_scale;
                float origin_y_units = baseline_y * inv_scale;

                for (const auto& sg : shaped.glyphs) {
                    render::GlyphInstance gi;
                    gi.glyph_id = sg.glyph_id;
                    gi.pen_x_units = sg.pen_x_units + origin_x_units;
                    gi.pen_y_units = sg.pen_y_units + origin_y_units;
                    gi.font_path_index = sg.font_path_index;
                    gi.units_per_em = sg.units_per_em;
                    merged.glyphs.push_back(gi);
                }

                float text_width = shaped.width_units * shaped.scale_to_pixels;
                float text_height = ascent + std::abs(shaped.descent_units * shaped.scale_to_pixels);
                min_x = std::min(min_x, x);
                min_y = std::min(min_y, y);
                max_x = std::max(max_x, x + text_width);
                max_y = std::max(max_y, y + text_height);
            }
        }
        pos = end + 1;
    }

    if (!merged.glyphs.empty()) {
        merged.rect = {min_x, min_y, max_x - min_x, max_y - min_y};
        overlay->addGlyphRun(std::move(merged));
    }
}

void porfforDrawRect(const std::string& config_json, dong::render::OverlayDraw* overlay) {
    if (!overlay) {
        return;
    }
    double d = 0;
    float x = 0, y = 0, w = 0, h = 0, radius = 0, strokeWidth = 0;
    if (pj::extractNumber(config_json, "x", d)) {
        x = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "y", d)) {
        y = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "w", d)) {
        w = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "h", d)) {
        h = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "radius", d)) {
        radius = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "strokeWidth", d)) {
        strokeWidth = static_cast<float>(d);
    }
    std::string color_str = "#ffffff";
    pj::extractString(config_json, "color", color_str);
    render::Color color = parseHexColor(color_str);

    if (radius > 0.01f || strokeWidth > 0.01f) {
        overlay->addRoundedRect({x, y, w, h}, color, radius, strokeWidth);
    } else {
        overlay->addRect({x, y, w, h}, color);
    }
}

void porfforDrawCircle(const std::string& config_json, dong::render::OverlayDraw* overlay) {
    if (!overlay) {
        return;
    }
    double d = 0;
    float cx = 0, cy = 0, r = 0, strokeWidth = 0;
    if (pj::extractNumber(config_json, "cx", d)) {
        cx = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "cy", d)) {
        cy = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "r", d)) {
        r = static_cast<float>(d);
    }
    if (pj::extractNumber(config_json, "strokeWidth", d)) {
        strokeWidth = static_cast<float>(d);
    }
    std::string color_str = "#ffffff";
    pj::extractString(config_json, "color", color_str);
    render::Color color = parseHexColor(color_str);
    overlay->addCircle(cx, cy, r, color, strokeWidth);
}

static const char* kPretextTypoText =
    "Typography is the art and technique of arranging type to make written "
    "language legible, readable, and appealing when displayed. The arrangement "
    "of type involves selecting typefaces, point sizes, line lengths, line "
    "spacing, and letter spacing, as well as adjusting the space between "
    "pairs of letters. The term typography is also applied to the style, "
    "arrangement, and appearance of the letters, numbers, and symbols created "
    "by the process. Type design is a closely related craft, sometimes "
    "considered part of typography; most typographers do not design typefaces, "
    "and some type designers do not consider themselves typographers. "
    "Typography is the work of typesetters, compositors, typographers, "
    "graphic designers, art directors, manga artists, comic book artists, "
    "and now anyone who arranges words, letters, numbers, and symbols for "
    "publication, display, or distribution. Until the Digital Age, typography "
    "was a specialized occupation. Digitization opened up typography to new "
    "generations of previously unrelated designers and lay users. As the "
    "capability to create typography has become ubiquitous, the application "
    "of principles and best practices developed over generations of skilled "
    "workers and professionals has diminished. So at a time when scientific "
    "knowledge of how typefaces affect readability has flourished, an "
    "understanding of conventional wisdom accumulated since Gutenberg has "
    "simultaneously diminished. Yet the craft endures, and each new generation "
    "discovers anew the profound satisfaction of placing words upon a page with "
    "care, precision, and an eye for beauty that transcends the merely functional.";

static const char* kPretextDualModeText =
    "In a dual-mode rendering engine, static UI structure lives in the DOM — "
    "menus, panels, labels, and interactive elements are described in HTML/CSS "
    "and rendered through the retained pipeline. But dynamic content — particles, "
    "animated text, real-time visualizations — bypasses the DOM entirely. "
    "The immediate-mode overlay API (dong.drawRect, dong.drawCircle, dong.renderText) "
    "injects GPU commands directly into the display list, achieving zero DOM tax "
    "on animation frames. This is the architecture pretext envisions: free from "
    "DOM-based layout constraints, while keeping DOM for what it does best.";

static std::string escapeHtml(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            out += "&quot;";
            break;
        default:
            out.push_back(c);
            break;
        }
    }
    return out;
}

struct LayoutLine {
    float x = 0;
    float y = 0;
    float width = 0;
    std::string text;
};

static std::vector<LayoutLine> parseLayoutLinesInternal(const std::string& layout_json) {
    std::vector<LayoutLine> lines;
    size_t cols_pos = layout_json.find("\"columns\"");
    if (cols_pos == std::string::npos) {
        return lines;
    }
    size_t pos = cols_pos;
    while ((pos = layout_json.find("\"lines\"", pos)) != std::string::npos) {
        size_t arr_start = layout_json.find('[', pos);
        size_t arr_end = layout_json.find(']', arr_start);
        if (arr_start == std::string::npos || arr_end == std::string::npos) {
            break;
        }
        std::string lines_arr = layout_json.substr(arr_start, arr_end - arr_start + 1);
        size_t lp = 0;
        while ((lp = lines_arr.find('{', lp)) != std::string::npos) {
            size_t end = lines_arr.find('}', lp);
            if (end == std::string::npos) {
                break;
            }
            std::string line_obj = lines_arr.substr(lp, end - lp + 1);
            LayoutLine ln;
            double d = 0;
            if (pj::extractNumber(line_obj, "x", d)) {
                ln.x = static_cast<float>(d);
            }
            if (pj::extractNumber(line_obj, "y", d)) {
                ln.y = static_cast<float>(d);
            }
            if (pj::extractNumber(line_obj, "width", d)) {
                ln.width = static_cast<float>(d);
            }
            pj::extractString(line_obj, "text", ln.text);
            if (!ln.text.empty()) {
                lines.push_back(std::move(ln));
            }
            lp = end + 1;
        }
        pos = arr_end + 1;
    }
    return lines;
}

std::vector<PretextLayoutLine> pretextParseLayoutLines(const std::string& layout_json) {
    const auto internal = parseLayoutLinesInternal(layout_json);
    std::vector<PretextLayoutLine> out;
    out.reserve(internal.size());
    for (const auto& ln : internal) {
        out.push_back({ln.x, ln.y, ln.width, ln.text});
    }
    return out;
}

static std::vector<LayoutLine> parseLayoutLines(const std::string& layout_json) {
    return parseLayoutLinesInternal(layout_json);
}

static int g_last_layout_line_count = 0;

int porfforLastLayoutLineCount() {
    return g_last_layout_line_count;
}

static std::string buildTypoConfig(const char* text, const std::string& columns_json,
                                   const std::string& obstacles_json, float font_size,
                                   float line_height) {
    std::ostringstream out;
    out << "{\"text\":\"" << pj::escapeJson(text) << "\""
        << ",\"font\":{\"family\":\"sans-serif\",\"weight\":\"normal\",\"style\":\"normal\",\"size\":"
        << font_size << "}"
        << ",\"lineHeight\":" << line_height << ",\"columns\":" << columns_json << ",\"obstacles\":"
        << obstacles_json << "}";
    return out.str();
}

std::string pretextTypoConfig(const std::string& columns_json, const std::string& obstacles_json) {
    return buildTypoConfig(kPretextTypoText, columns_json, obstacles_json, 14.0f, 20.0f);
}

std::string pretextDualModeTextConfig(const std::string& columns_json,
                                      const std::string& circles_json) {
    std::ostringstream obstacles;
    obstacles << "{\"circles\":" << circles_json << ",\"rects\":[]}";
    return buildTypoConfig(kPretextDualModeText, columns_json, obstacles.str(), 13.0f, 19.0f);
}

std::string pretextFlowObstaclesJson(float obs_a_cx, float obs_a_cy, float obs_a_r, float bx,
                                     float by, float obs_b_w, float obs_b_h, float obs_c_cx,
                                     float obs_c_cy, float obs_c_r) {
    std::ostringstream out;
    out << "{\"circles\":["
        << "{\"cx\":" << obs_a_cx << ",\"cy\":" << obs_a_cy << ",\"r\":" << obs_a_r
        << ",\"hPad\":8,\"vPad\":2}"
        << ",{\"cx\":" << obs_c_cx << ",\"cy\":" << obs_c_cy << ",\"r\":" << obs_c_r
        << ",\"hPad\":8,\"vPad\":2}"
        << "],\"rects\":[{\"x\":" << bx << ",\"y\":" << by << ",\"w\":" << obs_b_w
        << ",\"h\":" << obs_b_h << ",\"hPad\":8,\"vPad\":2}]}";
    return out.str();
}

std::string pretextFlowColumnsStatic() {
    return "[{\"x\":20,\"y\":56,\"width\":402,\"height\":390},{\"x\":444,\"y\":56,\"width\":436,"
           "\"height\":390}]";
}

std::string pretextFlowColumnsDynamic() {
    return "[{\"x\":20,\"y\":46,\"width\":402,\"height\":510},{\"x\":444,\"y\":46,\"width\":436,"
           "\"height\":510}]";
}

int porfforTextLayoutMountLines(uint64_t node_id, const std::string& config_json,
                                const std::string& line_class, bool stats_footer,
                                JSBindings* bindings) {
    const auto total_start = std::chrono::steady_clock::now();
    if (!bindings) {
        g_last_layout_line_count = 0;
        return 0;
    }
    const auto layout_start = std::chrono::steady_clock::now();
    const std::string layout_json = porfforTextLayout(config_json);
    const double layout_ms = elapsedMs(layout_start);
    const auto parse_start = std::chrono::steady_clock::now();
    const auto lines = parseLayoutLines(layout_json);
    const double parse_ms = elapsedMs(parse_start);

    const auto html_start = std::chrono::steady_clock::now();
    std::ostringstream html;
    const std::string cls = line_class.empty() ? "line" : line_class;
    for (const auto& ln : lines) {
        html << "<div class=\"" << cls
             << "\" style=\"position:absolute;left:" << static_cast<int>(std::lround(ln.x))
             << "px;top:" << static_cast<int>(std::lround(ln.y)) << "px;width:"
             << static_cast<int>(std::ceil(ln.width + 4.0f))
             << "px;color:#c9d1d9;font-size:14px;line-height:20px;white-space:nowrap;height:20px;font-family:sans-serif;\">"
             << escapeHtml(ln.text) << "</div>";
    }

    if (stats_footer) {
        int col_a = 0;
        int col_b = 0;
        size_t cols_pos = layout_json.find("\"columns\"");
        if (cols_pos != std::string::npos) {
            size_t pos = cols_pos;
            int col_idx = 0;
            while ((pos = layout_json.find("\"lines\"", pos)) != std::string::npos) {
                size_t arr_start = layout_json.find('[', pos);
                size_t arr_end = layout_json.find(']', arr_start);
                if (arr_start == std::string::npos || arr_end == std::string::npos) {
                    break;
                }
                int count = 0;
                size_t lp = arr_start;
                while ((lp = layout_json.find('{', lp)) != std::string::npos && lp < arr_end) {
                    ++count;
                    lp = layout_json.find('}', lp) + 1;
                }
                if (col_idx == 0) {
                    col_a = count;
                } else if (col_idx == 1) {
                    col_b = count;
                }
                ++col_idx;
                pos = arr_end + 1;
            }
        }
        html << "<div class=\"phase-label\" style=\"position:absolute;left:20px;bottom:8px;"
             << "width:300px;color:#3d5a80;font-size:10px;white-space:nowrap;\">A: "
             << col_a << " lines | B: " << col_b << " lines | total: " << lines.size() << "</div>";
    }

    const std::string html_str = html.str();
    const double html_ms = elapsedMs(html_start);
    const auto dom_start = std::chrono::steady_clock::now();
    bindings->setNodeInnerHTML(node_id, html_str);
    const double dom_ms = elapsedMs(dom_start);
    g_last_layout_line_count = static_cast<int>(lines.size());
    if (pretextTimingEnabled()) {
        DONG_LOG_INFO("[pretext timing] mountLines total=%.3f layout=%.3f parse=%.3f html=%.3f dom=%.3f node=%llu lines=%d html_bytes=%zu",
                      elapsedMs(total_start), layout_ms, parse_ms, html_ms, dom_ms,
                      static_cast<unsigned long long>(node_id), g_last_layout_line_count,
                      html_str.size());
    }
    return g_last_layout_line_count;
}

int porfforTextLayoutRenderOverlay(const std::string& config_json, const std::string& color,
                                   dong::render::OverlayDraw* overlay) {
    const auto total_start = std::chrono::steady_clock::now();
    if (!overlay) {
        g_last_layout_line_count = 0;
        return 0;
    }
    const auto layout_start = std::chrono::steady_clock::now();
    const std::string layout_json = porfforTextLayout(config_json);
    const double layout_ms = elapsedMs(layout_start);
    const auto parse_start = std::chrono::steady_clock::now();
    const auto lines = parseLayoutLines(layout_json);
    const double parse_ms = elapsedMs(parse_start);

    const auto json_start = std::chrono::steady_clock::now();
    std::ostringstream rt;
    rt << "{\"font\":{\"family\":\"sans-serif\",\"weight\":\"normal\",\"style\":\"normal\",\"size\":"
          "14},\"color\":\""
       << color << "\",\"lines\":[";
    bool first = true;
    for (const auto& ln : lines) {
        if (!first) {
            rt << ',';
        }
        first = false;
        rt << "{\"x\":" << ln.x << ",\"y\":" << ln.y << ",\"text\":\"" << pj::escapeJson(ln.text)
           << "\"}";
    }
    rt << "]}";
    const std::string render_json = rt.str();
    const double json_ms = elapsedMs(json_start);
    const auto overlay_start = std::chrono::steady_clock::now();
    porfforRenderText(render_json, overlay);
    const double overlay_ms = elapsedMs(overlay_start);
    g_last_layout_line_count = static_cast<int>(lines.size());
    if (pretextTimingEnabled()) {
        DONG_LOG_INFO("[pretext timing] renderOverlay total=%.3f layout=%.3f parse=%.3f json=%.3f overlay=%.3f lines=%d json_bytes=%zu",
                      elapsedMs(total_start), layout_ms, parse_ms, json_ms, overlay_ms,
                      g_last_layout_line_count, render_json.size());
    }
    return g_last_layout_line_count;
}

} // namespace dong::script
