// BC7 Encoder Implementation
// Simplified BC7 encoder for real-time texture compression
//
// BC7 has 8 modes (0-7) with different configurations.
// This implementation uses modes 6 (RGBA) and 4 (RGB with rotation) for speed.

#include "dong_bc_encoder.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

// =============================================================================
// Internal Constants
// =============================================================================

#define BC7_BLOCK_BYTES 16
#define BC6H_BLOCK_BYTES 16

// =============================================================================
// Color Utilities
// =============================================================================

typedef struct Color4f {
    float r, g, b, a;
} Color4f;

static inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static Color4f rgba8_to_color4f(const uint8_t* rgba) {
    Color4f c;
    c.r = rgba[0] / 255.0f;
    c.g = rgba[1] / 255.0f;
    c.b = rgba[2] / 255.0f;
    c.a = rgba[3] / 255.0f;
    return c;
}

// =============================================================================
// Endpoint Finding
// =============================================================================

static void find_endpoints_bbox(
    const uint8_t* pixels,  // 4x4 * 4 = 64 bytes
    Color4f* out_min,
    Color4f* out_max
) {
    Color4f cmin = {1.0f, 1.0f, 1.0f, 1.0f};
    Color4f cmax = {0.0f, 0.0f, 0.0f, 0.0f};

    for (int i = 0; i < 16; i++) {
        Color4f c = rgba8_to_color4f(pixels + i * 4);

        if (c.r < cmin.r) cmin.r = c.r;
        if (c.g < cmin.g) cmin.g = c.g;
        if (c.b < cmin.b) cmin.b = c.b;
        if (c.a < cmin.a) cmin.a = c.a;

        if (c.r > cmax.r) cmax.r = c.r;
        if (c.g > cmax.g) cmax.g = c.g;
        if (c.b > cmax.b) cmax.b = c.b;
        if (c.a > cmax.a) cmax.a = c.a;
    }

    *out_min = cmin;
    *out_max = cmax;
}

// =============================================================================
// Weight Computation
// =============================================================================

static int compute_weight_4bit(Color4f c, Color4f e0, Color4f e1) {
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

    return clampi((int)(t * 15.0f + 0.5f), 0, 15);
}

// =============================================================================
// BC7 Block Encoding
// =============================================================================

// BC7 Mode 6: RGBA, 1 subset, 4-bit color, 4-bit alpha, no partition
// This mode gives good quality for most RGBA content
static void encode_bc7_mode6(
    const uint8_t* pixels,
    uint8_t output[16]
) {
    memset(output, 0, 16);

    // Find endpoints
    Color4f e0, e1;
    find_endpoints_bbox(pixels, &e0, &e1);

    // Quantize endpoints to 7 bits (Mode 6 uses 7-bit endpoints)
    int e0_r = clampi((int)(e0.r * 127.0f + 0.5f), 0, 127);
    int e0_g = clampi((int)(e0.g * 127.0f + 0.5f), 0, 127);
    int e0_b = clampi((int)(e0.b * 127.0f + 0.5f), 0, 127);
    int e0_a = clampi((int)(e0.a * 127.0f + 0.5f), 0, 127);

    int e1_r = clampi((int)(e1.r * 127.0f + 0.5f), 0, 127);
    int e1_g = clampi((int)(e1.g * 127.0f + 0.5f), 0, 127);
    int e1_b = clampi((int)(e1.b * 127.0f + 0.5f), 0, 127);
    int e1_a = clampi((int)(e1.a * 127.0f + 0.5f), 0, 127);

    // Compute weights
    uint8_t weights[16];
    for (int i = 0; i < 16; i++) {
        Color4f c = rgba8_to_color4f(pixels + i * 4);
        weights[i] = (uint8_t)compute_weight_4bit(c, e0, e1);
    }

    // Mode 6 bit layout:
    // Bit 0-6: Mode (0b1000000 = 64)
    // Bits 7-62: Endpoints (7 bits * 8 = 56 bits)
    // Bits 63: P-bit 0
    // Bit 64: P-bit 1
    // Bits 65-128: Weights (4 bits * 16 = 64 bits)

    // Mode 6 indicator (bit 6 set)
    output[0] = 0x40;  // Mode 6

    // Pack endpoints (simplified - proper BC7 has specific bit ordering)
    // Endpoint 0: R G B A (7 bits each)
    // Endpoint 1: R G B A (7 bits each)

    // This is a simplified packing - actual BC7 has complex bit interleaving
    uint64_t ep_bits = 0;
    ep_bits |= (uint64_t)e0_r;
    ep_bits |= (uint64_t)e0_g << 7;
    ep_bits |= (uint64_t)e0_b << 14;
    ep_bits |= (uint64_t)e0_a << 21;
    ep_bits |= (uint64_t)e1_r << 28;
    ep_bits |= (uint64_t)e1_g << 35;
    ep_bits |= (uint64_t)e1_b << 42;
    ep_bits |= (uint64_t)e1_a << 49;

    // Store endpoints (starting at bit 7)
    output[0] |= (ep_bits & 0x1) << 7;
    output[1] = (ep_bits >> 1) & 0xFF;
    output[2] = (ep_bits >> 9) & 0xFF;
    output[3] = (ep_bits >> 17) & 0xFF;
    output[4] = (ep_bits >> 25) & 0xFF;
    output[5] = (ep_bits >> 33) & 0xFF;
    output[6] = (ep_bits >> 41) & 0xFF;
    output[7] = (ep_bits >> 49) & 0x7F;

    // P-bits (use 0 for simplicity)
    // Bits 63-64 are P-bits

    // Pack weights (4 bits each, 16 weights = 64 bits)
    uint64_t weight_bits = 0;
    for (int i = 0; i < 16; i++) {
        weight_bits |= (uint64_t)(weights[i] & 0xF) << (i * 4);
    }

    // Store weights (starting at bit 65, byte 8)
    output[8] = weight_bits & 0xFF;
    output[9] = (weight_bits >> 8) & 0xFF;
    output[10] = (weight_bits >> 16) & 0xFF;
    output[11] = (weight_bits >> 24) & 0xFF;
    output[12] = (weight_bits >> 32) & 0xFF;
    output[13] = (weight_bits >> 40) & 0xFF;
    output[14] = (weight_bits >> 48) & 0xFF;
    output[15] = (weight_bits >> 56) & 0xFF;
}

// =============================================================================
// BC6H Block Encoding (HDR)
// =============================================================================

// Half-float utilities
static float half_to_float(uint16_t h) {
    uint32_t sign = (h & 0x8000) << 16;
    uint32_t exp = (h >> 10) & 0x1F;
    uint32_t mant = h & 0x3FF;

    if (exp == 0) {
        if (mant == 0) {
            uint32_t f = sign;
            return *(float*)&f;
        }
        // Denormalized
        while ((mant & 0x400) == 0) {
            mant <<= 1;
            exp--;
        }
        exp++;
        mant &= ~0x400;
    } else if (exp == 31) {
        uint32_t f = sign | 0x7F800000 | (mant << 13);
        return *(float*)&f;
    }

    exp = exp + (127 - 15);
    mant = mant << 13;

    uint32_t f = sign | (exp << 23) | mant;
    return *(float*)&f;
}

static void encode_bc6h_block_simple(
    const uint16_t* pixels,  // 4x4 * 4 = 64 half-floats
    int is_signed,
    uint8_t output[16]
) {
    memset(output, 0, 16);

    // Find RGB min/max (ignore alpha for BC6H)
    float rmin = 65504.0f, rmax = 0.0f;
    float gmin = 65504.0f, gmax = 0.0f;
    float bmin = 65504.0f, bmax = 0.0f;

    for (int i = 0; i < 16; i++) {
        float r = half_to_float(pixels[i * 4 + 0]);
        float g = half_to_float(pixels[i * 4 + 1]);
        float b = half_to_float(pixels[i * 4 + 2]);

        if (r < rmin) rmin = r;
        if (r > rmax) rmax = r;
        if (g < gmin) gmin = g;
        if (g > gmax) gmax = g;
        if (b < bmin) bmin = b;
        if (b > bmax) bmax = b;
    }

    // BC6H uses various modes with different endpoint precisions
    // Mode 1 is a common choice: 10-bit endpoints, no partition

    // Simplified encoding - actual BC6H requires careful bit packing
    // For now, store a basic representation

    // Mode indicator (5 bits for mode 1)
    output[0] = 0x03;  // Mode 1 indicator

    // Quantize endpoints to 10 bits
    // BC6H operates on values up to 65504 (max half-float)
    float scale = 1023.0f / 65504.0f;

    int e0_r = clampi((int)(rmin * scale + 0.5f), 0, 1023);
    int e0_g = clampi((int)(gmin * scale + 0.5f), 0, 1023);
    int e0_b = clampi((int)(bmin * scale + 0.5f), 0, 1023);

    int e1_r = clampi((int)(rmax * scale + 0.5f), 0, 1023);
    int e1_g = clampi((int)(gmax * scale + 0.5f), 0, 1023);
    int e1_b = clampi((int)(bmax * scale + 0.5f), 0, 1023);

    // Pack endpoints (simplified)
    output[1] = e0_r & 0xFF;
    output[2] = ((e0_r >> 8) & 0x3) | ((e0_g & 0x3F) << 2);
    output[3] = ((e0_g >> 6) & 0xF) | ((e0_b & 0xF) << 4);
    output[4] = (e0_b >> 4) & 0x3F;

    output[5] = e1_r & 0xFF;
    output[6] = ((e1_r >> 8) & 0x3) | ((e1_g & 0x3F) << 2);
    output[7] = ((e1_g >> 6) & 0xF) | ((e1_b & 0xF) << 4);
    output[8] = (e1_b >> 4) & 0x3F;

    // Compute and pack weights
    float len_sq = (rmax - rmin) * (rmax - rmin) +
                   (gmax - gmin) * (gmax - gmin) +
                   (bmax - bmin) * (bmax - bmin);

    uint64_t weight_bits = 0;
    for (int i = 0; i < 16; i++) {
        float r = half_to_float(pixels[i * 4 + 0]);
        float g = half_to_float(pixels[i * 4 + 1]);
        float b = half_to_float(pixels[i * 4 + 2]);

        int w = 0;
        if (len_sq > 1e-10f) {
            float t = ((r - rmin) * (rmax - rmin) +
                       (g - gmin) * (gmax - gmin) +
                       (b - bmin) * (bmax - bmin)) / len_sq;
            t = clampf(t, 0.0f, 1.0f);
            w = clampi((int)(t * 15.0f + 0.5f), 0, 15);
        }

        weight_bits |= (uint64_t)(w & 0xF) << (i * 4);
    }

    // Store weights (last 8 bytes)
    output[8] |= (weight_bits & 0x3) << 6;
    output[9] = (weight_bits >> 2) & 0xFF;
    output[10] = (weight_bits >> 10) & 0xFF;
    output[11] = (weight_bits >> 18) & 0xFF;
    output[12] = (weight_bits >> 26) & 0xFF;
    output[13] = (weight_bits >> 34) & 0xFF;
    output[14] = (weight_bits >> 42) & 0xFF;
    output[15] = (weight_bits >> 50) & 0xFF;

    (void)is_signed;  // TODO: Handle signed format
}

// =============================================================================
// Public API Implementation
// =============================================================================

int dong_bc7_encode_block(
    const uint8_t pixels[64],
    const DongBCEncodeOptions* options,
    uint8_t output[16]
) {
    if (!pixels || !output) {
        return -1;
    }

    (void)options;  // Quality settings not used in simplified encoder

    encode_bc7_mode6(pixels, output);
    return 0;
}

size_t dong_bc7_encode_image(
    const uint8_t* pixels,
    uint32_t img_width,
    uint32_t img_height,
    const DongBCEncodeOptions* options,
    uint8_t* output,
    size_t output_size
) {
    if (!pixels || !output || img_width == 0 || img_height == 0) {
        return 0;
    }

    size_t required_size = dong_bc_calc_size(img_width, img_height, DONG_BC_FORMAT_BC7);
    if (output_size < required_size) {
        return 0;
    }

    uint32_t blocks_x = (img_width + 3) / 4;
    uint32_t blocks_y = (img_height + 3) / 4;

    uint8_t block_pixels[64];  // 4x4 * 4 bytes
    size_t out_offset = 0;

    for (uint32_t by = 0; by < blocks_y; by++) {
        for (uint32_t bx = 0; bx < blocks_x; bx++) {
            // Extract block
            uint32_t block_x = bx * 4;
            uint32_t block_y = by * 4;

            for (uint32_t py = 0; py < 4; py++) {
                for (uint32_t px = 0; px < 4; px++) {
                    uint32_t src_x = block_x + px;
                    uint32_t src_y = block_y + py;

                    // Clamp to image bounds
                    if (src_x >= img_width) src_x = img_width - 1;
                    if (src_y >= img_height) src_y = img_height - 1;

                    const uint8_t* src = pixels + (src_y * img_width + src_x) * 4;
                    uint8_t* dst = block_pixels + (py * 4 + px) * 4;

                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[2];
                    dst[3] = src[3];
                }
            }

            encode_bc7_mode6(block_pixels, output + out_offset);
            out_offset += 16;
        }
    }

    (void)options;
    return out_offset;
}

int dong_bc6h_encode_block(
    const uint16_t pixels[64],
    int is_signed,
    const DongBCEncodeOptions* options,
    uint8_t output[16]
) {
    if (!pixels || !output) {
        return -1;
    }

    (void)options;

    encode_bc6h_block_simple(pixels, is_signed, output);
    return 0;
}

size_t dong_bc6h_encode_image(
    const uint16_t* pixels,
    uint32_t img_width,
    uint32_t img_height,
    int is_signed,
    const DongBCEncodeOptions* options,
    uint8_t* output,
    size_t output_size
) {
    if (!pixels || !output || img_width == 0 || img_height == 0) {
        return 0;
    }

    size_t required_size = dong_bc_calc_size(img_width, img_height, DONG_BC_FORMAT_BC6H);
    if (output_size < required_size) {
        return 0;
    }

    uint32_t blocks_x = (img_width + 3) / 4;
    uint32_t blocks_y = (img_height + 3) / 4;

    uint16_t block_pixels[64];  // 4x4 * 4 half-floats
    size_t out_offset = 0;

    for (uint32_t by = 0; by < blocks_y; by++) {
        for (uint32_t bx = 0; bx < blocks_x; bx++) {
            uint32_t block_x = bx * 4;
            uint32_t block_y = by * 4;

            for (uint32_t py = 0; py < 4; py++) {
                for (uint32_t px = 0; px < 4; px++) {
                    uint32_t src_x = block_x + px;
                    uint32_t src_y = block_y + py;

                    if (src_x >= img_width) src_x = img_width - 1;
                    if (src_y >= img_height) src_y = img_height - 1;

                    const uint16_t* src = pixels + (src_y * img_width + src_x) * 4;
                    uint16_t* dst = block_pixels + (py * 4 + px) * 4;

                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[2];
                    dst[3] = src[3];
                }
            }

            encode_bc6h_block_simple(block_pixels, is_signed, output + out_offset);
            out_offset += 16;
        }
    }

    (void)options;
    return out_offset;
}
