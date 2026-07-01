cbuffer RoundRectUniforms {
    float4 uRect;
    float4 uRadius;   // x=corner radius, y=stroke width
    float4 uViewport;
    float4 uTransform[2];
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    float2 pixel : TEXCOORD1;
};

float sdRoundRect(float2 p, float2 halfSize, float rad) {
    float2 q = abs(p) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    return sdRoundRect(pt - center, halfSize, rad);
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

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.local = local;
    o.pixel = transformed;
    return o;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    float2 size = uRect.zw;
    float2 p = (input.local - 0.5) * size;
    float r = uRadius.x;
    r = min(r, min(size.x, size.y) * 0.5 - 0.5);
    r = max(r, 0.0);
    float stroke = max(uRadius.y, 0.0);
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

    float4 base = uColor;
    base.a *= alpha;
    return base;
}
