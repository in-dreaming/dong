#ifndef DONG_GTC_H
#define DONG_GTC_H

#include "dong_image_decoder.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DongGtcContext DongGtcContext;

typedef enum DongGtcBackendKind {
    DONG_GTC_BACKEND_SDL_GPU = 0,
    DONG_GTC_BACKEND_IN_DREAMING_GPU = 1,
} DongGtcBackendKind;

/* Create GTC context bound to an existing GPU device (SDL_GPUDevice* or GpuDevice*). */
DongGtcContext* dong_gtc_create(void* native_device, DongGtcBackendKind backend);
void dong_gtc_destroy(DongGtcContext* ctx);

/* Register device for global encode path (image decoder). */
void dong_gtc_set_default(DongGtcContext* ctx);
DongGtcContext* dong_gtc_get_default(void);

int dong_gtc_can_encode(DongGtcContext* ctx,
                        DongImageFormat src_format,
                        DongImageFormat dst_format);

DongImageDecoderResult dong_gtc_encode_rgba(DongGtcContext* ctx,
                                            const DongDecodedImage* src,
                                            DongImageFormat dst_format,
                                            const DongEncodeOptions* options,
                                            DongDecodedImage* out_image);

/* GPU-side: compress RGBA pixels into a sub-rectangle of a compressed atlas texture. */
int dong_gtc_compress_to_texture_region_sdl(DongGtcContext* ctx,
                                            void* sdl_gpu_device,
                                            const uint8_t* rgba_pixels,
                                            uint32_t src_width,
                                            uint32_t src_height,
                                            DongImageFormat dst_format,
                                            void* dst_sdl_texture,
                                            uint32_t dst_x,
                                            uint32_t dst_y);

/* GPU-side: compress into a DongGPUTexture sub-rectangle (in-dreaming/gpu backend). */
int dong_gtc_compress_to_texture_region_gpu(DongGtcContext* ctx,
                                          const uint8_t* rgba_pixels,
                                          uint32_t src_width,
                                          uint32_t src_height,
                                          DongImageFormat dst_format,
                                          void* dst_gpu_texture,
                                          uint32_t dst_x,
                                          uint32_t dst_y);

void dong_gtc_register_in_dreaming_callbacks(
    int (*can_encode)(void*, DongImageFormat, DongImageFormat),
    DongImageDecoderResult (*encode_rgba)(void*, const DongDecodedImage*, DongImageFormat,
                                          const DongEncodeOptions*, DongDecodedImage*),
    int (*compress_region)(void*, const uint8_t*, uint32_t, uint32_t, DongImageFormat, void*, uint32_t,
                           uint32_t));

size_t dong_gtc_compressed_size(uint32_t width, uint32_t height, DongImageFormat format);

#ifdef __cplusplus
}
#endif

#endif /* DONG_GTC_H */
