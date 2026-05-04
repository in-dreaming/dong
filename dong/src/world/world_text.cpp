#include "world_text.hpp"
#include "../render/display_list.hpp"
#include "../render/text_shaper.hpp"
#include <algorithm>
#include <cstring>

namespace dong::world {

uint32_t WorldTextManager::addText() {
    WorldTextInstance inst;
    inst.id = next_id_++;
    texts_.push_back(std::move(inst));
    return texts_.back().id;
}

void WorldTextManager::removeText(uint32_t id) {
    texts_.erase(std::remove_if(texts_.begin(), texts_.end(),
        [id](const WorldTextInstance& t) { return t.id == id; }),
        texts_.end());
}

WorldTextInstance* WorldTextManager::getText(uint32_t id) {
    for (auto& t : texts_) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

void WorldTextManager::setViewProjection(const float* vp_4x4) {
    if (vp_4x4) {
        std::memcpy(vp_, vp_4x4, sizeof(float) * 16);
    }
}

bool WorldTextManager::projectToScreen(float wx, float wy, float wz,
                                        float& sx, float& sy, float& depth) const {
    // Multiply world position by VP matrix (column-major)
    float clip_x = vp_[0]*wx + vp_[4]*wy + vp_[8]*wz  + vp_[12];
    float clip_y = vp_[1]*wx + vp_[5]*wy + vp_[9]*wz  + vp_[13];
    float clip_z = vp_[2]*wx + vp_[6]*wy + vp_[10]*wz + vp_[14];
    float clip_w = vp_[3]*wx + vp_[7]*wy + vp_[11]*wz + vp_[15];

    // Behind camera?
    if (clip_w <= 0.001f) return false;

    // NDC
    float ndc_x = clip_x / clip_w;
    float ndc_y = clip_y / clip_w;

    // Screen coordinates (0,0 = top-left)
    sx = (ndc_x * 0.5f + 0.5f) * viewport_w_;
    sy = (1.0f - (ndc_y * 0.5f + 0.5f)) * viewport_h_;
    depth = clip_z / clip_w;

    return true;
}

void WorldTextManager::buildDisplayItems(render::DisplayListBuilder& builder,
                                          render::TextShaper& shaper) {
    for (const auto& wt : texts_) {
        if (!wt.visible || wt.text.empty()) continue;

        float sx, sy, depth;
        if (!projectToScreen(wt.world_x, wt.world_y, wt.world_z, sx, sy, depth))
            continue;

        // Distance-based fade
        float dist = std::sqrt(wt.world_x * wt.world_x + wt.world_y * wt.world_y + wt.world_z * wt.world_z);
        float alpha = wt.color_a;
        if (dist > config_.fade_start && config_.fade_end > config_.fade_start) {
            float t = (dist - config_.fade_start) / (config_.fade_end - config_.fade_start);
            alpha *= std::max(0.0f, 1.0f - t);
        }
        if (alpha <= 0.001f) continue;

        // Shape the text
        render::TextShapeRequest req;
        req.text = wt.text;
        req.font_family = wt.font_family;
        req.font_weight = "400";
        req.font_style = "normal";
        req.font_size = wt.font_size;

        render::ShapedText shaped;
        if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) continue;

        float text_width = shaped.width_units * shaped.scale_to_pixels;

        // Center text at projected position
        float text_x = sx - text_width * 0.5f;
        float text_y = sy;

        // Build glyph run
        render::DrawGlyphRunData glyph_run;
        glyph_run.rect = {text_x, text_y - wt.font_size, text_width, wt.font_size * 1.2f};
        glyph_run.color = {wt.color_r * alpha, wt.color_g * alpha, wt.color_b * alpha, alpha};
        glyph_run.font_size = wt.font_size;
        glyph_run.font_family = wt.font_family;
        glyph_run.font_weight = "400";
        glyph_run.font_style = "normal";
        glyph_run.font_paths = shaped.font_paths;
        glyph_run.font_path = shaped.font_path;
        glyph_run.units_per_em = shaped.units_per_em;
        glyph_run.scale_to_pixels = shaped.scale_to_pixels;
        glyph_run.baseline_x = text_x;
        glyph_run.baseline_y = text_y;

        for (const auto& g : shaped.glyphs) {
            render::GlyphInstance gi;
            gi.glyph_id = g.glyph_id;
            gi.pen_x_units = g.pen_x_units;
            gi.pen_y_units = g.pen_y_units;
            gi.font_path_index = g.font_path_index;
            gi.units_per_em = g.units_per_em;
            glyph_run.glyphs.push_back(gi);
        }

        builder.addGlyphRun(std::move(glyph_run));
    }
}

} // namespace dong::world
