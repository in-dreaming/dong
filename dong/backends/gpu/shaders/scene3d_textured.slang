Texture2D screenTexture;
SamplerState screenSampler;

// Host-side matrices (dong_mat4_t, see dong_math.h) are column-major, matching DXC's default
// cbuffer layout used by every other Dong HLSL/host matrix in this codebase. Slang's own default
// (when compiled through the embedded Slang API, as this shader is) is row-major, so without this
// explicit qualifier the GPU would reinterpret the uploaded bytes as a transposed matrix - vertex
// positions become garbage/off-screen for every quad, i.e. the "3d_screens_simple 黑屏" symptom
// (composited backbuffer showed nothing but the clear color - no screens rendered at all).
cbuffer uniforms {
    column_major float4x4 uMVP;
    column_major float4x4 uModel;
    float4 uColor;
    float4 uHighlight;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

static const float3 kPositions[6] = {
    float3(-0.5, -0.5, 0.0),
    float3(0.5, -0.5, 0.0),
    float3(0.5, 0.5, 0.0),
    float3(-0.5, -0.5, 0.0),
    float3(0.5, 0.5, 0.0),
    float3(-0.5, 0.5, 0.0),
};

static const float2 kUVs[6] = {
    float2(0.0, 1.0),
    float2(1.0, 1.0),
    float2(1.0, 0.0),
    float2(0.0, 1.0),
    float2(1.0, 0.0),
    float2(0.0, 0.0),
};

[shader("vertex")]
VSOutput vsMain(uint vertexID : SV_VertexID) {
    VSOutput o;
    float3 pos = kPositions[vertexID];
    o.position = mul(uMVP, float4(pos, 1.0));
    o.uv = kUVs[vertexID];
    return o;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target0 {
    float4 c = screenTexture.Sample(screenSampler, input.uv) * uColor;
    if (uHighlight.x > 0.5) {
        float b = 0.02;
        if (input.uv.x < b || input.uv.x > 1.0 - b || input.uv.y < b || input.uv.y > 1.0 - b) {
            c.rgb = lerp(c.rgb, float3(0.3, 0.6, 1.0), 0.7);
        }
    }
    return c;
}
