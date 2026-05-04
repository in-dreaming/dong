// =============================================================================
// common.hlsl - Shared shader utilities for dong GPU rendering
// =============================================================================
// This file contains common functions used across all fragment shaders,
// including color space conversion functions for HDR support.
// =============================================================================

#ifndef DONG_COMMON_HLSL
#define DONG_COMMON_HLSL

// =============================================================================
// Color Space Conversion Functions
// =============================================================================

// Convert sRGB color to linear color space
// Input: sRGB color values (0-1 range)
// Output: Linear color values for HDR rendering
float3 srgb_to_linear3(float3 srgb) {
    // sRGB uses piecewise gamma curve:
    // For values <= 0.04045: linear = srgb / 12.92
    // For values > 0.04045: linear = pow((srgb + 0.055) / 1.055, 2.4)
    // Using approximation with pow(x, 2.2) for simplicity
    return pow(max(srgb, float3(0.0, 0.0, 0.0)), float3(2.2, 2.2, 2.2));
}

// Convert linear color to sRGB color space
// Input: Linear color values
// Output: sRGB color values for SDR rendering
float3 linear_to_srgb3(float3 linear) {
    // Inverse of sRGB gamma curve
    // Using approximation with pow(x, 1/2.2) for simplicity
    return pow(max(linear, float3(0.0, 0.0, 0.0)), float3(1.0/2.2, 1.0/2.2, 1.0/2.2));
}

// Convert sRGB color with alpha to linear color space
float4 srgb_to_linear4(float4 srgb) {
    return float4(srgb_to_linear3(srgb.rgb), srgb.a);
}

// Convert linear color with alpha to sRGB color space
float4 linear_to_srgb4(float4 linear) {
    return float4(linear_to_srgb3(linear.rgb), linear.a);
}

// =============================================================================
// HDR Output Helper
// =============================================================================
// When rendering to HDR Extended Linear (R16G16B16A16_FLOAT) swapchain,
// colors should be output in linear space.
// When rendering to SDR (R8G8B8A8_UNORM) swapchain,
// colors should remain in sRGB space (hardware does the conversion).
//
// Since CSS colors are specified in sRGB space, we need to:
// - SDR mode: Output sRGB colors directly (no conversion)
// - HDR mode: Convert sRGB colors to linear before output
//
// Usage in fragment shaders:
//   float4 color = getSRGBColor(); // Color from uniforms/textures in sRGB
//   return output_color_for_swapchain(color, uRenderParams.x);
// =============================================================================

// Output color for the current swapchain format
// hdr_mode: 0.0 = SDR, 1.0 = HDR
float4 output_color_for_swapchain(float4 srgb_color, float hdr_mode) {
    if (hdr_mode > 0.5) {
        // HDR mode: convert sRGB to linear
        return srgb_to_linear4(srgb_color);
    }
    // SDR mode: output sRGB directly
    return srgb_color;
}

#endif // DONG_COMMON_HLSL
