/**
 * @file dong_drawlist.h
 * @brief Dong Draw List - Stable public C ABI for consuming GPU draw commands.
 *
 * This header defines the draw list interface that host engines (UE, Unity, Godot,
 * custom engines) can consume to render Dong UI without depending on internal types.
 *
 * The draw list is a flat array of dong_draw_cmd_t structs, produced each tick by
 * dong_engine_tick(). The array is valid until the next call to dong_engine_tick().
 *
 * Usage:
 *   dong_engine_tick(engine);
 *   uint32_t count = 0;
 *   const dong_draw_cmd_t* cmds = dong_drawlist_get_commands(engine, &count);
 *   for (uint32_t i = 0; i < count; i++) {
 *       switch (cmds[i].type) { ... }
 *   }
 */

#ifndef DONG_DRAWLIST_H
#define DONG_DRAWLIST_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * DLL export/import macros (mirrors dong.h)
 * ============================================================================ */
#ifndef DONG_API
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_API __declspec(dllexport)
    #else
        #define DONG_API __declspec(dllimport)
    #endif
#else
    #define DONG_API __attribute__((visibility("default")))
#endif
#endif

/* ============================================================================
 * ABI Version
 * ============================================================================ */

/**
 * Draw list ABI version. Bumped on any breaking change to struct layout or
 * semantic changes to field interpretation. Host engines should check this
 * at startup via dong_drawlist_get_abi_version() and refuse to operate if
 * it doesn't match their compiled-against version.
 */
#define DONG_DRAWLIST_ABI_VERSION 1u

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */

typedef struct dong_engine_t dong_engine_t;

/* ============================================================================
 * Draw Command Types
 * ============================================================================ */

/**
 * Enumerates all draw command types in the draw list.
 *
 * Commands 0-9: Primitive drawing operations
 * Commands 10-19: State stack operations (clip, layer)
 * Commands 20+: Extension points
 */
typedef enum dong_draw_cmd_type_t {
    DONG_DRAW_CMD_RECT            = 0,   /**< Solid color rectangle fill */
    DONG_DRAW_CMD_ROUNDED_RECT    = 1,   /**< Rounded rectangle (fill or stroke) */
    DONG_DRAW_CMD_IMAGE           = 2,   /**< Textured quad (image_id + UV) */
    DONG_DRAW_CMD_TEXT            = 3,   /**< Text glyph run (glyph_data pointer) */
    DONG_DRAW_CMD_SHADOW          = 4,   /**< Box shadow with blur radius */
    DONG_DRAW_CMD_GRADIENT        = 5,   /**< Linear gradient fill */
    DONG_DRAW_CMD_CONIC_GRADIENT  = 6,   /**< Conic/angular gradient fill */

    DONG_DRAW_CMD_CLIP_PUSH       = 10,  /**< Push clip rectangle onto clip stack */
    DONG_DRAW_CMD_CLIP_POP        = 11,  /**< Pop clip rectangle from clip stack */
    DONG_DRAW_CMD_LAYER_PUSH      = 12,  /**< Push isolated layer (opacity/blend mode) */
    DONG_DRAW_CMD_LAYER_POP       = 13,  /**< Pop isolated layer, composite back */

    DONG_DRAW_CMD_HOST_VIEW       = 20,  /**< Host-provided content placeholder (P1-2) */
} dong_draw_cmd_type_t;

/* ============================================================================
 * Common Data Types
 * ============================================================================ */

/** Axis-aligned rectangle (origin at top-left). */
typedef struct dong_rect_t {
    float x;       /**< Left edge */
    float y;       /**< Top edge */
    float width;   /**< Width (non-negative) */
    float height;  /**< Height (non-negative) */
} dong_rect_t;

/** RGBA color with pre-multiplied alpha, each component in [0.0, 1.0]. */
typedef struct dong_color_t {
    float r;  /**< Red */
    float g;  /**< Green */
    float b;  /**< Blue */
    float a;  /**< Alpha (0 = transparent, 1 = opaque) */
} dong_color_t;

/* ============================================================================
 * Draw Command Struct
 * ============================================================================
 *
 * This is a flat "tagged union" style struct. Only fields relevant to the
 * command type are meaningful; others are zero-initialized. This design
 * trades memory for ABI stability - no pointers to variant structs needed.
 *
 * LAYOUT GUARANTEE: Fields are never reordered or removed. New fields are
 * added by consuming reserved slots. The struct size is stable across minor
 * ABI versions.
 */

typedef struct dong_draw_cmd_t {
    /** Command type tag - determines which fields are active. */
    dong_draw_cmd_type_t type;

    /** Destination rectangle in viewport coordinates (pixels).
     *  Used by: RECT, ROUNDED_RECT, IMAGE, TEXT, SHADOW, GRADIENT,
     *           CONIC_GRADIENT, CLIP_PUSH, HOST_VIEW */
    dong_rect_t rect;

    /** Primary color.
     *  Used by: RECT (fill color), ROUNDED_RECT (fill/stroke color),
     *           SHADOW (shadow color), TEXT (text color) */
    dong_color_t color;

    /** Corner radius in pixels.
     *  Used by: ROUNDED_RECT (corner rounding), SHADOW (source box radius),
     *           CLIP_PUSH (rounded clip) */
    float radius;

    /** Stroke width in pixels. 0 means filled.
     *  Used by: ROUNDED_RECT */
    float stroke_width;

    /** Blur radius in pixels (Gaussian sigma).
     *  Used by: SHADOW */
    float blur;

    /** Layer opacity [0.0, 1.0].
     *  Used by: LAYER_PUSH */
    float opacity;

    /** Opaque image handle (engine-managed texture ID).
     *  Used by: IMAGE. The host engine resolves this via dong image API.
     *  Value 0 means no image. */
    uint32_t image_id;

    /** UV coordinates for texture sampling: [u0, v0, u1, v1].
     *  Used by: IMAGE. (0,0) is top-left, (1,1) is bottom-right. */
    float uv[4];

    /** Number of glyphs in the glyph run.
     *  Used by: TEXT */
    uint32_t glyph_count;

    /** Pointer to glyph position/atlas data array.
     *  Used by: TEXT. Pointer is valid until the next dong_engine_tick() call.
     *  The internal layout of each glyph entry is defined separately. */
    const void* glyph_data;

    /** Gradient angle in degrees (0 = right, 90 = down).
     *  Used by: GRADIENT */
    float gradient_angle;

    /** Number of color stops in the gradient.
     *  Used by: GRADIENT, CONIC_GRADIENT */
    uint32_t stop_count;

    /** Pointer to gradient color stop array.
     *  Used by: GRADIENT, CONIC_GRADIENT. Valid until next dong_engine_tick().
     *  Each stop is a {float position; dong_color_t color;} pair. */
    const void* stop_data;

    /** Host view identifier for host-rendered content regions.
     *  Used by: HOST_VIEW. The host engine maps this to its own content. */
    uint32_t host_view_id;

    /** Reserved for future ABI-compatible extensions. Must be zero. */
    uint32_t reserved[4];

} dong_draw_cmd_t;

/* ============================================================================
 * Draw List API Functions
 * ============================================================================ */

/**
 * Get the draw command array for the current frame.
 *
 * Returns a pointer to a contiguous array of dong_draw_cmd_t. The array is
 * owned by the engine and valid until the next call to dong_engine_tick().
 *
 * @param engine    The engine instance (must not be NULL).
 * @param out_count Receives the number of commands in the array.
 *                  Set to 0 if engine has no commands. Must not be NULL.
 * @return Pointer to the first command, or NULL if no commands exist.
 */
DONG_API const dong_draw_cmd_t* dong_drawlist_get_commands(dong_engine_t* engine,
                                                           uint32_t* out_count);

/**
 * Query the draw list ABI version compiled into the library.
 *
 * Host engines should call this at initialization and compare against
 * DONG_DRAWLIST_ABI_VERSION to detect version mismatches.
 *
 * @return DONG_DRAWLIST_ABI_VERSION as compiled into dong.dll
 */
DONG_API uint32_t dong_drawlist_get_abi_version(void);

/**
 * Helper: extract the command type from a draw command.
 *
 * Equivalent to cmd->type but provides a function-call interface for
 * languages that cannot directly read C struct fields (e.g., via FFI).
 *
 * @param cmd Pointer to a draw command (must not be NULL).
 * @return The command type enum value.
 */
DONG_API dong_draw_cmd_type_t dong_draw_cmd_get_type(const dong_draw_cmd_t* cmd);

#ifdef __cplusplus
}
#endif

#endif /* DONG_DRAWLIST_H */
