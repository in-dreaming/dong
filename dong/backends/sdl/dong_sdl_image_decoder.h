// SDL Image Decoder Implementation
// Provides image decoding using stb_image and GPU texture compression support.

#ifndef DONG_SDL_IMAGE_DECODER_H
#define DONG_SDL_IMAGE_DECODER_H

#include "dong_image_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SDL Image Decoder API
// =============================================================================

// Create an SDL-based image decoder.
// Supports: PNG, JPEG, BMP, GIF, TGA, PSD, HDR, PIC decoding
// Supports: ASTC 4x4, BC6H, BC7 encoding (when encoder libraries are available)
DongImageDecoder* dong_sdl_image_decoder_create(void);

// Destroy the SDL image decoder.
void dong_sdl_image_decoder_destroy(DongImageDecoder* decoder);

#ifdef __cplusplus
}
#endif

#endif // DONG_SDL_IMAGE_DECODER_H
