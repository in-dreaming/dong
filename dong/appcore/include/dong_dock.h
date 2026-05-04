#ifndef DONG_DOCK_H
#define DONG_DOCK_H

#include "dong_app.h"
#include "dong.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Dock - Multi-Window Docking System
// =============================================================================
// DongDock manages a set of panes (each backed by a dong_engine_t) that can be
// arranged in split layouts within one or more OS windows.  Panes can be
// detached into standalone windows and re-attached via drag or API call.
//
// Architecture:
//   - One SDL_GPUDevice shared across all windows
//   - Each pane renders to its own offscreen texture (dong_engine_tick)
//   - Each OS window composites its panes from a binary split tree
//   - Optional shared-JS mode: multiple panes share one ScriptEngine
//
// Relationship to other AppCore modules:
//   - dong_app:     single-window, single-engine (simple apps)
//   - dong_scene3d: single-window, multi-engine in 3D space
//   - dong_dock:    multi-window, multi-engine in 2D docking layout
// =============================================================================

typedef struct dong_dock_t         dong_dock_t;
typedef struct dong_dock_pane_t    dong_dock_pane_t;
typedef struct dong_dock_window_t  dong_dock_window_t;

// =============================================================================
// Enums
// =============================================================================

// Edge direction for split / attach operations.
typedef enum dong_dock_edge_t {
    DONG_DOCK_LEFT   = 0,
    DONG_DOCK_RIGHT  = 1,
    DONG_DOCK_TOP    = 2,
    DONG_DOCK_BOTTOM = 3,
    DONG_DOCK_TAB    = 4,   // Merge as tab (no spatial split)
} dong_dock_edge_t;

// =============================================================================
// Configuration
// =============================================================================

typedef struct dong_dock_config_t {
    const char* title;              // Default window title
    uint32_t    width;              // Initial window width  (0 = 1280)
    uint32_t    height;             // Initial window height (0 = 720)
    int         vsync;              // Non-zero to enable vsync
    int         enable_hdr;         // Non-zero to request HDR mode
    float       hdr_max_luminance;  // Max luminance in nits (0 = default)
} dong_dock_config_t;

typedef struct dong_dock_pane_config_t {
    const char*    title;           // Tab / window title
    uint32_t       width;           // Render width  (0 = auto from layout)
    uint32_t       height;          // Render height (0 = auto from layout)
    const char*    html;            // Optional: load HTML on creation
    const char*    html_file;       // Optional: load HTML file on creation
    const char*    resource_root;   // Optional: base dir for relative paths
    dong_engine_t* shared_js;       // Optional: share JS context with this engine
    const char*    view_name;       // Optional: register as dong.getView(name)
} dong_dock_pane_config_t;

// =============================================================================
// Dock Lifecycle
// =============================================================================

// Create a dock manager.  Creates the first OS window and GPU device.
// Returns NULL on failure.
DONG_APPCORE_API dong_dock_t* dong_dock_create(const dong_dock_config_t* config);

// Destroy the dock manager, all windows, and all panes.
DONG_APPCORE_API void dong_dock_destroy(dong_dock_t* dock);

// =============================================================================
// Main Loop
// =============================================================================

// Poll and dispatch events for all windows.
// Returns 0 when all windows are closed, 1 otherwise.
DONG_APPCORE_API int dong_dock_poll_events(dong_dock_t* dock);

// Tick all pane engines and composite each window.
DONG_APPCORE_API void dong_dock_render(dong_dock_t* dock);

// Check if any window is still open.
DONG_APPCORE_API int dong_dock_is_running(dong_dock_t* dock);

// Get time elapsed since last frame (seconds).
DONG_APPCORE_API float dong_dock_get_delta_time(dong_dock_t* dock);

// Convenience: blocking main loop.
typedef void (*dong_dock_tick_fn)(dong_dock_t* dock, float dt, void* user_data);
DONG_APPCORE_API void dong_dock_run(dong_dock_t* dock,
                                     dong_dock_tick_fn tick,
                                     void* user_data);

// =============================================================================
// Pane Management
// =============================================================================

// Add the first pane (fills the entire primary window).
// Subsequent panes must use dong_dock_split or dong_dock_attach.
DONG_APPCORE_API dong_dock_pane_t* dong_dock_add_pane(
    dong_dock_t* dock,
    const dong_dock_pane_config_t* config);

// Split an existing pane's area along `edge`, placing the new pane on that side.
// `ratio` controls the split position (0.0-1.0, 0 = default 0.5).
// Returns the newly created pane, or NULL on failure.
DONG_APPCORE_API dong_dock_pane_t* dong_dock_split(
    dong_dock_t* dock,
    dong_dock_pane_t* neighbor,
    dong_dock_edge_t edge,
    float ratio,
    const dong_dock_pane_config_t* config);

// Remove a pane and collapse its split node.
// The sibling pane expands to fill the vacated space.
DONG_APPCORE_API void dong_dock_remove_pane(dong_dock_t* dock, dong_dock_pane_t* pane);

// =============================================================================
// Detach / Attach (Multi-Window)
// =============================================================================

// Detach a pane into a new standalone OS window.
// Returns the new window, or NULL on failure.
DONG_APPCORE_API dong_dock_window_t* dong_dock_detach(
    dong_dock_t* dock,
    dong_dock_pane_t* pane,
    int window_x, int window_y,    // Screen position (0,0 = auto)
    uint32_t window_w, uint32_t window_h);  // Size (0 = use pane size)

// Attach a pane next to `target` at `edge`.
// If the pane is in a different window, it is moved and the source window
// is destroyed when it becomes empty.
DONG_APPCORE_API void dong_dock_attach(
    dong_dock_t* dock,
    dong_dock_pane_t* pane,
    dong_dock_pane_t* target,
    dong_dock_edge_t edge);

// =============================================================================
// Pane Properties
// =============================================================================

// Get the dong_engine_t backing a pane (for advanced use: eval_script, etc.).
DONG_APPCORE_API dong_engine_t* dong_dock_pane_get_engine(dong_dock_pane_t* pane);

// Load HTML content into a pane.
DONG_APPCORE_API int dong_dock_pane_load_html(dong_dock_pane_t* pane, const char* html);

// Load HTML from a file path.
DONG_APPCORE_API int dong_dock_pane_load_html_file(dong_dock_pane_t* pane, const char* path);

// Set pane title (shown in tab bar).
DONG_APPCORE_API void dong_dock_pane_set_title(dong_dock_pane_t* pane, const char* title);

// Get pane title.
DONG_APPCORE_API const char* dong_dock_pane_get_title(dong_dock_pane_t* pane);

// Set base directory for resolving relative resource paths.
DONG_APPCORE_API void dong_dock_pane_set_resource_root(dong_dock_pane_t* pane, const char* root);

// Execute JavaScript in a pane.
DONG_APPCORE_API int dong_dock_pane_eval_script(dong_dock_pane_t* pane, const char* script);

// =============================================================================
// Split Ratio Adjustment
// =============================================================================

// Set the split ratio of the split node that contains `pane`.
// Only meaningful if the pane's parent is a split (not root / tab group).
DONG_APPCORE_API void dong_dock_set_split_ratio(
    dong_dock_t* dock,
    dong_dock_pane_t* pane,
    float ratio);

// =============================================================================
// Window Access
// =============================================================================

// Get the number of open dock windows.
DONG_APPCORE_API int dong_dock_get_window_count(dong_dock_t* dock);

// Get a dock window by index.
DONG_APPCORE_API dong_dock_window_t* dong_dock_get_window(dong_dock_t* dock, int index);

// Get the primary (first-created) window.
DONG_APPCORE_API dong_dock_window_t* dong_dock_get_primary_window(dong_dock_t* dock);

// Get the OS window handle (SDL_Window* cast to void*).
DONG_APPCORE_API void* dong_dock_window_get_native(dong_dock_window_t* win);

// =============================================================================
// Layout Persistence
// =============================================================================

// Save current layout to a JSON file.
// Captures: split tree structure, ratios, pane titles, window positions/sizes.
// Does NOT capture HTML content (caller must re-load on restore).
DONG_APPCORE_API int dong_dock_save_layout(dong_dock_t* dock, const char* path);

// Restore layout from a JSON file.
// Creates windows and panes according to the saved structure.
// Fires the `pane_created` callback for each pane so the caller can load content.
typedef void (*dong_dock_pane_created_fn)(dong_dock_pane_t* pane,
                                           const char* title,
                                           void* user_data);
DONG_APPCORE_API int dong_dock_load_layout(dong_dock_t* dock,
                                            const char* path,
                                            dong_dock_pane_created_fn on_pane,
                                            void* user_data);

// =============================================================================
// Callbacks
// =============================================================================

// Called when a pane is about to be closed (user clicked X on the tab).
// Return non-zero to allow close, 0 to veto.
typedef int (*dong_dock_pane_close_fn)(dong_dock_pane_t* pane, void* user_data);
DONG_APPCORE_API void dong_dock_set_pane_close_callback(
    dong_dock_t* dock,
    dong_dock_pane_close_fn callback,
    void* user_data);

// Called when a window is about to be closed (user clicked OS close button).
// Return non-zero to allow close, 0 to veto.
// If the last window closes, dong_dock_is_running returns 0.
typedef int (*dong_dock_window_close_fn)(dong_dock_window_t* win, void* user_data);
DONG_APPCORE_API void dong_dock_set_window_close_callback(
    dong_dock_t* dock,
    dong_dock_window_close_fn callback,
    void* user_data);

#ifdef __cplusplus
}
#endif

#endif // DONG_DOCK_H
