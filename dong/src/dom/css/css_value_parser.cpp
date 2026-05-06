#include "css_parser.hpp"
#include "css_parser_internal.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace dong::dom {

CSSValue CSSParser::parseValue(const std::string& value) {
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) v = v.substr(0, last + 1);

    if (v == "auto") {
        return CSSValue(0.0f, CSSValue::Unit::AUTO);
    }
    if (v == "inherit") {
        return CSSValue(0.0f, CSSValue::Unit::INHERIT);
    }
    if (v == "content") {
        return CSSValue(0.0f, CSSValue::Unit::CONTENT);
    }
    
    // Check for calc()
    if (v.find("calc(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string expr = v.substr(5, end - 5);
            auto calc_expr = parseCalc(expr);
            if (calc_expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = calc_expr;
                return result;
            }
        }
    }
    
    // Check for min()
    if (v.find("min(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(4, end - 4);
            auto expr = parseMinMaxClamp("min", args);
            if (expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = expr;
                return result;
            }
        }
    }
    
    // Check for max()
    if (v.find("max(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(4, end - 4);
            auto expr = parseMinMaxClamp("max", args);
            if (expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = expr;
                return result;
            }
        }
    }
    
    // Check for clamp()
    if (v.find("clamp(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(6, end - 6);
            auto expr = parseMinMaxClamp("clamp", args);
            if (expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = expr;
                return result;
            }
        }
    }

    // Check for env()
    if (v.find("env(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(4, end - 4);

            // Parse env() arguments: env(name) or env(name, fallback)
            size_t comma_pos = args.find(',');
            std::string env_name;
            std::string fallback_value;

            if (comma_pos != std::string::npos) {
                // env(name, fallback)
                env_name = args.substr(0, comma_pos);
                fallback_value = args.substr(comma_pos + 1);

                // Trim whitespace
                env_name.erase(0, env_name.find_first_not_of(" \t\n\r"));
                env_name.erase(env_name.find_last_not_of(" \t\n\r") + 1);
                fallback_value.erase(0, fallback_value.find_first_not_of(" \t\n\r"));
                fallback_value.erase(fallback_value.find_last_not_of(" \t\n\r") + 1);
            } else {
                // env(name)
                env_name = args;
                env_name.erase(0, env_name.find_first_not_of(" \t\n\r"));
                env_name.erase(env_name.find_last_not_of(" \t\n\r") + 1);
            }

            // Create ENV value
            CSSValue result;
            result.unit = CSSValue::Unit::ENV;
            result.env_name = env_name;

            // Parse fallback value if provided
            if (!fallback_value.empty()) {
                result.env_fallback = std::make_shared<CSSValue>(parseValue(fallback_value));
            }

            return result;
        }
    }

    float num = parseFloat(v);
    
    if (v.find('%') != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::PERCENT);
    }
    if (v.find("rem") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::REM);
    }
    if (v.find("em") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::EM);
    }
    if (v.find("vw") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VW);
    }
    if (v.find("vh") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VH);
    }
    if (v.find("vmin") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VMIN);
    }
    if (v.find("vmax") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VMAX);
    }
    if (v.find("ch") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::CH);
    }
    
    return CSSValue(num, CSSValue::Unit::PIXEL);
}

// Parse calc() expression
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalc(const std::string& expr) {
    size_t pos = 0;
    return parseCalcExpression(expr, pos);
}

// Parse min()/max()/clamp() expressions
std::shared_ptr<CSSCalcExpression> CSSParser::parseMinMaxClamp(const std::string& func, const std::string& args) {
    auto result = std::make_shared<CSSCalcExpression>();
    
    if (func == "min") {
        result->op = CSSCalcExpression::Op::MIN;
    } else if (func == "max") {
        result->op = CSSCalcExpression::Op::MAX;
    } else if (func == "clamp") {
        result->op = CSSCalcExpression::Op::CLAMP;
    } else {
        return nullptr;
    }
    
    // Split arguments by comma (respecting parentheses)
    std::vector<std::string> parts;
    std::string current;
    int depth = 0;
    
    for (char c : args) {
        if (c == '(') {
            ++depth;
            current += c;
        } else if (c == ')') {
            --depth;
            current += c;
        } else if (c == ',' && depth == 0) {
            // Trim and add
            size_t start = current.find_first_not_of(" \t\n\r");
            size_t end = current.find_last_not_of(" \t\n\r");
            if (start != std::string::npos) {
                parts.push_back(current.substr(start, end - start + 1));
            }
            current.clear();
        } else {
            current += c;
        }
    }
    
    // Add last part
    size_t start = current.find_first_not_of(" \t\n\r");
    size_t end = current.find_last_not_of(" \t\n\r");
    if (start != std::string::npos) {
        parts.push_back(current.substr(start, end - start + 1));
    }
    
    // Parse each argument
    for (const auto& part : parts) {
        // Check if it's a nested calc/min/max/clamp
        CSSValue val = parseValue(part);
        
        auto child = std::make_shared<CSSCalcExpression>();
        if (val.isCalc() && val.calc_expr) {
            child = val.calc_expr;
        } else {
            child->op = CSSCalcExpression::Op::VALUE;
            child->value = val;
        }
        result->children.push_back(child);
    }
    
    return result;
}

// Parse calc expression (handles + and -)
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalcExpression(const std::string& expr, size_t& pos) {
    auto left = parseCalcTerm(expr, pos);
    if (!left) return nullptr;
    
    while (pos < expr.size()) {
        // Skip whitespace
        while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos]))) ++pos;
        
        if (pos >= expr.size()) break;
        
        char op = expr[pos];
        if (op != '+' && op != '-') break;
        
        // Check for whitespace around operator (required by CSS calc spec)
        if (pos > 0 && !std::isspace(static_cast<unsigned char>(expr[pos - 1]))) break;
        if (pos + 1 < expr.size() && !std::isspace(static_cast<unsigned char>(expr[pos + 1]))) break;
        
        ++pos;
        
        auto right = parseCalcTerm(expr, pos);
        if (!right) break;
        
        auto combined = std::make_shared<CSSCalcExpression>();
        combined->op = (op == '+') ? CSSCalcExpression::Op::ADD : CSSCalcExpression::Op::SUBTRACT;
        combined->children.push_back(left);
        combined->children.push_back(right);
        left = combined;
    }
    
    return left;
}

// Parse calc term (handles * and /)
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalcTerm(const std::string& expr, size_t& pos) {
    auto left = parseCalcFactor(expr, pos);
    if (!left) return nullptr;
    
    while (pos < expr.size()) {
        // Skip whitespace
        while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos]))) ++pos;
        
        if (pos >= expr.size()) break;
        
        char op = expr[pos];
        if (op != '*' && op != '/') break;
        
        ++pos;
        
        auto right = parseCalcFactor(expr, pos);
        if (!right) break;
        
        auto combined = std::make_shared<CSSCalcExpression>();
        combined->op = (op == '*') ? CSSCalcExpression::Op::MULTIPLY : CSSCalcExpression::Op::DIVIDE;
        combined->children.push_back(left);
        combined->children.push_back(right);
        left = combined;
    }
    
    return left;
}

// Parse calc factor (value or parenthesized expression)
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalcFactor(const std::string& expr, size_t& pos) {
    // Skip whitespace
    while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos]))) ++pos;

    if (pos >= expr.size()) return nullptr;

    // Check for parenthesized expression
    if (expr[pos] == '(') {
        ++pos;
        auto inner = parseCalcExpression(expr, pos);
        // Skip closing paren
        while (pos < expr.size() && expr[pos] != ')') ++pos;
        if (pos < expr.size()) ++pos;
        return inner;
    }

    // Check for env() inside calc
    if (pos + 4 <= expr.size() && expr.substr(pos, 4) == "env(") {
        // Find matching closing paren (depth-aware)
        size_t start = pos;
        int depth = 0;
        while (pos < expr.size()) {
            if (expr[pos] == '(') ++depth;
            else if (expr[pos] == ')') {
                --depth;
                if (depth == 0) { ++pos; break; }
            }
            ++pos;
        }
        std::string env_token = expr.substr(start, pos - start);
        CSSValue val = parseValue(env_token);
        auto result = std::make_shared<CSSCalcExpression>();
        result->op = CSSCalcExpression::Op::VALUE;
        result->value = val;
        return result;
    }

    // Parse value with unit
    size_t start = pos;
    bool has_digit = false;

    // Handle negative sign
    if (pos < expr.size() && expr[pos] == '-') ++pos;

    // Parse number
    while (pos < expr.size() && (std::isdigit(static_cast<unsigned char>(expr[pos])) || expr[pos] == '.')) {
        has_digit = true;
        ++pos;
    }

    if (!has_digit) return nullptr;

    // Parse unit
    size_t unit_start = pos;
    while (pos < expr.size() && std::isalpha(static_cast<unsigned char>(expr[pos]))) ++pos;
    if (pos < expr.size() && expr[pos] == '%') ++pos;

    std::string value_str = expr.substr(start, pos - start);
    CSSValue val = parseValue(value_str);

    auto result = std::make_shared<CSSCalcExpression>();
    result->op = CSSCalcExpression::Op::VALUE;
    result->value = val;
    return result;
}



float CSSParser::parseFloat(const std::string& s) {
    std::string v = s;
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos && last + 1 < v.size()) {
        v.erase(last + 1);
    }
    
    size_t i = 0;
    while (i < v.size() && (std::isdigit(static_cast<unsigned char>(v[i])) || 
           v[i] == '-' || v[i] == '+' || v[i] == '.')) {
        ++i;
    }
    if (i == 0) return 0.0f;
    try {
        return std::stof(v.substr(0, i));
    } catch (...) {
        return 0.0f;
    }
}


std::vector<CSSTransition> CSSParser::parseTransition(const std::string& value) {
    std::vector<CSSTransition> transitions;
    
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    if (v == "none") {
        return transitions;
    }
    
    std::vector<std::string> parts = css_parser_internal::splitTopLevel(v, ',');
    
    for (const auto& part : parts) {
        CSSTransition transition;
        
        std::istringstream iss(part);
        std::string token;
        std::vector<std::string> tokens;
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        for (const auto& t : tokens) {
            if (t.find('s') != std::string::npos && t.find("ease") == std::string::npos) {
                float time = parseFloat(t);
                if (t.find("ms") != std::string::npos) {
                    time /= 1000.0f;
                }
                if (transition.duration == 0.0f) {
                    transition.duration = time;
                } else {
                    transition.delay = time;
                }
            } else if (t == "ease" || t == "ease-in" || t == "ease-out" || 
                       t == "ease-in-out" || t == "linear" || 
                       t.find("cubic-bezier") == 0 || t.find("steps") == 0) {
                transition.timing_function = t;
            } else {
                transition.property = t;
            }
        }
        
        transitions.push_back(transition);
    }
    
    return transitions;
}


CSSAnimation CSSParser::parseAnimation(const std::string& value) {
    CSSAnimation animation;
    
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    std::istringstream iss(v);
    std::string token;
    std::vector<std::string> tokens;
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    for (const auto& t : tokens) {
        if (t.find('s') != std::string::npos && 
            t.find("ease") == std::string::npos &&
            t.find("forwards") == std::string::npos &&
            t.find("backwards") == std::string::npos) {
            float time = parseFloat(t);
            if (t.find("ms") != std::string::npos) {
                time /= 1000.0f;
            }
            if (animation.duration == 0.0f) {
                animation.duration = time;
            } else {
                animation.delay = time;
            }
        } else if (t == "ease" || t == "ease-in" || t == "ease-out" || 
                   t == "ease-in-out" || t == "linear" ||
                   t.find("cubic-bezier") == 0 || t.find("steps") == 0) {
            animation.timing_function = t;
        } else if (t == "infinite") {
            animation.iteration_count = -1;
        } else if (t == "normal" || t == "reverse" || 
                   t == "alternate" || t == "alternate-reverse") {
            animation.direction = t;
        } else if (t == "none" || t == "forwards" || 
                   t == "backwards" || t == "both") {
            animation.fill_mode = t;
        } else if (t == "running" || t == "paused") {
            animation.play_state = t;
        } else if (std::isdigit(static_cast<unsigned char>(t[0]))) {
            animation.iteration_count = static_cast<int>(parseFloat(t));
        } else {
            animation.name = t;
        }
    }
    
    return animation;
}


void CSSParser::parseBoxShadow(const std::string& value, ComputedStyle& style) {
    style.box_shadows.clear();
    
    if (value == "none") {
        return;
    }
    
    std::vector<std::string> shadow_parts = css_parser_internal::splitTopLevel(value, ',');
    
    for (const auto& part : shadow_parts) {
        BoxShadow shadow;
        
        // Check for inset
        std::string remaining = part;
        if (remaining.find("inset") != std::string::npos) {
            shadow.inset = true;
            size_t inset_pos = remaining.find("inset");
            remaining = remaining.substr(0, inset_pos) + remaining.substr(inset_pos + 5);
        }
        
        // Parse lengths and color
        std::vector<float> lengths;
        std::string color;
        
        size_t i = 0;
        while (i < remaining.size()) {
            while (i < remaining.size() && std::isspace(static_cast<unsigned char>(remaining[i]))) ++i;
            if (i >= remaining.size()) break;
            
            size_t token_start = i;
            while (i < remaining.size() && !std::isspace(static_cast<unsigned char>(remaining[i]))) ++i;
            std::string token = remaining.substr(token_start, i - token_start);
            
            if (!token.empty() && (token[0] == '#' || std::isalpha(static_cast<unsigned char>(token[0])))) {
                // Color - rest of the string is color
                color = remaining.substr(token_start);
                color.erase(0, color.find_first_not_of(" \t\n\r"));
                size_t last = color.find_last_not_of(" \t\n\r");
                if (last != std::string::npos) color = color.substr(0, last + 1);
                break;
            }
            lengths.push_back(parseFloat(token));
        }
        
        if (lengths.size() >= 2) {
            shadow.offset_x = lengths[0];
            shadow.offset_y = lengths[1];
        }
        if (lengths.size() >= 3) {
            shadow.blur_radius = lengths[2];
        }
        if (lengths.size() >= 4) {
            shadow.spread_radius = lengths[3];
        }
        if (!color.empty()) {
            shadow.color = parseColor(color);
        }
        
        style.box_shadows.push_back(shadow);
    }
}


void CSSParser::parseTextShadow(const std::string& value, ComputedStyle& style) {
    if (value == "none") {
        style.text_shadow_offset_x = 0.0f;
        style.text_shadow_offset_y = 0.0f;
        style.text_shadow_blur = 0.0f;
        style.text_shadow_color.clear();
        return;
    }
    
    std::istringstream iss(value);
    std::string part;
    std::vector<std::string> parts;
    while (iss >> part) parts.push_back(part);
    
    std::vector<float> lengths;
    std::string color;
    
    for (const auto& p : parts) {
        if (!p.empty() && (std::isdigit(static_cast<unsigned char>(p[0])) || p[0] == '-' || p[0] == '.')) {
            lengths.push_back(parseFloat(p));
        } else if (!p.empty() && (p[0] == '#' || std::isalpha(static_cast<unsigned char>(p[0])))) {
            color = p;
        }
    }
    
    if (lengths.size() >= 2) {
        style.text_shadow_offset_x = lengths[0];
        style.text_shadow_offset_y = lengths[1];
        if (lengths.size() >= 3) {
            style.text_shadow_blur = lengths[2];
        }
    }
    if (!color.empty()) {
        style.text_shadow_color = parseColor(color);
    }
}


void CSSParser::parseTransform(const std::string& value, ComputedStyle& style) {
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    // Reset transform values
    style.transform_translate_x = 0.0f;
    style.transform_translate_y = 0.0f;
    style.transform_translate_x_is_percent = false;
    style.transform_translate_y_is_percent = false;
    style.transform_scale_x = 1.0f;
    style.transform_scale_y = 1.0f;
    style.transform_rotate = 0.0f;
    style.transform_skew_x = 0.0f;
    style.transform_skew_y = 0.0f;
    
    if (v == "none") {
        return;
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
        
        // Parse arguments
        std::vector<std::string> arg_parts;
        std::string current;
        for (char c : args) {
            if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    arg_parts.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            arg_parts.push_back(current);
        }
        
        auto parse_translate_arg = [](const std::string& token, float& out_value, bool& out_is_percent) {
            out_value = parseFloat(token);
            out_is_percent = (token.find('%') != std::string::npos);
        };

        if (func_name == "translate" || func_name == "translate3d") {
            if (!arg_parts.empty()) {
                parse_translate_arg(arg_parts[0], style.transform_translate_x, style.transform_translate_x_is_percent);
                if (arg_parts.size() > 1) {
                    parse_translate_arg(arg_parts[1], style.transform_translate_y, style.transform_translate_y_is_percent);
                }
            }
        } else if (func_name == "translatex") {
            parse_translate_arg(args, style.transform_translate_x, style.transform_translate_x_is_percent);
        } else if (func_name == "translatey") {
            parse_translate_arg(args, style.transform_translate_y, style.transform_translate_y_is_percent);
        } else if (func_name == "scale" || func_name == "scale3d") {
            if (!arg_parts.empty()) {
                float sx = parseFloat(arg_parts[0]);
                float sy = arg_parts.size() > 1 ? parseFloat(arg_parts[1]) : sx;
                if (sx != 0.0f) style.transform_scale_x = sx;
                if (sy != 0.0f) style.transform_scale_y = sy;
            }
        } else if (func_name == "scalex") {
            float sx = parseFloat(args);
            if (sx != 0.0f) style.transform_scale_x = sx;
        } else if (func_name == "scaley") {
            float sy = parseFloat(args);
            if (sy != 0.0f) style.transform_scale_y = sy;
        } else if (func_name == "rotate" || func_name == "rotatez") {
            float angle = parseFloat(args);
            if (args.find("rad") != std::string::npos) {
                angle = angle * 180.0f / 3.14159265358979f;
            } else if (args.find("turn") != std::string::npos) {
                angle = angle * 360.0f;
            }
            style.transform_rotate = angle;
        } else if (func_name == "skew") {
            if (!arg_parts.empty()) {
                style.transform_skew_x = parseFloat(arg_parts[0]);
                if (arg_parts.size() > 1) {
                    style.transform_skew_y = parseFloat(arg_parts[1]);
                }
            }
        } else if (func_name == "skewx") {
            style.transform_skew_x = parseFloat(args);
        } else if (func_name == "skewy") {
            style.transform_skew_y = parseFloat(args);
        }
        
        pos = rparen + 1;
    }
}

// Parse CSS background-position value with support for 1-value, 2-value, 3-value, and 4-value syntax
// Examples:
// - "center" -> "50% 50%"
// - "left top" -> "0% 0%"
// - "right 10px" -> "calc(100% - 10px) 50%"
// - "right 10px bottom 20px" -> "calc(100% - 10px) calc(100% - 20px)"


std::string CSSParser::parseBackgroundPosition(const std::string& value) {
    if (value.empty()) return "0% 0%";

    std::istringstream iss(value);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    // Handle different number of tokens
    if (tokens.size() == 1) {
        // Single value: center, left, right, top, bottom, percentage, length
        std::string first = tokens[0];

        if (first == "center") return "50% 50%";
        if (first == "left") return "0% 50%";
        if (first == "right") return "100% 50%";
        if (first == "top") return "50% 0%";
        if (first == "bottom") return "50% 100%";

        // Single percentage or length: horizontal, vertical defaults to center per CSS
        return first + " 50%";
    }
    else if (tokens.size() == 2) {
        // Two values: horizontal vertical (order-insensitive for keyword pairs like "top left")
        std::string first = tokens[0];
        std::string second = tokens[1];

        auto isHorizontalKeyword = [](const std::string& t) {
            return t == "left" || t == "right" || t == "center";
        };
        // If written as "top left" etc, swap to horizontal vertical order.
        if ((tokens[0] == "top" || tokens[0] == "bottom") && isHorizontalKeyword(tokens[1])) {
            std::swap(first, second);
        }

        // Convert keywords to percentages
        if (first == "left") first = "0%";
        else if (first == "right") first = "100%";
        else if (first == "center") first = "50%";

        if (second == "top") second = "0%";
        else if (second == "bottom") second = "100%";
        else if (second == "center") second = "50%";

        return first + " " + second;
    }
    else if (tokens.size() == 3) {
        // Three values: edge offset vertical
        std::string edge = tokens[0];
        std::string offset = tokens[1];
        std::string vertical = tokens[2];

        if (edge == "left" || edge == "right") {
            // Horizontal edge with offset, then vertical position
            std::string horizontal;
            if (edge == "left") horizontal = offset;
            else horizontal = "calc(100% - " + offset + ")";

            if (vertical == "top") vertical = "0%";
            else if (vertical == "bottom") vertical = "100%";
            else if (vertical == "center") vertical = "50%";

            return horizontal + " " + vertical;
        }
        else if (edge == "top" || edge == "bottom") {
            // Vertical edge with offset, then horizontal position
            std::string vertical;
            if (edge == "top") vertical = offset;
            else vertical = "calc(100% - " + offset + ")";

            if (tokens[2] == "left") tokens[2] = "0%";
            else if (tokens[2] == "right") tokens[2] = "100%";
            else if (tokens[2] == "center") tokens[2] = "50%";

            return tokens[2] + " " + vertical;
        }
    }
    else if (tokens.size() == 4) {
        // Four values: horizontal-edge horizontal-offset vertical-edge vertical-offset
        std::string horizEdge = tokens[0];
        std::string horizOffset = tokens[1];
        std::string vertEdge = tokens[2];
        std::string vertOffset = tokens[3];

        std::string horizontal, vertical;

        if (horizEdge == "left") horizontal = horizOffset;
        else if (horizEdge == "right") horizontal = "calc(100% - " + horizOffset + ")";
        else if (horizEdge == "center") horizontal = "calc(50% + " + horizOffset + ")";

        if (vertEdge == "top") vertical = vertOffset;
        else if (vertEdge == "bottom") vertical = "calc(100% - " + vertOffset + ")";
        else if (vertEdge == "center") vertical = "calc(50% + " + vertOffset + ")";

        return horizontal + " " + vertical;
    }

    // Fallback for invalid syntax
    return "0% 0%";
}

} // namespace dong::dom
