#pragma once

#include "../../../third_party/gpu_texture_compress/sdk/include/gtc_formats.h"
#include "dong_image_decoder.h"

namespace dong::gtc_internal {

inline bool image_format_to_gtc(DongImageFormat fmt, ::gtc::GtcFormat* out) {
    if (!out) return false;
    switch (fmt) {
    case DONG_IMAGE_FORMAT_BC1: *out = ::gtc::GTC_FORMAT_BC1; return true;
    case DONG_IMAGE_FORMAT_BC3: *out = ::gtc::GTC_FORMAT_BC3; return true;
    case DONG_IMAGE_FORMAT_BC4: *out = ::gtc::GTC_FORMAT_BC4; return true;
    case DONG_IMAGE_FORMAT_BC5: *out = ::gtc::GTC_FORMAT_BC5; return true;
    case DONG_IMAGE_FORMAT_BC6H: *out = ::gtc::GTC_FORMAT_BC6H; return true;
    case DONG_IMAGE_FORMAT_BC7: *out = ::gtc::GTC_FORMAT_BC7; return true;
    case DONG_IMAGE_FORMAT_ASTC_4x4: *out = ::gtc::GTC_FORMAT_ASTC_4x4; return true;
    case DONG_IMAGE_FORMAT_ASTC_5x4: *out = ::gtc::GTC_FORMAT_ASTC_5x4; return true;
    case DONG_IMAGE_FORMAT_ASTC_5x5: *out = ::gtc::GTC_FORMAT_ASTC_5x5; return true;
    case DONG_IMAGE_FORMAT_ASTC_6x5: *out = ::gtc::GTC_FORMAT_ASTC_6x5; return true;
    case DONG_IMAGE_FORMAT_ASTC_6x6: *out = ::gtc::GTC_FORMAT_ASTC_6x6; return true;
    case DONG_IMAGE_FORMAT_ASTC_8x5: *out = ::gtc::GTC_FORMAT_ASTC_8x5; return true;
    case DONG_IMAGE_FORMAT_ASTC_8x6: *out = ::gtc::GTC_FORMAT_ASTC_8x6; return true;
    case DONG_IMAGE_FORMAT_ASTC_8x8: *out = ::gtc::GTC_FORMAT_ASTC_8x8; return true;
    case DONG_IMAGE_FORMAT_ASTC_10x5: *out = ::gtc::GTC_FORMAT_ASTC_10x5; return true;
    case DONG_IMAGE_FORMAT_ASTC_10x6: *out = ::gtc::GTC_FORMAT_ASTC_10x6; return true;
    case DONG_IMAGE_FORMAT_ASTC_10x8: *out = ::gtc::GTC_FORMAT_ASTC_10x8; return true;
    case DONG_IMAGE_FORMAT_ASTC_10x10: *out = ::gtc::GTC_FORMAT_ASTC_10x10; return true;
    case DONG_IMAGE_FORMAT_ASTC_12x10: *out = ::gtc::GTC_FORMAT_ASTC_12x10; return true;
    case DONG_IMAGE_FORMAT_ASTC_12x12: *out = ::gtc::GTC_FORMAT_ASTC_12x12; return true;
    default: return false;
    }
}

inline size_t compressed_size_for_format(uint32_t width, uint32_t height, DongImageFormat fmt) {
    ::gtc::GtcFormat gtc_fmt{};
    if (!image_format_to_gtc(fmt, &gtc_fmt)) {
        return 0;
    }
    const auto& info = ::gtc::get_format_info(gtc_fmt);
    const uint32_t blocks_x = (width + info.block_width - 1) / info.block_width;
    const uint32_t blocks_y = (height + info.block_height - 1) / info.block_height;
    return (size_t)blocks_x * blocks_y * info.block_bytes;
}

} // namespace dong::gtc_internal
