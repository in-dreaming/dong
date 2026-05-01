// Platform singleton implementation
// This file provides the global Platform instance for dependency injection.

#include "dong_platform.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Override the macro for this implementation file
#undef DONG_PLATFORM_API
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_PLATFORM_API __declspec(dllexport)
    #else
        #define DONG_PLATFORM_API
    #endif
#else
    #define DONG_PLATFORM_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Default FileSystem
// =============================================================================

static int default_fs_exists(DongFileSystem* fs, const char* path) {
    (void)fs;
    if (!path || !path[0]) {
        return 0;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

static long default_fs_file_size(FILE* f) {
    if (!f) {
        return -1;
    }
    long cur = ftell(f);
    if (cur < 0) {
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        return -1;
    }
    long size = ftell(f);
    (void)fseek(f, cur, SEEK_SET);
    return size;
}

static DongFileSystemResult default_fs_read_all(DongFileSystem* fs, const char* path, DongFileData* out_data) {
    (void)fs;
    if (!out_data) {
        return DONG_FS_ERR_INVALID_ARG;
    }
    out_data->data = NULL;
    out_data->size = 0;

    if (!path || !path[0]) {
        return DONG_FS_ERR_INVALID_ARG;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        return DONG_FS_ERR_NOT_FOUND;
    }

    long size = default_fs_file_size(f);
    if (size < 0) {
        fclose(f);
        return DONG_FS_ERR_IO;
    }

    if (size == 0) {
        fclose(f);
        return DONG_FS_OK;
    }

    void* buf = malloc((size_t)size);
    if (!buf) {
        fclose(f);
        return DONG_FS_ERR_IO;
    }

    size_t nread = fread(buf, 1, (size_t)size, f);
    fclose(f);

    if (nread != (size_t)size) {
        free(buf);
        return DONG_FS_ERR_IO;
    }

    out_data->data = buf;
    out_data->size = (size_t)size;
    return DONG_FS_OK;
}

static void default_fs_free_data(DongFileSystem* fs, DongFileData* data) {
    (void)fs;
    if (!data || !data->data) {
        return;
    }
    free(data->data);
    data->data = NULL;
    data->size = 0;
}

static const DongFileSystemVTable g_default_fs_vtable = {
    .exists = default_fs_exists,
    .read_all = default_fs_read_all,
    .free_data = default_fs_free_data,
};

static DongFileSystem g_default_fs = {
    .vtable = &g_default_fs_vtable,
    .user_data = NULL,
};

// =============================================================================
// Default Logger
// =============================================================================

static const char* default_logger_level_prefix(DongLoggerLevel level) {
    switch (level) {
        case DONG_LOGGER_LEVEL_TRACE: return "[TRACE] ";
        case DONG_LOGGER_LEVEL_DEBUG: return "[DEBUG] ";
        case DONG_LOGGER_LEVEL_INFO:  return "[INFO] ";
        case DONG_LOGGER_LEVEL_WARN:  return "[WARN] ";
        case DONG_LOGGER_LEVEL_ERROR: return "[ERROR] ";
        default: return "[LOG] ";
    }
}

static void default_logger_log(DongLogger* logger, DongLoggerLevel level, const char* message) {
    (void)logger;
    if (!message) {
        return;
    }

    /* Respect DONG_LOG_TO_STDOUT env var (P0-7 bench capture support) */
    FILE* out = stderr;
    const char* to_stdout = getenv("DONG_LOG_TO_STDOUT");
    if (to_stdout && (to_stdout[0] == '1' || to_stdout[0] == 'y' || to_stdout[0] == 'Y')) {
        out = stdout;
    }
    fprintf(out, "%s%s\n", default_logger_level_prefix(level), message);
    fflush(out);
}

static const DongLoggerVTable g_default_logger_vtable = {
    .log = default_logger_log,
};

static DongLogger g_default_logger = {
    .vtable = &g_default_logger_vtable,
    .user_data = NULL,
};

// =============================================================================
// Default HTTP Client
// =============================================================================

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winhttp.h>

static DongHttpResult default_http_get(DongHttpClient* client, const char* url,
                                       DongHttpResponse* out) {
    (void)client;
    if (!url || !out) return DONG_HTTP_ERR_INVALID_URL;

    // Convert URL to wide string
    int wlen = MultiByteToWideChar(CP_UTF8, 0, url, -1, NULL, 0);
    if (wlen <= 0) return DONG_HTTP_ERR_INVALID_URL;
    wchar_t* wurl = (wchar_t*)malloc(wlen * sizeof(wchar_t));
    if (!wurl) return DONG_HTTP_ERR_NETWORK;
    MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, wlen);

    // Crack URL
    URL_COMPONENTS uc;
    memset(&uc, 0, sizeof(uc));
    uc.dwStructSize = sizeof(uc);
    wchar_t host_buf[256] = {0};
    wchar_t path_buf[2048] = {0};
    uc.lpszHostName = host_buf;
    uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path_buf;
    uc.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(wurl, 0, 0, &uc)) {
        free(wurl);
        return DONG_HTTP_ERR_INVALID_URL;
    }
    free(wurl);

    HINTERNET session = WinHttpOpen(L"Dong/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) return DONG_HTTP_ERR_NETWORK;

    HINTERNET conn = WinHttpConnect(session, host_buf, uc.nPort, 0);
    if (!conn) { WinHttpCloseHandle(session); return DONG_HTTP_ERR_NETWORK; }

    DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET req = WinHttpOpenRequest(conn, L"GET", path_buf, NULL,
                                       WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!req) { WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return DONG_HTTP_ERR_NETWORK; }

    if (!WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(req, NULL)) {
        WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session);
        return DONG_HTTP_ERR_NETWORK;
    }

    // Read status code
    DWORD status = 0, sz = sizeof(status);
    WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
    out->status_code = (int)status;

    // Read body
    size_t total = 0, cap = 4096;
    char* buf = (char*)malloc(cap);
    if (!buf) { WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return DONG_HTTP_ERR_NETWORK; }
    for (;;) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(req, &avail) || avail == 0) break;
        if (total + avail + 1 > cap) {
            cap = (total + avail + 1) * 2;
            char* tmp = (char*)realloc(buf, cap);
            if (!tmp) { free(buf); WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return DONG_HTTP_ERR_NETWORK; }
            buf = tmp;
        }
        DWORD nread = 0;
        WinHttpReadData(req, buf + total, avail, &nread);
        total += nread;
    }
    buf[total] = '\0';

    out->body = buf;
    out->body_size = total;

    WinHttpCloseHandle(req);
    WinHttpCloseHandle(conn);
    WinHttpCloseHandle(session);
    return DONG_HTTP_OK;
}

static void default_http_free_response(DongHttpClient* client, DongHttpResponse* resp) {
    (void)client;
    if (resp && resp->body) { free(resp->body); resp->body = NULL; resp->body_size = 0; }
}

static const DongHttpClientVTable g_default_http_vtable = {
    .get = default_http_get,
    .free_response = default_http_free_response,
};

static DongHttpClient g_default_http = {
    .vtable = &g_default_http_vtable,
    .user_data = NULL,
};

#else
// Non-Windows: stub that returns UNSUPPORTED.
// Applications should inject their own HTTP client via dong_platform_set_http_client().
static DongHttpResult stub_http_get(DongHttpClient* client, const char* url,
                                    DongHttpResponse* out) {
    (void)client; (void)url; (void)out;
    return DONG_HTTP_ERR_UNSUPPORTED;
}
static void stub_http_free_response(DongHttpClient* client, DongHttpResponse* resp) {
    (void)client;
    if (resp && resp->body) { free(resp->body); resp->body = NULL; resp->body_size = 0; }
}
static const DongHttpClientVTable g_default_http_vtable = {
    .get = stub_http_get,
    .free_response = stub_http_free_response,
};
static DongHttpClient g_default_http = {
    .vtable = &g_default_http_vtable,
    .user_data = NULL,
};
#endif

// =============================================================================
// Platform Implementation
// =============================================================================

typedef struct DongPlatformImpl {
    DongGPUDriver* gpu_driver;
    DongSurfaceFactory* surface_factory;
    DongFileSystem* file_system;
    DongLogger* logger;
    DongImageDecoder* image_decoder;
    DongClipboard* clipboard;
    DongHttpClient* http_client;
} DongPlatformImpl;

// Global singleton storage
static DongPlatformImpl g_platform_instance = {0};
static int g_platform_initialized = 0;

// Cast impl to opaque type
static inline DongPlatform* impl_to_platform(DongPlatformImpl* impl) {
    return (DongPlatform*)impl;
}

static inline DongPlatformImpl* platform_to_impl(DongPlatform* platform) {
    return (DongPlatformImpl*)platform;
}

static void platform_init_defaults(DongPlatformImpl* impl) {
    if (!impl) {
        return;
    }
    impl->file_system = &g_default_fs;
    impl->logger = &g_default_logger;
    impl->http_client = &g_default_http;
}

// =============================================================================
// Public API
// =============================================================================

DONG_PLATFORM_API DongPlatform* dong_platform_get(void) {
    if (!g_platform_initialized) {
        memset(&g_platform_instance, 0, sizeof(g_platform_instance));
        platform_init_defaults(&g_platform_instance);
        g_platform_initialized = 1;
    }
    return impl_to_platform(&g_platform_instance);
}

DONG_PLATFORM_API void dong_platform_reset(void) {
    memset(&g_platform_instance, 0, sizeof(g_platform_instance));
    platform_init_defaults(&g_platform_instance);
    g_platform_initialized = 0;
}

DONG_PLATFORM_API void dong_platform_set_gpu_driver(DongPlatform* platform, DongGPUDriver* driver) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->gpu_driver = driver;
}

DONG_PLATFORM_API void dong_platform_set_surface_factory(DongPlatform* platform, DongSurfaceFactory* factory) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->surface_factory = factory;
}

DONG_PLATFORM_API void dong_platform_set_file_system(DongPlatform* platform, DongFileSystem* fs) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->file_system = fs ? fs : &g_default_fs;
}

DONG_PLATFORM_API void dong_platform_set_logger(DongPlatform* platform, DongLogger* logger) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->logger = logger ? logger : &g_default_logger;
}

DONG_PLATFORM_API void dong_platform_set_image_decoder(DongPlatform* platform, DongImageDecoder* decoder) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->image_decoder = decoder;
}

DONG_PLATFORM_API DongGPUDriver* dong_platform_get_gpu_driver(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->gpu_driver;
}

DONG_PLATFORM_API DongSurfaceFactory* dong_platform_get_surface_factory(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->surface_factory;
}

DONG_PLATFORM_API DongFileSystem* dong_platform_get_file_system(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->file_system;
}

DONG_PLATFORM_API DongLogger* dong_platform_get_logger(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->logger;
}

DONG_PLATFORM_API DongImageDecoder* dong_platform_get_image_decoder(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->image_decoder;
}

DONG_PLATFORM_API void dong_platform_set_clipboard(DongPlatform* platform, DongClipboard* clipboard) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->clipboard = clipboard;
}

DONG_PLATFORM_API DongClipboard* dong_platform_get_clipboard(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->clipboard;
}

DONG_PLATFORM_API void dong_platform_set_http_client(DongPlatform* platform, DongHttpClient* client) {
    if (!platform) return;
    DongPlatformImpl* impl = platform_to_impl(platform);
    impl->http_client = client ? client : &g_default_http;
}

DONG_PLATFORM_API DongHttpClient* dong_platform_get_http_client(DongPlatform* platform) {
    if (!platform) return NULL;
    DongPlatformImpl* impl = platform_to_impl(platform);
    return impl->http_client;
}

#ifdef __cplusplus
}
#endif
