// =============================================================================
// BC7 Encoder Compute Shader
// =============================================================================
// GPU implementation of BC7 Mode 6 texture compression
// Based on AMD Compressonator (MIT License)
// =============================================================================

#include "bc7_common.hlsl"

// Input texture
Texture2D<float4> g_srcTexture : register(t0);

// Output buffer (16 bytes = uint4 per block)
RWStructuredBuffer<uint4> g_dstBuffer : register(u0);

// Parameters
cbuffer CompressParams : register(b0)
{
    uint g_srcWidth;
    uint g_srcHeight;
    uint g_blocksX;
    uint g_blocksY;
    float g_quality;  // 0.0 = fast, 1.0 = quality
    uint g_pad0;
    uint g_pad1;
    uint g_pad2;
};

// Refine endpoints using least squares
void refineEndpoints(
    in float4 pixels[16],
    inout float4 ep0,
    inout float4 ep1,
    in uint indices[16]
)
{
    // Accumulate weighted sums
    float4 sum0 = float4(0, 0, 0, 0);
    float4 sum1 = float4(0, 0, 0, 0);
    float weight0 = 0;
    float weight1 = 0;

    [unroll]
    for (int i = 0; i < 16; i++)
    {
        float w = (float)indices[i] / 15.0;
        float w0 = 1.0 - w;
        float w1 = w;

        sum0 += pixels[i] * w0;
        sum1 += pixels[i] * w1;
        weight0 += w0;
        weight1 += w1;
    }

    if (weight0 > 0.001)
        ep0 = saturate4(sum0 / weight0);
    if (weight1 > 0.001)
        ep1 = saturate4(sum1 / weight1);
}

// Compute error for current encoding
float computeError(
    in float4 pixels[16],
    in float4 ep0,
    in float4 ep1,
    in uint indices[16]
)
{
    float totalError = 0;

    [unroll]
    for (int i = 0; i < 16; i++)
    {
        float t = (float)indices[i] / 15.0;
        float4 reconstructed = lerp(ep0, ep1, t);
        float4 diff = pixels[i] - reconstructed;
        totalError += dot(diff, diff);
    }

    return totalError;
}

// Main compression function
uint4 compressBlockBC7(in float4 pixels[16], float quality)
{
    // Find initial endpoints using bounding box
    float4 ep0, ep1;
    findEndpointsBBox(pixels, ep0, ep1);

    // Compute initial indices
    uint indices[16];
    [unroll]
    for (int i = 0; i < 16; i++)
    {
        indices[i] = computeIndex4bit(pixels[i], ep0, ep1);
    }

    // Refine endpoints (iterative improvement)
    int numIterations = (quality > 0.5) ? 2 : 1;

    [unroll]
    for (int iter = 0; iter < 2; iter++)
    {
        if (iter >= numIterations) break;

        // Refine endpoints based on current indices
        refineEndpoints(pixels, ep0, ep1, indices);

        // Recompute indices with new endpoints
        [unroll]
        for (int j = 0; j < 16; j++)
        {
            indices[j] = computeIndex4bit(pixels[j], ep0, ep1);
        }
    }

    // Quantize endpoints to 7 bits
    uint4 qep0 = quantizeEndpoint7bit(ep0);
    uint4 qep1 = quantizeEndpoint7bit(ep1);

    // Ensure endpoint order (for index compression)
    // If ep0 > ep1, swap them and invert indices
    uint luma0 = qep0.r + qep0.g * 2 + qep0.b;
    uint luma1 = qep1.r + qep1.g * 2 + qep1.b;

    if (luma0 > luma1)
    {
        uint4 temp = qep0;
        qep0 = qep1;
        qep1 = temp;

        [unroll]
        for (int k = 0; k < 16; k++)
        {
            indices[k] = 15 - indices[k];
        }
    }

    // Encode the block
    return encodeBC7Mode6(qep0, qep1, indices);
}

// Thread group: 8x8, each thread handles one 4x4 block
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint bx = dispatchThreadID.x;
    uint by = dispatchThreadID.y;

    if (bx >= g_blocksX || by >= g_blocksY)
        return;

    // Load 4x4 block
    uint blockStartX = bx * 4;
    uint blockStartY = by * 4;

    float4 pixels[16];

    [unroll]
    for (uint py = 0; py < 4; py++)
    {
        [unroll]
        for (uint px = 0; px < 4; px++)
        {
            uint srcX = min(blockStartX + px, g_srcWidth - 1);
            uint srcY = min(blockStartY + py, g_srcHeight - 1);
            pixels[py * 4 + px] = g_srcTexture.Load(int3(srcX, srcY, 0));
        }
    }

    // Compress block
    uint4 compressed = compressBlockBC7(pixels, g_quality);

    // Write result
    uint blockIndex = by * g_blocksX + bx;
    g_dstBuffer[blockIndex] = compressed;
}
