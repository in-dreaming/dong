#include "js_text_layout_porffor.hpp"

#include "porffor_mini_json.hpp"
#include "../../render/text_layout_core.hpp"
#include "../../render/overlay_draw.hpp"
#include "../../render/text_shaper.hpp"

#include <cmath>
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
    if (key != s_cached_key) {
        s_cached_prepared = s_layout_core.prepare(text, family, weight, style, font_size, line_height);
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
            return out.str();
        }
    }

    out << "],\"lineHeight\":0,\"ascent\":0}";
    return out.str();
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

} // namespace dong::script
