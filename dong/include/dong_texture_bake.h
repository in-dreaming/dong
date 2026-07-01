#ifndef DONG_TEXTURE_BAKE_H
#define DONG_TEXTURE_BAKE_H

#include "dong_image_decoder.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DongTextureBakeRequest DongTextureBakeRequest;
typedef struct DongGtcContext DongGtcContext;

typedef enum DongTextureBakeBlend {
    DONG_TEXTURE_BAKE_BLEND_SRC_OVER = 0,
    DONG_TEXTURE_BAKE_BLEND_MULTIPLY,
} DongTextureBakeBlend;

typedef struct DongTextureBakeLayer {
    const uint8_t* rgba;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
    int32_t dst_x;
    int32_t dst_y;
    DongTextureBakeBlend blend;
} DongTextureBakeLayer;

DongTextureBakeRequest* dong_texture_bake_create(uint32_t width, uint32_t height);
void dong_texture_bake_destroy(DongTextureBakeRequest* req);

int dong_texture_bake_add_layer(DongTextureBakeRequest* req,
                                const DongTextureBakeLayer* layer);

/* Bake layers to RGBA staging, then GPU-compress to dst_format. */
DongImageDecoderResult dong_texture_bake_submit(DongGtcContext* gtc,
                                                DongTextureBakeRequest* req,
                                                DongImageFormat dst_format,
                                                const DongEncodeOptions* options,
                                                DongDecodedImage* out_image);

#ifdef __cplusplus
}
#endif

#endif /* DONG_TEXTURE_BAKE_H */
