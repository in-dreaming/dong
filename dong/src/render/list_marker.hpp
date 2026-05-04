#pragma once

#include <string>

namespace dong {

/**
 * @brief List marker type constants
 */
namespace ListMarkerType {
    constexpr const char* DISC = "disc";
    constexpr const char* CIRCLE = "circle";
    constexpr const char* SQUARE = "square";
    constexpr const char* DECIMAL = "decimal";
    constexpr const char* LOWER_ALPHA = "lower-alpha";
    constexpr const char* UPPER_ALPHA = "upper-alpha";
    constexpr const char* LOWER_ROMAN = "lower-roman";
    constexpr const char* UPPER_ROMAN = "upper-roman";
    constexpr const char* NONE = "none";
}

/**
 * @brief Generate marker text for a list item
 *
 * @param counter The counter value (1-based index)
 * @param type The marker type (disc, circle, square, decimal, lower-alpha, upper-alpha, lower-roman, upper-roman, none)
 * @return std::string The marker text (e.g., "1.", "a.", "•", "")
 */
std::string generateMarkerText(int counter, const std::string& type);

/**
 * @brief Convert a number to Roman numerals
 *
 * @param num The number to convert (1-3999)
 * @param uppercase Whether to use uppercase letters
 * @return std::string The Roman numeral representation
 */
std::string toRoman(int num, bool uppercase = true);

/**
 * @brief Convert a number to alphabetic representation
 *
 * @param num The number to convert (1-26: a-z, 27-52: aa-az, etc.)
 * @param uppercase Whether to use uppercase letters
 * @return std::string The alphabetic representation
 */
std::string toAlphabetic(int num, bool uppercase = true);

/**
 * @brief Check if a marker type is unordered (symbol-based)
 *
 * @param type The marker type
 * @return true if the type is unordered (disc, circle, square)
 */
bool isUnorderedMarker(const std::string& type);

/**
 * @brief Check if a marker type is ordered (numeric/alphabetic)
 *
 * @param type The marker type
 * @return true if the type is ordered (decimal, lower-alpha, upper-alpha, lower-roman, upper-roman)
 */
bool isOrderedMarker(const std::string& type);

} // namespace dong
