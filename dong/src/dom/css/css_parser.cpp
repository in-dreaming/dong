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

namespace {

// Forward declarations for helper functions used in handlers
inline float parseFontSizeHelper(const std::string& s);
inline float parseFloatHelper(const std::string& s);
inline void parseMarginShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parsePaddingShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBorderShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBorderRadiusShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseFlexShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBackgroundShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseFontShorthandHelper(const std::string& value, ComputedStyle& style);

// CSS counters / generated content helpers
inline std::vector<ComputedStyle::CounterDirective> parseCounterDirectiveList(const std::string& val,
                                                                            bool is_increment);
inline std::vector<std::string> parseQuotesList(const std::string& val);
inline std::vector<ComputedStyle::ContentToken> parseContentTokens(const std::string& val,
                                                                   std::string& out_literal_only);

// Logical property shorthand helpers forward declarations
inline void parseMarginInlineShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseMarginBlockShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parsePaddingInlineShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parsePaddingBlockShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBorderInlineShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBorderBlockShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseInsetInlineShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseInsetBlockShorthandHelper(const std::string& value, ComputedStyle& style);

// Place shorthand property helpers forward declarations
inline void parsePlaceItemsShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parsePlaceContentShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parsePlaceSelfShorthandHelper(const std::string& value, ComputedStyle& style);

// Property handler function type
using PropertyHandler = void(*)(const std::string& val, ComputedStyle& style);

// Build the property dispatch table (called once, returns static reference)
const std::unordered_map<std::string_view, PropertyHandler>& getPropertyHandlers() {
    static const std::unordered_map<std::string_view, PropertyHandler> handlers = {
        // Display & Position
        {"display", [](const std::string& val, ComputedStyle& style) { 
            style.display = val;
            style.layout_mode = deriveLayoutModeFromDisplay(val);
        }},
        {"position", [](const std::string& val, ComputedStyle& style) { style.position = val; }},
        
        // Box model - dimensions
        {"width", [](const std::string& val, ComputedStyle& style) { style.width = CSSParser::parseValue(val); }},
        {"height", [](const std::string& val, ComputedStyle& style) { style.height = CSSParser::parseValue(val); }},
        {"min-width", [](const std::string& val, ComputedStyle& style) { style.min_width = CSSParser::parseValue(val); }},
        {"max-width", [](const std::string& val, ComputedStyle& style) { style.max_width = CSSParser::parseValue(val); }},
        {"min-height", [](const std::string& val, ComputedStyle& style) { style.min_height = CSSParser::parseValue(val); }},
        {"max-height", [](const std::string& val, ComputedStyle& style) { style.max_height = CSSParser::parseValue(val); }},
        
        // Margin
        {"margin", [](const std::string& val, ComputedStyle& style) { parseMarginShorthandHelper(val, style); }},
        {"margin-top", [](const std::string& val, ComputedStyle& style) { style.margin_top = CSSParser::parseValue(val); }},
        {"margin-right", [](const std::string& val, ComputedStyle& style) { style.margin_right = CSSParser::parseValue(val); }},
        {"margin-bottom", [](const std::string& val, ComputedStyle& style) { style.margin_bottom = CSSParser::parseValue(val); }},
        {"margin-left", [](const std::string& val, ComputedStyle& style) { style.margin_left = CSSParser::parseValue(val); }},

        // Logical margins
        {"margin-inline-start", [](const std::string& val, ComputedStyle& style) { style.margin_inline_start = CSSParser::parseValue(val); }},
        {"margin-inline-end", [](const std::string& val, ComputedStyle& style) { style.margin_inline_end = CSSParser::parseValue(val); }},
        {"margin-block-start", [](const std::string& val, ComputedStyle& style) { style.margin_block_start = CSSParser::parseValue(val); }},
        {"margin-block-end", [](const std::string& val, ComputedStyle& style) { style.margin_block_end = CSSParser::parseValue(val); }},
        {"margin-inline", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);
            if (parts.size() == 1) {
                auto v = CSSParser::parseValue(parts[0]);
                style.margin_inline_start = v;
                style.margin_inline_end = v;
            } else if (parts.size() >= 2) {
                style.margin_inline_start = CSSParser::parseValue(parts[0]);
                style.margin_inline_end = CSSParser::parseValue(parts[1]);
            }
        }},
        {"margin-block", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);
            if (parts.size() == 1) {
                auto v = CSSParser::parseValue(parts[0]);
                style.margin_block_start = v;
                style.margin_block_end = v;
            } else if (parts.size() >= 2) {
                style.margin_block_start = CSSParser::parseValue(parts[0]);
                style.margin_block_end = CSSParser::parseValue(parts[1]);
            }
        }},
        
        // Padding

        {"padding", [](const std::string& val, ComputedStyle& style) { parsePaddingShorthandHelper(val, style); }},
        {"padding-top", [](const std::string& val, ComputedStyle& style) { style.padding_top = CSSParser::parseValue(val); }},
        {"padding-right", [](const std::string& val, ComputedStyle& style) { style.padding_right = CSSParser::parseValue(val); }},
        {"padding-bottom", [](const std::string& val, ComputedStyle& style) { style.padding_bottom = CSSParser::parseValue(val); }},
        {"padding-left", [](const std::string& val, ComputedStyle& style) { style.padding_left = CSSParser::parseValue(val); }},

        // Logical paddings
        {"padding-inline-start", [](const std::string& val, ComputedStyle& style) { style.padding_inline_start = CSSParser::parseValue(val); }},
        {"padding-inline-end", [](const std::string& val, ComputedStyle& style) { style.padding_inline_end = CSSParser::parseValue(val); }},
        {"padding-block-start", [](const std::string& val, ComputedStyle& style) { style.padding_block_start = CSSParser::parseValue(val); }},
        {"padding-block-end", [](const std::string& val, ComputedStyle& style) { style.padding_block_end = CSSParser::parseValue(val); }},
        {"padding-inline", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);
            if (parts.size() == 1) {
                auto v = CSSParser::parseValue(parts[0]);
                style.padding_inline_start = v;
                style.padding_inline_end = v;
            } else if (parts.size() >= 2) {
                style.padding_inline_start = CSSParser::parseValue(parts[0]);
                style.padding_inline_end = CSSParser::parseValue(parts[1]);
            }
        }},
        {"padding-block", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);
            if (parts.size() == 1) {
                auto v = CSSParser::parseValue(parts[0]);
                style.padding_block_start = v;
                style.padding_block_end = v;
            } else if (parts.size() >= 2) {
                style.padding_block_start = CSSParser::parseValue(parts[0]);
                style.padding_block_end = CSSParser::parseValue(parts[1]);
            }
        }},
        
        // Position offsets

        {"top", [](const std::string& val, ComputedStyle& style) { style.top = CSSParser::parseValue(val); }},
        {"right", [](const std::string& val, ComputedStyle& style) { style.right = CSSParser::parseValue(val); }},
        {"bottom", [](const std::string& val, ComputedStyle& style) { style.bottom = CSSParser::parseValue(val); }},
        {"left", [](const std::string& val, ComputedStyle& style) { style.left = CSSParser::parseValue(val); }},
        {"inset", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);
            if (parts.size() == 1) {
                auto v = CSSParser::parseValue(parts[0]);
                style.top = style.right = style.bottom = style.left = v;
            } else if (parts.size() == 2) {
                style.top = style.bottom = CSSParser::parseValue(parts[0]);
                style.left = style.right = CSSParser::parseValue(parts[1]);
            } else if (parts.size() == 3) {
                style.top = CSSParser::parseValue(parts[0]);
                style.left = style.right = CSSParser::parseValue(parts[1]);
                style.bottom = CSSParser::parseValue(parts[2]);
            } else if (parts.size() >= 4) {
                style.top = CSSParser::parseValue(parts[0]);
                style.right = CSSParser::parseValue(parts[1]);
                style.bottom = CSSParser::parseValue(parts[2]);
                style.left = CSSParser::parseValue(parts[3]);
            }
        }},
        {"z-index", [](const std::string& val, ComputedStyle& style) {
            if (val == "auto") {
                style.z_index = std::nullopt;  // auto = no stacking context
            } else {
                try {
                    style.z_index = std::stoi(val);  // numeric value = create stacking context
                } catch (...) {
                    style.z_index = std::nullopt;
                }
            }
        }},
        
        // Visual - colors & backgrounds
        {"color", [](const std::string& val, ComputedStyle& style) { style.color = CSSParser::parseColor(val); }},
        {"background-color", [](const std::string& val, ComputedStyle& style) { style.background_color = CSSParser::parseColor(val); }},
        {"background-image", [](const std::string& val, ComputedStyle& style) {
            if (val.find("gradient") != std::string::npos) {
                style.background_gradients.push_back(CSSParser::parseGradient(val));
            } else {
                style.background_image = val;
            }
        }},
        {"background", [](const std::string& val, ComputedStyle& style) { parseBackgroundShorthandHelper(val, style); }},
        {"background-size", [](const std::string& val, ComputedStyle& style) { style.background_size = val; }},
        {"background-repeat", [](const std::string& val, ComputedStyle& style) { style.background_repeat = val; }},
        {"background-position", [](const std::string& val, ComputedStyle& style) { style.background_position = CSSParser::parseBackgroundPosition(val); }},
        {"background-attachment", [](const std::string& val, ComputedStyle& style) { style.background_attachment = val; }},
        {"background-clip", [](const std::string& val, ComputedStyle& style) { style.background_clip = val; }},
        {"background-origin", [](const std::string& val, ComputedStyle& style) { style.background_origin = val; }},
        {"object-fit", [](const std::string& val, ComputedStyle& style) { style.object_fit = val; }},
        {"object-position", [](const std::string& val, ComputedStyle& style) { style.object_position = val; }},
        {"image-rendering", [](const std::string& val, ComputedStyle& style) { style.image_rendering = val; }},


        {"opacity", [](const std::string& val, ComputedStyle& style) {
            float v = parseFloatHelper(val);
            // Support percentage values (e.g., "50%" -> 0.5)
            std::string trimmed = val;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
            size_t last = trimmed.find_last_not_of(" \t\n\r");
            if (last != std::string::npos) trimmed = trimmed.substr(0, last + 1);
            if (!trimmed.empty() && trimmed.back() == '%') {
                v = v / 100.0f;
            }
            style.opacity = std::max(0.0f, std::min(1.0f, v));
        }},

        
        // Border (shorthand)
        {"border", [](const std::string& val, ComputedStyle& style) { parseBorderShorthandHelper(val, style); }},
        {"border-width", [](const std::string& val, ComputedStyle& style) { style.border_width = parseFloatHelper(val); }},
        {"border-color", [](const std::string& val, ComputedStyle& style) { style.border_color = CSSParser::parseColor(val); }},
        {"border-style", [](const std::string& val, ComputedStyle& style) { style.border_style = val; }},

        // Border (per-side shorthands)
        {"border-top", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_top_width = tmp.border_width;
            style.border_top_color = tmp.border_color;
            style.border_top_style = tmp.border_style;
        }},
        {"border-right", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_right_width = tmp.border_width;
            style.border_right_color = tmp.border_color;
            style.border_right_style = tmp.border_style;
        }},
        {"border-bottom", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_bottom_width = tmp.border_width;
            style.border_bottom_color = tmp.border_color;
            style.border_bottom_style = tmp.border_style;
        }},
        {"border-left", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_left_width = tmp.border_width;
            style.border_left_color = tmp.border_color;
            style.border_left_style = tmp.border_style;
        }},

        // Logical border shorthands
        {"border-inline", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_inline_start_width = tmp.border_width;
            style.border_inline_end_width = tmp.border_width;
            style.border_inline_start_color = tmp.border_color;
            style.border_inline_end_color = tmp.border_color;
            style.border_inline_start_style = tmp.border_style;
            style.border_inline_end_style = tmp.border_style;
        }},
        {"border-block", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_block_start_width = tmp.border_width;
            style.border_block_end_width = tmp.border_width;
            style.border_block_start_color = tmp.border_color;
            style.border_block_end_color = tmp.border_color;
            style.border_block_start_style = tmp.border_style;
            style.border_block_end_style = tmp.border_style;
        }},
        {"border-inline-start", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_inline_start_width = tmp.border_width;
            style.border_inline_start_color = tmp.border_color;
            style.border_inline_start_style = tmp.border_style;
        }},
        {"border-inline-end", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_inline_end_width = tmp.border_width;
            style.border_inline_end_color = tmp.border_color;
            style.border_inline_end_style = tmp.border_style;
        }},
        {"border-block-start", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_block_start_width = tmp.border_width;
            style.border_block_start_color = tmp.border_color;
            style.border_block_start_style = tmp.border_style;
        }},
        {"border-block-end", [](const std::string& val, ComputedStyle& style) {
            ComputedStyle tmp;
            parseBorderShorthandHelper(val, tmp);
            style.border_block_end_width = tmp.border_width;
            style.border_block_end_color = tmp.border_color;
            style.border_block_end_style = tmp.border_style;
        }},

        // Logical border components
        {"border-inline-start-width", [](const std::string& val, ComputedStyle& style) { style.border_inline_start_width = parseFloatHelper(val); }},
        {"border-inline-end-width", [](const std::string& val, ComputedStyle& style) { style.border_inline_end_width = parseFloatHelper(val); }},
        {"border-block-start-width", [](const std::string& val, ComputedStyle& style) { style.border_block_start_width = parseFloatHelper(val); }},
        {"border-block-end-width", [](const std::string& val, ComputedStyle& style) { style.border_block_end_width = parseFloatHelper(val); }},
        {"border-inline-start-color", [](const std::string& val, ComputedStyle& style) { style.border_inline_start_color = CSSParser::parseColor(val); }},
        {"border-inline-end-color", [](const std::string& val, ComputedStyle& style) { style.border_inline_end_color = CSSParser::parseColor(val); }},
        {"border-block-start-color", [](const std::string& val, ComputedStyle& style) { style.border_block_start_color = CSSParser::parseColor(val); }},
        {"border-block-end-color", [](const std::string& val, ComputedStyle& style) { style.border_block_end_color = CSSParser::parseColor(val); }},
        {"border-inline-start-style", [](const std::string& val, ComputedStyle& style) { style.border_inline_start_style = val; }},
        {"border-inline-end-style", [](const std::string& val, ComputedStyle& style) { style.border_inline_end_style = val; }},
        {"border-block-start-style", [](const std::string& val, ComputedStyle& style) { style.border_block_start_style = val; }},
        {"border-block-end-style", [](const std::string& val, ComputedStyle& style) { style.border_block_end_style = val; }},

        // Border per-side components

        {"border-top-width", [](const std::string& val, ComputedStyle& style) { style.border_top_width = parseFloatHelper(val); }},
        {"border-right-width", [](const std::string& val, ComputedStyle& style) { style.border_right_width = parseFloatHelper(val); }},
        {"border-bottom-width", [](const std::string& val, ComputedStyle& style) { style.border_bottom_width = parseFloatHelper(val); }},
        {"border-left-width", [](const std::string& val, ComputedStyle& style) { style.border_left_width = parseFloatHelper(val); }},
        {"border-top-color", [](const std::string& val, ComputedStyle& style) { style.border_top_color = CSSParser::parseColor(val); }},
        {"border-right-color", [](const std::string& val, ComputedStyle& style) { style.border_right_color = CSSParser::parseColor(val); }},
        {"border-bottom-color", [](const std::string& val, ComputedStyle& style) { style.border_bottom_color = CSSParser::parseColor(val); }},
        {"border-left-color", [](const std::string& val, ComputedStyle& style) { style.border_left_color = CSSParser::parseColor(val); }},
        {"border-top-style", [](const std::string& val, ComputedStyle& style) { style.border_top_style = val; }},
        {"border-right-style", [](const std::string& val, ComputedStyle& style) { style.border_right_style = val; }},
        {"border-bottom-style", [](const std::string& val, ComputedStyle& style) { style.border_bottom_style = val; }},
        {"border-left-style", [](const std::string& val, ComputedStyle& style) { style.border_left_style = val; }},

        // Border radius
        {"border-radius", [](const std::string& val, ComputedStyle& style) { parseBorderRadiusShorthandHelper(val, style); }},
        {"border-top-left-radius", [](const std::string& val, ComputedStyle& style) { style.border_top_left_radius = CSSParser::parseValue(val); }},
        {"border-top-right-radius", [](const std::string& val, ComputedStyle& style) { style.border_top_right_radius = CSSParser::parseValue(val); }},
        {"border-bottom-left-radius", [](const std::string& val, ComputedStyle& style) { style.border_bottom_left_radius = CSSParser::parseValue(val); }},
        {"border-bottom-right-radius", [](const std::string& val, ComputedStyle& style) { style.border_bottom_right_radius = CSSParser::parseValue(val); }},

        
        // Outline
        {"outline", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::string part;
            while (iss >> part) {
                if (std::isdigit(static_cast<unsigned char>(part[0])) || part[0] == '.') {
                    style.outline_width = parseFloatHelper(part);
                } else if (part == "solid" || part == "dashed" || part == "dotted" || part == "none") {
                    style.outline_style = part;
                } else {
                    style.outline_color = CSSParser::parseColor(part);
                }
            }
        }},
        {"outline-width", [](const std::string& val, ComputedStyle& style) { style.outline_width = parseFloatHelper(val); }},
        {"outline-color", [](const std::string& val, ComputedStyle& style) { style.outline_color = CSSParser::parseColor(val); }},
        {"outline-style", [](const std::string& val, ComputedStyle& style) { style.outline_style = val; }},
        {"outline-offset", [](const std::string& val, ComputedStyle& style) { style.outline_offset = parseFloatHelper(val); }},
        
        // Overflow & visibility
        {"overflow", [](const std::string& val, ComputedStyle& style) {
            style.overflow = val;
            style.overflow_x = val;
            style.overflow_y = val;
        }},
        {"overflow-x", [](const std::string& val, ComputedStyle& style) { style.overflow_x = val; }},
        {"overflow-y", [](const std::string& val, ComputedStyle& style) { style.overflow_y = val; }},

        // Scroll behavior
        {"overscroll-behavior", [](const std::string& val, ComputedStyle& style) {
            style.overscroll_behavior = val;
            style.overscroll_behavior_x = val;
            style.overscroll_behavior_y = val;
        }},
        {"overscroll-behavior-x", [](const std::string& val, ComputedStyle& style) { style.overscroll_behavior_x = val; }},
        {"overscroll-behavior-y", [](const std::string& val, ComputedStyle& style) { style.overscroll_behavior_y = val; }},
        {"scroll-behavior", [](const std::string& val, ComputedStyle& style) { style.scroll_behavior = val; }},

        {"visibility", [](const std::string& val, ComputedStyle& style) { style.visibility = val; }},
        {"cursor", [](const std::string& val, ComputedStyle& style) { style.cursor = val; }},
        {"box-sizing", [](const std::string& val, ComputedStyle& style) { style.box_sizing = val; }},
        {"aspect-ratio", [](const std::string& val, ComputedStyle& style) {
            if (val == "auto") {
                style.aspect_ratio = 0.0f;
            } else {
                auto pos = val.find('/');
                if (pos != std::string::npos) {
                    float width = std::stof(val.substr(0, pos));
                    float height = std::stof(val.substr(pos + 1));
                    style.aspect_ratio = width / height;
                } else {
                    style.aspect_ratio = std::stof(val);
                }
            }
        }},

        // Box shadow & filters
        {"box-shadow", [](const std::string& val, ComputedStyle& style) { CSSParser::parseBoxShadow(val, style); }},
        {"filter", [](const std::string& val, ComputedStyle& style) { style.filters = CSSParser::parseFilter(val); }},
        {"backdrop-filter", [](const std::string& val, ComputedStyle& style) { style.backdrop_filters = CSSParser::parseFilter(val); }},
        {"mix-blend-mode", [](const std::string& val, ComputedStyle& style) { style.mix_blend_mode = val; }},
        {"background-blend-mode", [](const std::string& val, ComputedStyle& style) { style.background_blend_mode = val; }},
        
        // Text & font
        {"font-family", [](const std::string& val, ComputedStyle& style) { style.font_family = val; }},
        {"font-size", [](const std::string& val, ComputedStyle& style) { style.font_size = parseFontSizeHelper(val); }},
        {"font-weight", [](const std::string& val, ComputedStyle& style) { style.font_weight = val; }},
        {"font-style", [](const std::string& val, ComputedStyle& style) { style.font_style = val; }},
        {"font-variant", [](const std::string& val, ComputedStyle& style) { style.font_variant = val; }},
        {"font", [](const std::string& val, ComputedStyle& style) { parseFontShorthandHelper(val, style); }},
        {"text-align", [](const std::string& val, ComputedStyle& style) { style.text_align = val; }},
        {"text-align-last", [](const std::string& val, ComputedStyle& style) { style.text_align_last = val; }},
        {"text-decoration", [](const std::string& val, ComputedStyle& style) { style.text_decoration = val; }},
        {"text-decoration-line", [](const std::string& val, ComputedStyle& style) { style.text_decoration = val; }},
        {"text-decoration-color", [](const std::string& val, ComputedStyle& style) { style.text_decoration_color = CSSParser::parseColor(val); }},
        {"text-decoration-style", [](const std::string& val, ComputedStyle& style) { style.text_decoration_style = val; }},
        {"text-decoration-thickness", [](const std::string& val, ComputedStyle& style) { style.text_decoration_thickness = parseFloatHelper(val); }},
        {"letter-spacing", [](const std::string& val, ComputedStyle& style) {
            if (val == "normal") {
                style.letter_spacing_em = 0.0f;
            } else {
                float px = parseFloatHelper(val);
                if (val.find("em") != std::string::npos) {
                    style.letter_spacing_em = px;
                } else {
                    float font_px = style.font_size > 0.0f ? style.font_size : 16.0f;
                    style.letter_spacing_em = px / font_px;
                }
            }
        }},
        {"word-spacing", [](const std::string& val, ComputedStyle& style) {
            if (val == "normal") {
                style.word_spacing_px = 0.0f;
            } else {
                float px = parseFloatHelper(val);
                if (val.find("em") != std::string::npos) {
                    float font_px = style.font_size > 0.0f ? style.font_size : 16.0f;
                    style.word_spacing_px = px * font_px;
                } else {
                    style.word_spacing_px = px;
                }
            }
        }},
        {"line-height", [](const std::string& val, ComputedStyle& style) {
            style.has_line_height = true;
            if (val == "normal") {
                style.line_height = -1.0f;
                style.line_height_is_unitless = true;
            } else if (val.find("px") != std::string::npos) {
                style.line_height = parseFloatHelper(val);
                style.line_height_is_unitless = false;
            } else if (val.find('%') != std::string::npos) {
                style.line_height = parseFloatHelper(val) / 100.0f;
                style.line_height_is_unitless = true;
            } else {
                style.line_height = parseFloatHelper(val);
                style.line_height_is_unitless = true;
            }
        }},
        {"text-transform", [](const std::string& val, ComputedStyle& style) { style.text_transform = val; }},
        {"text-overflow", [](const std::string& val, ComputedStyle& style) { style.text_overflow = val; }},
        {"white-space", [](const std::string& val, ComputedStyle& style) { style.white_space = val; }},
        {"word-break", [](const std::string& val, ComputedStyle& style) { style.word_break = val; }},
        {"overflow-wrap", [](const std::string& val, ComputedStyle& style) { style.overflow_wrap = val; }},
        {"word-wrap", [](const std::string& val, ComputedStyle& style) { style.overflow_wrap = val; }},
        {"hyphens", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (v == "none" || v == "manual" || v == "auto") {
                style.hyphens = v;
            }
        }},

        {"vertical-align", [](const std::string& val, ComputedStyle& style) { style.vertical_align = val; }},
        {"direction", [](const std::string& val, ComputedStyle& style) { style.direction = val; }},
        {"unicode-bidi", [](const std::string& val, ComputedStyle& style) { style.unicode_bidi = val; }},
        {"text-indent", [](const std::string& val, ComputedStyle& style) { style.text_indent = parseFloatHelper(val); }},
        {"tab-size", [](const std::string& val, ComputedStyle& style) {
            int tab_val = static_cast<int>(parseFloatHelper(val));
            if (tab_val > 0) style.tab_size = tab_val;
        }},
        {"-webkit-line-clamp", [](const std::string& val, ComputedStyle& style) { style.webkit_line_clamp = static_cast<int>(parseFloatHelper(val)); }},
        {"text-shadow", [](const std::string& val, ComputedStyle& style) { CSSParser::parseTextShadow(val, style); }},
        
        // Flexbox
        {"flex-direction", [](const std::string& val, ComputedStyle& style) { style.flex_direction = val; }},
        {"flex-wrap", [](const std::string& val, ComputedStyle& style) { style.flex_wrap = val; }},
        {"flex-flow", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::string part;
            while (iss >> part) {
                if (part == "row" || part == "column" || part == "row-reverse" || part == "column-reverse") {
                    style.flex_direction = part;
                } else if (part == "wrap" || part == "nowrap" || part == "wrap-reverse") {
                    style.flex_wrap = part;
                }
            }
        }},
        {"justify-content", [](const std::string& val, ComputedStyle& style) { style.justify_content = val; }},
        {"justify-items", [](const std::string& val, ComputedStyle& style) { style.justify_items = val; }},
        {"justify-self", [](const std::string& val, ComputedStyle& style) { style.justify_self = val; }},
        {"align-items", [](const std::string& val, ComputedStyle& style) { style.align_items = val; }},
        {"align-content", [](const std::string& val, ComputedStyle& style) { style.align_content = val; }},
        {"align-self", [](const std::string& val, ComputedStyle& style) { style.align_self = val; }},
        {"place-items", [](const std::string& val, ComputedStyle& style) { parsePlaceItemsShorthandHelper(val, style); }},
        {"place-content", [](const std::string& val, ComputedStyle& style) { parsePlaceContentShorthandHelper(val, style); }},
        {"place-self", [](const std::string& val, ComputedStyle& style) { parsePlaceSelfShorthandHelper(val, style); }},
        {"flex", [](const std::string& val, ComputedStyle& style) { parseFlexShorthandHelper(val, style); }},
        {"flex-grow", [](const std::string& val, ComputedStyle& style) { style.flex_grow = parseFloatHelper(val); }},
        {"flex-shrink", [](const std::string& val, ComputedStyle& style) { style.flex_shrink = parseFloatHelper(val); }},
        {"flex-basis", [](const std::string& val, ComputedStyle& style) { style.flex_basis = CSSParser::parseValue(val); }},
        {"order", [](const std::string& val, ComputedStyle& style) { style.order = static_cast<int>(parseFloatHelper(val)); }},
        {"gap", [](const std::string& val, ComputedStyle& style) {
            // Support dual-value syntax: gap: row-gap column-gap
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);

            if (parts.size() == 1) {
                // Single value: applies to both row and column
                float v = parseFloatHelper(parts[0]);
                style.gap = v;
                style.row_gap = v;
                style.column_gap = v;
            } else if (parts.size() >= 2) {
                // Dual value: first is row-gap, second is column-gap
                style.row_gap = parseFloatHelper(parts[0]);
                style.column_gap = parseFloatHelper(parts[1]);
                style.gap = style.row_gap; // Set gap to row-gap for backwards compatibility
            } else {
                style.gap = 0.0f;
                style.row_gap = 0.0f;
                style.column_gap = 0.0f;
            }
        }},
        {"row-gap", [](const std::string& val, ComputedStyle& style) { style.row_gap = parseFloatHelper(val); }},
        {"column-gap", [](const std::string& val, ComputedStyle& style) { style.column_gap = parseFloatHelper(val); }},

        // List styling
        {"list-style-type", [](const std::string& val, ComputedStyle& style) {
            style.list_style_type = val;
        }},
        {"list-style-position", [](const std::string& val, ComputedStyle& style) {
            style.list_style_position = val;
        }},
        {"list-style", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::string part;

            // Reset all list-style properties first
            style.list_style_type = "none";
            style.list_style_position = "outside";
            style.list_style_image = "none";

            while (iss >> part) {
                if (part == "inside" || part == "outside") {
                    style.list_style_position = part;
                } else if (part == "none") {
                    // "none" sets both type and image to none
                    style.list_style_type = "none";
                    style.list_style_image = "none";
                } else if (part.find("url(") == 0) {
                    // list-style-image (url(...))
                    style.list_style_image = part;
                } else {
                    // list-style-type (disc, circle, square, decimal, etc.)
                    style.list_style_type = part;
                }
            }
        }},

        // Form control theming
        {"accent-color", [](const std::string& val, ComputedStyle& style) {
            std::string lowered = val;
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (lowered == "auto") {
                style.accent_color = "auto";
            } else {
                style.accent_color = CSSParser::parseColor(val);
            }



        }},

        // Appearance
        {"appearance", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            style.appearance = v;
        }},
        {"-webkit-appearance", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            style.appearance = v;
        }},
        {"resize", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (v == "none" || v == "both" || v == "horizontal" || v == "vertical") {
                style.resize = v;
            }
        }},
        {"will-change", [](const std::string& val, ComputedStyle& style) {
            style.will_change = trimWhitespace(val);
            if (style.will_change.empty()) {
                style.will_change = "auto";
            }
        }},


        // Table properties
        {"border-collapse", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            style.border_collapse = v;
        }},

        // Color scheme (UA/form control theming)
        {"color-scheme", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            // Accept the subset used by tests.
            if (v.find("dark") != std::string::npos) {
                style.color_scheme = "dark";
            } else if (v.find("light") != std::string::npos) {
                style.color_scheme = "light";
            } else {
                style.color_scheme = "normal";
            }
        }},

        {"border-spacing", [](const std::string& val, ComputedStyle& style) {
            // border-spacing: <length> | <length> <length>
            // Single value: both horizontal and vertical
            // Two values: horizontal, vertical
            std::string v = trimWhitespace(val);
            size_t space_pos = v.find(' ');
            if (space_pos != std::string::npos) {
                float h = parseFloatHelper(v.substr(0, space_pos));
                float vertical = parseFloatHelper(v.substr(space_pos + 1));
                style.border_spacing_x = h;
                style.border_spacing_y = vertical;
            } else {
                style.border_spacing_x = parseFloatHelper(v);
                style.border_spacing_y = style.border_spacing_x;
            }
        }},
        {"table-layout", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            style.table_layout = v;
        }},
        {"caption-side", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (v == "top" || v == "bottom") {
                style.caption_side = v;
            }
        }},

        // Transform

        {"transform", [](const std::string& val, ComputedStyle& style) { CSSParser::parseTransform(val, style); }},
        {"transform-origin", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);
            
            auto parseOrigin = [](const std::string& v) -> float {
                std::string lowered = v;
                std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                if (lowered == "left" || lowered == "top") return 0.0f;
                if (lowered == "center") return 50.0f;
                if (lowered == "right" || lowered == "bottom") return 100.0f;
                if (lowered.find('%') != std::string::npos) return parseFloatHelper(lowered);
                return parseFloatHelper(lowered) * 0.5f;
            };
            
            if (!parts.empty()) {
                style.transform_origin_x = parseOrigin(parts[0]);
                style.transform_origin_y = parts.size() > 1 ? parseOrigin(parts[1]) : style.transform_origin_x;
            }
        }},
        {"transform-style", [](const std::string& val, ComputedStyle& style) { style.transform_style = val; }},
        {"perspective", [](const std::string& val, ComputedStyle& style) { style.perspective = parseFloatHelper(val); }},
        {"perspective-origin", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<std::string> parts;
            std::string part;
            while (iss >> part) parts.push_back(part);
            if (!parts.empty()) {
                style.perspective_origin_x = parseFloatHelper(parts[0]);
                style.perspective_origin_y = parts.size() > 1 ? parseFloatHelper(parts[1]) : style.perspective_origin_x;
            }
        }},
        {"backface-visibility", [](const std::string& val, ComputedStyle& style) { style.backface_visibility = val; }},
        
        // Transitions
        {"transition", [](const std::string& val, ComputedStyle& style) { style.transitions = CSSParser::parseTransition(val); }},
        {"transition-property", [](const std::string& val, ComputedStyle& style) {
            if (style.transitions.empty()) style.transitions.push_back(CSSTransition());
            style.transitions[0].property = val;
        }},
        {"transition-duration", [](const std::string& val, ComputedStyle& style) {
            if (style.transitions.empty()) style.transitions.push_back(CSSTransition());
            float time = parseFloatHelper(val);
            if (val.find("ms") != std::string::npos) time /= 1000.0f;
            style.transitions[0].duration = time;
        }},
        {"transition-timing-function", [](const std::string& val, ComputedStyle& style) {
            if (style.transitions.empty()) style.transitions.push_back(CSSTransition());
            style.transitions[0].timing_function = val;
        }},
        {"transition-delay", [](const std::string& val, ComputedStyle& style) {
            if (style.transitions.empty()) style.transitions.push_back(CSSTransition());
            float time = parseFloatHelper(val);
            if (val.find("ms") != std::string::npos) time /= 1000.0f;
            style.transitions[0].delay = time;
        }},
        
        // Animations
        {"animation", [](const std::string& val, ComputedStyle& style) { style.animations.push_back(CSSParser::parseAnimation(val)); }},
        {"animation-name", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            style.animations[0].name = val;
        }},
        {"animation-duration", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            float time = parseFloatHelper(val);
            if (val.find("ms") != std::string::npos) time /= 1000.0f;
            style.animations[0].duration = time;
        }},
        {"animation-timing-function", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            style.animations[0].timing_function = val;
        }},
        {"animation-delay", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            float time = parseFloatHelper(val);
            if (val.find("ms") != std::string::npos) time /= 1000.0f;
            style.animations[0].delay = time;
        }},
        {"animation-iteration-count", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            style.animations[0].iteration_count = (val == "infinite") ? -1 : static_cast<int>(parseFloatHelper(val));
        }},
        {"animation-direction", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            style.animations[0].direction = val;
        }},
        {"animation-fill-mode", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            style.animations[0].fill_mode = val;
        }},
        {"animation-play-state", [](const std::string& val, ComputedStyle& style) {
            if (style.animations.empty()) style.animations.push_back(CSSAnimation());
            style.animations[0].play_state = val;
        }},
        
        // Misc
        {"clip-path", [](const std::string& val, ComputedStyle& style) { style.clip_path = val; }},
        {"pointer-events", [](const std::string& val, ComputedStyle& style) { style.pointer_events = val; }},
        {"user-select", [](const std::string& val, ComputedStyle& style) { style.user_select = val; }},
        {"touch-action", [](const std::string& val, ComputedStyle& style) { style.touch_action = val; }},
        {"caret-color", [](const std::string& val, ComputedStyle& style) { style.caret_color = CSSParser::parseColor(val); }},
        {"float", [](const std::string& val, ComputedStyle& style) { style.float_value = val; }},
        {"clear", [](const std::string& val, ComputedStyle& style) { style.clear = val; }},
        {"isolation", [](const std::string& val, ComputedStyle& style) { style.isolation_isolate = (val == "isolate"); }},

        // Counters / generated content
        {"counter-reset", [](const std::string& val, ComputedStyle& style) {
            style.counter_resets = parseCounterDirectiveList(val, false);
        }},
        {"counter-increment", [](const std::string& val, ComputedStyle& style) {
            style.counter_increments = parseCounterDirectiveList(val, true);
        }},
        {"quotes", [](const std::string& val, ComputedStyle& style) {
            style.has_quotes = true;
            std::string lower_val = val;
            std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower_val == "auto") {
                style.quotes_auto = true;
                // Empty vector as placeholder - will be resolved later
                style.quotes.clear();
            } else {
                style.quotes_auto = false;
                style.quotes = parseQuotesList(val);
            }
        }},
        {"content", [](const std::string& val, ComputedStyle& style) {
            style.content_raw = val;
            style.content_tokens.clear();
            style.content.clear();

            if (val == "none" || val == "normal") {
                style.content_raw.clear();
                return;
            }

            std::string literal_only;
            style.content_tokens = parseContentTokens(val, literal_only);
            style.content = literal_only;
        }},
    };
    return handlers;
}

// Helper: parseFloat wrapper for use in lambdas
inline float parseFontSizeHelper(const std::string& s) {
    // Trim and convert to lowercase for keyword comparison
    std::string v = s;
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos && last + 1 < v.size()) {
        v.erase(last + 1);
    }

    std::string lower = v;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Handle font-size absolute keywords (CSS spec)
    if (lower == "xx-small") return 10.0f;
    if (lower == "x-small") return 12.0f;
    if (lower == "small") return 14.0f;
    if (lower == "medium") return 16.0f;  // Default font size
    if (lower == "large") return 18.0f;
    if (lower == "x-large") return 24.0f;
    if (lower == "xx-large") return 32.0f;

    // For relative keywords (smaller/larger), we need parent context
    // For now, return a reasonable default - this should be handled in style engine
    // TODO: Implement smaller/larger in style_engine.cpp using parent font-size
    if (lower == "smaller") return 14.0f;  // Approximate smaller
    if (lower == "larger") return 18.0f;   // Approximate larger

    return CSSParser::parseFloat(s);
}

inline float parseFloatHelper(const std::string& s) {
    // Trim and convert to lowercase for keyword comparison
    std::string v = s;
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos && last + 1 < v.size()) {
        v.erase(last + 1);
    }

    std::string lower = v;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Handle border-width keywords (CSS spec: thin=1px, medium=3px, thick=5px)
    if (lower == "thin") return 1.0f;
    if (lower == "medium") return 3.0f;
    if (lower == "thick") return 5.0f;

    return CSSParser::parseFloat(s);
}

// ============================================================================
// Counters / generated content helpers
// ============================================================================

static void skipWs(std::string_view sv, size_t& i) {
    while (i < sv.size() && std::isspace(static_cast<unsigned char>(sv[i]))) {
        ++i;
    }
}

static bool consumeChar(std::string_view sv, size_t& i, char c) {
    skipWs(sv, i);
    if (i < sv.size() && sv[i] == c) {
        ++i;
        return true;
    }
    return false;
}

static std::string parseIdentToken(std::string_view sv, size_t& i) {
    skipWs(sv, i);
    size_t start = i;
    while (i < sv.size()) {
        char c = sv[i];
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ) {
            ++i;
            continue;
        }
        break;
    }
    if (i <= start) {
        return {};
    }
    return std::string(sv.substr(start, i - start));
}

static bool parseSignedIntToken(std::string_view sv, size_t& i, int& out) {
    skipWs(sv, i);
    if (i >= sv.size()) return false;

    size_t start = i;
    if (sv[i] == '+' || sv[i] == '-') {
        ++i;
    }
    size_t digits_start = i;
    while (i < sv.size() && std::isdigit(static_cast<unsigned char>(sv[i]))) {
        ++i;
    }
    if (i == digits_start) {
        i = start;
        return false;
    }

    try {
        out = std::stoi(std::string(sv.substr(start, i - start)));
        return true;
    } catch (...) {
        i = start;
        return false;
    }
}

static std::string parseQuotedStringToken(std::string_view sv, size_t& i, bool& ok) {
    skipWs(sv, i);
    ok = false;
    if (i >= sv.size()) return {};

    char quote = sv[i];
    if (quote != '\'' && quote != '"') {
        return {};
    }
    ++i;

    auto isHex = [](char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    };
    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    };
    auto appendUtf8 = [](std::string& dst, uint32_t cp) {
        if (cp <= 0x7Fu) {
            dst.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FFu) {
            dst.push_back(static_cast<char>(0xC0u | ((cp >> 6) & 0x1Fu)));
            dst.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
        } else if (cp <= 0xFFFFu) {
            dst.push_back(static_cast<char>(0xE0u | ((cp >> 12) & 0x0Fu)));
            dst.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
            dst.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
        } else {
            dst.push_back(static_cast<char>(0xF0u | ((cp >> 18) & 0x07u)));
            dst.push_back(static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu)));
            dst.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
            dst.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
        }
    };

    std::string out;
    while (i < sv.size()) {
        char c = sv[i++];
        if (c == quote) {
            ok = true;
            return out;
        }
        if (c != '\\' || i >= sv.size()) {
            out.push_back(c);
            continue;
        }

        // CSS-style escapes inside strings.
        char n = sv[i++];

        // Line continuation: "\\\n" or "\\\r\n".
        if (n == '\n') {
            continue;
        }
        if (n == '\r') {
            if (i < sv.size() && sv[i] == '\n') ++i;
            continue;
        }



        // Standard CSS hex escape: \HHHHHH (1-6 hex digits) with optional trailing whitespace.
        if (isHex(n)) {
            uint32_t cp = static_cast<uint32_t>(hexVal(n));
            int digits = 1;
            while (i < sv.size() && digits < 6 && isHex(sv[i])) {
                cp = (cp << 4) | static_cast<uint32_t>(hexVal(sv[i++]));
                ++digits;
            }
            // Optional whitespace after hex escape.
            if (i < sv.size() && std::isspace(static_cast<unsigned char>(sv[i]))) {
                ++i;
            }
            appendUtf8(out, cp);
            continue;
        }

        // Simple escapes.
        switch (n) {
            case 'n': out.push_back('\n'); break;
            case 't': out.push_back('\t'); break;
            case 'r': out.push_back('\r'); break;
            case '\\': out.push_back('\\'); break;
            case '\'': out.push_back('\''); break;
            case '"': out.push_back('"'); break;
            default: out.push_back(n); break;
        }
    }
    return {};
}


static std::string toLowerAscii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

inline std::vector<ComputedStyle::CounterDirective> parseCounterDirectiveList(const std::string& val,
                                                                            bool is_increment) {
    std::vector<ComputedStyle::CounterDirective> out;
    std::string_view sv(val);
    size_t i = 0;
    skipWs(sv, i);

    if (toLowerAscii(std::string(sv.substr(i))) == "none") {
        return out;
    }

    while (i < sv.size()) {
        std::string name = parseIdentToken(sv, i);
        if (name.empty()) {
            break;
        }

        ComputedStyle::CounterDirective d;
        d.name = name;
        d.value = is_increment ? 1 : 0;
        d.has_value = false;

        int num = 0;
        size_t before_num = i;
        if (parseSignedIntToken(sv, i, num)) {
            d.value = num;
            d.has_value = true;
        } else {
            i = before_num;
        }

        out.push_back(std::move(d));
        skipWs(sv, i);
    }

    return out;
}

// Helper function to get language-specific quotes
// Returns quotes for given language code (e.g., "en", "fr", "de")
// If language not found, returns default English quotes
inline std::vector<std::string> getQuotesForLanguage(const std::string& lang) {
    std::string lower_lang = trimWhitespace(lang);
    std::transform(lower_lang.begin(), lower_lang.end(), lower_lang.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    // Language-specific quote pairs
    static const std::unordered_map<std::string, std::vector<std::string>> kLanguageQuotes = {
        // Default/English
        {"", {"\u201C", "\u201D", "\u2018", "\u2019"}},
        {"en", {"\u201C", "\u201D", "\u2018", "\u2019"}},

        // French
        {"fr", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},

        // German
        {"de", {"\u201E", "\u201C", "\u201A", "\u2018"}},

        // Spanish/Catalan
        {"es", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},
        {"ca", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},

        // Italian
        {"it", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},

        // Russian/Ukrainian
        {"ru", {"\u00AB", "\u00BB", "\u201E", "\u201C"}},
        {"uk", {"\u00AB", "\u00BB", "\u201E", "\u201C"}},

        // Chinese (simplified and traditional both use corner brackets)
        {"zh", {"\u300C", "\u300D", "\u300E", "\u300F"}},

        // Japanese (same as Chinese)
        {"ja", {"\u300C", "\u300D", "\u300E", "\u300F"}},

        // Korean (also uses corner brackets)
        {"ko", {"\u300C", "\u300D", "\u300E", "\u300F"}},

        // Greek
        {"el", {"\u00AB", "\u00BB", "\u2018", "\u2019"}},

        // Hungarian
        {"hu", {"\u201E", "\u201D", "\u00BB", "\u00AB"}},

        // Polish
        {"pl", {"\u201E", "\u201D", "\u00AB", "\u00BB"}},

        // Czech
        {"cs", {"\u201E", "\u201C", "\u201A", "\u2018"}},

        // Slovak
        {"sk", {"\u201E", "\u201C", "\u201A", "\u2018"}},

        // Swedish
        {"sv", {"\u201D", "\u201D", "\u2019", "\u2019"}},

        // Finnish
        {"fi", {"\u201D", "\u201D", "\u2019", "\u2019"}},

        // Danish (same as Swedish/Finnish)
        {"da", {"\u201D", "\u201D", "\u2019", "\u2019"}},

        // Norwegian
        {"no", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},
        {"nb", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},  // Norwegian Bokmål
        {"nn", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},  // Norwegian Nynorsk
    };

    // Look for exact language match
    auto it = kLanguageQuotes.find(lower_lang);
    if (it != kLanguageQuotes.end()) {
        return it->second;
    }

    // Check for primary language code (before any hyphen, e.g., "en-US" -> "en")
    size_t hyphen_pos = lower_lang.find('-');
    if (hyphen_pos != std::string::npos) {
        std::string primary = lower_lang.substr(0, hyphen_pos);
        it = kLanguageQuotes.find(primary);
        if (it != kLanguageQuotes.end()) {
            return it->second;
        }
    }

    // Default to English quotes
    return kLanguageQuotes.at("");
}

inline std::vector<std::string> parseQuotesList(const std::string& val) {
    std::string_view sv(val);
    size_t i = 0;
    skipWs(sv, i);

    // Check for "auto" or "none"
    std::string lower = toLowerAscii(std::string(sv.substr(i)));

    if (lower == "none") {
        return {};  // Empty vector means no quotes
    }

    if (lower == "auto") {
        // Return empty vector with special marker to resolve at rendering time
        // We use a single empty string as a marker
        std::vector<std::string> marker;
        marker.push_back("");  // Empty string indicates "auto"
        return marker;
    }

    // Parse custom quote pairs
    std::vector<std::string> out;
    while (i < sv.size()) {
        bool ok = false;
        std::string s = parseQuotedStringToken(sv, i, ok);
        if (!ok) {
            break;
        }
        out.push_back(std::move(s));
        skipWs(sv, i);
    }

    // Must be pairs.
    if (out.size() % 2 == 1) {
        out.pop_back();
    }
    return out;
}

static void consumeUntilMatchingParen(std::string_view sv, size_t& i) {
    int depth = 0;
    while (i < sv.size()) {
        char c = sv[i++];
        if (c == '(') ++depth;
        else if (c == ')') {
            if (depth == 0) return;
            --depth;
        } else if (c == '\'' || c == '"') {
            // Skip strings inside args.
            --i;
            bool ok = false;
            (void)parseQuotedStringToken(sv, i, ok);
        }
    }
}

inline std::vector<ComputedStyle::ContentToken> parseContentTokens(const std::string& val,
                                                                   std::string& out_literal_only) {
    std::vector<ComputedStyle::ContentToken> tokens;

    std::string_view sv(val);
    size_t i = 0;
    bool literal_only = true;
    std::string literal_acc;

    while (i < sv.size()) {
        skipWs(sv, i);
        if (i >= sv.size()) break;

        // String literal
        if (sv[i] == '\'' || sv[i] == '"') {
            bool ok = false;
            std::string s = parseQuotedStringToken(sv, i, ok);
            if (!ok) break;

            ComputedStyle::ContentToken t;
            t.type = ComputedStyle::ContentToken::Type::String;
            t.text = s;
            tokens.push_back(std::move(t));
            literal_acc += s;
            continue;
        }

        // Identifier / function
        std::string name = parseIdentToken(sv, i);
        if (name.empty()) {
            break;
        }
        std::string lower = toLowerAscii(name);

        // Keywords
        if (lower == "open-quote" || lower == "close-quote" ||
            lower == "no-open-quote" || lower == "no-close-quote") {
            ComputedStyle::ContentToken t;
            if (lower == "open-quote") t.type = ComputedStyle::ContentToken::Type::OpenQuote;
            else if (lower == "close-quote") t.type = ComputedStyle::ContentToken::Type::CloseQuote;
            else if (lower == "no-open-quote") t.type = ComputedStyle::ContentToken::Type::NoOpenQuote;
            else t.type = ComputedStyle::ContentToken::Type::NoCloseQuote;
            tokens.push_back(std::move(t));
            literal_only = false;
            continue;
        }

        // counter(...) / counters(...)
        if (consumeChar(sv, i, '(')) {
            if (lower == "counter" || lower == "counters") {
                ComputedStyle::ContentToken t;
                t.type = (lower == "counter")
                    ? ComputedStyle::ContentToken::Type::Counter
                    : ComputedStyle::ContentToken::Type::Counters;

                // name
                std::string counter_name = parseIdentToken(sv, i);
                if (counter_name.empty()) {
                    // Try quoted name.
                    bool ok = false;
                    counter_name = parseQuotedStringToken(sv, i, ok);
                }
                t.text = counter_name;

                if (t.type == ComputedStyle::ContentToken::Type::Counters) {
                    // separator (optional)
                    if (consumeChar(sv, i, ',')) {
                        bool ok = false;
                        std::string sep = parseQuotedStringToken(sv, i, ok);
                        if (ok) {
                            t.separator = sep;
                        }
                    }
                    if (t.separator.empty()) {
                        t.separator = ".";
                    }
                }

                // Ignore rest of args / style param
                consumeUntilMatchingParen(sv, i);

                tokens.push_back(std::move(t));
                literal_only = false;
                continue;
            }

            // Unknown function: skip args.
            consumeUntilMatchingParen(sv, i);
            literal_only = false;
            continue;
        }

        // Unknown identifier token: treat as non-literal, ignore.
        literal_only = false;
    }

    out_literal_only = literal_only ? literal_acc : std::string();
    return tokens;
}

// Helper wrappers that call CSSParser static methods (for use in dispatch table)
inline void parseMarginShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.margin_top = style.margin_right = style.margin_bottom = style.margin_left = v;
    } else if (parts.size() == 2) {
        style.margin_top = style.margin_bottom = CSSParser::parseValue(parts[0]);
        style.margin_left = style.margin_right = CSSParser::parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.margin_top = CSSParser::parseValue(parts[0]);
        style.margin_left = style.margin_right = CSSParser::parseValue(parts[1]);
        style.margin_bottom = CSSParser::parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.margin_top = CSSParser::parseValue(parts[0]);
        style.margin_right = CSSParser::parseValue(parts[1]);
        style.margin_bottom = CSSParser::parseValue(parts[2]);
        style.margin_left = CSSParser::parseValue(parts[3]);
    }
}

inline void parsePaddingShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.padding_top = style.padding_right = style.padding_bottom = style.padding_left = v;
    } else if (parts.size() == 2) {
        style.padding_top = style.padding_bottom = CSSParser::parseValue(parts[0]);
        style.padding_left = style.padding_right = CSSParser::parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.padding_top = CSSParser::parseValue(parts[0]);
        style.padding_left = style.padding_right = CSSParser::parseValue(parts[1]);
        style.padding_bottom = CSSParser::parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.padding_top = CSSParser::parseValue(parts[0]);
        style.padding_right = CSSParser::parseValue(parts[1]);
        style.padding_bottom = CSSParser::parseValue(parts[2]);
        style.padding_left = CSSParser::parseValue(parts[3]);
    }
}

inline void parseBorderShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;
    
    for (size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        if (c == '(') { ++paren_depth; current += c; }
        else if (c == ')') { --paren_depth; current += c; }
        else if (c == ' ' && paren_depth == 0) {
            if (!current.empty()) { parts.push_back(current); current.clear(); }
        } else { current += c; }
    }
    if (!current.empty()) parts.push_back(current);
    
    for (const auto& part : parts) {
        if (part.empty()) continue;
        if (std::isdigit(static_cast<unsigned char>(part[0])) || part[0] == '.') {
            style.border_width = parseFloatHelper(part);
        } else if (part == "solid" || part == "dashed" || part == "dotted" || 
                   part == "double" || part == "none" || part == "hidden") {
            style.border_style = part;
        } else {
            style.border_color = CSSParser::parseColor(part);
        }
    }

    // Shorthand `border` sets all sides.
    style.border_top_width = style.border_width;
    style.border_right_width = style.border_width;
    style.border_bottom_width = style.border_width;
    style.border_left_width = style.border_width;
    style.border_top_color = style.border_color;
    style.border_right_color = style.border_color;
    style.border_bottom_color = style.border_color;
    style.border_left_color = style.border_color;
    style.border_top_style = style.border_style;
    style.border_right_style = style.border_style;
    style.border_bottom_style = style.border_style;
    style.border_left_style = style.border_style;
}


inline void parseBorderRadiusShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        CSSValue v = CSSParser::parseValue(parts[0]);
        // Still set the legacy float for backwards compatibility
        style.border_radius = (v.unit == CSSValue::Unit::PIXEL) ? v.value : v.value;
        style.border_top_left_radius = v;
        style.border_top_right_radius = v;
        style.border_bottom_left_radius = v;
        style.border_bottom_right_radius = v;
    } else if (parts.size() == 2) {
        style.border_top_left_radius = style.border_bottom_right_radius = CSSParser::parseValue(parts[0]);
        style.border_top_right_radius = style.border_bottom_left_radius = CSSParser::parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.border_top_left_radius = CSSParser::parseValue(parts[0]);
        style.border_top_right_radius = style.border_bottom_left_radius = CSSParser::parseValue(parts[1]);
        style.border_bottom_right_radius = CSSParser::parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.border_top_left_radius = CSSParser::parseValue(parts[0]);
        style.border_top_right_radius = CSSParser::parseValue(parts[1]);
        style.border_bottom_right_radius = CSSParser::parseValue(parts[2]);
        style.border_bottom_left_radius = CSSParser::parseValue(parts[3]);
    }
}

inline void parseFlexShorthandHelper(const std::string& value, ComputedStyle& style) {
    if (value == "none") {
        style.flex_grow = 0.0f;
        style.flex_shrink = 0.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);
        return;
    }
    if (value == "auto") {
        style.flex_grow = 1.0f;
        style.flex_shrink = 1.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);
        return;
    }
    
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        style.flex_grow = parseFloatHelper(parts[0]);
        style.flex_shrink = 1.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::PIXEL);
    } else if (parts.size() == 2) {
        style.flex_grow = parseFloatHelper(parts[0]);
        if (parts[1].find('%') != std::string::npos || parts[1].find("px") != std::string::npos) {
            style.flex_basis = CSSParser::parseValue(parts[1]);
        } else {
            style.flex_shrink = parseFloatHelper(parts[1]);
        }
    } else if (parts.size() >= 3) {
        style.flex_grow = parseFloatHelper(parts[0]);
        style.flex_shrink = parseFloatHelper(parts[1]);
        style.flex_basis = CSSParser::parseValue(parts[2]);
    }
}

inline void parseBackgroundShorthandHelper(const std::string& value, ComputedStyle& style) {
    // Handle gradients
    if (value.find("gradient") != std::string::npos) {
        style.background_gradients.push_back(CSSParser::parseGradient(value));
        return;
    }
    
    // Split by spaces but respect parentheses
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;
    
    for (char c : value) {
        if (c == '(') { ++paren_depth; current += c; }
        else if (c == ')') { --paren_depth; current += c; }
        else if (c == ' ' && paren_depth == 0) {
            if (!current.empty()) { parts.push_back(current); current.clear(); }
        } else { current += c; }
    }
    if (!current.empty()) parts.push_back(current);
    
    for (const auto& part : parts) {
        if (part.empty()) continue;
        if (part.find("url(") != std::string::npos) {
            style.background_image = part;
        } else if (part == "repeat" || part == "no-repeat" || part == "repeat-x" || part == "repeat-y") {
            style.background_repeat = part;
        } else if (part == "cover" || part == "contain" || part.find('%') != std::string::npos) {
            style.background_size = part;
        } else {
            style.background_color = CSSParser::parseColor(part);
        }
    }
}

inline void parseFontShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::string part;
    bool found_size = false;
    
    while (iss >> part) {
        if (part == "italic" || part == "oblique") {
            style.font_style = part;
        } else if (part == "bold" || part == "bolder" || part == "lighter" ||
                   part == "100" || part == "200" || part == "300" || part == "400" ||
                   part == "500" || part == "600" || part == "700" || part == "800" || part == "900") {
            style.font_weight = part;
        } else if (part == "small-caps") {
            style.font_variant = part;
        } else if (!found_size) {
            // Try to parse as font-size (handles both numbers and keywords)
            // Check for line-height (e.g., "16px/1.5" or "medium/1.5")
            size_t slash = part.find('/');
            if (slash != std::string::npos) {
                style.font_size = parseFontSizeHelper(part.substr(0, slash));
                std::string lh = part.substr(slash + 1);
                style.line_height = parseFloatHelper(lh);
                style.has_line_height = true;
                style.line_height_is_unitless = (lh.find("px") == std::string::npos);
                found_size = true;
            } else {
                // Check if this looks like a font-size (number or keyword)
                std::string lower = part;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                bool is_size_keyword = (lower == "xx-small" || lower == "x-small" ||
                                       lower == "small" || lower == "medium" ||
                                       lower == "large" || lower == "x-large" ||
                                       lower == "xx-large" || lower == "smaller" ||
                                       lower == "larger");
                if (std::isdigit(static_cast<unsigned char>(part[0])) || part[0] == '.' || is_size_keyword) {
                    style.font_size = parseFontSizeHelper(part);
                    found_size = true;
                }
            }
        } else if (found_size) {
            // Everything after size is font-family
            std::string family = part;
            while (iss >> part) { family += " " + part; }
            // Remove quotes
            if (family.size() >= 2 && 
                ((family.front() == '"' && family.back() == '"') ||
                 (family.front() == '\'' && family.back() == '\''))) {
                family = family.substr(1, family.size() - 2);
            }
            style.font_family = family;
            break;
        }
    }
}

// Logical property shorthand helpers implementation
inline void parseMarginInlineShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.margin_left = style.margin_right = v;
    } else if (parts.size() >= 2) {
        style.margin_left = CSSParser::parseValue(parts[0]);
        style.margin_right = CSSParser::parseValue(parts[1]);
    }
}

inline void parseMarginBlockShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.margin_top = style.margin_bottom = v;
    } else if (parts.size() >= 2) {
        style.margin_top = CSSParser::parseValue(parts[0]);
        style.margin_bottom = CSSParser::parseValue(parts[1]);
    }
}

inline void parsePaddingInlineShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.padding_left = style.padding_right = v;
    } else if (parts.size() >= 2) {
        style.padding_left = CSSParser::parseValue(parts[0]);
        style.padding_right = CSSParser::parseValue(parts[1]);
    }
}

inline void parsePaddingBlockShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.padding_top = style.padding_bottom = v;
    } else if (parts.size() >= 2) {
        style.padding_top = CSSParser::parseValue(parts[0]);
        style.padding_bottom = CSSParser::parseValue(parts[1]);
    }
}

inline void parseBorderInlineShorthandHelper(const std::string& value, ComputedStyle& style) {
    ComputedStyle tmp;
    parseBorderShorthandHelper(value, tmp);
    style.border_left_width = tmp.border_width;
    style.border_left_color = tmp.border_color;
    style.border_left_style = tmp.border_style;
    style.border_right_width = tmp.border_width;
    style.border_right_color = tmp.border_color;
    style.border_right_style = tmp.border_style;
}

inline void parseBorderBlockShorthandHelper(const std::string& value, ComputedStyle& style) {
    ComputedStyle tmp;
    parseBorderShorthandHelper(value, tmp);
    style.border_top_width = tmp.border_width;
    style.border_top_color = tmp.border_color;
    style.border_top_style = tmp.border_style;
    style.border_bottom_width = tmp.border_width;
    style.border_bottom_color = tmp.border_color;
    style.border_bottom_style = tmp.border_style;
}

inline void parseInsetInlineShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.left = style.right = v;
    } else if (parts.size() >= 2) {
        style.left = CSSParser::parseValue(parts[0]);
        style.right = CSSParser::parseValue(parts[1]);
    }
}

inline void parseInsetBlockShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        auto v = CSSParser::parseValue(parts[0]);
        style.top = style.bottom = v;
    } else if (parts.size() >= 2) {
        style.top = CSSParser::parseValue(parts[0]);
        style.bottom = CSSParser::parseValue(parts[1]);
    }
}

// Place shorthand property implementations
inline void parsePlaceItemsShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        // Single value: applies to both align-items and justify-items
        style.align_items = parts[0];
        style.justify_items = parts[0];
    } else if (parts.size() == 2) {
        // Two values: first is align-items, second is justify-items
        style.align_items = parts[0];
        style.justify_items = parts[1];
    }
    // Invalid number of values: ignore
}

inline void parsePlaceContentShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        // Single value: applies to both align-content and justify-content
        style.align_content = parts[0];
        style.justify_content = parts[0];
    } else if (parts.size() == 2) {
        // Two values: first is align-content, second is justify-content
        style.align_content = parts[0];
        style.justify_content = parts[1];
    }
    // Invalid number of values: ignore
}

inline void parsePlaceSelfShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        // Single value: applies to both align-self and justify-self
        style.align_self = parts[0];
        style.justify_self = parts[0];
    } else if (parts.size() == 2) {
        // Two values: first is align-self, second is justify-self
        style.align_self = parts[0];
        style.justify_self = parts[1];
    }
    // Invalid number of values: ignore
}

} // anonymous namespace

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

float CSSParser::parseFloat(const std::string& s) {
    std::string v = s;
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos && last + 1 < v.size()) {
        v.erase(last + 1);
    }
    
    size_t i = 0;
    while (i < v.size() && (std::isdigit(static_cast<unsigned char>(v[i])) || 
           v[i] == '-' || v[i] == '+' || v[i] == '.')) {
        ++i;
    }
    if (i == 0) return 0.0f;
    try {
        return std::stof(v.substr(0, i));
    } catch (...) {
        return 0.0f;
    }
}

CSSValue CSSParser::parseValue(const std::string& value) {
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) v = v.substr(0, last + 1);

    if (v == "auto") {
        return CSSValue(0.0f, CSSValue::Unit::AUTO);
    }
    if (v == "inherit") {
        return CSSValue(0.0f, CSSValue::Unit::INHERIT);
    }
    if (v == "content") {
        return CSSValue(0.0f, CSSValue::Unit::CONTENT);
    }
    
    // Check for calc()
    if (v.find("calc(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string expr = v.substr(5, end - 5);
            auto calc_expr = parseCalc(expr);
            if (calc_expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = calc_expr;
                return result;
            }
        }
    }
    
    // Check for min()
    if (v.find("min(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(4, end - 4);
            auto expr = parseMinMaxClamp("min", args);
            if (expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = expr;
                return result;
            }
        }
    }
    
    // Check for max()
    if (v.find("max(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(4, end - 4);
            auto expr = parseMinMaxClamp("max", args);
            if (expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = expr;
                return result;
            }
        }
    }
    
    // Check for clamp()
    if (v.find("clamp(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(6, end - 6);
            auto expr = parseMinMaxClamp("clamp", args);
            if (expr) {
                CSSValue result;
                result.unit = CSSValue::Unit::CALC;
                result.calc_expr = expr;
                return result;
            }
        }
    }

    // Check for env()
    if (v.find("env(") == 0) {
        size_t end = v.rfind(')');
        if (end != std::string::npos) {
            std::string args = v.substr(4, end - 4);

            // Parse env() arguments: env(name) or env(name, fallback)
            size_t comma_pos = args.find(',');
            std::string env_name;
            std::string fallback_value;

            if (comma_pos != std::string::npos) {
                // env(name, fallback)
                env_name = args.substr(0, comma_pos);
                fallback_value = args.substr(comma_pos + 1);

                // Trim whitespace
                env_name.erase(0, env_name.find_first_not_of(" \t\n\r"));
                env_name.erase(env_name.find_last_not_of(" \t\n\r") + 1);
                fallback_value.erase(0, fallback_value.find_first_not_of(" \t\n\r"));
                fallback_value.erase(fallback_value.find_last_not_of(" \t\n\r") + 1);
            } else {
                // env(name)
                env_name = args;
                env_name.erase(0, env_name.find_first_not_of(" \t\n\r"));
                env_name.erase(env_name.find_last_not_of(" \t\n\r") + 1);
            }

            // Create ENV value
            CSSValue result;
            result.unit = CSSValue::Unit::ENV;
            result.env_name = env_name;

            // Parse fallback value if provided
            if (!fallback_value.empty()) {
                result.env_fallback = std::make_shared<CSSValue>(parseValue(fallback_value));
            }

            return result;
        }
    }

    float num = parseFloat(v);
    
    if (v.find('%') != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::PERCENT);
    }
    if (v.find("rem") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::REM);
    }
    if (v.find("em") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::EM);
    }
    if (v.find("vw") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VW);
    }
    if (v.find("vh") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VH);
    }
    if (v.find("vmin") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VMIN);
    }
    if (v.find("vmax") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::VMAX);
    }
    if (v.find("ch") != std::string::npos) {
        return CSSValue(num, CSSValue::Unit::CH);
    }
    
    return CSSValue(num, CSSValue::Unit::PIXEL);
}

// Parse calc() expression
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalc(const std::string& expr) {
    size_t pos = 0;
    return parseCalcExpression(expr, pos);
}

// Parse min()/max()/clamp() expressions
std::shared_ptr<CSSCalcExpression> CSSParser::parseMinMaxClamp(const std::string& func, const std::string& args) {
    auto result = std::make_shared<CSSCalcExpression>();
    
    if (func == "min") {
        result->op = CSSCalcExpression::Op::MIN;
    } else if (func == "max") {
        result->op = CSSCalcExpression::Op::MAX;
    } else if (func == "clamp") {
        result->op = CSSCalcExpression::Op::CLAMP;
    } else {
        return nullptr;
    }
    
    // Split arguments by comma (respecting parentheses)
    std::vector<std::string> parts;
    std::string current;
    int depth = 0;
    
    for (char c : args) {
        if (c == '(') {
            ++depth;
            current += c;
        } else if (c == ')') {
            --depth;
            current += c;
        } else if (c == ',' && depth == 0) {
            // Trim and add
            size_t start = current.find_first_not_of(" \t\n\r");
            size_t end = current.find_last_not_of(" \t\n\r");
            if (start != std::string::npos) {
                parts.push_back(current.substr(start, end - start + 1));
            }
            current.clear();
        } else {
            current += c;
        }
    }
    
    // Add last part
    size_t start = current.find_first_not_of(" \t\n\r");
    size_t end = current.find_last_not_of(" \t\n\r");
    if (start != std::string::npos) {
        parts.push_back(current.substr(start, end - start + 1));
    }
    
    // Parse each argument
    for (const auto& part : parts) {
        // Check if it's a nested calc/min/max/clamp
        CSSValue val = parseValue(part);
        
        auto child = std::make_shared<CSSCalcExpression>();
        if (val.isCalc() && val.calc_expr) {
            child = val.calc_expr;
        } else {
            child->op = CSSCalcExpression::Op::VALUE;
            child->value = val;
        }
        result->children.push_back(child);
    }
    
    return result;
}

// Parse calc expression (handles + and -)
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalcExpression(const std::string& expr, size_t& pos) {
    auto left = parseCalcTerm(expr, pos);
    if (!left) return nullptr;
    
    while (pos < expr.size()) {
        // Skip whitespace
        while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos]))) ++pos;
        
        if (pos >= expr.size()) break;
        
        char op = expr[pos];
        if (op != '+' && op != '-') break;
        
        // Check for whitespace around operator (required by CSS calc spec)
        if (pos > 0 && !std::isspace(static_cast<unsigned char>(expr[pos - 1]))) break;
        if (pos + 1 < expr.size() && !std::isspace(static_cast<unsigned char>(expr[pos + 1]))) break;
        
        ++pos;
        
        auto right = parseCalcTerm(expr, pos);
        if (!right) break;
        
        auto combined = std::make_shared<CSSCalcExpression>();
        combined->op = (op == '+') ? CSSCalcExpression::Op::ADD : CSSCalcExpression::Op::SUBTRACT;
        combined->children.push_back(left);
        combined->children.push_back(right);
        left = combined;
    }
    
    return left;
}

// Parse calc term (handles * and /)
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalcTerm(const std::string& expr, size_t& pos) {
    auto left = parseCalcFactor(expr, pos);
    if (!left) return nullptr;
    
    while (pos < expr.size()) {
        // Skip whitespace
        while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos]))) ++pos;
        
        if (pos >= expr.size()) break;
        
        char op = expr[pos];
        if (op != '*' && op != '/') break;
        
        ++pos;
        
        auto right = parseCalcFactor(expr, pos);
        if (!right) break;
        
        auto combined = std::make_shared<CSSCalcExpression>();
        combined->op = (op == '*') ? CSSCalcExpression::Op::MULTIPLY : CSSCalcExpression::Op::DIVIDE;
        combined->children.push_back(left);
        combined->children.push_back(right);
        left = combined;
    }
    
    return left;
}

// Parse calc factor (value or parenthesized expression)
std::shared_ptr<CSSCalcExpression> CSSParser::parseCalcFactor(const std::string& expr, size_t& pos) {
    // Skip whitespace
    while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos]))) ++pos;
    
    if (pos >= expr.size()) return nullptr;
    
    // Check for parenthesized expression
    if (expr[pos] == '(') {
        ++pos;
        auto inner = parseCalcExpression(expr, pos);
        // Skip closing paren
        while (pos < expr.size() && expr[pos] != ')') ++pos;
        if (pos < expr.size()) ++pos;
        return inner;
    }
    
    // Parse value with unit
    size_t start = pos;
    bool has_digit = false;
    
    // Handle negative sign
    if (pos < expr.size() && expr[pos] == '-') ++pos;
    
    // Parse number
    while (pos < expr.size() && (std::isdigit(static_cast<unsigned char>(expr[pos])) || expr[pos] == '.')) {
        has_digit = true;
        ++pos;
    }
    
    if (!has_digit) return nullptr;
    
    // Parse unit
    size_t unit_start = pos;
    while (pos < expr.size() && std::isalpha(static_cast<unsigned char>(expr[pos]))) ++pos;
    if (pos < expr.size() && expr[pos] == '%') ++pos;
    
    std::string value_str = expr.substr(start, pos - start);
    CSSValue val = parseValue(value_str);
    
    auto result = std::make_shared<CSSCalcExpression>();
    result->op = CSSCalcExpression::Op::VALUE;
    result->value = val;
    return result;
}

std::string CSSParser::parseColor(const std::string& value) {
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    v.erase(0, v.find_first_not_of(" \t\n\r"));
    size_t last = v.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) v = v.substr(0, last + 1);
    
    // Check for light-dark() function
    if (v.find("light-dark(") == 0) {
        // Parse light-dark(light-color, dark-color) function
        size_t start = v.find('(');
        size_t end = v.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string args = v.substr(start + 1, end - start - 1);

            // Split arguments by comma (handling nested parentheses)
            std::vector<std::string> parts;
            std::string current;
            int paren_depth = 0;

            for (char c : args) {
                if (c == '(') paren_depth++;
                else if (c == ')') paren_depth--;

                if (c == ',' && paren_depth == 0) {
                    std::string trimmed = current;
                    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
                    size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
                    if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
                    if (!trimmed.empty()) parts.push_back(trimmed);
                    current.clear();
                } else {
                    current += c;
                }
            }

            if (!current.empty()) {
                std::string trimmed = current;
                trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
                size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
                if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
                if (!trimmed.empty()) parts.push_back(trimmed);
            }

            // Should have exactly 2 arguments: light-color and dark-color
            if (parts.size() == 2) {
                // Return the light-dark function as-is for later resolution
                return v;
            }
        }
    }

    // Already a valid color format (including oklab/oklch)
    if (v[0] == '#' || v.find("rgb") == 0 || v.find("hsl") == 0 || v.find("color-mix") == 0 ||
        v.find("oklab") == 0 || v.find("oklch") == 0) {
        return v;
    }
    
    // Named colors (common ones)
    static const std::unordered_map<std::string, std::string> named_colors = {
        {"transparent", "transparent"},
        {"black", "#000000"},
        {"white", "#ffffff"},
        {"red", "#ff0000"},
        {"green", "#008000"},
        {"blue", "#0000ff"},
        {"yellow", "#ffff00"},
        {"cyan", "#00ffff"},
        {"magenta", "#ff00ff"},
        {"gray", "#808080"},
        {"grey", "#808080"},
        {"silver", "#c0c0c0"},
        {"maroon", "#800000"},
        {"olive", "#808000"},
        {"lime", "#00ff00"},
        {"aqua", "#00ffff"},
        {"teal", "#008080"},
        {"navy", "#000080"},
        {"fuchsia", "#ff00ff"},
        {"purple", "#800080"},
        {"orange", "#ffa500"},
        {"pink", "#ffc0cb"},
        {"brown", "#a52a2a"},
        {"coral", "#ff7f50"},
        {"crimson", "#dc143c"},
        {"gold", "#ffd700"},
        {"indigo", "#4b0082"},
        {"ivory", "#fffff0"},
        {"khaki", "#f0e68c"},
        {"lavender", "#e6e6fa"},
        {"lightblue", "#add8e6"},
        {"lightgray", "#d3d3d3"},
        {"lightgreen", "#90ee90"},
        {"lightyellow", "#ffffe0"},
        {"darkblue", "#00008b"},
        {"darkgray", "#a9a9a9"},
        {"darkgreen", "#006400"},
        {"darkred", "#8b0000"},
    };
    
    auto it = named_colors.find(v);
    if (it != named_colors.end()) {
        return it->second;
    }
    
    return value;
}

CSSGradient CSSParser::parseGradient(const std::string& value) {
    CSSGradient gradient;
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    if (v.find("repeating-linear-gradient") == 0) {
        gradient.type = CSSGradient::Type::REPEATING_LINEAR;
    } else if (v.find("repeating-radial-gradient") == 0) {
        gradient.type = CSSGradient::Type::REPEATING_RADIAL;
    } else if (v.find("linear-gradient") == 0) {
        gradient.type = CSSGradient::Type::LINEAR;
    } else if (v.find("radial-gradient") == 0) {
        gradient.type = CSSGradient::Type::RADIAL;
    } else if (v.find("conic-gradient") == 0) {
        gradient.type = CSSGradient::Type::CONIC;
    }
    
    // Extract content inside parentheses
    size_t start = v.find('(');
    size_t end = v.rfind(')');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return gradient;
    }
    
    std::string content = v.substr(start + 1, end - start - 1);
    
    // Parse gradient stops and direction
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;
    
    for (char c : content) {
        if (c == '(') {
            ++paren_depth;
            current += c;
        } else if (c == ')') {
            --paren_depth;
            current += c;
        } else if (c == ',' && paren_depth == 0) {
            std::string trimmed = current;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
            size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
            if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
            if (!trimmed.empty()) {
                parts.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        std::string trimmed = current;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        size_t last_pos = trimmed.find_last_not_of(" \t\n\r");
        if (last_pos != std::string::npos) trimmed = trimmed.substr(0, last_pos + 1);
        if (!trimmed.empty()) {
            parts.push_back(trimmed);
        }
    }
    
    // Parse direction/angle (first part if it's not a color)
    size_t color_start = 0;
    if (!parts.empty()) {
        const std::string& first = parts[0];
        if (first.find("deg") != std::string::npos) {
            gradient.angle = parseFloat(first);
            color_start = 1;
        } else if (first.find("to ") == 0) {
            // Parse direction keywords
            if (first.find("right") != std::string::npos) gradient.angle = 90.0f;
            else if (first.find("left") != std::string::npos) gradient.angle = 270.0f;
            else if (first.find("bottom") != std::string::npos) gradient.angle = 180.0f;
            else if (first.find("top") != std::string::npos) gradient.angle = 0.0f;
            color_start = 1;
        }
    }
    
    // Parse color stops
    float auto_position = 0.0f;
    float auto_step = parts.size() > color_start + 1 ? 
                      1.0f / (parts.size() - color_start - 1) : 1.0f;
    
    for (size_t i = color_start; i < parts.size(); ++i) {
        GradientStop stop;
        const std::string& part = parts[i];
        
        // Check for position
        size_t percent_pos = part.rfind('%');
        if (percent_pos != std::string::npos) {
            // Find where position starts
            size_t pos_start = part.rfind(' ', percent_pos);
            if (pos_start != std::string::npos) {
                stop.color = parseColor(part.substr(0, pos_start));
                stop.position = parseFloat(part.substr(pos_start)) / 100.0f;
            } else {
                stop.position = parseFloat(part) / 100.0f;
            }
        } else {
            stop.color = parseColor(part);
            stop.position = auto_position;
            auto_position += auto_step;
        }
        
        if (stop.color.empty()) {
            stop.color = part;
        }
        
        gradient.stops.push_back(stop);
    }
    
    return gradient;
}

std::vector<CSSFilter> CSSParser::parseFilter(const std::string& value) {
    std::vector<CSSFilter> filters;
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    if (v == "none") {
        return filters;
    }
    
    size_t pos = 0;
    while (pos < v.size()) {
        size_t lparen = v.find('(', pos);
        if (lparen == std::string::npos) break;
        size_t rparen = v.find(')', lparen + 1);
        if (rparen == std::string::npos) break;
        
        std::string func_name = v.substr(pos, lparen - pos);
        func_name.erase(0, func_name.find_first_not_of(" \t\n\r"));
        size_t last = func_name.find_last_not_of(" \t\n\r");
        if (last != std::string::npos) func_name = func_name.substr(0, last + 1);
        
        std::string args = v.substr(lparen + 1, rparen - lparen - 1);
        
        CSSFilter filter;
        
        if (func_name == "blur") {
            filter.type = CSSFilter::Type::BLUR;
            filter.value = parseFloat(args);
        } else if (func_name == "brightness") {
            filter.type = CSSFilter::Type::BRIGHTNESS;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "contrast") {
            filter.type = CSSFilter::Type::CONTRAST;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "grayscale") {
            filter.type = CSSFilter::Type::GRAYSCALE;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "hue-rotate") {
            filter.type = CSSFilter::Type::HUE_ROTATE;
            filter.value = parseFloat(args);
        } else if (func_name == "invert") {
            filter.type = CSSFilter::Type::INVERT;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "opacity") {
            filter.type = CSSFilter::Type::OPACITY;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "saturate") {
            filter.type = CSSFilter::Type::SATURATE;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "sepia") {
            filter.type = CSSFilter::Type::SEPIA;
            filter.value = parseFloat(args);
            if (args.find('%') != std::string::npos) {
                filter.value /= 100.0f;
            }
        } else if (func_name == "drop-shadow") {
            filter.type = CSSFilter::Type::DROP_SHADOW;
            // Parse drop-shadow arguments
            std::istringstream iss(args);
            std::string part;
            std::vector<std::string> parts;
            while (iss >> part) {
                parts.push_back(part);
            }
            if (parts.size() >= 2) {
                filter.offset_x = parseFloat(parts[0]);
                filter.offset_y = parseFloat(parts[1]);
            }
            if (parts.size() >= 3) {
                filter.blur = parseFloat(parts[2]);
            }
            if (parts.size() >= 4) {
                filter.color = parseColor(parts[3]);
            }
        }
        
        filters.push_back(filter);
        pos = rparen + 1;
    }
    
    return filters;
}

std::vector<CSSTransition> CSSParser::parseTransition(const std::string& value) {
    std::vector<CSSTransition> transitions;
    
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    if (v == "none") {
        return transitions;
    }
    
    // Split by comma (respecting parentheses)
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;
    
    for (char c : v) {
        if (c == '(') {
            ++paren_depth;
            current += c;
        } else if (c == ')') {
            --paren_depth;
            current += c;
        } else if (c == ',' && paren_depth == 0) {
            std::string trimmed = current;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
            size_t last = trimmed.find_last_not_of(" \t\n\r");
            if (last != std::string::npos) trimmed = trimmed.substr(0, last + 1);
            if (!trimmed.empty()) {
                parts.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        std::string trimmed = current;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        size_t last = trimmed.find_last_not_of(" \t\n\r");
        if (last != std::string::npos) trimmed = trimmed.substr(0, last + 1);
        if (!trimmed.empty()) {
            parts.push_back(trimmed);
        }
    }
    
    for (const auto& part : parts) {
        CSSTransition transition;
        
        std::istringstream iss(part);
        std::string token;
        std::vector<std::string> tokens;
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        for (const auto& t : tokens) {
            if (t.find('s') != std::string::npos && t.find("ease") == std::string::npos) {
                float time = parseFloat(t);
                if (t.find("ms") != std::string::npos) {
                    time /= 1000.0f;
                }
                if (transition.duration == 0.0f) {
                    transition.duration = time;
                } else {
                    transition.delay = time;
                }
            } else if (t == "ease" || t == "ease-in" || t == "ease-out" || 
                       t == "ease-in-out" || t == "linear" || 
                       t.find("cubic-bezier") == 0 || t.find("steps") == 0) {
                transition.timing_function = t;
            } else {
                transition.property = t;
            }
        }
        
        transitions.push_back(transition);
    }
    
    return transitions;
}

CSSAnimation CSSParser::parseAnimation(const std::string& value) {
    CSSAnimation animation;
    
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    std::istringstream iss(v);
    std::string token;
    std::vector<std::string> tokens;
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    for (const auto& t : tokens) {
        if (t.find('s') != std::string::npos && 
            t.find("ease") == std::string::npos &&
            t.find("forwards") == std::string::npos &&
            t.find("backwards") == std::string::npos) {
            float time = parseFloat(t);
            if (t.find("ms") != std::string::npos) {
                time /= 1000.0f;
            }
            if (animation.duration == 0.0f) {
                animation.duration = time;
            } else {
                animation.delay = time;
            }
        } else if (t == "ease" || t == "ease-in" || t == "ease-out" || 
                   t == "ease-in-out" || t == "linear" ||
                   t.find("cubic-bezier") == 0 || t.find("steps") == 0) {
            animation.timing_function = t;
        } else if (t == "infinite") {
            animation.iteration_count = -1;
        } else if (t == "normal" || t == "reverse" || 
                   t == "alternate" || t == "alternate-reverse") {
            animation.direction = t;
        } else if (t == "none" || t == "forwards" || 
                   t == "backwards" || t == "both") {
            animation.fill_mode = t;
        } else if (t == "running" || t == "paused") {
            animation.play_state = t;
        } else if (std::isdigit(static_cast<unsigned char>(t[0]))) {
            animation.iteration_count = static_cast<int>(parseFloat(t));
        } else {
            animation.name = t;
        }
    }
    
    return animation;
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

std::vector<KeyframesRule> CSSParser::parseKeyframes(const std::string& css) {
    std::vector<KeyframesRule> result;
    std::string css_clean = removeComments(css);
    
    size_t pos = 0;
    while ((pos = css_clean.find("@keyframes", pos)) != std::string::npos) {
        size_t name_start = pos + 10;
        while (name_start < css_clean.length() && 
               std::isspace(static_cast<unsigned char>(css_clean[name_start]))) {
            ++name_start;
        }
        
        size_t brace_open = css_clean.find('{', name_start);
        if (brace_open == std::string::npos) break;
        
        std::string name = trimWhitespace(css_clean.substr(name_start, brace_open - name_start));
        
        // Find matching closing brace
        int depth = 1;
        size_t end = brace_open + 1;
        while (end < css_clean.length() && depth > 0) {
            if (css_clean[end] == '{') ++depth;
            else if (css_clean[end] == '}') --depth;
            ++end;
        }
        
        std::string content = css_clean.substr(brace_open + 1, end - brace_open - 2);
        
        KeyframesRule keyframes;
        keyframes.name = name;
        
        // Parse keyframe blocks
        size_t kf_pos = 0;
        while (kf_pos < content.length()) {
            size_t kf_brace = content.find('{', kf_pos);
            if (kf_brace == std::string::npos) break;
            
            size_t kf_close = content.find('}', kf_brace);
            if (kf_close == std::string::npos) break;
            
            std::string offset_str = trimWhitespace(content.substr(kf_pos, kf_brace - kf_pos));
            std::string props_str = content.substr(kf_brace + 1, kf_close - kf_brace - 1);
            
            CSSKeyframe keyframe;
            if (offset_str == "from") {
                keyframe.offset = 0.0f;
            } else if (offset_str == "to") {
                keyframe.offset = 1.0f;
            } else {
                keyframe.offset = parseFloat(offset_str) / 100.0f;
            }
            
            auto decls = splitDeclarations(props_str);
            for (const auto& decl : decls) {
                size_t colon = decl.find(':');
                if (colon != std::string::npos) {
                    std::string prop = trimWhitespace(decl.substr(0, colon));
                    std::string value = trimWhitespace(decl.substr(colon + 1));
                    keyframe.properties[prop] = value;
                }
            }
            
            keyframes.keyframes.push_back(keyframe);
            kf_pos = kf_close + 1;
        }
        
        result.push_back(keyframes);
        keyframes_map_[name] = keyframes;
        pos = end;
    }
    
    return result;
}

std::vector<MediaRule> CSSParser::parseMediaRules(const std::string& css) {
    std::vector<MediaRule> result;
    std::string css_clean = removeComments(css);
    
    size_t pos = 0;
    while ((pos = css_clean.find("@media", pos)) != std::string::npos) {
        size_t brace_open = css_clean.find('{', pos);
        if (brace_open == std::string::npos) break;
        
        std::string query = trimWhitespace(css_clean.substr(pos + 6, brace_open - pos - 6));
        
        // Find matching closing brace
        int depth = 1;
        size_t end = brace_open + 1;
        while (end < css_clean.length() && depth > 0) {
            if (css_clean[end] == '{') ++depth;
            else if (css_clean[end] == '}') --depth;
            ++end;
        }
        
        std::string content = css_clean.substr(brace_open + 1, end - brace_open - 2);
        
        MediaRule media;
        media.query = query;
        
        // Parse rules inside media block
        CSSParser inner_parser;
        media.rules = inner_parser.parse(content);
        
        result.push_back(media);
        pos = end;
    }
    
    return result;
}

std::vector<FontFaceRule> CSSParser::parseFontFaceRules(const std::string& css) {
    std::vector<FontFaceRule> result;
    std::string css_clean = removeComments(css);
    
    size_t pos = 0;
    while ((pos = css_clean.find("@font-face", pos)) != std::string::npos) {
        size_t brace_open = css_clean.find('{', pos);
        if (brace_open == std::string::npos) break;
        
        size_t brace_close = css_clean.find('}', brace_open);
        if (brace_close == std::string::npos) break;
        
        std::string content = css_clean.substr(brace_open + 1, brace_close - brace_open - 1);
        
        FontFaceRule font_face;
        
        auto decls = splitDeclarations(content);
        for (const auto& decl : decls) {
            size_t colon = decl.find(':');
            if (colon != std::string::npos) {
                std::string prop = trimWhitespace(decl.substr(0, colon));
                std::string value = trimWhitespace(decl.substr(colon + 1));
                
                if (prop == "font-family") {
                    // Remove quotes
                    if ((value.front() == '"' && value.back() == '"') ||
                        (value.front() == '\'' && value.back() == '\'')) {
                        value = value.substr(1, value.length() - 2);
                    }
                    font_face.family = value;
                } else if (prop == "src") {
                    font_face.src = value;
                } else if (prop == "font-style") {
                    font_face.style = value;
                } else if (prop == "font-weight") {
                    font_face.weight = value;
                }
            }
        }
        
        result.push_back(font_face);
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

void CSSParser::applyProperty(const std::string& property, const std::string& value,
                               ComputedStyle& style) {
    // Normalize property name to lowercase
    std::string prop = property;
    std::transform(prop.begin(), prop.end(), prop.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    // Normalize value: remove trailing semicolon and trim whitespace
    std::string val = value;
    if (!val.empty() && val.back() == ';') {
        val.pop_back();
    }
    val.erase(0, val.find_first_not_of(" \t\n\r"));
    size_t last = val.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) val = val.substr(0, last + 1);

    // Handle CSS global keywords: inherit, initial, unset
    if (val == "inherit" || val == "initial" || val == "unset") {
        // Mark the property with a special value to indicate global keyword
        style.global_keyword_properties_[prop] = val;
        // Mark as explicitly set to prevent normal inheritance
        style.markExplicitlySet(prop);
        return;
    }

    // O(1) dispatch table lookup instead of O(n) if-else chain
    static const auto& handlers = getPropertyHandlers();
    auto it = handlers.find(prop);
    if (it != handlers.end()) {
        it->second(val, style);
    }
    // Unknown properties are silently ignored (standard CSS behavior)
}

void CSSParser::parseMarginShorthand(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        auto v = parseValue(parts[0]);
        style.margin_top = style.margin_right = style.margin_bottom = style.margin_left = v;
    } else if (parts.size() == 2) {
        style.margin_top = style.margin_bottom = parseValue(parts[0]);
        style.margin_left = style.margin_right = parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.margin_top = parseValue(parts[0]);
        style.margin_left = style.margin_right = parseValue(parts[1]);
        style.margin_bottom = parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.margin_top = parseValue(parts[0]);
        style.margin_right = parseValue(parts[1]);
        style.margin_bottom = parseValue(parts[2]);
        style.margin_left = parseValue(parts[3]);
    }
}

void CSSParser::parsePaddingShorthand(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        auto v = parseValue(parts[0]);
        style.padding_top = style.padding_right = style.padding_bottom = style.padding_left = v;
    } else if (parts.size() == 2) {
        style.padding_top = style.padding_bottom = parseValue(parts[0]);
        style.padding_left = style.padding_right = parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.padding_top = parseValue(parts[0]);
        style.padding_left = style.padding_right = parseValue(parts[1]);
        style.padding_bottom = parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.padding_top = parseValue(parts[0]);
        style.padding_right = parseValue(parts[1]);
        style.padding_bottom = parseValue(parts[2]);
        style.padding_left = parseValue(parts[3]);
    }
}

void CSSParser::parseBorderShorthand(const std::string& value, ComputedStyle& style) {
    // Split by spaces but respect parentheses (for rgba, rgb, etc.)
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;
    
    for (size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        if (c == '(') {
            ++paren_depth;
            current += c;
        } else if (c == ')') {
            --paren_depth;
            current += c;
        } else if (c == ' ' && paren_depth == 0) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    
    for (const auto& part : parts) {
        if (part.empty()) continue;
        if (std::isdigit(static_cast<unsigned char>(part[0])) || part[0] == '.') {
            style.border_width = parseFloat(part);
        } else if (part == "solid" || part == "dashed" || part == "dotted" || 
                   part == "double" || part == "none" || part == "hidden") {
            style.border_style = part;
        } else {
            style.border_color = parseColor(part);
        }
    }
}

void CSSParser::parseBorderRadiusShorthand(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        CSSValue v = parseValue(parts[0]);
        style.border_radius = (v.unit == CSSValue::Unit::PIXEL) ? v.value : v.value;
        style.border_top_left_radius = v;
        style.border_top_right_radius = v;
        style.border_bottom_left_radius = v;
        style.border_bottom_right_radius = v;
    } else if (parts.size() == 2) {
        style.border_top_left_radius = style.border_bottom_right_radius = parseValue(parts[0]);
        style.border_top_right_radius = style.border_bottom_left_radius = parseValue(parts[1]);
    } else if (parts.size() == 3) {
        style.border_top_left_radius = parseValue(parts[0]);
        style.border_top_right_radius = style.border_bottom_left_radius = parseValue(parts[1]);
        style.border_bottom_right_radius = parseValue(parts[2]);
    } else if (parts.size() >= 4) {
        style.border_top_left_radius = parseValue(parts[0]);
        style.border_top_right_radius = parseValue(parts[1]);
        style.border_bottom_right_radius = parseValue(parts[2]);
        style.border_bottom_left_radius = parseValue(parts[3]);
    }
}

void CSSParser::parseFlexShorthand(const std::string& value, ComputedStyle& style) {
    if (value == "none") {
        style.flex_grow = 0.0f;
        style.flex_shrink = 0.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);
        return;
    }
    if (value == "auto") {
        style.flex_grow = 1.0f;
        style.flex_shrink = 1.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);
        return;
    }
    
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);
    
    if (parts.size() == 1) {
        style.flex_grow = parseFloat(parts[0]);
        style.flex_shrink = 1.0f;
        style.flex_basis = CSSValue(0.0f, CSSValue::Unit::PIXEL);
    } else if (parts.size() == 2) {
        style.flex_grow = parseFloat(parts[0]);
        if (parts[1].find('%') != std::string::npos || parts[1].find("px") != std::string::npos) {
            style.flex_basis = parseValue(parts[1]);
        } else {
            style.flex_shrink = parseFloat(parts[1]);
        }
    } else if (parts.size() >= 3) {
        style.flex_grow = parseFloat(parts[0]);
        style.flex_shrink = parseFloat(parts[1]);
        style.flex_basis = parseValue(parts[2]);
    }
    
    style.flex = style.flex_grow;
}

void CSSParser::parseBackgroundShorthand(const std::string& value, ComputedStyle& style) {
    if (value.find("gradient") != std::string::npos) {
        style.background_gradients.push_back(parseGradient(value));
    } else if (value.find("url(") != std::string::npos) {
        size_t url_start = value.find("url(");
        size_t url_end = value.find(")", url_start);
        if (url_end != std::string::npos) {
            style.background_image = value.substr(url_start, url_end - url_start + 1);
        }
    } else {
        style.background_color = parseColor(value);
    }
}

void CSSParser::parseFontShorthand(const std::string& value, ComputedStyle& style) {
    // Simplified font shorthand parsing
    std::istringstream iss(value);
    std::string part;
    std::vector<std::string> parts;
    while (iss >> part) parts.push_back(part);
    
    for (size_t i = 0; i < parts.size(); ++i) {
        const std::string& p = parts[i];
        if (p == "italic" || p == "oblique") {
            style.font_style = p;
        } else if (p == "bold" || p == "bolder" || p == "lighter" ||
                   (std::isdigit(static_cast<unsigned char>(p[0])) && p.find("px") == std::string::npos)) {
            style.font_weight = p;
        } else if (p.find("px") != std::string::npos || p.find("em") != std::string::npos ||
                   p.find("rem") != std::string::npos || p.find('%') != std::string::npos) {
            // Check for line-height
            size_t slash = p.find('/');
            if (slash != std::string::npos) {
                style.font_size = parseFloat(p.substr(0, slash));
                std::string lh = p.substr(slash + 1);
                style.has_line_height = true;
                if (lh.find("px") != std::string::npos) {
                    style.line_height = parseFloat(lh);
                    style.line_height_is_unitless = false;
                } else {
                    style.line_height = parseFloat(lh);
                    style.line_height_is_unitless = true;
                }

            } else {
                style.font_size = parseFloat(p);
            }
        } else {
            // Assume it's font-family (rest of the string)
            std::string family;
            for (size_t j = i; j < parts.size(); ++j) {
                if (!family.empty()) family += " ";
                family += parts[j];
            }
            style.font_family = family;
            break;
        }
    }
}

void CSSParser::parseBoxShadow(const std::string& value, ComputedStyle& style) {
    style.box_shadows.clear();
    
    if (value == "none") {
        return;
    }
    
    // Split by comma (respecting parentheses for rgba, etc.)
    std::vector<std::string> shadow_parts;
    int paren_depth = 0;
    std::string current;
    for (char c : value) {
        if (c == '(') {
            ++paren_depth;
            current.push_back(c);
        } else if (c == ')') {
            --paren_depth;
            current.push_back(c);
        } else if (c == ',' && paren_depth == 0) {
            std::string trimmed = current;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
            size_t last = trimmed.find_last_not_of(" \t\n\r");
            if (last != std::string::npos) trimmed = trimmed.substr(0, last + 1);
            if (!trimmed.empty()) {
                shadow_parts.push_back(trimmed);
            }
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        std::string trimmed = current;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        size_t last = trimmed.find_last_not_of(" \t\n\r");
        if (last != std::string::npos) trimmed = trimmed.substr(0, last + 1);
        if (!trimmed.empty()) {
            shadow_parts.push_back(trimmed);
        }
    }
    
    for (const auto& part : shadow_parts) {
        BoxShadow shadow;
        
        // Check for inset
        std::string remaining = part;
        if (remaining.find("inset") != std::string::npos) {
            shadow.inset = true;
            size_t inset_pos = remaining.find("inset");
            remaining = remaining.substr(0, inset_pos) + remaining.substr(inset_pos + 5);
        }
        
        // Parse lengths and color
        std::vector<float> lengths;
        std::string color;
        
        size_t i = 0;
        while (i < remaining.size()) {
            while (i < remaining.size() && std::isspace(static_cast<unsigned char>(remaining[i]))) ++i;
            if (i >= remaining.size()) break;
            
            size_t token_start = i;
            while (i < remaining.size() && !std::isspace(static_cast<unsigned char>(remaining[i]))) ++i;
            std::string token = remaining.substr(token_start, i - token_start);
            
            if (!token.empty() && (token[0] == '#' || std::isalpha(static_cast<unsigned char>(token[0])))) {
                // Color - rest of the string is color
                color = remaining.substr(token_start);
                color.erase(0, color.find_first_not_of(" \t\n\r"));
                size_t last = color.find_last_not_of(" \t\n\r");
                if (last != std::string::npos) color = color.substr(0, last + 1);
                break;
            }
            lengths.push_back(parseFloat(token));
        }
        
        if (lengths.size() >= 2) {
            shadow.offset_x = lengths[0];
            shadow.offset_y = lengths[1];
        }
        if (lengths.size() >= 3) {
            shadow.blur_radius = lengths[2];
        }
        if (lengths.size() >= 4) {
            shadow.spread_radius = lengths[3];
        }
        if (!color.empty()) {
            shadow.color = parseColor(color);
        }
        
        style.box_shadows.push_back(shadow);
    }
}

void CSSParser::parseTextShadow(const std::string& value, ComputedStyle& style) {
    if (value == "none") {
        style.text_shadow_offset_x = 0.0f;
        style.text_shadow_offset_y = 0.0f;
        style.text_shadow_blur = 0.0f;
        style.text_shadow_color.clear();
        return;
    }
    
    std::istringstream iss(value);
    std::string part;
    std::vector<std::string> parts;
    while (iss >> part) parts.push_back(part);
    
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
        style.text_shadow_color = parseColor(color);
    }
}

void CSSParser::parseTransform(const std::string& value, ComputedStyle& style) {
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    
    // Reset transform values
    style.transform_translate_x = 0.0f;
    style.transform_translate_y = 0.0f;
    style.transform_scale_x = 1.0f;
    style.transform_scale_y = 1.0f;
    style.transform_rotate = 0.0f;
    style.transform_skew_x = 0.0f;
    style.transform_skew_y = 0.0f;
    
    if (v == "none") {
        return;
    }
    
    size_t pos = 0;
    while (pos < v.size()) {
        size_t lparen = v.find('(', pos);
        if (lparen == std::string::npos) break;
        size_t rparen = v.find(')', lparen + 1);
        if (rparen == std::string::npos) break;
        
        std::string func_name = v.substr(pos, lparen - pos);
        func_name.erase(0, func_name.find_first_not_of(" \t\n\r"));
        size_t last = func_name.find_last_not_of(" \t\n\r");
        if (last != std::string::npos) func_name = func_name.substr(0, last + 1);
        
        std::string args = v.substr(lparen + 1, rparen - lparen - 1);
        
        // Parse arguments
        std::vector<std::string> arg_parts;
        std::string current;
        for (char c : args) {
            if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    arg_parts.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            arg_parts.push_back(current);
        }
        
        if (func_name == "translate" || func_name == "translate3d") {
            if (!arg_parts.empty()) {
                style.transform_translate_x = parseFloat(arg_parts[0]);
                if (arg_parts.size() > 1) {
                    style.transform_translate_y = parseFloat(arg_parts[1]);
                }
            }
        } else if (func_name == "translatex") {
            style.transform_translate_x = parseFloat(args);
        } else if (func_name == "translatey") {
            style.transform_translate_y = parseFloat(args);
        } else if (func_name == "scale" || func_name == "scale3d") {
            if (!arg_parts.empty()) {
                float sx = parseFloat(arg_parts[0]);
                float sy = arg_parts.size() > 1 ? parseFloat(arg_parts[1]) : sx;
                if (sx != 0.0f) style.transform_scale_x = sx;
                if (sy != 0.0f) style.transform_scale_y = sy;
            }
        } else if (func_name == "scalex") {
            float sx = parseFloat(args);
            if (sx != 0.0f) style.transform_scale_x = sx;
        } else if (func_name == "scaley") {
            float sy = parseFloat(args);
            if (sy != 0.0f) style.transform_scale_y = sy;
        } else if (func_name == "rotate" || func_name == "rotatez") {
            float angle = parseFloat(args);
            if (args.find("rad") != std::string::npos) {
                angle = angle * 180.0f / 3.14159265358979f;
            } else if (args.find("turn") != std::string::npos) {
                angle = angle * 360.0f;
            }
            style.transform_rotate = angle;
        } else if (func_name == "skew") {
            if (!arg_parts.empty()) {
                style.transform_skew_x = parseFloat(arg_parts[0]);
                if (arg_parts.size() > 1) {
                    style.transform_skew_y = parseFloat(arg_parts[1]);
                }
            }
        } else if (func_name == "skewx") {
            style.transform_skew_x = parseFloat(args);
        } else if (func_name == "skewy") {
            style.transform_skew_y = parseFloat(args);
        }
        
        pos = rparen + 1;
    }
}

// Parse CSS background-position value with support for 1-value, 2-value, 3-value, and 4-value syntax
// Examples:
// - "center" -> "50% 50%"
// - "left top" -> "0% 0%"
// - "right 10px" -> "calc(100% - 10px) 50%"
// - "right 10px bottom 20px" -> "calc(100% - 10px) calc(100% - 20px)"
std::string CSSParser::parseBackgroundPosition(const std::string& value) {
    if (value.empty()) return "0% 0%";

    std::istringstream iss(value);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    // Handle different number of tokens
    if (tokens.size() == 1) {
        // Single value: center, left, right, top, bottom, percentage, length
        std::string first = tokens[0];

        if (first == "center") return "50% 50%";
        if (first == "left") return "0% 50%";
        if (first == "right") return "100% 50%";
        if (first == "top") return "50% 0%";
        if (first == "bottom") return "50% 100%";

        // Single percentage or length: apply to both axes
        return first + " " + first;
    }
    else if (tokens.size() == 2) {
        // Two values: horizontal vertical
        std::string first = tokens[0];
        std::string second = tokens[1];

        // Convert keywords to percentages
        if (first == "left") first = "0%";
        else if (first == "right") first = "100%";
        else if (first == "center") first = "50%";

        if (second == "top") second = "0%";
        else if (second == "bottom") second = "100%";
        else if (second == "center") second = "50%";

        return first + " " + second;
    }
    else if (tokens.size() == 3) {
        // Three values: edge offset vertical
        std::string edge = tokens[0];
        std::string offset = tokens[1];
        std::string vertical = tokens[2];

        if (edge == "left" || edge == "right") {
            // Horizontal edge with offset, then vertical position
            std::string horizontal;
            if (edge == "left") horizontal = offset;
            else horizontal = "calc(100% - " + offset + ")";

            if (vertical == "top") vertical = "0%";
            else if (vertical == "bottom") vertical = "100%";
            else if (vertical == "center") vertical = "50%";

            return horizontal + " " + vertical;
        }
        else if (edge == "top" || edge == "bottom") {
            // Vertical edge with offset, then horizontal position
            std::string vertical;
            if (edge == "top") vertical = offset;
            else vertical = "calc(100% - " + offset + ")";

            if (tokens[2] == "left") tokens[2] = "0%";
            else if (tokens[2] == "right") tokens[2] = "100%";
            else if (tokens[2] == "center") tokens[2] = "50%";

            return tokens[2] + " " + vertical;
        }
    }
    else if (tokens.size() == 4) {
        // Four values: horizontal-edge horizontal-offset vertical-edge vertical-offset
        std::string horizEdge = tokens[0];
        std::string horizOffset = tokens[1];
        std::string vertEdge = tokens[2];
        std::string vertOffset = tokens[3];

        std::string horizontal, vertical;

        if (horizEdge == "left") horizontal = horizOffset;
        else if (horizEdge == "right") horizontal = "calc(100% - " + horizOffset + ")";
        else if (horizEdge == "center") horizontal = "calc(50% + " + horizOffset + ")";

        if (vertEdge == "top") vertical = vertOffset;
        else if (vertEdge == "bottom") vertical = "calc(100% - " + vertOffset + ")";
        else if (vertEdge == "center") vertical = "calc(50% + " + vertOffset + ")";

        return horizontal + " " + vertical;
    }

    // Fallback for invalid syntax
    return "0% 0%";
}

std::vector<LayerRule> CSSParser::parseLayerRules(const std::string& css) {
    std::vector<LayerRule> result;
    std::string css_clean = removeComments(css);

    size_t pos = 0;
    int layer_order_counter = 0;

    while (pos < css_clean.length()) {
        // 查找@layer声明
        size_t layer_start = css_clean.find("@layer", pos);
        if (layer_start == std::string::npos) break;

        size_t name_start = layer_start + 6; // "@layer"的长度

        // 跳过空白字符
        while (name_start < css_clean.length() &&
               std::isspace(static_cast<unsigned char>(css_clean[name_start]))) {
            ++name_start;
        }

        if (name_start >= css_clean.length()) break;

        // 检查是否有开括号（表示带规则的层）
        size_t brace_open = css_clean.find('{', name_start);
        size_t semicolon = css_clean.find(';', name_start);

        // 处理预声明层（@layer name; 或 @layer name1, name2;）
        if (semicolon != std::string::npos && (brace_open == std::string::npos || semicolon < brace_open)) {
            std::string layer_names = css_clean.substr(name_start, semicolon - name_start);
            layer_names = trimWhitespace(layer_names);

            // 解析多个层名（逗号分隔）
            size_t comma_pos = 0;
            size_t comma_next = layer_names.find(',', comma_pos);

            while (true) {
                std::string layer_name;
                if (comma_next == std::string::npos) {
                    layer_name = trimWhitespace(layer_names.substr(comma_pos));
                } else {
                    layer_name = trimWhitespace(layer_names.substr(comma_pos, comma_next - comma_pos));
                }

                if (!layer_name.empty()) {
                    LayerRule layer(layer_name, layer_order_counter++, true);
                    result.push_back(layer);
                }

                if (comma_next == std::string::npos) break;
                comma_pos = comma_next + 1;
                comma_next = layer_names.find(',', comma_pos);
            }

            pos = semicolon + 1;
            continue;
        }

        // 处理带规则的层（@layer name { ... } 或 @layer { ... }）
        if (brace_open != std::string::npos) {
            // 提取层名（匿名层为空字符串）
            std::string layer_name;
            if (name_start < brace_open) {
                layer_name = trimWhitespace(css_clean.substr(name_start, brace_open - name_start));
            }

            // 查找匹配的闭括号
            int depth = 1;
            size_t end = brace_open + 1;
            while (end < css_clean.length() && depth > 0) {
                if (css_clean[end] == '{') ++depth;
                else if (css_clean[end] == '}') --depth;
                ++end;
            }

            if (depth > 0) break; // 括号不匹配

            std::string content = css_clean.substr(brace_open + 1, end - brace_open - 2);

            // 解析层内的CSS规则
            LayerRule layer(layer_name, layer_order_counter++, false);
            CSSParser inner_parser;
            layer.rules = inner_parser.parse(content);

            result.push_back(layer);
            pos = end;
        } else {
            // 无效语法，跳过
            pos = name_start + 1;
        }
    }

    return result;
}

} // namespace dong::dom
