/**
 * @file dong_decal.h
 * @brief World-space UI decal API (P2-2)
 *
 * Projects a 2D UI element (texture or color) onto 3D world geometry.
 * The host engine provides depth+normal information; dong provides the
 * decal parameters through the DongDrawList C ABI.
 *
 * Typical use cases:
 * - Health bars projected onto floors beneath characters
 * - Selection circles / area-of-effect indicators
 * - Contextual UI projected onto surfaces (e.g., interact prompts)
 * - Graffiti / spray-paint systems
 *
 * Architecture:
 *   Dong defines the decal (position, orientation, size, texture, opacity).
 *   The host engine receives DONG_DRAW_CMD_DECAL commands via DongDrawList
 *   and performs the actual projection using its depth/normal buffers.
 *
 * Usage:
 *   dong_decal_t* decal = dong_decal_create(engine);
 *   dong_decal_set_position(decal, 0.0f, 0.01f, -3.0f);  // slightly above ground
 *   dong_decal_set_size(decal, 2.0f, 2.0f);  // 2x2 meter projection
 *   dong_decal_set_color(decal, 1.0f, 0.0f, 0.0f, 0.6f);  // red, 60% opacity
 *   dong_decal_set_texture(decal, "ui/selection_circle.png");
 *   dong_decal_set_normal_threshold(decal, 0.7f);  // only project on mostly-flat surfaces
 *   // ... host receives DONG_DRAW_CMD_DECAL in draw list and projects it
 *   dong_decal_destroy(decal);
 */

#ifndef DONG_DECAL_H
#define DONG_DECAL_H

#include "dong.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dong_decal dong_decal_t;

/** Create a world-space decal instance. */
DONG_API dong_decal_t* dong_decal_create(dong_engine_t* engine);

/** Destroy a decal instance. */
DONG_API void dong_decal_destroy(dong_decal_t* decal);

/** Set the 3D world position (center of the decal projection volume). */
DONG_API void dong_decal_set_position(dong_decal_t* decal, float x, float y, float z);

/** Set the projection direction (normalized vector, default: 0,-1,0 = project downward). */
DONG_API void dong_decal_set_direction(dong_decal_t* decal, float dx, float dy, float dz);

/** Set the decal size in world units (width, height of the projection area). */
DONG_API void dong_decal_set_size(dong_decal_t* decal, float width, float height);

/** Set the projection depth (how far the decal projects along its direction, default: 1.0). */
DONG_API void dong_decal_set_depth(dong_decal_t* decal, float depth);

/** Set the decal color (RGBA, linear space). Used as tint when texture is set. */
DONG_API void dong_decal_set_color(dong_decal_t* decal, float r, float g, float b, float a);

/** Set the texture source path (optional; if NULL, solid color is used). */
DONG_API void dong_decal_set_texture(dong_decal_t* decal, const char* texture_path);

/**
 * Set the normal threshold for projection (dot product, 0.0-1.0).
 * Only surfaces whose normal dot projection_direction >= threshold will receive the decal.
 * Default: 0.5 (projects on surfaces up to 60° from the projection direction).
 */
DONG_API void dong_decal_set_normal_threshold(dong_decal_t* decal, float threshold);

/** Set visibility (default: true). */
DONG_API void dong_decal_set_visible(dong_decal_t* decal, int visible);

/** Set a rotation angle around the projection axis (radians, default: 0). */
DONG_API void dong_decal_set_rotation(dong_decal_t* decal, float radians);

#ifdef __cplusplus
}
#endif

#endif /* DONG_DECAL_H */
