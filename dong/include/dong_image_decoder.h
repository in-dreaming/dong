#ifndef DONG_IMAGE_DECODER_H
#define DONG_IMAGE_DECODER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Dong Image Decoder (Platform-injected)
// =============================================================================
// Provides image decoding and GPU texture compression capabilities.
// Applications/backends inject an implementation via dong_platform_set_image_decoder().
//
// Supports:
// - Decoding: PNG, JPEG, BMP, etc. -> RGBA/RGB
// - GPU compressed formats: ASTC, BC1-BC7, etc.
// - Encoding: RGBA -> GPU compressed formats (for atlas optimization)

// =============================================================================
// Image Formats
// =============================================================================

typedef enum DongImageFormat {
    DONG_IMAGE_FORMAT_UNKNOWN = 0,

    // Uncompressed formats
    DONG_IMAGE_FORMAT_R8,           // Single channel, 8-bit
    DONG_IMAGE_FORMAT_RG8,          // Two channels, 8-bit each
    DONG_IMAGE_FORMAT_RGB8,         // Three channels, 8-bit each
    DONG_IMAGE_FORMAT_RGBA8,        // Four channels, 8-bit each (standard)
    DONG_IMAGE_FORMAT_RGBA16F,      // Four channels, 16-bit float (HDR)
    DONG_IMAGE_FORMAT_RGBA32F,      // Four channels, 32-bit float (HDR)

    // ASTC compressed formats (mobile + modern desktop)
    DONG_IMAGE_FORMAT_ASTC_4x4,     // 8 bpp, highest quality
    DONG_IMAGE_FORMAT_ASTC_5x4,     // 6.4 bpp
    DONG_IMAGE_FORMAT_ASTC_5x5,     // 5.12 bpp
    DONG_IMAGE_FORMAT_ASTC_6x5,     // 4.27 bpp
    DONG_IMAGE_FORMAT_ASTC_6x6,     // 3.56 bpp
    DONG_IMAGE_FORMAT_ASTC_8x5,     // 3.2 bpp
    DONG_IMAGE_FORMAT_ASTC_8x6,     // 2.67 bpp
    DONG_IMAGE_FORMAT_ASTC_8x8,     // 2 bpp, smallest
    DONG_IMAGE_FORMAT_ASTC_10x5,    // 2.56 bpp
    DONG_IMAGE_FORMAT_ASTC_10x6,    // 2.13 bpp
    DONG_IMAGE_FORMAT_ASTC_10x8,    // 1.6 bpp
    DONG_IMAGE_FORMAT_ASTC_10x10,   // 1.28 bpp
    DONG_IMAGE_FORMAT_ASTC_12x10,   // 1.07 bpp
    DONG_IMAGE_FORMAT_ASTC_12x12,   // 0.89 bpp, lowest quality

    // BC (DXT/DXGI) compressed formats (desktop)
    DONG_IMAGE_FORMAT_BC1,          // DXT1, 4 bpp, RGB + 1-bit alpha
    DONG_IMAGE_FORMAT_BC2,          // DXT3, 8 bpp, RGB + explicit alpha
    DONG_IMAGE_FORMAT_BC3,          // DXT5, 8 bpp, RGB + interpolated alpha
    DONG_IMAGE_FORMAT_BC4,          // 4 bpp, single channel
    DONG_IMAGE_FORMAT_BC5,          // 8 bpp, two channels (normal maps)
    DONG_IMAGE_FORMAT_BC6H,         // 8 bpp, HDR RGB (unsigned)
    DONG_IMAGE_FORMAT_BC6H_SF,      // 8 bpp, HDR RGB (signed)
    DONG_IMAGE_FORMAT_BC7,          // 8 bpp, high quality RGBA

    // ETC compressed formats (OpenGL ES / mobile)
    DONG_IMAGE_FORMAT_ETC1,         // 4 bpp, RGB
    DONG_IMAGE_FORMAT_ETC2_RGB,     // 4 bpp, RGB
    DONG_IMAGE_FORMAT_ETC2_RGBA,    // 8 bpp, RGBA
    DONG_IMAGE_FORMAT_ETC2_RGBA1,   // 4 bpp, RGB + 1-bit alpha

    DONG_IMAGE_FORMAT_COUNT
} DongImageFormat;

// =============================================================================
// Decoded Image Data
// =============================================================================

typedef struct DongDecodedImage {
    void* data;                 // Pixel/block data (owned by decoder until free_image)
    size_t data_size;           // Total size in bytes
    uint32_t width;             // Image width in pixels
    uint32_t height;            // Image height in pixels
    uint32_t row_bytes;         // Bytes per row (pitch), may include padding
    DongImageFormat format;     // Pixel format
    uint32_t mip_levels;        // Number of mip levels (1 = no mipmaps)
    void* user_data;            // Decoder-specific data for free_image
} DongDecodedImage;

// =============================================================================
// Encode Quality Settings
// =============================================================================

typedef enum DongEncodeQuality {
    DONG_ENCODE_QUALITY_FASTEST = 0,    // Fastest encoding, lower quality
    DONG_ENCODE_QUALITY_FAST = 25,
    DONG_ENCODE_QUALITY_NORMAL = 50,    // Balanced
    DONG_ENCODE_QUALITY_HIGH = 75,
    DONG_ENCODE_QUALITY_BEST = 100,     // Best quality, slowest
} DongEncodeQuality;

typedef struct DongEncodeOptions {
    DongEncodeQuality quality;
    int generate_mipmaps;       // 1 = generate mipmaps, 0 = single level
    int srgb;                   // 1 = sRGB color space, 0 = linear
    int normal_map;             // 1 = optimize for normal maps
    int hdr;                    // 1 = HDR content (for BC6H)
} DongEncodeOptions;

// Default encode options
static inline DongEncodeOptions dong_encode_options_default(void) {
    DongEncodeOptions opts = {
        .quality = DONG_ENCODE_QUALITY_NORMAL,
        .generate_mipmaps = 0,
        .srgb = 1,
        .normal_map = 0,
        .hdr = 0
    };
    return opts;
}

// =============================================================================
// Image Decoder Interface
// =============================================================================

typedef struct DongImageDecoder DongImageDecoder;

typedef enum DongImageDecoderResult {
    DONG_IMAGE_OK = 0,
    DONG_IMAGE_ERR_INVALID_ARG = 1,
    DONG_IMAGE_ERR_UNSUPPORTED_FORMAT = 2,
    DONG_IMAGE_ERR_DECODE_FAILED = 3,
    DONG_IMAGE_ERR_ENCODE_FAILED = 4,
    DONG_IMAGE_ERR_OUT_OF_MEMORY = 5,
    DONG_IMAGE_ERR_IO = 6,
} DongImageDecoderResult;

typedef struct DongImageDecoderVTable {
    // Check if the decoder can handle this data.
    // hint_ext: optional file extension hint (e.g., ".png", ".ktx2")
    // Returns: 1 if can decode, 0 otherwise
    int (*can_decode)(DongImageDecoder* decoder,
                      const void* data, size_t size,
                      const char* hint_ext);

    // Decode image data.
    // Returns: DONG_IMAGE_OK on success
    DongImageDecoderResult (*decode)(DongImageDecoder* decoder,
                                     const void* data, size_t size,
                                     DongDecodedImage* out_image);

    // Free decoded image data.
    void (*free_image)(DongImageDecoder* decoder, DongDecodedImage* image);

    // Check if encoding from src_format to dst_format is supported.
    // Returns: 1 if supported, 0 otherwise
    int (*can_encode)(DongImageDecoder* decoder,
                      DongImageFormat src_format,
                      DongImageFormat dst_format);

    // Encode/transcode image to a different format.
    // src: Source image (must be uncompressed for GPU compression)
    // dst_format: Target format
    // options: Encoding options
    // out_image: Output image (caller must call free_image)
    // Returns: DONG_IMAGE_OK on success
    DongImageDecoderResult (*encode)(DongImageDecoder* decoder,
                                     const DongDecodedImage* src,
                                     DongImageFormat dst_format,
                                     const DongEncodeOptions* options,
                                     DongDecodedImage* out_image);

    // Query GPU format support (optional, may be NULL)
    // Returns: 1 if the GPU supports this format natively
    int (*gpu_supports_format)(DongImageDecoder* decoder, DongImageFormat format);

} DongImageDecoderVTable;

struct DongImageDecoder {
    const DongImageDecoderVTable* vtable;
    void* user_data;
};

// =============================================================================
// Convenience Functions
// =============================================================================

static inline int dong_image_can_decode(DongImageDecoder* decoder,
                                        const void* data, size_t size,
                                        const char* hint_ext) {
    if (!decoder || !decoder->vtable || !decoder->vtable->can_decode) {
        return 0;
    }
    return decoder->vtable->can_decode(decoder, data, size, hint_ext);
}

static inline DongImageDecoderResult dong_image_decode(DongImageDecoder* decoder,
                                                       const void* data, size_t size,
                                                       DongDecodedImage* out_image) {
    if (!decoder || !decoder->vtable || !decoder->vtable->decode) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }
    if (!out_image) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }
    return decoder->vtable->decode(decoder, data, size, out_image);
}

static inline void dong_image_free(DongImageDecoder* decoder, DongDecodedImage* image) {
    if (!decoder || !decoder->vtable || !decoder->vtable->free_image || !image) {
        return;
    }
    decoder->vtable->free_image(decoder, image);
}

static inline int dong_image_can_encode(DongImageDecoder* decoder,
                                        DongImageFormat src_format,
                                        DongImageFormat dst_format) {
    if (!decoder || !decoder->vtable || !decoder->vtable->can_encode) {
        return 0;
    }
    return decoder->vtable->can_encode(decoder, src_format, dst_format);
}

static inline DongImageDecoderResult dong_image_encode(DongImageDecoder* decoder,
                                                       const DongDecodedImage* src,
                                                       DongImageFormat dst_format,
                                                       const DongEncodeOptions* options,
                                                       DongDecodedImage* out_image) {
    if (!decoder || !decoder->vtable || !decoder->vtable->encode) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }
    if (!src || !out_image) {
        return DONG_IMAGE_ERR_INVALID_ARG;
    }
    DongEncodeOptions default_opts = dong_encode_options_default();
    return decoder->vtable->encode(decoder, src, dst_format,
                                   options ? options : &default_opts, out_image);
}

static inline int dong_image_gpu_supports(DongImageDecoder* decoder, DongImageFormat format) {
    if (!decoder || !decoder->vtable || !decoder->vtable->gpu_supports_format) {
        return 0;
    }
    return decoder->vtable->gpu_supports_format(decoder, format);
}

// =============================================================================
// Format Utilities
// =============================================================================

// Check if format is a compressed GPU format
static inline int dong_image_format_is_compressed(DongImageFormat format) {
    return format >= DONG_IMAGE_FORMAT_ASTC_4x4 && format < DONG_IMAGE_FORMAT_COUNT;
}

// Check if format is ASTC
static inline int dong_image_format_is_astc(DongImageFormat format) {
    return format >= DONG_IMAGE_FORMAT_ASTC_4x4 && format <= DONG_IMAGE_FORMAT_ASTC_12x12;
}

// Check if format is BC (DXT)
static inline int dong_image_format_is_bc(DongImageFormat format) {
    return format >= DONG_IMAGE_FORMAT_BC1 && format <= DONG_IMAGE_FORMAT_BC7;
}

// Check if format is ETC
static inline int dong_image_format_is_etc(DongImageFormat format) {
    return format >= DONG_IMAGE_FORMAT_ETC1 && format <= DONG_IMAGE_FORMAT_ETC2_RGBA1;
}

// Get block size for compressed formats (returns 1 for uncompressed)
static inline void dong_image_format_block_size(DongImageFormat format,
                                                uint32_t* block_w,
                                                uint32_t* block_h) {
    uint32_t w = 1, h = 1;
    switch (format) {
        case DONG_IMAGE_FORMAT_ASTC_4x4:  w = 4;  h = 4;  break;
        case DONG_IMAGE_FORMAT_ASTC_5x4:  w = 5;  h = 4;  break;
        case DONG_IMAGE_FORMAT_ASTC_5x5:  w = 5;  h = 5;  break;
        case DONG_IMAGE_FORMAT_ASTC_6x5:  w = 6;  h = 5;  break;
        case DONG_IMAGE_FORMAT_ASTC_6x6:  w = 6;  h = 6;  break;
        case DONG_IMAGE_FORMAT_ASTC_8x5:  w = 8;  h = 5;  break;
        case DONG_IMAGE_FORMAT_ASTC_8x6:  w = 8;  h = 6;  break;
        case DONG_IMAGE_FORMAT_ASTC_8x8:  w = 8;  h = 8;  break;
        case DONG_IMAGE_FORMAT_ASTC_10x5: w = 10; h = 5;  break;
        case DONG_IMAGE_FORMAT_ASTC_10x6: w = 10; h = 6;  break;
        case DONG_IMAGE_FORMAT_ASTC_10x8: w = 10; h = 8;  break;
        case DONG_IMAGE_FORMAT_ASTC_10x10:w = 10; h = 10; break;
        case DONG_IMAGE_FORMAT_ASTC_12x10:w = 12; h = 10; break;
        case DONG_IMAGE_FORMAT_ASTC_12x12:w = 12; h = 12; break;
        case DONG_IMAGE_FORMAT_BC1:
        case DONG_IMAGE_FORMAT_BC2:
        case DONG_IMAGE_FORMAT_BC3:
        case DONG_IMAGE_FORMAT_BC4:
        case DONG_IMAGE_FORMAT_BC5:
        case DONG_IMAGE_FORMAT_BC6H:
        case DONG_IMAGE_FORMAT_BC6H_SF:
        case DONG_IMAGE_FORMAT_BC7:
        case DONG_IMAGE_FORMAT_ETC1:
        case DONG_IMAGE_FORMAT_ETC2_RGB:
        case DONG_IMAGE_FORMAT_ETC2_RGBA:
        case DONG_IMAGE_FORMAT_ETC2_RGBA1:
            w = 4; h = 4; break;
        default:
            w = 1; h = 1; break;
    }
    if (block_w) *block_w = w;
    if (block_h) *block_h = h;
}

// Get bytes per block for compressed formats, or bytes per pixel for uncompressed
static inline uint32_t dong_image_format_bytes_per_block(DongImageFormat format) {
    switch (format) {
        // Uncompressed (bytes per pixel)
        case DONG_IMAGE_FORMAT_R8:        return 1;
        case DONG_IMAGE_FORMAT_RG8:       return 2;
        case DONG_IMAGE_FORMAT_RGB8:      return 3;
        case DONG_IMAGE_FORMAT_RGBA8:     return 4;
        case DONG_IMAGE_FORMAT_RGBA16F:   return 8;
        case DONG_IMAGE_FORMAT_RGBA32F:   return 16;

        // ASTC (all block sizes are 16 bytes)
        case DONG_IMAGE_FORMAT_ASTC_4x4:
        case DONG_IMAGE_FORMAT_ASTC_5x4:
        case DONG_IMAGE_FORMAT_ASTC_5x5:
        case DONG_IMAGE_FORMAT_ASTC_6x5:
        case DONG_IMAGE_FORMAT_ASTC_6x6:
        case DONG_IMAGE_FORMAT_ASTC_8x5:
        case DONG_IMAGE_FORMAT_ASTC_8x6:
        case DONG_IMAGE_FORMAT_ASTC_8x8:
        case DONG_IMAGE_FORMAT_ASTC_10x5:
        case DONG_IMAGE_FORMAT_ASTC_10x6:
        case DONG_IMAGE_FORMAT_ASTC_10x8:
        case DONG_IMAGE_FORMAT_ASTC_10x10:
        case DONG_IMAGE_FORMAT_ASTC_12x10:
        case DONG_IMAGE_FORMAT_ASTC_12x12:
            return 16;

        // BC formats
        case DONG_IMAGE_FORMAT_BC1:       return 8;   // 4x4 = 16 pixels, 8 bytes
        case DONG_IMAGE_FORMAT_BC2:       return 16;  // 4x4 = 16 pixels, 16 bytes
        case DONG_IMAGE_FORMAT_BC3:       return 16;
        case DONG_IMAGE_FORMAT_BC4:       return 8;
        case DONG_IMAGE_FORMAT_BC5:       return 16;
        case DONG_IMAGE_FORMAT_BC6H:      return 16;
        case DONG_IMAGE_FORMAT_BC6H_SF:   return 16;
        case DONG_IMAGE_FORMAT_BC7:       return 16;

        // ETC formats
        case DONG_IMAGE_FORMAT_ETC1:      return 8;
        case DONG_IMAGE_FORMAT_ETC2_RGB:  return 8;
        case DONG_IMAGE_FORMAT_ETC2_RGBA: return 16;
        case DONG_IMAGE_FORMAT_ETC2_RGBA1:return 8;

        default: return 0;
    }
}

// Calculate compressed image size
static inline size_t dong_image_format_calc_size(DongImageFormat format,
                                                 uint32_t width, uint32_t height) {
    uint32_t block_w, block_h;
    dong_image_format_block_size(format, &block_w, &block_h);

    uint32_t bytes_per_block = dong_image_format_bytes_per_block(format);

    if (block_w == 1 && block_h == 1) {
        // Uncompressed
        return (size_t)width * height * bytes_per_block;
    } else {
        // Compressed: round up to block boundaries
        uint32_t blocks_x = (width + block_w - 1) / block_w;
        uint32_t blocks_y = (height + block_h - 1) / block_h;
        return (size_t)blocks_x * blocks_y * bytes_per_block;
    }
}

// Get format name string
static inline const char* dong_image_format_name(DongImageFormat format) {
    switch (format) {
        case DONG_IMAGE_FORMAT_UNKNOWN:     return "UNKNOWN";
        case DONG_IMAGE_FORMAT_R8:          return "R8";
        case DONG_IMAGE_FORMAT_RG8:         return "RG8";
        case DONG_IMAGE_FORMAT_RGB8:        return "RGB8";
        case DONG_IMAGE_FORMAT_RGBA8:       return "RGBA8";
        case DONG_IMAGE_FORMAT_RGBA16F:     return "RGBA16F";
        case DONG_IMAGE_FORMAT_RGBA32F:     return "RGBA32F";
        case DONG_IMAGE_FORMAT_ASTC_4x4:    return "ASTC_4x4";
        case DONG_IMAGE_FORMAT_ASTC_5x4:    return "ASTC_5x4";
        case DONG_IMAGE_FORMAT_ASTC_5x5:    return "ASTC_5x5";
        case DONG_IMAGE_FORMAT_ASTC_6x5:    return "ASTC_6x5";
        case DONG_IMAGE_FORMAT_ASTC_6x6:    return "ASTC_6x6";
        case DONG_IMAGE_FORMAT_ASTC_8x5:    return "ASTC_8x5";
        case DONG_IMAGE_FORMAT_ASTC_8x6:    return "ASTC_8x6";
        case DONG_IMAGE_FORMAT_ASTC_8x8:    return "ASTC_8x8";
        case DONG_IMAGE_FORMAT_ASTC_10x5:   return "ASTC_10x5";
        case DONG_IMAGE_FORMAT_ASTC_10x6:   return "ASTC_10x6";
        case DONG_IMAGE_FORMAT_ASTC_10x8:   return "ASTC_10x8";
        case DONG_IMAGE_FORMAT_ASTC_10x10:  return "ASTC_10x10";
        case DONG_IMAGE_FORMAT_ASTC_12x10:  return "ASTC_12x10";
        case DONG_IMAGE_FORMAT_ASTC_12x12:  return "ASTC_12x12";
        case DONG_IMAGE_FORMAT_BC1:         return "BC1";
        case DONG_IMAGE_FORMAT_BC2:         return "BC2";
        case DONG_IMAGE_FORMAT_BC3:         return "BC3";
        case DONG_IMAGE_FORMAT_BC4:         return "BC4";
        case DONG_IMAGE_FORMAT_BC5:         return "BC5";
        case DONG_IMAGE_FORMAT_BC6H:        return "BC6H";
        case DONG_IMAGE_FORMAT_BC6H_SF:     return "BC6H_SF";
        case DONG_IMAGE_FORMAT_BC7:         return "BC7";
        case DONG_IMAGE_FORMAT_ETC1:        return "ETC1";
        case DONG_IMAGE_FORMAT_ETC2_RGB:    return "ETC2_RGB";
        case DONG_IMAGE_FORMAT_ETC2_RGBA:   return "ETC2_RGBA";
        case DONG_IMAGE_FORMAT_ETC2_RGBA1:  return "ETC2_RGBA1";
        default:                            return "INVALID";
    }
}

#ifdef __cplusplus
}
#endif

#endif // DONG_IMAGE_DECODER_H
