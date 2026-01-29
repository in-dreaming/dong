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

// 计算 screenPxRange
// params.x = 预计算的 screenPxRange（CPU端根据 font_size/msdf_scale 计算）
// 这是更准确的值，因为CPU知道确切的渲染参数
float calcScreenPxRange(float2 uv, float precomputed, float unitRange) {
    // 优先使用CPU预计算的值，因为它基于确切的字体参数
    // precomputed = atlas_range * (pixel_scale / msdf_scale)
    
    // 可选：用 fwidth 进行微调，但保持预计算值的主导地位
    // 这可以处理缩放变换的情况
    float2 fw = fwidth(uv);
    float texSize = max(length(fw), 1e-6);
    float screenTexSize = 1.0 / texSize;
    float dynamicRange = unitRange * screenTexSize;
    
    // 混合：70% 预计算值 + 30% 动态值
    // 这样既能保持准确性，又能响应变换
    float blended = precomputed * 0.7 + dynamicRange * 0.3;
    
    // 限制范围：最小1.5（保证基本抗锯齿），最大8（避免过度模糊）
    return clamp(blended, 1.5, 8.0);
}

// 计算 MSDF 的 opacity
// 综合优化：Stem Darkening + Gamma 校正 + 自适应抗锯齿
float calcMSDFOpacity(float3 msdf, float screenPxRange) {
    float sd = median(msdf.r, msdf.g, msdf.b);
    float screenPxDistance = screenPxRange * (sd - 0.5);
    
    // === Stem Darkening (笔画加深) ===
    // 小字体需要加粗笔画以提高可读性
    // 通过添加正偏移，让更多像素被视为"在字形内"
    float stemDarkening = 0.0;
    if (screenPxRange < 2.5) {
        // 小字体：显著加粗
        stemDarkening = 0.25;
    } else if (screenPxRange < 4.0) {
        // 中等字体：轻微加粗
        stemDarkening = 0.15;
    }
    // 大字体不需要加粗
    
    // === 自适应 smoothstep 范围 ===
    float range = 0.5;
    if (screenPxRange < 2.0) {
        range = 0.3;   // 小字体：较窄范围保持锐利
    } else if (screenPxRange < 3.0) {
        range = 0.4;   // 中等字体
    } else if (screenPxRange > 4.0) {
        range = 0.6;   // 大字体：较宽范围更平滑
    }
    
    // 应用 stem darkening 偏移
    float adjustedDistance = screenPxDistance + stemDarkening;
    
    // 计算基础 opacity
    float opacity = smoothstep(-range, range, adjustedDistance);
    
    // === Gamma 校正 ===
    // 在 sRGB 空间进行 gamma 调整，使小字体更清晰
    // 参考：浏览器通常使用 gamma 1.8-2.2
    const float gamma = 1.6;  // 较小的 gamma 值会增加对比度
    opacity = pow(opacity, 1.0 / gamma);
    
    return opacity;
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    // 采样 MSDF 纹理
    float3 msdf = msdfTexture.Sample(msdfSampler, input.uv).rgb;
    
    // 使用 fwidth 动态计算 screenPxRange
    float precomputedRange = input.params.x;
    float unitRange = input.params.y;
    float screenPxRange = calcScreenPxRange(input.uv, precomputedRange, unitRange);
    
    // 计算 opacity（使用改进的抗锯齿算法）
    float opacity = calcMSDFOpacity(msdf, screenPxRange);
    
    // 输出颜色
    float4 result;
    result.rgb = input.color.rgb;
    result.a = input.color.a * opacity;
    
    return result;
}
