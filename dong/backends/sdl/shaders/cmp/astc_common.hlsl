// =============================================================================
// ASTC Common Utilities
// =============================================================================
// Based on Unity ASTC GPU Encoder
// Adapted for Dong Engine GPU compression
// =============================================================================

#ifndef ASTC_COMMON_HLSL
#define ASTC_COMMON_HLSL

// ASTC block sizes supported
// 4x4:  16 texels, 3-bit weights (8 levels)
// 5x5:  25 texels, 3-bit weights
// 6x6:  36 texels, 2-bit weights (4 levels)
// 8x8:  64 texels, 2-bit weights

#define ASTC_BLOCK_BYTES 16
#define MAX_TEXELS 64

// Helper functions
float3 saturate3(float3 v) { return clamp(v, 0.0, 1.0); }
int clampI(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
uint clampU(uint v, uint lo, uint hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Get weight range based on block size
uint getWeightRange(uint blockW, uint blockH)
{
    uint texels = blockW * blockH;
    if (texels <= 16) return 7;   // 3-bit weights (8 levels: 0-7)
    if (texels <= 25) return 7;   // 3-bit weights
    if (texels <= 36) return 3;   // 2-bit weights (4 levels: 0-3)
    return 3;                      // 2-bit weights for 64 texels
}

// Reverse bits for ASTC weight encoding
uint reverseBits3(uint v)
{
    // Reverse 3 bits: 0->0, 1->4, 2->2, 3->6, 4->1, 5->5, 6->3, 7->7
    static const uint REV_BITS_3[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
    return REV_BITS_3[v & 7];
}

uint reverseBits2(uint v)
{
    // Reverse 2 bits: 0->0, 1->2, 2->1, 3->3
    static const uint REV_BITS_2[4] = { 0, 2, 1, 3 };
    return REV_BITS_2[v & 3];
}

// Find bounding box for block pixels
void findBlockMinMax(
    in float3 pixels[MAX_TEXELS],
    in uint numPixels,
    out float3 minColor,
    out float3 maxColor
)
{
    minColor = pixels[0];
    maxColor = pixels[0];

    for (uint i = 1; i < numPixels; i++)
    {
        minColor = min(minColor, pixels[i]);
        maxColor = max(maxColor, pixels[i]);
    }
}

// Quantize endpoint to specified range
uint3 quantizeEndpoint(float3 color, uint range)
{
    return uint3(
        clampU((uint)(saturate(color.r) * range + 0.5), 0, range),
        clampU((uint)(saturate(color.g) * range + 0.5), 0, range),
        clampU((uint)(saturate(color.b) * range + 0.5), 0, range)
    );
}

// Compute interpolation weight
uint computeWeight(float3 color, float3 e0, float3 e1, uint range)
{
    float3 d = e1 - e0;
    float lenSq = dot(d, d);

    if (lenSq < 1e-10)
        return 0;

    float t = saturate(dot(color - e0, d) / lenSq);
    return clampU((uint)(t * range + 0.5), 0, range);
}

#endif // ASTC_COMMON_HLSL
