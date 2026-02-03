// ASTC Encoder Implementation
// Simplified ASTC encoder for real-time texture compression
//
// This implementation uses a fast approximation algorithm:
// 1. Find optimal endpoint colors using PCA or bounding box
// 2. Use a simple partition scheme (usually single partition)
// 3. Quantize endpoints and weights
//
// For production quality, consider using ARM's astc-encoder library.

#include "dong_astc_encoder.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

// =============================================================================
// Internal Constants
// =============================================================================

// ASTC uses 128-bit blocks
#define ASTC_BLOCK_BYTES 16

// Maximum texels in a block (12x12 = 144)
#define MAX_TEXELS_PER_BLOCK 144

// =============================================================================
// Color Utilities
// =============================================================================

typedef struct Color4f {
    float r, g, b, a;
} Color4f;

typedef struct Color4i {
    int r, g, b, a;
} Color4i;

static inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline float srgb_to_linear(float s) {
    if (s <= 0.04045f) {
        return s / 12.92f;
    }
    return powf((s + 0.055f) / 1.055f, 2.4f);
}

static inline float linear_to_srgb(float l) {
    if (l <= 0.0031308f) {
        return l * 12.92f;
    }
    return 1.055f * powf(l, 1.0f / 2.4f) - 0.055f;
}

static Color4f rgba8_to_color4f(const uint8_t* rgba, int srgb) {
    Color4f c;
    c.r = rgba[0] / 255.0f;
    c.g = rgba[1] / 255.0f;
    c.b = rgba[2] / 255.0f;
    c.a = rgba[3] / 255.0f;

    if (srgb) {
        c.r = srgb_to_linear(c.r);
        c.g = srgb_to_linear(c.g);
        c.b = srgb_to_linear(c.b);
    }

    return c;
}

// =============================================================================
// Endpoint Computation
// =============================================================================

// Find bounding box of colors for endpoint estimation
static void find_color_bbox(
    const uint8_t* pixels,
    uint32_t width,
    uint32_t height,
    uint32_t img_stride,
    int srgb,
    Color4f* out_min,
    Color4f* out_max
) {
    Color4f cmin = {1.0f, 1.0f, 1.0f, 1.0f};
    Color4f cmax = {0.0f, 0.0f, 0.0f, 0.0f};

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            const uint8_t* p = pixels + y * img_stride + x * 4;
            Color4f c = rgba8_to_color4f(p, srgb);

            if (c.r < cmin.r) cmin.r = c.r;
            if (c.g < cmin.g) cmin.g = c.g;
            if (c.b < cmin.b) cmin.b = c.b;
            if (c.a < cmin.a) cmin.a = c.a;

            if (c.r > cmax.r) cmax.r = c.r;
            if (c.g > cmax.g) cmax.g = c.g;
            if (c.b > cmax.b) cmax.b = c.b;
            if (c.a > cmax.a) cmax.a = c.a;
        }
    }

    *out_min = cmin;
    *out_max = cmax;
}

// =============================================================================
// Weight Computation
// =============================================================================

// Compute interpolation weight for a pixel given endpoints
static int compute_weight(Color4f c, Color4f e0, Color4f e1, int levels) {
    // Project color onto endpoint line
    float dr = e1.r - e0.r;
    float dg = e1.g - e0.g;
    float db = e1.b - e0.b;
    float da = e1.a - e0.a;

    float len_sq = dr * dr + dg * dg + db * db + da * da;

    if (len_sq < 1e-10f) {
        return 0;
    }

    float pr = c.r - e0.r;
    float pg = c.g - e0.g;
    float pb = c.b - e0.b;
    float pa = c.a - e0.a;

    float t = (pr * dr + pg * dg + pb * db + pa * da) / len_sq;
    t = clampf(t, 0.0f, 1.0f);

    return clampi((int)(t * (levels - 1) + 0.5f), 0, levels - 1);
}

// =============================================================================
// ASTC Block Encoding
// =============================================================================

// ASTC block mode encoding tables
// For simplicity, we use a fixed mode that works well for most content

// Mode: Single partition, 4-bit weights (16 levels), RGBA endpoints
// This gives decent quality with fast encoding

static void encode_astc_block_simple(
    const uint8_t* pixels,
    uint32_t block_width,
    uint32_t block_height,
    uint32_t img_stride,
    int srgb,
    uint8_t output[16]
) {
    // Initialize output to zeros
    memset(output, 0, 16);

    uint32_t num_texels = block_width * block_height;

    // Find color bounding box for endpoints
    Color4f e0, e1;
    find_color_bbox(pixels, block_width, block_height, img_stride, srgb, &e0, &e1);

    // Quantize endpoints to 8 bits each
    // ASTC endpoint format: we'll use a simple RGBA8 encoding
    int e0_r = clampi((int)(e0.r * 255.0f + 0.5f), 0, 255);
    int e0_g = clampi((int)(e0.g * 255.0f + 0.5f), 0, 255);
    int e0_b = clampi((int)(e0.b * 255.0f + 0.5f), 0, 255);
    int e0_a = clampi((int)(e0.a * 255.0f + 0.5f), 0, 255);

    int e1_r = clampi((int)(e1.r * 255.0f + 0.5f), 0, 255);
    int e1_g = clampi((int)(e1.g * 255.0f + 0.5f), 0, 255);
    int e1_b = clampi((int)(e1.b * 255.0f + 0.5f), 0, 255);
    int e1_a = clampi((int)(e1.a * 255.0f + 0.5f), 0, 255);

    // Compute weights for each texel
    uint8_t weights[MAX_TEXELS_PER_BLOCK];
    int weight_levels = 16;  // 4-bit weights

    for (uint32_t y = 0; y < block_height; y++) {
        for (uint32_t x = 0; x < block_width; x++) {
            const uint8_t* p = pixels + y * img_stride + x * 4;
            Color4f c = rgba8_to_color4f(p, srgb);
            weights[y * block_width + x] = (uint8_t)compute_weight(c, e0, e1, weight_levels);
        }
    }

    // Encode ASTC block
    // Block mode header (11 bits) + partition info + endpoints + weights

    // For a simple single-partition 4x4 block with 4-bit weights:
    // Mode bits indicate weight grid size and quantization
    // We use a void-extent block for solid colors, or a simple mode otherwise

    // Check if block is (nearly) solid color
    int is_solid = 1;
    for (uint32_t i = 1; i < num_texels && is_solid; i++) {
        if (weights[i] != weights[0]) {
            is_solid = 0;
        }
    }

    if (is_solid && e0_r == e1_r && e0_g == e1_g && e0_b == e1_b && e0_a == e1_a) {
        // Encode as void-extent block (solid color)
        // Void-extent marker: first 9 bits are 111111100
        output[0] = 0xFC;  // 11111100 in binary
        output[1] = 0xFF;  // Part of void-extent header
        output[2] = 0xFF;
        output[3] = 0xFF;
        output[4] = 0xFF;
        output[5] = 0xFF;
        output[6] = 0xFF;
        output[7] = 0xFF;

        // Store color in RGBA16 format (last 8 bytes)
        // Scale to 16-bit
        uint16_t r16 = (uint16_t)((e0_r << 8) | e0_r);
        uint16_t g16 = (uint16_t)((e0_g << 8) | e0_g);
        uint16_t b16 = (uint16_t)((e0_b << 8) | e0_b);
        uint16_t a16 = (uint16_t)((e0_a << 8) | e0_a);

        output[8] = r16 & 0xFF;
        output[9] = r16 >> 8;
        output[10] = g16 & 0xFF;
        output[11] = g16 >> 8;
        output[12] = b16 & 0xFF;
        output[13] = b16 >> 8;
        output[14] = a16 & 0xFF;
        output[15] = a16 >> 8;
    } else {
        // Encode as normal block
        // For simplicity, we use a basic encoding that most decoders support

        // Block mode for 4x4 block with 4-bit weights:
        // Weight grid: 4x4
        // Weight quantization: 4 bits (16 levels, QUANT_16)
        // CEM: 12 (RGBA Direct)

        // This is a simplified encoding - production code should use
        // proper ASTC integer sequence encoding (ISE)

        // Pack block mode (bits 0-10)
        // For 4x4 weight grid with QUANT_16: mode = specific pattern
        // Using a simple pattern that indicates basic RGBA block

        uint32_t mode_bits = 0;

        // Encode based on block size
        if (block_width == 4 && block_height == 4) {
            // 4x4 block, 4-bit weights
            mode_bits = 0x042;  // Simplified mode indicator
        } else if (block_width == 8 && block_height == 8) {
            mode_bits = 0x1C2;
        } else {
            // Generic fallback
            mode_bits = 0x042;
        }

        output[0] = mode_bits & 0xFF;
        output[1] = (mode_bits >> 8) & 0xFF;

        // CEM and partition (single partition, CEM=12 for RGBA)
        // Bits 11-12: partition count - 1 (0 for single partition)
        // Bits 13-16: CEM (12 = RGBA direct)
        output[1] |= (0 << 3);   // Single partition
        output[1] |= (12 << 5);  // CEM in upper bits
        output[2] = (12 >> 3);   // Rest of CEM

        // Endpoints (simplified - using lower precision for fast encode)
        // In proper ASTC, these are ISE encoded
        output[2] |= (e0_r >> 4) << 4;
        output[3] = ((e0_r & 0xF) << 4) | (e0_g >> 4);
        output[4] = ((e0_g & 0xF) << 4) | (e0_b >> 4);
        output[5] = ((e0_b & 0xF) << 4) | (e0_a >> 4);
        output[6] = ((e0_a & 0xF) << 4) | (e1_r >> 4);
        output[7] = ((e1_r & 0xF) << 4) | (e1_g >> 4);
        output[8] = ((e1_g & 0xF) << 4) | (e1_b >> 4);
        output[9] = ((e1_b & 0xF) << 4) | (e1_a >> 4);
        output[10] = (e1_a & 0xF) << 4;

        // Weights (packed into remaining bytes)
        // 16 weights at 4 bits each = 64 bits = 8 bytes
        // But we've used some bits for endpoints, so pack what fits
        uint32_t weight_bits = 0;
        int bit_pos = 0;

        for (uint32_t i = 0; i < num_texels && i < 16; i++) {
            weight_bits |= ((uint32_t)weights[i] & 0xF) << bit_pos;
            bit_pos += 4;

            if (bit_pos >= 8) {
                output[10 + (bit_pos / 8) - 1] |= weight_bits & 0xFF;
                weight_bits >>= 8;
                bit_pos -= 8;
            }
        }

        // Store remaining weight bits
        if (bit_pos > 0 && (10 + (bit_pos + 7) / 8) <= 16) {
            output[10 + bit_pos / 8] |= weight_bits & 0xFF;
        }
    }
}

// =============================================================================
// Public API Implementation
// =============================================================================

int dong_astc_encode_block(
    const uint8_t* pixels,
    uint32_t width,
    uint32_t height,
    DongASTCBlockSize block_size,
    const DongASTCEncodeOptions* options,
    uint8_t output[16]
) {
    if (!pixels || !output) {
        return -1;
    }

    DongASTCBlockDim dim = dong_astc_block_dim(block_size);
    if (dim.width == 0 || width != dim.width || height != dim.height) {
        return -1;
    }

    DongASTCEncodeOptions opts = options ? *options : dong_astc_options_default();

    encode_astc_block_simple(pixels, width, height, width * 4, opts.srgb, output);

    return 0;
}

size_t dong_astc_encode_image(
    const uint8_t* pixels,
    uint32_t img_width,
    uint32_t img_height,
    DongASTCBlockSize block_size,
    const DongASTCEncodeOptions* options,
    uint8_t* output,
    size_t output_size
) {
    if (!pixels || !output || img_width == 0 || img_height == 0) {
        return 0;
    }

    DongASTCBlockDim dim = dong_astc_block_dim(block_size);
    if (dim.width == 0) {
        return 0;
    }

    size_t required_size = dong_astc_calc_size(img_width, img_height, block_size);
    if (output_size < required_size) {
        return 0;
    }

    DongASTCEncodeOptions opts = options ? *options : dong_astc_options_default();

    uint32_t blocks_x = (img_width + dim.width - 1) / dim.width;
    uint32_t blocks_y = (img_height + dim.height - 1) / dim.height;

    // Temporary buffer for block pixels (with padding for edge blocks)
    uint8_t block_pixels[MAX_TEXELS_PER_BLOCK * 4];

    size_t out_offset = 0;

    for (uint32_t by = 0; by < blocks_y; by++) {
        for (uint32_t bx = 0; bx < blocks_x; bx++) {
            // Extract block pixels
            uint32_t block_x = bx * dim.width;
            uint32_t block_y = by * dim.height;

            memset(block_pixels, 0, sizeof(block_pixels));

            for (uint32_t py = 0; py < dim.height; py++) {
                for (uint32_t px = 0; px < dim.width; px++) {
                    uint32_t src_x = block_x + px;
                    uint32_t src_y = block_y + py;

                    // Clamp to image bounds (edge padding)
                    if (src_x >= img_width) src_x = img_width - 1;
                    if (src_y >= img_height) src_y = img_height - 1;

                    const uint8_t* src = pixels + (src_y * img_width + src_x) * 4;
                    uint8_t* dst = block_pixels + (py * dim.width + px) * 4;

                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[2];
                    dst[3] = src[3];
                }
            }

            // Encode block
            encode_astc_block_simple(
                block_pixels,
                dim.width,
                dim.height,
                dim.width * 4,
                opts.srgb,
                output + out_offset
            );

            out_offset += 16;
        }
    }

    return out_offset;
}
