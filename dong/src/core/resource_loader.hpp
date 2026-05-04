#pragma once

#include <string>

namespace dong {

struct ResourceLoadResult {
    bool success = false;
    std::string content;
    int status_code = 0;    // HTTP status (0 for local file)
    std::string error_msg;
};

// Load a text resource from a URL or file path.
// Handles: absolute paths, resource_root-relative paths, file:// URLs, http(s):// URLs.
ResourceLoadResult loadTextResource(const std::string& url,
                                    const std::string& resource_root);

// Check whether a URL string is an HTTP(S) URL.
bool isHttpUrl(const std::string& url);

} // namespace dong
