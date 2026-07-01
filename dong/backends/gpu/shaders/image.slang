Texture2D imageTexture;
SamplerState imageSampler;

struct PSInput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
    float2 pixel : TEXCOORD1;
};

cbuffer ImageUniforms {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTransform[2];
    float4 uTint;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

struct VSOutput {
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

[shader("vertex")]
VSOutput vsMain(uint vertexID : SV_VertexID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }
    float2 pos = uRect.xy + local * uRect.zw;

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

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    float4 tex = imageTexture.Sample(imageSampler, input.uv);
    float4 tint;
    tint.rgb = uTint.rgb * uTint.a;
    tint.a = uTint.a;
    return float4(tex.rgb * tint.rgb, tex.a * tint.a);
}
