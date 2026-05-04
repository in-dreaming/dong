// Compatibility header - forwards to new modular structure
// Note: Consider updating includes to use "dom/html/html_parser.hpp" directly
#pragma once

#include "html/html_parser.hpp"

namespace dong::dom {
// Alias for backward compatibility
using Parser = HTMLParser;
} // namespace dong::dom
