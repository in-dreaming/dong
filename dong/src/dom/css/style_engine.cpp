#include "style_engine.hpp"
#include "style_engine_internal.hpp"
#include "../dom/dom_node.hpp"
#include "../dialog_element.hpp"
#include "../../render/list_marker.hpp"

#include "../../core/log.h"
#include "../../core/profiler.h"
#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <string_view>


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
    if (d == "inline" || d == "inline-block") {
        return LayoutMode::Inline;
    }
    return LayoutMode::Block;
}

// Logical properties (margin/padding/border-inline/block).
// Note: we currently only map for horizontal-tb writing mode.
// ── Sub-functions for applyRuleProperties (declared before the caller) ──



void applyInlineStyleAttributeIfAny(DOMNodePtr node) {
    if (!node) return;
    if (!node->hasAttribute("style")) return;
    const std::string style_str = node->getAttribute("style");
    if (style_str.empty()) return;

    // Inline style has the highest precedence in author styles.
    CSSParser::parseInlineStyle(style_str, node->getComputedStyle());
}

// Apply HTML presentational attributes that map to CSS properties.
// These have lower priority than author styles (inline or external),
// so they are applied BEFORE inline styles in the cascade.
void applyPresentationalAttributesIfAny(DOMNodePtr node) {
    if (!node) return;
    const std::string& tag = node->getTagName();
    auto& cs = node->getComputedStyle();

    // <img width="N" height="N"> �?CSS width/height (if no CSS width/height already set)
    if (tag == "img" || tag == "video" || tag == "canvas") {
        if (!cs.isExplicitlySet("width") && node->hasAttribute("width")) {
            const std::string& w = node->getAttribute("width");
            if (!w.empty()) {
                // Parse as integer pixels
                try {
                    float px = std::stof(w);
                    cs.width = CSSValue(px, CSSValue::Unit::PIXEL);
                    cs.markExplicitlySet("width");
                } catch (...) {}
            }
        }
        if (!cs.isExplicitlySet("height") && node->hasAttribute("height")) {
            const std::string& h = node->getAttribute("height");
            if (!h.empty()) {
                try {
                    float px = std::stof(h);
                    cs.height = CSSValue(px, CSSValue::Unit::PIXEL);
                    cs.markExplicitlySet("height");
                } catch (...) {}
            }
        }
    }
}

void applyDirAttributeIfAny(DOMNodePtr node) {
    // Apply dir attribute to CSS direction property
    // The dir attribute maps to the direction CSS property
    if (!node) return;

    auto& computed = node->getComputedStyle();
    computed.direction = node->getEffectiveDirection();
}

using TagStyleHandler = void(*)(ComputedStyle&);

const std::unordered_map<std::string_view, TagStyleHandler> kTagDefaultHandlers = {
    // Block-level elements
    {"div", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"p", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"body", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"html", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"main", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"section", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"article", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"nav", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"header", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"footer", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"aside", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"address", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"blockquote", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"figure", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"figcaption", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"dl", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"dt", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"dd", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"form", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"fieldset", [](ComputedStyle& s) { s.setDisplay("block"); }},

    // Inline elements
    {"span", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"a", [](ComputedStyle& s) { s.setDisplay("inline"); s.color = "#0000EE"; s.text_decoration = "underline"; s.cursor = "pointer"; s.markExplicitlySet("color"); }},
    {"b", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_weight = "bold"; s.markExplicitlySet("font-weight"); }},
    {"i", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; s.markExplicitlySet("font-style"); }},
    {"strong", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_weight = "bold"; s.markExplicitlySet("font-weight"); }},
    {"em", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; s.markExplicitlySet("font-style"); }},
    {"code", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; s.markExplicitlySet("font-family"); }},
    {"kbd", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; s.markExplicitlySet("font-family"); }},
    {"samp", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; s.markExplicitlySet("font-family"); }},
    {"var", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; s.markExplicitlySet("font-style"); }},
    {"small", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_size = 12.0f; s.markExplicitlySet("font-size"); }},
    {"s", [](ComputedStyle& s) { s.setDisplay("inline"); s.text_decoration = "line-through"; }},
    {"cite", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; s.markExplicitlySet("font-style"); }},
    {"q", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"mark", [](ComputedStyle& s) { s.setDisplay("inline"); s.background_color = "#ffff00"; }},
    {"sub", [](ComputedStyle& s) { s.setDisplay("inline"); s.vertical_align = "sub"; s.font_size = 12.0f; s.markExplicitlySet("font-size"); }},
    {"sup", [](ComputedStyle& s) { s.setDisplay("inline"); s.vertical_align = "super"; s.font_size = 12.0f; s.markExplicitlySet("font-size"); }},
    {"u", [](ComputedStyle& s) { s.setDisplay("inline"); s.text_decoration = "underline"; }},
    {"abbr", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"time", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"data", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"wbr", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"label", [](ComputedStyle& s) { s.setDisplay("inline"); }},

    // Headings - sizes match browser defaults (based on 16px base)
    {"h1", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 32.0f; s.markExplicitlySet("font-weight"); s.markExplicitlySet("font-size"); }},
    {"h2", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 24.0f; s.markExplicitlySet("font-weight"); s.markExplicitlySet("font-size"); }},
    {"h3", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 18.72f; s.markExplicitlySet("font-weight"); s.markExplicitlySet("font-size"); }},
    {"h4", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 16.0f; s.markExplicitlySet("font-weight"); s.markExplicitlySet("font-size"); }},
    {"h5", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 13.28f; s.markExplicitlySet("font-weight"); s.markExplicitlySet("font-size"); }},
    {"h6", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 10.72f; s.markExplicitlySet("font-weight"); s.markExplicitlySet("font-size"); }},

    // List elements
    {"ul", [](ComputedStyle& s) { s.setDisplay("block"); s.list_style_type = "disc"; }},
    {"ol", [](ComputedStyle& s) { s.setDisplay("block"); s.list_style_type = "decimal"; }},
    {"li", [](ComputedStyle& s) { s.setDisplay("list-item"); }},

    // Form elements (inline-block)
    {"button", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"input", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"select", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"textarea", [](ComputedStyle& s) { s.setDisplay("inline-block"); s.white_space = "pre-wrap"; s.markExplicitlySet("white-space"); }},

    // Media elements (inline-block)
    {"img", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"video", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"canvas", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"svg", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"picture", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},

    // Special elements
    {"br", [](ComputedStyle& s) {
        s.setDisplay("inline");
        s.height = CSSValue(0.0f, CSSValue::Unit::PIXEL);
    }},
    {"hr", [](ComputedStyle& s) {
        s.setDisplay("block");
        s.height = CSSValue(1.0f, CSSValue::Unit::PIXEL);
        s.margin_top = CSSValue(8.0f, CSSValue::Unit::PIXEL);
        s.margin_bottom = CSSValue(8.0f, CSSValue::Unit::PIXEL);
        s.border_width = 0.0f;
        s.background_color = "#cccccc";
    }},
    {"pre", [](ComputedStyle& s) {
        s.setDisplay("block");
        s.font_family = "Menlo, Consolas, monospace";
        s.markExplicitlySet("font-family");
    }},

    // Table elements
    {"table", [](ComputedStyle& s) { s.setDisplay("table"); }},
    {"caption", [](ComputedStyle& s) { s.setDisplay("table-caption"); }},
    {"tr", [](ComputedStyle& s) { s.setDisplay("table-row"); }},
    {"td", [](ComputedStyle& s) { s.setDisplay("table-cell"); }},
    {"th", [](ComputedStyle& s) { s.setDisplay("table-cell"); }},
    {"thead", [](ComputedStyle& s) { s.setDisplay("table-row-group"); }},
    {"tbody", [](ComputedStyle& s) { s.setDisplay("table-row-group"); }},
    {"tfoot", [](ComputedStyle& s) { s.setDisplay("table-row-group"); }},
};

void applyDefaultStyleForElement(const DOMNodePtr& node) {
    auto& style = node->getComputedStyle();
    const std::string& tag = node->getTagName();
    auto it = kTagDefaultHandlers.find(tag);
    if (it != kTagDefaultHandlers.end()) {
        it->second(style);
        return;
    }
    style.setDisplay("block");
}

} // anonymous namespace



void Stylesheet::addRule(const std::string& selector, const ComputedStyle& style, 
                         int specificity, int order) {
    rules_.push_back({selector, style, specificity, order});
}

bool Stylesheet::insertRuleAt(size_t index, const CSSRule& rule) {
    if (index > rules_.size()) return false;
    rules_.insert(rules_.begin() + static_cast<std::ptrdiff_t>(index), rule);
    return true;
}

bool Stylesheet::deleteRuleAt(size_t index) {
    if (index >= rules_.size()) return false;
    rules_.erase(rules_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
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

StyleEngine::StyleEngine() {
    // Minimal UA stylesheet so that common defaults match browsers closer.
    // NOTE: This is not full UA behavior; it's a pragmatic baseline for common controls.
    // Author CSS will override these due to later source order.
    static const char* kUserAgentCSS = R"CSS(
html, body {
  background-color: #ffffff;
}

body {
  margin: 8px;
}

/* Basic block spacing to better match browser defaults. */
h1 {
  margin: 0.67em 0;
}

h2 {
  margin: 0.83em 0;
}

h3 {
  margin: 1em 0;
}

h4 {
  margin: 1.33em 0;
}

h5 {
  margin: 1.67em 0;
}

h6 {
  margin: 2.33em 0;
}

p {
  margin: 1em 0;
}

ol, ul {
  margin: 1em 0;
  padding-left: 40px;
}

li {
  display: list-item;
}

/* Controls default to inline-block in browsers; our ComputedStyle defaults to block. */
button, input, select, textarea {
  display: inline-block;
  box-sizing: border-box;
  color: #202124;
}

/* Approximate Chromium default form control styling (headless baseline). */
button {
  padding: 1px 6px;
  background-color: #f1f3f4;
  white-space: nowrap;

  border-width: 1px;
  border-style: solid;
  border-color: #dadce0;
  border-radius: 2px;
  cursor: pointer;
}

input, select, textarea {
  padding: 1px 4px;
  background-color: #ffffff;
  border-width: 1px;
  border-style: solid;
  border-color: #dadce0;
  border-radius: 2px;
  min-height: 24px;
}

/* Default focus style for inputs (matches Chromium default outline) */
input:focus, select:focus, textarea:focus {
  outline: 2px solid #1a73e8;
  outline-offset: 1px;
}

/* Default width for text inputs (matches browser defaults) */
input[type="text"], input[type="password"], input[type="email"], input[type="search"], input[type="url"] {
  width: 150px;
}

/* Override author "input { width:100% }" for checkbox/radio-like controls. */
input[type="checkbox"], input[type="radio"] {
  display: inline-block;
  width: 13px;
  height: 13px;
  padding: 0;
  margin-top: 0;
  margin-right: 4px;
  margin-bottom: 0;
  margin-left: 0;
  background-color: #ffffff;
  border-width: 1px;
  border-style: solid;
  border-color: #5f6368;
  box-sizing: border-box;
}

input[type="checkbox"] {
  border-radius: 2px;
}

input[type="radio"] {
  border-radius: 50%;
}



/* Table defaults (match common browser UA behavior). */
caption {
  display: table-caption;
  text-align: center;
}

/* [hidden] attribute support - must use !important to override author styles */
[hidden] {
  display: none !important;
}


/* Disabled elements - visual feedback and pointer-events */
button[disabled], input[disabled], select[disabled], textarea[disabled] {
  opacity: 0.5;
  cursor: not-allowed;
  pointer-events: none;
}
)CSS";


    addStylesheet(std::string(kUserAgentCSS));
}

void StyleEngine::applyDefaultStyleForNode(DOMNodePtr node) {
    if (!node) return;

    node->getComputedStyle() = ComputedStyle{};

    if (node->getType() != DOMNode::NodeType::ELEMENT) {
        return;
    }

    applyDefaultStyleForElement(node);
}

void StyleEngine::applyDefaultStylesRecursive(DOMNodePtr node) {
    if (!node) return;

    applyDefaultStyleForNode(node);

    for (const auto& child : node->getChildren()) {
        applyDefaultStylesRecursive(child);
    }
}

void StyleEngine::addStylesheet(const std::string& css) {

    Stylesheet sheet;
    
    // Parse unlayered rules (parse() skips @layer blocks)
    auto rules = parseCSS(css);
    for (const auto& rule : rules) {
        // Rules from parse() have empty layer_name = unlayered (highest priority)
        sheet.getRulesMutable().push_back(rule);
    }
    
    // Parse @layer rules and integrate them with proper layer names
    auto layer_rules = parser_.parseLayerRules(css);
    if (!layer_rules.empty()) {
        // Assign unique names to anonymous layers
        for (auto& layer : layer_rules) {
            if (layer.name.empty() && !layer.is_predeclared) {
                layer.name = "__anon_" + std::to_string(anon_layer_counter_++);
            }
        }
        
        // Build predeclared order map from this CSS's predeclarations.
        // @layer base, theme; means base=0 (lower priority), theme=1 (higher priority).
        std::unordered_map<std::string, int> predeclared_order;
        for (const auto& layer : layer_rules) {
            if (layer.is_predeclared && !layer.name.empty()) {
                predeclared_order[layer.name] = layer.declaration_order;
            }
        }

        // Merge layer ordering into layer_context_ (append, not replace).
        // If a layer was predeclared, use its predeclared order (not the definition counter order),
        // so @layer base, theme; followed by @layer theme {...} @layer base {...} keeps base < theme.
        for (const auto& layer : layer_rules) {
            if (!layer.name.empty() && !layer.is_predeclared) {
                int order = layer.declaration_order;
                auto pred_it = predeclared_order.find(layer.name);
                if (pred_it != predeclared_order.end()) {
                    order = pred_it->second;
                }
                // Only insert if not already in map (first declaration wins per CSS spec)
                if (layer_context_.layer_order_map.find(layer.name) == layer_context_.layer_order_map.end()) {
                    layer_context_.layer_order_map[layer.name] = order;
                    layer_context_.layers.push_back(layer);
                }
            }
        }
        
        // Add rules from each layer with layer_name set
        for (const auto& layer : layer_rules) {
            if (!layer.is_predeclared) {
                for (auto rule : layer.rules) {
                    rule.layer_name = layer.name;
                    sheet.getRulesMutable().push_back(rule);
                }
            }
        }
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
    index_dirty_ = true;  // 标记索引需要重�?
}

void StyleEngine::addStylesheet(const Stylesheet& sheet) {
    stylesheets_.push_back(sheet);
    index_dirty_ = true;  // 标记索引需要重�?
}

Stylesheet* StyleEngine::stylesheetAt(size_t index) {
    if (index >= stylesheets_.size()) return nullptr;
    return &stylesheets_[index];
}

const Stylesheet* StyleEngine::stylesheetAt(size_t index) const {
    if (index >= stylesheets_.size()) return nullptr;
    return &stylesheets_[index];
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
    DONG_PROFILE_FUNCTION();

    if (!node) return;

    applyDefaultStyleForNode(node);

    // Collect matching rules once
    std::vector<CSSRule> matching_rules;
    for (const auto& sheet : stylesheets_) {
        for (const auto& rule : sheet.getRules()) {
            // Skip pseudo-element rules (::before, ::after, etc.) �?they are applied
            // exclusively via processPseudoElements(), not to the originating element.
            if (rule.selector.find("::before") != std::string::npos ||
                rule.selector.find(":before") != std::string::npos ||
                rule.selector.find("::after") != std::string::npos ||
                rule.selector.find(":after") != std::string::npos ||
                rule.selector.find("::marker") != std::string::npos ||
                rule.selector.find(":marker") != std::string::npos ||
                rule.selector.find("::placeholder") != std::string::npos ||
                rule.selector.find(":placeholder") != std::string::npos ||
                rule.selector.find("::selection") != std::string::npos ||
                rule.selector.find("::backdrop") != std::string::npos) {
                continue;
            }
            if (matcher_.matches(rule.selector, node)) {
                matching_rules.push_back(rule);
            }
        }
    }

    // Sort by layer priority, then specificity, then source order
    sortRulesWithLayerPriority(matching_rules);



    // Apply all properties from matching rules
    auto& computed = node->getComputedStyle();
    for (const auto& rule : matching_rules) {
        style_engine_internal::applyRuleProperties(rule.style, computed);
    }

    // Inherit from parent
    inheritFromParent(node);

    // Presentational HTML attributes (e.g. img width/height) �?lower priority than inline styles.
    applyPresentationalAttributesIfAny(node);

    // Inline style overrides author rules
    applyInlineStyleAttributeIfAny(node);

    // Re-apply !important properties from rules - these override inline styles
    for (const auto& rule : matching_rules) {
        style_engine_internal::applyImportantPropertiesOnly(rule.style, computed);
    }

    // Resolve logical properties (margin/padding/border-inline/block) after cascade.
    style_engine_internal::applyLogicalProperties(computed);

    // Hide certain elements


    static const std::unordered_set<std::string> kAlwaysHiddenTags = {
        "head", "style", "script", "meta", "title", "link"
    };
    if (kAlwaysHiddenTags.count(node->getTagName()) > 0) {
        node->getComputedStyle().display = "none";
    }

    // [hidden] attribute support
    if (node->hasAttribute("hidden")) {
        node->getComputedStyle().display = "none";
    }

    // <details> hiding: non-<summary> children of a closed <details> are hidden.
    if (auto parent = node->getParent()) {
        if (parent->getTagName() == "details" &&
            !parent->hasAttribute("open") &&
            node->getTagName() != "summary") {
            node->getComputedStyle().display = "none";
        }
    }

    // <dialog> element: hidden unless open attribute is present
    if (node->getTagName() == "dialog") {
        if (!node->hasAttribute("open")) {
            node->getComputedStyle().display = "none";
        } else {
            // Default dialog styles (only when not explicitly styled)
            auto& cs = node->getComputedStyle();

            // Check if this is a modal dialog
            auto* state = dong::dom::getDialogState(node);
            bool is_modal = state && state->isModal();

            if (is_modal) {
                // Modal dialogs are positioned fixed, centered
                if (!cs.isExplicitlySet("position")) {
                    cs.position = "fixed";
                }
                if (!cs.isExplicitlySet("top")) {
                    cs.top = CSSValue(0.0f, CSSValue::Unit::PIXEL);
                }
                // Note: do NOT set left/right to 0 — Yoga doesn't support width:fit-content,
                // so setting all four edges causes the dialog to stretch to full viewport.
                // Instead, rely on margin:auto for horizontal centering.
                if (!cs.isExplicitlySet("bottom")) {
                    cs.bottom = CSSValue(0.0f, CSSValue::Unit::PIXEL);
                }
                if (!cs.isExplicitlySet("margin")) {
                    cs.margin_top = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_right = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_bottom = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_left = CSSValue(0.0f, CSSValue::Unit::AUTO);
                }
                if (!cs.isExplicitlySet("max-width")) {
                    // calc(100% - 2em) equivalent: leave some margin
                    cs.max_width = CSSValue(90.0f, CSSValue::Unit::PERCENT);
                }
                if (!cs.isExplicitlySet("max-height")) {
                    cs.max_height = CSSValue(90.0f, CSSValue::Unit::PERCENT);
                }
            } else {
                // Non-modal: block display, centered horizontally
                if (!cs.isExplicitlySet("display")) {
                    cs.display = "block";
                }
                if (!cs.isExplicitlySet("margin")) {
                    cs.margin_top = CSSValue(1.0f, CSSValue::Unit::EM);
                    cs.margin_bottom = CSSValue(1.0f, CSSValue::Unit::EM);
                    cs.margin_left = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_right = CSSValue(0.0f, CSSValue::Unit::AUTO);
                }
            }

            if (!cs.isExplicitlySet("border-style")) {
                cs.border_style = "solid";
                cs.border_top_style = "solid";
                cs.border_right_style = "solid";
                cs.border_bottom_style = "solid";
                cs.border_left_style = "solid";
            }
            if (!cs.isExplicitlySet("border-width")) {
                cs.border_width = 1.0f;
                cs.border_top_width = 1.0f;
                cs.border_right_width = 1.0f;
                cs.border_bottom_width = 1.0f;
                cs.border_left_width = 1.0f;
            }
            if (!cs.isExplicitlySet("border-color")) {
                cs.border_top_color = "#000000"; cs.border_right_color = "#000000";
                cs.border_bottom_color = "#000000"; cs.border_left_color = "#000000";
            }
            if (!cs.isExplicitlySet("padding")) {
                cs.padding_top = CSSValue(16.0f, CSSValue::Unit::PIXEL); cs.padding_right = CSSValue(16.0f, CSSValue::Unit::PIXEL);
                cs.padding_bottom = CSSValue(16.0f, CSSValue::Unit::PIXEL); cs.padding_left = CSSValue(16.0f, CSSValue::Unit::PIXEL);
            }
            if (!cs.isExplicitlySet("background-color") && cs.background_color == "transparent") {
                cs.background_color = "#FFFFFF";
            }
        }
    }

    // [dir] attribute mapping to CSS direction property
    // Only apply if direction is not explicitly set by CSS
    if (node->hasAttribute("dir") && !computed.isExplicitlySet("direction")) {
        auto& comp_style = node->getComputedStyle();
        comp_style.direction = node->getEffectiveDirection();
        comp_style.markExplicitlySet("direction");
    }

    // Resolve logical text-align (start/end) based on final direction.
    style_engine_internal::resolveTextAlignForDirection(node->getComputedStyle());

    node->getComputedStyle().layout_mode = deriveLayoutModeFromDisplay(node->getComputedStyle());

    // Update BFC flag based on computed style properties
    node->getComputedStyle().updateBFCFlag();

    // Resolve light-dark() functions based on color-scheme
    resolveLightDarkFunctions(node);

    // Process pseudo-elements (::before/::after)
    processPseudoElements(node);

    // Recursively compute styles for children
    for (const auto& child : node->getChildren()) {
        computeStyles(child);
    }
}

void StyleEngine::recomputeNodeStyle(DOMNodePtr node) {
    if (!node) return;
    applyDefaultStyleForNode(node);
    applyMatchingRules(node);
    inheritFromParent(node);
    applyInlineStyleAttributeIfAny(node);

    style_engine_internal::applyLogicalProperties(node->getComputedStyle());

    // Hidden elements
    static const std::unordered_set<std::string> kAlwaysHiddenTags = {
        "head", "style", "script", "meta", "title", "link"
    };
    if (kAlwaysHiddenTags.count(node->getTagName()) > 0) {
        node->getComputedStyle().display = "none";
    }
    if (node->hasAttribute("hidden")) {
        node->getComputedStyle().display = "none";
    }

    // <details> hiding
    if (auto parent = node->getParent()) {
        if (parent->getTagName() == "details" &&
            !parent->hasAttribute("open") &&
            node->getTagName() != "summary") {
            node->getComputedStyle().display = "none";
        }
    }

    // <dialog> element: hidden unless open attribute is present
    if (node->getTagName() == "dialog") {
        if (!node->hasAttribute("open")) {
            node->getComputedStyle().display = "none";
        } else {
            auto& cs = node->getComputedStyle();
            auto* state = dong::dom::getDialogState(node);
            bool is_modal = state && state->isModal();

            if (is_modal) {
                if (!cs.isExplicitlySet("position")) cs.position = "fixed";
                if (!cs.isExplicitlySet("top")) cs.top = CSSValue(0.0f, CSSValue::Unit::PIXEL);
                if (!cs.isExplicitlySet("left")) cs.left = CSSValue(0.0f, CSSValue::Unit::PIXEL);
                if (!cs.isExplicitlySet("right")) cs.right = CSSValue(0.0f, CSSValue::Unit::PIXEL);
                if (!cs.isExplicitlySet("bottom")) cs.bottom = CSSValue(0.0f, CSSValue::Unit::PIXEL);
                if (!cs.isExplicitlySet("margin")) {
                    cs.margin_top = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_right = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_bottom = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_left = CSSValue(0.0f, CSSValue::Unit::AUTO);
                }
                if (!cs.isExplicitlySet("max-width")) cs.max_width = CSSValue(90.0f, CSSValue::Unit::PERCENT);
                if (!cs.isExplicitlySet("max-height")) cs.max_height = CSSValue(90.0f, CSSValue::Unit::PERCENT);
            } else {
                if (!cs.isExplicitlySet("display")) cs.display = "block";
                if (!cs.isExplicitlySet("margin")) {
                    cs.margin_top = CSSValue(1.0f, CSSValue::Unit::EM);
                    cs.margin_bottom = CSSValue(1.0f, CSSValue::Unit::EM);
                    cs.margin_left = CSSValue(0.0f, CSSValue::Unit::AUTO);
                    cs.margin_right = CSSValue(0.0f, CSSValue::Unit::AUTO);
                }
            }
            if (!cs.isExplicitlySet("border-style")) {
                cs.border_style = "solid";
                cs.border_top_style = "solid"; cs.border_right_style = "solid";
                cs.border_bottom_style = "solid"; cs.border_left_style = "solid";
            }
            if (!cs.isExplicitlySet("border-width")) {
                cs.border_width = 1.0f;
                cs.border_top_width = 1.0f; cs.border_right_width = 1.0f;
                cs.border_bottom_width = 1.0f; cs.border_left_width = 1.0f;
            }
            if (!cs.isExplicitlySet("border-color")) {
                cs.border_top_color = "#000000"; cs.border_right_color = "#000000";
                cs.border_bottom_color = "#000000"; cs.border_left_color = "#000000";
            }
            if (!cs.isExplicitlySet("padding")) {
                cs.padding_top = CSSValue(16.0f, CSSValue::Unit::PIXEL); cs.padding_right = CSSValue(16.0f, CSSValue::Unit::PIXEL);
                cs.padding_bottom = CSSValue(16.0f, CSSValue::Unit::PIXEL); cs.padding_left = CSSValue(16.0f, CSSValue::Unit::PIXEL);
            }
            if (!cs.isExplicitlySet("background-color") && cs.background_color == "transparent") {
                cs.background_color = "#FFFFFF";
            }
        }
    }

    // [dir] attribute mapping to CSS direction property
    // Only apply if direction is not explicitly set by CSS
    auto& computed = node->getComputedStyle();
    if (node->hasAttribute("dir") && !computed.isExplicitlySet("direction")) {
        computed.direction = node->getEffectiveDirection();
        computed.markExplicitlySet("direction");
    }

    node->getComputedStyle().layout_mode = deriveLayoutModeFromDisplay(node->getComputedStyle());

    node->getComputedStyle().updateBFCFlag();

    // Resolve light-dark() functions based on color-scheme
    resolveLightDarkFunctions(node);
}



void StyleEngine::applyMatchingRules(DOMNodePtr node) {
    std::vector<CSSRule> matching_rules;

    for (const auto& sheet : stylesheets_) {
        for (const auto& rule : sheet.getRules()) {
            // Skip pseudo-element rules �?applied via processPseudoElements().
            if (rule.selector.find("::before") != std::string::npos ||
                rule.selector.find(":before") != std::string::npos ||
                rule.selector.find("::after") != std::string::npos ||
                rule.selector.find(":after") != std::string::npos ||
                rule.selector.find("::marker") != std::string::npos ||
                rule.selector.find(":marker") != std::string::npos ||
                rule.selector.find("::placeholder") != std::string::npos ||
                rule.selector.find(":placeholder") != std::string::npos ||
                rule.selector.find("::selection") != std::string::npos) {
                continue;
            }
            if (matcher_.matches(rule.selector, node)) {
                matching_rules.push_back(rule);
            }
        }
    }

    // Sort with layer priority, then specificity, then source order
    sortRulesWithLayerPriority(matching_rules);

    auto& computed = node->getComputedStyle();
    for (const auto& rule : matching_rules) {
        style_engine_internal::applyRuleProperties(rule.style, computed);
    }
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

} // namespace dong::dom