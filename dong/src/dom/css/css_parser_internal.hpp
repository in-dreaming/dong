#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace dong::dom::css_parser_internal {

inline std::string trimASCII(std::string_view input) {
    size_t begin = 0;
    while (begin < input.size() && std::isspace(static_cast<unsigned char>(input[begin]))) {
        ++begin;
    }

    size_t end = input.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }

    return std::string(input.substr(begin, end - begin));
}

inline std::vector<std::string> splitTopLevel(std::string_view input, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    int paren_depth = 0;

    for (char c : input) {
        if (c == '(') {
            ++paren_depth;
            current.push_back(c);
            continue;
        }
        if (c == ')') {
            --paren_depth;
            current.push_back(c);
            continue;
        }

        if (c == delimiter && paren_depth == 0) {
            std::string trimmed = trimASCII(current);
            if (!trimmed.empty()) {
                parts.push_back(std::move(trimmed));
            }
            current.clear();
            continue;
        }

        current.push_back(c);
    }

    std::string trimmed = trimASCII(current);
    if (!trimmed.empty()) {
        parts.push_back(std::move(trimmed));
    }

    return parts;
}

} // namespace dong::dom::css_parser_internal
