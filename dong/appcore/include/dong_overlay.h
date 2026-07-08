#ifndef DONG_OVERLAY_H
#define DONG_OVERLAY_H

#include "dong_app.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Overlay - HUD and UI Layer
// =============================================================================
// DongOverlay provides a simple way to add HTML-based HUD/UI overlays
// that render on top of the scene.
//
// Features:
// - Automatic input forwarding when mouse is over the overlay
// - Transparent background support
// - Easy HTML content loading
// =============================================================================

typedef struct dong_overlay_t dong_overlay_t;

// =============================================================================
// Overlay Lifecycle
// =============================================================================

// Create an overlay with the given dimensions.
// width/height: Size of the overlay texture (0 = match window)
// Returns NULL on failure.
DONG_APPCORE_API dong_overlay_t* dong_overlay_create(dong_app_t* app, uint32_t width, uint32_t height);

// Destroy an overlay and release resources.
DONG_APPCORE_API void dong_overlay_destroy(dong_overlay_t* overlay);

// =============================================================================
// Content Management
// =============================================================================

// Load HTML content into the overlay.
DONG_APPCORE_API int dong_overlay_load_html(dong_overlay_t* overlay, const char* html);

// Load HTML from a file path.
DONG_APPCORE_API int dong_overlay_load_file(dong_overlay_t* overlay, const char* file_path);

// Set the base directory for resolving relative resource paths.
DONG_APPCORE_API void dong_overlay_set_resource_root(dong_overlay_t* overlay, const char* root);

// Execute JavaScript in the overlay.
DONG_APPCORE_API int dong_overlay_eval_script(dong_overlay_t* overlay, const char* script);

// Call a compiled Porffor export in the overlay.
DONG_APPCORE_API int dong_overlay_call_porffor_export(
    dong_overlay_t* overlay, const char* module_name, const char* export_name);

// Call a compiled Porffor export in the overlay with one numeric argument.
DONG_APPCORE_API int dong_overlay_call_porffor_export1(
    dong_overlay_t* overlay, const char* module_name, const char* export_name, double arg0);

// =============================================================================
// Rendering
// =============================================================================

// Update the overlay (process events, layout, etc.).
// Should be called each frame before rendering.
DONG_APPCORE_API void dong_overlay_update(dong_overlay_t* overlay, float dt);

// Render the overlay to the current render target.
// Call this after rendering the main scene.
DONG_APPCORE_API void dong_overlay_render(dong_overlay_t* overlay);

// =============================================================================
// Input Handling
// =============================================================================

// Check if a point is over the overlay (for input routing).
// Returns non-zero if the point hits non-transparent content.
DONG_APPCORE_API int dong_overlay_hit_test(dong_overlay_t* overlay, int32_t x, int32_t y);

// Forward input events to the overlay.
// These are called automatically by dong_app when overlay is active.
DONG_APPCORE_API void dong_overlay_send_mouse_move(dong_overlay_t* overlay, int32_t x, int32_t y);
DONG_APPCORE_API void dong_overlay_send_mouse_button(dong_overlay_t* overlay, int32_t button, int pressed);
DONG_APPCORE_API void dong_overlay_send_mouse_wheel(dong_overlay_t* overlay, float delta_x, float delta_y);
DONG_APPCORE_API void dong_overlay_send_key(dong_overlay_t* overlay, uint32_t key_code, int pressed);
DONG_APPCORE_API void dong_overlay_send_text(dong_overlay_t* overlay, const char* text);
DONG_APPCORE_API void dong_overlay_send_text_editing(dong_overlay_t* overlay,
                                                      const char* text,
                                                      int32_t cursor,
                                                      int32_t selection_length);

// =============================================================================
// Configuration
// =============================================================================

// Set overlay position (default is 0,0 - top-left corner).
DONG_APPCORE_API void dong_overlay_set_position(dong_overlay_t* overlay, int32_t x, int32_t y);

// Set overlay opacity (0.0 - 1.0, default is 1.0).
DONG_APPCORE_API void dong_overlay_set_opacity(dong_overlay_t* overlay, float opacity);

// Enable/disable the overlay (disabled overlays don't render or receive input).
DONG_APPCORE_API void dong_overlay_set_enabled(dong_overlay_t* overlay, int enabled);

// Get overlay texture (for custom rendering).
// Returns SDL_GPUTexture* cast to void*.
DONG_APPCORE_API void* dong_overlay_get_texture(dong_overlay_t* overlay);

#ifdef __cplusplus
}
#endif

#endif // DONG_OVERLAY_H
