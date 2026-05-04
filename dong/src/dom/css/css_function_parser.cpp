#include "css_parser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>

namespace dong::dom {

std::string CSSParser::parseColor(const std::string& value) {
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) v = v.substr(0, last + 1);
    
    // Check for light-dark() function
    if (v.find("light-dark(") == 0) {
        // Parse light-dark(light-color, dark-color) function
        size_t start = v.find('(');
        size_t end = v.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string args = v.substr(start + 1, end - start - 1);

            // Split arguments by comma (handling nested parentheses)
            std::vector<std::string> parts;
            std::string current;
            int paren_depth = 0;

            for (char c : args) {
                if (c == '(') paren_depth++;
                else if (c == ')') paren_depth--;

                if (c == ',' && paren_depth == 0) {
                    std::string trimmed = current;
                    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
                    size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
                    if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
                    if (!trimmed.empty()) parts.push_back(trimmed);
                    current.clear();
                } else {
                    current += c;
                }
            }

            if (!current.empty()) {
                std::string trimmed = current;
                trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
                size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
                if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
                if (!trimmed.empty()) parts.push_back(trimmed);
            }

            // Should have exactly 2 arguments: light-color and dark-color
            if (parts.size() == 2) {
                // Return the light-dark function as-is for later resolution
                return v;
            }
        }
    }

    // Already a valid color format (including oklab/oklch)
    if (v[0] == '#' || v.find("rgb") == 0 || v.find("hsl") == 0 || v.find("color-mix") == 0 ||
        v.find("oklab") == 0 || v.find("oklch") == 0) {
        return v;
    }
    
    // Named colors (common ones)
    static const std::unordered_map<std::string, std::string> named_colors = {
        {"transparent", "transparent"},
        {"black", "#000000"},
        {"white", "#ffffff"},
        {"red", "#ff0000"},
        {"green", "#008000"},
        {"blue", "#0000ff"},
        {"yellow", "#ffff00"},
        {"cyan", "#00ffff"},
        {"magenta", "#ff00ff"},
        {"gray", "#808080"},
        {"grey", "#808080"},
        {"silver", "#c0c0c0"},
        {"maroon", "#800000"},
        {"olive", "#808000"},
        {"lime", "#00ff00"},
        {"aqua", "#00ffff"},
        {"teal", "#008080"},
        {"navy", "#000080"},
        {"fuchsia", "#ff00ff"},
        {"purple", "#800080"},
        {"orange", "#ffa500"},
        {"pink", "#ffc0cb"},
        {"brown", "#a52a2a"},
        {"coral", "#ff7f50"},
        {"crimson", "#dc143c"},
        {"gold", "#ffd700"},
        {"indigo", "#4b0082"},
        {"ivory", "#fffff0"},
        {"khaki", "#f0e68c"},
        {"lavender", "#e6e6fa"},
        {"lightblue", "#add8e6"},
        {"lightgray", "#d3d3d3"},
        {"lightgreen", "#90ee90"},
        {"lightyellow", "#ffffe0"},
        {"lightpink", "#ffb6c1"},
        {"lightcoral", "#f08080"},
        {"lightsalmon", "#ffa07a"},
        {"lightseagreen", "#20b2aa"},
        {"lightskyblue", "#87cefa"},
        {"lightslategray", "#778899"},
        {"lightslategrey", "#778899"},
        {"lightsteelblue", "#b0c4de"},
        {"darkblue", "#00008b"},
        {"darkgray", "#a9a9a9"},
        {"darkgreen", "#006400"},
        {"darkred", "#8b0000"},
    };
    
    auto it = named_colors.find(v);
    if (it != named_colors.end()) {
        return it->second;
    }
    
    return value;
}

CSSGradient CSSParser::parseGradient(const std::string& value) {
    CSSGradient gradient;
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    if (v.find("repeating-linear-gradient") == 0) {
        gradient.type = CSSGradient::Type::REPEATING_LINEAR;
    } else if (v.find("repeating-radial-gradient") == 0) {
        gradient.type = CSSGradient::Type::REPEATING_RADIAL;
    } else if (v.find("linear-gradient") == 0) {
        gradient.type = CSSGradient::Type::LINEAR;
    } else if (v.find("radial-gradient") == 0) {
        gradient.type = CSSGradient::Type::RADIAL;
    } else if (v.find("conic-gradient") == 0) {
        gradient.type = CSSGradient::Type::CONIC;
    }
    
    // Extract content inside parentheses
    size_t start = v.find('(');
    size_t end = v.rfind(')');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return gradient;
    }
    
    std::string content = v.substr(start + 1, end - start - 1);
    
    // Parse gradient stops and direction
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;
    
    for (char c : content) {
        if (c == '(') {
            ++paren_depth;
            current += c;
        } else if (c == ')') {
            --paren_depth;
            current += c;
        } else if (c == ',' && paren_depth == 0) {
            std::string trimmed = current;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
            size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
            if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
            if (!trimmed.empty()) {
                parts.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        std::string trimmed = current;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
        if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
        if (!trimmed.empty()) {
            parts.push_back(trimmed);
        }
    }
    
    // Parse direction/angle (first part if it's not a color)
    size_t color_start = 0;
    if (!parts.empty()) {
        const std::string& first = parts[0];
        if (first.find("deg") != std::string::npos) {
            gradient.angle = parseFloat(first);
            color_start = 1;
        } else if (first.find("to ") == 0) {
            // Parse direction keywords
            if (first.find("right") != std::string::npos) gradient.angle = 90.0f;
            else if (first.find("left") != std::string::npos) gradient.angle = 270.0f;
            else if (first.find("bottom") != std::string::npos) gradient.angle = 180.0f;
            else if (first.find("top") != std::string::npos) gradient.angle = 0.0f;
            color_start = 1;
        }
    }
    
    // Parse color stops
    float auto_position = 0.0f;
    float auto_step = parts.size() > color_start + 1 ? 
                      1.0f / (parts.size() - color_start - 1) : 1.0f;
    
    for (size_t i = color_start; i < parts.size(); ++i) {
        GradientStop stop;
        const std::string& part = parts[i];
        
        // Check for position
        size_t percent_pos = part.rfind('%');
        if (percent_pos != std::string::npos) {
            // Find where position starts
            size_t pos_start = part.rfind(' ', percent_pos);
            if (pos_start != std::string::npos) {
                stop.color = parseColor(part.substr(0, pos_start));
                stop.position = parseFloat(part.substr(pos_start)) / 100.0f;
            } else {
                stop.position = parseFloat(part) / 100.0f;
            }
        } else {
            stop.color = parseColor(part);
            stop.position = auto_position;
            auto_position += auto_step;
        }
        
        if (stop.color.empty()) {
            stop.color = part;
        }
        
        gradient.stops.push_back(stop);
    }
    
    return gradient;
}

std::vector<CSSFilter> CSSParser::parseFilter(const std::string& value) {
    std::vector<CSSFilter> filters;
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    if (v == "none") {
        return filters;
    }
    
    size_t pos = 0;
    while (pos < v.size()) {
        size_t lparen = v.find('(', pos);
        if (lparen == std::string::npos) break;
        size_t rparen = v.find(')', lparen + 1);
        if (rparen == std::string::npos) break;
        
        std::string func_name = v.substr(pos, lparen - pos);
        func_name.erase(0, func_name.find_first_not_of(" \t\n\r"));
        size_t last = func_name.find_last_not_of(" \t\n\r");
        if (last != std::string::npos) func_name = func_name.substr(0, last + 1);
        
        std::string args = v.substr(lparen + 1, rparen - lparen - 1);
        
        CSSFilter filter;
        
        if (func_name == "blur") {
            filter.type = CSSFilter::Type::BLUR;
            filter.value = parseFloat(args);
        } else if (func_name == "brightness") {
            filter.type = CSSFilter::Type::BRIGHTNESS;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "contrast") {
            filter.type = CSSFilter::Type::CONTRAST;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "grayscale") {
            filter.type = CSSFilter::Type::GRAYSCALE;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "hue-rotate") {
            filter.type = CSSFilter::Type::HUE_ROTATE;
            filter.value = parseFloat(args);
        } else if (func_name == "invert") {
            filter.type = CSSFilter::Type::INVERT;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "opacity") {
            filter.type = CSSFilter::Type::OPACITY;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "saturate") {
            filter.type = CSSFilter::Type::SATURATE;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "sepia") {
            filter.type = CSSFilter::Type::SEPIA;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "drop-shadow") {
            filter.type = CSSFilter::Type::DROP_SHADOW;
            // Parse drop-shadow arguments
            std::istringstream iss(args);
            std::string part;
            std::vector<std::string> parts;
            while (iss >> part) {
                parts.push_back(part);
            }
            if (parts.size() >= 2) {
                filter.offset_x = parseFloat(parts[0]);
                filter.offset_y = parseFloat(parts[1]);
            }
            if (parts.size() >= 3) {
                filter.blur = parseFloat(parts[2]);
            }
            if (parts.size() >= 4) {
                filter.color = parseColor(parts[3]);
            }
        }
        
        filters.push_back(filter);
        pos = rparen + 1;
    }
    
    return filters;
}

} // namespace dong::dom
