struct VSOutput {
    float4 position : SV_Position;
    float2 pixel : TEXCOORD0;
    float2 local_uv : TEXCOORD1;  // 0..1 within the rect
};

cbuffer GradientUniforms : register(b0, space1) {
    float4 uRect;          // x, y, w, h
    float4 uViewport;      // viewport_w, viewport_h, 0, 0
    float4 uTransform[2];  // 2D transform matrix
    float4 uGradientParams; // angle_rad, stop_count, radius, 0
    float4 uStopColors[8]; // RGBA for each stop
    float4 uStopPositions[2]; // 8 positions packed into 2 float4s
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

VSOutput main(uint vertexID : SV_VertexID) {
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
