// GPU backend image decoder implementation.
// Same decoding strategy as the SDL backend's dong_sdl_image_decoder.c (stb_image +
// DongGtc for GPU-compressed encode), but with no SDL dependency at all, so the pure
// GPU backend can register an image decoder without linking SDL.

#include "dong_gpu_image_decoder.h"
#include "dong_gtc.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Premultiplied-alpha helper
// =============================================================================

static void premultiply_rgba8(unsigned char* pixels, int width, int height) {
    if (!pixels || width <= 0 || height <= 0) return;
    const size_t count = (size_t)width * (size_t)height;
    for (size_t i = 0; i < count; ++i) {
        unsigned char* p = pixels + i * 4;
        const unsigned char a = p[3];
        if (a == 255) continue;
        if (a == 0) {
            p[0] = p[1] = p[2] = 0;
            continue;
        }
        p[0] = (unsigned char)((p[0] * a + 127) / 255);
        p[1] = (unsigned char)((p[1] * a + 127) / 255);
        p[2] = (unsigned char)((p[2] * a + 127) / 255);
    }
}

// =============================================================================
// stb_image integration
// =============================================================================

#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244 4996)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include "stb_image.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// =============================================================================
// Internal Data Structures
// =============================================================================

typedef struct GpuImageDecoderData {
    int placeholder;
} GpuImageDecoderData;

// =============================================================================
// Format Detection
// =============================================================================

static int is_png(const void* data, size_t size) {
    if (size < 8) return 0;
    const unsigned char* p = (const unsigned char*)data;
    return p[0] == 0x89 && p[1] == 0x50 && p[2] == 0x4E && p[3] == 0x47 &&
           p[4] == 0x0D && p[5] == 0x0A && p[6] == 0x1A && p[7] == 0x0A;
}

static int is_jpeg(const void* data, size_t size) {
    if (size < 3) return 0;
    const unsigned char* p = (const unsigned char*)data;
    return p[0] == 0xFF && p[1] == 0xD8 && p[2] == 0xFF;
}

static int is_bmp(const void* data, size_t size) {
    if (size < 2) return 0;
    const unsigned char* p = (const unsigned char*)data;
    return p[0] == 0x42 && p[1] == 0x4D;
}

static int is_gif(const void* data, size_t size) {
    if (size < 6) return 0;
    const unsigned char* p = (const unsigned char*)data;
    return p[0] == 'G' && p[1] == 'I' && p[2] == 'F' &&
           p[3] == '8' && (p[4] == '7' || p[4] == '9') && p[5] == 'a';
}

static int is_tga(const void* data, size_t size, const char* hint_ext) {
    (void)data;
    (void)size;
    if (!hint_ext) return 0;
    size_t len = strlen(hint_ext);
    if (len < 4) return 0;
    const char* ext = hint_ext + len - 4;
    return (strcmp(ext, ".tga") == 0 || strcmp(ext, ".TGA") == 0);
}

// =============================================================================
// VTable Implementation
// =============================================================================

static int gpu_can_decode(DongImageDecoder* decoder, const void* data, size_t size, const char* hint_ext) {
    (void)decoder;
    if (!data || size == 0) return 0;

    if (is_png(data, size)) return 1;
    if (is_jpeg(data, size)) return 1;
    if (is_bmp(data, size)) return 1;
    if (is_gif(data, size)) return 1;
    if (is_tga(data, size, hint_ext)) return 1;

    if (hint_ext) {
        size_t len = strlen(hint_ext);
        if (len >= 4) {
            const char* ext = hint_ext + len - 4;
            if (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0) return 1;
            if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".JPG") == 0) return 1;
            if (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0) return 1;
            if (strcmp(ext, ".gif") == 0 || strcmp(ext, ".GIF") == 0) return 1;
            if (strcmp(ext, ".tga") == 0 || strcmp(ext, ".TGA") == 0) return 1;
        }
        if (len >= 5) {
            const char* ext = hint_ext + len - 5;
            if (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".JPEG") == 0) return 1;
        }
    }

    return 0;
}

static DongImageDecoderResult gpu_decode(DongImageDecoder* decoder, const void* data, size_t size,
                                         DongDecodedImage* out_image) {
    (void)decoder;
    if (!data || size == 0 || !out_image) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }

    memset(out_image, 0, sizeof(*out_image));

    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = stbi_load_from_memory(
        (const stbi_uc*)data,
        (int)size,
        &width, &height, &channels,
        4  // Force RGBA
    );

    if (!pixels) {
        return DONG_IMAGE_ERR_DECODE_FAILED;
    }
    if (width <= 0 || height <= 0) {
        stbi_image_free(pixels);
        return DONG_IMAGE_ERR_DECODE_FAILED;
    }

    // Store decoded images as premultiplied alpha to avoid halo artifacts under
    // linear filtering (matches the SDL backend decoder's convention).
    premultiply_rgba8(pixels, width, height);

    out_image->data = pixels;
    out_image->data_size = (size_t)width * (size_t)height * 4;
    out_image->width = (uint32_t)width;
    out_image->height = (uint32_t)height;
    out_image->row_bytes = (uint32_t)width * 4;
    out_image->format = DONG_IMAGE_FORMAT_RGBA8;
    out_image->mip_levels = 1;
    out_image->user_data = NULL;

    return DONG_IMAGE_OK;
}

static void gpu_free_image(DongImageDecoder* decoder, DongDecodedImage* image) {
    (void)decoder;
    if (!image || !image->data) return;

    if (dong_image_format_is_compressed(image->format)) {
        free(image->data);
    } else {
        stbi_image_free(image->data);
    }
    memset(image, 0, sizeof(*image));
}

static int gpu_can_encode(DongImageDecoder* decoder, DongImageFormat src_format, DongImageFormat dst_format) {
    (void)decoder;
    DongGtcContext* gtc = dong_gtc_get_default();
    if (!gtc) {
        return 0;
    }
    return dong_gtc_can_encode(gtc, src_format, dst_format);
}

static DongImageDecoderResult gpu_encode(DongImageDecoder* decoder,
                                         const DongDecodedImage* src,
                                         DongImageFormat dst_format,
                                         const DongEncodeOptions* options,
                                         DongDecodedImage* out_image) {
    (void)decoder;
    DongGtcContext* gtc = dong_gtc_get_default();
    if (!gtc) {
        return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;
    }
    return dong_gtc_encode_rgba(gtc, src, dst_format, options, out_image);
}

static int gpu_supports_format(DongImageDecoder* decoder, DongImageFormat format) {
    (void)decoder;
    switch (format) {
        case DONG_IMAGE_FORMAT_RGBA8:
            return 1;
        default:
            return 0;
    }
}

// =============================================================================
// VTable Definition
// =============================================================================

static const DongImageDecoderVTable g_gpu_image_decoder_vtable = {
    .can_decode = gpu_can_decode,
    .decode = gpu_decode,
    .free_image = gpu_free_image,
    .can_encode = gpu_can_encode,
    .encode = gpu_encode,
    .gpu_supports_format = gpu_supports_format,
};

// =============================================================================
// Public API
// =============================================================================

DongImageDecoder* dong_gpu_image_decoder_create(void) {
    DongImageDecoder* decoder = (DongImageDecoder*)malloc(sizeof(DongImageDecoder) + sizeof(GpuImageDecoderData));
    if (!decoder) {
        return NULL;
    }

    decoder->vtable = &g_gpu_image_decoder_vtable;
    decoder->user_data = (char*)decoder + sizeof(DongImageDecoder);

    GpuImageDecoderData* data = (GpuImageDecoderData*)decoder->user_data;
    memset(data, 0, sizeof(*data));

    return decoder;
}

void dong_gpu_image_decoder_destroy(DongImageDecoder* decoder) {
    if (!decoder) return;
    free(decoder);
}
