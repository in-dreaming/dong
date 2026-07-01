Texture2D msdfTexture;
SamplerState msdfSampler;

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
    float4 params : TEXCOORD2;
};

cbuffer TextUniforms {
    float4 uViewport;
    float4 uTransform[2];
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
    float4 uGlyphData[244];
};

struct VSOutput {
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

float calcMSDFOpacity(float3 msdf, float screenPxRange) {
    float sd = median(msdf.r, msdf.g, msdf.b);
    float screenPxDistance = screenPxRange * (sd - 0.5);
    return clamp(screenPxDistance + 0.5, 0.0, 1.0);
}

[shader("vertex")]
VSOutput vsMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }

    uint base = instanceID * 4;
    float4 rect = uGlyphData[base + 0];
    float4 uvRect = uGlyphData[base + 1];
    float4 color = uGlyphData[base + 2];
    float4 params = uGlyphData[base + 3];

    float2 pos = rect.xy + local * rect.zw;
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;

    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;

    float2 uv = float2(lerp(uvRect.x, uvRect.z, local.x), lerp(uvRect.y, uvRect.w, local.y));

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = uv;
    o.color = color;
    o.pixel = transformed;
    o.params = params;
    return o;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    // Color bitmap path: params.x <= 0 signals a pre-rendered color glyph
    // (COLR v0/v1, CBDT, sbix emoji, etc.) - sample texture directly as RGBA
    // color instead of decoding it as a 3-channel MSDF distance field.
    if (input.params.x <= 0.0) {
        float4 texColor = msdfTexture.Sample(msdfSampler, input.uv);
        texColor.a *= input.color.a;
        if (texColor.a < 1.0 / 255.0) {
            discard;
        }
        texColor.rgb *= texColor.a;
        return texColor;
    }

    float screenPxRange = max(input.params.x, 2.0);
    float3 msdf = msdfTexture.Sample(msdfSampler, input.uv).rgb;
    float opacity = calcMSDFOpacity(msdf, screenPxRange);
    if (opacity <= 0.001) {
        discard;
    }
    float4 outColor = input.color;
    outColor.a *= opacity;
    outColor.rgb *= outColor.a;
    return outColor;
}
