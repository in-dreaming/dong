// =============================================================================
// SDL GPU Driver - Font/Glyph Management (Phase 2B)
// =============================================================================
// Migrated from: src/render/sdl_render/gpu_driver_sdl_fonts.cpp
// =============================================================================

#include "sdl_gpu_driver.hpp"

#include "../../src/core/log.h"
#include "../../src/core/global_shared.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace dong {
namespace render {

GlyphAtlas* SDLGPUDriver::getGlyphAtlasForFontSize(float font_size) {
    // 如果使用 GlobalShared，从 GlobalShared 获取
    if (use_global_shared_glyph_atlas_) {
        auto* global_shared = GlobalShared::instance();
        if (global_shared) {
            auto* tier = global_shared->getGlyphAtlasTierForFontSize(font_size);
            if (tier && tier->atlas) {
                return tier->atlas.get();
            }
        }
        DONG_LOG_ERROR("SDLGPUDriver::getGlyphAtlasForFontSize: GlobalShared not available");
        return nullptr;
    }
    
    // 本地模式：从自己的 glyph_atlas_tiers_ 获取
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

    return best->atlas.get();
}

GlyphAtlas* SDLGPUDriver::getGlyphAtlasForBitmapPx(uint32_t bitmap_px) {
    // 如果使用 GlobalShared，从 GlobalShared 获取
    if (use_global_shared_glyph_atlas_) {
        auto* global_shared = GlobalShared::instance();
        if (global_shared) {
            auto* tier = global_shared->getGlyphAtlasTier(bitmap_px);
            if (tier && tier->atlas) {
                return tier->atlas.get();
            }
        }
        return nullptr;
    }
    
    // 本地模式：从自己的 glyph_atlas_tiers_ 查找
    for (auto& tier : glyph_atlas_tiers_) {
        if (tier.bitmap_px == bitmap_px) {
            return tier.atlas.get();
        }
    }
    return nullptr;
}

FT_Face SDLGPUDriver::getOrCreateFace(const std::string& font_path, uint32_t pixel_size) {
    if (!ft_library_ || font_path.empty()) {
        return nullptr;
    }
    auto it = ft_face_cache_.find(font_path);
    if (it == ft_face_cache_.end()) {
        FT_Face face = nullptr;
        if (FT_New_Face(ft_library_, font_path.c_str(), 0, &face) != 0) {
            DONG_LOG_ERROR("SDLGPUDriver: failed to load font face '%s'", font_path.c_str());
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

} // namespace render
} // namespace dong
