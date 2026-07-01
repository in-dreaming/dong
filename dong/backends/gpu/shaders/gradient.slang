cbuffer GradientUniforms {
    float4 uRect;
    float4 uViewport;
    float4 uTransform[2];
    float4 uGradientParams; // angle_rad, stop_count, radius, 0
    float4 uStopColors[8];
    float4 uStopPositions[2]; // 8 positions packed
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 pixel : TEXCOORD0;
    float2 local_uv : TEXCOORD1;
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

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.pixel = transformed;
    o.local_uv = local;
    return o;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    float radius = uGradientParams.z;
    if (radius > 0.0f) {
        float2 center = uRect.xy + uRect.zw * 0.5;
        float2 halfSize = uRect.zw * 0.5;
        float d = sdRoundedRect(input.pixel - center, halfSize, radius);
        if (d > 0.5f) {
            discard;
        }
    }

    float angle = uGradientParams.x;
    int stopCount = (int)uGradientParams.y;
    float dx = sin(angle);
    float dy = -cos(angle);
    float2 uv = input.local_uv - 0.5;
    float t = saturate(dot(uv, float2(dx, dy)) + 0.5);

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
