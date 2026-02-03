/**
 * @file dong_image_atlas.h
 * @brief Image Atlas interface for GPU texture packing
 *
 * This atlas supports both uncompressed (RGBA8) and compressed (ASTC, BC) formats.
 * For compressed formats, allocations are automatically aligned to block boundaries.
 *
 * Design:
 * - Platform-agnostic interface (C API)
 * - Supports runtime format selection based on GPU capabilities
 * - Block-aligned allocation for compressed formats
 * - Simple row-based packing algorithm (expandable to shelf/skyline)
 */

#ifndef DONG_IMAGE_ATLAS_H
#define DONG_IMAGE_ATLAS_H

#include "dong_image_decoder.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Types
// =============================================================================

/** Opaque handle for image atlas */
typedef struct DongImageAtlas DongImageAtlas;

/** Atlas entry - region within the atlas texture */
typedef struct DongAtlasEntry {
    uint32_t atlas_page;   /**< Page index (for multi-page atlases) */
    uint32_t x;            /**< X offset in pixels */
    uint32_t y;            /**< Y offset in pixels */
    uint32_t width;        /**< Width in pixels */
    uint32_t height;       /**< Height in pixels */
    float u0, v0;          /**< Top-left UV coordinates */
    float u1, v1;          /**< Bottom-right UV coordinates */
} DongAtlasEntry;

/** Result codes */
typedef enum DongAtlasResult {
    DONG_ATLAS_OK = 0,
    DONG_ATLAS_ERR_INVALID_ARG,
    DONG_ATLAS_ERR_OUT_OF_SPACE,
    DONG_ATLAS_ERR_FORMAT_MISMATCH,
    DONG_ATLAS_ERR_INTERNAL,
} DongAtlasResult;

/** Atlas configuration */
typedef struct DongAtlasConfig {
    uint32_t width;              /**< Atlas width in pixels (default: 2048) */
    uint32_t height;             /**< Atlas height in pixels (default: 2048) */
    uint32_t max_pages;          /**< Maximum number of pages (default: 4) */
    DongImageFormat format;      /**< Texture format (default: RGBA8) */
    uint32_t padding;            /**< Padding between entries in pixels (default: 2) */
} DongAtlasConfig;

// =============================================================================
// Atlas VTable (for backend implementations)
// =============================================================================

typedef struct DongImageAtlasVTable {
    /**
     * Allocate a region in the atlas.
     * For compressed formats, coordinates are aligned to block boundaries.
     *
     * @param atlas     The atlas instance
     * @param width     Requested width in pixels
     * @param height    Requested height in pixels
     * @param out_entry Output: allocated region
     * @return DONG_ATLAS_OK on success
     */
    DongAtlasResult (*alloc)(DongImageAtlas* atlas,
                             uint32_t width,
                             uint32_t height,
                             DongAtlasEntry* out_entry);

    /**
     * Upload image data to a previously allocated region.
     * Data format must match the atlas format.
     *
     * @param atlas     The atlas instance
     * @param entry     Region to upload to
     * @param data      Image data (format must match atlas format)
     * @param data_size Size of data in bytes
     * @return DONG_ATLAS_OK on success
     */
    DongAtlasResult (*upload)(DongImageAtlas* atlas,
                              const DongAtlasEntry* entry,
                              const void* data,
                              size_t data_size);

    /**
     * Get native texture handle for a page.
     * The returned pointer type depends on the backend:
     * - SDL: SDL_GPUTexture*
     * - Vulkan: VkImage
     * - D3D12: ID3D12Resource*
     *
     * @param atlas      The atlas instance
     * @param page_index Page index
     * @return Native texture handle, or NULL if invalid
     */
    void* (*get_texture)(DongImageAtlas* atlas, uint32_t page_index);

    /**
     * Clear all allocations (but keep texture memory).
     */
    void (*clear)(DongImageAtlas* atlas);

    /**
     * Destroy the atlas and free all resources.
     */
    void (*destroy)(DongImageAtlas* atlas);

} DongImageAtlasVTable;

struct DongImageAtlas {
    const DongImageAtlasVTable* vtable;
    DongAtlasConfig config;
    // Backend-specific data follows...
};

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * Initialize config with default values.
 */
static inline DongAtlasConfig dong_atlas_config_default(void) {
    DongAtlasConfig cfg = {0};
    cfg.width = 2048;
    cfg.height = 2048;
    cfg.max_pages = 4;
    cfg.format = DONG_IMAGE_FORMAT_RGBA8;
    cfg.padding = 2;
    return cfg;
}

/**
 * Align a coordinate to block boundary for the given format.
 * Returns the aligned value (>= input).
 */
static inline uint32_t dong_atlas_align_to_block(uint32_t value, DongImageFormat format) {
    uint32_t block_w, block_h;
    dong_image_format_block_size(format, &block_w, &block_h);

    if (block_w <= 1) {
        return value;
    }

    // Align up to block boundary
    return ((value + block_w - 1) / block_w) * block_w;
}

/**
 * Calculate the minimum allocation size for the given dimensions.
 * For compressed formats, dimensions are aligned to block boundaries.
 */
static inline void dong_atlas_min_alloc_size(DongImageFormat format,
                                              uint32_t width,
                                              uint32_t height,
                                              uint32_t* out_width,
                                              uint32_t* out_height) {
    uint32_t block_w, block_h;
    dong_image_format_block_size(format, &block_w, &block_h);

    // Align up to block boundaries
    *out_width = ((width + block_w - 1) / block_w) * block_w;
    *out_height = ((height + block_h - 1) / block_h) * block_h;
}

// =============================================================================
// Wrapper Functions (call through vtable)
// =============================================================================

static inline DongAtlasResult dong_atlas_alloc(DongImageAtlas* atlas,
                                                uint32_t width,
                                                uint32_t height,
                                                DongAtlasEntry* out_entry) {
    if (!atlas || !atlas->vtable || !atlas->vtable->alloc) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }
    return atlas->vtable->alloc(atlas, width, height, out_entry);
}

static inline DongAtlasResult dong_atlas_upload(DongImageAtlas* atlas,
                                                 const DongAtlasEntry* entry,
                                                 const void* data,
                                                 size_t data_size) {
    if (!atlas || !atlas->vtable || !atlas->vtable->upload) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }
    return atlas->vtable->upload(atlas, entry, data, data_size);
}

static inline void* dong_atlas_get_texture(DongImageAtlas* atlas, uint32_t page_index) {
    if (!atlas || !atlas->vtable || !atlas->vtable->get_texture) {
        return NULL;
    }
    return atlas->vtable->get_texture(atlas, page_index);
}

static inline void dong_atlas_clear(DongImageAtlas* atlas) {
    if (atlas && atlas->vtable && atlas->vtable->clear) {
        atlas->vtable->clear(atlas);
    }
}

static inline void dong_atlas_destroy(DongImageAtlas* atlas) {
    if (atlas && atlas->vtable && atlas->vtable->destroy) {
        atlas->vtable->destroy(atlas);
    }
}

#ifdef __cplusplus
}
#endif

#endif // DONG_IMAGE_ATLAS_H
