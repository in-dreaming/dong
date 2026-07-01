#ifndef DONG_FORMAT_POLICY_H
#define DONG_FORMAT_POLICY_H

#include "dong_image_decoder.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DongTextureUsage {
    DONG_TEXTURE_USAGE_UI_COLOR = 0,
    DONG_TEXTURE_USAGE_NORMAL_MAP,
    DONG_TEXTURE_USAGE_SINGLE_CHANNEL,
    DONG_TEXTURE_USAGE_HDR,
} DongTextureUsage;

typedef struct DongFormatPolicyCaps {
    int supports_bcn;
    int supports_astc;
} DongFormatPolicyCaps;

DongImageFormat dong_format_policy_choose(DongTextureUsage usage,
                                          const DongFormatPolicyCaps* caps);

DongImageFormat dong_format_policy_atlas_default(const DongFormatPolicyCaps* caps);

#ifdef __cplusplus
}
#endif

#endif /* DONG_FORMAT_POLICY_H */
