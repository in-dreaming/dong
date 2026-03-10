#include "style_engine_internal.hpp"

#include <algorithm>

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

} // anonymous namespace

namespace style_engine_internal {

void applyRuleTextProperties(const ComputedStyle& rs, ComputedStyle& computed) {
    if (rs.font_size != 16.0f) { computed.font_size = rs.font_size; computed.markExplicitlySet("font-size"); }
    if (!rs.font_weight.empty() && rs.font_weight != "normal") { computed.font_weight = rs.font_weight; computed.markExplicitlySet("font-weight"); }
    if (!rs.font_style.empty() && rs.font_style != "normal") { computed.font_style = rs.font_style; computed.markExplicitlySet("font-style"); }
    if (!rs.font_family.empty() && rs.font_family != "Arial") { computed.font_family = rs.font_family; computed.markExplicitlySet("font-family"); }
    if (!rs.font_variant.empty() && rs.font_variant != "normal") computed.font_variant = rs.font_variant;

    if (!rs.text_align.empty() && rs.text_align != "left") { computed.text_align = rs.text_align; computed.markExplicitlySet("text-align"); }
    if (!rs.text_decoration.empty() && rs.text_decoration != "none")
        computed.text_decoration = rs.text_decoration;
    if (!rs.text_decoration_color.empty()) computed.text_decoration_color = rs.text_decoration_color;
    if (!rs.text_decoration_style.empty() && rs.text_decoration_style != "solid")
        computed.text_decoration_style = rs.text_decoration_style;
    if (rs.text_decoration_thickness != 1.0f)
        computed.text_decoration_thickness = rs.text_decoration_thickness;
    if (rs.letter_spacing_em != 0.0f) { computed.letter_spacing_em = rs.letter_spacing_em; computed.markExplicitlySet("letter-spacing"); }
    if (rs.word_spacing_px != 0.0f) { computed.word_spacing_px = rs.word_spacing_px; computed.markExplicitlySet("word-spacing"); }
    if (rs.has_line_height) {
        computed.has_line_height = true;
        computed.line_height = rs.line_height;
        computed.line_height_is_unitless = rs.line_height_is_unitless;
    }
    if (!rs.text_transform.empty() && rs.text_transform != "none")
        { computed.text_transform = rs.text_transform; computed.markExplicitlySet("text-transform"); }
    if (!rs.text_overflow.empty() && rs.text_overflow != "clip")
        computed.text_overflow = rs.text_overflow;
    if (!rs.white_space.empty() && rs.white_space != "normal")
        { computed.white_space = rs.white_space; computed.markExplicitlySet("white-space"); }
    if (!rs.word_break.empty() && rs.word_break != "normal")
        { computed.word_break = rs.word_break; computed.markExplicitlySet("word-break"); }
    if (!rs.overflow_wrap.empty() && rs.overflow_wrap != "normal")
        { computed.overflow_wrap = rs.overflow_wrap; computed.markExplicitlySet("overflow-wrap"); }
    if (rs.isExplicitlySet("hyphens"))
        { computed.hyphens = rs.hyphens; computed.markExplicitlySet("hyphens"); }
    if (!rs.vertical_align.empty() && rs.vertical_align != "baseline")
        computed.vertical_align = rs.vertical_align;

    if (!rs.direction.empty() && rs.direction != "ltr") { computed.direction = rs.direction; computed.markExplicitlySet("direction"); }
    if (rs.text_indent != 0.0f) { computed.text_indent = rs.text_indent; computed.markExplicitlySet("text-indent"); }
    if (rs.webkit_line_clamp != 0) computed.webkit_line_clamp = rs.webkit_line_clamp;

    // Text shadow
    if (rs.text_shadow_offset_x != 0.0f || rs.text_shadow_offset_y != 0.0f ||
        rs.text_shadow_blur != 0.0f || !rs.text_shadow_color.empty()) {
        computed.text_shadow_offset_x = rs.text_shadow_offset_x;
        computed.text_shadow_offset_y = rs.text_shadow_offset_y;
        computed.text_shadow_blur = rs.text_shadow_blur;
        computed.text_shadow_color = rs.text_shadow_color;
    }
}

void applyRuleBorderRadius(const ComputedStyle& rs, ComputedStyle& computed) {
    if (rs.border_radius != 0.0f) {
        computed.border_radius = rs.border_radius;
        CSSValue v(rs.border_radius, CSSValue::Unit::PIXEL);
        computed.border_top_left_radius = v;
        computed.border_top_right_radius = v;
        computed.border_bottom_left_radius = v;
        computed.border_bottom_right_radius = v;
    }
    if (rs.border_top_left_radius.isSet())
        computed.border_top_left_radius = rs.border_top_left_radius;
    if (rs.border_top_right_radius.isSet())
        computed.border_top_right_radius = rs.border_top_right_radius;
    if (rs.border_bottom_left_radius.isSet())
        computed.border_bottom_left_radius = rs.border_bottom_left_radius;
    if (rs.border_bottom_right_radius.isSet())
        computed.border_bottom_right_radius = rs.border_bottom_right_radius;

    if (computed.border_radius == 0.0f) {
        // Update legacy border_radius from individual corners (for backwards compatibility)
        // Use pixel value if all corners are pixels
        if (computed.border_top_left_radius.isPixel() &&
            computed.border_top_right_radius.isPixel() &&
            computed.border_bottom_left_radius.isPixel() &&
            computed.border_bottom_right_radius.isPixel()) {
            const float max_corner = std::max(
                std::max(computed.border_top_left_radius.value, computed.border_top_right_radius.value),
                std::max(computed.border_bottom_left_radius.value, computed.border_bottom_right_radius.value)
            );
            if (max_corner > 0.0f) {
                computed.border_radius = max_corner;
            }
        }
    }
}

void applyRuleBorderProperties(const ComputedStyle& rs, ComputedStyle& computed) {
    if (rs.border_width >= 0.0f) computed.border_width = rs.border_width;
    if (!rs.border_color.empty()) computed.border_color = rs.border_color;
    if (!rs.border_style.empty()) computed.border_style = rs.border_style;

    if (rs.border_top_width >= 0.0f) computed.border_top_width = rs.border_top_width;
    if (rs.border_right_width >= 0.0f) computed.border_right_width = rs.border_right_width;
    if (rs.border_bottom_width >= 0.0f) computed.border_bottom_width = rs.border_bottom_width;
    if (rs.border_left_width >= 0.0f) computed.border_left_width = rs.border_left_width;
    if (!rs.border_top_color.empty()) computed.border_top_color = rs.border_top_color;
    if (!rs.border_right_color.empty()) computed.border_right_color = rs.border_right_color;
    if (!rs.border_bottom_color.empty()) computed.border_bottom_color = rs.border_bottom_color;
    if (!rs.border_left_color.empty()) computed.border_left_color = rs.border_left_color;
    if (!rs.border_top_style.empty()) computed.border_top_style = rs.border_top_style;
    if (!rs.border_right_style.empty()) computed.border_right_style = rs.border_right_style;
    if (!rs.border_bottom_style.empty()) computed.border_bottom_style = rs.border_bottom_style;
    if (!rs.border_left_style.empty()) computed.border_left_style = rs.border_left_style;

    // Logical border properties cascade
    if (rs.border_inline_start_width >= 0.0f) computed.border_inline_start_width = rs.border_inline_start_width;
    if (!rs.border_inline_start_color.empty()) computed.border_inline_start_color = rs.border_inline_start_color;
    if (!rs.border_inline_start_style.empty()) computed.border_inline_start_style = rs.border_inline_start_style;
    if (rs.border_inline_end_width >= 0.0f) computed.border_inline_end_width = rs.border_inline_end_width;
    if (!rs.border_inline_end_color.empty()) computed.border_inline_end_color = rs.border_inline_end_color;
    if (!rs.border_inline_end_style.empty()) computed.border_inline_end_style = rs.border_inline_end_style;
    if (rs.border_block_start_width >= 0.0f) computed.border_block_start_width = rs.border_block_start_width;
    if (!rs.border_block_start_color.empty()) computed.border_block_start_color = rs.border_block_start_color;
    if (!rs.border_block_start_style.empty()) computed.border_block_start_style = rs.border_block_start_style;
    if (rs.border_block_end_width >= 0.0f) computed.border_block_end_width = rs.border_block_end_width;
    if (!rs.border_block_end_color.empty()) computed.border_block_end_color = rs.border_block_end_color;
    if (!rs.border_block_end_style.empty()) computed.border_block_end_style = rs.border_block_end_style;
}

void applyRuleOverflowProperties(const ComputedStyle& rs, ComputedStyle& computed) {
    if (!rs.overflow.empty() && rs.overflow != "visible") {
        computed.overflow = rs.overflow;
        computed.overflow_x = rs.overflow;
        computed.overflow_y = rs.overflow;
    }
    if (!rs.overflow_x.empty() && rs.overflow_x != "visible") computed.overflow_x = rs.overflow_x;
    if (!rs.overflow_y.empty() && rs.overflow_y != "visible") computed.overflow_y = rs.overflow_y;

    // Scroll behavior properties
    if (!rs.overscroll_behavior.empty() && rs.overscroll_behavior != "auto") {
        computed.overscroll_behavior = rs.overscroll_behavior;
        computed.overscroll_behavior_x = rs.overscroll_behavior;
        computed.overscroll_behavior_y = rs.overscroll_behavior;
    }
    if (!rs.overscroll_behavior_x.empty() && rs.overscroll_behavior_x != "auto") {
        computed.overscroll_behavior_x = rs.overscroll_behavior_x;
    }
    if (!rs.overscroll_behavior_y.empty() && rs.overscroll_behavior_y != "auto") {
        computed.overscroll_behavior_y = rs.overscroll_behavior_y;
    }
    if (!rs.scroll_behavior.empty() && rs.scroll_behavior != "auto") {
        computed.scroll_behavior = rs.scroll_behavior;
    }

    if (!rs.visibility.empty() && rs.visibility != "visible") { computed.visibility = rs.visibility; computed.markExplicitlySet("visibility"); }
    if (!rs.cursor.empty() && rs.cursor != "auto") { computed.cursor = rs.cursor; computed.markExplicitlySet("cursor"); }
}

void applyRuleBoxModel(const ComputedStyle& rs, ComputedStyle& computed) {
    // width/height/min/max: only apply if explicitly set (not UNSET)
    if (rs.width.isSet() && !rs.width.isAuto()) computed.width = rs.width;
    if (rs.height.isSet() && !rs.height.isAuto()) computed.height = rs.height;
    if (rs.min_width.isSet() && !rs.min_width.isAuto()) computed.min_width = rs.min_width;
    if (rs.max_width.isSet() && !rs.max_width.isAuto()) computed.max_width = rs.max_width;
    if (rs.min_height.isSet() && !rs.min_height.isAuto()) computed.min_height = rs.min_height;
    if (rs.max_height.isSet() && !rs.max_height.isAuto()) computed.max_height = rs.max_height;

    // margin: only apply if explicitly set
    if (rs.margin_top.isSet()) computed.margin_top = rs.margin_top;
    if (rs.margin_right.isSet()) computed.margin_right = rs.margin_right;
    if (rs.margin_bottom.isSet()) computed.margin_bottom = rs.margin_bottom;
    if (rs.margin_left.isSet()) computed.margin_left = rs.margin_left;

    // padding: only apply if explicitly set
    if (rs.padding_top.isSet()) computed.padding_top = rs.padding_top;
    if (rs.padding_right.isSet()) computed.padding_right = rs.padding_right;
    if (rs.padding_bottom.isSet()) computed.padding_bottom = rs.padding_bottom;
    if (rs.padding_left.isSet()) computed.padding_left = rs.padding_left;

    // position offsets: only apply if explicitly set
    if (rs.top.isSet() && !rs.top.isAuto()) computed.top = rs.top;
    if (rs.right.isSet() && !rs.right.isAuto()) computed.right = rs.right;
    if (rs.bottom.isSet() && !rs.bottom.isAuto()) computed.bottom = rs.bottom;
    if (rs.left.isSet() && !rs.left.isAuto()) computed.left = rs.left;

    if (rs.z_index != 0) computed.z_index = rs.z_index;
}

void applyRuleFlexbox(const ComputedStyle& rs, ComputedStyle& computed) {
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
    if (rs.flex_basis.isSet() && !rs.flex_basis.isAuto()) computed.flex_basis = rs.flex_basis;
    if (rs.order != 0) computed.order = rs.order;
    if (rs.gap > 0.0f) {
        computed.gap = rs.gap;
        computed.row_gap = rs.gap;
        computed.column_gap = rs.gap;
    }
    if (rs.row_gap > 0.0f) computed.row_gap = rs.row_gap;
    if (rs.column_gap > 0.0f) computed.column_gap = rs.column_gap;
}

void applyRuleTransform(const ComputedStyle& rs, ComputedStyle& computed) {
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
}

// ── Main shared property application function ──
// Called by both applyMatchingRules() and applyMatchingRulesIndexed().
void applyRuleProperties(const ComputedStyle& rs, ComputedStyle& computed) {
    // Helper macro to apply property with !important support
    // Only apply if: target is not !important OR source is also !important
    #define APPLY_PROP(prop_name, src_val, condition) \
        if ((condition) && (!computed.isImportant(prop_name) || rs.isImportant(prop_name))) { \
            src_val; \
            if (rs.isImportant(prop_name)) computed.markImportant(prop_name); \
        }

    // Visual / color
    APPLY_PROP("color", computed.color = rs.color; computed.markExplicitlySet("color"),
               !rs.color.empty() && rs.color != "#000000");
    APPLY_PROP("background-color", computed.background_color = rs.background_color,
               !rs.background_color.empty() && rs.background_color != "transparent");
    APPLY_PROP("background-image", computed.background_image = rs.background_image,
               !rs.background_image.empty());
    APPLY_PROP("background-size", computed.background_size = rs.background_size,
               !rs.background_size.empty() && rs.background_size != "auto");
    APPLY_PROP("background-repeat", computed.background_repeat = rs.background_repeat,
               !rs.background_repeat.empty() && rs.background_repeat != "repeat");
    APPLY_PROP("background-position", computed.background_position = rs.background_position,
               !rs.background_position.empty() && rs.background_position != "0% 0%");
    APPLY_PROP("background-attachment", computed.background_attachment = rs.background_attachment,
               !rs.background_attachment.empty() && rs.background_attachment != "scroll");
    APPLY_PROP("background-clip", computed.background_clip = rs.background_clip,
               !rs.background_clip.empty() && rs.background_clip != "border-box");
    APPLY_PROP("background-origin", computed.background_origin = rs.background_origin,
               !rs.background_origin.empty() && rs.background_origin != "padding-box");
    APPLY_PROP("object-fit", computed.object_fit = rs.object_fit,
               !rs.object_fit.empty() && rs.object_fit != "fill");
    APPLY_PROP("object-position", computed.object_position = rs.object_position,
               !rs.object_position.empty() && rs.object_position != "50% 50%");
    APPLY_PROP("image-rendering", computed.image_rendering = rs.image_rendering,
               !rs.image_rendering.empty() && rs.image_rendering != "auto");
    APPLY_PROP("background-gradient", computed.background_gradients = rs.background_gradients,
               !rs.background_gradients.empty());

    // Color scheme hint (UA/form control theming)
    APPLY_PROP("color-scheme",
               computed.color_scheme = rs.color_scheme; computed.markExplicitlySet("color-scheme"),
               rs.isExplicitlySet("color-scheme"));

    applyRuleTextProperties(rs, computed);


    // Display / position
    if (!rs.display.empty() && (!computed.isImportant("display") || rs.isImportant("display"))) {
        computed.display = rs.display;
        computed.layout_mode = deriveLayoutModeFromDisplay(computed);
        if (rs.isImportant("display")) computed.markImportant("display");
    }
    APPLY_PROP("position", computed.position = rs.position,
               !rs.position.empty() && rs.position != "static");

    applyRuleBorderRadius(rs, computed);
    applyRuleBorderProperties(rs, computed);
    applyRuleOverflowProperties(rs, computed);

    // List styling (affects ::marker auto-generation)
    APPLY_PROP("list-style-type",
               computed.list_style_type = rs.list_style_type; computed.markExplicitlySet("list-style-type"),
               rs.isExplicitlySet("list-style-type") || rs.isExplicitlySet("list-style"));
    APPLY_PROP("list-style-position",
               computed.list_style_position = rs.list_style_position; computed.markExplicitlySet("list-style-position"),
               rs.isExplicitlySet("list-style-position") || rs.isExplicitlySet("list-style"));

    // Outline

    APPLY_PROP("outline-width", computed.outline_width = rs.outline_width,
               rs.outline_width != 0.0f);
    APPLY_PROP("outline-color", computed.outline_color = rs.outline_color,
               !rs.outline_color.empty() && rs.outline_color != "#000000");
    APPLY_PROP("outline-style", computed.outline_style = rs.outline_style,
               !rs.outline_style.empty() && rs.outline_style != "none");
    APPLY_PROP("outline-offset", computed.outline_offset = rs.outline_offset,
               rs.outline_offset != 0.0f);

    // Box misc
    APPLY_PROP("box-sizing", computed.box_sizing = rs.box_sizing,
               !rs.box_sizing.empty() && rs.box_sizing != "content-box");
    APPLY_PROP("opacity", computed.opacity = rs.opacity,
               rs.opacity != 1.0f);
    APPLY_PROP("isolation", computed.isolation_isolate = true,
               rs.isolation_isolate);
    APPLY_PROP("box-shadow", computed.box_shadows = rs.box_shadows,
               !rs.box_shadows.empty());

    // Filters
    APPLY_PROP("filter", computed.filters = rs.filters,
               !rs.filters.empty());
    APPLY_PROP("backdrop-filter", computed.backdrop_filters = rs.backdrop_filters,
               !rs.backdrop_filters.empty());
    APPLY_PROP("mix-blend-mode", computed.mix_blend_mode = rs.mix_blend_mode,
               !rs.mix_blend_mode.empty() && rs.mix_blend_mode != "normal");
    APPLY_PROP("background-blend-mode", computed.background_blend_mode = rs.background_blend_mode,
               !rs.background_blend_mode.empty() && rs.background_blend_mode != "normal");

    applyRuleBoxModel(rs, computed);
    applyRuleFlexbox(rs, computed);
    applyRuleTransform(rs, computed);

    // Transitions and animations
    APPLY_PROP("transition", computed.transitions = rs.transitions,
               !rs.transitions.empty());
    APPLY_PROP("animation", computed.animations = rs.animations,
               !rs.animations.empty());

    // Clip path
    APPLY_PROP("clip-path", computed.clip_path = rs.clip_path,
               !rs.clip_path.empty());

    // Pointer events / interaction
    APPLY_PROP("pointer-events", computed.pointer_events = rs.pointer_events,
               !rs.pointer_events.empty() && rs.pointer_events != "auto");
    APPLY_PROP("user-select", computed.user_select = rs.user_select,
               !rs.user_select.empty() && rs.user_select != "auto");
    APPLY_PROP("touch-action", computed.touch_action = rs.touch_action,
               !rs.touch_action.empty() && rs.touch_action != "auto");
    APPLY_PROP("caret-color", computed.caret_color = rs.caret_color,
               !rs.caret_color.empty() && rs.caret_color != "auto");

    // Float/Clear
    APPLY_PROP("float", computed.float_value = rs.float_value,
               !rs.float_value.empty() && rs.float_value != "none");
    APPLY_PROP("clear", computed.clear = rs.clear,
               !rs.clear.empty() && rs.clear != "none");

    // Form control theming
    APPLY_PROP("accent-color", computed.accent_color = rs.accent_color; computed.markExplicitlySet("accent-color"),
               !rs.accent_color.empty() && rs.accent_color != "auto");

    // Appearance
    APPLY_PROP("appearance", computed.appearance = rs.appearance; computed.markExplicitlySet("appearance"),
               !rs.appearance.empty() && rs.appearance != "auto");
    APPLY_PROP("resize", computed.resize = rs.resize; computed.markExplicitlySet("resize"),
               !rs.resize.empty() && rs.resize != "none");
    APPLY_PROP("will-change", computed.will_change = rs.will_change; computed.markExplicitlySet("will-change"),
               !rs.will_change.empty() && rs.will_change != "auto");


    // Table properties

    APPLY_PROP("border-collapse", computed.border_collapse = rs.border_collapse; computed.markExplicitlySet("border-collapse"),
               !rs.border_collapse.empty() && rs.border_collapse != "separate");
    APPLY_PROP("border-spacing", computed.border_spacing_x = rs.border_spacing_x; computed.border_spacing_y = rs.border_spacing_y; computed.markExplicitlySet("border-spacing"),
               rs.border_spacing_x != 2.0f || rs.border_spacing_y != 2.0f);
    APPLY_PROP("table-layout", computed.table_layout = rs.table_layout; computed.markExplicitlySet("table-layout"),
               !rs.table_layout.empty() && rs.table_layout != "auto");
    APPLY_PROP("caption-side", computed.caption_side = rs.caption_side; computed.markExplicitlySet("caption-side"),
               !rs.caption_side.empty() && rs.caption_side != "top");

    // Counters / generated content

    APPLY_PROP("counter-reset",
               computed.counter_resets = rs.counter_resets; computed.markExplicitlySet("counter-reset"),
               rs.isExplicitlySet("counter-reset"));
    APPLY_PROP("counter-increment",
               computed.counter_increments = rs.counter_increments; computed.markExplicitlySet("counter-increment"),
               rs.isExplicitlySet("counter-increment"));
    APPLY_PROP("quotes",
               computed.quotes = rs.quotes; computed.has_quotes = rs.has_quotes; computed.quotes_auto = rs.quotes_auto; computed.markExplicitlySet("quotes"),
               rs.isExplicitlySet("quotes"));
    APPLY_PROP("content",
               computed.content_raw = rs.content_raw; computed.content_tokens = rs.content_tokens; computed.content = rs.content;
               computed.markExplicitlySet("content"),
               rs.isExplicitlySet("content"));

    #undef APPLY_PROP
}

// Apply only !important properties from a rule
// This is called after inline styles to enforce !important semantics
void applyImportantPropertiesOnly(const ComputedStyle& rs, ComputedStyle& computed) {
    // Helper macro to apply property only if marked as !important
    #define APPLY_IF_IMPORTANT(prop_name, src_val, condition) \
        if ((condition) && rs.isImportant(prop_name)) { \
            src_val; \
            computed.markImportant(prop_name); \
        }

    // Visual / color
    APPLY_IF_IMPORTANT("color", computed.color = rs.color; computed.markExplicitlySet("color"),
                       !rs.color.empty() && rs.color != "#000000");
    APPLY_IF_IMPORTANT("background-color", computed.background_color = rs.background_color,
                       !rs.background_color.empty() && rs.background_color != "transparent");
    APPLY_IF_IMPORTANT("background-image", computed.background_image = rs.background_image,
                       !rs.background_image.empty());

    // Color scheme hint (UA/form control theming)
    APPLY_IF_IMPORTANT("color-scheme",
                       computed.color_scheme = rs.color_scheme; computed.markExplicitlySet("color-scheme"),
                       !rs.color_scheme.empty());

    // Display / position

    if (!rs.display.empty() && rs.isImportant("display")) {
        computed.display = rs.display;
        computed.layout_mode = deriveLayoutModeFromDisplay(computed);
        computed.markImportant("display");
    }
    APPLY_IF_IMPORTANT("position", computed.position = rs.position,
                       !rs.position.empty() && rs.position != "static");

    // Font properties
    if (rs.font_size != 16.0f && rs.isImportant("font-size")) {
        computed.font_size = rs.font_size;
        computed.markExplicitlySet("font-size");
        computed.markImportant("font-size");
    }
    APPLY_IF_IMPORTANT("font-family", computed.font_family = rs.font_family; computed.markExplicitlySet("font-family"),
                       !rs.font_family.empty() && rs.font_family != "Arial");
    APPLY_IF_IMPORTANT("font-weight", computed.font_weight = rs.font_weight; computed.markExplicitlySet("font-weight"),
                       !rs.font_weight.empty() && rs.font_weight != "normal");

    // Add more properties as needed...
    // For now, this covers the most common use cases

    #undef APPLY_IF_IMPORTANT
}

} // namespace style_engine_internal

} // namespace dong::dom