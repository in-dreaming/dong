#include "dong_format_policy.h"
#include "dong_image_decoder.h"
#include <stdlib.h>
#include <string.h>

static int policy_env_override(DongImageFormat* out) {
    const char* env = getenv("DONG_ATLAS_FORMAT");
    if (!env) {
        return 0;
    }
    if (strcmp(env, "BC7") == 0) {
        *out = DONG_IMAGE_FORMAT_BC7;
        return 1;
    }
    if (strcmp(env, "ASTC") == 0) {
        *out = DONG_IMAGE_FORMAT_ASTC_6x6;
        return 1;
    }
    return 0;
}

DongImageFormat dong_format_policy_choose(DongTextureUsage usage,
                                          const DongFormatPolicyCaps* caps) {
    DongImageFormat env_fmt = DONG_IMAGE_FORMAT_RGBA8;
    if (policy_env_override(&env_fmt)) {
        if (caps) {
            if (dong_image_format_is_bc(env_fmt) && !caps->supports_bcn) {
                return DONG_IMAGE_FORMAT_RGBA8;
            }
            if (dong_image_format_is_astc(env_fmt) && !caps->supports_astc) {
                return DONG_IMAGE_FORMAT_RGBA8;
            }
        }
        return env_fmt;
    }

    const int bcn = caps && caps->supports_bcn;
    const int astc = caps && caps->supports_astc;

    switch (usage) {
    case DONG_TEXTURE_USAGE_NORMAL_MAP:
        if (bcn) return DONG_IMAGE_FORMAT_BC5;
        if (astc) return DONG_IMAGE_FORMAT_ASTC_6x6;
        return DONG_IMAGE_FORMAT_RG8;
    case DONG_TEXTURE_USAGE_SINGLE_CHANNEL:
        if (bcn) return DONG_IMAGE_FORMAT_BC4;
        if (astc) return DONG_IMAGE_FORMAT_ASTC_8x8;
        return DONG_IMAGE_FORMAT_R8;
    case DONG_TEXTURE_USAGE_HDR:
        if (bcn) return DONG_IMAGE_FORMAT_BC6H;
        return DONG_IMAGE_FORMAT_RGBA16F;
    case DONG_TEXTURE_USAGE_UI_COLOR:
    default:
        if (bcn) return DONG_IMAGE_FORMAT_BC7;
        if (astc) return DONG_IMAGE_FORMAT_ASTC_6x6;
        return DONG_IMAGE_FORMAT_RGBA8;
    }
}

DongImageFormat dong_format_policy_atlas_default(const DongFormatPolicyCaps* caps) {
    return dong_format_policy_choose(DONG_TEXTURE_USAGE_UI_COLOR, caps);
}
