#include "style_engine.hpp"
#include "../dom/dom_node.hpp"
#include "../../core/log.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <unordered_set>

namespace dong::dom {

namespace {

LayoutMode deriveLayoutModeFromDisplay(const ComputedStyle& style) {
    const std::string& d = style.display;
    if (d == "none") {
        return LayoutMode::None;
    }
    if (d == "flex" || d == "inline-flex") {
        return LayoutMode::Flex;
    }
    if (d == "inline") {
        return LayoutMode::Inline;
    }
    return LayoutMode::Block;
}

} // anonymous namespace

void Stylesheet::addRule(const std::string& selector, const ComputedStyle& style, 
                         int specificity, int order) {
    rules_.push_back({selector, style, specificity, order});
}

void Stylesheet::addKeyframes(const KeyframesRule& keyframes) {
    keyframes_[keyframes.name] = keyframes;
}

const KeyframesRule* Stylesheet::getKeyframes(const std::string& name) const {
    auto it = keyframes_.find(name);
    return it != keyframes_.end() ? &it->second : nullptr;
}

void Stylesheet::addFontFace(const FontFaceRule& font_face) {
    font_faces_.push_back(font_face);
}

StyleEngine::StyleEngine() = default;

void StyleEngine::addStylesheet(const std::string& css) {
    Stylesheet sheet;
    auto rules = parseCSS(css);
    for (const auto& rule : rules) {
        sheet.addRule(rule.selector, rule.style, rule.specificity, rule.source_order);
    }
    
    // Parse and add keyframes
    auto keyframes = parser_.parseKeyframes(css);
    for (const auto& kf : keyframes) {
        sheet.addKeyframes(kf);
    }
    
    // Parse and add font-faces
    auto font_faces = parser_.parseFontFaceRules(css);
    for (const auto& ff : font_faces) {
        sheet.addFontFace(ff);
    }
    
    stylesheets_.push_back(sheet);
}

void StyleEngine::addStylesheet(const Stylesheet& sheet) {
    stylesheets_.push_back(sheet);
}

std::vector<CSSRule> StyleEngine::parseCSS(const std::string& css) {
    return parser_.parse(css);
}

void StyleEngine::applyInlineStyleProperty(const std::string& property,
                                           const std::string& value,
                                           ComputedStyle& style) {
    CSSParser::applyProperty(property, value, style);
}

void StyleEngine::computeStyles(DOMNodePtr node) {
    if (!node) return;

    // Apply matching rules
    applyMatchingRules(node);
    
    // Inherit from parent
    inheritFromParent(node);
    
    // Hide certain elements
    static const std::unordered_set<std::string> kAlwaysHiddenTags = {
        "head", "style", "script", "meta", "title", "link"
    };
    if (kAlwaysHiddenTags.count(node->getTagName()) > 0) {
        node->getComputedStyle().display = "none";
    }

    // Derive layout mode
    node->getComputedStyle().layout_mode = deriveLayoutModeFromDisplay(node->getComputedStyle());

    // Process pseudo-elements (::before/::after)
    processPseudoElements(node);

    // Recursively compute styles for children
    for (const auto& child : node->getChildren()) {
        computeStyles(child);
    }
}

void StyleEngine::recomputeNodeStyle(DOMNodePtr node) {
    if (!node) return;
    applyMatchingRules(node);
    inheritFromParent(node);
    node->getComputedStyle().layout_mode = deriveLayoutModeFromDisplay(node->getComputedStyle());
}

void StyleEngine::applyMatchingRules(DOMNodePtr node) {
    std::vector<CSSRule> matching_rules;
    
    for (const auto& sheet : stylesheets_) {
        for (const auto& rule : sheet.getRules()) {
            if (matcher_.matches(rule.selector, node)) {
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
    
    auto& computed = node->getComputedStyle();
    
    for (const auto& rule : matching_rules) {
        const auto& rs = rule.style;
        
        // Apply non-default values
        if (!rs.color.empty() && rs.color != "#000000") computed.color = rs.color;
        if (!rs.background_color.empty() && rs.background_color != "transparent") 
            computed.background_color = rs.background_color;
        if (!rs.background_image.empty()) computed.background_image = rs.background_image;
        if (!rs.background_size.empty() && rs.background_size != "auto") 
            computed.background_size = rs.background_size;
        if (!rs.background_repeat.empty() && rs.background_repeat != "repeat") 
            computed.background_repeat = rs.background_repeat;
        if (!rs.background_position.empty()) computed.background_position = rs.background_position;
        if (!rs.object_fit.empty() && rs.object_fit != "fill") computed.object_fit = rs.object_fit;
        if (!rs.background_gradients.empty()) computed.background_gradients = rs.background_gradients;

        
        if (rs.font_size != 16.0f) computed.font_size = rs.font_size;
        if (!rs.font_weight.empty() && rs.font_weight != "normal") computed.font_weight = rs.font_weight;
        if (!rs.font_style.empty() && rs.font_style != "normal") computed.font_style = rs.font_style;
        if (!rs.font_family.empty() && rs.font_family != "Arial") computed.font_family = rs.font_family;
        if (!rs.font_variant.empty() && rs.font_variant != "normal") computed.font_variant = rs.font_variant;
        
        if (!rs.text_align.empty() && rs.text_align != "left") computed.text_align = rs.text_align;
        if (!rs.text_decoration.empty() && rs.text_decoration != "none") 
            computed.text_decoration = rs.text_decoration;
        if (!rs.text_decoration_color.empty()) computed.text_decoration_color = rs.text_decoration_color;
        if (!rs.text_decoration_style.empty() && rs.text_decoration_style != "solid")
            computed.text_decoration_style = rs.text_decoration_style;
        if (rs.text_decoration_thickness != 1.0f) 
            computed.text_decoration_thickness = rs.text_decoration_thickness;
        if (rs.letter_spacing_em != 0.0f) computed.letter_spacing_em = rs.letter_spacing_em;
        if (rs.word_spacing_px != 0.0f) computed.word_spacing_px = rs.word_spacing_px;
        if (rs.has_line_height) {
            computed.has_line_height = true;
            computed.line_height = rs.line_height;
            computed.line_height_is_unitless = rs.line_height_is_unitless;
        }
        if (!rs.text_transform.empty() && rs.text_transform != "none") 
            computed.text_transform = rs.text_transform;

        if (!rs.text_overflow.empty() && rs.text_overflow != "clip") 
            computed.text_overflow = rs.text_overflow;
        if (!rs.white_space.empty() && rs.white_space != "normal") 
            computed.white_space = rs.white_space;
        if (!rs.word_break.empty() && rs.word_break != "normal") 
            computed.word_break = rs.word_break;
        if (!rs.overflow_wrap.empty() && rs.overflow_wrap != "normal") 
            computed.overflow_wrap = rs.overflow_wrap;
        if (!rs.vertical_align.empty() && rs.vertical_align != "baseline") 
            computed.vertical_align = rs.vertical_align;
        if (!rs.direction.empty() && rs.direction != "ltr") computed.direction = rs.direction;
        if (rs.text_indent != 0.0f) computed.text_indent = rs.text_indent;
        if (rs.webkit_line_clamp != 0) computed.webkit_line_clamp = rs.webkit_line_clamp;
        
        // Text shadow
        if (rs.text_shadow_offset_x != 0.0f || rs.text_shadow_offset_y != 0.0f || 
            rs.text_shadow_blur != 0.0f || !rs.text_shadow_color.empty()) {
            computed.text_shadow_offset_x = rs.text_shadow_offset_x;
            computed.text_shadow_offset_y = rs.text_shadow_offset_y;
            computed.text_shadow_blur = rs.text_shadow_blur;
            computed.text_shadow_color = rs.text_shadow_color;
        }
        
        if (!rs.display.empty()) {
            computed.display = rs.display;
            computed.layout_mode = deriveLayoutModeFromDisplay(rs.display);
        }
        if (!rs.position.empty() && rs.position != "static") computed.position = rs.position;
        
        if (rs.border_radius != 0.0f) {
            computed.border_radius = rs.border_radius;
            computed.border_top_left_radius = rs.border_radius;
            computed.border_top_right_radius = rs.border_radius;
            computed.border_bottom_left_radius = rs.border_radius;
            computed.border_bottom_right_radius = rs.border_radius;
        }
        if (rs.border_top_left_radius != 0.0f) 
            computed.border_top_left_radius = rs.border_top_left_radius;
        if (rs.border_top_right_radius != 0.0f) 
            computed.border_top_right_radius = rs.border_top_right_radius;
        if (rs.border_bottom_left_radius != 0.0f) 
            computed.border_bottom_left_radius = rs.border_bottom_left_radius;
        if (rs.border_bottom_right_radius != 0.0f) 
            computed.border_bottom_right_radius = rs.border_bottom_right_radius;
        
        if (rs.border_width != 0.0f) computed.border_width = rs.border_width;
        if (!rs.border_color.empty() && rs.border_color != "#000000") 
            computed.border_color = rs.border_color;
        if (!rs.border_style.empty() && rs.border_style != "none") 
            computed.border_style = rs.border_style;
        
        if (!rs.overflow.empty() && rs.overflow != "visible") {
            computed.overflow = rs.overflow;
            computed.overflow_x = rs.overflow;
            computed.overflow_y = rs.overflow;
        }
        if (!rs.overflow_x.empty() && rs.overflow_x != "visible") computed.overflow_x = rs.overflow_x;
        if (!rs.overflow_y.empty() && rs.overflow_y != "visible") computed.overflow_y = rs.overflow_y;
        if (!rs.visibility.empty() && rs.visibility != "visible") computed.visibility = rs.visibility;
        if (!rs.cursor.empty() && rs.cursor != "auto") computed.cursor = rs.cursor;
        
        if (rs.outline_width != 0.0f) computed.outline_width = rs.outline_width;
        if (!rs.outline_color.empty() && rs.outline_color != "#000000") 
            computed.outline_color = rs.outline_color;
        if (!rs.outline_style.empty() && rs.outline_style != "none") 
            computed.outline_style = rs.outline_style;
        if (rs.outline_offset != 0.0f) computed.outline_offset = rs.outline_offset;
        
        if (!rs.box_sizing.empty() && rs.box_sizing != "content-box") 
            computed.box_sizing = rs.box_sizing;
        if (rs.opacity != 1.0f) computed.opacity = rs.opacity;
        if (rs.isolation_isolate) computed.isolation_isolate = true;
        if (!rs.box_shadows.empty()) computed.box_shadows = rs.box_shadows;
        
        // Filters
        if (!rs.filters.empty()) computed.filters = rs.filters;
        if (!rs.backdrop_filters.empty()) computed.backdrop_filters = rs.backdrop_filters;
        if (!rs.mix_blend_mode.empty() && rs.mix_blend_mode != "normal") 
            computed.mix_blend_mode = rs.mix_blend_mode;
        if (!rs.background_blend_mode.empty() && rs.background_blend_mode != "normal")
            computed.background_blend_mode = rs.background_blend_mode;
        
        // Box model
        if (rs.width.unit != CSSValue::Unit::AUTO) computed.width = rs.width;
        if (rs.height.unit != CSSValue::Unit::AUTO) computed.height = rs.height;
        if (rs.min_width.unit != CSSValue::Unit::AUTO) computed.min_width = rs.min_width;
        if (rs.max_width.unit != CSSValue::Unit::AUTO) computed.max_width = rs.max_width;
        if (rs.min_height.unit != CSSValue::Unit::AUTO) computed.min_height = rs.min_height;
        if (rs.max_height.unit != CSSValue::Unit::AUTO) computed.max_height = rs.max_height;
        
        if (rs.margin_top.unit != CSSValue::Unit::AUTO || rs.margin_top.value != 0.0f) 
            computed.margin_top = rs.margin_top;
        if (rs.margin_right.unit != CSSValue::Unit::AUTO || rs.margin_right.value != 0.0f) 
            computed.margin_right = rs.margin_right;
        if (rs.margin_bottom.unit != CSSValue::Unit::AUTO || rs.margin_bottom.value != 0.0f) 
            computed.margin_bottom = rs.margin_bottom;
        if (rs.margin_left.unit != CSSValue::Unit::AUTO || rs.margin_left.value != 0.0f) 
            computed.margin_left = rs.margin_left;
        
        if (rs.padding_top.value != 0.0f) computed.padding_top = rs.padding_top;
        if (rs.padding_right.value != 0.0f) computed.padding_right = rs.padding_right;
        if (rs.padding_bottom.value != 0.0f) computed.padding_bottom = rs.padding_bottom;
        if (rs.padding_left.value != 0.0f) computed.padding_left = rs.padding_left;
        
        if (rs.top.unit != CSSValue::Unit::AUTO) computed.top = rs.top;
        if (rs.right.unit != CSSValue::Unit::AUTO) computed.right = rs.right;
        if (rs.bottom.unit != CSSValue::Unit::AUTO) computed.bottom = rs.bottom;
        if (rs.left.unit != CSSValue::Unit::AUTO) computed.left = rs.left;
        if (rs.z_index != 0) computed.z_index = rs.z_index;
        
        // Flexbox
        if (!rs.flex_direction.empty() && rs.flex_direction != "row") 
            computed.flex_direction = rs.flex_direction;
        if (!rs.flex_wrap.empty() && rs.flex_wrap != "nowrap") computed.flex_wrap = rs.flex_wrap;
        if (!rs.justify_content.empty() && rs.justify_content != "flex-start") 
            computed.justify_content = rs.justify_content;
        if (!rs.align_items.empty() && rs.align_items != "stretch") 
            computed.align_items = rs.align_items;
        if (!rs.align_content.empty() && rs.align_content != "stretch") 
            computed.align_content = rs.align_content;
        if (!rs.align_self.empty() && rs.align_self != "auto") computed.align_self = rs.align_self;
        if (rs.flex != 0.0f) computed.flex = rs.flex;
        if (rs.flex_grow != 0.0f) computed.flex_grow = rs.flex_grow;
        if (rs.flex_shrink != 1.0f) computed.flex_shrink = rs.flex_shrink;
        if (rs.flex_basis.unit != CSSValue::Unit::AUTO) computed.flex_basis = rs.flex_basis;
        if (rs.order != 0) computed.order = rs.order;
        if (rs.gap > 0.0f) {
            computed.gap = rs.gap;
            computed.row_gap = rs.gap;
            computed.column_gap = rs.gap;
        }
        if (rs.row_gap > 0.0f) computed.row_gap = rs.row_gap;
        if (rs.column_gap > 0.0f) computed.column_gap = rs.column_gap;
        
        // Transform
        if (rs.transform_translate_x != 0.0f) computed.transform_translate_x = rs.transform_translate_x;
        if (rs.transform_translate_y != 0.0f) computed.transform_translate_y = rs.transform_translate_y;
        if (rs.transform_scale_x != 1.0f) computed.transform_scale_x = rs.transform_scale_x;
        if (rs.transform_scale_y != 1.0f) computed.transform_scale_y = rs.transform_scale_y;
        if (rs.transform_rotate != 0.0f) computed.transform_rotate = rs.transform_rotate;
        if (rs.transform_skew_x != 0.0f) computed.transform_skew_x = rs.transform_skew_x;
        if (rs.transform_skew_y != 0.0f) computed.transform_skew_y = rs.transform_skew_y;
        if (rs.transform_origin_x != 50.0f) computed.transform_origin_x = rs.transform_origin_x;
        if (rs.transform_origin_y != 50.0f) computed.transform_origin_y = rs.transform_origin_y;
        if (!rs.transform_style.empty() && rs.transform_style != "flat") 
            computed.transform_style = rs.transform_style;
        if (rs.perspective != 0.0f) computed.perspective = rs.perspective;
        if (!rs.backface_visibility.empty() && rs.backface_visibility != "visible")
            computed.backface_visibility = rs.backface_visibility;
        
        // Transitions and animations
        if (!rs.transitions.empty()) computed.transitions = rs.transitions;
        if (!rs.animations.empty()) computed.animations = rs.animations;
        
        // Clip path
        if (!rs.clip_path.empty()) computed.clip_path = rs.clip_path;
        
        // Pointer events
        if (!rs.pointer_events.empty() && rs.pointer_events != "auto") 
            computed.pointer_events = rs.pointer_events;
        if (!rs.user_select.empty() && rs.user_select != "auto") 
            computed.user_select = rs.user_select;
        if (!rs.touch_action.empty() && rs.touch_action != "auto") 
            computed.touch_action = rs.touch_action;
        if (!rs.caret_color.empty() && rs.caret_color != "auto") 
            computed.caret_color = rs.caret_color;
        
        // Float/Clear
        if (!rs.float_value.empty() && rs.float_value != "none") 
            computed.float_value = rs.float_value;
        if (!rs.clear.empty() && rs.clear != "none") computed.clear = rs.clear;
    }
}

void StyleEngine::inheritFromParent(DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return;
    
    auto& computed = node->getComputedStyle();
    const auto& parent_style = parent->getComputedStyle();
    
    // Inherit text properties
    if (computed.color == "#000000") computed.color = parent_style.color;
    if (computed.font_family == "Arial") computed.font_family = parent_style.font_family;
    if (computed.font_size == 16.0f) computed.font_size = parent_style.font_size;
    if (computed.font_weight == "normal") computed.font_weight = parent_style.font_weight;
    if (computed.font_style == "normal") computed.font_style = parent_style.font_style;
    if (computed.text_align == "left") computed.text_align = parent_style.text_align;
    if (!computed.has_line_height) {
        computed.line_height = parent_style.line_height;
        computed.line_height_is_unitless = parent_style.line_height_is_unitless;
    }

    if (computed.letter_spacing_em == 0.0f) computed.letter_spacing_em = parent_style.letter_spacing_em;
    if (computed.word_spacing_px == 0.0f) computed.word_spacing_px = parent_style.word_spacing_px;
    if (computed.white_space == "normal") computed.white_space = parent_style.white_space;
    if (computed.direction == "ltr") computed.direction = parent_style.direction;
    if (computed.cursor == "auto") computed.cursor = parent_style.cursor;
}

bool StyleEngine::matches(const std::string& selector, DOMNodePtr node) {
    return matcher_.matches(selector, node);
}

const KeyframesRule* StyleEngine::getKeyframes(const std::string& name) const {
    for (const auto& sheet : stylesheets_) {
        if (auto kf = sheet.getKeyframes(name)) {
            return kf;
        }
    }
    return nullptr;
}

void StyleEngine::setViewportSize(float width, float height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

bool StyleEngine::evaluateMediaQuery(const std::string& query) const {
    // Simplified media query evaluation
    std::string q = query;
    std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    if (q.find("screen") != std::string::npos) return true;
    if (q.find("all") != std::string::npos) return true;
    
    // Check min-width
    size_t min_width_pos = q.find("min-width");
    if (min_width_pos != std::string::npos) {
        size_t colon = q.find(':', min_width_pos);
        if (colon != std::string::npos) {
            float min_width = 0.0f;
            try {
                min_width = std::stof(q.substr(colon + 1));
            } catch (...) {}
            if (viewport_width_ < min_width) return false;
        }
    }
    
    // Check max-width
    size_t max_width_pos = q.find("max-width");
    if (max_width_pos != std::string::npos) {
        size_t colon = q.find(':', max_width_pos);
        if (colon != std::string::npos) {
            float max_width = 0.0f;
            try {
                max_width = std::stof(q.substr(colon + 1));
            } catch (...) {}
            if (viewport_width_ > max_width) return false;
        }
    }
    
    return true;
}

LayoutMode StyleEngine::deriveLayoutMode(const ComputedStyle& style) {
    return deriveLayoutModeFromDisplay(style);
}

bool StyleEngine::matchesSelector(const std::string& selector, DOMNodePtr node) {
    return matcher_.matches(selector, node);
}

int StyleEngine::calculateSpecificity(const std::string& selector) {
    return CSSParser::calculateSpecificity(selector);
}

int StyleEngine::countIdSelectors(const std::string& selector) {
    int count = 0;
    for (size_t i = 0; i < selector.length(); ++i) {
        if (selector[i] == '#') {
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
        
        if (std::isalpha(c) && !in_selector) {
            if (i == 0 || selector[i - 1] == ' ' || selector[i - 1] == '>' || 
                selector[i - 1] == '+' || selector[i - 1] == '~') {
                ++count;
            }
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

std::pair<std::string, ComputedStyle> StyleEngine::parseRule(const std::string& rule_str) {
    ComputedStyle style;
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

void StyleEngine::applyStyleProperty(const std::string& property, const std::string& value, 
                                      ComputedStyle& style) {
    CSSParser::applyProperty(property, value, style);
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
                    parts.push_back({SelectorPart::Type::PSEUDO_CLASS, component.substr(1), ""});
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
        while (pos < selector.length() && 
               (std::isalnum(selector[pos]) || selector[pos] == '-' || selector[pos] == '_')) {
            component += selector[pos++];
        }
    } else if (selector[pos] == '#') {
        component += '#';
        ++pos;
        while (pos < selector.length() && 
               (std::isalnum(selector[pos]) || selector[pos] == '-' || selector[pos] == '_')) {
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
        while (pos < selector.length() && 
               (std::isalpha(selector[pos]) || selector[pos] == '-')) {
            component += selector[pos++];
        }
    } else {
        while (pos < selector.length() && 
               (std::isalnum(selector[pos]) || selector[pos] == '-')) {
            component += selector[pos++];
        }
    }
    
    return component;
}

void StyleEngine::processPseudoElements(DOMNodePtr node) {
    if (!node || node->getType() != DOMNode::NodeType::ELEMENT) return;
    
    // Check for ::before rules
    bool has_before = false;
    bool has_after = false;
    ComputedStyle before_style;
    ComputedStyle after_style;
    
    for (const auto& sheet : stylesheets_) {
        for (const auto& rule : sheet.getRules()) {
            // Check for ::before
            if (rule.selector.find("::before") != std::string::npos ||
                rule.selector.find(":before") != std::string::npos) {
                // Extract base selector
                std::string base_selector = rule.selector;
                size_t pos = base_selector.find("::before");
                if (pos == std::string::npos) pos = base_selector.find(":before");
                if (pos != std::string::npos) {
                    base_selector = base_selector.substr(0, pos);
                }
                
                if (matcher_.matches(base_selector, node)) {
                    has_before = true;
                    // Apply rule styles to before_style
                    const auto& rs = rule.style;
                    if (!rs.content.empty() || rs.content == "") {
                        before_style.content = rs.content;
                    }
                    // Copy other properties
                    if (!rs.display.empty()) {
                        before_style.display = rs.display;
                        before_style.layout_mode = deriveLayoutModeFromDisplay(rs.display);
                    }
                    if (!rs.color.empty()) before_style.color = rs.color;
                    if (!rs.background_color.empty()) before_style.background_color = rs.background_color;
                    if (rs.width.value != 0.0f || !rs.width.isAuto()) before_style.width = rs.width;
                    if (rs.height.value != 0.0f || !rs.height.isAuto()) before_style.height = rs.height;
                    before_style.margin_top = rs.margin_top;
                    before_style.margin_right = rs.margin_right;
                    before_style.margin_bottom = rs.margin_bottom;
                    before_style.margin_left = rs.margin_left;
                    before_style.padding_top = rs.padding_top;
                    before_style.padding_right = rs.padding_right;
                    before_style.padding_bottom = rs.padding_bottom;
                    before_style.padding_left = rs.padding_left;
                    if (rs.font_size != 16.0f) before_style.font_size = rs.font_size;
                    if (!rs.font_family.empty()) before_style.font_family = rs.font_family;
                    if (rs.border_radius > 0.0f) before_style.border_radius = rs.border_radius;
                    if (rs.border_width > 0.0f) before_style.border_width = rs.border_width;
                    if (!rs.border_color.empty()) before_style.border_color = rs.border_color;
                    if (!rs.border_style.empty()) before_style.border_style = rs.border_style;
                }
            }
            
            // Check for ::after
            if (rule.selector.find("::after") != std::string::npos ||
                rule.selector.find(":after") != std::string::npos) {
                std::string base_selector = rule.selector;
                size_t pos = base_selector.find("::after");
                if (pos == std::string::npos) pos = base_selector.find(":after");
                if (pos != std::string::npos) {
                    base_selector = base_selector.substr(0, pos);
                }
                
                if (matcher_.matches(base_selector, node)) {
                    has_after = true;
                    const auto& rs = rule.style;
                    if (!rs.content.empty() || rs.content == "") {
                        after_style.content = rs.content;
                    }
                    if (!rs.display.empty()) {
                        after_style.display = rs.display;
                        after_style.layout_mode = deriveLayoutModeFromDisplay(rs.display);
                    }
                    if (!rs.color.empty()) after_style.color = rs.color;
                    if (!rs.background_color.empty()) after_style.background_color = rs.background_color;
                    if (rs.width.value != 0.0f || !rs.width.isAuto()) after_style.width = rs.width;
                    if (rs.height.value != 0.0f || !rs.height.isAuto()) after_style.height = rs.height;
                    after_style.margin_top = rs.margin_top;
                    after_style.margin_right = rs.margin_right;
                    after_style.margin_bottom = rs.margin_bottom;
                    after_style.margin_left = rs.margin_left;
                    after_style.padding_top = rs.padding_top;
                    after_style.padding_right = rs.padding_right;
                    after_style.padding_bottom = rs.padding_bottom;
                    after_style.padding_left = rs.padding_left;
                    if (rs.font_size != 16.0f) after_style.font_size = rs.font_size;
                    if (!rs.font_family.empty()) after_style.font_family = rs.font_family;
                    if (rs.border_radius > 0.0f) after_style.border_radius = rs.border_radius;
                    if (rs.border_width > 0.0f) after_style.border_width = rs.border_width;
                    if (!rs.border_color.empty()) after_style.border_color = rs.border_color;
                    if (!rs.border_style.empty()) after_style.border_style = rs.border_style;
                }
            }
        }
    }
    
    // Create ::before pseudo-element if needed
    if (has_before && !before_style.content.empty()) {
        auto pseudo = createPseudoElement(node, "before");
        if (pseudo) {
            pseudo->getComputedStyle() = before_style;
            pseudo->getComputedStyle().is_pseudo_element = true;
            pseudo->getComputedStyle().pseudo_type = "before";
            pseudo->setTextContent(before_style.content);
            node->setPseudoBefore(pseudo);
        }
    } else {
        node->setPseudoBefore(nullptr);
    }
    
    // Create ::after pseudo-element if needed
    if (has_after && !after_style.content.empty()) {
        auto pseudo = createPseudoElement(node, "after");
        if (pseudo) {
            pseudo->getComputedStyle() = after_style;
            pseudo->getComputedStyle().is_pseudo_element = true;
            pseudo->getComputedStyle().pseudo_type = "after";
            pseudo->setTextContent(after_style.content);
            node->setPseudoAfter(pseudo);
        }
    } else {
        node->setPseudoAfter(nullptr);
    }
}

DOMNodePtr StyleEngine::createPseudoElement(DOMNodePtr parent, const std::string& pseudo_type) {
    if (!parent) return nullptr;
    
    auto pseudo = std::make_shared<DOMNode>(DOMNode::NodeType::ELEMENT, "::"+pseudo_type);
    // Inherit from parent
    inheritFromParent(pseudo);
    
    return pseudo;
}

} // namespace dong::dom
