Texture2D curveTexture;
SamplerState curveSampler;
Texture2D bandTexture;
SamplerState bandSampler;

cbuffer SlugVertexUniforms {
    float4 slug_matrix[4];
    float4 slug_viewport;
    float4 uClipRects[4];
    float4 uClipRadii;
    float4 uClipMeta;
};

cbuffer SlugFragUniforms {
    float4 uViewport;
    float4 uClipRectsFrag[4];
    float4 uClipRadiiFrag;
    float4 uClipMetaFrag;
    float4 uTextureSizes;
};

struct VSInput {
    float4 pos : TEXCOORD0;
    float4 tex : TEXCOORD1;
    float4 jac : TEXCOORD2;
    float4 bnd : TEXCOORD3;
    float4 col : TEXCOORD4;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
    nointerpolation float4 banding : TEXCOORD1;
    nointerpolation int4 glyph : TEXCOORD2;
    float2 pixel : TEXCOORD3;
};

void SlugUnpack(float4 tex, float4 bnd, out float4 vbnd, out int4 vgly) {
    uint2 g = asuint(tex.zw);
    vgly = int4(g.x & 0xFFFFu, g.x >> 16u, g.y & 0xFFFFu, g.y >> 16u);
    vbnd = bnd;
}

float sdRoundedClip(float2 pt, float4 rc, float rad) {
    float2 halfSize = float2((rc.z - rc.x) * 0.5, (rc.w - rc.y) * 0.5);
    float2 center = float2(rc.x, rc.y) + halfSize;
    float2 local = pt - center;
    float2 q = abs(local) - (halfSize - rad);
    float2 qmax = float2(max(q.x, 0.0), max(q.y, 0.0));
    return length(qmax) + min(max(q.x, q.y), 0.0) - rad;
}

bool discardByClip(float2 px) {
    uint clipCount = (uint)uClipMetaFrag.x;
    for (uint i = 0; i < clipCount; ++i) {
        float rad = uClipRadiiFrag[i];
        float4 rc = uClipRectsFrag[i];
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

#define kLogBandTextureWidth 12

uint4 BandLoadU(int2 loc) {
    float2 uv = (float2(loc) + 0.5) / float2(uTextureSizes.z, uTextureSizes.w);
    float4 f = bandTexture.SampleLevel(bandSampler, uv, 0);
    return uint4(uint(f.x), uint(f.y), uint(f.z), uint(f.w));
}

uint CalcRootCode(float y1, float y2, float y3) {
    uint i1 = asuint(y1) >> 31u;
    uint i2 = asuint(y2) >> 30u;
    uint i3 = asuint(y3) >> 29u;
    uint shift = (i2 & 2u) | (i1 & ~2u);
    shift = (i3 & 4u) | (shift & ~4u);
    return ((0x2E74u >> shift) & 0x0101u);
}

float2 SolveHorizPoly(float4 p12, float2 p3) {
    float2 a = p12.xy - p12.zw * 2.0 + p3;
    float2 b = p12.xy - p12.zw;
    float ra = 1.0 / a.y;
    float rb = 0.5 / b.y;
    float d = sqrt(max(b.y * b.y - a.y * p12.y, 0.0));
    float t1 = (b.y - d) * ra;
    float t2 = (b.y + d) * ra;
    if (abs(a.y) < 1.0 / 65536.0) {
        t1 = t2 = p12.y * rb;
    }
    return float2(
        (a.x * t1 - b.x * 2.0) * t1 + p12.x,
        (a.x * t2 - b.x * 2.0) * t2 + p12.x
    );
}

float2 SolveVertPoly(float4 p12, float2 p3) {
    float2 a = p12.xy - p12.zw * 2.0 + p3;
    float2 b = p12.xy - p12.zw;
    float ra = 1.0 / a.x;
    float rb = 0.5 / b.x;
    float d = sqrt(max(b.x * b.x - a.x * p12.x, 0.0));
    float t1 = (b.x - d) * ra;
    float t2 = (b.x + d) * ra;
    if (abs(a.x) < 1.0 / 65536.0) {
        t1 = t2 = p12.x * rb;
    }
    return float2(
        (a.y * t1 - b.y * 2.0) * t1 + p12.y,
        (a.y * t2 - b.y * 2.0) * t2 + p12.y
    );
}

int2 CalcBandLoc(int2 glyphLoc, uint offset) {
    int2 bandLoc = int2(glyphLoc.x + (int)offset, glyphLoc.y);
    bandLoc.y += bandLoc.x >> kLogBandTextureWidth;
    bandLoc.x &= (1 << kLogBandTextureWidth) - 1;
    return bandLoc;
}

float CalcCoverage(float xcov, float ycov, float xwgt, float ywgt, uint flags) {
    float coverage;
    if (flags != 0u) {
        coverage = max(
            abs(xcov * xwgt + ycov * ywgt) / max(xwgt + ywgt, 1.0 / 65536.0),
            min(abs(xcov), abs(ycov))
        );
    } else {
        coverage = max(
            abs(xcov * xwgt + ycov * ywgt) / max(xwgt + ywgt, 1.0 / 65536.0),
            min(abs(xcov), abs(ycov))
        );
    }
    return saturate(coverage);
}

void ProcessHorizontalBands(float2 renderCoord, float2 pixelsPerEm, int2 glyphLoc, int bandIndex, int2 bandMax,
                          inout float xcov, inout float xwgt) {
    uint2 hbandData = BandLoadU(int2(glyphLoc.x + bandIndex, glyphLoc.y)).xy;
    int2 hbandLoc = CalcBandLoc(glyphLoc, hbandData.y);

    for (int ci = 0; ci < (int)hbandData.x; ci++) {
        int2 curveLoc = (int2)BandLoadU(int2(hbandLoc.x + ci, hbandLoc.y)).xy;
        float4 p12 = curveTexture.Load(int3(curveLoc, 0)) - float4(renderCoord, renderCoord);
        float2 p3 = curveTexture.Load(int3(curveLoc.x + 1, curveLoc.y, 0)).xy - renderCoord;

        if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) {
            break;
        }

        uint code = CalcRootCode(p12.y, p12.w, p3.y);
        if (code != 0u) {
            float2 r = SolveHorizPoly(p12, p3) * pixelsPerEm.x;
            if ((code & 1u) != 0u) {
                xcov += saturate(r.x + 0.5);
                xwgt = max(xwgt, saturate(1.0 - abs(r.x) * 2.0));
            }
            if (code > 1u) {
                xcov -= saturate(r.y + 0.5);
                xwgt = max(xwgt, saturate(1.0 - abs(r.y) * 2.0));
            }
        }
    }
}

void ProcessVerticalBands(float2 renderCoord, float2 pixelsPerEm, int2 glyphLoc, int bandIndex, int2 bandMax,
                          inout float ycov, inout float ywgt) {
    uint2 vbandData = BandLoadU(int2(glyphLoc.x + bandMax.y + 1 + bandIndex, glyphLoc.y)).xy;
    int2 vbandLoc = CalcBandLoc(glyphLoc, vbandData.y);

    for (int ci = 0; ci < (int)vbandData.x; ci++) {
        int2 curveLoc = (int2)BandLoadU(int2(vbandLoc.x + ci, vbandLoc.y)).xy;
        float4 p12 = curveTexture.Load(int3(curveLoc, 0)) - float4(renderCoord, renderCoord);
        float2 p3 = curveTexture.Load(int3(curveLoc.x + 1, curveLoc.y, 0)).xy - renderCoord;

        if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) {
            break;
        }

        uint code = CalcRootCode(p12.x, p12.z, p3.x);
        if (code != 0u) {
            float2 r = SolveVertPoly(p12, p3) * pixelsPerEm.y;
            if ((code & 1u) != 0u) {
                ycov -= saturate(r.x + 0.5);
                ywgt = max(ywgt, saturate(1.0 - abs(r.x) * 2.0));
            }
            if (code > 1u) {
                ycov += saturate(r.y + 0.5);
                ywgt = max(ywgt, saturate(1.0 - abs(r.y) * 2.0));
            }
        }
    }
}

float SlugRender(float2 renderCoord, float4 bandTransform, int4 glyphData) {
    float2 emsPerPixel = fwidth(renderCoord);
    float2 pixelsPerEm = 1.0 / max(emsPerPixel, float2(1e-10, 1e-10));

    int2 bandMax = glyphData.zw;
    uint flags = (uint)bandMax.y >> 8u;
    bandMax.y &= 0x00FF;

    int2 bandIndex = clamp(
        int2(renderCoord * bandTransform.xy + bandTransform.zw),
        int2(0, 0), bandMax
    );
    int2 glyphLoc = glyphData.xy;

    float xcov = 0.0, xwgt = 0.0;
    ProcessHorizontalBands(renderCoord, pixelsPerEm, glyphLoc, bandIndex.y, bandMax, xcov, xwgt);

    float ycov = 0.0, ywgt = 0.0;
    ProcessVerticalBands(renderCoord, pixelsPerEm, glyphLoc, bandIndex.x, bandMax, ycov, ywgt);

    return CalcCoverage(xcov, ycov, xwgt, ywgt, flags);
}

[shader("vertex")]
VSOutput vsMain(VSInput input) {
    VSOutput result;
    float2 p = input.pos.xy;
    result.texcoord = input.tex.xy;

    result.position.x = p.x * slug_matrix[0].x + p.y * slug_matrix[0].y + slug_matrix[0].w;
    result.position.y = p.x * slug_matrix[1].x + p.y * slug_matrix[1].y + slug_matrix[1].w;
    result.position.z = p.x * slug_matrix[2].x + p.y * slug_matrix[2].y + slug_matrix[2].w;
    result.position.w = p.x * slug_matrix[3].x + p.y * slug_matrix[3].y + slug_matrix[3].w;

    float2 ndc = result.position.xy / result.position.w;
    result.pixel = float2((ndc.x * 0.5 + 0.5) * slug_viewport.x,
                          (0.5 - ndc.y * 0.5) * slug_viewport.y);

    SlugUnpack(input.tex, input.bnd, result.banding, result.glyph);
    result.color = input.col;
    return result;
}

[shader("fragment")]
float4 fsMain(VSOutput input) : SV_Target0 {
    if (discardByClip(input.pixel)) {
        discard;
    }

    float coverage = SlugRender(input.texcoord, input.banding, input.glyph);
    if (coverage < 1.0 / 255.0) {
        discard;
    }

    return float4(input.color.rgb, input.color.a * coverage);
}
