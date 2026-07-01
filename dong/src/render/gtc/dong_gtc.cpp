#include "dong_gtc.h"

#include "dong_gtc_compression.hpp"

#include "dong_gtc_format_map.hpp"



#include <SDL3/SDL_gpu.h>



#include <cstring>

#include <mutex>

#include <string>



struct DongGtcContext {

    DongGtcBackendKind backend;

    void* native_device;

    std::unique_ptr<dong::gtc_internal::CompressionService> sdl_service;

    std::string shader_dir;



    int (*in_dreaming_can_encode)(void* device, DongImageFormat src, DongImageFormat dst);

    DongImageDecoderResult (*in_dreaming_encode_rgba)(void* device, const DongDecodedImage* src, DongImageFormat dst,
                                                      const DongEncodeOptions* options, DongDecodedImage* out);

    int (*in_dreaming_compress_region)(void* device, const uint8_t* rgba, uint32_t w, uint32_t h,

                                       DongImageFormat dst, void* dst_texture, uint32_t x, uint32_t y);

};



static std::mutex g_default_mutex;

static DongGtcContext* g_default_ctx = nullptr;



static std::string default_shader_dir() {

#ifdef DONG_GTC_SHADER_DIR

    return DONG_GTC_SHADER_DIR;

#else

    return "third_party/gpu_texture_compress/sdk/shaders";

#endif

}



void dong_gtc_register_in_dreaming_callbacks(
    int (*can_encode)(void*, DongImageFormat, DongImageFormat),
    DongImageDecoderResult (*encode_rgba)(void*, const DongDecodedImage*, DongImageFormat,
                                            const DongEncodeOptions*, DongDecodedImage*),
    int (*compress_region)(void*, const uint8_t*, uint32_t, uint32_t, DongImageFormat, void*, uint32_t,
                           uint32_t)) {

    std::lock_guard<std::mutex> lock(g_default_mutex);

    if (g_default_ctx && g_default_ctx->backend == DONG_GTC_BACKEND_IN_DREAMING_GPU) {

        g_default_ctx->in_dreaming_can_encode = can_encode;

        g_default_ctx->in_dreaming_encode_rgba = encode_rgba;

        g_default_ctx->in_dreaming_compress_region = compress_region;

    }

}



DongGtcContext* dong_gtc_create(void* native_device, DongGtcBackendKind backend) {

    if (!native_device) {

        return nullptr;

    }

    auto* ctx = new (std::nothrow) DongGtcContext();

    if (!ctx) {

        return nullptr;

    }

    ctx->backend = backend;

    ctx->native_device = native_device;

    ctx->shader_dir = default_shader_dir();

    if (backend == DONG_GTC_BACKEND_SDL_GPU) {

        ctx->sdl_service = std::make_unique<dong::gtc_internal::CompressionService>(

            static_cast<SDL_GPUDevice*>(native_device), ctx->shader_dir);

    }

    return ctx;

}



void dong_gtc_destroy(DongGtcContext* ctx) {

    if (!ctx) {

        return;

    }

    {

        std::lock_guard<std::mutex> lock(g_default_mutex);

        if (g_default_ctx == ctx) {

            g_default_ctx = nullptr;

        }

    }

    delete ctx;

}



void dong_gtc_set_default(DongGtcContext* ctx) {

    std::lock_guard<std::mutex> lock(g_default_mutex);

    g_default_ctx = ctx;

}



DongGtcContext* dong_gtc_get_default(void) {

    std::lock_guard<std::mutex> lock(g_default_mutex);

    return g_default_ctx;

}



int dong_gtc_can_encode(DongGtcContext* ctx, DongImageFormat src_format, DongImageFormat dst_format) {

    if (src_format != DONG_IMAGE_FORMAT_RGBA8) {

        return 0;

    }

    if (!dong_image_format_is_compressed(dst_format)) {

        return 0;

    }

    if (!ctx) {

        return 0;

    }

    if (ctx->backend == DONG_GTC_BACKEND_IN_DREAMING_GPU) {

        if (ctx->in_dreaming_can_encode) {

            return ctx->in_dreaming_can_encode(ctx->native_device, src_format, dst_format);

        }

        return 0;

    }

    if (!ctx->sdl_service) {

        return 0;

    }

    ::gtc::GtcFormat gtc_fmt{};

    return dong::gtc_internal::image_format_to_gtc(dst_format, &gtc_fmt) ? 1 : 0;

}



static int encode_flags_from_options(const DongEncodeOptions* options) {

    if (!options) {

        return 0;

    }

    int flags = 0;

    if (options->normal_map) flags |= GTC_FLAG_NORMALMAP;

    if (options->srgb) flags |= GTC_FLAG_SRGB;

    return flags;

}



DongImageDecoderResult dong_gtc_encode_rgba(DongGtcContext* ctx, const DongDecodedImage* src,

                                           DongImageFormat dst_format, const DongEncodeOptions* options,

                                           DongDecodedImage* out_image) {

    if (!ctx || !src || !src->data || !out_image) {

        return DONG_IMAGE_ERR_INVALID_ARG;

    }

    if (src->format != DONG_IMAGE_FORMAT_RGBA8) {

        return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;

    }



    if (ctx->backend == DONG_GTC_BACKEND_IN_DREAMING_GPU) {

        if (ctx->in_dreaming_encode_rgba) {
            return ctx->in_dreaming_encode_rgba(ctx->native_device, src, dst_format, options, out_image);
        }

        return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;

    }



    if (!ctx->sdl_service) {

        return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;

    }



    std::memset(out_image, 0, sizeof(*out_image));

    std::vector<uint8_t> compressed;

    const int q = options ? (options->quality >= 67 ? 2 : (options->quality >= 34 ? 1 : 0)) : 1;

    if (!ctx->sdl_service->compress_rgba(static_cast<const uint8_t*>(src->data), src->width, src->height,

                                         dst_format, q, encode_flags_from_options(options), compressed)) {

        return DONG_IMAGE_ERR_ENCODE_FAILED;

    }



    void* heap = std::malloc(compressed.size());

    if (!heap) {

        return DONG_IMAGE_ERR_OUT_OF_MEMORY;

    }

    std::memcpy(heap, compressed.data(), compressed.size());

    out_image->data = heap;

    out_image->data_size = compressed.size();

    out_image->width = src->width;

    out_image->height = src->height;

    out_image->row_bytes = 0;

    out_image->format = dst_format;

    out_image->mip_levels = 1;

    return DONG_IMAGE_OK;

}



int dong_gtc_compress_to_texture_region_sdl(DongGtcContext* ctx, void* sdl_gpu_device,

                                            const uint8_t* rgba_pixels, uint32_t src_width,

                                            uint32_t src_height, DongImageFormat dst_format,

                                            void* dst_sdl_texture, uint32_t dst_x, uint32_t dst_y) {

    (void)sdl_gpu_device;

    if (!ctx || !ctx->sdl_service || !rgba_pixels || !dst_sdl_texture) {

        return 0;

    }

    return ctx->sdl_service->compress_to_texture_region(

               rgba_pixels, src_width, src_height, dst_format,

               static_cast<SDL_GPUTexture*>(dst_sdl_texture), dst_x, dst_y)

        ? 1

        : 0;

}



int dong_gtc_compress_to_texture_region_gpu(DongGtcContext* ctx, const uint8_t* rgba_pixels,

                                            uint32_t src_width, uint32_t src_height,

                                            DongImageFormat dst_format, void* dst_gpu_texture,

                                            uint32_t dst_x, uint32_t dst_y) {

    if (!ctx || ctx->backend != DONG_GTC_BACKEND_IN_DREAMING_GPU || !rgba_pixels || !dst_gpu_texture) {

        return 0;

    }

    if (ctx->in_dreaming_compress_region) {

        return ctx->in_dreaming_compress_region(ctx->native_device, rgba_pixels, src_width, src_height,

                                                dst_format, dst_gpu_texture, dst_x, dst_y);

    }

    return 0;

}



size_t dong_gtc_compressed_size(uint32_t width, uint32_t height, DongImageFormat format) {

    return dong::gtc_internal::compressed_size_for_format(width, height, format);

}


