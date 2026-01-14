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

    // 将字号映射到"期望的 MSDF 像素分辨率"，再在现有档位中选择最近的一档。
    // 使用较高的 MSDF 分辨率以保证小字号文本的清晰度：
    //   target_msdf_px ≈ font_px_size * 2.5
    // 这样对于 13px 字号会选择 32px tier，而不是更低的分辨率
    const float clamped_font_size = std::max(font_size, 1.0f);
    const float target_msdf_px_f = std::ceil(clamped_font_size * 2.5f);
    const uint32_t target_msdf_px = target_msdf_px_f > 0.0f
        ? static_cast<uint32_t>(target_msdf_px_f)
        : 32u;

    GlyphAtlasTier* best = nullptr;
    uint32_t best_error = std::numeric_limits<uint32_t>::max();

    for (auto& tier : glyph_atlas_tiers_) {
        const uint32_t tier_px = tier.bitmap_px;
        const uint32_t error = (tier_px > target_msdf_px)
            ? (tier_px - target_msdf_px)
            : (target_msdf_px - tier_px);
        if (error < best_error) {
            best_error = error;
            best = &tier;
        } else if (error == best_error && best && tier_px > best->bitmap_px) {
            // 误差相同时，偏向更高分辨率的一档，保证质量优先
            best = &tier;
        }
    }

    if (!best) {
        best = &glyph_atlas_tiers_.front();
    }

    // DEBUG: 输出选择的 tier
    static bool first_log = true;
    if (first_log) {
        SDL_Log("[TIER SELECT] font_size=%.1f target_msdf=%u -> selected tier=%upx atlas=%p",
                font_size, target_msdf_px, best->bitmap_px, (void*)best->atlas.get());
        first_log = false;
    }

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
