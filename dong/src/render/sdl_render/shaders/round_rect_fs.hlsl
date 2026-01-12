struct PSInput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD3;
};

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer RoundRectUniforms : register(b0, space3) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uColor;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

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

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    float2 size = input.size;
    float2 p = (input.local - 0.5) * size;
    float r = input.radius;
    float2 halfSize = size * 0.5;
    float dist = sdRoundRect(p, halfSize, r);
    const float aa = 1.0;
    float alpha = saturate(0.5 - dist / aa);
    float4 base = input.color;
    base.a *= alpha;
    // 直接输出 sRGB 颜色，不做 gamma 转换
    return base;
}
