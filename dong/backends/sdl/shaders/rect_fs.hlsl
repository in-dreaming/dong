struct PSInput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 pixel : TEXCOORD0;
};

cbuffer RectUniforms : register(b0, space3) {
    float4 uRect;
    float4 uColor;
    float4 uViewport;
    float4 uTransform[2];
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

// NOTE(D3D12): dynamic clip loops in this shader break SDL_CreateGPUGraphicsPipeline
// (E_INVALIDARG). Clipping for solid rects is handled via scissor/uber paths when needed.
float4 main(PSInput input) : SV_Target0 {
    return uColor;
}
