#include "style_engine.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <iostream>

namespace dong::dom {

void Stylesheet::addRule(const std::string& selector, const ComputedStyle& style, 
                         int specificity, int order) {
    rules.push_back({selector, style, specificity, order});
}

StyleEngine::StyleEngine() = default;

void StyleEngine::addStylesheet(const std::string& css) {
    Stylesheet sheet;
    auto rules = parseCSS(css);
    for (const auto& rule : rules) {
        sheet.addRule(rule.selector, rule.style, rule.specificity, rule.source_order);
    }
    stylesheets.push_back(sheet);
}

std::vector<CSSRule> StyleEngine::parseCSS(const std::string& css) {
    std::vector<CSSRule> result;
    
    size_t pos = 0;
    while (pos < css.length()) {
        // Find opening brace
        size_t brace_open = css.find('{', pos);
        if (brace_open == std::string::npos) break;
        
        // Find closing brace
        size_t brace_close = css.find('}', brace_open);
        if (brace_close == std::string::npos) break;
        
        // Extract selector and declarations
        std::string selector = css.substr(pos, brace_open - pos);
        std::string declarations = css.substr(brace_open + 1, brace_close - brace_open - 1);
        
        selector = trimWhitespace(selector);
        
        // Handle multiple selectors separated by comma (e.g., "div, p { ... }")
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
                // Parse declarations into ComputedStyle
                auto style = ComputedStyle();
                
                // Parse individual declarations
                auto decls = splitDeclarations(declarations);
                for (const auto& decl : decls) {
                    size_t colon = decl.find(':');
                    if (colon != std::string::npos) {
                        std::string prop = trimWhitespace(decl.substr(0, colon));
                        std::string value = trimWhitespace(decl.substr(colon + 1));
                        applyStyleProperty(prop, value, style);
                    }
                }
                
                int specificity = calculateSpecificity(single_selector);
                CSSRule rule{single_selector, style, specificity, rule_order_counter++};
                result.push_back(rule);
            }
            
            if (comma_next == std::string::npos) break;
            comma_pos = comma_next + 1;
            comma_next = selector.find(',', comma_pos);
        }
        
        pos = brace_close + 1;
    }
    
    return result;
}

void StyleEngine::computeStyles(DOMNodePtr node) {
    if (!node) return;

    // Collect all matching rules for this node
    std::vector<CSSRule> matching_rules;
    
    for (const auto& sheet : stylesheets) {
        for (const auto& rule : sheet.getRules()) {
            if (matchesSelector(rule.selector, node)) {
                matching_rules.push_back(rule);
            }
        }
    }
    
    // Sort by specificity, then by source order
    std::sort(matching_rules.begin(), matching_rules.end(),
        [](const CSSRule& a, const CSSRule& b) {
            if (a.specificity != b.specificity) {
                return a.specificity < b.specificity;
            }
            return a.source_order < b.source_order;
        });
    
    // Apply rules in cascade order (later rules override earlier ones)
    auto& computed = node->getComputedStyle();
    for (const auto& rule : matching_rules) {
        // Merge styles with proper cascade
        if (!rule.style.color.empty() && rule.style.color != "#000000") 
            computed.color = rule.style.color;
        if (!rule.style.background_color.empty() && rule.style.background_color != "#ffffff") 
            computed.background_color = rule.style.background_color;
        if (rule.style.font_size != 16.0f) 
            computed.font_size = rule.style.font_size;
        if (!rule.style.font_weight.empty()) 
            computed.font_weight = rule.style.font_weight;
        if (!rule.style.text_align.empty()) 
            computed.text_align = rule.style.text_align;
        if (!rule.style.display.empty()) 
            computed.display = rule.style.display;
        if (!rule.style.position.empty()) 
            computed.position = rule.style.position;
        if (rule.style.border_radius != 0.0f) 
            computed.border_radius = rule.style.border_radius;
        if (rule.style.border_width != 0.0f) 
            computed.border_width = rule.style.border_width;
        if (!rule.style.border_color.empty()) 
            computed.border_color = rule.style.border_color;
        if (!rule.style.overflow.empty() && rule.style.overflow != "visible")
            computed.overflow = rule.style.overflow;
        if (rule.style.opacity != 1.0f)
            computed.opacity = rule.style.opacity;
        
        // Box model properties
        if (rule.style.width.unit != CSSValue::Unit::AUTO) 
            computed.width = rule.style.width;
        if (rule.style.height.unit != CSSValue::Unit::AUTO) 
            computed.height = rule.style.height;
        if (rule.style.min_width.unit != CSSValue::Unit::AUTO) 
            computed.min_width = rule.style.min_width;
        if (rule.style.max_width.unit != CSSValue::Unit::AUTO) 
            computed.max_width = rule.style.max_width;
        if (rule.style.min_height.unit != CSSValue::Unit::AUTO) 
            computed.min_height = rule.style.min_height;
        if (rule.style.max_height.unit != CSSValue::Unit::AUTO) 
            computed.max_height = rule.style.max_height;
        if (rule.style.margin_top.unit != CSSValue::Unit::AUTO) 
            computed.margin_top = rule.style.margin_top;
        if (rule.style.margin_right.unit != CSSValue::Unit::AUTO) 
            computed.margin_right = rule.style.margin_right;
        if (rule.style.margin_bottom.unit != CSSValue::Unit::AUTO) 
            computed.margin_bottom = rule.style.margin_bottom;
        if (rule.style.margin_left.unit != CSSValue::Unit::AUTO) 
            computed.margin_left = rule.style.margin_left;
        if (rule.style.padding_top.unit != CSSValue::Unit::AUTO) 
            computed.padding_top = rule.style.padding_top;
        if (rule.style.padding_right.unit != CSSValue::Unit::AUTO) 
            computed.padding_right = rule.style.padding_right;
        if (rule.style.padding_bottom.unit != CSSValue::Unit::AUTO) 
            computed.padding_bottom = rule.style.padding_bottom;
        if (rule.style.padding_left.unit != CSSValue::Unit::AUTO) 
            computed.padding_left = rule.style.padding_left;
        
        // Flexbox
        if (!rule.style.flex_direction.empty()) 
            computed.flex_direction = rule.style.flex_direction;
        if (!rule.style.justify_content.empty()) 
            computed.justify_content = rule.style.justify_content;
        if (!rule.style.align_items.empty()) 
            computed.align_items = rule.style.align_items;
        if (rule.style.flex != 0.0f) 
            computed.flex = rule.style.flex;
        if (rule.style.flex_grow != 0.0f) 
            computed.flex_grow = rule.style.flex_grow;
        if (rule.style.flex_shrink != 1.0f) 
            computed.flex_shrink = rule.style.flex_shrink;
        if (rule.style.flex_basis.unit != CSSValue::Unit::AUTO) 
            computed.flex_basis = rule.style.flex_basis;
    }

    // Inherit selected text-related properties from parent when not explicitly set
    auto parent = node->getParent();
    if (parent) {
        const auto& parent_style = parent->getComputedStyle();

        if (computed.color == "#000000") {
            computed.color = parent_style.color;
        }
        if (computed.font_family == "Arial") {
            computed.font_family = parent_style.font_family;
        }
        if (computed.font_size == 16.0f) {
            computed.font_size = parent_style.font_size;
        }
        if (computed.font_weight == "normal") {
            computed.font_weight = parent_style.font_weight;
        }
        if (computed.text_align == "left") {
            computed.text_align = parent_style.text_align;
        }
    }

    // Recursively compute styles for children
    for (const auto& child : node->getChildren()) {
        computeStyles(child);
    }
}

bool StyleEngine::matchesSelector(const std::string& selector, DOMNodePtr node) {
    if (!node || selector.empty()) return false;
    
    // Check for descendant/child combinators
    size_t space_pos = selector.rfind(' ');
    size_t gt_pos = selector.rfind('>');
    size_t plus_pos = selector.rfind('+');
    size_t tilde_pos = selector.rfind('~');
    
    // Find the last combinator
    size_t last_combinator = std::string::npos;
    std::string combinator;
    
    if (gt_pos != std::string::npos) {
        last_combinator = gt_pos;
        combinator = ">";
    }
    if (plus_pos != std::string::npos && (last_combinator == std::string::npos || plus_pos > last_combinator)) {
        last_combinator = plus_pos;
        combinator = "+";
    }
    if (tilde_pos != std::string::npos && (last_combinator == std::string::npos || tilde_pos > last_combinator)) {
        last_combinator = tilde_pos;
        combinator = "~";
    }
    if (space_pos != std::string::npos && (last_combinator == std::string::npos || space_pos > last_combinator)) {
        // Only consider space as combinator if it's between actual selectors
        if (space_pos > 0 && space_pos < selector.length() - 1) {
            last_combinator = space_pos;
            combinator = " ";
        }
    }
    
    // If there's a combinator, process it
    if (last_combinator != std::string::npos) {
        std::string right_selector = trimWhitespace(selector.substr(last_combinator + (combinator == " " ? 1 : 1)));
        std::string left_selector = trimWhitespace(selector.substr(0, last_combinator));
        
        // First check if the right part matches
        if (!matchesSimpleSelector(right_selector, node)) {
            return false;
        }
        
        // Then check the combinator relationship
        auto parent = node->getParent();
        if (!parent) return false;
        
        if (combinator == " ") {
            // Descendant combinator
            return matchesDescendant(left_selector, parent);
        } else if (combinator == ">") {
            // Child combinator
            return matchesSimpleSelector(left_selector, parent);
        } else if (combinator == "+") {
            // Adjacent sibling combinator - find previous sibling
            auto grandparent = parent->getParent();
            if (!grandparent) return false;
            
            const auto& siblings = grandparent->getChildren();
            for (size_t i = 0; i < siblings.size(); ++i) {
                if (siblings[i] == parent) {
                    if (i > 0 && matchesSimpleSelector(left_selector, siblings[i - 1])) {
                        return true;
                    }
                    break;
                }
            }
            return false;
        }
    }
    
    return matchesSimpleSelector(selector, node);
}

bool StyleEngine::matchesComplexSelector(const std::string& selector, DOMNodePtr node) {
    // Handle multiple simple selectors chained together (e.g., "div.highlight#main")
    return matchesSimpleSelector(selector, node);
}

bool StyleEngine::matchesSimpleSelector(const std::string& selector, DOMNodePtr node) {
    std::string trimmed = trimWhitespace(selector);
    
    if (trimmed.empty()) return false;
    if (!node) return false;

    // Parse the selector to extract components (e.g., "div.class#id")
    size_t pos = 0;
    
    // First, check if it's just a tag selector or has class/id
    size_t dot_pos = trimmed.find('.');
    size_t hash_pos = trimmed.find('#');
    
    size_t tag_end = std::string::npos;
    if (dot_pos != std::string::npos) {
        tag_end = (hash_pos != std::string::npos && hash_pos < dot_pos) ? hash_pos : dot_pos;
    } else if (hash_pos != std::string::npos) {
        tag_end = hash_pos;
    }
    
    // Check tag name (if not starting with . or #)
    if (trimmed[0] != '.' && trimmed[0] != '#') {
        std::string tag = tag_end != std::string::npos ? 
            trimmed.substr(0, tag_end) : trimmed;
        
        if (tag != "*" && !matchesTagSelector(tag, node)) {
            return false;
        }
        
        pos = tag_end;
    }
    
    // Check class and id selectors
    while (pos < trimmed.length()) {
        if (trimmed[pos] == '.') {
            size_t next_pos = std::string::npos;
            for (size_t i = pos + 1; i < trimmed.length(); ++i) {
                if (trimmed[i] == '.' || trimmed[i] == '#' || trimmed[i] == '[') {
                    next_pos = i;
                    break;
                }
            }
            
            std::string cls = next_pos != std::string::npos ? 
                trimmed.substr(pos + 1, next_pos - pos - 1) : trimmed.substr(pos + 1);
            
            if (!matchesClassSelector(cls, node)) {
                return false;
            }
            
            pos = next_pos;
        } else if (trimmed[pos] == '#') {
            size_t next_pos = std::string::npos;
            for (size_t i = pos + 1; i < trimmed.length(); ++i) {
                if (trimmed[i] == '.' || trimmed[i] == '#' || trimmed[i] == '[') {
                    next_pos = i;
                    break;
                }
            }
            
            std::string id = next_pos != std::string::npos ? 
                trimmed.substr(pos + 1, next_pos - pos - 1) : trimmed.substr(pos + 1);
            
            if (!matchesIdSelector(id, node)) {
                return false;
            }
            
            pos = next_pos;
        } else if (trimmed[pos] == '[') {
            size_t bracket_close = trimmed.find(']', pos);
            if (bracket_close != std::string::npos) {
                std::string attr_sel = trimmed.substr(pos + 1, bracket_close - pos - 1);
                if (!matchesAttributeSelector(attr_sel, node)) {
                    return false;
                }
                pos = bracket_close + 1;
            } else {
                break;
            }
        } else if (trimmed[pos] == ':') {
            size_t next_pos = pos + 1;
            while (next_pos < trimmed.length() && 
                   (std::isalnum(trimmed[next_pos]) || trimmed[next_pos] == '-')) {
                ++next_pos;
            }
            std::string pseudo = trimmed.substr(pos + 1, next_pos - pos - 1);
            if (!matchesPseudoClass(pseudo, node)) {
                return false;
            }
            pos = next_pos;
        } else {
            ++pos;
        }
    }
    
    return true;
}

bool StyleEngine::matchesTagSelector(const std::string& tag, DOMNodePtr node) {
    if (!node) return false;
    if (tag == "*") return true;  // Universal selector
    return node->getTagName() == tag;
}

bool StyleEngine::matchesClassSelector(const std::string& cls, DOMNodePtr node) {
    if (!node || !node->hasAttribute("class")) return false;
    
    std::string classes = node->getAttribute("class");
    std::istringstream iss(classes);
    std::string class_name;
    
    while (iss >> class_name) {
        if (class_name == cls) return true;
    }
    
    return false;
}

bool StyleEngine::matchesIdSelector(const std::string& id, DOMNodePtr node) {
    if (!node || !node->hasAttribute("id")) return false;
    return node->getAttribute("id") == id;
}

bool StyleEngine::matchesAttributeSelector(const std::string& attr_sel, DOMNodePtr node) {
    if (!node) return false;
    
    // Parse attribute selector: [attr], [attr="value"], [attr~="value"], [attr|="value"], etc.
    size_t eq_pos = attr_sel.find('=');
    
    if (eq_pos == std::string::npos) {
        // Just [attr]
        std::string attr_name = trimWhitespace(attr_sel);
        return node->hasAttribute(attr_name);
    }
    
    // [attr op="value"]
    size_t op_start = attr_sel.rfind(' ', eq_pos);
    if (op_start == std::string::npos) op_start = 0;
    else op_start++;
    
    std::string attr_name = trimWhitespace(attr_sel.substr(0, op_start));
    std::string op;
    
    if (attr_sel[op_start] == '~' || attr_sel[op_start] == '|' || 
        attr_sel[op_start] == '^' || attr_sel[op_start] == '$' || attr_sel[op_start] == '*') {
        if (eq_pos > op_start + 1) {
            op = attr_sel[op_start];
            op += '=';
        } else {
            op = "=";
        }
    } else {
        op = "=";
    }
    
    if (!node->hasAttribute(attr_name)) return false;
    
    std::string attr_value = node->getAttribute(attr_name);
    
    // Extract the value from selector (remove quotes)
    size_t val_start = attr_sel.find('"', eq_pos);
    if (val_start == std::string::npos) {
        val_start = attr_sel.find('\'', eq_pos);
    }
    if (val_start == std::string::npos) return false;
    
    size_t val_end = attr_sel.rfind('"');
    if (val_end == std::string::npos) {
        val_end = attr_sel.rfind('\'');
    }
    if (val_end == std::string::npos || val_end <= val_start) return false;
    
    std::string expected_value = attr_sel.substr(val_start + 1, val_end - val_start - 1);
    
    if (op == "=") {
        return attr_value == expected_value;
    } else if (op == "~=") {
        // Word match
        std::istringstream iss(attr_value);
        std::string word;
        while (iss >> word) {
            if (word == expected_value) return true;
        }
        return false;
    } else if (op == "|=") {
        // Language match
        return attr_value == expected_value || 
               (attr_value.length() > expected_value.length() &&
                attr_value.substr(0, expected_value.length() + 1) == expected_value + "-");
    } else if (op == "^=") {
        // Starts with
        return attr_value.find(expected_value) == 0;
    } else if (op == "$=") {
        // Ends with
        return attr_value.length() >= expected_value.length() &&
               attr_value.substr(attr_value.length() - expected_value.length()) == expected_value;
    } else if (op == "*=") {
        // Contains
        return attr_value.find(expected_value) != std::string::npos;
    }
    
    return false;
}

bool StyleEngine::matchesPseudoClass(const std::string& pseudo, DOMNodePtr node) {
    // Basic pseudo-class support
    if (pseudo == "root") {
        return node->getParent() == nullptr;
    } else if (pseudo == "first-child") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        return !children.empty() && children[0] == node;
    } else if (pseudo == "last-child") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        return !children.empty() && children.back() == node;
    } else if (pseudo == "only-child") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        return children.size() == 1 && children[0] == node;
    }
    
    // Unsupported pseudo-classes (like :hover, :focus) don't match by default
    return false;
}

bool StyleEngine::matchesDescendant(const std::string& ancestor_sel, DOMNodePtr node) {
    DOMNodePtr current = node;
    while (current) {
        if (matchesSimpleSelector(ancestor_sel, current)) {
            return true;
        }
        current = current->getParent();
    }
    return false;
}

bool StyleEngine::matchesChild(const std::string& parent_sel, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;
    return matchesSimpleSelector(parent_sel, parent);
}

bool StyleEngine::matchesAdjacent(const std::string& prev_sel, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;
    
    const auto& siblings = parent->getChildren();
    for (size_t i = 0; i < siblings.size(); ++i) {
        if (siblings[i] == node && i > 0) {
            return matchesSimpleSelector(prev_sel, siblings[i - 1]);
        }
    }
    return false;
}

int StyleEngine::calculateSpecificity(const std::string& selector) {
    // Specificity = (id_count * 10000) + (class_count * 100) + element_count
    int id_count = countIdSelectors(selector);
    int class_count = countClassSelectors(selector);
    int element_count = countElementSelectors(selector);
    
    return (id_count * 10000) + (class_count * 100) + element_count;
}

int StyleEngine::countIdSelectors(const std::string& selector) {
    int count = 0;
    for (size_t i = 0; i < selector.length(); ++i) {
        if (selector[i] == '#') {
            // Make sure it's not part of a color value or inside a string
            if (i == 0 || !std::isdigit(selector[i - 1])) {
                ++count;
            }
        }
    }
    return count;
}

int StyleEngine::countClassSelectors(const std::string& selector) {
    int count = 0;
    for (size_t i = 0; i < selector.length(); ++i) {
        if (selector[i] == '.') {
            // Make sure it's not part of a decimal number
            if (i == 0 || !std::isdigit(selector[i - 1])) {
                ++count;
            }
        }
    }
    return count;
}

int StyleEngine::countElementSelectors(const std::string& selector) {
    int count = 0;
    bool in_selector = false;
    
    for (size_t i = 0; i < selector.length(); ++i) {
        char c = selector[i];
        
        if (std::isalpha(c) && (i == 0 || !std::isalnum(selector[i - 1]) && selector[i - 1] != '-')) {
            // Start of an element selector
            if (!in_selector && i > 0) {
                if (selector[i - 1] == ' ' || selector[i - 1] == '>' || 
                    selector[i - 1] == '+' || selector[i - 1] == '~') {
                    ++count;
                }
            }
            if (i == 0) ++count;
            in_selector = true;
        } else if (!std::isalnum(c) && c != '-') {
            in_selector = false;
        }
    }
    
    return count;
}

std::string StyleEngine::trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> StyleEngine::splitDeclarations(const std::string& css) {
    std::vector<std::string> result;
    std::istringstream iss(css);
    std::string decl;
    
    while (std::getline(iss, decl, ';')) {
        decl = trimWhitespace(decl);
        if (!decl.empty()) {
            result.push_back(decl);
        }
    }
    
    return result;
}

void StyleEngine::applyStyleProperty(const std::string& property, const std::string& value, 
                                     ComputedStyle& style) {
    std::string prop = trimWhitespace(property);
    std::string val = trimWhitespace(value);
    
    // Remove trailing semicolon if present
    if (!val.empty() && val.back() == ';') {
        val.pop_back();
        val = trimWhitespace(val);
    }

    auto parseFloat = [](const std::string& s) -> float {
        std::string v = s;
        // trim spaces
        v.erase(0, v.find_first_not_of(" \t\n\r"));
        size_t last = v.find_last_not_of(" \t\n\r");
        if (last != std::string::npos && last + 1 < v.size()) {
            v.erase(last + 1);
        }
        // take leading numeric part
        size_t i = 0;
        while (i < v.size() && (std::isdigit(static_cast<unsigned char>(v[i])) || v[i] == '-' || v[i] == '+' || v[i] == '.')) {
            ++i;
        }
        if (i == 0) return 0.0f;
        try {
            return std::stof(v.substr(0, i));
        } catch (...) {
            return 0.0f;
        }
    };

    auto parseLength = [&](const std::string& s) -> CSSValue {
        std::string v = trimWhitespace(s);
        if (v == "auto") {
            return CSSValue(0.0f, CSSValue::Unit::AUTO);
        }
        bool is_percent = v.find('%') != std::string::npos;
        float num = parseFloat(v);
        if (num == 0.0f && v.find('0') == std::string::npos) {
            // failed to parse, treat as AUTO
            return CSSValue(0.0f, CSSValue::Unit::AUTO);
        }
        return CSSValue(num, is_percent ? CSSValue::Unit::PERCENT : CSSValue::Unit::PIXEL);
    };
    
    if (prop == "color") {
        style.color = val;
    }
    else if (prop == "background-color" || prop == "background") {
        style.background_color = val;
    }
    else if (prop == "font-size") {
        style.font_size = parseFloat(val);
    }
    else if (prop == "font-weight") {
        style.font_weight = val;
    }
    else if (prop == "font-family") {
        style.font_family = val;
    }
    else if (prop == "text-align") {
        style.text_align = val;
    }
    else if (prop == "display") {
        style.display = val;
    }
    else if (prop == "position") {
        style.position = val;
    }
    else if (prop == "overflow") {
        style.overflow = val;
    }
    else if (prop == "width") {
        style.width = parseLength(val);
    }
    else if (prop == "height") {
        style.height = parseLength(val);
    }
    else if (prop == "min-width") {
        style.min_width = parseLength(val);
    }
    else if (prop == "max-width") {
        style.max_width = parseLength(val);
    }
    else if (prop == "min-height") {
        style.min_height = parseLength(val);
    }
    else if (prop == "max-height") {
        style.max_height = parseLength(val);
    }
    else if (prop == "margin") {
        // margin: 10px or 10px 20px or 10px 20px 30px or 10px 20px 30px 40px
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        
        if (parts.size() == 1) {
            auto v = parseLength(parts[0]);
            style.margin_top = style.margin_right = style.margin_bottom = style.margin_left = v;
        } else if (parts.size() == 2) {
            auto v1 = parseLength(parts[0]);
            auto v2 = parseLength(parts[1]);
            style.margin_top = style.margin_bottom = v1;
            style.margin_left = style.margin_right = v2;
        } else if (parts.size() >= 4) {
            style.margin_top = parseLength(parts[0]);
            style.margin_right = parseLength(parts[1]);
            style.margin_bottom = parseLength(parts[2]);
            style.margin_left = parseLength(parts[3]);
        }
    }
    else if (prop == "margin-top") {
        style.margin_top = parseLength(val);
    }
    else if (prop == "margin-right") {
        style.margin_right = parseLength(val);
    }
    else if (prop == "margin-bottom") {
        style.margin_bottom = parseLength(val);
    }
    else if (prop == "margin-left") {
        style.margin_left = parseLength(val);
    }
    else if (prop == "padding") {
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        
        if (parts.size() == 1) {
            auto v = parseLength(parts[0]);
            style.padding_top = style.padding_right = style.padding_bottom = style.padding_left = v;
        } else if (parts.size() == 2) {
            auto v1 = parseLength(parts[0]);
            auto v2 = parseLength(parts[1]);
            style.padding_top = style.padding_bottom = v1;
            style.padding_left = style.padding_right = v2;
        } else if (parts.size() >= 4) {
            style.padding_top = parseLength(parts[0]);
            style.padding_right = parseLength(parts[1]);
            style.padding_bottom = parseLength(parts[2]);
            style.padding_left = parseLength(parts[3]);
        }
    }
    else if (prop == "padding-top") {
        style.padding_top = parseLength(val);
    }
    else if (prop == "padding-right") {
        style.padding_right = parseLength(val);
    }
    else if (prop == "padding-bottom") {
        style.padding_bottom = parseLength(val);
    }
    else if (prop == "padding-left") {
        style.padding_left = parseLength(val);
    }
    else if (prop == "flex-direction") {
        style.flex_direction = val;
    }
    else if (prop == "justify-content") {
        style.justify_content = val;
    }
    else if (prop == "align-items") {
        style.align_items = val;
    }
    else if (prop == "flex") {
        style.flex = parseFloat(val);
    }
    else if (prop == "flex-grow") {
        style.flex_grow = parseFloat(val);
    }
    else if (prop == "flex-shrink") {
        style.flex_shrink = parseFloat(val);
    }
    else if (prop == "flex-basis") {
        style.flex_basis = parseLength(val);
    }
    else if (prop == "border-radius") {
        style.border_radius = parseFloat(val);
    }
    else if (prop == "border-width") {
        style.border_width = parseFloat(val);
    }
    else if (prop == "border-color") {
        style.border_color = val;
    }
    else if (prop == "opacity") {
        float v = parseFloat(val);
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        style.opacity = v;
    }
}

std::pair<std::string, ComputedStyle> StyleEngine::parseRule(const std::string& rule_str) {
    ComputedStyle style;
    std::string selector;
    
    size_t brace = rule_str.find('{');
    if (brace != std::string::npos) {
        selector = trimWhitespace(rule_str.substr(0, brace));
        
        size_t end_brace = rule_str.find('}', brace);
        std::string declarations = rule_str.substr(brace + 1, 
            end_brace != std::string::npos ? end_brace - brace - 1 : std::string::npos);
        
        auto decls = splitDeclarations(declarations);
        for (const auto& decl : decls) {
            size_t colon = decl.find(':');
            if (colon != std::string::npos) {
                std::string prop = trimWhitespace(decl.substr(0, colon));
                std::string value = trimWhitespace(decl.substr(colon + 1));
                applyStyleProperty(prop, value, style);
            }
        }
    }
    
    return {selector, style};
}

std::vector<SelectorPart> StyleEngine::parseSelector(const std::string& selector) {
    std::vector<SelectorPart> parts;
    
    size_t pos = 0;
    while (pos < selector.length()) {
        char c = selector[pos];
        
        if (c == ' ') {
            parts.push_back({SelectorPart::Type::COMBINATOR, "", " "});
            ++pos;
        } else if (c == '>') {
            parts.push_back({SelectorPart::Type::COMBINATOR, "", ">"});
            ++pos;
        } else if (c == '+') {
            parts.push_back({SelectorPart::Type::COMBINATOR, "", "+"});
            ++pos;
        } else if (c == '~') {
            parts.push_back({SelectorPart::Type::COMBINATOR, "", "~"});
            ++pos;
        } else {
            std::string component = extractSelectorComponent(selector, pos);
            if (!component.empty()) {
                if (component[0] == '.') {
                    parts.push_back({SelectorPart::Type::CLASS, component.substr(1), ""});
                } else if (component[0] == '#') {
                    parts.push_back({SelectorPart::Type::ID, component.substr(1), ""});
                } else if (component[0] == '[') {
                    parts.push_back({SelectorPart::Type::ATTRIBUTE, component, ""});
                } else if (component[0] == ':') {
                    parts.push_back({SelectorPart::Type::PSEUDO, component.substr(1), ""});
                } else {
                    parts.push_back({SelectorPart::Type::ELEMENT, component, ""});
                }
            }
        }
    }
    
    return parts;
}

std::string StyleEngine::extractSelectorComponent(const std::string& selector, size_t& pos) {
    if (pos >= selector.length()) return "";
    
    std::string component;
    
    if (selector[pos] == '.') {
        component += '.';
        ++pos;
        while (pos < selector.length() && (std::isalnum(selector[pos]) || selector[pos] == '-' || selector[pos] == '_')) {
            component += selector[pos++];
        }
    } else if (selector[pos] == '#') {
        component += '#';
        ++pos;
        while (pos < selector.length() && (std::isalnum(selector[pos]) || selector[pos] == '-' || selector[pos] == '_')) {
            component += selector[pos++];
        }
    } else if (selector[pos] == '[') {
        size_t end = selector.find(']', pos);
        if (end != std::string::npos) {
            component = selector.substr(pos, end - pos + 1);
            pos = end + 1;
        }
    } else if (selector[pos] == ':') {
        component += ':';
        ++pos;
        while (pos < selector.length() && (std::isalpha(selector[pos]) || selector[pos] == '-')) {
            component += selector[pos++];
        }
    } else {
        // Element selector
        while (pos < selector.length() && (std::isalnum(selector[pos]) || selector[pos] == '-')) {
            component += selector[pos++];
        }
    }
    
    return component;
}

} // namespace dong::dom
