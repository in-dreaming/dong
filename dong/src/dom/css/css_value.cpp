#include "css_value.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace dong::dom {

float CSSValue::resolvePixels(float parent_size, float root_font_size, 
                               float viewport_width, float viewport_height) const {
    switch (unit) {
        case Unit::PIXEL:
            return value;
        case Unit::PERCENT:
            return value * parent_size / 100.0f;
        case Unit::EM:
            return value * parent_size;  // parent_size is parent font size for em
        case Unit::REM:
            return value * root_font_size;
        case Unit::VW:
            return value * viewport_width / 100.0f;
        case Unit::VH:
            return value * viewport_height / 100.0f;
        case Unit::VMIN:
            return value * std::min(viewport_width, viewport_height) / 100.0f;
        case Unit::VMAX:
            return value * std::max(viewport_width, viewport_height) / 100.0f;
        case Unit::CH:
            return value * root_font_size * 0.5f;  // Approximate ch as 0.5em
        case Unit::CALC:
            if (calc_expr) {
                return calc_expr->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            }
            return 0.0f;
        case Unit::ENV:
            // env() values are resolved during style computation, not here
            // This should never be called for ENV values
            return 0.0f;
        case Unit::AUTO:
        case Unit::INHERIT:
        case Unit::UNSET:
        default:
            return 0.0f;
    }
}

// CSSCalcExpression implementation
float CSSCalcExpression::evaluate(float parent_size, float root_font_size,
                                   float viewport_width, float viewport_height) const {
    switch (op) {
        case Op::VALUE:
            return value.resolvePixels(parent_size, root_font_size, viewport_width, viewport_height);
            
        case Op::ADD: {
            if (children.size() < 2) return 0.0f;
            float result = children[0]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            for (size_t i = 1; i < children.size(); ++i) {
                result += children[i]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            }
            return result;
        }
        
        case Op::SUBTRACT: {
            if (children.size() < 2) return 0.0f;
            float result = children[0]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            for (size_t i = 1; i < children.size(); ++i) {
                result -= children[i]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            }
            return result;
        }
        
        case Op::MULTIPLY: {
            if (children.size() < 2) return 0.0f;
            float result = children[0]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            for (size_t i = 1; i < children.size(); ++i) {
                result *= children[i]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            }
            return result;
        }
        
        case Op::DIVIDE: {
            if (children.size() < 2) return 0.0f;
            float result = children[0]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            for (size_t i = 1; i < children.size(); ++i) {
                float divisor = children[i]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
                if (std::abs(divisor) > 1e-6f) {
                    result /= divisor;
                }
            }
            return result;
        }
        
        case Op::MIN: {
            if (children.empty()) return 0.0f;
            float result = children[0]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            for (size_t i = 1; i < children.size(); ++i) {
                result = std::min(result, children[i]->evaluate(parent_size, root_font_size, viewport_width, viewport_height));
            }
            return result;
        }
        
        case Op::MAX: {
            if (children.empty()) return 0.0f;
            float result = children[0]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            for (size_t i = 1; i < children.size(); ++i) {
                result = std::max(result, children[i]->evaluate(parent_size, root_font_size, viewport_width, viewport_height));
            }
            return result;
        }
        
        case Op::CLAMP: {
            if (children.size() < 3) return 0.0f;
            float min_val = children[0]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            float val = children[1]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            float max_val = children[2]->evaluate(parent_size, root_font_size, viewport_width, viewport_height);
            return std::clamp(val, min_val, max_val);
        }
    }
    return 0.0f;
}

// CSSVariables implementation
void CSSVariables::set(const std::string& name, const std::string& value) {
    variables_[name] = value;
}

std::string CSSVariables::get(const std::string& name, const std::string& fallback) const {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return it->second;
    }
    return fallback;
}

bool CSSVariables::has(const std::string& name) const {
    return variables_.find(name) != variables_.end();
}

void CSSVariables::remove(const std::string& name) {
    variables_.erase(name);
}

void CSSVariables::clear() {
    variables_.clear();
}

std::string CSSVariables::resolveVarReferences(const std::string& value) const {
    std::string result = value;
    size_t pos = 0;
    
    while ((pos = result.find("var(", pos)) != std::string::npos) {
        // Find matching closing parenthesis
        int depth = 1;
        size_t start = pos + 4;
        size_t end = start;
        
        while (end < result.size() && depth > 0) {
            if (result[end] == '(') ++depth;
            else if (result[end] == ')') --depth;
            ++end;
        }
        
        if (depth != 0) {
            // Malformed var(), skip
            pos += 4;
            continue;
        }
        
        // Extract var() content
        std::string var_content = result.substr(start, end - start - 1);
        
        // Parse variable name and optional fallback
        std::string var_name;
        std::string fallback;
        
        size_t comma = var_content.find(',');
        if (comma != std::string::npos) {
            var_name = var_content.substr(0, comma);
            fallback = var_content.substr(comma + 1);
            // Trim whitespace
            while (!var_name.empty() && (var_name.back() == ' ' || var_name.back() == '\t')) {
                var_name.pop_back();
            }
            while (!fallback.empty() && (fallback.front() == ' ' || fallback.front() == '\t')) {
                fallback.erase(0, 1);
            }
        } else {
            var_name = var_content;
        }
        
        // Trim whitespace from var_name
        while (!var_name.empty() && (var_name.front() == ' ' || var_name.front() == '\t')) {
            var_name.erase(0, 1);
        }
        while (!var_name.empty() && (var_name.back() == ' ' || var_name.back() == '\t')) {
            var_name.pop_back();
        }
        
        // Get variable value
        std::string resolved = get(var_name, fallback);
        
        // Recursively resolve nested var() references
        if (resolved.find("var(") != std::string::npos) {
            resolved = resolveVarReferences(resolved);
        }
        
        // Replace var() with resolved value
        result.replace(pos, end - pos, resolved);
    }
    
    return result;
}

} // namespace dong::dom
