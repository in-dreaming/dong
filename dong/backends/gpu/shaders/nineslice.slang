Texture2D imageTexture;
SamplerState imageSampler;

cbuffer ImageUniforms {
    float4 uRect;
    float4 uUVRect;
    float4 uViewport;
    float4 uTransform[2];
    float4 uTint;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
    float4 uNineSlice;  // slice_top, slice_right, slice_bottom, slice_left (UV 0..1)
    float4 uNineWidth;  // width_top, width_right, width_bottom, width_left (dest pixels)
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
    // Pass local 0..1 as uv (FS remaps into atlas via uUVRect).
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = local;
    o.tint = uTint;
    o.pixel = transformed;
    return o;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    float2 uv = saturate(input.uv);
    float st = uNineSlice.x;
    float sr = uNineSlice.y;
    float sb = uNineSlice.z;
    float sl = uNineSlice.w;

    float wt = uNineWidth.x / max(uRect.w, 1e-6);
    float wr = uNineWidth.y / max(uRect.z, 1e-6);
    float wb = uNineWidth.z / max(uRect.w, 1e-6);
    float wl = uNineWidth.w / max(uRect.z, 1e-6);

    int col = (uv.x < wl) ? 0 : (uv.x > (1.0 - wr) ? 2 : 1);
    int row = (uv.y < wt) ? 0 : (uv.y > (1.0 - wb) ? 2 : 1);

    float2 src_uv;
    if (col == 0) {
        src_uv.x = (wl > 1e-6) ? (uv.x / wl) * sl : 0.0;
    } else if (col == 2) {
        src_uv.x = 1.0 - sr + ((wr > 1e-6) ? ((uv.x - (1.0 - wr)) / wr) * sr : 0.0);
    } else {
        float denom = 1.0 - wl - wr;
        float t = (denom > 1e-6) ? (uv.x - wl) / denom : 0.0;
        src_uv.x = sl + t * (1.0 - sl - sr);
    }
    if (row == 0) {
        src_uv.y = (wt > 1e-6) ? (uv.y / wt) * st : 0.0;
    } else if (row == 2) {
        src_uv.y = 1.0 - sb + ((wb > 1e-6) ? ((uv.y - (1.0 - wb)) / wb) * sb : 0.0);
    } else {
        float denom = 1.0 - wt - wb;
        float t = (denom > 1e-6) ? (uv.y - wt) / denom : 0.0;
        src_uv.y = st + t * (1.0 - st - sb);
    }

    src_uv = float2(lerp(uUVRect.x, uUVRect.z, src_uv.x), lerp(uUVRect.y, uUVRect.w, src_uv.y));
    float4 tex = imageTexture.Sample(imageSampler, src_uv);
    float4 tint;
    tint.rgb = uTint.rgb * uTint.a;
    tint.a = uTint.a;
    return float4(tex.rgb * tint.rgb, tex.a * tint.a);
}
