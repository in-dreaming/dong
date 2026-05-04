#include "resource_loader.hpp"
#include "log.h"
#include "dong_platform.h"

#include <filesystem>
#include <cctype>

namespace dong {

bool isHttpUrl(const std::string& url) {
    return url.rfind("http://", 0) == 0 || url.rfind("https://", 0) == 0;
}

static bool isAbsolutePath(const std::string& p) {
    if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':')
        return true;
    if (!p.empty() && (p[0] == '/' || p[0] == '\\'))
        return true;
    return false;
}

static ResourceLoadResult loadFromFileSystem(const std::string& path,
                                             const std::string& resource_root) {
    ResourceLoadResult result;

    std::string resolved = path;
    if (!resource_root.empty() && !isAbsolutePath(path)) {
        try {
            namespace fs = std::filesystem;
            resolved = (fs::path(resource_root) / fs::path(path)).lexically_normal().string();
        } catch (...) {
            resolved = path;
        }
    }

    DongPlatform* platform = dong_platform_get();
    DongFileSystem* fs = platform ? dong_platform_get_file_system(platform) : nullptr;
    if (!fs) {
        result.error_msg = "No filesystem available";
        return result;
    }

    DongFileData data{};
    DongFileSystemResult r = dong_fs_read_all(fs, resolved.c_str(), &data);
    if (r != DONG_FS_OK || !data.data || data.size == 0) {
        dong_fs_free_data(fs, &data);
        result.error_msg = "File not found or empty: " + resolved;
        return result;
    }

    result.success = true;
    result.content.assign(static_cast<const char*>(data.data), data.size);
    dong_fs_free_data(fs, &data);
    return result;
}

static ResourceLoadResult loadFromHttp(const std::string& url) {
    ResourceLoadResult result;

    DongPlatform* platform = dong_platform_get();
    DongHttpClient* http = platform ? dong_platform_get_http_client(platform) : nullptr;
    if (!http) {
        result.error_msg = "No HTTP client available";
        return result;
    }

    DongHttpResponse resp{};
    DongHttpResult hr = dong_http_get(http, url.c_str(), &resp);
    if (hr != DONG_HTTP_OK) {
        dong_http_free_response(http, &resp);
        result.error_msg = "HTTP request failed for: " + url;
        return result;
    }

    result.status_code = resp.status_code;
    if (resp.status_code >= 200 && resp.status_code < 300 && resp.body && resp.body_size > 0) {
        result.success = true;
        result.content.assign(static_cast<const char*>(resp.body), resp.body_size);
    } else {
        result.error_msg = "HTTP " + std::to_string(resp.status_code) + " for: " + url;
    }

    dong_http_free_response(http, &resp);
    return result;
}

ResourceLoadResult loadTextResource(const std::string& url,
                                    const std::string& resource_root) {
    if (url.empty()) {
        ResourceLoadResult r;
        r.error_msg = "Empty URL";
        return r;
    }

    if (isHttpUrl(url)) {
        return loadFromHttp(url);
    }

    // Strip file:// prefix if present
    std::string path = url;
    if (path.rfind("file://", 0) == 0) {
        path = path.substr(7);
    }

    return loadFromFileSystem(path, resource_root);
}

} // namespace dong
