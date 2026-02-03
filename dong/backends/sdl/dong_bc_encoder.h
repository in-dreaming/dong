// BC7 Encoder - Lightweight implementation for Dong Engine
// Based on the BC7 specification (Microsoft DirectX)
//
// BC7 is the highest quality BC format, supporting RGBA with high precision.
// Each 4x4 block is encoded to 16 bytes.

#ifndef DONG_BC_ENCODER_H
#define DONG_BC_ENCODER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BC Format Types
// =============================================================================

typedef enum DongBCFormat {
    DONG_BC_FORMAT_BC1,     // DXT1: RGB + 1-bit alpha, 8 bytes/block
    DONG_BC_FORMAT_BC3,     // DXT5: RGB + interpolated alpha, 16 bytes/block
    DONG_BC_FORMAT_BC4,     // Single channel, 8 bytes/block
    DONG_BC_FORMAT_BC5,     // Two channels (normal maps), 16 bytes/block
    DONG_BC_FORMAT_BC6H,    // HDR RGB, 16 bytes/block
    DONG_BC_FORMAT_BC7,     // High quality RGBA, 16 bytes/block
    DONG_BC_FORMAT_COUNT
} DongBCFormat;

// =============================================================================
// Encoder Quality
// =============================================================================

typedef enum DongBCQuality {
    DONG_BC_QUALITY_FASTEST = 0,
    DONG_BC_QUALITY_FAST = 1,
    DONG_BC_QUALITY_MEDIUM = 2,
    DONG_BC_QUALITY_HIGH = 3,
    DONG_BC_QUALITY_BEST = 4,
} DongBCQuality;

// =============================================================================
// Encoder Options
// =============================================================================

typedef struct DongBCEncodeOptions {
    DongBCQuality quality;
    int srgb;           // 1 = sRGB, 0 = linear
    int perceptual;     // 1 = use perceptual error metric
} DongBCEncodeOptions;

static inline DongBCEncodeOptions dong_bc_options_default(void) {
    DongBCEncodeOptions opts = {0};
    opts.quality = DONG_BC_QUALITY_MEDIUM;
    opts.srgb = 1;
    opts.perceptual = 1;
    return opts;
}

// =============================================================================
// Block Sizes
// =============================================================================

// Get bytes per block for a BC format
static inline uint32_t dong_bc_bytes_per_block(DongBCFormat format) {
    switch (format) {
        case DONG_BC_FORMAT_BC1: return 8;
        case DONG_BC_FORMAT_BC4: return 8;
        case DONG_BC_FORMAT_BC3:
        case DONG_BC_FORMAT_BC5:
        case DONG_BC_FORMAT_BC6H:
        case DONG_BC_FORMAT_BC7: return 16;
        default: return 0;
    }
}

// Calculate required buffer size for an image
static inline size_t dong_bc_calc_size(uint32_t width, uint32_t height, DongBCFormat format) {
    uint32_t bytes_per_block = dong_bc_bytes_per_block(format);
    if (bytes_per_block == 0) return 0;

    uint32_t blocks_x = (width + 3) / 4;
    uint32_t blocks_y = (height + 3) / 4;

    return (size_t)blocks_x * blocks_y * bytes_per_block;
}

// =============================================================================
// BC7 Encoder API
// =============================================================================

// Encode a single BC7 block (16 bytes output)
// pixels: RGBA8 pixel data for the 4x4 block (64 bytes)
// output: 16-byte output buffer
// Returns: 0 on success
int dong_bc7_encode_block(
    const uint8_t pixels[64],
    const DongBCEncodeOptions* options,
    uint8_t output[16]
);

// Encode an entire image to BC7
// pixels: RGBA8 pixel data
// img_width, img_height: Image dimensions
// options: Encoding options (NULL for defaults)
// output: Output buffer (pre-allocated)
// output_size: Size of output buffer
// Returns: Bytes written, or 0 on error
size_t dong_bc7_encode_image(
    const uint8_t* pixels,
    uint32_t img_width,
    uint32_t img_height,
    const DongBCEncodeOptions* options,
    uint8_t* output,
    size_t output_size
);

// =============================================================================
// BC6H Encoder API (HDR)
// =============================================================================

// Encode a single BC6H block (16 bytes output)
// pixels: RGBA16F pixel data for the 4x4 block (128 bytes, 16 half-floats * 4 components)
// output: 16-byte output buffer
// is_signed: 1 for signed, 0 for unsigned
// Returns: 0 on success
int dong_bc6h_encode_block(
    const uint16_t pixels[64],  // 4x4 * 4 components (half-float)
    int is_signed,
    const DongBCEncodeOptions* options,
    uint8_t output[16]
);

// Encode an entire HDR image to BC6H
// pixels: RGBA16F pixel data (half-floats)
// img_width, img_height: Image dimensions
// is_signed: 1 for signed, 0 for unsigned
// options: Encoding options (NULL for defaults)
// output: Output buffer
// output_size: Size of output buffer
// Returns: Bytes written, or 0 on error
size_t dong_bc6h_encode_image(
    const uint16_t* pixels,
    uint32_t img_width,
    uint32_t img_height,
    int is_signed,
    const DongBCEncodeOptions* options,
    uint8_t* output,
    size_t output_size
);

#ifdef __cplusplus
}
#endif

#endif // DONG_BC_ENCODER_H
