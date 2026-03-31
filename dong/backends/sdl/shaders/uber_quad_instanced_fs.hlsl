struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float4 params : TEXCOORD2;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD3;
    float2 local_uv : TEXCOORD4;
};

// Per-batch shared data (must match VS layout for clip access)
cbuffer UberQuadBatchUniforms : register(b0, space3) {
    float4 uViewport;
    float4 uTransform[2];
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

// ---- SDF helpers ----

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

// ---- Material implementations ----

float4 materialSolidRect(PSInput input) {
    return input.color;
}

float4 materialRoundedRect(PSInput input) {
    float2 size = input.size;
    float2 p = (input.local - 0.5) * size;
    float r = input.params.y;
    float stroke = input.params.z;
    float2 halfSize = size * 0.5;

    float distOuter = sdRoundRect(p, halfSize, r);
    float aa = max(fwidth(distOuter), 1e-4);
    float alpha = saturate(0.5 - distOuter / aa);

    if (stroke > 0.0) {
        float2 innerHalf = halfSize - stroke;
        if (innerHalf.x > 0.0 && innerHalf.y > 0.0) {
            float innerR = max(r - stroke, 0.0);
            innerR = min(innerR, min(innerHalf.x, innerHalf.y) - 0.5);
            innerR = max(innerR, 0.0);

            float distInner = sdRoundRect(p, innerHalf, innerR);
            float distRing = max(distOuter, -distInner);
            aa = max(fwidth(distRing), 1e-4);
            alpha = saturate(0.5 - distRing / aa);
        }
    }

    float4 base = input.color;
    base.a *= alpha;
    return base;
}

float4 materialShadow(PSInput input) {
    float2 size = input.size;
    float2 p = (input.local - 0.5) * size;
    float r = input.params.y;
    float blur = input.params.w;
    float2 halfSize = size * 0.5;

    float dist = sdRoundRect(p, halfSize, r);
    float sigma = max(blur * 0.33333334, 0.5);
    float alpha = 1.0 - smoothstep(-sigma, sigma, dist);

    float4 base = input.color;
    base.a *= alpha;
    return base;
}

// ---- Entry point ----

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    int material = (int)input.params.x;

    if (material == 0) return materialSolidRect(input);
    if (material == 1) return materialRoundedRect(input);
    if (material == 2) return materialShadow(input);

    return float4(1, 0, 1, 1);
}
