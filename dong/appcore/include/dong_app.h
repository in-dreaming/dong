#ifndef DONG_APP_H
#define DONG_APP_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DLL export/import macros
// =============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_APPCORE_BUILDING_DLL
        #define DONG_APPCORE_API __declspec(dllexport)
    #else
        #define DONG_APPCORE_API __declspec(dllimport)
    #endif
#else
    #define DONG_APPCORE_API __attribute__((visibility("default")))
#endif

// =============================================================================
// AppCore - Application Framework
// =============================================================================
// DongApp provides a high-level application framework that manages:
// - Window creation and event handling
// - GPU device initialization
// - Main loop and frame timing
// - Dong engine integration
//
// This simplifies application development from 1500+ lines to ~50 lines.
// =============================================================================

// Forward declarations
typedef struct dong_app_t dong_app_t;
typedef struct dong_renderer_t dong_renderer_t;

// =============================================================================
// AppCore input event abstraction
// =============================================================================
// AppCore must not expose SDL types to its users.

typedef enum dong_app_event_type_t {
    DONG_APP_EVENT_NONE = 0,
    DONG_APP_EVENT_QUIT,
    DONG_APP_EVENT_WINDOW_RESIZED,
    DONG_APP_EVENT_MOUSE_MOVE,
    DONG_APP_EVENT_MOUSE_BUTTON,
    DONG_APP_EVENT_MOUSE_WHEEL,
    DONG_APP_EVENT_KEY,
    DONG_APP_EVENT_TEXT,
} dong_app_event_type_t;

typedef struct dong_app_event_t {
    dong_app_event_type_t type;

    union {
        struct {
            uint32_t width;
            uint32_t height;
        } window_resized;

        struct {
            int32_t x;
            int32_t y;
        } mouse_move;

        struct {
            int32_t button;
            int pressed;
            int32_t x;
            int32_t y;
        } mouse_button;


        struct {
            float delta_x;
            float delta_y;
        } mouse_wheel;

        struct {
            // SDL virtual keycode (layout-dependent), e.g. SDLK_W
            uint32_t key_code;
            // SDL physical scancode (layout-independent), e.g. SDL_SCANCODE_W
            uint32_t scancode;
            int pressed;
            int repeat;
        } key;

        struct {
            const char* text;
        } text;
    };
} dong_app_event_t;

// Event callback for input handling (allows 3D scene to receive input events)
typedef void (*dong_app_event_callback_t)(void* user_data, const dong_app_event_t* event);


// =============================================================================
// Application Configuration
// =============================================================================

typedef struct dong_app_config_t {
    const char* title;          // Window title
    uint32_t width;             // Initial window width (0 = default 800)
    uint32_t height;            // Initial window height (0 = default 600)
    int resizable;              // Non-zero to allow window resize
    int enable_dong;            // Non-zero to enable HTML rendering
    int vsync;                  // Non-zero to enable vsync
    int fullscreen;             // Non-zero for fullscreen mode
    int enable_hdr;             // Non-zero to request HDR mode (HDR Extended Linear)
    float hdr_max_luminance;    // Max luminance in nits (0 = default 1000)
} dong_app_config_t;

// =============================================================================
// Application Lifecycle
// =============================================================================

// Create a new application with the given configuration.
// Returns NULL on failure.
DONG_APPCORE_API dong_app_t* dong_app_create(const dong_app_config_t* config);

// Destroy an application and release all resources.
DONG_APPCORE_API void dong_app_destroy(dong_app_t* app);

// Check if the application should continue running.
// Returns 0 when user closes window or requests quit.
DONG_APPCORE_API int dong_app_is_running(dong_app_t* app);

// Request the application to quit.
DONG_APPCORE_API void dong_app_quit(dong_app_t* app);

// =============================================================================
// Main Loop (Blocking)
// =============================================================================

// Run the application main loop until quit.
// Calls the user's tick callback each frame.
// Returns when application exits.
typedef void (*dong_app_tick_fn)(dong_app_t* app, float dt, void* user_data);
DONG_APPCORE_API void dong_app_run(dong_app_t* app, dong_app_tick_fn tick, void* user_data);

// =============================================================================
// Frame Control (Non-Blocking)
// =============================================================================

// Poll and process all pending events.
// Returns 0 if quit was requested, 1 otherwise.
DONG_APPCORE_API int dong_app_poll_events(dong_app_t* app);

// Get time elapsed since last frame (in seconds).
DONG_APPCORE_API float dong_app_get_delta_time(dong_app_t* app);

// Begin a new frame (prepare for rendering).
DONG_APPCORE_API int dong_app_begin_frame(dong_app_t* app);

// End the current frame and present to screen.
DONG_APPCORE_API void dong_app_present(dong_app_t* app);

// =============================================================================
// Window Information
// =============================================================================

// Get current window dimensions.
DONG_APPCORE_API void dong_app_get_size(dong_app_t* app, uint32_t* out_width, uint32_t* out_height);

// Get current mouse position.
DONG_APPCORE_API void dong_app_get_mouse_position(dong_app_t* app, int32_t* out_x, int32_t* out_y);

// =============================================================================
// Internal Handle Access
// =============================================================================

// Get the internal renderer handle (for advanced use).
DONG_APPCORE_API dong_renderer_t* dong_app_get_renderer(dong_app_t* app);

// Get the native SDL_GPUDevice pointer (cast from void*).
DONG_APPCORE_API void* dong_app_get_gpu_device(dong_app_t* app);

// Get the native SDL_Window pointer (cast from void*).
DONG_APPCORE_API void* dong_app_get_window(dong_app_t* app);

// Get the dong_engine_t pointer (if enable_dong was set).
DONG_APPCORE_API void* dong_app_get_dong_engine(dong_app_t* app);

// Check if HDR mode is active.
// Returns non-zero if HDR is enabled, 0 if running in SDR mode.
DONG_APPCORE_API int dong_app_is_hdr_enabled(dong_app_t* app);

// Get the maximum HDR luminance in nits.
// Returns 0 if HDR is not enabled.
DONG_APPCORE_API float dong_app_get_hdr_max_luminance(dong_app_t* app);

// =============================================================================
// HTML Content Management
// =============================================================================

// Load HTML content into the application.
// This is a convenience wrapper that handles internal view/engine management.
DONG_APPCORE_API int dong_app_load_html(dong_app_t* app, const char* html);

// Load HTML from a file path.
DONG_APPCORE_API int dong_app_load_html_file(dong_app_t* app, const char* path);

// =============================================================================
// Input Management
// =============================================================================

// Enable or disable text input mode.
// Required for receiving SDL_EVENT_TEXT_INPUT events (for input fields).
DONG_APPCORE_API void dong_app_enable_text_input(dong_app_t* app, int enable);

// Set event callback for input handling.
// This allows external code (e.g., 3D scene) to receive SDL input events.
DONG_APPCORE_API void dong_app_set_event_callback(dong_app_t* app, dong_app_event_callback_t callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif // DONG_APP_H
