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
};

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
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

// screenPxRange: CPU 预计算 = atlas_range * (pixel_scale / msdf_scale)
// 直接使用，不混合 fwidth 动态值（fwidth 在三角形边缘有噪声）
float calcScreenPxRange(float precomputed) {
    return max(precomputed, 2.0);
}

// 标准 MSDF opacity 解码
float calcMSDFOpacity(float3 msdf, float screenPxRange) {
    float sd = median(msdf.r, msdf.g, msdf.b);
    float screenPxDistance = screenPxRange * (sd - 0.5);
    return clamp(screenPxDistance + 0.5, 0.0, 1.0);
}

// 4-tap 超采样：旋转网格采样 MSDF，独立解码后平均 opacity。
// 仅在 screenPxRange < 2.5（小字体高缩小比）时激活，消除残余锯齿。
float calcMSDFOpacity4x(float2 uv, float screenPxRange) {
    // 旋转网格偏移（约 26.6° 旋转，0.375 texel）
    float2 texelSize = fwidth(uv);
    float ox = texelSize.x * 0.375;
    float oy = texelSize.y * 0.375;

    float3 s0 = msdfTexture.Sample(msdfSampler, uv + float2(-ox, -oy)).rgb;
    float3 s1 = msdfTexture.Sample(msdfSampler, uv + float2( ox, -oy)).rgb;
    float3 s2 = msdfTexture.Sample(msdfSampler, uv + float2(-ox,  oy)).rgb;
    float3 s3 = msdfTexture.Sample(msdfSampler, uv + float2( ox,  oy)).rgb;

    float a0 = calcMSDFOpacity(s0, screenPxRange);
    float a1 = calcMSDFOpacity(s1, screenPxRange);
    float a2 = calcMSDFOpacity(s2, screenPxRange);
    float a3 = calcMSDFOpacity(s3, screenPxRange);

    return (a0 + a1 + a2 + a3) * 0.25;
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    // 计算 screenPxRange
    float precomputedRange = input.params.x;
    float screenPxRange = calcScreenPxRange(precomputedRange);

    // 计算 opacity：小字体用 4-tap 超采样消除残余锯齿，大字体用单次采样
    float opacity;
    if (screenPxRange < 2.5) {
        opacity = calcMSDFOpacity4x(input.uv, screenPxRange);
    } else {
        float3 msdf = msdfTexture.Sample(msdfSampler, input.uv).rgb;
        opacity = calcMSDFOpacity(msdf, screenPxRange);
    }
    
    // 输出颜色
    float4 result;
    result.rgb = input.color.rgb;
    result.a = input.color.a * opacity;
    
    return result;
}
