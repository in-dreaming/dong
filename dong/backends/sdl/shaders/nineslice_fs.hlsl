// Nine-slice fragment shader (P0-2)
// Uses the image pipeline's texture + sampler but does 9-region UV remapping.
// Uniforms match the image pipeline (ImageUniforms) plus extra nine-slice params.

Texture2D imageTexture : register(t0, space2);
SamplerState imageSampler : register(s0, space2);

cbuffer ImageUniforms : register(b0, space3) {
    float4 uRect;          // dest rect: x, y, w, h
    float4 uUVRect;        // source UV rect: u0, v0, u1, v1 (usually 0,0,1,1 for nine-slice)
    float4 uViewport;
    float4 uTransform[2];
    float4 uTint;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
    // Nine-slice params (packed into available uniform space)
    // uNineSlice.xyzw = slice_top, slice_right, slice_bottom, slice_left (in UV space, 0..1)
    // uNineWidth.xyzw = width_top, width_right, width_bottom, width_left (in dest pixels)
    float4 uNineSlice;
    float4 uNineWidth;
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
            if (px.x < rc.x || px.x > rc.z || px.y < rc.y || px.y > rc.w)
                return true;
            continue;
        }
        if (sdRoundedClip(px, rc, rad) > 0.0f)
            return true;
    }
    return false;
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel))
        discard;

    float2 uv = input.uv;  // 0..1 across the entire dest rect

    // Nine-slice parameters
    float st = uNineSlice.x;  // slice top (UV)
    float sr = uNineSlice.y;  // slice right (UV)
    float sb = uNineSlice.z;  // slice bottom (UV)
    float sl = uNineSlice.w;  // slice left (UV)

    float wt = uNineWidth.x / uRect.w;  // width top as fraction of dest height
    float wr = uNineWidth.y / uRect.z;  // width right as fraction of dest width
    float wb = uNineWidth.z / uRect.w;  // width bottom as fraction of dest height
    float wl = uNineWidth.w / uRect.z;  // width left as fraction of dest width

    // Determine which column (0=left, 1=middle, 2=right)
    int col = (uv.x < wl) ? 0 : (uv.x > (1.0 - wr) ? 2 : 1);
    // Determine which row (0=top, 1=middle, 2=bottom)
    int row = (uv.y < wt) ? 0 : (uv.y > (1.0 - wb) ? 2 : 1);

    // Remap UV to source texture coordinates
    float2 src_uv;

    // Horizontal mapping
    if (col == 0) {
        // Left column: map [0, wl] -> [0, sl]
        src_uv.x = (uv.x / wl) * sl;
    } else if (col == 2) {
        // Right column: map [1-wr, 1] -> [1-sr, 1]
        src_uv.x = 1.0 - sr + ((uv.x - (1.0 - wr)) / wr) * sr;
    } else {
        // Middle column: map [wl, 1-wr] -> [sl, 1-sr] (stretch)
        float t = (uv.x - wl) / (1.0 - wl - wr);
        src_uv.x = sl + t * (1.0 - sl - sr);
    }

    // Vertical mapping
    if (row == 0) {
        // Top row: map [0, wt] -> [0, st]
        src_uv.y = (uv.y / wt) * st;
    } else if (row == 2) {
        // Bottom row: map [1-wb, 1] -> [1-sb, 1]
        src_uv.y = 1.0 - sb + ((uv.y - (1.0 - wb)) / wb) * sb;
    } else {
        // Middle row: map [wt, 1-wb] -> [st, 1-sb] (stretch)
        float t = (uv.y - wt) / (1.0 - wt - wb);
        src_uv.y = st + t * (1.0 - st - sb);
    }

    // Apply source UV rect transform
    src_uv = float2(
        lerp(uUVRect.x, uUVRect.z, src_uv.x),
        lerp(uUVRect.y, uUVRect.w, src_uv.y)
    );

    float4 tex = imageTexture.Sample(imageSampler, src_uv);

    // Apply tint (premultiplied alpha)
    float4 tint;
    tint.rgb = uTint.rgb * uTint.a;
    tint.a = uTint.a;
    return float4(tex.rgb * tint.rgb, tex.a * tint.a);
}
