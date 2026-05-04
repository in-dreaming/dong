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
#include "../../dom/dom/dom_node.hpp"
#include "../../core/string_utils.h"

namespace dong::render::painter_detail {

using dong::collapseWhitespace;

// ============================================================================
// OKLab/OKLCH Color Space Conversion (CSS Color Module Level 4)
// ============================================================================

// Convert OKLCH to OKLab
// L in [0,1], C in [0,0.4], H in [0,360]
inline void oklchToOklab(float L, float C, float H_deg, float& a_out, float& b_out) {
    // Convert H to radians and compute chroma components
    float H_rad = H_deg * (3.14159265358979323846f / 180.0f);
    a_out = C * std::cos(H_rad);
    b_out = C * std::sin(H_rad);
}

// Convert OKLab to XYZ D65
// L in [0,1], a in [-0.4,0.4], b in [-0.4,0.4]
inline void oklabToXyz(float L, float a, float b, float& X, float& Y, float& Z) {
    // Step 1: OKLab to LMS_ (inverse of M2)
    // M2_inv computed from the OKLab spec's M2 matrix
    float l_ = L + 0.3963377774f * a + 0.2158037573f * b;
    float m_ = L - 0.1055613458f * a - 0.0638541728f * b;
    float s_ = L - 0.0894841775f * a - 1.2914855480f * b;

    // Step 2: LMS_ to LMS (cube)
    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    // Step 3: LMS to XYZ D65 (inverse of M1)
    X = +1.2270138511f * l - 0.5577999807f * m + 0.2812561490f * s;
    Y = -0.0405801784f * l + 1.1122568696f * m - 0.0716766787f * s;
    Z = -0.0763812845f * l - 0.4214819784f * m + 1.5861632204f * s;
}

// Convert XYZ D65 to linear sRGB
inline void xyzToLinearSrgb(float X, float Y, float Z, float& R, float& G, float& B) {
    // D65 to linear RGB matrix
    float r_lin = +3.2404542f * X - 1.5371385f * Y - 0.4985314f * Z;
    float g_lin = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    float b_lin = +0.0556434f * X - 0.2040259f * Y + 1.0572252f * Z;

    R = r_lin;
    G = g_lin;
    B = b_lin;
}

// Convert linear sRGB to sRGB (gamma correction)
inline float linearToSrgb(float c) {
    if (c <= 0.0031308f) {
        return 12.92f * c;
    } else {
        return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
    }
}

// Convert OKLab to sRGB (0-255 range)
inline void oklabToSrgb(float L, float a, float b, uint8_t& r8, uint8_t& g8, uint8_t& b8) {
    float X, Y, Z;
    oklabToXyz(L, a, b, X, Y, Z);

    float R_lin, G_lin, B_lin;
    xyzToLinearSrgb(X, Y, Z, R_lin, G_lin, B_lin);

    auto clampToByte = [](float v) -> uint8_t {
        if (v < 0.0f) return 0;
        if (v > 255.0f) return 255;
        return static_cast<uint8_t>(std::round(v));
    };

    r8 = clampToByte(linearToSrgb(R_lin) * 255.0f);
    g8 = clampToByte(linearToSrgb(G_lin) * 255.0f);
    b8 = clampToByte(linearToSrgb(B_lin) * 255.0f);
}

// ============================================================================
// CSS 颜色解析器：支持 #rgb/#rgba/#rrggbb/#rrggbbaa 与 rgb()/rgba() 子集
// 以及 oklab()/oklch() 颜色函数
// ============================================================================
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

    // color-mix must be processed BEFORE whitespace stripping because it needs
    // spaces to parse "in srgb", color names, and percentages correctly.
    {
        // Case-insensitive prefix check for "color-mix("
        std::string css_lower = css;
        std::transform(css_lower.begin(), css_lower.end(), css_lower.begin(),
                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        // Trim leading whitespace
        size_t start = css_lower.find_first_not_of(" \t\n\r");
        if (start != std::string::npos && css_lower.substr(start, 10) == "color-mix(") {
            std::string orig = css.substr(start);
            size_t lparen = orig.find('(');
            size_t rparen = orig.rfind(')');
            if (lparen != std::string::npos && rparen != std::string::npos && rparen > lparen + 1) {
                std::string args = orig.substr(lparen + 1, rparen - lparen - 1);

                // Split by comma, respecting nested parentheses
                std::vector<std::string> parts;
                std::string current;
                int paren_depth = 0;
                for (char c : args) {
                    if (c == '(') { ++paren_depth; current.push_back(c); }
                    else if (c == ')') { --paren_depth; current.push_back(c); }
                    else if (c == ',' && paren_depth == 0) {
                        if (!current.empty()) { parts.push_back(current); current.clear(); }
                    } else { current.push_back(c); }
                }
                if (!current.empty()) parts.push_back(current);

                auto trim = [](std::string s) -> std::string {
                    s.erase(0, s.find_first_not_of(" \t\n\r"));
                    if (!s.empty()) s.erase(s.find_last_not_of(" \t\n\r") + 1);
                    return s;
                };
                auto trimLower = [&](std::string s) -> std::string {
                    s = trim(s);
                    std::transform(s.begin(), s.end(), s.begin(),
                                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                    return s;
                };

                if (parts.size() >= 3) {
                    // Parse "in <colorspace>"
                    std::string colorspace_part = trimLower(parts[0]);
                    if (colorspace_part.substr(0, 3) == "in ") {
                        colorspace_part = trim(colorspace_part.substr(3));
                    }

                    if (colorspace_part == "srgb") {
                        // Parse first color + optional percentage
                        auto parseColorPart = [&](const std::string& part, std::string& color_str, float& pct, float default_pct) {
                            std::string p = trim(part);
                            size_t last_space = p.find_last_of(' ');
                            if (last_space != std::string::npos) {
                                std::string potential_pct = trim(p.substr(last_space + 1));
                                if (!potential_pct.empty() && potential_pct.back() == '%') {
                                    potential_pct.pop_back();
                                    try {
                                        pct = std::stof(potential_pct);
                                        color_str = trim(p.substr(0, last_space));
                                        return;
                                    } catch (...) {}
                                }
                            }
                            color_str = p;
                            pct = default_pct;
                        };

                        std::string color1_str, color2_str;
                        float color1_pct = 50.0f, color2_pct = 50.0f;
                        parseColorPart(parts[1], color1_str, color1_pct, 50.0f);
                        // default for color2 is complement of color1
                        color2_pct = 100.0f - color1_pct;
                        parseColorPart(parts[2], color2_str, color2_pct, color2_pct);

                        // Clamp
                        if (color1_pct < 0.0f) color1_pct = 0.0f;
                        if (color1_pct > 100.0f) color1_pct = 100.0f;
                        if (color2_pct < 0.0f) color2_pct = 0.0f;
                        if (color2_pct > 100.0f) color2_pct = 100.0f;

                        // Normalize
                        float total = color1_pct + color2_pct;
                        if (total > 0.0f) {
                            color1_pct /= total;
                            color2_pct /= total;
                        } else {
                            color1_pct = color2_pct = 0.5f;
                        }

                        uint8_t r1 = 0, g1 = 0, b1 = 0, a1 = 255;
                        uint8_t r2 = 0, g2 = 0, b2 = 0, a2 = 255;
                        parseCssColor(color1_str, r1, g1, b1, a1);
                        parseCssColor(color2_str, r2, g2, b2, a2);

                        float r1f = r1/255.0f, g1f = g1/255.0f, b1f = b1/255.0f, a1f = a1/255.0f;
                        float r2f = r2/255.0f, g2f = g2/255.0f, b2f = b2/255.0f, a2f = a2/255.0f;
                        // Pre-multiplied alpha mixing
                        float r1p = r1f*a1f, g1p = g1f*a1f, b1p = b1f*a1f;
                        float r2p = r2f*a2f, g2p = g2f*a2f, b2p = b2f*a2f;
                        float rp = r1p*color1_pct + r2p*color2_pct;
                        float gp = g1p*color1_pct + g2p*color2_pct;
                        float bp = b1p*color1_pct + b2p*color2_pct;
                        float ap = a1f*color1_pct + a2f*color2_pct;
                        if (ap > 0.0001f) {
                            r = clampToByte(static_cast<int>((rp/ap)*255.0f));
                            g = clampToByte(static_cast<int>((gp/ap)*255.0f));
                            b = clampToByte(static_cast<int>((bp/ap)*255.0f));
                        } else {
                            r = g = b = 0;
                        }
                        a = clampToByte(static_cast<int>(ap*255.0f));
                        return;
                    }
                }
            }
        }
    }

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

    // oklab(L a b) or oklab(L a b / alpha)
    // L: 0-1, a/b: approximately -0.4 to 0.4
    // NOTE: uses original `css` string (before whitespace stripping) because args are space-separated
    if (startsWith(s, "oklab(")) {
        // Re-parse from original css with spaces preserved
        std::string orig_lower = css;
        std::transform(orig_lower.begin(), orig_lower.end(), orig_lower.begin(),
                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        orig_lower.erase(0, orig_lower.find_first_not_of(" \t\n\r"));
        size_t lparen = orig_lower.find('(');
        size_t rparen = orig_lower.rfind(')');
        if (lparen != std::string::npos && rparen != std::string::npos && rparen > lparen + 1) {
            std::string args = orig_lower.substr(lparen + 1, rparen - lparen - 1);

            // Check for slash (alpha separator)
            size_t slash_pos = args.find('/');
            bool has_alpha = (slash_pos != std::string::npos);

            std::string color_args;
            std::string alpha_arg;

            if (has_alpha) {
                color_args = args.substr(0, slash_pos);
                alpha_arg = args.substr(slash_pos + 1);
            } else {
                color_args = args;
                alpha_arg = "1";
            }

            // Parse L, a, b values (comma or space separated)
            std::vector<float> components;
            std::string current;
            for (char c : color_args) {
                if (c == ',' || c == ' ') {
                    if (!current.empty()) {
                        try {
                            components.push_back(std::stof(current));
                        } catch (...) {
                            components.push_back(0.0f);
                        }
                        current.clear();
                    }
                } else {
                    current.push_back(c);
                }
            }
            if (!current.empty()) {
                try {
                    components.push_back(std::stof(current));
                } catch (...) {
                    components.push_back(0.0f);
                }
            }

            if (components.size() >= 3) {
                float L_val = components[0];
                float a_val = components[1];
                float b_val = components[2];

                // Parse alpha
                float alpha_val = 1.0f;
                std::string alpha_trimmed = alpha_arg;
                while (!alpha_trimmed.empty() && std::isspace(static_cast<unsigned char>(alpha_trimmed.back()))) {
                    alpha_trimmed.pop_back();
                }
                alpha_trimmed.erase(0, alpha_trimmed.find_first_not_of(" \t\n\r"));
                if (!alpha_trimmed.empty() && alpha_trimmed.back() == '%') {
                    alpha_trimmed.pop_back();
                    try {
                        alpha_val = std::stof(alpha_trimmed) / 100.0f;
                    } catch (...) {
                        alpha_val = 1.0f;
                    }
                } else if (!alpha_trimmed.empty()) {
                    try {
                        alpha_val = std::stof(alpha_trimmed);
                    } catch (...) {
                        alpha_val = 1.0f;
                    }
                }

                // Clamp values
                L_val = std::max(0.0f, std::min(1.0f, L_val));
                alpha_val = std::max(0.0f, std::min(1.0f, alpha_val));

                // Convert OKLab to sRGB
                oklabToSrgb(L_val, a_val, b_val, r, g, b);
                a = clampToByte(static_cast<int>(alpha_val * 255.0f));
                return;
            }
        }
    }

    // oklch(L C H) or oklch(L C H / alpha)
    // L: 0-1, C: 0-0.4, H: 0-360 degrees
    // NOTE: uses original `css` string (before whitespace stripping) because args are space-separated
    if (startsWith(s, "oklch(")) {
        std::string orig_lower2 = css;
        std::transform(orig_lower2.begin(), orig_lower2.end(), orig_lower2.begin(),
                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        orig_lower2.erase(0, orig_lower2.find_first_not_of(" \t\n\r"));
        size_t lparen = orig_lower2.find('(');
        size_t rparen = orig_lower2.rfind(')');
        if (lparen != std::string::npos && rparen != std::string::npos && rparen > lparen + 1) {
            std::string args = orig_lower2.substr(lparen + 1, rparen - lparen - 1);

            // Check for slash (alpha separator)
            size_t slash_pos = args.find('/');
            bool has_alpha = (slash_pos != std::string::npos);

            std::string color_args;
            std::string alpha_arg;

            if (has_alpha) {
                color_args = args.substr(0, slash_pos);
                alpha_arg = args.substr(slash_pos + 1);
            } else {
                color_args = args;
                alpha_arg = "1";
            }

            // Parse L, C, H values (comma or space separated)
            std::vector<float> components;
            std::string current;
            for (char c : color_args) {
                if (c == ',' || c == ' ') {
                    if (!current.empty()) {
                        try {
                            components.push_back(std::stof(current));
                        } catch (...) {
                            components.push_back(0.0f);
                        }
                        current.clear();
                    }
                } else {
                    current.push_back(c);
                }
            }
            if (!current.empty()) {
                try {
                    components.push_back(std::stof(current));
                } catch (...) {
                    components.push_back(0.0f);
                }
            }

            if (components.size() >= 3) {
                float L_val = components[0];
                float C_val = components[1];
                float H_val = components[2];

                // Parse alpha
                float alpha_val = 1.0f;
                std::string alpha_trimmed = alpha_arg;
                while (!alpha_trimmed.empty() && std::isspace(static_cast<unsigned char>(alpha_trimmed.back()))) {
                    alpha_trimmed.pop_back();
                }
                alpha_trimmed.erase(0, alpha_trimmed.find_first_not_of(" \t\n\r"));
                if (!alpha_trimmed.empty() && alpha_trimmed.back() == '%') {
                    alpha_trimmed.pop_back();
                    try {
                        alpha_val = std::stof(alpha_trimmed) / 100.0f;
                    } catch (...) {
                        alpha_val = 1.0f;
                    }
                } else if (!alpha_trimmed.empty()) {
                    try {
                        alpha_val = std::stof(alpha_trimmed);
                    } catch (...) {
                        alpha_val = 1.0f;
                    }
                }

                // Clamp values
                L_val = std::max(0.0f, std::min(1.0f, L_val));
                C_val = std::max(0.0f, C_val);  // Chroma is never negative
                alpha_val = std::max(0.0f, std::min(1.0f, alpha_val));

                // Normalize hue to [0, 360)
                H_val = std::fmod(H_val, 360.0f);
                if (H_val < 0.0f) H_val += 360.0f;

                // Convert OKLCH to OKLab
                float a_val, b_val;
                oklchToOklab(L_val, C_val, H_val, a_val, b_val);

                // Convert OKLab to sRGB
                oklabToSrgb(L_val, a_val, b_val, r, g, b);
                a = clampToByte(static_cast<int>(alpha_val * 255.0f));
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
    if (s == "lightpink")  { r = 255; g = 182; b = 193; a = 255; return; }
    if (s == "lightgreen") { r = 144; g = 238; b = 144; a = 255; return; }
    if (s == "lightblue")  { r = 173; g = 216; b = 230; a = 255; return; }
    if (s == "lightyellow"){ r = 255; g = 255; b = 224; a = 255; return; }
    if (s == "lightcoral") { r = 240; g = 128; b = 128; a = 255; return; }
    if (s == "lightsalmon"){ r = 255; g = 160; b = 122; a = 255; return; }
    if (s == "yellow")     { r = 255; g = 255; b = 0;   a = 255; return; }
    if (s == "orange")     { r = 255; g = 165; b = 0;   a = 255; return; }
    if (s == "pink")       { r = 255; g = 192; b = 203; a = 255; return; }
    if (s == "purple")     { r = 128; g = 0;   b = 128; a = 255; return; }
    if (s == "cyan" || s == "aqua") { r = 0; g = 255; b = 255; a = 255; return; }
    if (s == "magenta" || s == "fuchsia") { r = 255; g = 0; b = 255; a = 255; return; }
    if (s == "silver")     { r = g = b = 192; a = 255; return; }
    if (s == "maroon")     { r = 128; g = 0;   b = 0;   a = 255; return; }
    if (s == "olive")      { r = 128; g = 128; b = 0;   a = 255; return; }
    if (s == "lime")       { r = 0;   g = 255; b = 0;   a = 255; return; }
    if (s == "teal")       { r = 0;   g = 128; b = 128; a = 255; return; }
    if (s == "navy")       { r = 0;   g = 0;   b = 128; a = 255; return; }
    if (s == "darkblue")   { r = 0;   g = 0;   b = 139; a = 255; return; }
    if (s == "darkgray" || s == "darkgrey") { r = g = b = 169; a = 255; return; }
    if (s == "darkgreen")  { r = 0;   g = 100; b = 0;   a = 255; return; }
    if (s == "darkred")    { r = 139; g = 0;   b = 0;   a = 255; return; }
    if (s == "gold")       { r = 255; g = 215; b = 0;   a = 255; return; }
    if (s == "coral")      { r = 255; g = 127; b = 80;  a = 255; return; }
    if (s == "crimson")    { r = 220; g = 20;  b = 60;  a = 255; return; }
    if (s == "indigo")     { r = 75;  g = 0;   b = 130; a = 255; return; }
    if (s == "lavender")   { r = 230; g = 230; b = 250; a = 255; return; }
    if (s == "ivory")      { r = 255; g = 255; b = 240; a = 255; return; }
    if (s == "khaki")      { r = 240; g = 230; b = 140; a = 255; return; }
    if (s == "brown")      { r = 165; g = 42;  b = 42;  a = 255; return; }
    if (s == "beige")      { r = 245; g = 245; b = 220; a = 255; return; }
    if (s == "tan")        { r = 210; g = 180; b = 140; a = 255; return; }
    if (s == "salmon")     { r = 250; g = 128; b = 114; a = 255; return; }
    if (s == "tomato")     { r = 255; g = 99;  b = 71;  a = 255; return; }
    if (s == "orchid")     { r = 218; g = 112; b = 214; a = 255; return; }
    if (s == "plum")       { r = 221; g = 160; b = 221; a = 255; return; }
    if (s == "violet")     { r = 238; g = 130; b = 238; a = 255; return; }
    if (s == "wheat")      { r = 245; g = 222; b = 179; a = 255; return; }

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

inline void fillTextShadow(DrawGlyphRunData& glyph_run, const dong::dom::ComputedStyle& style, const dong::dom::DOMNodePtr& node = nullptr) {
    // Check for ::selection pseudo-element styles first
    if (node) {
        auto selection_pseudo = node->getPseudoSelection();
        if (selection_pseudo && (
            selection_pseudo->getComputedStyle().text_shadow_offset_x != 0.0f ||
            selection_pseudo->getComputedStyle().text_shadow_offset_y != 0.0f ||
            selection_pseudo->getComputedStyle().text_shadow_blur != 0.0f ||
            !selection_pseudo->getComputedStyle().text_shadow_color.empty())) {

            glyph_run.has_text_shadow = true;
            glyph_run.text_shadow_offset_x = selection_pseudo->getComputedStyle().text_shadow_offset_x;
            glyph_run.text_shadow_offset_y = selection_pseudo->getComputedStyle().text_shadow_offset_y;
            glyph_run.text_shadow_blur = selection_pseudo->getComputedStyle().text_shadow_blur;
            if (!selection_pseudo->getComputedStyle().text_shadow_color.empty()) {
                glyph_run.text_shadow_color = makeColorFromCss(selection_pseudo->getComputedStyle().text_shadow_color);
            } else {
                glyph_run.text_shadow_color = Color{0.0f, 0.0f, 0.0f, 1.0f};
            }
            return;
        }
    }

    // Fall back to regular style if no ::selection pseudo-element or no text-shadow in it
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

// Use shared toLower from string_utils.h
using dong::toLower;
inline std::string toLowerCopy(const std::string& s) { return toLower(s); }

// Delegate to shared implementation in string_utils.h
using dong::collapseSpacesPreserveNewlines;

} // namespace dong::render::painter_detail
