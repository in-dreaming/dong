#ifndef DONG_H
#define DONG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DLL export/import macros
// =============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_API __declspec(dllexport)
    #else
        #define DONG_API __declspec(dllimport)
    #endif
#else
    #define DONG_API __attribute__((visibility("default")))
#endif

// =============================================================================
// Dong public C ABI (stable entrypoints)
// =============================================================================

// Increment this when making a breaking ABI change.
#define DONG_API_VERSION 2u

typedef struct dong_engine_t dong_engine_t;

typedef enum dong_result_t {
    DONG_OK = 0,
    DONG_ERR_INVALID_ARG = 1,
    DONG_ERR_VERSION_MISMATCH = 2,
    DONG_ERR_PLUGIN_MISSING_CAP = 3,
    DONG_ERR_NOT_IMPLEMENTED = 4,
    DONG_ERR_INTERNAL = 255,
} dong_result_t;

// Plugin API is a separate C ABI header.
#include "dong_plugin_api.h"

typedef struct dong_engine_desc_t {
    uint32_t api_version;        // must be DONG_API_VERSION
    uint32_t plugin_api_version; // must be DONG_PLUGIN_API_VERSION
    const dong_plugin_vtable_t* plugin;
    void* plugin_user;
    const char* html;            // optional: initial HTML content to load
    uint32_t width;              // window width (0 = default 960)
    uint32_t height;             // window height (0 = default 540)
} dong_engine_desc_t;


// ABI version query
DONG_API uint32_t dong_get_api_version(void);

// Engine lifecycle
// NOTE: In current codebase this is a thin wrapper. Full pluginized behavior will
// be implemented during the refactor (dong.dll must not depend on SDL).
DONG_API dong_result_t dong_engine_create(const dong_engine_desc_t* desc, dong_engine_t** out_engine);
DONG_API void dong_engine_destroy(dong_engine_t* engine);

// Drive one tick/frame (update + render).
// For headless tests this may be a no-op depending on configuration.
DONG_API dong_result_t dong_engine_tick(dong_engine_t* engine);

// Load HTML content into the engine.
DONG_API dong_result_t dong_engine_load_html(dong_engine_t* engine, const char* html);

// Resize the engine viewport.
DONG_API dong_result_t dong_engine_resize(dong_engine_t* engine, uint32_t width, uint32_t height);

// Set GPU device and window for rendering (must be called before first tick if using GPU).
// The device and window are owned by the caller (typically the plugin).
DONG_API dong_result_t dong_engine_set_gpu(dong_engine_t* engine, void* gpu_device, void* window);

// Set resource root for resolving relative asset/script paths.
DONG_API dong_result_t dong_engine_set_resource_root(dong_engine_t* engine, const char* root);

// Query cursor at a given position (returns CSS cursor name; valid until next tick).
DONG_API const char* dong_engine_get_cursor_at(dong_engine_t* engine, int32_t x, int32_t y);

// Input event handling
DONG_API dong_result_t dong_engine_send_mouse_move(dong_engine_t* engine, int32_t x, int32_t y);

DONG_API dong_result_t dong_engine_send_mouse_button(dong_engine_t* engine, int32_t button, int pressed);
DONG_API dong_result_t dong_engine_send_mouse_wheel(dong_engine_t* engine, float delta_x, float delta_y);
DONG_API dong_result_t dong_engine_send_key(dong_engine_t* engine, uint32_t key_code, int pressed);
DONG_API dong_result_t dong_engine_send_text(dong_engine_t* engine, const char* text);
DONG_API dong_result_t dong_engine_send_text_editing(dong_engine_t* engine, const char* text, int32_t cursor, int32_t selection_length);

// Script evaluation
DONG_API dong_result_t dong_engine_eval_script(dong_engine_t* engine, const char* code);

// =============================================================================
// Rendering (GPU command list access)
// =============================================================================

// Get the current GPU command list (opaque pointer).
// Returns NULL if no commands have been generated.
// The pointer is valid until the next dong_engine_tick() call.
// External renderers can cast this to GPUCommandList* and execute it.
DONG_API const void* dong_engine_get_command_list(dong_engine_t* engine);

// Force regeneration of GPU commands on next tick.
DONG_API void dong_engine_invalidate_commands(dong_engine_t* engine);

// =============================================================================
// View API (legacy / advanced)
// =============================================================================
// Not included by default.
// If you need the view API, include "dong_view.h" explicitly.


#ifdef __cplusplus
}
#endif

#endif // DONG_H
