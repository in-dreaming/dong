// Video YUV420P fragment shader
// - Samples Y, U, V planes and converts to RGB in shader.
// - Follows the same uniform layout and clip logic as image_fs.hlsl.
//
// NOTE: Fragment shader textures/samplers must use space2.
//       Uniform buffer must use space3.

Texture2D texY : register(t0, space2);
SamplerState sampY : register(s0, space2);
Texture2D texU : register(t1, space2);
SamplerState sampU : register(s1, space2);
Texture2D texV : register(t2, space2);
SamplerState sampV : register(s2, space2);

cbuffer ImageUniforms : register(b0, space3) {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTransform[2];
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

// Limited-range YUV (TV range) BT.709
float3 yuv_to_rgb_bt709(float y, float u, float v) {
    // Y in [16/255..235/255], UV in [16/255..240/255]
    float yy = 1.1643f * (y - 0.0625f);
    float uu = u - 0.5f;
    float vv = v - 0.5f;

    float r = yy + 1.7927f * vv;
    float g = yy - 0.2132f * uu - 0.5329f * vv;
    float b = yy + 2.1124f * uu;

    return float3(r, g, b);
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    // Apply atlas UV rect (for consistency), though videos typically use full UV.
    float2 uv = float2(
        lerp(uUVRect.x, uUVRect.z, input.uv.x),
        lerp(uUVRect.y, uUVRect.w, input.uv.y)
    );

    float y = texY.Sample(sampY, uv).r;
    float u = texU.Sample(sampU, uv).r;
    float v = texV.Sample(sampV, uv).r;

    float3 rgb = yuv_to_rgb_bt709(y, u, v);
    rgb = saturate(rgb);

    // Use uniform tint instead of interpolated varying to avoid backend linkage issues.
    float3 tinted = rgb * uTint.rgb;
    float alpha = uTint.a;

    return float4(tinted, alpha);
}
