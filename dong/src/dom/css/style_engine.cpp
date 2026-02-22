#include "style_engine.hpp"
#include "../dom/dom_node.hpp"


#include "../../core/log.h"
#include "../../core/profiler.h"
#include <algorithm>
#include <sstream>
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

// ── Sub-functions for applyRuleProperties (declared before the caller) ──

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

    if (computed.border_radius == 0.0f) {
        const float max_corner = std::max(
            std::max(computed.border_top_left_radius, computed.border_top_right_radius),
            std::max(computed.border_bottom_left_radius, computed.border_bottom_right_radius)
        );
        if (max_corner > 0.0f) {
            computed.border_radius = max_corner;
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
}

void applyRuleOverflowProperties(const ComputedStyle& rs, ComputedStyle& computed) {
    if (!rs.overflow.empty() && rs.overflow != "visible") {
        computed.overflow = rs.overflow;
        computed.overflow_x = rs.overflow;
        computed.overflow_y = rs.overflow;
    }
    if (!rs.overflow_x.empty() && rs.overflow_x != "visible") computed.overflow_x = rs.overflow_x;
    if (!rs.overflow_y.empty() && rs.overflow_y != "visible") computed.overflow_y = rs.overflow_y;
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
    APPLY_PROP("background-gradient", computed.background_gradients = rs.background_gradients,
               !rs.background_gradients.empty());

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

void applyInlineStyleAttributeIfAny(DOMNodePtr node) {
    if (!node) return;
    if (!node->hasAttribute("style")) return;
    const std::string style_str = node->getAttribute("style");
    if (style_str.empty()) return;

    // Inline style has the highest precedence in author styles.
    CSSParser::parseInlineStyle(style_str, node->getComputedStyle());
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
    {"ul", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"ol", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"li", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"dl", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"dt", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"dd", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"form", [](ComputedStyle& s) { s.setDisplay("block"); }},
    {"fieldset", [](ComputedStyle& s) { s.setDisplay("block"); }},

    // Inline elements
    {"span", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"a", [](ComputedStyle& s) { s.setDisplay("inline"); s.color = "#0000EE"; s.text_decoration = "underline"; s.cursor = "pointer"; }},
    {"b", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_weight = "bold"; }},
    {"i", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
    {"strong", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_weight = "bold"; }},
    {"em", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
    {"code", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; }},
    {"kbd", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; }},
    {"samp", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; }},
    {"var", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
    {"small", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_size = 12.0f; }},
    {"s", [](ComputedStyle& s) { s.setDisplay("inline"); s.text_decoration = "line-through"; }},
    {"cite", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
    {"q", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"mark", [](ComputedStyle& s) { s.setDisplay("inline"); s.background_color = "#ffff00"; }},
    {"sub", [](ComputedStyle& s) { s.setDisplay("inline"); s.vertical_align = "sub"; s.font_size = 12.0f; }},
    {"sup", [](ComputedStyle& s) { s.setDisplay("inline"); s.vertical_align = "super"; s.font_size = 12.0f; }},
    {"u", [](ComputedStyle& s) { s.setDisplay("inline"); s.text_decoration = "underline"; }},
    {"abbr", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"time", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"data", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"wbr", [](ComputedStyle& s) { s.setDisplay("inline"); }},
    {"label", [](ComputedStyle& s) { s.setDisplay("inline"); }},

    // Headings
    {"h1", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 32.0f; }},
    {"h2", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 28.0f; }},
    {"h3", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 24.0f; }},
    {"h4", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 20.0f; }},
    {"h5", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 18.0f; }},
    {"h6", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 16.0f; }},

    // Form elements (inline-block)
    {"button", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"input", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"select", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
    {"textarea", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},

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
    }},

    // Table elements
    {"table", [](ComputedStyle& s) { s.setDisplay("table"); }},
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

/* Basic block spacing to better match browser defaults. */
h1 {
  margin: 0.67em 0;
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
}


button {
  padding: 2px 6px;
  background-color: #f3f3f3;
  white-space: nowrap;

  border-width: 2px;
  border-style: outset;
  border-color: #767676;
  border-radius: 2px;
  cursor: pointer;
}
button:active {

  border-style: inset;
}

input, select, textarea {
  padding: 2px 4px;
  background-color: #ffffff;
  border-width: 2px;
  border-style: inset;
  border-color: #767676;
  border-radius: 2px;
  min-height: 24px;
}

/* Default focus style for inputs (matches browser default outline) */
input:focus, select:focus, textarea:focus {
  outline: 2px solid #4A90E2;
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
  border-width: 1px;
  border-style: solid;
  border-color: #767676;
  background-color: #ffffff;
  border-radius: 2px;
  box-sizing: border-box;
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
    index_dirty_ = true;  // 标记索引需要重建
}

void StyleEngine::addStylesheet(const Stylesheet& sheet) {
    stylesheets_.push_back(sheet);
    index_dirty_ = true;  // 标记索引需要重建
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

    // Apply all properties from matching rules
    auto& computed = node->getComputedStyle();
    for (const auto& rule : matching_rules) {
        applyRuleProperties(rule.style, computed);
    }

    // Inherit from parent
    inheritFromParent(node);

    // Inline style overrides author rules
    applyInlineStyleAttributeIfAny(node);

    // Re-apply !important properties from rules - these override inline styles
    for (const auto& rule : matching_rules) {
        applyImportantPropertiesOnly(rule.style, computed);
    }

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
    applyDefaultStyleForNode(node);
    applyMatchingRules(node);
    inheritFromParent(node);
    applyInlineStyleAttributeIfAny(node);
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
        applyRuleProperties(rule.style, computed);
    }
}

void StyleEngine::processGlobalKeywords(DOMNodePtr node, DOMNodePtr parent) {
    if (!node || !parent) return;

    auto& computed = node->getComputedStyle();
    const auto& parent_style = parent->getComputedStyle();

    // 全局关键字 initial/unset 的语义必须“覆盖掉已经算出来的值”。
    // 不能仅依赖 ComputedStyle 的构造默认值，否则会把更早匹配到的规则残留在 computed 里。
    static const ComputedStyle kInitial;

    auto resetToInitial = [&](const std::string& prop) {
        if (prop == "color") computed.color = kInitial.color;
        else if (prop == "font-family") computed.font_family = kInitial.font_family;
        else if (prop == "font-size") computed.font_size = kInitial.font_size;
        else if (prop == "font-weight") computed.font_weight = kInitial.font_weight;
        else if (prop == "font-style") computed.font_style = kInitial.font_style;
        else if (prop == "text-align") computed.text_align = kInitial.text_align;
        else if (prop == "line-height") {
            computed.has_line_height = kInitial.has_line_height;
            computed.line_height = kInitial.line_height;
            computed.line_height_is_unitless = kInitial.line_height_is_unitless;
        }
        else if (prop == "letter-spacing") computed.letter_spacing_em = kInitial.letter_spacing_em;
        else if (prop == "word-spacing") computed.word_spacing_px = kInitial.word_spacing_px;
        else if (prop == "white-space") computed.white_space = kInitial.white_space;
        else if (prop == "direction") computed.direction = kInitial.direction;
        else if (prop == "cursor") computed.cursor = kInitial.cursor;
        else if (prop == "visibility") computed.visibility = kInitial.visibility;
        else if (prop == "text-indent") computed.text_indent = kInitial.text_indent;
        else if (prop == "text-transform") computed.text_transform = kInitial.text_transform;
        else if (prop == "word-break") computed.word_break = kInitial.word_break;
        else if (prop == "overflow-wrap") computed.overflow_wrap = kInitial.overflow_wrap;

        // Non-inheritable properties used by tests
        else if (prop == "border") {
            computed.border_color = kInitial.border_color;
            computed.border_width = kInitial.border_width;
            computed.border_style = kInitial.border_style;

            computed.border_top_width = kInitial.border_top_width;
            computed.border_right_width = kInitial.border_right_width;
            computed.border_bottom_width = kInitial.border_bottom_width;
            computed.border_left_width = kInitial.border_left_width;
            computed.border_top_color = kInitial.border_top_color;
            computed.border_right_color = kInitial.border_right_color;
            computed.border_bottom_color = kInitial.border_bottom_color;
            computed.border_left_color = kInitial.border_left_color;
            computed.border_top_style = kInitial.border_top_style;
            computed.border_right_style = kInitial.border_right_style;
            computed.border_bottom_style = kInitial.border_bottom_style;
            computed.border_left_style = kInitial.border_left_style;
        }
        else if (prop == "padding") {
            computed.padding_top = kInitial.padding_top;
            computed.padding_right = kInitial.padding_right;
            computed.padding_bottom = kInitial.padding_bottom;
            computed.padding_left = kInitial.padding_left;
        }
        else if (prop == "margin") {
            computed.margin_top = kInitial.margin_top;
            computed.margin_right = kInitial.margin_right;
            computed.margin_bottom = kInitial.margin_bottom;
            computed.margin_left = kInitial.margin_left;
        }
        else if (prop == "background-color") {
            computed.background_color = kInitial.background_color;
        }
        else if (prop == "width") computed.width = kInitial.width;
        else if (prop == "height") computed.height = kInitial.height;
        else if (prop == "display") {
            computed.display = kInitial.display;
            computed.layout_mode = kInitial.layout_mode;
        }
    };

    // Define inheritable properties
    static const std::unordered_set<std::string> kInheritable = {
        "color", "font-family", "font-size", "font-weight", "font-style",
        "text-align", "line-height", "letter-spacing", "word-spacing",
        "white-space", "direction", "cursor", "visibility",
        "text-indent", "text-transform", "word-break", "overflow-wrap"
    };

    // Process each global keyword property
    for (const auto& [prop, keyword] : computed.global_keyword_properties_) {
        if (keyword == "inherit") {
            // Force inherit from parent, regardless of whether property is inheritable
            copyPropertyFromParent(prop, computed, parent_style);
        } else if (keyword == "initial") {
            resetToInitial(prop);
        } else if (keyword == "unset") {
            // If inheritable: behave like inherit; otherwise: behave like initial
            if (kInheritable.count(prop)) {
                copyPropertyFromParent(prop, computed, parent_style);
            } else {
                resetToInitial(prop);
            }
        }
    }
}

void StyleEngine::copyPropertyFromParent(const std::string& prop,
                                         ComputedStyle& child_style,
                                         const ComputedStyle& parent_style) {
    // Copy property value from parent to child
    // This is a simplified implementation - in production, you'd use a property map
    if (prop == "color") child_style.color = parent_style.color;
    else if (prop == "font-family") child_style.font_family = parent_style.font_family;
    else if (prop == "font-size") child_style.font_size = parent_style.font_size;
    else if (prop == "font-weight") child_style.font_weight = parent_style.font_weight;
    else if (prop == "font-style") child_style.font_style = parent_style.font_style;
    else if (prop == "text-align") child_style.text_align = parent_style.text_align;
    else if (prop == "line-height") {
        child_style.line_height = parent_style.line_height;
        child_style.line_height_is_unitless = parent_style.line_height_is_unitless;
    }
    else if (prop == "letter-spacing") child_style.letter_spacing_em = parent_style.letter_spacing_em;
    else if (prop == "word-spacing") child_style.word_spacing_px = parent_style.word_spacing_px;
    else if (prop == "white-space") child_style.white_space = parent_style.white_space;
    else if (prop == "direction") child_style.direction = parent_style.direction;
    else if (prop == "cursor") child_style.cursor = parent_style.cursor;
    else if (prop == "visibility") child_style.visibility = parent_style.visibility;
    else if (prop == "text-indent") child_style.text_indent = parent_style.text_indent;
    else if (prop == "text-transform") child_style.text_transform = parent_style.text_transform;
    else if (prop == "word-break") child_style.word_break = parent_style.word_break;
    else if (prop == "overflow-wrap") child_style.overflow_wrap = parent_style.overflow_wrap;
    // Non-inheritable properties
    else if (prop == "border") {
        // Copy the full border shorthand + per-side overrides.
        // This is required for correct `border: inherit` semantics.
        child_style.border_color = parent_style.border_color;
        child_style.border_width = parent_style.border_width;
        child_style.border_style = parent_style.border_style;

        child_style.border_top_width = parent_style.border_top_width;
        child_style.border_right_width = parent_style.border_right_width;
        child_style.border_bottom_width = parent_style.border_bottom_width;
        child_style.border_left_width = parent_style.border_left_width;
        child_style.border_top_color = parent_style.border_top_color;
        child_style.border_right_color = parent_style.border_right_color;
        child_style.border_bottom_color = parent_style.border_bottom_color;
        child_style.border_left_color = parent_style.border_left_color;
        child_style.border_top_style = parent_style.border_top_style;
        child_style.border_right_style = parent_style.border_right_style;
        child_style.border_bottom_style = parent_style.border_bottom_style;
        child_style.border_left_style = parent_style.border_left_style;
    }
    else if (prop == "margin") {
        child_style.margin_top = parent_style.margin_top;
        child_style.margin_right = parent_style.margin_right;
        child_style.margin_bottom = parent_style.margin_bottom;
        child_style.margin_left = parent_style.margin_left;
    }
    else if (prop == "padding") {
        child_style.padding_top = parent_style.padding_top;
        child_style.padding_right = parent_style.padding_right;
        child_style.padding_bottom = parent_style.padding_bottom;
        child_style.padding_left = parent_style.padding_left;
    }
    else if (prop == "background-color") child_style.background_color = parent_style.background_color;
    else if (prop == "width") child_style.width = parent_style.width;
    else if (prop == "height") child_style.height = parent_style.height;
    else if (prop == "display") {
        child_style.display = parent_style.display;
        child_style.layout_mode = parent_style.layout_mode;
    }
    // Add more properties as needed
}

void StyleEngine::inheritFromParent(DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return;

    auto& computed = node->getComputedStyle();
    const auto& parent_style = parent->getComputedStyle();

    // Process global keywords first
    processGlobalKeywords(node, parent);

    // Inherit text properties (only if not explicitly set)
    if (!computed.isExplicitlySet("color")) computed.color = parent_style.color;
    if (!computed.isExplicitlySet("font-family")) computed.font_family = parent_style.font_family;
    if (!computed.isExplicitlySet("font-size")) computed.font_size = parent_style.font_size;
    if (!computed.isExplicitlySet("font-weight")) computed.font_weight = parent_style.font_weight;
    if (!computed.isExplicitlySet("font-style")) computed.font_style = parent_style.font_style;
    if (!computed.isExplicitlySet("text-align")) computed.text_align = parent_style.text_align;
    if (!computed.has_line_height) {
        computed.line_height = parent_style.line_height;
        computed.line_height_is_unitless = parent_style.line_height_is_unitless;
    }

    if (!computed.isExplicitlySet("letter-spacing")) computed.letter_spacing_em = parent_style.letter_spacing_em;
    if (!computed.isExplicitlySet("word-spacing")) computed.word_spacing_px = parent_style.word_spacing_px;
    if (!computed.isExplicitlySet("white-space")) computed.white_space = parent_style.white_space;
    if (!computed.isExplicitlySet("direction")) computed.direction = parent_style.direction;
    if (!computed.isExplicitlySet("cursor")) computed.cursor = parent_style.cursor;
    if (!computed.isExplicitlySet("visibility")) computed.visibility = parent_style.visibility;
    if (!computed.isExplicitlySet("text-indent")) computed.text_indent = parent_style.text_indent;
    if (!computed.isExplicitlySet("text-transform")) computed.text_transform = parent_style.text_transform;
    if (!computed.isExplicitlySet("word-break")) computed.word_break = parent_style.word_break;
    if (!computed.isExplicitlySet("overflow-wrap")) computed.overflow_wrap = parent_style.overflow_wrap;
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
                    const auto& rs = rule.style;
                    if (!rs.content.empty() || rs.content == "") {
                        before_style.content = rs.content;
                    }
                    applyRuleProperties(rs, before_style);
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
                    applyRuleProperties(rs, after_style);
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

// 优化策略3：重建规则索引
void StyleEngine::rebuildRuleIndex() {
    DONG_PROFILE_SCOPE_CAT("StyleEngine::rebuildRuleIndex", "style");
    
    rule_index_.clear();
    all_rules_.clear();
    
    int normalized_source_order = 0;

    // 收集所有规则到扁平列表（并按"样式表顺序 + 规则顺序"重排 source_order）
    for (const auto& sheet : stylesheets_) {
        for (const auto& rule : sheet.getRules()) {
            CSSRule normalized = rule;
            normalized.source_order = normalized_source_order++;

            size_t idx = all_rules_.size();
            all_rules_.push_back(normalized);
            
            // 从选择器中提取索引键
            std::string tag;
            std::vector<std::string> classes;
            std::string id;
            extractIndexKeys(normalized.selector, tag, classes, id);

            
            // 添加到索引
            bool indexed = false;
            
            if (!id.empty()) {
                rule_index_.by_id[id].push_back(idx);
                indexed = true;
            }
            
            for (const auto& cls : classes) {
                rule_index_.by_class[cls].push_back(idx);
                indexed = true;
            }
            
            if (!tag.empty() && tag != "*") {
                rule_index_.by_tag[tag].push_back(idx);
                indexed = true;
            }
            
            // 如果没有被任何索引收录，放入通用列表
            if (!indexed || tag == "*") {
                rule_index_.universal.push_back(idx);
            }
        }
    }
    
    index_dirty_ = false;
}

// 优化策略3：从选择器中提取索引键
void StyleEngine::extractIndexKeys(const std::string& selector, 
                                    std::string& out_tag, 
                                    std::vector<std::string>& out_classes,
                                    std::string& out_id) {
    out_tag.clear();
    out_classes.clear();
    out_id.clear();
    
    // 只提取选择器最右边的简单选择器（最具体的部分）
    // 例如 "div .container button" -> 只看 "button"
    // 例如 ".header .nav-item" -> 只看 ".nav-item"
    
    // 找到最后一个组合符（空格、>、+、~）
    size_t last_combinator = selector.find_last_of(" >+~");
    std::string simple_selector = (last_combinator != std::string::npos) 
        ? selector.substr(last_combinator + 1) 
        : selector;
    
    // 去除前导空格
    size_t start = simple_selector.find_first_not_of(" \t");
    if (start != std::string::npos) {
        simple_selector = simple_selector.substr(start);
    }
    
    // 解析简单选择器
    size_t pos = 0;
    while (pos < simple_selector.length()) {
        char c = simple_selector[pos];
        
        if (c == '#') {
            // ID 选择器
            ++pos;
            size_t id_start = pos;
            while (pos < simple_selector.length() && 
                   (std::isalnum(simple_selector[pos]) || simple_selector[pos] == '-' || simple_selector[pos] == '_')) {
                ++pos;
            }
            out_id = simple_selector.substr(id_start, pos - id_start);
        } else if (c == '.') {
            // Class 选择器
            ++pos;
            size_t class_start = pos;
            while (pos < simple_selector.length() && 
                   (std::isalnum(simple_selector[pos]) || simple_selector[pos] == '-' || simple_selector[pos] == '_')) {
                ++pos;
            }
            out_classes.push_back(simple_selector.substr(class_start, pos - class_start));
        } else if (c == '[' || c == ':') {
            // 属性选择器或伪类，跳过
            if (c == '[') {
                size_t end = simple_selector.find(']', pos);
                pos = (end != std::string::npos) ? end + 1 : simple_selector.length();
            } else {
                // 伪类
                ++pos;
                while (pos < simple_selector.length() && 
                       (std::isalpha(simple_selector[pos]) || simple_selector[pos] == '-')) {
                    ++pos;
                }
                // 处理 :not(), :has() 等函数式伪类
                if (pos < simple_selector.length() && simple_selector[pos] == '(') {
                    int paren_depth = 1;
                    ++pos;
                    while (pos < simple_selector.length() && paren_depth > 0) {
                        if (simple_selector[pos] == '(') ++paren_depth;
                        else if (simple_selector[pos] == ')') --paren_depth;
                        ++pos;
                    }
                }
            }
        } else if (std::isalpha(c) || c == '*') {
            // 标签选择器
            size_t tag_start = pos;
            // 处理 * 通配符
            if (c == '*') {
                ++pos;
            }
            while (pos < simple_selector.length() && 
                   (std::isalnum(simple_selector[pos]) || simple_selector[pos] == '-')) {
                ++pos;
            }
            out_tag = simple_selector.substr(tag_start, pos - tag_start);
        } else {
            ++pos;
        }
    }
}

// 优化策略3：使用索引快速查找匹配规则
void StyleEngine::applyMatchingRulesIndexed(DOMNodePtr node) {
    if (index_dirty_) {
        rebuildRuleIndex();
    }
    
    // 收集候选规则索引
    std::unordered_set<size_t> candidate_indices;
    
    // 1. 通用规则总是候选
    for (size_t idx : rule_index_.universal) {
        candidate_indices.insert(idx);
    }
    
    // 2. 按 tag 查找
    const std::string& tag = node->getTagName();
    auto tag_it = rule_index_.by_tag.find(tag);
    if (tag_it != rule_index_.by_tag.end()) {
        for (size_t idx : tag_it->second) {
            candidate_indices.insert(idx);
        }
    }
    
    // 3. 按 class 查找
    if (node->hasAttribute("class")) {
        const std::string& class_attr = node->getAttribute("class");
        // 分割 class 属性
        std::istringstream iss(class_attr);
        std::string cls;
        while (iss >> cls) {
            auto class_it = rule_index_.by_class.find(cls);
            if (class_it != rule_index_.by_class.end()) {
                for (size_t idx : class_it->second) {
                    candidate_indices.insert(idx);
                }
            }
        }
    }
    
    // 4. 按 id 查找
    if (node->hasAttribute("id")) {
        const std::string& id = node->getAttribute("id");
        auto id_it = rule_index_.by_id.find(id);
        if (id_it != rule_index_.by_id.end()) {
            for (size_t idx : id_it->second) {
                candidate_indices.insert(idx);
            }
        }
    }
    
    // 收集实际匹配的规则
    std::vector<CSSRule> matching_rules;
    matching_rules.reserve(candidate_indices.size());
    
    for (size_t idx : candidate_indices) {
        const CSSRule& rule = all_rules_[idx];
        if (matcher_.matches(rule.selector, node)) {
            matching_rules.push_back(rule);
        }
    }
    
    // 按 specificity 和 source order 排序
    std::sort(matching_rules.begin(), matching_rules.end(),
        [](const CSSRule& a, const CSSRule& b) {
            if (a.specificity != b.specificity) {
                return a.specificity < b.specificity;
            }
            return a.source_order < b.source_order;
        });
    
    // 应用规则（使用共享的属性应用函数）
    auto& computed = node->getComputedStyle();
    for (const auto& rule : matching_rules) {
        applyRuleProperties(rule.style, computed);
    }
}

// 优化策略3：增量样式计算 - 只重算 dirty 节点
void StyleEngine::computeStylesIncremental(DOMNodePtr node) {
    DONG_PROFILE_FUNCTION();
    
    if (!node) return;

    const bool self_dirty = node->isStyleDirty();
    const bool subtree_dirty = node->isStyleSubtreeDirty();

    if (self_dirty) {
        recomputeNodeStyleFull(node);
    }

    // Only recurse into children if subtree has dirty nodes
    if (self_dirty || subtree_dirty) {
        for (const auto& child : node->getChildren()) {
            computeStylesIncremental(child);
        }
    }
}

void StyleEngine::recomputeNodeStyleFull(DOMNodePtr node) {
    applyDefaultStyleForNode(node);
    applyMatchingRulesIndexed(node);
    inheritFromParent(node);
    applyInlineStyleAttributeIfAny(node);

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

    node->getComputedStyle().layout_mode = deriveLayoutModeFromDisplay(node->getComputedStyle());
    processPseudoElements(node);
}

} // namespace dong::dom
