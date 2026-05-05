/**
 * @file dong_world_overlay.h
 * @brief World-space HTML overlay API (P2-3)
 *
 * Renders a dong HTML view onto a 3D world-space quad.
 * The quad has position, orientation, and size in world coordinates.
 * Supports occlusion (can be hidden behind world geometry) and
 * interactive input forwarding.
 *
 * Typical use cases:
 * - Interactive 3D terminals / computer screens in game world
 * - Floating information panels (NPC dialogue, quest info)
 * - In-world billboards and advertisements
 * - Holographic UI displays
 *
 * Architecture:
 *   Each world overlay has its own dong engine view (separate DOM/layout/render).
 *   The host engine receives the rendered texture coordinates + world transform
 *   via DONG_DRAW_CMD_WORLD_OVERLAY in the draw list, and composites it into
 *   the 3D scene with proper depth testing.
 *
 * Usage:
 *   dong_world_overlay_t* ov = dong_world_overlay_create(engine, 512, 256);
 *   dong_world_overlay_load_html(ov, "<div>Hello 3D World</div>");
 *   dong_world_overlay_set_position(ov, 0.0f, 2.0f, -5.0f);
 *   dong_world_overlay_set_rotation(ov, 0.0f, 45.0f, 0.0f);  // face player
 *   dong_world_overlay_set_size(ov, 2.0f, 1.0f);  // 2m x 1m in world
 *   // ... host receives DONG_DRAW_CMD_WORLD_OVERLAY with texture + transform
 *   dong_world_overlay_destroy(ov);
 */

#ifndef DONG_WORLD_OVERLAY_H
#define DONG_WORLD_OVERLAY_H

#include "dong.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dong_world_overlay dong_world_overlay_t;

/**
 * Create a world-space overlay with an internal render target.
 * @param engine Parent engine instance
 * @param render_width Texture width in pixels (internal resolution)
 * @param render_height Texture height in pixels (internal resolution)
 */
DONG_API dong_world_overlay_t* dong_world_overlay_create(dong_engine_t* engine,
                                                          uint32_t render_width,
                                                          uint32_t render_height);

/** Destroy a world overlay and free its render target. */
DONG_API void dong_world_overlay_destroy(dong_world_overlay_t* overlay);

/** Load HTML content into the overlay's internal view. */
DONG_API void dong_world_overlay_load_html(dong_world_overlay_t* overlay, const char* html);

/** Set the 3D world position of the overlay center. */
DONG_API void dong_world_overlay_set_position(dong_world_overlay_t* overlay, float x, float y, float z);

/** Set the rotation in degrees (Euler angles: pitch, yaw, roll). */
DONG_API void dong_world_overlay_set_rotation(dong_world_overlay_t* overlay,
                                               float pitch_deg, float yaw_deg, float roll_deg);

/** Set the world-space size of the overlay quad (meters). */
DONG_API void dong_world_overlay_set_size(dong_world_overlay_t* overlay, float width, float height);

/** Set visibility. */
DONG_API void dong_world_overlay_set_visible(dong_world_overlay_t* overlay, int visible);

/** Enable/disable occlusion testing (default: enabled).
 *  When enabled, the overlay can be hidden by world geometry. */
DONG_API void dong_world_overlay_set_occlusion(dong_world_overlay_t* overlay, int enable);

/** Enable/disable interactive input (default: disabled).
 *  When enabled, host can forward raycasted mouse events to the overlay. */
DONG_API void dong_world_overlay_set_interactive(dong_world_overlay_t* overlay, int enable);

/** Forward a mouse event to the overlay (coordinates in overlay-local pixels).
 *  Only meaningful when interactive mode is enabled. */
DONG_API void dong_world_overlay_send_mouse(dong_world_overlay_t* overlay,
                                             int32_t local_x, int32_t local_y,
                                             int32_t button, int pressed);

/** Tick the overlay's internal view (call once per frame). */
DONG_API void dong_world_overlay_tick(dong_world_overlay_t* overlay);

/** Get the overlay's render width. */
DONG_API uint32_t dong_world_overlay_get_width(dong_world_overlay_t* overlay);

/** Get the overlay's render height. */
DONG_API uint32_t dong_world_overlay_get_height(dong_world_overlay_t* overlay);

#ifdef __cplusplus
}
#endif

#endif /* DONG_WORLD_OVERLAY_H */
