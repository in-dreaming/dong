// SDL Image Decoder Implementation
// Uses stb_image for decoding common formats.
// Includes ASTC and BC encoders for GPU texture compression.

#include "dong_sdl_image_decoder.h"
#include "dong_gtc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <SDL3/SDL.h>

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

// stb_image (implementation lives in dong_render / gpu_texture_compress texture_loader)
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA

// Silence warnings in stb_image
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

typedef struct SDLImageDecoderData {
    int placeholder;  // Reserved for future use (e.g., encoder state)
} SDLImageDecoderData;

// =============================================================================
// Format Detection
// =============================================================================

static int is_png(const void* data, size_t size) {
    if (size < 8) return 0;
    const unsigned char* p = (const unsigned char*)data;
    // PNG signature: 89 50 4E 47 0D 0A 1A 0A
    return p[0] == 0x89 && p[1] == 0x50 && p[2] == 0x4E && p[3] == 0x47 &&
           p[4] == 0x0D && p[5] == 0x0A && p[6] == 0x1A && p[7] == 0x0A;
}

static int is_jpeg(const void* data, size_t size) {
    if (size < 3) return 0;
    const unsigned char* p = (const unsigned char*)data;
    // JPEG signature: FF D8 FF
    return p[0] == 0xFF && p[1] == 0xD8 && p[2] == 0xFF;
}

static int is_bmp(const void* data, size_t size) {
    if (size < 2) return 0;
    const unsigned char* p = (const unsigned char*)data;
    // BMP signature: 42 4D ("BM")
    return p[0] == 0x42 && p[1] == 0x4D;
}

static int is_gif(const void* data, size_t size) {
    if (size < 6) return 0;
    const unsigned char* p = (const unsigned char*)data;
    // GIF signature: "GIF87a" or "GIF89a"
    return p[0] == 'G' && p[1] == 'I' && p[2] == 'F' &&
           p[3] == '8' && (p[4] == '7' || p[4] == '9') && p[5] == 'a';
}

static int is_tga(const void* data, size_t size, const char* hint_ext) {
    // TGA has no reliable magic number, rely on extension hint
    if (!hint_ext) return 0;
    size_t len = strlen(hint_ext);
    if (len < 4) return 0;
    const char* ext = hint_ext + len - 4;
    return (strcmp(ext, ".tga") == 0 || strcmp(ext, ".TGA") == 0);
}

static int is_ktx2(const void* data, size_t size) {
    if (size < 12) return 0;
    const unsigned char* p = (const unsigned char*)data;
    // KTX2 identifier
    static const unsigned char ktx2_id[12] = {
        0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
    };
    return memcmp(p, ktx2_id, 12) == 0;
}

static int is_dds(const void* data, size_t size) {
    if (size < 4) return 0;
    const unsigned char* p = (const unsigned char*)data;
    // DDS magic: "DDS "
    return p[0] == 'D' && p[1] == 'D' && p[2] == 'S' && p[3] == ' ';
}

static int is_astc(const void* data, size_t size) {
    if (size < 4) return 0;
    const unsigned char* p = (const unsigned char*)data;
    // ASTC magic: 0x5CA1AB13
    return p[0] == 0x13 && p[1] == 0xAB && p[2] == 0xA1 && p[3] == 0x5C;
}

// =============================================================================
// VTable Implementation
// =============================================================================

static int sdl_can_decode(DongImageDecoder* decoder, const void* data, size_t size, const char* hint_ext) {
    (void)decoder;

    if (!data || size == 0) return 0;

    // Check by magic number
    if (is_png(data, size)) return 1;
    if (is_jpeg(data, size)) return 1;
    if (is_bmp(data, size)) return 1;
    if (is_gif(data, size)) return 1;
    if (is_tga(data, size, hint_ext)) return 1;

    // Compressed formats (for future support)
    if (is_ktx2(data, size)) return 1;
    if (is_dds(data, size)) return 1;
    if (is_astc(data, size)) return 1;

    // Check by extension hint
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
            if (strcmp(ext, ".ktx2") == 0 || strcmp(ext, ".KTX2") == 0) return 1;
            if (strcmp(ext, ".astc") == 0 || strcmp(ext, ".ASTC") == 0) return 1;
        }
    }

    return 0;
}

static DongImageDecoderResult sdl_decode(DongImageDecoder* decoder, const void* data, size_t size, DongDecodedImage* out_image) {
    (void)decoder;

    if (!data || size == 0 || !out_image) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }

    memset(out_image, 0, sizeof(*out_image));

    // Handle compressed formats (KTX2/DDS/ASTC)
    if (is_ktx2(data, size)) {
        // TODO: KTX2 parsing - extract compressed data directly
        // For now, return unsupported
        return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;
    }

    if (is_dds(data, size)) {
        // TODO: DDS parsing - extract BC compressed data
        return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;
    }

    if (is_astc(data, size)) {
        // TODO: ASTC file parsing - extract compressed blocks
        return DONG_IMAGE_ERR_UNSUPPORTED_FORMAT;
    }

    // Use stb_image for uncompressed formats
    int width = 0, height = 0, channels = 0;

    // Always request RGBA output for consistency
    unsigned char* pixels = stbi_load_from_memory(
        (const stbi_uc*)data,
        (int)size,
        &width, &height, &channels,
        4  // Force 4 channels (RGBA)
    );

    if (!pixels) {
        return DONG_IMAGE_ERR_DECODE_FAILED;
    }

    if (width <= 0 || height <= 0) {
        stbi_image_free(pixels);
        return DONG_IMAGE_ERR_DECODE_FAILED;
    }

    // Store decoded images as premultiplied alpha.
    // This avoids halo artifacts when sampling with linear filtering.
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

static void sdl_free_image(DongImageDecoder* decoder, DongDecodedImage* image) {
    (void)decoder;

    if (!image || !image->data) return;

    // For compressed formats (from encode), we used malloc
    // For decoded formats (from stb_image), stbi_image_free uses its allocator
    // Both default to malloc/free, so free() works for both
    if (dong_image_format_is_compressed(image->format)) {
        free(image->data);
    } else {
        stbi_image_free(image->data);
    }

    memset(image, 0, sizeof(*image));
}

static int sdl_can_encode(DongImageDecoder* decoder, DongImageFormat src_format, DongImageFormat dst_format) {
    (void)decoder;
    DongGtcContext* gtc = dong_gtc_get_default();
    if (!gtc) {
        return 0;
    }
    return dong_gtc_can_encode(gtc, src_format, dst_format);
}

static DongImageDecoderResult sdl_encode(DongImageDecoder* decoder,
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

static int sdl_gpu_supports_format(DongImageDecoder* decoder, DongImageFormat format) {
    (void)decoder;

    // TODO: Query SDL_GPU for actual format support
    // For now, assume common formats are supported

    switch (format) {
        case DONG_IMAGE_FORMAT_RGBA8:
            return 1;  // Always supported

        case DONG_IMAGE_FORMAT_ASTC_4x4:
        case DONG_IMAGE_FORMAT_ASTC_5x5:
        case DONG_IMAGE_FORMAT_ASTC_6x6:
        case DONG_IMAGE_FORMAT_ASTC_8x8:
            // ASTC is widely supported on:
            // - All iOS devices (A8+)
            // - Most Android devices (OpenGL ES 3.2+)
            // - Intel/AMD/NVIDIA desktop GPUs (Vulkan)
            // TODO: Actually query SDL_GPU caps
            return 0;  // Conservative default

        case DONG_IMAGE_FORMAT_BC1:
        case DONG_IMAGE_FORMAT_BC2:
        case DONG_IMAGE_FORMAT_BC3:
        case DONG_IMAGE_FORMAT_BC4:
        case DONG_IMAGE_FORMAT_BC5:
        case DONG_IMAGE_FORMAT_BC7:
            // BC formats supported on:
            // - All Windows/DirectX GPUs
            // - Most desktop Vulkan implementations
            // TODO: Actually query SDL_GPU caps
            return 0;  // Conservative default

        case DONG_IMAGE_FORMAT_BC6H:
        case DONG_IMAGE_FORMAT_BC6H_SF:
            // BC6H requires D3D11+ / Vulkan
            return 0;

        default:
            return 0;
    }
}

// =============================================================================
// VTable Definition
// =============================================================================

static const DongImageDecoderVTable g_sdl_image_decoder_vtable = {
    .can_decode = sdl_can_decode,
    .decode = sdl_decode,
    .free_image = sdl_free_image,
    .can_encode = sdl_can_encode,
    .encode = sdl_encode,
    .gpu_supports_format = sdl_gpu_supports_format,
};

// =============================================================================
// Public API
// =============================================================================

DongImageDecoder* dong_sdl_image_decoder_create(void) {
    DongImageDecoder* decoder = (DongImageDecoder*)malloc(sizeof(DongImageDecoder) + sizeof(SDLImageDecoderData));
    if (!decoder) {
        return NULL;
    }

    decoder->vtable = &g_sdl_image_decoder_vtable;
    decoder->user_data = (char*)decoder + sizeof(DongImageDecoder);

    SDLImageDecoderData* data = (SDLImageDecoderData*)decoder->user_data;
    memset(data, 0, sizeof(*data));

    return decoder;
}

void dong_sdl_image_decoder_destroy(DongImageDecoder* decoder) {
    if (!decoder) return;
    free(decoder);
}
