#include "gpu_gtc_service.hpp"

#include "../../src/core/log.h"
#include "dong_gtc.h"
#include "../../src/render/gtc/dong_gtc_format_map.hpp"
#include "gpu_resource_manager.hpp"

#include <vector>

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU

static int gtc_can_encode(void* /*device*/, DongImageFormat src, DongImageFormat dst) {
    if (src != DONG_IMAGE_FORMAT_RGBA8 || !dong_image_format_is_compressed(dst)) {
        return 0;
    }
    ::gtc::GtcFormat gtc_fmt{};
    return dong::gtc_internal::image_format_to_gtc(dst, &gtc_fmt) ? 1 : 0;
}

static DongImageDecoderResult gtc_encode_rgba(void* /*device*/, const DongDecodedImage* src,
                                              DongImageFormat dst_format, const DongEncodeOptions* options,
                                              DongDecodedImage* out_image) {
    (void)options;
    if (!src || !out_image) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }
    /* BC7/ASTC compute encode via in-dreaming/gpu is pending; RGBA atlas path does not require this. */
    DONG_LOG_WARN("GpuGtcService: encode_rgba(%d) not yet implemented on in-dreaming/gpu", (int)dst_format);
    (void)src;
    return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;
}

static int gtc_compress_region(void* device, const uint8_t* rgba, uint32_t w, uint32_t h,
                               DongImageFormat dst_format, void* dst_texture, uint32_t x, uint32_t y) {
    (void)device;
    (void)rgba;
    (void)w;
    (void)h;
    (void)dst_format;
    (void)dst_texture;
    (void)x;
    (void)y;
    DONG_LOG_WARN("GpuGtcService: compress_to_texture_region not yet implemented on in-dreaming/gpu");
    return 0;
}

void gpu_gtc_service_register() {
    dong_gtc_register_in_dreaming_callbacks(gtc_can_encode, gtc_encode_rgba, gtc_compress_region);
}

#else

void gpu_gtc_service_register() {}

#endif

} // namespace dong::gpu_backend
