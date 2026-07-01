#ifndef DONG_GPU_FORMAT_H
#define DONG_GPU_FORMAT_H

#include "dong_gpu_driver.h"
#include "dong_image_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

DongGPUTextureFormat dong_image_format_to_gpu_texture_format(DongImageFormat format);
DongImageFormat dong_gpu_texture_format_to_image_format(DongGPUTextureFormat format);

int dong_image_format_is_gpu_compressed(DongImageFormat format);

#ifdef __cplusplus
}
#endif

#endif /* DONG_GPU_FORMAT_H */
