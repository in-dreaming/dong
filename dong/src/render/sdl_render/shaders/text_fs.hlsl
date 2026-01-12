// 关键：Fragment shader 的纹理和采样器必须使用 space2
// Uniform buffer 必须使用 space3
// 这是 SDL_GPU 对 SPIR-V/Vulkan 的要求
Texture2D msdfTexture : register(t0, space2);
SamplerState msdfSampler : register(s0, space2);

struct GlyphInstanceData {
    float4 rect;
    float4 uvRect;
    float4 color;
    float4 params;
};

// 布局必须与 Vertex Shader (space1) 和 C++ TextBatchUniformData 完全一致
cbuffer TextUniforms : register(b0, space3) {
    float4 uViewport;           // offset 0, size 16
    float4 uTransform[2];       // offset 16, size 32
    float4 uClipRects[4];       // offset 48, size 64
    float4 uClipRadii;          // offset 112, size 16
    float4 uClipMeta;           // offset 128, size 16
    float4 uGlyphData[244];     // offset 144, size 3904 (61 glyphs * 4 float4)
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
    float4 params : TEXCOORD2;
    float4 debug : TEXCOORD3;
};

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float3 toLinear(float3 col, float gam) {
    return pow(saturate(col), float3(gam, gam, gam));
}

float3 toSRGB(float3 col, float gam) {
    float invGamma = 1.0 / gam;
    return pow(saturate(col), float3(invGamma, invGamma, invGamma));
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    float2 q = abs(local) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

bool discardByClip(float2 px) {
    uint clipCount = (uint)uClipMeta.x;
    for (uint i = 0; i < clipCount; ++i) {
        float rad = uClipRadii[i];
        float4 rc = uClipRects[i];
        if (rad <= 0.0f) {
            if (px.x < rc.x || px.x > rc.z || px.y < rc.y || px.y > rc.w) {
                return true;
            }
            continue;
        }
        if (sdRoundedClip(px, rc, rad) > 0.0f) {
            return true;
        }
    }
    return false;
}

// 使用 fwidth 动态计算 screenPxRange，实现更精确的抗锯齿
// params.x = 预计算的 screenPxRange（如果 < 2.0 表示需要模糊效果）
// params.y = distance_range / msdf_texture_size（用于 fwidth 计算）
float calcScreenPxRange(float2 uv, float precomputed, float unitRange) {
    // 使用 fwidth 计算 UV 在屏幕空间的变化率
    // fwidth(uv) 返回 uv 在相邻像素间的变化量
    float2 screenTexSize = float2(1.0, 1.0) / fwidth(uv);
    
    // unitRange = distance_range / texture_size
    // screenPxRange = unitRange * screenTexSize（取平均）
    float dynamicRange = 0.5 * (unitRange * screenTexSize.x + unitRange * screenTexSize.y);
    
    // 如果 precomputed < 2.0，这是一个模糊/发光效果的信号
    // 在这种情况下，使用较小的 screenPxRange 来产生柔和边缘
    if (precomputed < 2.0) {
        // 对于模糊效果，使用 precomputed 值（可能很小）与动态计算值的混合
        // 但限制最大值，以确保边缘足够柔和
        return min(max(dynamicRange * precomputed, 0.5), 2.0);
    }
    
    // 正常文本渲染：确保 screenPxRange 至少为 2.0，以获得良好的抗锯齿效果
    // msdfgen 官方建议 screenPxRange >= 2
    return max(max(dynamicRange, precomputed), 2.0);
}

// 计算 MSDF 的 opacity，使用改进的抗锯齿算法
float calcMSDFOpacity(float3 msdf, float screenPxRange) {
    float sd = median(msdf.r, msdf.g, msdf.b);
    float screenPxDistance = screenPxRange * (sd - 0.5);
    
    // 使用 smoothstep 实现更平滑的抗锯齿过渡
    // 扩展过渡范围以获得更柔和的边缘
    return smoothstep(-0.5, 0.5, screenPxDistance);
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    // 采样 MSDF 纹理
    float3 msdf = msdfTexture.Sample(msdfSampler, input.uv).rgb;
    float sd = median(msdf.r, msdf.g, msdf.b);
    
    // 使用 fwidth 动态计算 screenPxRange
    float precomputedRange = input.params.x;
    float unitRange = input.params.y;
    float screenPxRange = calcScreenPxRange(input.uv, precomputedRange, unitRange);
    
    // 计算 opacity
    float screenPxDistance = screenPxRange * (sd - 0.5);
    float opacity = smoothstep(-0.5, 0.5, screenPxDistance);
    
    // 输出颜色
    float4 result;
    result.rgb = input.color.rgb;
    result.a = input.color.a * opacity;
    
    return result;
}
