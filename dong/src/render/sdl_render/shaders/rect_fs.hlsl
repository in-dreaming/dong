struct PSInput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD0;
};

// Fragment shader 的 uniform buffer 必须使用 space3
cbuffer RectUniforms : register(b0, space3) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix: [m0,m1,m2,0] [m3,m4,m5,0]
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

float4 main(PSInput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }
    // 直接输出 sRGB 颜色，不做 gamma 转换
    return input.color;
}
