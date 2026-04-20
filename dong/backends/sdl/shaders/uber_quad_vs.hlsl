struct VSOutput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float4 params : TEXCOORD2;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD3;
    float2 local_uv : TEXCOORD4;
};

// params.x: material (0=solid, 1=rounded, 2=shadow, 4=gradient, 5=nineslice reserved)
// params.y: corner radius
// params.z: stroke width
// params.w: blur radius

cbuffer UberQuadUniforms : register(b0, space1) {
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

VSOutput main(uint vertexID : SV_VertexID) {
    float2 corner;
    if (vertexID == 0) { corner = float2(0.0, 0.0); }
    else if (vertexID == 1) { corner = float2(1.0, 0.0); }
    else if (vertexID == 2) { corner = float2(0.0, 1.0); }
    else { corner = float2(1.0, 1.0); }

    int material = (int)uParams.x;
    float blur = uParams.w;

    float2 rect_pos = uRect.xy;
    float2 rect_size = uRect.zw;

    // Shadow: expand quad by blur amount
    if (material == 2 && blur > 0.0) {
        rect_pos -= blur;
        rect_size += blur * 2.0;
    }

    float2 pos = rect_pos + corner * rect_size;

    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;

    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;

    float radius = uParams.y;
    radius = min(radius, min(uRect.z, uRect.w) * 0.5 - 0.5);
    radius = max(radius, 0.0);

    float stroke = uParams.z;
    stroke = max(stroke, 0.0);
    stroke = min(stroke, min(uRect.z, uRect.w) * 0.5 - 0.5);

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.color = uColor;
    o.pixel = transformed;
    o.size = uRect.zw;
    o.params = float4((float)material, radius, stroke, blur);

    if (material == 2 && blur > 0.0) {
        o.local = (corner * rect_size - blur) / uRect.zw;
    } else {
        o.local = corner;
    }
    o.local_uv = corner;

    return o;
}
