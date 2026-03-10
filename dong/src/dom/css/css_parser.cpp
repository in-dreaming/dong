#include "css_parser.hpp"
#include "../../core/profiler.h"
#include "../../core/string_utils.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <cmath>
#include <functional>
#include <string_view>

namespace dong::dom {

// ============================================================================
// CSS Property Dispatch Table - O(1) lookup instead of O(n) if-else chain
// ============================================================================


std::string CSSParser::removeComments(const std::string& css) {
    std::string result;
    result.reserve(css.length());
    size_t i = 0;
    while (i < css.length()) {
        if (i + 1 < css.length() && css[i] == '/' && css[i + 1] == '*') {
            size_t end = css.find("*/", i + 2);
            if (end != std::string::npos) {
                i = end + 2;
            } else {
                break;
            }
        } else {
            result += css[i];
            ++i;
        }
    }
    return result;
}

std::string CSSParser::trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> CSSParser::splitDeclarations(const std::string& css) {
    std::vector<std::string> result;
    std::string current;
    int paren_depth = 0;
    
    for (char c : css) {
        if (c == '(') {
            ++paren_depth;
            current += c;
        } else if (c == ')') {
            --paren_depth;
            current += c;
        } else if (c == ';' && paren_depth == 0) {
            std::string trimmed = trimWhitespace(current);
            if (!trimmed.empty()) {
                result.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }
    
    std::string trimmed = trimWhitespace(current);
    if (!trimmed.empty()) {
        result.push_back(trimmed);
    }
    
    return result;
}

int CSSParser::calculateSpecificity(const std::string& selector) {
    int id_count = 0;
    int class_count = 0;
    int element_count = 0;
    
    for (size_t i = 0; i < selector.length(); ++i) {
        if (selector[i] == '#') {
            if (i == 0 || !std::isdigit(selector[i - 1])) {
                ++id_count;
            }
        } else if (selector[i] == '.') {
            if (i == 0 || !std::isdigit(selector[i - 1])) {
                ++class_count;
            }
        } else if (selector[i] == ':') {
            ++class_count;  // Pseudo-classes count as classes
        } else if (selector[i] == '[') {
            ++class_count;  // Attribute selectors count as classes
        }
    }
    
    // Count element selectors
    bool in_element = false;
    for (size_t i = 0; i < selector.length(); ++i) {
        char c = selector[i];
        if (std::isalpha(c) && !in_element) {
            if (i == 0 || selector[i - 1] == ' ' || selector[i - 1] == '>' || 
                selector[i - 1] == '+' || selector[i - 1] == '~') {
                ++element_count;
                in_element = true;
            }
        } else if (!std::isalnum(c) && c != '-') {
            in_element = false;
        }
    }
    
    return (id_count * 10000) + (class_count * 100) + element_count;
}

std::vector<CSSRule> CSSParser::parse(const std::string& css) {
    DONG_PROFILE_SCOPE_CAT("CSSParser::parse", "parse");
    std::vector<CSSRule> result;
    std::string css_clean = removeComments(css);
    
    size_t pos = 0;
    while (pos < css_clean.length()) {
        // Skip leading whitespace
        while (pos < css_clean.length() && std::isspace(static_cast<unsigned char>(css_clean[pos]))) {
            ++pos;
        }
        if (pos >= css_clean.length()) break;

        // Handle @layer rules separately
        if (css_clean[pos] == '@' && css_clean.substr(pos, 6) == "@layer") {
            size_t brace = css_clean.find('{', pos);
            size_t semicolon = css_clean.find(';', pos);

            // Find the end of the @layer rule
            size_t end = pos;
            if (brace != std::string::npos && (semicolon == std::string::npos || brace < semicolon)) {
                // @layer with rules: @layer name { ... }
                int depth = 1;
                end = brace + 1;
                while (end < css_clean.length() && depth > 0) {
                    if (css_clean[end] == '{') ++depth;
                    else if (css_clean[end] == '}') --depth;
                    ++end;
                }
            } else if (semicolon != std::string::npos) {
                // @layer predeclaration: @layer name; or @layer name1, name2;
                end = semicolon + 1;
            } else {
                // Invalid syntax, skip to next character
                end = pos + 1;
            }
            pos = end;
            continue;
        }

        // Skip other @-rules (handled separately)
        if (css_clean[pos] == '@') {
            size_t brace = css_clean.find('{', pos);
            if (brace != std::string::npos) {
                // Find matching closing brace
                int depth = 1;
                size_t end = brace + 1;
                while (end < css_clean.length() && depth > 0) {
                    if (css_clean[end] == '{') ++depth;
                    else if (css_clean[end] == '}') --depth;
                    ++end;
                }
                pos = end;
            } else {
                break;
            }
            continue;
        }
        
        // Find opening brace
        size_t brace_open = css_clean.find('{', pos);
        if (brace_open == std::string::npos) break;
        
        // Find closing brace
        size_t brace_close = css_clean.find('}', brace_open);
        if (brace_close == std::string::npos) break;
        
        std::string selector = css_clean.substr(pos, brace_open - pos);
        std::string declarations = css_clean.substr(brace_open + 1, brace_close - brace_open - 1);
        
        selector = trimWhitespace(selector);
        
        // Handle multiple selectors separated by comma
        std::vector<std::string> selectors;
        size_t comma_pos = 0;
        size_t comma_next = selector.find(',', comma_pos);
        
        while (true) {
            std::string single_selector;
            if (comma_next == std::string::npos) {
                single_selector = trimWhitespace(selector.substr(comma_pos));
            } else {
                single_selector = trimWhitespace(selector.substr(comma_pos, comma_next - comma_pos));
            }
            
            if (!single_selector.empty()) {
                selectors.push_back(single_selector);
            }
            
            if (comma_next == std::string::npos) break;
            comma_pos = comma_next + 1;
            comma_next = selector.find(',', comma_pos);
        }
        
        // Parse declarations
        auto decls = splitDeclarations(declarations);
        ComputedStyle style;
        // Use empty/negative sentinels for "unspecified" so StyleEngine can cascade correctly.
        style.display = "";
        style.background_color = "";
        style.color = "";
        style.border_style = "";
        style.border_color = "";
        style.border_width = -1.0f;
        style.color_scheme = "";



        for (const auto& decl : decls) {
            size_t colon = decl.find(':');
            if (colon != std::string::npos) {
                std::string prop = trimWhitespace(decl.substr(0, colon));
                std::string value = trimWhitespace(decl.substr(colon + 1));

                // Check for !important
                bool is_important = false;
                size_t important_pos = value.find("!important");
                if (important_pos != std::string::npos) {
                    is_important = true;
                    value = value.substr(0, important_pos);
                    value = trimWhitespace(value);
                }

                // Normalize property key to lowercase so `isExplicitlySet()` checks are consistent.
                std::string prop_key = prop;
                std::transform(prop_key.begin(), prop_key.end(), prop_key.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });

                applyProperty(prop_key, value, style);

                // Mark as explicitly set so the cascade (and generated content/counters) works.
                style.markExplicitlySet(prop_key);

                if (is_important) {
                    style.markImportant(prop_key);
                }
            }
        }
        
        // Create rules for each selector
        for (const auto& sel : selectors) {
            int specificity = calculateSpecificity(sel);
            CSSRule rule{sel, style, specificity, rule_order_counter_++};
            result.push_back(rule);
        }
        
        pos = brace_close + 1;
    }
    
    return result;
}

void CSSParser::parseInlineStyle(const std::string& style_str, ComputedStyle& style) {
    size_t pos = 0;
    while (pos < style_str.length()) {
        size_t semicolon = style_str.find(';', pos);
        if (semicolon == std::string::npos) semicolon = style_str.length();

        std::string declaration = style_str.substr(pos, semicolon - pos);
        pos = semicolon + 1;

        size_t colon = declaration.find(':');
        if (colon == std::string::npos) continue;

        std::string property = declaration.substr(0, colon);
        std::string value = declaration.substr(colon + 1);

        // Trim whitespace
        property.erase(0, property.find_first_not_of(" \t"));
        property.erase(property.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        // Check for !important
        bool is_important = false;
        size_t important_pos = value.find("!important");
        if (important_pos != std::string::npos) {
            is_important = true;
            value = value.substr(0, important_pos);
            // Trim trailing whitespace after removing !important
            value.erase(value.find_last_not_of(" \t") + 1);
        }

        applyProperty(property, value, style);
        // Mark as explicitly set for inheritance tracking
        style.markExplicitlySet(property);
        if (is_important) {
            style.markImportant(property);
        }
    }
}

} // namespace dong::dom
