#include "style_engine.hpp"
#include "../core/log.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <iostream>
#include <unordered_set>

namespace dong::dom {

namespace {

LayoutMode deriveLayoutModeFromDisplay(const ComputedStyle& style) {
    const std::string& d = style.display;
    if (d == "none") {
        return LayoutMode::None;
    }
    if (d == "flex") {
        return LayoutMode::Flex;
    }
    if (d == "inline") {
        return LayoutMode::Inline;
    }
    if (d == "inline-block") {
        // 澶栧眰鐩掍粛鎸?block 鍙備笌甯冨眬锛岀敱琛屽唴鏍煎紡鍖栦笂涓嬫枃璐熻矗鍏跺唴鑱旇〃鐜?
        return LayoutMode::Block;
    }
    // 鍏跺畠 display 鍊肩洰鍓嶇粺涓€瑙嗕负鍧楃骇鐩?
    return LayoutMode::Block;
}

} // anonymous namespace

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
    
    // 棣栧厛绉婚櫎 CSS 娉ㄩ噴锛?* ... */锛?
    std::string css_no_comments;
    css_no_comments.reserve(css.length());
    size_t i = 0;
    while (i < css.length()) {
        if (i + 1 < css.length() && css[i] == '/' && css[i + 1] == '*') {
            // 鎵惧埌娉ㄩ噴寮€濮嬶紝璺宠繃鐩村埌鎵惧埌 */
            size_t end = css.find("*/", i + 2);
            if (end != std::string::npos) {
                i = end + 2;
            } else {
                // 娌℃湁鎵惧埌娉ㄩ噴缁撴潫锛岃烦杩囧墿浣欏唴瀹?
                break;
            }
        } else {
            css_no_comments += css[i];
            ++i;
        }
    }
    
    size_t pos = 0;
    while (pos < css_no_comments.length()) {
        // Find opening brace
        size_t brace_open = css_no_comments.find('{', pos);
        if (brace_open == std::string::npos) break;
        
        // Find closing brace
        size_t brace_close = css_no_comments.find('}', brace_open);
        if (brace_close == std::string::npos) break;
        
        // Extract selector and declarations
        std::string selector = css_no_comments.substr(pos, brace_open - pos);
        std::string declarations = css_no_comments.substr(brace_open + 1, brace_close - brace_open - 1);
        
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
                // 閲嶈锛氬皢杩欎簺灞炴€ц缃负绌猴紝琛ㄧず"鏈缃?銆?
                // ComputedStyle 鏈夐粯璁ゅ€硷紝浣嗗湪 CSS 瑙勫垯涓紝濡傛灉娌℃湁鏄惧紡璁剧疆杩欎簺灞炴€э紝
                // 鎴戜滑涓嶅簲璇ョ敤榛樿鍊艰鐩栧厓绱犵殑鍘熸湁鍊笺€?
                style.display = "";
                style.background_color = "";
                style.color = "";
                
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
    
    std::string node_class = node->getAttribute("class");
    bool debug_soft = node_class.find("soft") != std::string::npos;
    
    for (const auto& sheet : stylesheets) {
        for (const auto& rule : sheet.getRules()) {
            if (matchesSelector(rule.selector, node)) {
                matching_rules.push_back(rule);
                if (debug_soft) {
                    DONG_LOG_INFO("[computeStyles] soft element matched selector='%s' bg='%s'",
                            rule.selector.c_str(), rule.style.background_color.c_str());
                }
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
        // 鍙湁褰撹鍒欐樉寮忚缃簡灞炴€э紙闈炵┖锛夋椂鎵嶅簲鐢?
        if (!rule.style.color.empty()) {
            computed.color = rule.style.color;
        }
        if (!rule.style.background_color.empty()) {
            computed.background_color = rule.style.background_color;
            if (debug_soft) {
                DONG_LOG_INFO("[computeStyles] soft: applying bg='%s' from selector='%s'",
                        rule.style.background_color.c_str(), rule.selector.c_str());
            }
        }
        if (!rule.style.background_image.empty())
            computed.background_image = rule.style.background_image;
        if (!rule.style.background_size.empty() && rule.style.background_size != "auto")
            computed.background_size = rule.style.background_size;
        if (!rule.style.background_repeat.empty() && rule.style.background_repeat != "repeat")
            computed.background_repeat = rule.style.background_repeat;
        if (!rule.style.background_position.empty())
            computed.background_position = rule.style.background_position;
        if (rule.style.font_size != 16.0f) 
            computed.font_size = rule.style.font_size;
        if (!rule.style.font_weight.empty()) 
            computed.font_weight = rule.style.font_weight;
        if (!rule.style.font_style.empty() && rule.style.font_style != "normal")
            computed.font_style = rule.style.font_style;
        if (!rule.style.font_family.empty() && rule.style.font_family != "Arial")
            computed.font_family = rule.style.font_family;
        if (!rule.style.text_align.empty()) 
            computed.text_align = rule.style.text_align;
        if (!rule.style.text_decoration.empty() && rule.style.text_decoration != "none")
            computed.text_decoration = rule.style.text_decoration;
        if (!rule.style.text_decoration_color.empty())
            computed.text_decoration_color = rule.style.text_decoration_color; 
            computed.text_align = rule.style.text_align;
        if (rule.style.letter_spacing_em != 0.0f)
            computed.letter_spacing_em = rule.style.letter_spacing_em;
        if (rule.style.word_spacing_px != 0.0f)
            computed.word_spacing_px = rule.style.word_spacing_px;
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
        if (!rule.style.visibility.empty() && rule.style.visibility != "visible")
            computed.visibility = rule.style.visibility;
        if (!rule.style.cursor.empty() && rule.style.cursor != "auto")
            computed.cursor = rule.style.cursor;
        if (rule.style.outline_width != 0.0f)
            computed.outline_width = rule.style.outline_width;
        if (!rule.style.outline_color.empty() && rule.style.outline_color != "#000000")
            computed.outline_color = rule.style.outline_color;
        if (!rule.style.outline_style.empty() && rule.style.outline_style != "none")
            computed.outline_style = rule.style.outline_style;
        if (rule.style.outline_offset != 0.0f)
            computed.outline_offset = rule.style.outline_offset;
        if (rule.style.box_sizing != "content-box")
            computed.box_sizing = rule.style.box_sizing;
        if (rule.style.opacity != 1.0f)
            computed.opacity = rule.style.opacity;
        if (rule.style.isolation_isolate)
            computed.isolation_isolate = true;
        
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
        // Position offsets
        if (rule.style.top.unit != CSSValue::Unit::AUTO)
            computed.top = rule.style.top;
        if (rule.style.right.unit != CSSValue::Unit::AUTO)
            computed.right = rule.style.right;
        if (rule.style.bottom.unit != CSSValue::Unit::AUTO)
            computed.bottom = rule.style.bottom;
        if (rule.style.left.unit != CSSValue::Unit::AUTO)
            computed.left = rule.style.left;
        
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
        if (rule.style.gap > 0.0f)
            computed.gap = rule.style.gap;
        
        // Text properties
        if (rule.style.line_height > 0.0f || rule.style.line_height == -1.0f) {
            computed.line_height = rule.style.line_height;
            computed.line_height_is_unitless = rule.style.line_height_is_unitless;
        }
        if (!rule.style.text_transform.empty() && rule.style.text_transform != "none")
            computed.text_transform = rule.style.text_transform;
        if (!rule.style.vertical_align.empty() && rule.style.vertical_align != "baseline")
            computed.vertical_align = rule.style.vertical_align;
        
        // Text shadow
        if (rule.style.text_shadow_offset_x != 0.0f || rule.style.text_shadow_offset_y != 0.0f || 
            rule.style.text_shadow_blur != 0.0f || !rule.style.text_shadow_color.empty()) {
            computed.text_shadow_offset_x = rule.style.text_shadow_offset_x;
            computed.text_shadow_offset_y = rule.style.text_shadow_offset_y;
            computed.text_shadow_blur = rule.style.text_shadow_blur;
            computed.text_shadow_color = rule.style.text_shadow_color;
        }
        // Box shadow
        if (!rule.style.box_shadows.empty()) {
            computed.box_shadows = rule.style.box_shadows;
        }
    }
    
    if (debug_soft) {
        DONG_LOG_INFO("[computeStyles] soft element FINAL bg='%s'", computed.background_color.c_str());
    }

    static const std::unordered_set<std::string> kAlwaysHiddenTags = {
        "head", "style", "script", "meta", "title"
    };
    if (kAlwaysHiddenTags.count(node->getTagName()) > 0) {
        computed.display = "none";
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

    // Derive layout mode from final computed display
    computed.layout_mode = deriveLayoutModeFromDisplay(computed);

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

void StyleEngine::applyInlineStyleProperty(const std::string& property,
                                           const std::string& value,
                                           ComputedStyle& style) {
    StyleEngine engine;
    engine.applyStyleProperty(property, value, style);
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
        // background 简写可能包含多个值，这里简化处理
        // 如果值包含 url(...)，则是 background-image
        if (val.find("url(") != std::string::npos) {
            // 提取 url(...) 部分
            size_t url_start = val.find("url(");
            size_t url_end = val.find(")", url_start);
            if (url_end != std::string::npos) {
                std::string url_part = val.substr(url_start, url_end - url_start + 1);
                style.background_image = url_part;
            }
        } else {
            style.background_color = val;
        }
    }
    else if (prop == "background-image") {
        style.background_image = val;
    }
    else if (prop == "background-size") {
        style.background_size = val;
    }
    else if (prop == "background-repeat") {
        style.background_repeat = val;
    }
    else if (prop == "background-position") {
        style.background_position = val;
    }
    else if (prop == "font-size") {
        style.font_size = parseFloat(val);
    }
    else if (prop == "font-weight") {
        style.font_weight = val;
    }
    else if (prop == "font-style") {
        // font-style: normal | italic | oblique
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (lowered == "italic" || lowered == "oblique") {
            style.font_style = lowered;
        } else {
            style.font_style = "normal";
        }
    }
    else if (prop == "font-family") {
        style.font_family = val;
    }
    else if (prop == "text-align") {
        style.text_align = val;
    }
    else if (prop == "text-decoration" || prop == "text-decoration-line") {
        // text-decoration: none | underline | line-through | overline
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (lowered == "underline" || lowered == "line-through" || lowered == "overline") {
            style.text_decoration = lowered;
        } else {
            style.text_decoration = "none";
        }
    }
    else if (prop == "text-decoration-color") {
        style.text_decoration_color = val;
    }
    else if (prop == "letter-spacing") {
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        float em = 0.0f;
        if (lowered == "normal") {
            em = 0.0f;
        } else {
            em = parseFloat(lowered);
            if (lowered.find("px") != std::string::npos) {
                float font_px = style.font_size > 0.0f ? style.font_size : 16.0f;
                if (font_px > 0.0f) {
                    em = em / font_px;
                }
            }
        }
        style.letter_spacing_em = em;
    }
    else if (prop == "word-spacing") {
        // word-spacing: normal | <length>
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        float px = 0.0f;
        if (lowered == "normal") {
            px = 0.0f;
        } else {
            px = parseFloat(lowered);
            // 濡傛灉鏄?em 鍗曚綅锛岃浆鎹负鍍忕礌
            if (lowered.find("em") != std::string::npos) {
                float font_px = style.font_size > 0.0f ? style.font_size : 16.0f;
                px = px * font_px;
            }
            // px 鍗曚綅鎴栨棤鍗曚綅鏁板瓧鐩存帴浣跨敤
        }
        style.word_spacing_px = px;
    }
    else if (prop == "line-height") {
        // line-height: normal | <number> | <length> | <percentage>
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (lowered == "normal") {
            style.line_height = -1.0f;
            style.line_height_is_unitless = true;
        } else if (lowered.find("px") != std::string::npos) {
            // 鍍忕礌鍊?
            style.line_height = parseFloat(lowered);
            style.line_height_is_unitless = false;
        } else if (lowered.find('%') != std::string::npos) {
            // 鐧惧垎姣旓細杞崲涓哄€嶆暟
            style.line_height = parseFloat(lowered) / 100.0f;
            style.line_height_is_unitless = true;
        } else {
            // 鏃犲崟浣嶆暟瀛楋紙鍊嶆暟锛?
            style.line_height = parseFloat(lowered);
            style.line_height_is_unitless = true;
        }
    }
    else if (prop == "text-transform") {
        style.text_transform = val;
    }
    else if (prop == "text-shadow") {
        // text-shadow: <offset-x> <offset-y> [<blur-radius>] <color>
        // 简化解析：仅支持单个 text-shadow
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        
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
            style.text_shadow_color = color;
        }
    }
    else if (prop == "vertical-align") {
        style.vertical_align = val;
    }
    else if (prop == "display") {
        style.display = val;
    }
    else if (prop == "position") {
        style.position = val;
    }
    else if (prop == "top") {
        style.top = parseLength(val);
    }
    else if (prop == "right") {
        style.right = parseLength(val);
    }
    else if (prop == "bottom") {
        style.bottom = parseLength(val);
    }
    else if (prop == "left") {
        style.left = parseLength(val);
    }
    else if (prop == "inset") {
        // CSS inset shorthand: top/right/bottom/left
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        if (parts.size() == 1) {
            auto v = parseLength(parts[0]);
            style.top = style.right = style.bottom = style.left = v;
        } else if (parts.size() == 2) {
            auto v1 = parseLength(parts[0]); // block (top/bottom)
            auto v2 = parseLength(parts[1]); // inline (left/right)
            style.top = style.bottom = v1;
            style.left = style.right = v2;
        } else if (parts.size() == 3) {
            auto v1 = parseLength(parts[0]); // top
            auto v2 = parseLength(parts[1]); // inline
            auto v3 = parseLength(parts[2]); // bottom
            style.top = v1;
            style.left = style.right = v2;
            style.bottom = v3;
        } else if (parts.size() >= 4) {
            style.top = parseLength(parts[0]);
            style.right = parseLength(parts[1]);
            style.bottom = parseLength(parts[2]);
            style.left = parseLength(parts[3]);
        }
    }
    else if (prop == "z-index") {
        // 浠呮敮鎸佹暣鏁?z-index锛岄潪娉曞€兼寜 0 澶勭悊
        try {
            style.z_index = std::stoi(val);
        } catch (...) {
            style.z_index = 0;
        }
    }
    else if (prop == "overflow") {
        style.overflow = val;
    }
    else if (prop == "visibility") {
        // visibility: visible | hidden | collapse
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (lowered == "hidden" || lowered == "collapse") {
            style.visibility = lowered;
        } else {
            style.visibility = "visible";
        }
    }
    else if (prop == "box-sizing") {
        // box-sizing: content-box | border-box
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (lowered == "border-box") {
            style.box_sizing = "border-box";
        } else {
            style.box_sizing = "content-box";
        }
    }
    else if (prop == "cursor") {
        // cursor: auto | default | pointer | text | move | wait | help | crosshair | not-allowed | grab | grabbing
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        style.cursor = lowered;
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
    else if (prop == "gap") {
        style.gap = parseFloat(val);
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
    else if (prop == "border") {
        // border 绠€鍐欙細border: <width> <style> <color>
        // 鐩墠浠呰В鏋?width 鍜?color锛宻tyle 鏆傛椂蹇界暐
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        for (const auto& p : parts) {
            // 灏濊瘯瑙ｆ瀽涓哄搴︼紙浠ユ暟瀛楀紑澶达級
            if (!p.empty() && (std::isdigit(static_cast<unsigned char>(p[0])) || p[0] == '.')) {
                style.border_width = parseFloat(p);
            }
            // 灏濊瘯瑙ｆ瀽涓洪鑹诧紙浠?# 鎴栧瓧姣嶅紑澶达紝鎺掗櫎 style 鍏抽敭瀛楋級
            else if (!p.empty() && (p[0] == '#' || 
                     (std::isalpha(static_cast<unsigned char>(p[0])) &&
                      p != "solid" && p != "dashed" && p != "dotted" && 
                      p != "double" && p != "none" && p != "hidden"))) {
                style.border_color = p;
            }
            // border-style 鏆傛椂蹇界暐锛坰olid/dashed/dotted 绛夛級
        }
    }
    else if (prop == "border-top" || prop == "border-right" || 
             prop == "border-bottom" || prop == "border-left") {
        // 鍗曡竟 border 绠€鍐欙紝鐩墠绠€鍖栧鐞嗕负鍏ㄥ眬 border
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        for (const auto& p : parts) {
            if (!p.empty() && (std::isdigit(static_cast<unsigned char>(p[0])) || p[0] == '.')) {
                style.border_width = parseFloat(p);
            }
            else if (!p.empty() && (p[0] == '#' || 
                     (std::isalpha(static_cast<unsigned char>(p[0])) &&
                      p != "solid" && p != "dashed" && p != "dotted" && 
                      p != "double" && p != "none" && p != "hidden"))) {
                style.border_color = p;
            }
        }
    }
    else if (prop == "outline") {
        // outline 简写：outline: <width> <style> <color>
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        for (const auto& p : parts) {
            if (!p.empty() && (std::isdigit(static_cast<unsigned char>(p[0])) || p[0] == '.')) {
                style.outline_width = parseFloat(p);
            }
            else if (p == "solid" || p == "dashed" || p == "dotted" || p == "none") {
                style.outline_style = p;
            }
            else if (!p.empty() && (p[0] == '#' || std::isalpha(static_cast<unsigned char>(p[0])))) {
                style.outline_color = p;
            }
        }
    }
    else if (prop == "outline-width") {
        style.outline_width = parseFloat(val);
    }
    else if (prop == "outline-color") {
        style.outline_color = val;
    }
    else if (prop == "outline-style") {
        style.outline_style = val;
    }
    else if (prop == "outline-offset") {
        style.outline_offset = parseFloat(val);
    }
    else if (prop == "box-shadow") {
        style.box_shadows.clear();
        std::string src = val;

        // 鎸夐€楀彿鎷嗗垎澶氫釜闃村奖锛屾敞鎰忚烦杩?rgba(...) 鍐呴儴鐨勯€楀彿
        std::vector<std::string> shadow_parts;
        int paren_depth = 0;
        std::string current;
        for (char c : src) {
            if (c == '(') {
                ++paren_depth;
                current.push_back(c);
            } else if (c == ')') {
                --paren_depth;
                current.push_back(c);
            } else if (c == ',' && paren_depth == 0) {
                std::string trimmed = trimWhitespace(current);
                if (!trimmed.empty()) {
                    shadow_parts.push_back(trimmed);
                }
                current.clear();
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            std::string trimmed = trimWhitespace(current);
            if (!trimmed.empty()) {
                shadow_parts.push_back(trimmed);
            }
        }

        for (const std::string& part_raw : shadow_parts) {
            std::string part = trimWhitespace(part_raw);
            if (part.empty() || part == "none") {
                continue;
            }

            // 浠庡乏鍒板彸鍏堝彇 offset/blur/spread锛岄亣鍒扮涓€涓互瀛楁瘝鎴?'#' 寮€澶寸殑 token 鍚庯紝鍓╀綑閮ㄥ垎褰撲綔棰滆壊
            std::vector<std::string> length_tokens;
            size_t color_start = std::string::npos;

            size_t i = 0;
            const size_t n = part.size();
            while (i < n) {
                while (i < n && std::isspace(static_cast<unsigned char>(part[i]))) {
                    ++i;
                }
                if (i >= n) break;
                size_t token_start = i;
                while (i < n && !std::isspace(static_cast<unsigned char>(part[i]))) {
                    ++i;
                }
                size_t token_end = i;
                if (token_end <= token_start) break;
                std::string token = part.substr(token_start, token_end - token_start);

                if (!token.empty() && (token[0] == '#' || std::isalpha(static_cast<unsigned char>(token[0])))) {
                    color_start = token_start;
                    break;
                }
                length_tokens.push_back(token);
            }

            BoxShadow shadow;

            if (!length_tokens.empty()) {
                // offset-x, offset-y, blur-radius, spread-radius
                if (length_tokens.size() >= 1) {
                    shadow.offset_x = parseLength(length_tokens[0]).value;
                }
                if (length_tokens.size() >= 2) {
                    shadow.offset_y = parseLength(length_tokens[1]).value;
                }
                if (length_tokens.size() >= 3) {
                    shadow.blur_radius = parseLength(length_tokens[2]).value;
                }
                if (length_tokens.size() >= 4) {
                    shadow.spread_radius = parseLength(length_tokens[3]).value;
                }
            }

            if (color_start != std::string::npos) {
                std::string color_str = trimWhitespace(part.substr(color_start));
                if (!color_str.empty()) {
                    shadow.color = color_str;
                }
            }

            // 鑻ユ湭鎻愪緵棰滆壊锛屽垯浣跨敤褰撳墠鏂囨湰鑹蹭綔涓洪槾褰遍鑹茬殑鍩哄噯
            if (shadow.color.empty()) {
                shadow.color = style.color;
            }

            style.box_shadows.push_back(shadow);
        }
    }
    else if (prop == "opacity") {
        float v = parseFloat(val);
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        style.opacity = v;
    }
    else if (prop == "isolation") {
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        style.isolation_isolate = (lowered == "isolate");
    }
    else if (prop == "transform") {
        // 鏋佺畝 transform 鏀寔锛氬彧瑙ｆ瀽 translate/scale锛岀敤浜?LayerTree 鍥惧眰骞崇Щ/缂╂斁
        std::string lowered = val;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        // 榛樿閲嶇疆涓?identity
        style.transform_translate_x = 0.0f;
        style.transform_translate_y = 0.0f;
        style.transform_scale_x = 1.0f;
        style.transform_scale_y = 1.0f;

        auto trim = [this](const std::string& s) {
            return trimWhitespace(s);
        };

        if (lowered == "none") {
            return;
        }

        auto parseArgs = [&trim](const std::string& args) -> std::vector<std::string> {
            std::vector<std::string> result;
            std::string current;
            for (char c : args) {
                if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
                    if (!current.empty()) {
                        result.push_back(trim(current));
                        current.clear();
                    }
                } else {
                    current.push_back(c);
                }
            }
            if (!current.empty()) {
                result.push_back(trim(current));
            }
            return result;
        };

        auto parseLengthToPx = [&](const std::string& s) -> float {
            CSSValue v = parseLength(s);
            if (v.unit == CSSValue::Unit::PIXEL) {
                return v.value;
            }
            return 0.0f;
        };

        auto handleFunc = [&](const std::string& func_name, const std::string& args_raw) {
            std::string name = trim(func_name);
            std::string args = trim(args_raw);

            if (name == "translate" || name == "translate3d") {
                auto parts = parseArgs(args);
                if (!parts.empty()) {
                    style.transform_translate_x = parseLengthToPx(parts[0]);
                    if (parts.size() > 1) {
                        style.transform_translate_y = parseLengthToPx(parts[1]);
                    }
                }
            } else if (name == "translatex") {
                style.transform_translate_x = parseLengthToPx(args);
            } else if (name == "translatey") {
                style.transform_translate_y = parseLengthToPx(args);
            } else if (name == "scale") {
                auto parts = parseArgs(args);
                if (!parts.empty()) {
                    float sx = parseFloat(parts[0]);
                    float sy = (parts.size() > 1) ? parseFloat(parts[1]) : sx;
                    if (sx != 0.0f) style.transform_scale_x = sx;
                    if (sy != 0.0f) style.transform_scale_y = sy;
                }
            } else if (name == "scalex") {
                float sx = parseFloat(args);
                if (sx != 0.0f) style.transform_scale_x = sx;
            } else if (name == "scaley") {
                float sy = parseFloat(args);
                if (sy != 0.0f) style.transform_scale_y = sy;
            } else if (name == "rotate") {
                float angle = parseFloat(args);
                if (args.find("rad") != std::string::npos) {
                    angle = angle * 180.0f / 3.14159265358979f;
                } else if (args.find("turn") != std::string::npos) {
                    angle = angle * 360.0f;
                }
                style.transform_rotate = angle;
            } else if (name == "skew") {
                auto parts = parseArgs(args);
                if (!parts.empty()) {
                    float ax = parseFloat(parts[0]);
                    float ay = (parts.size() > 1) ? parseFloat(parts[1]) : 0.0f;
                    style.transform_skew_x = ax;
                    style.transform_skew_y = ay;
                }
            } else if (name == "skewx") {
                style.transform_skew_x = parseFloat(args);
            } else if (name == "skewy") {
                style.transform_skew_y = parseFloat(args);
            }
        };

        // 鍏佽 transform 閲屽悓鏃跺嚭鐜?scale/translate锛岄『搴忕洰鍓嶄笉褰卞搷锛堜粎鍋氱畝鍗曠粍鍚堬級
        size_t pos = 0;
        while (pos < lowered.size()) {
            size_t lparen = lowered.find('(', pos);
            if (lparen == std::string::npos) break;
            size_t rparen = lowered.find(')', lparen + 1);
            if (rparen == std::string::npos) break;

            std::string func_name = lowered.substr(pos, lparen - pos);
            std::string args = lowered.substr(lparen + 1, rparen - lparen - 1);
            handleFunc(func_name, args);

            pos = rparen + 1;
        }
    }
    else if (prop == "transform-origin") {
        // transform-origin: <x> <y>
        // 支持: left/center/right/top/bottom 或百分比或像素值
        std::istringstream iss(val);
        std::string part;
        std::vector<std::string> parts;
        while (iss >> part) {
            parts.push_back(part);
        }
        
        auto parseOriginValue = [&](const std::string& v) -> float {
            std::string lowered = v;
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (lowered == "left" || lowered == "top") return 0.0f;
            if (lowered == "center") return 50.0f;
            if (lowered == "right" || lowered == "bottom") return 100.0f;
            if (lowered.find('%') != std::string::npos) {
                return parseFloat(lowered);
            }
            // 像素值暂时转换为百分比（假设 100px = 50%）
            return parseFloat(lowered) * 0.5f;
        };
        
        if (!parts.empty()) {
            style.transform_origin_x = parseOriginValue(parts[0]);
            if (parts.size() > 1) {
                style.transform_origin_y = parseOriginValue(parts[1]);
            } else {
                style.transform_origin_y = style.transform_origin_x;
            }
        }
    }
}

std::pair<std::string, ComputedStyle> StyleEngine::parseRule(const std::string& rule_str) {
    ComputedStyle style;
    // 閲嶈锛氬皢 display 璁剧疆涓虹┖锛岃〃绀?鏈缃?銆?
    // ComputedStyle 鐨勯粯璁ゅ€兼槸 "block"锛屼絾鍦?CSS 瑙勫垯涓紝濡傛灉娌℃湁鏄惧紡璁剧疆 display锛?
    // 鎴戜滑涓嶅簲璇ョ敤榛樿鍊艰鐩栧厓绱犵殑鍘熸湁 display 鍊笺€?
    style.display = "";
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
