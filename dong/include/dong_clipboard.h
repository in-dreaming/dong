#ifndef DONG_CLIPBOARD_H
#define DONG_CLIPBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Dong Clipboard (Platform-injected)
// =============================================================================
// The core engine should not access OS clipboard APIs directly.
// Applications/backends can inject a clipboard via dong_platform_set_clipboard().

typedef struct DongClipboard DongClipboard;

typedef struct DongClipboardVTable {
    // Get clipboard text. Returns a malloc'd string (caller must free), or NULL.
    char* (*get_text)(DongClipboard* clipboard);

    // Set clipboard text. Returns 1 on success, 0 on failure.
    int (*set_text)(DongClipboard* clipboard, const char* text);

    // Check if clipboard has text content.
    int (*has_text)(DongClipboard* clipboard);
} DongClipboardVTable;

struct DongClipboard {
    const DongClipboardVTable* vtable;
    void* user_data;
};

static inline char* dong_clipboard_get_text(DongClipboard* cb) {
    return (cb && cb->vtable && cb->vtable->get_text) ? cb->vtable->get_text(cb) : NULL;
}

static inline int dong_clipboard_set_text(DongClipboard* cb, const char* text) {
    return (cb && cb->vtable && cb->vtable->set_text) ? cb->vtable->set_text(cb, text) : 0;
}

static inline int dong_clipboard_has_text(DongClipboard* cb) {
    return (cb && cb->vtable && cb->vtable->has_text) ? cb->vtable->has_text(cb) : 0;
}

#ifdef __cplusplus
}
#endif

#endif // DONG_CLIPBOARD_H
