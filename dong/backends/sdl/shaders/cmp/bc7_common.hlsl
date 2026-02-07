// =============================================================================
// BC7 Common Utilities
// =============================================================================
// Based on AMD Compressonator (MIT License)
// Adapted for Dong Engine GPU compression
// =============================================================================

#ifndef BC7_COMMON_HLSL
#define BC7_COMMON_HLSL

// BC7 Mode 6 configuration:
// - 1 subset (no partitioning)
// - 7-bit endpoints per channel (RGBA)
// - 4-bit indices (16 levels)
// - 1 p-bit per endpoint
// Total: 1 mode bit + 56 endpoint bits + 2 p-bits + 63 index bits = 122 bits (fits in 128)

#define BC7_BLOCK_SIZE 16
#define BC7_MODE6_CLUSTERS 16
#define BC7_MODE6_ENDPOINT_BITS 7
#define BC7_MODE6_INDEX_BITS 4

// Helper functions
float3 saturate3(float3 v) { return clamp(v, 0.0, 1.0); }
float4 saturate4(float4 v) { return clamp(v, 0.0, 1.0); }

int clampI(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
uint clampU(uint v, uint lo, uint hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Quantize float [0,1] to N-bit integer
uint quantize(float v, uint bits)
{
    uint maxVal = (1u << bits) - 1u;
    return clampU((uint)(v * maxVal + 0.5), 0, maxVal);
}

// Unquantize N-bit integer to float [0,1]
float unquantize(uint v, uint bits)
{
    uint maxVal = (1u << bits) - 1u;
    return (float)v / (float)maxVal;
}

// BC7 Mode 6 endpoint quantization (7 bits + 1 p-bit = 8 bits effective)
uint4 quantizeEndpoint7bit(float4 color)
{
    // Quantize to 7 bits (0-127)
    return uint4(
        quantize(color.r, 7),
        quantize(color.g, 7),
        quantize(color.b, 7),
        quantize(color.a, 7)
    );
}

// Compute interpolation weight for 4-bit indices (16 levels)
uint computeIndex4bit(float4 color, float4 e0, float4 e1)
{
    float4 d = e1 - e0;
    float lenSq = dot(d, d);

    if (lenSq < 1e-10)
        return 0;

    float4 p = color - e0;
    float t = saturate(dot(p, d) / lenSq);

    return clampU((uint)(t * 15.0 + 0.5), 0, 15);
}

// Find bounding box endpoints for 16 pixels
void findEndpointsBBox(in float4 pixels[16], out float4 minColor, out float4 maxColor)
{
    minColor = pixels[0];
    maxColor = pixels[0];

    [unroll]
    for (int i = 1; i < 16; i++)
    {
        minColor = min(minColor, pixels[i]);
        maxColor = max(maxColor, pixels[i]);
    }
}

// Encode BC7 Mode 6 block
// Layout (128 bits):
// [0-6]   : Mode (bit 6 set = 0x40 for mode 6)
// [7-62]  : Endpoints (7 bits * 8 = 56 bits: R0,R1,G0,G1,B0,B1,A0,A1)
// [63-64] : P-bits (2 bits)
// [65-127]: Indices (4 bits * 16 = 64 bits, first index uses 3 bits)
uint4 encodeBC7Mode6(uint4 ep0, uint4 ep1, uint indices[16])
{
    uint4 result = uint4(0, 0, 0, 0);

    // Mode 6 indicator (bit 6 set)
    result.x = 0x40;

    // Pack endpoints (7 bits each, interleaved R0,R1,G0,G1,B0,B1,A0,A1)
    // Bit positions: 7-13 (R0), 14-20 (R1), 21-27 (G0), 28-34 (G1)...

    // First uint32 [bits 0-31]:
    // [0-6]: mode = 0x40
    // [7-13]: ep0.r (7 bits)
    // [14-20]: ep1.r (7 bits)
    // [21-27]: ep0.g (7 bits)
    // [28-31]: ep1.g low 4 bits
    result.x |= (ep0.r & 0x7F) << 7;
    result.x |= (ep1.r & 0x7F) << 14;
    result.x |= (ep0.g & 0x7F) << 21;
    result.x |= (ep1.g & 0x0F) << 28;

    // Second uint32 [bits 32-63]:
    // [0-2]: ep1.g high 3 bits
    // [3-9]: ep0.b (7 bits)
    // [10-16]: ep1.b (7 bits)
    // [17-23]: ep0.a (7 bits)
    // [24-30]: ep1.a (7 bits)
    // [31]: p-bit 0
    result.y = (ep1.g >> 4) & 0x07;
    result.y |= (ep0.b & 0x7F) << 3;
    result.y |= (ep1.b & 0x7F) << 10;
    result.y |= (ep0.a & 0x7F) << 17;
    result.y |= (ep1.a & 0x7F) << 24;
    result.y |= 0u << 31; // p-bit 0 = 0

    // Third uint32 [bits 64-95]:
    // [0]: p-bit 1
    // [1-63]: indices (first index is 3 bits, rest are 4 bits)
    result.z = 0u; // p-bit 1 = 0

    // Pack indices - first index uses only 3 bits (MSB is implicit 0)
    // Index 0: bits 1-3 (3 bits)
    result.z |= (indices[0] & 0x07) << 1;

    // Indices 1-7: bits 4-31 (7 * 4 = 28 bits)
    [unroll]
    for (int i = 1; i < 8; i++)
    {
        result.z |= (indices[i] & 0x0F) << (1 + 3 + (i - 1) * 4);
    }

    // Fourth uint32 [bits 96-127]:
    // Indices 8-15: 8 * 4 = 32 bits
    [unroll]
    for (int j = 0; j < 8; j++)
    {
        result.w |= (indices[8 + j] & 0x0F) << (j * 4);
    }

    return result;
}

#endif // BC7_COMMON_HLSL
