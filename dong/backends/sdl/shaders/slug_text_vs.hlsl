// =============================================================================
// Slug Text Vertex Shader (Dong Engine)
// =============================================================================
// Uses the canonical Slug vertex buffer architecture with 5 float4 attributes:
//   attrib[0] = pos: (pos.xy = object-space vertex, pos.zw = miter normal)
//   attrib[1] = tex: (tex.xy = em-space coords, tex.z = band location packed,
//                      tex.w = band max + flags packed)
//   attrib[2] = jac: inverse Jacobian (M^-1_00, M^-1_01, M^-1_10, M^-1_11)
//   attrib[3] = bnd: (bandScale.x, bandScale.y, bandOffset.x, bandOffset.y)
//   attrib[4] = col: vertex RGBA color
//
// Based on the Slug algorithm (MIT License, Eric Lengyel 2017).
// Adapted for Dong engine's 2D orthographic pipeline.
// =============================================================================

struct VSOutput {
    float4 position     : SV_Position;
    float4 color        : COLOR0;
    float2 texcoord     : TEXCOORD0;
    nointerpolation float4 banding : TEXCOORD1;
    nointerpolation int4   glyph  : TEXCOORD2;
    float2 pixel        : TEXCOORD3;
};

cbuffer SlugVertexUniforms : register(b0, space1) {
    float4 slug_matrix[4];  // 4 rows of MVP matrix
    float4 slug_viewport;   // (viewport_w, viewport_h, 0, 0)
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

// Unpack band location and band max from packed uint-as-float fields.
void SlugUnpack(float4 tex, float4 bnd, out float4 vbnd, out int4 vgly) {
    uint2 g = asuint(tex.zw);
    vgly = int4(g.x & 0xFFFFu, g.x >> 16u, g.y & 0xFFFFu, g.y >> 16u);
    vbnd = bnd;
}

// Dynamic vertex dilation along miter normals for sub-pixel coverage.
// Returns dilated em-space sample coordinates; outputs dilated position.
float2 SlugDilate(float4 pos, float4 tex, float4 jac,
                  float4 m0, float4 m1, float4 m3,
                  float2 dim, out float2 vpos) {
    float2 n = normalize(pos.zw);
    float s = dot(m3.xy, pos.xy) + m3.w;
    float t = dot(m3.xy, n);

    float u = (s * dot(m0.xy, n) - t * (dot(m0.xy, pos.xy) + m0.w)) * dim.x;
    float v = (s * dot(m1.xy, n) - t * (dot(m1.xy, pos.xy) + m1.w)) * dim.y;

    float s2 = s * s;
    float st = s * t;
    float uv = u * u + v * v;
    float2 d = pos.zw * (s2 * (st + sqrt(uv)) / (uv - st * st));

    vpos = pos.xy + d;
    return float2(tex.x + dot(d, jac.xy), tex.y + dot(d, jac.zw));
}

VSOutput main(float4 attrib0 : TEXCOORD0,
              float4 attrib1 : TEXCOORD1,
              float4 attrib2 : TEXCOORD2,
              float4 attrib3 : TEXCOORD3,
              float4 attrib4 : TEXCOORD4) {
    VSOutput result;

    // For 2D orthographic rendering, skip SlugDilate and use vertex
    // positions directly. Dilation is only needed for perspective
    // projections; in ortho the half-pixel expansion is handled by
    // slightly expanding the glyph bounding box on the CPU side.
    float2 p = attrib0.xy;
    result.texcoord = attrib1.xy;

    // Apply MVP matrix to vertex position
    result.position.x = p.x * slug_matrix[0].x + p.y * slug_matrix[0].y + slug_matrix[0].w;
    result.position.y = p.x * slug_matrix[1].x + p.y * slug_matrix[1].y + slug_matrix[1].w;
    result.position.z = p.x * slug_matrix[2].x + p.y * slug_matrix[2].y + slug_matrix[2].w;
    result.position.w = p.x * slug_matrix[3].x + p.y * slug_matrix[3].y + slug_matrix[3].w;

    // Pixel coordinates for clipping (from clip-space to screen-space)
    float2 ndc = result.position.xy / result.position.w;
    result.pixel = float2((ndc.x * 0.5 + 0.5) * slug_viewport.x,
                          (0.5 - ndc.y * 0.5) * slug_viewport.y);

    // Unpack band data and color
    SlugUnpack(attrib1, attrib3, result.banding, result.glyph);
    result.color = attrib4;

    return result;
}
