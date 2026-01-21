#pragma once

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "../display_list.hpp" // Color, DrawGlyphRunData
#include "../../dom/css/computed_style.hpp"

namespace dong::render::painter_detail {

// CSS 颜色解析器：支持 #rgb/#rgba/#rrggbb/#rrggbbaa 与 rgb()/rgba() 子集
inline void parseCssColor(const std::string& css, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    auto clampToByte = [](int v) -> uint8_t {
        if (v < 0) return 0;
        if (v > 255) return 255;
        return static_cast<uint8_t>(v);
    };

    auto parseHexNibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    };

    auto parseHex2 = [&](char c1, char c2) -> uint8_t {
        int hi = parseHexNibble(c1);
        int lo = parseHexNibble(c2);
        return clampToByte((hi << 4) | lo);
    };

    auto parseComponent = [&](const std::string& s, bool is_alpha, int& out_int, float& out_alpha) {
        std::string v = s;
        v.erase(v.begin(), std::find_if(v.begin(), v.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.back()))) {
            v.pop_back();
        }

        bool is_percent = false;
        if (!v.empty() && v.back() == '%') {
            is_percent = true;
            v.pop_back();
        }

        float f = 0.0f;
        try {
            f = std::stof(v);
        } catch (...) {
            f = 0.0f;
        }

        if (!is_alpha) {
            if (is_percent) {
                f = f * 255.0f / 100.0f;
            }
            out_int = static_cast<int>(std::round(f));
            out_alpha = 1.0f;
        } else {
            if (is_percent) {
                f = f / 100.0f;
            }
            if (f < 0.0f) f = 0.0f;
            if (f > 1.0f) f = 1.0f;
            out_int = static_cast<int>(std::round(f * 255.0f));
            out_alpha = f;
        }
    };

    std::string s = css;
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); }), s.end());
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    r = g = b = 240;
    a = 255;

    if (s.empty() || s == "transparent") {
        r = g = b = 0;
        a = 0;
        return;
    }

    if (s[0] == '#') {
        if (s.size() == 4) {
            int r4 = parseHexNibble(s[1]);
            int g4 = parseHexNibble(s[2]);
            int b4 = parseHexNibble(s[3]);
            r = clampToByte((r4 << 4) | r4);
            g = clampToByte((g4 << 4) | g4);
            b = clampToByte((b4 << 4) | b4);
            a = 255;
            return;
        } else if (s.size() == 5) {
            int r4 = parseHexNibble(s[1]);
            int g4 = parseHexNibble(s[2]);
            int b4 = parseHexNibble(s[3]);
            int a4 = parseHexNibble(s[4]);
            r = clampToByte((r4 << 4) | r4);
            g = clampToByte((g4 << 4) | g4);
            b = clampToByte((b4 << 4) | b4);
            a = clampToByte((a4 << 4) | a4);
            return;
        } else if (s.size() == 7) {
            r = parseHex2(s[1], s[2]);
            g = parseHex2(s[3], s[4]);
            b = parseHex2(s[5], s[6]);
            a = 255;
            return;
        } else if (s.size() == 9) {
            r = parseHex2(s[1], s[2]);
            g = parseHex2(s[3], s[4]);
            b = parseHex2(s[5], s[6]);
            a = parseHex2(s[7], s[8]);
            return;
        }
    }

    auto startsWith = [](const std::string& str, const char* prefix) {
        const size_t len = std::strlen(prefix);
        return str.size() >= len && std::equal(prefix, prefix + len, str.begin());
    };

    if (startsWith(s, "rgb(") || startsWith(s, "rgba(")) {
        bool has_alpha = startsWith(s, "rgba(");
        size_t lparen = s.find('(');
        size_t rparen = s.rfind(')');
        if (lparen != std::string::npos && rparen != std::string::npos && rparen > lparen + 1) {
            std::string args = s.substr(lparen + 1, rparen - lparen - 1);
            std::vector<std::string> parts;
            std::string current;
            for (char c : args) {
                if (c == ',') {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                } else {
                    current.push_back(c);
                }
            }
            if (!current.empty()) parts.push_back(current);

            int r_int = 0, g_int = 0, b_int = 0, a_int = 255;
            float a_float = 1.0f;
            if ((has_alpha && parts.size() == 4) || (!has_alpha && parts.size() == 3)) {
                int dummy_int = 0;
                float dummy_alpha = 1.0f;
                parseComponent(parts[0], false, r_int, dummy_alpha);
                parseComponent(parts[1], false, g_int, dummy_alpha);
                parseComponent(parts[2], false, b_int, dummy_alpha);
                if (has_alpha) {
                    parseComponent(parts[3], true, a_int, a_float);
                }
                r = clampToByte(r_int);
                g = clampToByte(g_int);
                b = clampToByte(b_int);
                a = clampToByte(a_int);
                return;
            }
        }
    }

    if (s == "white")      { r = g = b = 255; a = 255; return; }
    if (s == "black")      { r = g = b = 0;   a = 255; return; }
    if (s == "red")        { r = 255; g = 0;   b = 0;   a = 255; return; }
    if (s == "green")      { r = 0;   g = 128; b = 0;   a = 255; return; }
    if (s == "blue")       { r = 0;   g = 0;   b = 255; a = 255; return; }
    if (s == "gray" || s == "grey") { r = g = b = 128; a = 255; return; }
    if (s == "lightgray" || s == "lightgrey") { r = g = b = 211; a = 255; return; }
}

inline Color makeColorFromCss(const std::string& css) {
    uint8_t r8 = 255, g8 = 255, b8 = 255, a8 = 255;
    parseCssColor(css, r8, g8, b8, a8);
    Color c;
    c.r = r8 / 255.0f;
    c.g = g8 / 255.0f;
    c.b = b8 / 255.0f;
    c.a = a8 / 255.0f;
    return c;
}

inline void fillTextShadow(DrawGlyphRunData& glyph_run, const dong::dom::ComputedStyle& style) {
    if (style.text_shadow_offset_x != 0.0f || style.text_shadow_offset_y != 0.0f ||
        style.text_shadow_blur != 0.0f || !style.text_shadow_color.empty()) {
        glyph_run.has_text_shadow = true;
        glyph_run.text_shadow_offset_x = style.text_shadow_offset_x;
        glyph_run.text_shadow_offset_y = style.text_shadow_offset_y;
        glyph_run.text_shadow_blur = style.text_shadow_blur;
        if (!style.text_shadow_color.empty()) {
            glyph_run.text_shadow_color = makeColorFromCss(style.text_shadow_color);
        } else {
            glyph_run.text_shadow_color = Color{0.0f, 0.0f, 0.0f, 1.0f};
        }
    }
}

inline std::string collapseWhitespace(const std::string& input) {
    if (input.empty()) return "";

    std::string output;
    output.reserve(input.size());
    bool in_space = false;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
        } else {
            output.push_back(c);
            in_space = false;
        }
    }

    size_t first = output.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = output.find_last_not_of(' ');
    return output.substr(first, last - first + 1);
}

inline std::string toLowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

// Similar to collapseWhitespace, but preserves explicit newlines (\n).
// Used to implement `white-space: pre-line`-like behavior without destroying line breaks.
inline std::string collapseSpacesPreserveNewlines(const std::string& input) {
    if (input.empty()) return "";

    std::string output;
    output.reserve(input.size());

    bool in_space = false;
    for (char c : input) {
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            output.push_back('\n');
            in_space = false;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\v' || c == '\f') {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
            continue;
        }
        output.push_back(c);
        in_space = false;
    }

    return output;
}

} // namespace dong::render::painter_detail
