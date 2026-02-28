struct GlyphInstanceData {
    float4 rect;
    float4 uvRect;
    float4 color;
    float4 params;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD1;
    float4 params : TEXCOORD2;
};

// SDL_GPU 规范：Vertex shader 的 uniform buffer 使用 space1
// 布局必须与 C++ TextBatchUniformData 完全一致
// 总大小 <= 4096 bytes 以兼容大多数 GPU
cbuffer TextUniforms : register(b0, space1) {
    float4 uViewport;           // offset 0, size 16
    float4 uTransform[2];       // offset 16, size 32
    float4 uClipRects[4];       // offset 48, size 64
    float4 uClipRadii;          // offset 112, size 16
    float4 uClipMeta;           // offset 128, size 16
    float4 uGlyphData[244];     // offset 144, size 3904 (61 glyphs * 4 float4)
};

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID) {
    float2 local;
    if (vertexID == 0) { local = float2(0.0, 0.0); }
    else if (vertexID == 1) { local = float2(1.0, 0.0); }
    else if (vertexID == 2) { local = float2(0.0, 1.0); }
    else { local = float2(1.0, 1.0); }

    // 每个 glyph 占用 4 个 float4 (rect, uvRect, color, params)
    // glyph 数据从 uGlyphData[0] 开始
    uint base = instanceID * 4;
    float4 rect = uGlyphData[base + 0];
    float4 uvRect = uGlyphData[base + 1];
    float4 color = uGlyphData[base + 2];
    float4 params = uGlyphData[base + 3];

    float2 pos = rect.xy + local * rect.zw;

    // 应用 2D transform
    float2 transformed;
    transformed.x = uTransform[0].x * pos.x + uTransform[0].y * pos.y + uTransform[0].z;
    transformed.y = uTransform[1].x * pos.x + uTransform[1].y * pos.y + uTransform[1].z;

    float2 ndc;
    ndc.x = (transformed.x / uViewport.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (transformed.y / uViewport.y) * 2.0;

    float2 uv = float2(
        lerp(uvRect.x, uvRect.z, local.x),
        lerp(uvRect.y, uvRect.w, local.y)
    );

    VSOutput o;
    o.position = float4(ndc, 0.0, 1.0);
    o.uv = uv;
    o.color = color;
    o.pixel = transformed;
    o.params = params;
    return o;
}
