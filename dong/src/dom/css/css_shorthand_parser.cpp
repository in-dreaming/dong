#include "css_parser.hpp"

#include <cctype>
#include <sstream>
#include <vector>

namespace dong::dom {

void CSSParser::parseMarginShorthand(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        auto v = parseValue(parts[0]);
        style.margin_top = style.margin_right = style.margin_bottom = style.margin_left = v;
    } else if (parts.size() == 2) {
        style.margin_top = style.margin_bottom = parseValue(parts[0]);
        style.margin_left = style.margin_right = parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.margin_top = parseValue(parts[0]);
        style.margin_left = style.margin_right = parseValue(parts[1]);
        style.margin_bottom = parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.margin_top = parseValue(parts[0]);
        style.margin_right = parseValue(parts[1]);
        style.margin_bottom = parseValue(parts[2]);
        style.margin_left = parseValue(parts[3]);
    }
}

void CSSParser::parsePaddingShorthand(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        auto v = parseValue(parts[0]);
        style.padding_top = style.padding_right = style.padding_bottom = style.padding_left = v;
    } else if (parts.size() == 2) {
        style.padding_top = style.padding_bottom = parseValue(parts[0]);
        style.padding_left = style.padding_right = parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.padding_top = parseValue(parts[0]);
        style.padding_left = style.padding_right = parseValue(parts[1]);
        style.padding_bottom = parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.padding_top = parseValue(parts[0]);
        style.padding_right = parseValue(parts[1]);
        style.padding_bottom = parseValue(parts[2]);
        style.padding_left = parseValue(parts[3]);
    }
}

void CSSParser::parseBorderShorthand(const std::string& value, ComputedStyle& style) {
    // Split by spaces but respect parentheses (for rgba, rgb, etc.)
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;
    
    for (size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        if (c == '(') {
            ++paren_depth;
            current += c;
        } else if (c == ')') {
            --paren_depth;
            current += c;
        } else if (c == ' ' && paren_depth == 0) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    
    for (const auto& part : parts) {
        if (part.empty()) continue;
        if (std::isdigit(static_cast<unsigned char>(part[0])) || part[0] == '.') {
            style.border_width = parseFloat(part);
        } else if (part == "solid" || part == "dashed" || part == "dotted" || 
                   part == "double" || part == "none" || part == "hidden") {
            style.border_style = part;
        } else {
            style.border_color = parseColor(part);
        }
    }
}

void CSSParser::parseBorderRadiusShorthand(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        CSSValue v = parseValue(parts[0]);
        style.border_radius = (v.unit == CSSValue::Unit::PIXEL) ? v.value : v.value;
        style.border_top_left_radius = v;
        style.border_top_right_radius = v;
        style.border_bottom_left_radius = v;
        style.border_bottom_right_radius = v;
    } else if (parts.size() == 2) {
        style.border_top_left_radius = style.border_bottom_right_radius = parseValue(parts[0]);
        style.border_top_right_radius = style.border_bottom_left_radius = parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.border_top_left_radius = parseValue(parts[0]);
        style.border_top_right_radius = style.border_bottom_left_radius = parseValue(parts[1]);
        style.border_bottom_right_radius = parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.border_top_left_radius = parseValue(parts[0]);
        style.border_top_right_radius = parseValue(parts[1]);
        style.border_bottom_right_radius = parseValue(parts[2]);
        style.border_bottom_left_radius = parseValue(parts[3]);
    }
}

void CSSParser::parseFlexShorthand(const std::string& value, ComputedStyle& style) {
    if (value == "none") {
        style.flex_grow = 0.0f;
        style.flex_shrink = 0.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);
        return;
    }
    if (value == "auto") {
        style.flex_grow = 1.0f;
        style.flex_shrink = 1.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);
        return;
    }
    
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        style.flex_grow = parseFloat(parts[0]);
        style.flex_shrink = 1.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::PIXEL);
    } else if (parts.size() == 2) {
        style.flex_grow = parseFloat(parts[0]);
        if (parts[1].find('%') != std::string::npos || parts[1].find("px") != std::string::npos) {
            style.flex_basis = parseValue(parts[1]);
        } else {
            style.flex_shrink = parseFloat(parts[1]);
        }
    } else if (parts.size() >= 3) {
        style.flex_grow = parseFloat(parts[0]);
        style.flex_shrink = parseFloat(parts[1]);
        style.flex_basis = parseValue(parts[2]);
    }
    
    style.flex = style.flex_grow;
}

void CSSParser::parseBackgroundShorthand(const std::string& value, ComputedStyle& style) {
    if (value.find("gradient") != std::string::npos) {
        style.background_gradients.push_back(parseGradient(value));
    } else if (value.find("url(") != std::string::npos) {
        size_t url_start = value.find("url(");
        size_t url_end = value.find(")", url_start);
        if (url_end != std::string::npos) {
            style.background_image = value.substr(url_start, url_end - url_start + 1);
        }
    } else {
        style.background_color = parseColor(value);
    }
}

void CSSParser::parseFontShorthand(const std::string& value, ComputedStyle& style) {
    // Simplified font shorthand parsing
    std::istringstream iss(value);
    std::string part;
    std::vector<std::string> parts;
    while (iss >> part) parts.push_back(part);
    
    for (size_t i = 0; i < parts.size(); ++i) {
        const std::string& p = parts[i];
        if (p == "italic" || p == "oblique") {
            style.font_style = p;
        } else if (p == "bold" || p == "bolder" || p == "lighter" ||
                   (std::isdigit(static_cast<unsigned char>(p[0])) && p.find("px") == std::string::npos)) {
            style.font_weight = p;
        } else if (p.find("px") != std::string::npos || p.find("em") != std::string::npos ||
                   p.find("rem") != std::string::npos || p.find('%') != std::string::npos) {
            // Check for line-height
            size_t slash = p.find('/');
            if (slash != std::string::npos) {
                style.font_size = parseFloat(p.substr(0, slash));
                std::string lh = p.substr(slash + 1);
                style.has_line_height = true;
                if (lh.find("px") != std::string::npos) {
                    style.line_height = parseFloat(lh);
                    style.line_height_is_unitless = false;
                } else {
                    style.line_height = parseFloat(lh);
                    style.line_height_is_unitless = true;
                }

            } else {
                style.font_size = parseFloat(p);
            }
        } else {
            // Assume it's font-family (rest of the string)
            std::string family;
            for (size_t j = i; j < parts.size(); ++j) {
                if (!family.empty()) family += " ";
                family += parts[j];
            }
            style.font_family = family;
            break;
        }
    }
}

} // namespace dong::dom
