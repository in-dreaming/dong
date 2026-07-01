#include "dong_texture_bake.h"
#include "dong_gtc.h"

#include <algorithm>
#include <cstring>
#include <vector>

struct DongTextureBakeRequest {
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> canvas;
};

DongTextureBakeRequest* dong_texture_bake_create(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return nullptr;
    }
    auto* req = new (std::nothrow) DongTextureBakeRequest();
    if (!req) {
        return nullptr;
    }
    req->width = width;
    req->height = height;
    req->canvas.assign((size_t)width * height * 4, 0);
    return req;
}

void dong_texture_bake_destroy(DongTextureBakeRequest* req) {
    delete req;
}

static void blend_pixel(uint8_t* dst, const uint8_t* src, DongTextureBakeBlend blend) {
    const uint8_t sa = src[3];
    if (sa == 0) {
        return;
    }
    if (blend == DONG_TEXTURE_BAKE_BLEND_MULTIPLY) {
        dst[0] = (uint8_t)((dst[0] * src[0]) / 255);
        dst[1] = (uint8_t)((dst[1] * src[1]) / 255);
        dst[2] = (uint8_t)((dst[2] * src[2]) / 255);
        dst[3] = (uint8_t)std::max(dst[3], sa);
        return;
    }
    const uint8_t inv = (uint8_t)(255 - sa);
    dst[0] = (uint8_t)((src[0] * sa + dst[0] * inv) / 255);
    dst[1] = (uint8_t)((src[1] * sa + dst[1] * inv) / 255);
    dst[2] = (uint8_t)((src[2] * sa + dst[2] * inv) / 255);
    dst[3] = (uint8_t)std::min(255, (int)sa + (int)dst[3]);
}

int dong_texture_bake_add_layer(DongTextureBakeRequest* req,
                                const DongTextureBakeLayer* layer) {
    if (!req || !layer || !layer->rgba || layer->width == 0 || layer->height == 0) {
        return 0;
    }
    const uint32_t stride = layer->stride_bytes ? layer->stride_bytes : layer->width * 4;
    for (uint32_t y = 0; y < layer->height; ++y) {
        const int32_t dst_y = layer->dst_y + (int32_t)y;
        if (dst_y < 0 || (uint32_t)dst_y >= req->height) {
            continue;
        }
        for (uint32_t x = 0; x < layer->width; ++x) {
            const int32_t dst_x = layer->dst_x + (int32_t)x;
            if (dst_x < 0 || (uint32_t)dst_x >= req->width) {
                continue;
            }
            const uint8_t* src_px = layer->rgba + (size_t)y * stride + x * 4;
            uint8_t* dst_px = req->canvas.data() + ((size_t)dst_y * req->width + dst_x) * 4;
            blend_pixel(dst_px, src_px, layer->blend);
        }
    }
    return 1;
}

DongImageDecoderResult dong_texture_bake_submit(DongGtcContext* gtc,
                                                DongTextureBakeRequest* req,
                                                DongImageFormat dst_format,
                                                const DongEncodeOptions* options,
                                                DongDecodedImage* out_image) {
    if (!gtc || !req || !out_image) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }
    DongDecodedImage src{};
    src.data = req->canvas.data();
    src.data_size = req->canvas.size();
    src.width = req->width;
    src.height = req->height;
    src.row_bytes = req->width * 4;
    src.format = DONG_IMAGE_FORMAT_RGBA8;
    src.mip_levels = 1;
    return dong_gtc_encode_rgba(gtc, &src, dst_format, options, out_image);
}
