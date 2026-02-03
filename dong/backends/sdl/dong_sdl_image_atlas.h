/**
 * @file dong_sdl_image_atlas.h
 * @brief SDL GPU backend implementation for ImageAtlas
 */

#ifndef DONG_SDL_IMAGE_ATLAS_H
#define DONG_SDL_IMAGE_ATLAS_H

#include "dong_image_atlas.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an SDL-based image atlas.
 *
 * @param device SDL_GPUDevice* - must remain valid for atlas lifetime
 * @param config Atlas configuration
 * @return New atlas instance, or NULL on failure
 */
DongImageAtlas* dong_sdl_image_atlas_create(void* device, const DongAtlasConfig* config);

/**
 * Destroy an SDL-based image atlas.
 */
void dong_sdl_image_atlas_destroy(DongImageAtlas* atlas);

/**
 * Get the SDL GPU texture format for a DongImageFormat.
 * Returns 0 (SDL_GPU_TEXTUREFORMAT_INVALID) if the format is not supported.
 */
int dong_sdl_image_format_to_gpu_format(DongImageFormat format);

#ifdef __cplusplus
}
#endif

#endif // DONG_SDL_IMAGE_ATLAS_H
