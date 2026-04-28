struct PSInput {
    float4 position : SV_Position;
    float2 pixel : TEXCOORD0;
    float2 local_uv : TEXCOORD1;
};

cbuffer ConicGradientUniforms : register(b0, space3) {
    float4 uRect;
    float4 uViewport;
    float4 uTransform[2];
    float4 uConicMeta;       // x=from_rad, y=stop_count, z=radius, w=repeating(0|1)
    float4 uConicCenterPeriod; // xy=center(px), z=repeat span (>0 normalized), w=reserved
    float4 uStopColors[8];
    float4 uStopPositions[2];
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

float sdRoundedRect(float2 pt, float2 halfSize, float rad) {
    float2 q = abs(pt) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

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

// CSS conic: angle increases clockwise, 0deg at top (12 o'clock). Y-down screen coords.
float conicParam(float2 px, float2 center, float fromRad) {
    float2 d = px - center;
    float dist = length(d);
    if (dist < 0.5f) {
        return 0.0;
    }
    float a = atan2(d.y, d.x);
    // align so that fromRad=0 matches top of circle (angle increases clockwise)
    float t = (a + 1.57079632679f - fromRad) / 6.28318530718f;
    return frac(t + 1.0f);
}

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    float radius = uConicMeta.z;
    if (radius > 0.0f) {
        float2 center = uRect.xy + uRect.zw * 0.5;
        float2 halfSize = uRect.zw * 0.5;
        float d = sdRoundedRect(input.pixel - center, halfSize, radius);
        if (d > 0.5f) {
            discard;
        }
    }

    int stopCount = (int)uConicMeta.y;
    if (stopCount < 1) {
        return float4(0, 0, 0, 0);
    }
    if (stopCount == 1) {
        return uStopColors[0];
    }

    float2 cxy = uConicCenterPeriod.xy;
    float fromRad = uConicMeta.x;
    bool repeating = uConicMeta.w > 0.5f;

    float t_circ = conicParam(input.pixel, cxy, fromRad);

    float t0 = getStopPosition(0);
    float tN = getStopPosition(stopCount - 1);
    float span = max(tN - t0, 1e-5f);

    float t = t_circ;
    if (repeating) {
        float u = (t_circ - t0) / span;
        u = frac(u);
        t = t0 + u * span;
    } else {
        t = clamp(t_circ, t0, tN);
    }

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
