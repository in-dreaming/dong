#ifndef DONG_SCENE3D_H
#define DONG_SCENE3D_H

#include "dong_app.h"
#include "dong_overlay.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Scene3D - 3D Scene with HTML Screens
// =============================================================================
// DongScene3D manages a 3D scene containing:
// - Multiple HTML screens positioned in 3D space
// - First-person camera with WASD+mouse controls
// - Ray-based input routing to HTML screens
// - HUD overlays
//
// This simplifies 3D demos from 1500+ lines to ~100 lines.
// =============================================================================

typedef struct dong_scene3d_t dong_scene3d_t;
typedef struct dong_screen3d_t dong_screen3d_t;

// =============================================================================
// Screen Configuration
// =============================================================================

typedef struct dong_screen3d_config_t {
    const char* html_file;      // Path to HTML file (NULL to use html_content)
    const char* html_content;   // Direct HTML content (used if html_file is NULL)
    const char* resource_root;  // Base directory for resolving relative paths (NULL for auto)
    uint32_t width;             // Render texture width (default 800)
    uint32_t height;            // Render texture height (default 600)
    float pos_x, pos_y, pos_z;  // Position in 3D space
    float yaw;                  // Rotation around Y axis (radians)
    float screen_width;         // Physical width in 3D units (default 4.0)
    float screen_height;        // Physical height in 3D units (default 3.0)
} dong_screen3d_config_t;

// =============================================================================
// Scene Lifecycle
// =============================================================================

// Create a 3D scene.
DONG_APPCORE_API dong_scene3d_t* dong_scene3d_create(dong_app_t* app);

// Destroy a 3D scene and all its screens.
DONG_APPCORE_API void dong_scene3d_destroy(dong_scene3d_t* scene);

// =============================================================================
// Screen Management
// =============================================================================

// Add an HTML screen to the scene.
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_add_screen(
    dong_scene3d_t* scene,
    const dong_screen3d_config_t* config
);

// Simplified version: add screen at position with default settings.
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_add_screen_simple(
    dong_scene3d_t* scene,
    const char* html_file,
    uint32_t width, uint32_t height,
    float pos_x, float pos_y, float pos_z
);

// Remove a screen from the scene.
DONG_APPCORE_API void dong_scene3d_remove_screen(dong_scene3d_t* scene, dong_screen3d_t* screen);

// Get screen count.
DONG_APPCORE_API int dong_scene3d_get_screen_count(dong_scene3d_t* scene);

// Get screen by index.
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_get_screen(dong_scene3d_t* scene, int index);

// =============================================================================
// Screen Properties
// =============================================================================

// Set screen position.
DONG_APPCORE_API void dong_screen3d_set_position(dong_screen3d_t* screen, float x, float y, float z);

// Set screen rotation (yaw around Y axis).
DONG_APPCORE_API void dong_screen3d_set_yaw(dong_screen3d_t* screen, float yaw);

// Execute JavaScript on a screen.
DONG_APPCORE_API int dong_screen3d_eval_script(dong_screen3d_t* screen, const char* script);

// Get the dong_view pointer for a screen (for advanced use).
DONG_APPCORE_API void* dong_screen3d_get_view(dong_screen3d_t* screen);

// =============================================================================
// HUD/Overlay
// =============================================================================

// Add an overlay to the scene (renders on top of 3D content).
DONG_APPCORE_API dong_overlay_t* dong_scene3d_add_overlay(
    dong_scene3d_t* scene,
    const char* html_content,
    uint32_t width, uint32_t height
);

// Add an overlay from a file (resolves path relative to resource root).
DONG_APPCORE_API dong_overlay_t* dong_scene3d_add_overlay_file(
    dong_scene3d_t* scene,
    const char* html_file,
    uint32_t width, uint32_t height
);

// Get overlay by index.
DONG_APPCORE_API dong_overlay_t* dong_scene3d_get_overlay(dong_scene3d_t* scene, int index);

// Get overlay count.
DONG_APPCORE_API int dong_scene3d_get_overlay_count(dong_scene3d_t* scene);

// =============================================================================
// Update and Render
// =============================================================================

// Handle input (camera movement, ray casting to screens).
// Returns the screen that received input, or NULL.
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_handle_input(dong_scene3d_t* scene);

// Update all screens and animations.
DONG_APPCORE_API void dong_scene3d_update(dong_scene3d_t* scene, float dt);

// Render the scene (3D screens + HUD overlays).
DONG_APPCORE_API void dong_scene3d_render(dong_scene3d_t* scene);

// =============================================================================
// Camera Control
// =============================================================================

// Get/set camera position.
DONG_APPCORE_API void dong_scene3d_get_camera_position(dong_scene3d_t* scene, float* x, float* y, float* z);
DONG_APPCORE_API void dong_scene3d_set_camera_position(dong_scene3d_t* scene, float x, float y, float z);

// Get/set camera rotation (yaw, pitch in radians).
DONG_APPCORE_API void dong_scene3d_get_camera_rotation(dong_scene3d_t* scene, float* yaw, float* pitch);
DONG_APPCORE_API void dong_scene3d_set_camera_rotation(dong_scene3d_t* scene, float yaw, float pitch);

// Enable/disable automatic camera controls (WASD+mouse, default enabled).
DONG_APPCORE_API void dong_scene3d_set_camera_controls_enabled(dong_scene3d_t* scene, int enabled);

// Set camera movement speed (default 5.0).
DONG_APPCORE_API void dong_scene3d_set_camera_speed(dong_scene3d_t* scene, float speed);

// Set camera mouse sensitivity (default 0.002).
DONG_APPCORE_API void dong_scene3d_set_camera_sensitivity(dong_scene3d_t* scene, float sensitivity);

// =============================================================================
// Scene Configuration
// =============================================================================

// Set background color (RGBA, values 0.0-1.0).
DONG_APPCORE_API void dong_scene3d_set_background_color(dong_scene3d_t* scene, float r, float g, float b, float a);

// Enable/disable depth testing (default enabled).
DONG_APPCORE_API void dong_scene3d_set_depth_test_enabled(dong_scene3d_t* scene, int enabled);

// Set resource root for all screens (base directory for resolving relative paths).
DONG_APPCORE_API void dong_scene3d_set_resource_root(dong_scene3d_t* scene, const char* root);

// =============================================================================
// Automatic Screen Arrangement
// =============================================================================

// Automatically arrange all screens in a grid pattern.
// spacing_x: horizontal spacing between screens (default 4.0)
// spacing_y: vertical spacing between screens (default 2.8)
// max_per_row: maximum screens per row (default 5)
DONG_APPCORE_API void dong_scene3d_arrange_screens(dong_scene3d_t* scene, float spacing_x, float spacing_y, int max_per_row);

#ifdef __cplusplus
}
#endif

#endif // DONG_SCENE3D_H
