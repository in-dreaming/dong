#pragma once

#include "css_value.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace dong::dom {

// Layout modes used by the layout engine
enum class LayoutMode {
    Block,
    Inline,
    Flex,
    None
};

// Helper to derive layout_mode from display value
inline LayoutMode deriveLayoutModeFromDisplay(const std::string& display) {
    if (display == "none") {
        return LayoutMode::None;
    }
    if (display == "flex" || display == "inline-flex") {
        return LayoutMode::Flex;
    }
    if (display == "inline" || display == "inline-block") {
        return LayoutMode::Inline;
    }
    return LayoutMode::Block;
}

// Computed style properties
struct ComputedStyle {
    // Box Model
    CSSValue width = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue height = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue min_width = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue max_width = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue min_height = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue max_height = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue margin_top;
    CSSValue margin_right;
    CSSValue margin_bottom;
    CSSValue margin_left;
    CSSValue padding_top;
    CSSValue padding_right;
    CSSValue padding_bottom;
    CSSValue padding_left;
    std::string box_sizing = "content-box";

    // Layout
    std::string display = "block";
    LayoutMode layout_mode = LayoutMode::Block;
    std::string position = "static";
    CSSValue top = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue right = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue bottom = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue left = CSSValue(0.0f, CSSValue::Unit::AUTO);
    int z_index = 0;

    // Visual
    std::string background_color = "transparent";
    std::string background_image;
    std::string background_size = "auto";
    std::string background_repeat = "repeat";
    std::string background_position = "0% 0%";
    std::string object_fit = "fill";

    std::vector<CSSGradient> background_gradients;
    std::string color = "#000000";
    float border_radius = 0.0f;
    float border_top_left_radius = 0.0f;
    float border_top_right_radius = 0.0f;
    float border_bottom_left_radius = 0.0f;
    float border_bottom_right_radius = 0.0f;
    std::string border_color = "#000000";
    float border_width = 0.0f;
    std::string border_style = "none";
    std::string overflow = "visible";
    std::string overflow_x = "visible";
    std::string overflow_y = "visible";
    std::string visibility = "visible";
    std::string cursor = "auto";
    
    // Outline
    float outline_width = 0.0f;
    std::string outline_color = "#000000";
    std::string outline_style = "none";
    float outline_offset = 0.0f;
    
    float opacity = 1.0f;
    bool isolation_isolate = false;
    std::vector<BoxShadow> box_shadows;
    
    // Filters
    std::vector<CSSFilter> filters;
    std::vector<CSSFilter> backdrop_filters;
    std::string mix_blend_mode = "normal";
    std::string background_blend_mode = "normal";

    // Text
    std::string font_family = "Arial";
    float font_size = 16.0f;
    std::string font_weight = "normal";
    std::string font_style = "normal";
    std::string font_variant = "normal";
    std::string text_align = "left";
    std::string text_align_last = "auto";
    std::string text_decoration = "none";
    std::string text_decoration_color;
    std::string text_decoration_style = "solid";
    float text_decoration_thickness = 1.0f;
    float letter_spacing_em = 0.0f;
    float word_spacing_px = 0.0f;

    // `line-height` is special: default/inherit behavior makes it hard to distinguish
    // "unspecified" from an explicit `normal` value. Track whether it was explicitly set.
    bool has_line_height = false;
    float line_height = -1.0f;
    bool line_height_is_unitless = true;

    std::string text_transform = "none";

    std::string text_overflow = "clip";
    std::string white_space = "normal";
    std::string word_break = "normal";
    std::string overflow_wrap = "normal";
    std::string vertical_align = "baseline";
    std::string direction = "ltr";
    std::string unicode_bidi = "normal";
    float text_indent = 0.0f;
    int webkit_line_clamp = 0;
    
    // Text shadow
    float text_shadow_offset_x = 0.0f;
    float text_shadow_offset_y = 0.0f;
    float text_shadow_blur = 0.0f;
    std::string text_shadow_color;

    // Flexbox
    std::string flex_direction = "row";
    std::string flex_wrap = "nowrap";
    std::string justify_content = "flex-start";
    std::string align_items = "stretch";
    std::string align_content = "stretch";
    std::string align_self = "auto";
    float flex = 0.0f;
    float flex_grow = 0.0f;
    float flex_shrink = 1.0f;
    CSSValue flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);
    int order = 0;
    float gap = 0.0f;
    float row_gap = 0.0f;
    float column_gap = 0.0f;

    // Transform
    float transform_translate_x = 0.0f;
    float transform_translate_y = 0.0f;
    float transform_scale_x = 1.0f;
    float transform_scale_y = 1.0f;
    float transform_rotate = 0.0f;
    float transform_skew_x = 0.0f;
    float transform_skew_y = 0.0f;
    float transform_origin_x = 50.0f;
    float transform_origin_y = 50.0f;
    std::string transform_style = "flat";
    float perspective = 0.0f;
    float perspective_origin_x = 50.0f;
    float perspective_origin_y = 50.0f;
    std::string backface_visibility = "visible";
    
    // Transitions
    std::vector<CSSTransition> transitions;
    
    // Animations
    std::vector<CSSAnimation> animations;
    
    // Clip path (basic shapes)
    std::string clip_path;
    
    // Pointer events
    std::string pointer_events = "auto";
    std::string user_select = "auto";
    std::string touch_action = "auto";
    std::string caret_color = "auto";
    
    // Float/Clear (partial support)
    std::string float_value = "none";
    std::string clear = "none";
    
    // Pseudo-element content (for ::before/::after)
    std::string content;  // Empty means no content, "none" disables
    bool is_pseudo_element = false;
    std::string pseudo_type;  // "before" or "after"
    
    // CSS custom properties (variables)
    std::shared_ptr<CSSVariables> css_variables;
    
    // Get or create CSS variables storage
    CSSVariables& getVariables() {
        if (!css_variables) {
            css_variables = std::make_shared<CSSVariables>();
        }
        return *css_variables;
    }
    
    // Helper method to set display and automatically update layout_mode
    void setDisplay(const std::string& value) {
        display = value;
        layout_mode = deriveLayoutModeFromDisplay(value);
    }
};

} // namespace dong::dom
