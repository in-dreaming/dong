// GPU backend image decoder.
// Provides image decoding via stb_image so the pure GPU backend (no SDL) can load
// <img>/background-image/poster assets the same way the SDL backend does.

#ifndef DONG_GPU_IMAGE_DECODER_H
#define DONG_GPU_IMAGE_DECODER_H

#include "dong_image_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_GPU_BUILDING_DLL
        #define DONG_GPU_IMAGE_DECODER_API __declspec(dllexport)
    #else
        #define DONG_GPU_IMAGE_DECODER_API __declspec(dllimport)
    #endif
#else
    #define DONG_GPU_IMAGE_DECODER_API __attribute__((visibility("default")))
#endif

// Create a GPU-backend image decoder.
// Supports: PNG, JPEG, BMP, GIF, TGA decoding (via stb_image).
// Supports: ASTC/BC encoding when a DongGtc context is registered as default.
DONG_GPU_IMAGE_DECODER_API DongImageDecoder* dong_gpu_image_decoder_create(void);

// Destroy the decoder.
DONG_GPU_IMAGE_DECODER_API void dong_gpu_image_decoder_destroy(DongImageDecoder* decoder);

#ifdef __cplusplus
}
#endif

#endif // DONG_GPU_IMAGE_DECODER_H
