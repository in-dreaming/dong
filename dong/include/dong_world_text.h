/**
 * @file dong_world_text.h
 * @brief World-space billboard text API (P2-1)
 *
 * Provides GPU-accelerated floating text in 3D world space.
 * Text is always camera-facing (billboard) and rendered using the
 * shared GlyphAtlas (no per-text texture allocation).
 *
 * Typical use cases:
 * - Damage numbers floating above enemies
 * - Player/NPC names above heads
 * - Item labels in the world
 * - Quest markers with text
 *
 * Usage:
 *   dong_world_text_t* wt = dong_world_text_create(engine);
 *   dong_world_text_set_text(wt, "999");
 *   dong_world_text_set_position(wt, 0.0f, 2.5f, -5.0f);
 *   dong_world_text_set_color(wt, 1.0f, 0.2f, 0.2f, 1.0f);
 *   dong_world_text_set_font_size(wt, 24.0f);
 *   // ... in render loop, world texts are drawn automatically after UI
 *   dong_world_text_destroy(wt);
 */

#ifndef DONG_WORLD_TEXT_H
#define DONG_WORLD_TEXT_H

#include "dong.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dong_world_text dong_world_text_t;

/**
 * Create a world-space text instance.
 * The text is invisible until dong_world_text_set_text() is called.
 */
DONG_API dong_world_text_t* dong_world_text_create(dong_engine_t* engine);

/** Destroy a world-space text instance. */
DONG_API void dong_world_text_destroy(dong_world_text_t* wt);

/** Set the text content (UTF-8). */
DONG_API void dong_world_text_set_text(dong_world_text_t* wt, const char* text);

/** Set the 3D world position (x, y, z). */
DONG_API void dong_world_text_set_position(dong_world_text_t* wt, float x, float y, float z);

/** Set the text color (RGBA, linear space). */
DONG_API void dong_world_text_set_color(dong_world_text_t* wt, float r, float g, float b, float a);

/** Set the font size in screen pixels (default: 16). */
DONG_API void dong_world_text_set_font_size(dong_world_text_t* wt, float size);

/** Set the font family (default: "sans-serif"). */
DONG_API void dong_world_text_set_font_family(dong_world_text_t* wt, const char* family);

/** Set visibility (default: true). */
DONG_API void dong_world_text_set_visible(dong_world_text_t* wt, int visible);

/** Set the view-projection matrix for 3D→2D projection (column-major 4x4). */
DONG_API void dong_engine_set_world_text_vp(dong_engine_t* engine, const float* vp_matrix_4x4);

/**
 * Configuration for world text rendering.
 */
typedef struct {
    float fade_start_distance;  /**< Distance in world units where text starts fading (default: 50) */
    float fade_end_distance;    /**< Distance where text is fully invisible (default: 100) */
    float min_scale;            /**< Minimum screen-space scale factor (default: 0.5) */
    float max_scale;            /**< Maximum screen-space scale factor (default: 3.0) */
} dong_world_text_config_t;

/** Set global configuration for world text rendering. */
DONG_API void dong_engine_set_world_text_config(dong_engine_t* engine, const dong_world_text_config_t* config);

#ifdef __cplusplus
}
#endif

#endif /* DONG_WORLD_TEXT_H */
