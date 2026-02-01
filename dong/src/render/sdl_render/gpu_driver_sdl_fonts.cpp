#include "../gpu_driver_sdl.hpp"



#include <SDL3/SDL_log.h>

#include <algorithm>
#include <cmath>
#include <limits>


namespace dong::render {

GPUDriverSDL::GlyphAtlasTier* GPUDriverSDL::selectGlyphAtlasTier(float font_size) {
    if (glyph_atlas_tiers_.empty()) {
        return nullptr;
    }

    // 新配置：128, 192, 256, 384
    // 更大的 MSDF 纹理 + 更大的 distance_range
    // 确保小字体的 screenPxRange >= 2
    //
    // 选择逻辑：
    //   - font_size <= 14px  -> 128px tier  (9:1 oversampling)
    //   - font_size <= 22px  -> 192px tier  (8:1 oversampling)
    //   - font_size <= 36px  -> 256px tier  (7:1 oversampling)
    //   - font_size > 36px   -> 384px tier  (10:1+ oversampling)

    const float clamped_font_size = std::max(font_size, 6.0f);

    uint32_t target_tier_px;
    if (clamped_font_size <= 14.0f) {
        target_tier_px = 128u;   // 9-14px：128px MSDF
    } else if (clamped_font_size <= 22.0f) {
        target_tier_px = 192u;   // 14-22px：192px MSDF
    } else if (clamped_font_size <= 36.0f) {
        target_tier_px = 256u;   // 22-36px：256px MSDF
    } else {
        target_tier_px = 384u;   // 36px+：384px MSDF
    }

    // 找到最接近的tier（优先不低于目标的分辨率）
    GlyphAtlasTier* best = nullptr;
    GlyphAtlasTier* lowest_above = nullptr;
    GlyphAtlasTier* highest_below = nullptr;

    for (auto& tier : glyph_atlas_tiers_) {
        const uint32_t tier_px = tier.bitmap_px;
        if (tier_px >= target_tier_px) {
            if (!lowest_above || tier_px < lowest_above->bitmap_px) {
                lowest_above = &tier;
            }
        } else {
            if (!highest_below || tier_px > highest_below->bitmap_px) {
                highest_below = &tier;
            }
        }
    }

    // 优先使用不低于目标的分辨率，如果没有则使用最高的低分辨率
    best = lowest_above ? lowest_above : highest_below;

    if (!best) {
        best = &glyph_atlas_tiers_.front();
    }

    // DEBUG: 每100帧输出一次tier选择统计
    //static int frame_count = 0;
    //static bool first_log = true;
    // if (first_log || frame_count % 100 == 0) {
    //     SDL_Log("[TIER SELECT] font_size=%.1fpx -> tier=%upx (target=%upx)",
    //             font_size, best->bitmap_px, target_tier_px);
    //     first_log = false;
    // }
    //++frame_count;

    return best;
}

FT_Face GPUDriverSDL::getOrCreateFace(const std::string& font_path, uint32_t pixel_size) {
    if (!ft_library_ || font_path.empty()) {
        return nullptr;
    }
    auto it = ft_face_cache_.find(font_path);
    if (it == ft_face_cache_.end()) {
        FT_Face face = nullptr;
        if (FT_New_Face(ft_library_, font_path.c_str(), 0, &face) != 0) {
            SDL_Log("GPUDriverSDL: failed to load font face '%s'", font_path.c_str());
            return nullptr;
        }
        ft_face_cache_[font_path] = face;
        it = ft_face_cache_.find(font_path);
    }
    FT_Face face = it->second;
    if (face) {
        FT_Set_Pixel_Sizes(face, 0, pixel_size);
    }
    return face;
}

} // namespace dong::render
