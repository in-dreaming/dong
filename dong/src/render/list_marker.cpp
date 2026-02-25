#include "list_marker.hpp"
#include <algorithm>
#include <stdexcept>

namespace dong {

std::string generateMarkerText(int counter, const std::string& type) {
    if (counter <= 0) {
        return "";
    }

    if (type == ListMarkerType::NONE) {
        return "";
    }

    if (type == ListMarkerType::DISC) {
        return "•";
    }

    if (type == ListMarkerType::CIRCLE) {
        return "○";
    }

    if (type == ListMarkerType::SQUARE) {
        return "▪";
    }

    if (type == ListMarkerType::DECIMAL) {
        return std::to_string(counter) + ".";
    }

    if (type == ListMarkerType::LOWER_ALPHA) {
        return toAlphabetic(counter, false) + ".";
    }

    if (type == ListMarkerType::UPPER_ALPHA) {
        return toAlphabetic(counter, true) + ".";
    }

    if (type == ListMarkerType::LOWER_ROMAN) {
        return toRoman(counter, false) + ".";
    }

    if (type == ListMarkerType::UPPER_ROMAN) {
        return toRoman(counter, true) + ".";
    }

    // Fallback: treat unknown types as decimal
    return std::to_string(counter) + ".";
}

std::string toRoman(int num, bool uppercase) {
    if (num <= 0 || num > 3999) {
        // Standard Roman numerals only support 1-3999
        return "";
    }

    const std::pair<int, std::string> values[] = {
        {1000, "M"},
        {900, "CM"},
        {500, "D"},
        {400, "CD"},
        {100, "C"},
        {90, "XC"},
        {50, "L"},
        {40, "XL"},
        {10, "X"},
        {9, "IX"},
        {5, "V"},
        {4, "IV"},
        {1, "I"}
    };

    std::string result;
    for (const auto& [value, numeral] : values) {
        while (num >= value) {
            result += numeral;
            num -= value;
        }
    }

    if (!uppercase) {
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
    }

    return result;
}

std::string toAlphabetic(int num, bool uppercase) {
    if (num <= 0) {
        return "";
    }

    std::string result;
    num--; // Make it 0-based

    while (num >= 0) {
        int digit = num % 26;
        char c = uppercase ? 'A' + digit : 'a' + digit;
        result = std::string(1, c) + result;
        num = num / 26 - 1;
        if (num < 0) break;
    }

    return result;
}

bool isUnorderedMarker(const std::string& type) {
    return type == ListMarkerType::DISC ||
           type == ListMarkerType::CIRCLE ||
           type == ListMarkerType::SQUARE;
}

bool isOrderedMarker(const std::string& type) {
    return type == ListMarkerType::DECIMAL ||
           type == ListMarkerType::LOWER_ALPHA ||
           type == ListMarkerType::UPPER_ALPHA ||
           type == ListMarkerType::LOWER_ROMAN ||
           type == ListMarkerType::UPPER_ROMAN;
}

} // namespace dong
