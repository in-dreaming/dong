#pragma once



// HLSL shader sources used by SDL_gpu backend.
// NOTE: These strings are compiled at runtime via SDL_shadercross (HLSL -> SPIR-V -> backend shader).

namespace dong::render::sdl_render::shaders {

// Rect
inline constexpr const char kRectVS[] = R"(
struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD0;
};

cbuffer RectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix: [m0,m1,m2,0] [m3,m4,m5,0]
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    
    // 计算本地坐标
    float2 pos = uRect.xy + local * uRect.zw;
    
    // 应用 2D transform: [x', y'] = [m0 m1 m2] [x]
    //                                [m3 m4 m5] [y]
    //                                           [1]
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;
    
    // 转换到 NDC
    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;
    
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.color = uColor;
    o.pixel = transformed;  // 使用变换后的坐标用于 clipping
    return o;
}
)";

inline constexpr const char kRectFS[] = R"(
struct PSInput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD0;
};

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer RectUniforms : register(b0, space3) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix: [m0,m1,m2,0] [m3,m4,m5,0]
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

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

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    // 直接输出 sRGB 颜色，不做 gamma 转换
    return input.color;
}
)";

// RoundRect
inline constexpr const char kRoundRectVS[] = R"(
struct VSOutput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD3;
};

cbuffer RoundRectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    
    // 计算本地坐标
    float2 pos = uRect.xy + local * uRect.zw;
    
    // 应用 2D transform
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;
    
    // 转换到 NDC
    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;
    
    float radius = uRadius.x;
    radius = min(radius, min(uRect.z, uRect.w) * 0.5 - 0.5);
    radius = max(radius, 0.0);
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.local = local;
    o.size = uRect.zw;
    o.radius = radius;
    o.color = uColor;
    o.pixel = transformed;  // 使用变换后的坐标
    return o;
}
)";

inline constexpr const char kRoundRectFS[] = R"(
struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD3;
};

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer RoundRectUniforms : register(b0, space3) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

float sdRoundRect(float2 p, float2 halfSize, float rad) {
    float2 q = abs(p) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    return sdRoundRect(local, halfSize, rad);
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

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    float2 size = input.size;
    float2 p = (input.local - 0.5) * size;
    float r = input.radius;
    float2 halfSize = size * 0.5;
    float dist = sdRoundRect(p, halfSize, r);
    const float aa = 1.0;
    float alpha = saturate(0.5 - dist / aa);
    float4 base = input.color;
    base.a *= alpha;
    // 直接输出 sRGB 颜色，不做 gamma 转换
    return base;
}
)";

// Shadow
inline constexpr const char kShadowVS[] = R"(
struct VSOutput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    nointerpolation float blur : TEXCOORD3;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD4;
};

cbuffer ShadowUniforms : register(b0, space1) {
    float4 uRect;
    float4 uRadius;   // x=corner radius, y=blur radius
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    
    float blur = uRadius.y;
    // 扩展绘制区域以容纳模糊
    float2 expanded_rect_pos = uRect.xy - blur;
    float2 expanded_rect_size = uRect.zw + blur * 2.0;
    
    float2 pos = expanded_rect_pos + local * expanded_rect_size;

    // 应用 2D transform
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;

    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;
    
    float radius = uRadius.x;
    radius = min(radius, min(uRect.z, uRect.w) * 0.5 - 0.5);
    radius = max(radius, 0.0);
    
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    // local 坐标相对于原始矩形（不是扩展后的）
    o.local = (local * expanded_rect_size - blur) / uRect.zw;
    o.size = uRect.zw;
    o.radius = radius;
    o.blur = blur;
    o.color = uColor;
    o.pixel = transformed;
    return o;
}
)";

inline constexpr const char kShadowFS[] = R"(
struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    nointerpolation float blur : TEXCOORD3;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD4;
};

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer ShadowUniforms : register(b0, space3) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

float sdRoundRect(float2 p, float2 halfSize, float rad) {
    float2 q = abs(p) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    return sdRoundRect(local, halfSize, rad);
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

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    
    float2 size = input.size;
    float2 p = (input.local - 0.5) * size;
    float r = input.radius;
    float blur = input.blur;
    float2 halfSize = size * 0.5;
    
    float dist = sdRoundRect(p, halfSize, r);
    
    // 使用 smoothstep 实现模糊效果
    // blur 为 0 时退化为硬边缘
    float sigma = max(blur, 0.5);
    float alpha = 1.0 - smoothstep(-sigma, sigma, dist);
    
    float4 base = input.color;
    base.a *= alpha;
    // 直接输出 sRGB 颜色，不做 gamma 转换
    return base;
}
)";

// Image
inline constexpr const char kImageVS[] = R"(
struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
    float2 pixel : TEXCOORD1;
};

cbuffer ImageUniforms : register(b0, space1) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uTint;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

VSOutput main(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    float2 pos = uRect.xy + local * uRect.zw;

    // 应用 2D transform
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;

    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;
    float2 uv = float2(lerp(uUVRect.x, uUVRect.z, local.x), lerp(uUVRect.y, uUVRect.w, local.y));
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = uv;
    o.tint = uTint;
    o.pixel = transformed;
    return o;
}
)";

inline constexpr const char kImageFS[] = R"(
// 关键：Fragment shader 的纹理和采样器必须使用 space2
// Uniform buffer 必须使用 space3
Texture2D imageTexture : register(t0, space2);
SamplerState imageSampler : register(s0, space2);

cbuffer ImageUniforms : register(b0, space3) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uTint;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
    float2 pixel : TEXCOORD1;
};

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

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    float4 tex = imageTexture.Sample(imageSampler, input.uv);
    // 直接在 sRGB 空间做 tint 乘法（简化处理）
    float3 tinted = tex.rgb * input.tint.rgb;
    float alpha = tex.a * input.tint.a;
    return float4(tinted, alpha);
}
)";

// Text (MSDF)
inline constexpr const char kTextVS[] = R"(
struct GlyphInstanceData {
    float4 rect;
    float4 uvRect;
    float4 color;
    float4 params;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
    float4 params : TEXCOORD2;
    float4 debug : TEXCOORD3;  // 调试信息
};

// SDL_GPU 规范：Vertex shader 的 uniform buffer 使用 space1
// 布局必须与 C++ TextBatchUniformData 完全一致
// 总大小 <= 4096 bytes 以兼容大多数 GPU
cbuffer TextUniforms : register(b0, space1) {
    float4 uViewport;           // offset 0, size 16
    float4 uTransform[2];       // offset 16, size 32
    float4 uClipRects[4];       // offset 48, size 64
    float4 uClipRadii;          // offset 112, size 16
    float4 uClipMeta;           // offset 128, size 16
    float4 uGlyphData[244];     // offset 144, size 3904 (61 glyphs * 4 float4)
};

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }

    // 每个 glyph 占用 4 个 float4 (rect, uvRect, color, params)
    // glyph 数据从 uGlyphData[0] 开始
    uint base = instanceID * 4;
    float4 rect = uGlyphData[base + 0];
    float4 uvRect = uGlyphData[base + 1];
    float4 color = uGlyphData[base + 2];
    float4 params = uGlyphData[base + 3];

    float2 pos = rect.xy + local * rect.zw;

    // 应用 2D transform
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;

    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;

    float2 uv = float2(
        lerp(uvRect.x, uvRect.z, local.x),
        lerp(uvRect.y, uvRect.w, local.y)
    );

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = uv;
    o.color = color;
    o.pixel = transformed;
    o.params = params;
    o.debug = rect;  // 调试用
    return o;
}
)";

inline constexpr const char kTextFS[] = R"(
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
)";

} // namespace dong::render::sdl_render::shaders
