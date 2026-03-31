struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float4 params : TEXCOORD2;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD3;
    float2 local_uv : TEXCOORD4;
};

cbuffer UberQuadUniforms : register(b0, space3) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
    float4 uTransform[2];
    float4 uParams;
    float4 uGradientParams;
    float4 uStopColors[8];
    float4 uStopPositions[2];
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

// ---- Clip logic (shared across all materials) ----

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

// ---- Gradient helpers ----

float getStopPosition(int idx) {
    int vec = idx / 4;
    int comp = idx % 4;
    if (vec == 0) {
        if (comp == 0) return uStopPositions[0].x;
        if (comp == 1) return uStopPositions[0].y;
        if (comp == 2) return uStopPositions[0].z;
        return uStopPositions[0].w;
    }
    if (comp == 0) return uStopPositions[1].x;
    if (comp == 1) return uStopPositions[1].y;
    if (comp == 2) return uStopPositions[1].z;
    return uStopPositions[1].w;
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

float4 materialGradient(PSInput input) {
    float radius = uGradientParams.z;
    if (radius > 0.0f) {
        float2 center = uRect.xy + uRect.zw * 0.5;
        float2 halfSize = uRect.zw * 0.5;
        float d = sdRoundRect(input.pixel - center, halfSize, radius);
        if (d > 0.5f) {
            discard;
        }
    }

    float angle = uGradientParams.x;
    int stopCount = (int)uGradientParams.y;

    float dx = sin(angle);
    float dy = -cos(angle);

    float2 uv = input.local_uv - 0.5;
    float t = dot(uv, float2(dx, dy)) + 0.5;
    t = saturate(t);

    float4 color = uStopColors[0];
    for (int i = 1; i < 8; ++i) {
        if (i >= stopCount) break;
        float p0 = getStopPosition(i - 1);
        float p1 = getStopPosition(i);
        if (t <= p1) {
            float range = p1 - p0;
            float factor = (range > 0.0001f) ? saturate((t - p0) / range) : 0.0f;
            color = lerp(uStopColors[i - 1], uStopColors[i], factor);
            break;
        }
        color = uStopColors[i];
    }

    return color;
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
    if (material == 3) return materialGradient(input);

    return float4(1, 0, 1, 1);
}
