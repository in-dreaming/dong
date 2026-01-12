struct VSOutput {
    float4 position : SV_Position;
    float2 local : TEXCOORD0;
    nointerpolation float2 size : TEXCOORD1;
    nointerpolation float radius : TEXCOORD2;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD3;
};

cbuffer RoundRectUniforms : register(b0, space1) {
    float4 uRect;
    float4 uRadius;
    float4 uViewport;
    float4 uTransform[2];  // 2D transform matrix
    float4 uColor;
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
    
    // 计算本地坐标
    float2 pos = uRect.xy + local * uRect.zw;
    
    // 应用 2D transform
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;
    
    // 转换到 NDC
    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;
    
    float radius = uRadius.x;
    radius = min(radius, min(uRect.z, uRect.w) * 0.5 - 0.5);
    radius = max(radius, 0.0);
    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.local = local;
    o.size = uRect.zw;
    o.radius = radius;
    o.color = uColor;
    o.pixel = transformed;  // 使用变换后的坐标
    return o;
}
