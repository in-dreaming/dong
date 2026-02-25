#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace dong {

// Collapse consecutive whitespace into single space, trim leading/trailing
inline std::string collapseWhitespace(const std::string& input) {
    if (input.empty()) return "";
    std::string output;
    output.reserve(input.size());
    bool in_space = false;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
        } else {
            output.push_back(c);
            in_space = false;
        }
    }
    size_t first = output.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = output.find_last_not_of(' ');
    return output.substr(first, last - first + 1);
}

// Collapse consecutive whitespace into single space, but keep trailing space.
// Used in Mixed text path where boundary spaces between text nodes and inline
// elements must be preserved for correct word spacing.
// Trims leading whitespace (HTML formatting indentation) but preserves trailing
// space so adjacent inline elements have proper word separation.
inline std::string collapseWhitespaceKeepTrailing(const std::string& input) {
    if (input.empty()) return "";
    std::string output;
    output.reserve(input.size());
    bool in_space = false;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
        } else {
            output.push_back(c);
            in_space = false;
        }
    }
    // Trim leading spaces only; preserve trailing space for word separation
    size_t first = output.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    return output.substr(first);
}

// Trim leading and trailing whitespace
inline std::string trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}

// Split string by whitespace into parts
inline std::vector<std::string> splitByWhitespace(const std::string& str) {
    std::vector<std::string> parts;
    std::istringstream iss(str);
    std::string part;
    while (iss >> part) {
        parts.push_back(part);
    }
    return parts;
}

// Convert string to lowercase (returns copy)
inline std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

} // namespace dong
