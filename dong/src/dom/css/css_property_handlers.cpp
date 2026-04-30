#include "css_parser.hpp"
#include "../../core/string_utils.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <cmath>
#include <functional>
#include <string_view>

namespace dong::dom {
namespace {

// Forward declarations for helper functions used in handlers
inline float parseFontSizeHelper(const std::string& s);
inline float parseFloatHelper(const std::string& s);
inline void parseMarginShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parsePaddingShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBorderShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBorderRadiusShorthandHelper(const std::string& value, ComputedStyle& style);
inline void parseBorderImageShorthandHelper(const std::string& value, ComputedStyle& style);
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
            style.setDisplay(val);
        }},
        {"position", [](const std::string& val, ComputedStyle& style) { style.position = positionFromString(val); }},
        
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
        {"background-repeat", [](const std::string& val, ComputedStyle& style) { style.background_repeat = backgroundRepeatFromString(val); }},
        {"background-position", [](const std::string& val, ComputedStyle& style) { style.background_position = CSSParser::parseBackgroundPosition(val); }},
        {"background-attachment", [](const std::string& val, ComputedStyle& style) { style.background_attachment = backgroundAttachmentFromString(val); }},
        {"background-clip", [](const std::string& val, ComputedStyle& style) { style.background_clip = backgroundBoxFromString(val); }},
        {"background-origin", [](const std::string& val, ComputedStyle& style) { style.background_origin = backgroundBoxFromString(val); }},
        {"object-fit", [](const std::string& val, ComputedStyle& style) { style.object_fit = objectFitFromString(val); }},
        {"object-position", [](const std::string& val, ComputedStyle& style) { style.object_position = val; }},
        {"image-rendering", [](const std::string& val, ComputedStyle& style) { style.image_rendering = imageRenderingFromString(val); }},


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
        {"border-style", [](const std::string& val, ComputedStyle& style) { style.border_style = borderStyleFromString(val); }},

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
        {"border-inline-start-style", [](const std::string& val, ComputedStyle& style) { style.border_inline_start_style = borderStyleFromString(val); }},
        {"border-inline-end-style", [](const std::string& val, ComputedStyle& style) { style.border_inline_end_style = borderStyleFromString(val); }},
        {"border-block-start-style", [](const std::string& val, ComputedStyle& style) { style.border_block_start_style = borderStyleFromString(val); }},
        {"border-block-end-style", [](const std::string& val, ComputedStyle& style) { style.border_block_end_style = borderStyleFromString(val); }},

        // Border per-side components

        {"border-top-width", [](const std::string& val, ComputedStyle& style) { style.border_top_width = parseFloatHelper(val); }},
        {"border-right-width", [](const std::string& val, ComputedStyle& style) { style.border_right_width = parseFloatHelper(val); }},
        {"border-bottom-width", [](const std::string& val, ComputedStyle& style) { style.border_bottom_width = parseFloatHelper(val); }},
        {"border-left-width", [](const std::string& val, ComputedStyle& style) { style.border_left_width = parseFloatHelper(val); }},
        {"border-top-color", [](const std::string& val, ComputedStyle& style) { style.border_top_color = CSSParser::parseColor(val); }},
        {"border-right-color", [](const std::string& val, ComputedStyle& style) { style.border_right_color = CSSParser::parseColor(val); }},
        {"border-bottom-color", [](const std::string& val, ComputedStyle& style) { style.border_bottom_color = CSSParser::parseColor(val); }},
        {"border-left-color", [](const std::string& val, ComputedStyle& style) { style.border_left_color = CSSParser::parseColor(val); }},
        {"border-top-style", [](const std::string& val, ComputedStyle& style) { style.border_top_style = borderStyleFromString(val); }},
        {"border-right-style", [](const std::string& val, ComputedStyle& style) { style.border_right_style = borderStyleFromString(val); }},
        {"border-bottom-style", [](const std::string& val, ComputedStyle& style) { style.border_bottom_style = borderStyleFromString(val); }},
        {"border-left-style", [](const std::string& val, ComputedStyle& style) { style.border_left_style = borderStyleFromString(val); }},

        // Border radius
        {"border-radius", [](const std::string& val, ComputedStyle& style) { parseBorderRadiusShorthandHelper(val, style); }},
        {"border-top-left-radius", [](const std::string& val, ComputedStyle& style) { style.border_top_left_radius = CSSParser::parseValue(val); }},
        {"border-top-right-radius", [](const std::string& val, ComputedStyle& style) { style.border_top_right_radius = CSSParser::parseValue(val); }},
        {"border-bottom-left-radius", [](const std::string& val, ComputedStyle& style) { style.border_bottom_left_radius = CSSParser::parseValue(val); }},
        {"border-bottom-right-radius", [](const std::string& val, ComputedStyle& style) { style.border_bottom_right_radius = CSSParser::parseValue(val); }},

        // Border image (nine-slice)
        {"border-image", [](const std::string& val, ComputedStyle& style) { parseBorderImageShorthandHelper(val, style); }},
        {"border-image-source", [](const std::string& val, ComputedStyle& style) {
            if (val == "none") {
                style.border_image_source.clear();
            } else {
                style.border_image_source = val;
            }
        }},
        {"border-image-slice", [](const std::string& val, ComputedStyle& style) {
            // Parse 1-4 numeric values + optional "fill" / "no-fill"
            std::istringstream iss(val);
            std::vector<float> nums;
            std::string part;
            bool found_fill = false;
            bool found_nofill = false;
            while (iss >> part) {
                std::string lower = part;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (lower == "fill") { found_fill = true; continue; }
                if (lower == "no-fill") { found_nofill = true; continue; }
                // Strip trailing '%' – stored as px for now
                float v = parseFloatHelper(part);
                nums.push_back(v);
            }
            if (found_nofill) style.border_image_fill = false;
            else if (found_fill) style.border_image_fill = true;

            if (nums.size() == 1) {
                style.border_image_slice_top = style.border_image_slice_right =
                    style.border_image_slice_bottom = style.border_image_slice_left = nums[0];
            } else if (nums.size() == 2) {
                style.border_image_slice_top = style.border_image_slice_bottom = nums[0];
                style.border_image_slice_right = style.border_image_slice_left = nums[1];
            } else if (nums.size() == 3) {
                style.border_image_slice_top = nums[0];
                style.border_image_slice_right = style.border_image_slice_left = nums[1];
                style.border_image_slice_bottom = nums[2];
            } else if (nums.size() >= 4) {
                style.border_image_slice_top = nums[0];
                style.border_image_slice_right = nums[1];
                style.border_image_slice_bottom = nums[2];
                style.border_image_slice_left = nums[3];
            }
        }},
        {"border-image-width", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::vector<float> nums;
            std::string part;
            while (iss >> part) {
                nums.push_back(parseFloatHelper(part));
            }
            if (nums.size() == 1) {
                style.border_image_width_top = style.border_image_width_right =
                    style.border_image_width_bottom = style.border_image_width_left = nums[0];
            } else if (nums.size() == 2) {
                style.border_image_width_top = style.border_image_width_bottom = nums[0];
                style.border_image_width_right = style.border_image_width_left = nums[1];
            } else if (nums.size() == 3) {
                style.border_image_width_top = nums[0];
                style.border_image_width_right = style.border_image_width_left = nums[1];
                style.border_image_width_bottom = nums[2];
            } else if (nums.size() >= 4) {
                style.border_image_width_top = nums[0];
                style.border_image_width_right = nums[1];
                style.border_image_width_bottom = nums[2];
                style.border_image_width_left = nums[3];
            }
        }},
        {"border-image-repeat", [](const std::string& val, ComputedStyle& style) {
            auto parseRepeat = [](const std::string& s) -> ComputedStyle::BorderImageRepeat {
                std::string lower = s;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (lower == "repeat") return ComputedStyle::BorderImageRepeat::Repeat;
                if (lower == "round") return ComputedStyle::BorderImageRepeat::Round;
                return ComputedStyle::BorderImageRepeat::Stretch;
            };
            std::istringstream iss(val);
            std::string h_str, v_str;
            iss >> h_str;
            style.border_image_repeat_h = parseRepeat(h_str);
            if (iss >> v_str) {
                style.border_image_repeat_v = parseRepeat(v_str);
            } else {
                style.border_image_repeat_v = style.border_image_repeat_h;
            }
        }},
        {"border-image-fill", [](const std::string& val, ComputedStyle& style) {
            std::string lower = val;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower == "true" || lower == "fill" || lower == "1") {
                style.border_image_fill = true;
            } else {
                style.border_image_fill = false;
            }
        }},

        // CSS mask
        {"mask-image", [](const std::string& val, ComputedStyle& style) {
            if (val == "none") {
                style.mask_image.clear();
            } else {
                style.mask_image = val;
            }
        }},
        {"-webkit-mask-image", [](const std::string& val, ComputedStyle& style) {
            if (val == "none") {
                style.mask_image.clear();
            } else {
                style.mask_image = val;
            }
        }},
        {"mask-mode", [](const std::string& val, ComputedStyle& style) {
            std::string lower = val;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower == "luminance") {
                style.mask_mode = ComputedStyle::MaskMode::Luminance;
            } else {
                style.mask_mode = ComputedStyle::MaskMode::Alpha;
            }
        }},
        {"mask-repeat", [](const std::string& val, ComputedStyle& style) {
            style.mask_repeat = val;
        }},
        {"mask-position", [](const std::string& val, ComputedStyle& style) {
            style.mask_position = val;
        }},
        {"mask-size", [](const std::string& val, ComputedStyle& style) {
            style.mask_size = val;
        }},
        {"mask-clip", [](const std::string& val, ComputedStyle& style) {
            std::string lower = val;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower == "padding-box") {
                style.mask_clip = ComputedStyle::MaskClip::PaddingBox;
            } else if (lower == "content-box") {
                style.mask_clip = ComputedStyle::MaskClip::ContentBox;
            } else {
                style.mask_clip = ComputedStyle::MaskClip::BorderBox;
            }
        }},
        {"mask", [](const std::string& val, ComputedStyle& style) {
            if (val == "none") {
                style.mask_image.clear();
                return;
            }
            // Extract the image function (url(), *-gradient()) by finding
            // the first function call (contains '(') and extracting with balanced parens
            std::string remaining;
            size_t paren_pos = val.find('(');
            if (paren_pos != std::string::npos) {
                // Walk back to find function name start
                size_t name_start = paren_pos;
                while (name_start > 0 && (std::isalnum(static_cast<unsigned char>(val[name_start - 1])) || val[name_start - 1] == '-')) {
                    --name_start;
                }
                // Find matching closing paren
                int depth = 1;
                size_t end = paren_pos + 1;
                while (end < val.size() && depth > 0) {
                    if (val[end] == '(') ++depth;
                    else if (val[end] == ')') --depth;
                    ++end;
                }
                style.mask_image = val.substr(name_start, end - name_start);
                remaining = val.substr(0, name_start) + val.substr(end);
            } else {
                remaining = val;
            }
            // Parse remaining keywords
            std::istringstream iss(remaining);
            std::string part;
            while (iss >> part) {
                std::string lower = part;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (lower == "alpha") {
                    style.mask_mode = ComputedStyle::MaskMode::Alpha;
                } else if (lower == "luminance") {
                    style.mask_mode = ComputedStyle::MaskMode::Luminance;
                } else if (lower == "no-repeat" || lower == "repeat" || lower == "space" || lower == "round") {
                    style.mask_repeat = lower;
                } else if (lower == "border-box") {
                    style.mask_clip = ComputedStyle::MaskClip::BorderBox;
                } else if (lower == "padding-box") {
                    style.mask_clip = ComputedStyle::MaskClip::PaddingBox;
                } else if (lower == "content-box") {
                    style.mask_clip = ComputedStyle::MaskClip::ContentBox;
                } else if (lower == "contain" || lower == "cover") {
                    style.mask_size = lower;
                } else if (lower == "center" || lower == "top" || lower == "bottom" ||
                           lower == "left" || lower == "right") {
                    style.mask_position = lower;
                }
            }
        }},
        {"-webkit-mask", [](const std::string& val, ComputedStyle& style) {
            // Delegate to mask handler
            if (val == "none") {
                style.mask_image.clear();
                return;
            }
            size_t paren_pos = val.find('(');
            if (paren_pos != std::string::npos) {
                size_t name_start = paren_pos;
                while (name_start > 0 && (std::isalnum(static_cast<unsigned char>(val[name_start - 1])) || val[name_start - 1] == '-')) {
                    --name_start;
                }
                int depth = 1;
                size_t end = paren_pos + 1;
                while (end < val.size() && depth > 0) {
                    if (val[end] == '(') ++depth;
                    else if (val[end] == ')') --depth;
                    ++end;
                }
                style.mask_image = val.substr(name_start, end - name_start);
            }
        }},

        // Spatial navigation (P0-4)
        {"nav-up", [](const std::string& val, ComputedStyle& style) { style.nav_up = val; }},
        {"nav-down", [](const std::string& val, ComputedStyle& style) { style.nav_down = val; }},
        {"nav-left", [](const std::string& val, ComputedStyle& style) { style.nav_left = val; }},
        {"nav-right", [](const std::string& val, ComputedStyle& style) { style.nav_right = val; }},


        // Outline
        {"outline", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::string part;
            while (iss >> part) {
                if (std::isdigit(static_cast<unsigned char>(part[0])) || part[0] == '.') {
                    style.outline_width = parseFloatHelper(part);
                } else if (part == "solid" || part == "dashed" || part == "dotted" || part == "none") {
                    style.outline_style = borderStyleFromString(part);
                } else {
                    style.outline_color = CSSParser::parseColor(part);
                }
            }
        }},
        {"outline-width", [](const std::string& val, ComputedStyle& style) { style.outline_width = parseFloatHelper(val); }},
        {"outline-color", [](const std::string& val, ComputedStyle& style) { style.outline_color = CSSParser::parseColor(val); }},
        {"outline-style", [](const std::string& val, ComputedStyle& style) { style.outline_style = borderStyleFromString(val); }},
        {"outline-offset", [](const std::string& val, ComputedStyle& style) { style.outline_offset = parseFloatHelper(val); }},
        
        // Overflow & visibility
        {"overflow", [](const std::string& val, ComputedStyle& style) {
            auto v = overflowFromString(val);
            style.overflow = v;
            style.overflow_x = v;
            style.overflow_y = v;
        }},
        {"overflow-x", [](const std::string& val, ComputedStyle& style) { style.overflow_x = overflowFromString(val); }},
        {"overflow-y", [](const std::string& val, ComputedStyle& style) { style.overflow_y = overflowFromString(val); }},

        // Scroll behavior
        {"overscroll-behavior", [](const std::string& val, ComputedStyle& style) {
            auto v = overscrollBehaviorFromString(val);
            style.overscroll_behavior = v;
            style.overscroll_behavior_x = v;
            style.overscroll_behavior_y = v;
        }},
        {"overscroll-behavior-x", [](const std::string& val, ComputedStyle& style) { style.overscroll_behavior_x = overscrollBehaviorFromString(val); }},
        {"overscroll-behavior-y", [](const std::string& val, ComputedStyle& style) { style.overscroll_behavior_y = overscrollBehaviorFromString(val); }},
        {"scroll-behavior", [](const std::string& val, ComputedStyle& style) { style.scroll_behavior = scrollBehaviorFromString(val); }},

        {"visibility", [](const std::string& val, ComputedStyle& style) { style.visibility = visibilityFromString(val); }},
        {"cursor", [](const std::string& val, ComputedStyle& style) { style.cursor = cursorFromString(val); }},
        {"box-sizing", [](const std::string& val, ComputedStyle& style) { style.box_sizing = boxSizingFromString(val); }},
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
        {"mix-blend-mode", [](const std::string& val, ComputedStyle& style) { style.mix_blend_mode = blendModeFromString(val); }},
        {"background-blend-mode", [](const std::string& val, ComputedStyle& style) { style.background_blend_mode = blendModeFromString(val); }},
        
        // Text & font
        {"font-family", [](const std::string& val, ComputedStyle& style) { style.font_family = val; }},
        {"font-size", [](const std::string& val, ComputedStyle& style) { style.font_size = parseFontSizeHelper(val); }},
        {"font-weight", [](const std::string& val, ComputedStyle& style) { style.font_weight = fontWeightFromString(val); }},
        {"font-style", [](const std::string& val, ComputedStyle& style) { style.font_style = fontStyleFromString(val); }},
        {"font-variant", [](const std::string& val, ComputedStyle& style) { style.font_variant = fontVariantFromString(val); }},
        {"font", [](const std::string& val, ComputedStyle& style) { parseFontShorthandHelper(val, style); }},
        {"text-align", [](const std::string& val, ComputedStyle& style) { style.text_align = textAlignFromString(val); }},
        {"text-align-last", [](const std::string& val, ComputedStyle& style) { style.text_align_last = textAlignLastFromString(val); }},
        {"text-decoration", [](const std::string& val, ComputedStyle& style) { style.text_decoration = textDecorationFromString(val); }},
        {"text-decoration-line", [](const std::string& val, ComputedStyle& style) { style.text_decoration = textDecorationFromString(val); }},
        {"text-decoration-color", [](const std::string& val, ComputedStyle& style) { style.text_decoration_color = CSSParser::parseColor(val); }},
        {"text-decoration-style", [](const std::string& val, ComputedStyle& style) { style.text_decoration_style = textDecorationStyleFromString(val); }},
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
        {"text-transform", [](const std::string& val, ComputedStyle& style) { style.text_transform = textTransformFromString(val); }},
        {"text-overflow", [](const std::string& val, ComputedStyle& style) { style.text_overflow = textOverflowFromString(val); }},
        {"white-space", [](const std::string& val, ComputedStyle& style) { style.white_space = whiteSpaceFromString(val); }},
        {"word-break", [](const std::string& val, ComputedStyle& style) { style.word_break = wordBreakFromString(val); }},
        {"overflow-wrap", [](const std::string& val, ComputedStyle& style) { style.overflow_wrap = overflowWrapFromString(val); }},
        {"word-wrap", [](const std::string& val, ComputedStyle& style) { style.overflow_wrap = overflowWrapFromString(val); }},
        {"hyphens", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (v == "none" || v == "manual" || v == "auto") {
                style.hyphens = hyphensFromString(v);
            }
        }},

        {"vertical-align", [](const std::string& val, ComputedStyle& style) { style.vertical_align = verticalAlignFromString(val); }},
        {"direction", [](const std::string& val, ComputedStyle& style) { style.direction = directionFromString(val); }},
        {"unicode-bidi", [](const std::string& val, ComputedStyle& style) { style.unicode_bidi = unicodeBidiFromString(val); }},
        {"text-indent", [](const std::string& val, ComputedStyle& style) { style.text_indent = parseFloatHelper(val); }},
        {"tab-size", [](const std::string& val, ComputedStyle& style) {
            int tab_val = static_cast<int>(parseFloatHelper(val));
            if (tab_val > 0) style.tab_size = tab_val;
        }},
        {"-webkit-line-clamp", [](const std::string& val, ComputedStyle& style) { style.webkit_line_clamp = static_cast<int>(parseFloatHelper(val)); }},
        {"text-shadow", [](const std::string& val, ComputedStyle& style) { CSSParser::parseTextShadow(val, style); }},
        
        // Flexbox
        {"flex-direction", [](const std::string& val, ComputedStyle& style) { style.flex_direction = flexDirectionFromString(val); }},
        {"flex-wrap", [](const std::string& val, ComputedStyle& style) { style.flex_wrap = flexWrapFromString(val); }},
        {"flex-flow", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::string part;
            while (iss >> part) {
                if (part == "row" || part == "column" || part == "row-reverse" || part == "column-reverse") {
                    style.flex_direction = flexDirectionFromString(part);
                } else if (part == "wrap" || part == "nowrap" || part == "wrap-reverse") {
                    style.flex_wrap = flexWrapFromString(part);
                }
            }
        }},
        {"justify-content", [](const std::string& val, ComputedStyle& style) { style.justify_content = justifyContentFromString(val); }},
        {"justify-items", [](const std::string& val, ComputedStyle& style) { style.justify_items = alignItemsFromString(val); }},
        {"justify-self", [](const std::string& val, ComputedStyle& style) { style.justify_self = alignSelfFromString(val); }},
        {"align-items", [](const std::string& val, ComputedStyle& style) { style.align_items = alignItemsFromString(val); }},
        {"align-content", [](const std::string& val, ComputedStyle& style) { style.align_content = alignContentFromString(val); }},
        {"align-self", [](const std::string& val, ComputedStyle& style) { style.align_self = alignSelfFromString(val); }},
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
            style.list_style_type = listStyleTypeFromString(val);
        }},
        {"list-style-position", [](const std::string& val, ComputedStyle& style) {
            style.list_style_position = listStylePositionFromString(val);
        }},
        {"list-style", [](const std::string& val, ComputedStyle& style) {
            std::istringstream iss(val);
            std::string part;

            // Reset all list-style properties first
            style.list_style_position = CSSListStylePosition::Outside;
            style.list_style_image = "none";
            // type starts as unset; we track whether an explicit type keyword was found
            std::string found_type;
            bool found_none = false;
            bool found_position = false;

            while (iss >> part) {
                if (part == "inside" || part == "outside") {
                    style.list_style_position = listStylePositionFromString(part);
                    found_position = true;
                } else if (part == "none") {
                    found_none = true;
                } else if (part.find("url(") == 0) {
                    style.list_style_image = part;
                } else {
                    found_type = part;
                }
            }

            if (!found_type.empty()) {
                style.list_style_type = listStyleTypeFromString(found_type);
            } else if (found_none) {
                style.list_style_type = CSSListStyleType::None;
            } else if (found_position) {
                style.list_style_type = CSSListStyleType::Disc;
            } else {
                style.list_style_type = CSSListStyleType::None;
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
            style.appearance = appearanceFromString(v);
        }},
        {"-webkit-appearance", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            style.appearance = appearanceFromString(v);
        }},
        {"resize", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (v == "none" || v == "both" || v == "horizontal" || v == "vertical") {
                style.resize = resizeFromString(v);
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
            style.border_collapse = borderCollapseFromString(v);
        }},

        // Color scheme (UA/form control theming)
        {"color-scheme", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            // Per CSS spec, for "light dark" or "dark light", the first listed scheme is preferred.
            size_t light_pos = v.find("light");
            size_t dark_pos = v.find("dark");
            if (light_pos == std::string::npos && dark_pos == std::string::npos) {
                style.color_scheme = CSSColorScheme::Normal;
            } else if (light_pos == std::string::npos) {
                style.color_scheme = CSSColorScheme::Dark;
            } else if (dark_pos == std::string::npos) {
                style.color_scheme = CSSColorScheme::Light;
            } else {
                style.color_scheme = (light_pos < dark_pos) ? CSSColorScheme::Light : CSSColorScheme::Dark;
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
            style.table_layout = tableLayoutFromString(v);
        }},
        {"caption-side", [](const std::string& val, ComputedStyle& style) {
            std::string v = val;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            if (v == "top" || v == "bottom") {
                style.caption_side = captionSideFromString(v);
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
        {"transform-style", [](const std::string& val, ComputedStyle& style) { style.transform_style = transformStyleFromString(val); }},
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
        {"backface-visibility", [](const std::string& val, ComputedStyle& style) { style.backface_visibility = backfaceVisibilityFromString(val); }},
        
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
        {"pointer-events", [](const std::string& val, ComputedStyle& style) { style.pointer_events = pointerEventsFromString(val); }},
        {"user-select", [](const std::string& val, ComputedStyle& style) { style.user_select = userSelectFromString(val); }},
        {"touch-action", [](const std::string& val, ComputedStyle& style) { style.touch_action = touchActionFromString(val); }},
        {"caret-color", [](const std::string& val, ComputedStyle& style) { style.caret_color = CSSParser::parseColor(val); }},
        {"float", [](const std::string& val, ComputedStyle& style) { style.float_value = floatFromString(val); }},
        {"clear", [](const std::string& val, ComputedStyle& style) { style.clear = clearFromString(val); }},
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
        {"nb", {"\u00AB", "\u00BB", "\u2039", "\u203A"}},  // Norwegian Bokm濮樻悋
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

                bool separator_set = false;
                if (t.type == ComputedStyle::ContentToken::Type::Counters) {
                    // separator (required 2nd arg for counters())
                    if (consumeChar(sv, i, ',')) {
                        bool ok = false;
                        std::string sep = parseQuotedStringToken(sv, i, ok);
                        if (ok) {
                            t.separator = sep;
                            separator_set = true;
                        }
                    }
                    if (!separator_set) {
                        t.separator = "."; // default separator
                    }
                    // Optional style param (3rd arg for counters())
                    if (consumeChar(sv, i, ',')) {
                        std::string style_ident = parseIdentToken(sv, i);
                        if (!style_ident.empty()) {
                            t.style = style_ident;
                        }
                    }
                } else {
                    // counter(): optional style param (2nd arg)
                    if (consumeChar(sv, i, ',')) {
                        std::string style_ident = parseIdentToken(sv, i);
                        if (!style_ident.empty()) {
                            t.style = style_ident;
                        }
                    }
                }

                // Skip any remaining args
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
    // Use paren-depth-aware tokenization so env(name, fallback) is kept as one token
    std::vector<std::string> parts;
    std::string part;
    int paren_depth = 0;
    for (size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        if (c == '(') { ++paren_depth; part += c; }
        else if (c == ')') { --paren_depth; part += c; }
        else if ((c == ' ' || c == '\t' || c == '\n' || c == '\r') && paren_depth == 0) {
            if (!part.empty()) { parts.push_back(part); part.clear(); }
        } else { part += c; }
    }
    if (!part.empty()) parts.push_back(part);
    
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
    // Use paren-depth-aware tokenization so env(name, fallback) is kept as one token
    std::vector<std::string> parts;
    std::string part;
    int paren_depth = 0;
    for (size_t i = 0; i < value.size(); ++i) {
        char c = value[i];
        if (c == '(') { ++paren_depth; part += c; }
        else if (c == ')') { --paren_depth; part += c; }
        else if ((c == ' ' || c == '\t' || c == '\n' || c == '\r') && paren_depth == 0) {
            if (!part.empty()) { parts.push_back(part); part.clear(); }
        } else { part += c; }
    }
    if (!part.empty()) parts.push_back(part);
    
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
            style.border_style = borderStyleFromString(part);
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

inline void parseBorderImageShorthandHelper(const std::string& value, ComputedStyle& style) {
    // border-image: <source> <slice> / <width> <repeat>
    // Simplified parser: extract url() first, then split on '/' for slice/width, then repeat keywords

    std::string remaining = value;

    // 1. Extract url(...) source
    size_t url_start = remaining.find("url(");
    if (url_start != std::string::npos) {
        // Find matching closing paren
        int depth = 0;
        size_t url_end = url_start + 4; // skip "url("
        depth = 1;
        while (url_end < remaining.size() && depth > 0) {
            if (remaining[url_end] == '(') ++depth;
            else if (remaining[url_end] == ')') --depth;
            ++url_end;
        }
        style.border_image_source = remaining.substr(url_start, url_end - url_start);
        remaining = remaining.substr(0, url_start) + remaining.substr(url_end);
    } else if (remaining.find("none") != std::string::npos) {
        style.border_image_source.clear();
        // Remove "none" from remaining
        size_t pos = remaining.find("none");
        remaining = remaining.substr(0, pos) + remaining.substr(pos + 4);
    }

    // Trim remaining
    remaining.erase(0, remaining.find_first_not_of(" \t\n\r"));
    size_t last = remaining.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) remaining = remaining.substr(0, last + 1);
    if (remaining.empty()) return;

    // 2. Split on '/' to separate slice from width
    std::string slice_part;
    std::string width_part;
    size_t slash_pos = remaining.find('/');
    if (slash_pos != std::string::npos) {
        slice_part = remaining.substr(0, slash_pos);
        width_part = remaining.substr(slash_pos + 1);
    } else {
        slice_part = remaining;
    }

    // 3. Parse slice part (numbers + fill/no-fill + repeat keywords)
    std::string repeat_str;
    {
        std::istringstream iss(slice_part);
        std::vector<float> nums;
        std::string part;
        bool found_fill = false;
        bool found_nofill = false;
        while (iss >> part) {
            std::string lower = part;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower == "fill") { found_fill = true; continue; }
            if (lower == "no-fill") { found_nofill = true; continue; }
            if (lower == "stretch" || lower == "repeat" || lower == "round") {
                repeat_str += (repeat_str.empty() ? "" : " ") + lower;
                continue;
            }
            float v = parseFloatHelper(part);
            nums.push_back(v);
        }
        if (found_nofill) style.border_image_fill = false;
        else if (found_fill) style.border_image_fill = true;

        if (nums.size() == 1) {
            style.border_image_slice_top = style.border_image_slice_right =
                style.border_image_slice_bottom = style.border_image_slice_left = nums[0];
        } else if (nums.size() == 2) {
            style.border_image_slice_top = style.border_image_slice_bottom = nums[0];
            style.border_image_slice_right = style.border_image_slice_left = nums[1];
        } else if (nums.size() == 3) {
            style.border_image_slice_top = nums[0];
            style.border_image_slice_right = style.border_image_slice_left = nums[1];
            style.border_image_slice_bottom = nums[2];
        } else if (nums.size() >= 4) {
            style.border_image_slice_top = nums[0];
            style.border_image_slice_right = nums[1];
            style.border_image_slice_bottom = nums[2];
            style.border_image_slice_left = nums[3];
        }
    }

    // 4. Parse width part (if present)
    if (!width_part.empty()) {
        std::istringstream iss(width_part);
        std::vector<float> nums;
        std::string part;
        while (iss >> part) {
            std::string lower = part;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (lower == "stretch" || lower == "repeat" || lower == "round") {
                repeat_str += (repeat_str.empty() ? "" : " ") + lower;
                continue;
            }
            nums.push_back(parseFloatHelper(part));
        }
        if (nums.size() == 1) {
            style.border_image_width_top = style.border_image_width_right =
                style.border_image_width_bottom = style.border_image_width_left = nums[0];
        } else if (nums.size() == 2) {
            style.border_image_width_top = style.border_image_width_bottom = nums[0];
            style.border_image_width_right = style.border_image_width_left = nums[1];
        } else if (nums.size() == 3) {
            style.border_image_width_top = nums[0];
            style.border_image_width_right = style.border_image_width_left = nums[1];
            style.border_image_width_bottom = nums[2];
        } else if (nums.size() >= 4) {
            style.border_image_width_top = nums[0];
            style.border_image_width_right = nums[1];
            style.border_image_width_bottom = nums[2];
            style.border_image_width_left = nums[3];
        }
    }

    // 5. Parse repeat keywords (if collected)
    if (!repeat_str.empty()) {
        auto parseRepeat = [](const std::string& s) -> ComputedStyle::BorderImageRepeat {
            if (s == "repeat") return ComputedStyle::BorderImageRepeat::Repeat;
            if (s == "round") return ComputedStyle::BorderImageRepeat::Round;
            return ComputedStyle::BorderImageRepeat::Stretch;
        };
        std::istringstream riss(repeat_str);
        std::string h_str, v_str;
        riss >> h_str;
        style.border_image_repeat_h = parseRepeat(h_str);
        if (riss >> v_str) {
            style.border_image_repeat_v = parseRepeat(v_str);
        } else {
            style.border_image_repeat_v = style.border_image_repeat_h;
        }
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
            style.background_repeat = backgroundRepeatFromString(part);
        } else if (part.find("color-mix(") == std::string::npos &&
                   (part == "cover" || part == "contain" || part.find('%') != std::string::npos)) {
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
            style.font_style = fontStyleFromString(part);
        } else if (part == "bold" || part == "bolder" || part == "lighter" ||
                   part == "100" || part == "200" || part == "300" || part == "400" ||
                   part == "500" || part == "600" || part == "700" || part == "800" || part == "900") {
            style.font_weight = fontWeightFromString(part);
        } else if (part == "small-caps") {
            style.font_variant = fontVariantFromString(part);
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
        style.align_items = alignItemsFromString(parts[0]);
        style.justify_items = alignItemsFromString(parts[0]);
    } else if (parts.size() == 2) {
        style.align_items = alignItemsFromString(parts[0]);
        style.justify_items = alignItemsFromString(parts[1]);
    }
}

inline void parsePlaceContentShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        style.align_content = alignContentFromString(parts[0]);
        style.justify_content = justifyContentFromString(parts[0]);
    } else if (parts.size() == 2) {
        style.align_content = alignContentFromString(parts[0]);
        style.justify_content = justifyContentFromString(parts[1]);
    }
}

inline void parsePlaceSelfShorthandHelper(const std::string& value, ComputedStyle& style) {
    std::istringstream iss(value);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) parts.push_back(part);

    if (parts.size() == 1) {
        style.align_self = alignSelfFromString(parts[0]);
        style.justify_self = alignSelfFromString(parts[0]);
    } else if (parts.size() == 2) {
        style.align_self = alignSelfFromString(parts[0]);
        style.justify_self = alignSelfFromString(parts[1]);
    }
}

} // anonymous namespace

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


} // namespace dong::dom
