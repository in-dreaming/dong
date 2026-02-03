#ifndef DONG_FILE_SYSTEM_H
#define DONG_FILE_SYSTEM_H

#include <stddef.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Dong FileSystem (Platform-injected)
// =============================================================================
// The core engine should not access OS file APIs directly.
// Applications/backends can inject a filesystem via dong_platform_set_file_system().
//
// If no filesystem is injected, the Platform provides a default filesystem.

typedef enum DongFileSystemResult {
    DONG_FS_OK = 0,
    DONG_FS_ERR_NOT_FOUND = 1,
    DONG_FS_ERR_IO = 2,
    DONG_FS_ERR_UNSUPPORTED = 3,
    DONG_FS_ERR_INVALID_ARG = 4,
} DongFileSystemResult;

typedef struct DongFileData {
    void* data;
    size_t size;
} DongFileData;

typedef struct DongFileSystem DongFileSystem;

typedef struct DongFileSystemVTable {
    int (*exists)(DongFileSystem* fs, const char* path);

    // Reads the full file into memory.
    // On success: returns DONG_FS_OK and fills out_data.
    // The returned buffer must be released via free_data().
    DongFileSystemResult (*read_all)(DongFileSystem* fs, const char* path, DongFileData* out_data);

    // Frees a buffer returned by read_all().
    // If NULL, the engine will fall back to free(out_data->data).
    void (*free_data)(DongFileSystem* fs, DongFileData* data);
} DongFileSystemVTable;

struct DongFileSystem {
    const DongFileSystemVTable* vtable;
    void* user_data;
};

static inline int dong_fs_exists(DongFileSystem* fs, const char* path) {
    return (fs && fs->vtable && fs->vtable->exists) ? fs->vtable->exists(fs, path) : 0;
}

static inline DongFileSystemResult dong_fs_read_all(DongFileSystem* fs, const char* path, DongFileData* out_data) {
    if (!out_data) {
        return DONG_FS_ERR_INVALID_ARG;
    }
    out_data->data = NULL;
    out_data->size = 0;
    return (fs && fs->vtable && fs->vtable->read_all) ? fs->vtable->read_all(fs, path, out_data) : DONG_FS_ERR_UNSUPPORTED;
}

static inline void dong_fs_free_data(DongFileSystem* fs, DongFileData* data) {
    if (!data || !data->data) {
        return;
    }
    if (fs && fs->vtable && fs->vtable->free_data) {
        fs->vtable->free_data(fs, data);
        return;
    }

    // Fallback: assume malloc/free ownership.
    // NOTE: prefer providing free_data() to avoid CRT boundary issues.
    free(data->data);
    data->data = NULL;
    data->size = 0;
}

#ifdef __cplusplus
}
#endif

#endif // DONG_FILE_SYSTEM_H
