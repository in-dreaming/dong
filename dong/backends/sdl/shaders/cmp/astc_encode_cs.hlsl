// =============================================================================
// ASTC Encoder Compute Shader
// =============================================================================
// GPU implementation of ASTC texture compression
// Based on Unity ASTC GPU Encoder
// Supports block sizes: 4x4, 5x5, 6x6, 8x8
// =============================================================================

#include "astc_common.hlsl"

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
    uint g_blockWidth;   // 4, 5, 6, or 8
    uint g_blockHeight;  // 4, 5, 6, or 8
    uint g_pad0;
    uint g_pad1;
};

// =============================================================================
// ASTC Block Mode Encoding
// =============================================================================

// Get block mode bits for different block sizes
// Block mode encodes weight grid dimensions and quantization level
uint getBlockMode(uint blockW, uint blockH)
{
    // ASTC block mode encoding (simplified)
    // These modes give good balance between quality and encoding complexity
    if (blockW == 4 && blockH == 4)
        return 0x0053;  // 4x4 weights, 8-level (3-bit) quantization
    if (blockW == 5 && blockH == 5)
        return 0x00F3;  // 5x5 weights, 8-level quantization
    if (blockW == 6 && blockH == 6)
        return 0x0108;  // 6x6 weights, 4-level (2-bit) quantization
    if (blockW == 8 && blockH == 8)
        return 0x0188;  // 8x8 weights, 4-level quantization

    return 0x0053;  // Default to 4x4
}

// Get color endpoint mode (CEM)
// CEM 12 = RGB Direct, 8 bits per channel
uint getColorEndpointMode()
{
    return 12;  // RGB Direct encoding
}

// =============================================================================
// ASTC 4x4 Block Encoding
// =============================================================================
uint4 encodeASTCBlock4x4(float3 minColor, float3 maxColor, uint weights[16])
{
    // Quantize endpoints to 8 bits (RGB888)
    uint3 minU = uint3(
        clampU((uint)(minColor.r * 255.0 + 0.5), 0, 255),
        clampU((uint)(minColor.g * 255.0 + 0.5), 0, 255),
        clampU((uint)(minColor.b * 255.0 + 0.5), 0, 255)
    );
    uint3 maxU = uint3(
        clampU((uint)(maxColor.r * 255.0 + 0.5), 0, 255),
        clampU((uint)(maxColor.g * 255.0 + 0.5), 0, 255),
        clampU((uint)(maxColor.b * 255.0 + 0.5), 0, 255)
    );

    uint4 result = uint4(0, 0, 0, 0);

    // Block mode (11 bits) + partition count (2 bits) + CEM (4 bits)
    // Block mode 0x0053: 4x4 weights, 8 levels, single plane
    result.x = 0x00010053;  // Mode + 1 partition + CEM 12

    // Pack endpoints (RGB888, interleaved min/max)
    // [17-24]: minR, [25-32]: maxR
    result.x |= (minU.r << 17);
    result.x |= (maxU.r << 25);

    // [33-40]: minG, [41-48]: maxG
    result.y = (maxU.r >> 7);
    result.y |= (minU.g << 1);
    result.y |= (maxU.g << 9);

    // [49-56]: minB, [57-64]: maxB
    result.y |= (minU.b << 17);
    result.y |= (maxU.b << 25);
    result.z = (maxU.b >> 7);

    // Pack weights (3 bits each, bit-reversed, stored from bit 127 downward)
    // Weight packing for 4x4: 16 weights * 3 bits = 48 bits
    // Weights are stored in reverse bit order within each weight

    [unroll]
    for (int i = 0; i < 10; i++)
    {
        uint w = reverseBits3(weights[i]);
        result.w |= w << (29 - i * 3);
    }

    // Weight 10 spans uint3 boundary
    uint w10 = reverseBits3(weights[10]);
    result.w |= (w10 >> 1);
    result.z |= (w10 & 1) << 31;

    [unroll]
    for (int j = 11; j < 16; j++)
    {
        uint w = reverseBits3(weights[j]);
        result.z |= w << (28 - (j - 11) * 3);
    }

    return result;
}

// =============================================================================
// ASTC 5x5 Block Encoding
// =============================================================================
uint4 encodeASTCBlock5x5(float3 minColor, float3 maxColor, uint weights[25])
{
    // Quantize endpoints to 6 bits (RGB666)
    uint3 minU = uint3(
        clampU((uint)(minColor.r * 63.0 + 0.5), 0, 63),
        clampU((uint)(minColor.g * 63.0 + 0.5), 0, 63),
        clampU((uint)(minColor.b * 63.0 + 0.5), 0, 63)
    );
    uint3 maxU = uint3(
        clampU((uint)(maxColor.r * 63.0 + 0.5), 0, 63),
        clampU((uint)(maxColor.g * 63.0 + 0.5), 0, 63),
        clampU((uint)(maxColor.b * 63.0 + 0.5), 0, 63)
    );

    uint4 result = uint4(0, 0, 0, 0);

    // Block mode for 5x5
    result.x = 0x000100F3;

    // Pack endpoints (RGB666)
    uint3 shiftedMin = minU << uint3(17, 29, 9);
    uint3 shiftedMax = maxU << uint3(23, 3, 15);

    result.x |= shiftedMin.r | shiftedMin.g | shiftedMax.r;
    result.y = minU.g >> 3;
    result.y |= shiftedMax.g | shiftedMin.b | shiftedMax.b;

    // Pack weights (3 bits each for 5x5 = 75 bits)
    [unroll]
    for (int i = 0; i < 10; i++)
    {
        uint w = reverseBits3(weights[i]);
        result.w |= w << (29 - i * 3);
    }

    // Remaining weights span boundaries
    uint w10 = reverseBits3(weights[10]);
    result.w |= (w10 >> 1);
    result.z |= (w10 & 1) << 31;

    [unroll]
    for (int j = 11; j < 21; j++)
    {
        uint w = reverseBits3(weights[j]);
        if (j < 21)
            result.z |= w << (28 - (j - 11) * 3);
    }

    uint w21 = reverseBits3(weights[21]);
    result.z |= (w21 >> 2);
    result.y |= (w21 & 3) << 30;

    [unroll]
    for (int k = 22; k < 25; k++)
    {
        uint w = reverseBits3(weights[k]);
        result.y |= w << (27 - (k - 22) * 3);
    }

    return result;
}

// =============================================================================
// ASTC 6x6 Block Encoding (2-bit weights)
// =============================================================================
uint4 encodeASTCBlock6x6(float3 minColor, float3 maxColor, uint weights[36])
{
    // 6x6 uses quint encoding for endpoints (more complex)
    // Simplified: use 4-bit per channel with quint
    uint3 minU = uint3(
        clampU((uint)(minColor.r * 79.0 + 0.5), 0, 79),
        clampU((uint)(minColor.g * 79.0 + 0.5), 0, 79),
        clampU((uint)(minColor.b * 79.0 + 0.5), 0, 79)
    );
    uint3 maxU = uint3(
        clampU((uint)(maxColor.r * 79.0 + 0.5), 0, 79),
        clampU((uint)(maxColor.g * 79.0 + 0.5), 0, 79),
        clampU((uint)(maxColor.b * 79.0 + 0.5), 0, 79)
    );

    uint4 result = uint4(0, 0, 0, 0);

    // Block mode for 6x6
    result.x = 0x00010108;

    // Simplified endpoint packing
    result.x |= (minU.r & 0xF) << 17;
    result.x |= (maxU.r & 0xF) << 21;
    result.x |= (minU.g & 0xF) << 25;
    result.x |= (maxU.g & 0x3) << 29;

    result.y = (maxU.g >> 2) & 0x3;
    result.y |= (minU.b & 0xF) << 2;
    result.y |= (maxU.b & 0xF) << 6;

    // Pack weights (2 bits each for 6x6 = 72 bits)
    [unroll]
    for (int i = 0; i < 16; i++)
    {
        uint w = reverseBits2(weights[i]);
        result.w |= w << (30 - i * 2);
    }

    [unroll]
    for (int j = 16; j < 32; j++)
    {
        uint w = reverseBits2(weights[j]);
        result.z |= w << (62 - j * 2);
    }

    [unroll]
    for (int k = 32; k < 36; k++)
    {
        uint w = reverseBits2(weights[k]);
        result.y |= w << (94 - k * 2);
    }

    return result;
}

// =============================================================================
// ASTC 8x8 Block Encoding (2-bit weights)
// =============================================================================
uint4 encodeASTCBlock8x8(float3 minColor, float3 maxColor, uint weights[64])
{
    // Quantize endpoints
    uint3 minU = uint3(
        clampU((uint)(minColor.r * 255.0 + 0.5), 0, 255),
        clampU((uint)(minColor.g * 255.0 + 0.5), 0, 255),
        clampU((uint)(minColor.b * 255.0 + 0.5), 0, 255)
    );
    uint3 maxU = uint3(
        clampU((uint)(maxColor.r * 255.0 + 0.5), 0, 255),
        clampU((uint)(maxColor.g * 255.0 + 0.5), 0, 255),
        clampU((uint)(maxColor.b * 255.0 + 0.5), 0, 255)
    );

    uint4 result = uint4(0, 0, 0, 0);

    // Block mode for 8x8
    result.x = 0x00010188;

    // Pack endpoints
    result.x |= (minU.r << 17);
    result.x |= (maxU.r << 25);
    result.y = (maxU.r >> 7);
    result.y |= (minU.g << 1);
    result.y |= (maxU.g << 9);
    result.y |= (minU.b << 17);
    result.y |= (maxU.b << 25);
    result.z = (maxU.b >> 7);

    // Pack weights (2 bits each for 8x8 = 128 bits, but we only have 64 bits left)
    // This is a simplified encoding that drops some weights

    // Pack first 16 weights into result.w
    [unroll]
    for (int i = 0; i < 16; i++)
    {
        uint w = reverseBits2(weights[i]);
        result.w |= w << (30 - i * 2);
    }

    // Pack weights 16-31 into result.z (remaining bits)
    [unroll]
    for (int j = 16; j < 32; j++)
    {
        uint w = reverseBits2(weights[j]);
        uint bitPos = (31 - j) * 2;
        if (bitPos < 32)
            result.z |= w << bitPos;
    }

    return result;
}

// =============================================================================
// Main Compression Function
// =============================================================================
uint4 compressBlockASTC(
    in float3 pixels[MAX_TEXELS],
    uint numPixels,
    uint blockW,
    uint blockH
)
{
    // Find min/max colors
    float3 minColor, maxColor;
    findBlockMinMax(pixels, numPixels, minColor, maxColor);

    // Get weight range for this block size
    uint weightRange = getWeightRange(blockW, blockH);

    // Compute weights
    float3 range = maxColor - minColor;
    float scale = (float)weightRange / max(1e-5, dot(range, range));
    float3 scaledRange = range * scale;
    float bias = (dot(minColor, minColor) - dot(maxColor, minColor)) * scale;

    uint weights[MAX_TEXELS];
    for (uint i = 0; i < numPixels; i++)
    {
        float t = dot(pixels[i], scaledRange) + bias;
        weights[i] = clampU((uint)(t + 0.5), 0, weightRange);
    }

    // Normalize colors to quantized range
    float3 qMin = round(saturate3(minColor) * 255.0) / 255.0;
    float3 qMax = round(saturate3(maxColor) * 255.0) / 255.0;

    // Encode based on block size
    if (blockW == 4 && blockH == 4)
    {
        uint w16[16];
        [unroll] for (int j = 0; j < 16; j++) w16[j] = weights[j];
        return encodeASTCBlock4x4(qMin, qMax, w16);
    }
    else if (blockW == 5 && blockH == 5)
    {
        uint w25[25];
        [unroll] for (int j = 0; j < 25; j++) w25[j] = weights[j];
        return encodeASTCBlock5x5(qMin, qMax, w25);
    }
    else if (blockW == 6 && blockH == 6)
    {
        uint w36[36];
        [unroll] for (int j = 0; j < 36; j++) w36[j] = weights[j];
        return encodeASTCBlock6x6(qMin, qMax, w36);
    }
    else // 8x8
    {
        return encodeASTCBlock8x8(qMin, qMax, weights);
    }
}

// =============================================================================
// Compute Shader Entry Point
// =============================================================================
[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint bx = dispatchThreadID.x;
    uint by = dispatchThreadID.y;

    if (bx >= g_blocksX || by >= g_blocksY)
        return;

    // Load block pixels
    uint blockStartX = bx * g_blockWidth;
    uint blockStartY = by * g_blockHeight;
    uint numPixels = g_blockWidth * g_blockHeight;

    float3 pixels[MAX_TEXELS];

    for (uint py = 0; py < g_blockHeight; py++)
    {
        for (uint px = 0; px < g_blockWidth; px++)
        {
            uint srcX = min(blockStartX + px, g_srcWidth - 1);
            uint srcY = min(blockStartY + py, g_srcHeight - 1);
            float4 color = g_srcTexture.Load(int3(srcX, srcY, 0));
            pixels[py * g_blockWidth + px] = color.rgb;
        }
    }

    // Compress block
    uint4 compressed = compressBlockASTC(pixels, numPixels, g_blockWidth, g_blockHeight);

    // Write result
    uint blockIndex = by * g_blocksX + bx;
    g_dstBuffer[blockIndex] = compressed;
}
