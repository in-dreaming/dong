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

    FILE* out = stderr;
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
// Platform Implementation
// =============================================================================

typedef struct DongPlatformImpl {
    DongGPUDriver* gpu_driver;
    DongSurfaceFactory* surface_factory;
    DongFileSystem* file_system;
    DongLogger* logger;
    DongImageDecoder* image_decoder;
    DongClipboard* clipboard;
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

#ifdef __cplusplus
}
#endif
