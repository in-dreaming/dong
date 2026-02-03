// ASTC Encoder - Lightweight implementation for Dong Engine
// Based on the ASTC specification (Khronos)
// This is a simplified encoder optimized for fast encoding, not maximum quality.
//
// Reference: https://registry.khronos.org/DataFormat/specs/1.3/dataformat.1.3.html#ASTC

#ifndef DONG_ASTC_ENCODER_H
#define DONG_ASTC_ENCODER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ASTC Block Sizes
// =============================================================================

typedef enum DongASTCBlockSize {
    DONG_ASTC_4x4 = 0,
    DONG_ASTC_5x4,
    DONG_ASTC_5x5,
    DONG_ASTC_6x5,
    DONG_ASTC_6x6,
    DONG_ASTC_8x5,
    DONG_ASTC_8x6,
    DONG_ASTC_8x8,
    DONG_ASTC_10x5,
    DONG_ASTC_10x6,
    DONG_ASTC_10x8,
    DONG_ASTC_10x10,
    DONG_ASTC_12x10,
    DONG_ASTC_12x12,
    DONG_ASTC_BLOCK_COUNT
} DongASTCBlockSize;

// =============================================================================
// Encoder Quality Presets
// =============================================================================

typedef enum DongASTCQuality {
    DONG_ASTC_QUALITY_FASTEST = 0,  // ~10 trials per block
    DONG_ASTC_QUALITY_FAST = 1,     // ~25 trials per block
    DONG_ASTC_QUALITY_MEDIUM = 2,   // ~50 trials per block
    DONG_ASTC_QUALITY_HIGH = 3,     // ~100 trials per block
    DONG_ASTC_QUALITY_BEST = 4,     // ~200 trials per block
} DongASTCQuality;

// =============================================================================
// Encoder Options
// =============================================================================

typedef struct DongASTCEncodeOptions {
    DongASTCQuality quality;
    int srgb;           // 1 = sRGB color space, 0 = linear
    int normal_map;     // 1 = optimize for normal maps (use RG channels)
    int alpha_weight;   // 0-100, weight for alpha channel in error metric
} DongASTCEncodeOptions;

static inline DongASTCEncodeOptions dong_astc_options_default(void) {
    DongASTCEncodeOptions opts = {0};
    opts.quality = DONG_ASTC_QUALITY_MEDIUM;
    opts.srgb = 1;
    opts.normal_map = 0;
    opts.alpha_weight = 50;
    return opts;
}

// =============================================================================
// Block Dimensions
// =============================================================================

typedef struct DongASTCBlockDim {
    uint8_t width;
    uint8_t height;
} DongASTCBlockDim;

// Get block dimensions for a given block size
static inline DongASTCBlockDim dong_astc_block_dim(DongASTCBlockSize size) {
    static const DongASTCBlockDim dims[DONG_ASTC_BLOCK_COUNT] = {
        {4, 4}, {5, 4}, {5, 5}, {6, 5}, {6, 6}, {8, 5}, {8, 6}, {8, 8},
        {10, 5}, {10, 6}, {10, 8}, {10, 10}, {12, 10}, {12, 12}
    };
    if (size >= DONG_ASTC_BLOCK_COUNT) {
        DongASTCBlockDim invalid = {0, 0};
        return invalid;
    }
    return dims[size];
}

// =============================================================================
// ASTC Encoder API
// =============================================================================

// Encode a single ASTC block (16 bytes output)
// pixels: RGBA8 pixel data for the block (width * height * 4 bytes)
// width, height: Block dimensions (must match block_size)
// output: 16-byte output buffer for the encoded block
// Returns: 0 on success, non-zero on error
int dong_astc_encode_block(
    const uint8_t* pixels,
    uint32_t width,
    uint32_t height,
    DongASTCBlockSize block_size,
    const DongASTCEncodeOptions* options,
    uint8_t output[16]
);

// Encode an entire image to ASTC
// pixels: RGBA8 pixel data (img_width * img_height * 4 bytes)
// img_width, img_height: Image dimensions
// block_size: ASTC block size to use
// options: Encoding options (NULL for defaults)
// output: Output buffer (must be pre-allocated)
// output_size: Size of output buffer
// Returns: Number of bytes written, or 0 on error
size_t dong_astc_encode_image(
    const uint8_t* pixels,
    uint32_t img_width,
    uint32_t img_height,
    DongASTCBlockSize block_size,
    const DongASTCEncodeOptions* options,
    uint8_t* output,
    size_t output_size
);

// Calculate required output buffer size for an image
static inline size_t dong_astc_calc_size(uint32_t width, uint32_t height, DongASTCBlockSize block_size) {
    DongASTCBlockDim dim = dong_astc_block_dim(block_size);
    if (dim.width == 0 || dim.height == 0) return 0;

    uint32_t blocks_x = (width + dim.width - 1) / dim.width;
    uint32_t blocks_y = (height + dim.height - 1) / dim.height;

    return (size_t)blocks_x * blocks_y * 16;  // Each block is 16 bytes
}

#ifdef __cplusplus
}
#endif

#endif // DONG_ASTC_ENCODER_H
