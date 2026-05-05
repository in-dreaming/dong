/**
 * @file dong_vector.h
 * @brief Vector Animation API — Lottie / Rive support (P2-4)
 *
 * Plays Lottie (Bodymovin JSON) and Rive (.riv) vector animations
 * using the Slug GPU bezier pipeline for hardware-accelerated rendering.
 *
 * HTML elements:
 *   <dong-lottie src="loading.json" width="200" height="200" autoplay="1" loop="1">
 *   <dong-rive src="character.riv" state-machine="Walk" width="400" height="400">
 *
 * C API usage:
 *   DongVectorAnimation* va = dong_vector_load(engine, DONG_VECTOR_LOTTIE, data, len);
 *   dong_vector_play(va);
 *   // each frame:
 *   dong_vector_tick(va, dt);
 *   dong_vector_draw(va, x, y, w, h);
 *   // cleanup:
 *   dong_vector_destroy(va);
 */

#ifndef DONG_VECTOR_H
#define DONG_VECTOR_H

#include "dong.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DongVectorAnimation DongVectorAnimation;

typedef enum {
    DONG_VECTOR_LOTTIE = 0,
    DONG_VECTOR_RIVE   = 1,
} DongVectorFormat;

typedef enum {
    DONG_VECTOR_STOPPED = 0,
    DONG_VECTOR_PLAYING = 1,
    DONG_VECTOR_PAUSED  = 2,
} DongVectorState;

/**
 * Load a vector animation from memory.
 * @param engine Engine instance (for GPU resource allocation)
 * @param format DONG_VECTOR_LOTTIE or DONG_VECTOR_RIVE
 * @param data Animation file data (JSON for Lottie, binary for Rive)
 * @param len Data length in bytes
 * @return Animation handle, or NULL on failure
 */
DONG_API DongVectorAnimation* dong_vector_load(dong_engine_t* engine,
                                                DongVectorFormat format,
                                                const void* data, size_t len);

/**
 * Load a vector animation from a file path.
 */
DONG_API DongVectorAnimation* dong_vector_load_file(dong_engine_t* engine,
                                                     DongVectorFormat format,
                                                     const char* path);

/** Destroy a vector animation and free GPU resources. */
DONG_API void dong_vector_destroy(DongVectorAnimation* va);

/** Get the animation duration in seconds. */
DONG_API double dong_vector_duration(DongVectorAnimation* va);

/** Get the current playback state. */
DONG_API DongVectorState dong_vector_state(DongVectorAnimation* va);

/** Start or resume playback. */
DONG_API void dong_vector_play(DongVectorAnimation* va);

/** Pause playback. */
DONG_API void dong_vector_pause(DongVectorAnimation* va);

/** Stop and reset to beginning. */
DONG_API void dong_vector_stop(DongVectorAnimation* va);

/** Seek to a specific time (seconds). */
DONG_API void dong_vector_seek(DongVectorAnimation* va, double time_seconds);

/** Set loop mode (0 = once, 1 = loop forever, N = loop N times). */
DONG_API void dong_vector_set_loop(DongVectorAnimation* va, int loop_count);

/** Set playback speed multiplier (1.0 = normal). */
DONG_API void dong_vector_set_speed(DongVectorAnimation* va, float speed);

/**
 * Advance animation time by dt seconds.
 * Call once per frame before draw.
 */
DONG_API void dong_vector_tick(DongVectorAnimation* va, float dt);

/**
 * Draw the animation at the given rect (screen coordinates).
 * Emits draw commands to the current view's display list.
 */
DONG_API void dong_vector_draw(DongVectorAnimation* va,
                                float x, float y, float w, float h);

/* ============================================================================
 * Rive State Machine API
 * ============================================================================ */

/** Play a Rive state machine by name. Returns 0 on success, -1 on failure. */
DONG_API int dong_vector_rive_play_state_machine(DongVectorAnimation* va,
                                                  const char* state_machine_name);

/** Set a boolean input on the active Rive state machine. */
DONG_API int dong_vector_rive_set_input_bool(DongVectorAnimation* va,
                                              const char* input_name, int value);

/** Set a number input on the active Rive state machine. */
DONG_API int dong_vector_rive_set_input_number(DongVectorAnimation* va,
                                                const char* input_name, double value);

/** Fire a trigger on the active Rive state machine. */
DONG_API int dong_vector_rive_fire_trigger(DongVectorAnimation* va,
                                            const char* trigger_name);

#ifdef __cplusplus
}
#endif

#endif /* DONG_VECTOR_H */
