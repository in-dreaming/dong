#include "style_engine.hpp"
#include "style_engine_internal.hpp"
#include "../dom/dom_node.hpp"

namespace dong::dom {

namespace {

bool isRTL(const ComputedStyle& style) {
    return style.direction == CSSDirection::Rtl;
}

void applyLogicalMarginsAndPaddingsInternal(ComputedStyle& style) {
    const bool rtl = isRTL(style);
    if (style.margin_inline_start.isSet()) {
        (rtl ? style.margin_right : style.margin_left) = style.margin_inline_start;
    }
    if (style.margin_inline_end.isSet()) {
        (rtl ? style.margin_left : style.margin_right) = style.margin_inline_end;
    }
    if (style.padding_inline_start.isSet()) {
        (rtl ? style.padding_right : style.padding_left) = style.padding_inline_start;
    }
    if (style.padding_inline_end.isSet()) {
        (rtl ? style.padding_left : style.padding_right) = style.padding_inline_end;
    }

    if (style.margin_block_start.isSet()) style.margin_top = style.margin_block_start;
    if (style.margin_block_end.isSet()) style.margin_bottom = style.margin_block_end;
    if (style.padding_block_start.isSet()) style.padding_top = style.padding_block_start;
    if (style.padding_block_end.isSet()) style.padding_bottom = style.padding_block_end;
}

void applyLogicalBordersInternal(ComputedStyle& style) {
    const bool rtl = isRTL(style);

    auto apply_side = [&](bool to_right,
                          float width, const std::string& color, CSSBorderStyle st) {
        if (width >= 0.0f) {
            if (to_right) style.border_right_width = width;
            else style.border_left_width = width;
        }
        if (!color.empty()) {
            if (to_right) style.border_right_color = color;
            else style.border_left_color = color;
        }
        if (st != CSSBorderStyleUnset) {
            if (to_right) style.border_right_style = st;
            else style.border_left_style = st;
        }
    };

    if (style.border_inline_start_width >= 0.0f || !style.border_inline_start_color.empty() || style.border_inline_start_style != CSSBorderStyleUnset) {
        apply_side(rtl, style.border_inline_start_width, style.border_inline_start_color, style.border_inline_start_style);
    }
    if (style.border_inline_end_width >= 0.0f || !style.border_inline_end_color.empty() || style.border_inline_end_style != CSSBorderStyleUnset) {
        apply_side(!rtl, style.border_inline_end_width, style.border_inline_end_color, style.border_inline_end_style);
    }

    if (style.border_block_start_width >= 0.0f) style.border_top_width = style.border_block_start_width;
    if (!style.border_block_start_color.empty()) style.border_top_color = style.border_block_start_color;
    if (style.border_block_start_style != CSSBorderStyleUnset) style.border_top_style = style.border_block_start_style;

    if (style.border_block_end_width >= 0.0f) style.border_bottom_width = style.border_block_end_width;
    if (!style.border_block_end_color.empty()) style.border_bottom_color = style.border_block_end_color;
    if (style.border_block_end_style != CSSBorderStyleUnset) style.border_bottom_style = style.border_block_end_style;
}

} // anonymous namespace

namespace style_engine_internal {

void applyLogicalProperties(ComputedStyle& style) {
    applyLogicalMarginsAndPaddingsInternal(style);
    applyLogicalBordersInternal(style);
}

void resolveTextAlignForDirection(ComputedStyle& style) {
    const bool rtl = (style.direction == CSSDirection::Rtl);
    if (style.text_align == CSSTextAlign::Start) {
        style.text_align = rtl ? CSSTextAlign::Right : CSSTextAlign::Left;
    } else if (style.text_align == CSSTextAlign::End) {
        style.text_align = rtl ? CSSTextAlign::Left : CSSTextAlign::Right;
    } else if (!style.isExplicitlySet("text-align") && rtl) {
        style.text_align = CSSTextAlign::Right;
    }
}

} // namespace style_engine_internal

void StyleEngine::processGlobalKeywords(DOMNodePtr node, DOMNodePtr parent) {
    if (!node || !parent) return;

    auto& computed = node->getComputedStyle();
    const auto& parent_style = parent->getComputedStyle();

    // ??????initial/unset ???????????????????
    // ??????ComputedStyle ???????????????????????computed ???
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
        else if (prop == "tab-size") computed.tab_size = kInitial.tab_size;
        else if (prop == "quotes") {
            computed.quotes = kInitial.quotes;
            computed.has_quotes = kInitial.has_quotes;
            computed.quotes_auto = kInitial.quotes_auto;
        }


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
        "text-indent", "text-transform", "word-break", "overflow-wrap", "hyphens", "tab-size",

        "border-collapse", "border-spacing",
        "quotes",
        "color-scheme"

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
    else if (prop == "hyphens") child_style.hyphens = parent_style.hyphens;
    else if (prop == "tab-size") child_style.tab_size = parent_style.tab_size;

    else if (prop == "quotes") {
        child_style.quotes = parent_style.quotes;
        child_style.has_quotes = parent_style.has_quotes;
        child_style.quotes_auto = parent_style.quotes_auto;
    }
    else if (prop == "border-collapse") child_style.border_collapse = parent_style.border_collapse;

    else if (prop == "border-spacing") { child_style.border_spacing_x = parent_style.border_spacing_x; child_style.border_spacing_y = parent_style.border_spacing_y; }
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
    else if (prop == "accent-color") child_style.accent_color = parent_style.accent_color;
    else if (prop == "color-scheme") child_style.color_scheme = parent_style.color_scheme;
    else if (prop == "caption-side") child_style.caption_side = parent_style.caption_side;

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
    if (!computed.isExplicitlySet("font-weight")) {
        computed.font_weight = parent_style.font_weight;
    } else {
        // Resolve bolder/lighter relative to parent weight (CSS spec)
        if (computed.font_weight == CSSFontWeight::Bolder || computed.font_weight == CSSFontWeight::Lighter) {
            int parent_w = 400;
            switch (parent_style.font_weight) {
                case CSSFontWeight::W100: parent_w = 100; break;
                case CSSFontWeight::W200: parent_w = 200; break;
                case CSSFontWeight::W300: parent_w = 300; break;
                case CSSFontWeight::Normal: case CSSFontWeight::W400: parent_w = 400; break;
                case CSSFontWeight::W500: parent_w = 500; break;
                case CSSFontWeight::W600: parent_w = 600; break;
                case CSSFontWeight::Bold: case CSSFontWeight::W700: parent_w = 700; break;
                case CSSFontWeight::W800: parent_w = 800; break;
                case CSSFontWeight::W900: parent_w = 900; break;
                default: parent_w = 400; break;
            }
            int resolved_w;
            if (computed.font_weight == CSSFontWeight::Bolder) {
                if (parent_w < 350) resolved_w = 400;
                else if (parent_w < 550) resolved_w = 700;
                else resolved_w = 900;
            } else {
                if (parent_w < 550) resolved_w = 100;
                else if (parent_w < 750) resolved_w = 400;
                else resolved_w = 700;
            }
            computed.font_weight = fontWeightFromString(std::to_string(resolved_w));
        }
    }
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
    if (!computed.isExplicitlySet("hyphens")) computed.hyphens = parent_style.hyphens;
    if (!computed.isExplicitlySet("tab-size")) computed.tab_size = parent_style.tab_size;


    // List styling properties (inherited per CSS spec)
    if (!computed.isExplicitlySet("list-style-type")) computed.list_style_type = parent_style.list_style_type;
    if (!computed.isExplicitlySet("list-style-position")) computed.list_style_position = parent_style.list_style_position;

    // Table properties (inherited per CSS spec)
    if (!computed.isExplicitlySet("border-collapse")) computed.border_collapse = parent_style.border_collapse;
    if (!computed.isExplicitlySet("border-spacing")) { computed.border_spacing_x = parent_style.border_spacing_x; computed.border_spacing_y = parent_style.border_spacing_y; }

    // `color-scheme` is not strictly inheritable per spec, but it affects UA/form control
    // rendering for descendants. Treat it as inheritable so container schemes work.
    if (!computed.isExplicitlySet("color-scheme")) computed.color_scheme = parent_style.color_scheme;


    // Generated content properties
    // `quotes` is inheritable; counters are not.
    if (!computed.isExplicitlySet("quotes")) {
        computed.quotes = parent_style.quotes;
        computed.has_quotes = parent_style.has_quotes;
        computed.quotes_auto = parent_style.quotes_auto;
    }
}


} // namespace dong::dom