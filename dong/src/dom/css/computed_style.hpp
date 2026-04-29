#pragma once

#include "css_value.hpp"
#include "css_enums.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace dong::dom {

// Layout modes used by the layout engine
enum class LayoutMode {
    Block,
    Inline,
    Flex,
    None
};

inline LayoutMode deriveLayoutModeFromDisplay(CSSDisplay display) {
    switch (display) {
    case CSSDisplay::None:       return LayoutMode::None;
    case CSSDisplay::Flex:
    case CSSDisplay::InlineFlex: return LayoutMode::Flex;
    case CSSDisplay::Inline:
    case CSSDisplay::InlineBlock: return LayoutMode::Inline;
    default:                      return LayoutMode::Block;
    }
}

// Overload accepting string for backward compatibility at parse boundaries
inline LayoutMode deriveLayoutModeFromDisplay(const std::string& display) {
    return deriveLayoutModeFromDisplay(displayFromString(display));
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

    // Logical margins (writing-mode: horizontal-tb only for now)
    CSSValue margin_inline_start;
    CSSValue margin_inline_end;
    CSSValue margin_block_start;
    CSSValue margin_block_end;

    CSSValue padding_top;
    CSSValue padding_right;
    CSSValue padding_bottom;
    CSSValue padding_left;

    // Logical paddings (writing-mode: horizontal-tb only for now)
    CSSValue padding_inline_start;
    CSSValue padding_inline_end;
    CSSValue padding_block_start;
    CSSValue padding_block_end;

    CSSBoxSizing box_sizing = CSSBoxSizing::ContentBox;
    float aspect_ratio = 0.0f;  // 0 = auto, >0 = width/height ratio


    // Layout
    CSSDisplay display = CSSDisplay::Block;
    LayoutMode layout_mode = LayoutMode::Block;
    bool creates_block_formatting_context = false;
    CSSPosition position = CSSPosition::Static;
    CSSValue top = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue right = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue bottom = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue left = CSSValue(0.0f, CSSValue::Unit::AUTO);
    std::optional<int> z_index;  // nullopt = auto (no stacking context), value = create stacking context

    // Visual
    std::string background_color = "transparent";
    std::string background_image;
    std::string background_size = "auto";
    CSSBackgroundRepeat background_repeat = CSSBackgroundRepeat::Repeat;
    std::string background_position = "0% 0%";
    CSSBackgroundAttachment background_attachment = CSSBackgroundAttachment::Scroll;
    CSSBackgroundBox background_clip = CSSBackgroundBox::BorderBox;
    CSSBackgroundBox background_origin = CSSBackgroundBox::PaddingBox;
    CSSObjectFit object_fit = CSSObjectFit::Fill;
    std::string object_position = "50% 50%";
    CSSImageRendering image_rendering = CSSImageRendering::Auto;

    CSSColorScheme color_scheme = CSSColorScheme::Normal;

    std::vector<CSSGradient> background_gradients;

    std::string color = "#000000";
    float border_radius = 0.0f;
    CSSValue border_top_left_radius;
    CSSValue border_top_right_radius;
    CSSValue border_bottom_left_radius;
    CSSValue border_bottom_right_radius;

    // Border (shorthand / all-sides)
    std::string border_color = "#000000";
    float border_width = 0.0f;
    CSSBorderStyle border_style = CSSBorderStyle::None;

    // Border per-side overrides (unset = fallback to shorthand)
    float border_top_width = -1.0f;
    float border_right_width = -1.0f;
    float border_bottom_width = -1.0f;
    float border_left_width = -1.0f;
    std::string border_top_color;
    std::string border_right_color;
    std::string border_bottom_color;
    std::string border_left_color;
    CSSBorderStyle border_top_style = CSSBorderStyleUnset;
    CSSBorderStyle border_right_style = CSSBorderStyleUnset;
    CSSBorderStyle border_bottom_style = CSSBorderStyleUnset;
    CSSBorderStyle border_left_style = CSSBorderStyleUnset;

    // Logical borders (writing-mode: horizontal-tb only for now)
    float border_inline_start_width = -1.0f;
    float border_inline_end_width = -1.0f;
    float border_block_start_width = -1.0f;
    float border_block_end_width = -1.0f;
    std::string border_inline_start_color;
    std::string border_inline_end_color;
    std::string border_block_start_color;
    std::string border_block_end_color;
    CSSBorderStyle border_inline_start_style = CSSBorderStyleUnset;
    CSSBorderStyle border_inline_end_style = CSSBorderStyleUnset;
    CSSBorderStyle border_block_start_style = CSSBorderStyleUnset;
    CSSBorderStyle border_block_end_style = CSSBorderStyleUnset;

    // border-image (nine-slice panel)
    std::string border_image_source;  // url("panel.png") or empty
    float border_image_slice_top = 0;
    float border_image_slice_right = 0;
    float border_image_slice_bottom = 0;
    float border_image_slice_left = 0;
    float border_image_width_top = 0;
    float border_image_width_right = 0;
    float border_image_width_bottom = 0;
    float border_image_width_left = 0;
    enum class BorderImageRepeat : uint8_t { Stretch = 0, Repeat = 1, Round = 2 };
    BorderImageRepeat border_image_repeat_h = BorderImageRepeat::Stretch;
    BorderImageRepeat border_image_repeat_v = BorderImageRepeat::Stretch;
    bool border_image_fill = true;  // dong default: true (games need center fill)

    CSSOverflow overflow = CSSOverflow::Visible;

    CSSOverflow overflow_x = CSSOverflow::Visible;
    CSSOverflow overflow_y = CSSOverflow::Visible;

    // Scroll behavior
    CSSOverscrollBehavior overscroll_behavior = CSSOverscrollBehavior::Auto;
    CSSOverscrollBehavior overscroll_behavior_x = CSSOverscrollBehavior::Auto;
    CSSOverscrollBehavior overscroll_behavior_y = CSSOverscrollBehavior::Auto;
    CSSScrollBehavior scroll_behavior = CSSScrollBehavior::Auto;

    CSSVisibility visibility = CSSVisibility::Visible;
    CSSCursor cursor = CSSCursor::Auto;
    
    // Outline
    float outline_width = 0.0f;
    std::string outline_color = "#000000";
    CSSBorderStyle outline_style = CSSBorderStyle::None;
    float outline_offset = 0.0f;
    
    float opacity = 1.0f;
    bool isolation_isolate = false;
    std::vector<BoxShadow> box_shadows;
    
    // Filters
    std::vector<CSSFilter> filters;
    std::vector<CSSFilter> backdrop_filters;
    CSSBlendMode mix_blend_mode = CSSBlendMode::Normal;
    CSSBlendMode background_blend_mode = CSSBlendMode::Normal;

    // Text
    std::string font_family = "Arial";
    float font_size = 16.0f;
    CSSFontWeight font_weight = CSSFontWeight::Normal;
    CSSFontStyle font_style = CSSFontStyle::Normal;
    CSSFontVariant font_variant = CSSFontVariant::Normal;
    CSSTextAlign text_align = CSSTextAlign::Left;
    CSSTextAlignLast text_align_last = CSSTextAlignLast::Auto;
    CSSTextDecoration text_decoration = CSSTextDecoration::None;
    std::string text_decoration_color;
    CSSTextDecorationStyle text_decoration_style = CSSTextDecorationStyle::Solid;
    float text_decoration_thickness = 1.0f;
    float letter_spacing_em = 0.0f;
    float word_spacing_px = 0.0f;

    // `line-height` is special: default/inherit behavior makes it hard to distinguish
    // "unspecified" from an explicit `normal` value. Track whether it was explicitly set.
    bool has_line_height = false;
    float line_height = -1.0f;
    bool line_height_is_unitless = true;

    CSSTextTransform text_transform = CSSTextTransform::None;

    CSSTextOverflow text_overflow = CSSTextOverflow::Clip;
    CSSWhiteSpace white_space = CSSWhiteSpace::Normal;
    CSSWordBreak word_break = CSSWordBreak::Normal;
    CSSOverflowWrap overflow_wrap = CSSOverflowWrap::Normal;
    CSSHyphens hyphens = CSSHyphens::Manual;
    CSSVerticalAlign vertical_align = CSSVerticalAlign::Baseline;

    CSSDirection direction = CSSDirection::Ltr;
    CSSUnicodeBidi unicode_bidi = CSSUnicodeBidi::Normal;
    float text_indent = 0.0f;
    int webkit_line_clamp = 0;
    int tab_size = 8;  // CSS tab-size property (default: 8 spaces)

    // Text shadow
    float text_shadow_offset_x = 0.0f;
    float text_shadow_offset_y = 0.0f;
    float text_shadow_blur = 0.0f;
    std::string text_shadow_color;

    // Flexbox
    CSSFlexDirection flex_direction = CSSFlexDirection::Row;
    CSSFlexWrap flex_wrap = CSSFlexWrap::Nowrap;
    CSSJustifyContent justify_content = CSSJustifyContent::FlexStart;
    CSSAlignItems justify_items = CSSAlignItems::Stretch;
    CSSAlignSelf justify_self = CSSAlignSelf::Auto;
    CSSAlignItems align_items = CSSAlignItems::Stretch;
    CSSAlignContent align_content = CSSAlignContent::Stretch;
    CSSAlignSelf align_self = CSSAlignSelf::Auto;
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
    CSSTransformStyle transform_style = CSSTransformStyle::Flat;
    float perspective = 0.0f;
    float perspective_origin_x = 50.0f;
    float perspective_origin_y = 50.0f;
    CSSBackfaceVisibility backface_visibility = CSSBackfaceVisibility::Visible;
    
    // Transitions
    std::vector<CSSTransition> transitions;
    
    // Animations
    std::vector<CSSAnimation> animations;
    
    // Clip path (basic shapes)
    std::string clip_path;
    
    // Pointer events
    CSSPointerEvents pointer_events = CSSPointerEvents::Auto;
    CSSUserSelect user_select = CSSUserSelect::Auto;
    CSSTouchAction touch_action = CSSTouchAction::Auto;
    std::string caret_color = "auto";
    
    // Float/Clear (partial support)
    CSSFloat float_value = CSSFloat::None;
    CSSClear clear = CSSClear::None;
    
    // Pseudo-element content (for ::before/::after)
    struct ContentToken {
        enum class Type {
            String,
            Counter,
            Counters,
            OpenQuote,
            CloseQuote,
            NoOpenQuote,
            NoCloseQuote,
        } type = Type::String;

        std::string text;        // String literal, or counter name
        std::string separator;   // For counters(name, separator)
        std::string style;       // Counter style: decimal, lower-alpha, upper-alpha, lower-roman, upper-roman
    };

    // `content` as authored (normalized, without trailing ';').
    // Used for parsing/evaluating `counter()` / `open-quote` etc.
    std::string content_raw;

    // Parsed tokens for `content`.
    std::vector<ContentToken> content_tokens;

    // Back-compat: when `content` is just a string literal, this holds the decoded result.
    std::string content;

    bool is_pseudo_element = false;
    CSSPseudoType pseudo_type = CSSPseudoType::None;


    // List styling
    CSSListStyleType list_style_type = CSSListStyleType::None;
    CSSListStylePosition list_style_position = CSSListStylePosition::Outside;
    std::string list_style_image = "none";     // For future, always "none" for now

    // CSS counters
    struct CounterDirective {
        std::string name;
        int value = 0; // reset value or increment amount
        bool has_value = false;
    };

    std::vector<CounterDirective> counter_resets;
    std::vector<CounterDirective> counter_increments;

    // Quotes (for open-quote/close-quote)
    // Stored as alternating open/close strings: [open1, close1, open2, close2, ...]
    std::vector<std::string> quotes = {"\u201C", "\u201D", "\u2018", "\u2019"};
    bool has_quotes = false;
    bool quotes_auto = false;  // True when quotes: auto (resolves to language-specific quotes)

    // Form control theming
    std::string accent_color = "auto";  // auto | <color>

    // Appearance (form control styling)

    CSSAppearance appearance = CSSAppearance::Auto;
    CSSResize resize = CSSResize::None;
    std::string will_change = "auto"; // auto | <animatable-feature>#


    // Table properties
    CSSBorderCollapse border_collapse = CSSBorderCollapse::Separate;
    float border_spacing_x = 2.0f;             // px (inheritable, default 2px horizontal)
    float border_spacing_y = 2.0f;             // px (inheritable, default 2px vertical)
    CSSTableLayout table_layout = CSSTableLayout::Auto;
    CSSCaptionSide caption_side = CSSCaptionSide::Top;
    

    // Track which properties were explicitly set by CSS rules or inline styles.
    // Used by inheritFromParent() to avoid overriding explicitly set values.
    std::unordered_set<std::string> explicitly_set_properties_;

    // Track properties with global keywords (inherit, initial, unset)
    std::unordered_map<std::string, std::string> global_keyword_properties_;

    // Track properties with !important declarations
    std::unordered_set<std::string> important_properties_;

    bool isExplicitlySet(const std::string& prop) const {
        return explicitly_set_properties_.count(prop) > 0;
    }
    void markExplicitlySet(const std::string& prop) {
        explicitly_set_properties_.insert(prop);
    }
    void clearExplicitlySet() {
        explicitly_set_properties_.clear();
    }

    // Check if a property has a global keyword
    bool hasGlobalKeyword(const std::string& prop) const {
        return global_keyword_properties_.count(prop) > 0;
    }
    const std::string* getGlobalKeyword(const std::string& prop) const {
        auto it = global_keyword_properties_.find(prop);
        return it != global_keyword_properties_.end() ? &it->second : nullptr;
    }

    // Check if a property has !important declaration
    bool isImportant(const std::string& prop) const {
        return important_properties_.count(prop) > 0;
    }
    void markImportant(const std::string& prop) {
        important_properties_.insert(prop);
    }

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
    void setDisplay(CSSDisplay value) {
        display = value;
        layout_mode = deriveLayoutModeFromDisplay(value);
    }

    // Convenience overload accepting string (for parse boundaries)
    void setDisplay(const std::string& value) {
        setDisplay(displayFromString(value));
    }

    // Update BFC flag based on style properties
    void updateBFCFlag();
};

} // namespace dong::dom
