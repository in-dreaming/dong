#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cmath>

namespace dong::render {
class DisplayListBuilder;
class TextShaper;
}

namespace dong::world {

struct WorldTextInstance {
    std::string text;
    float world_x = 0.0f, world_y = 0.0f, world_z = 0.0f;
    float color_r = 1.0f, color_g = 1.0f, color_b = 1.0f, color_a = 1.0f;
    float font_size = 16.0f;
    std::string font_family = "sans-serif";
    bool visible = true;
    uint32_t id = 0;
};

struct WorldTextConfig {
    float fade_start = 50.0f;
    float fade_end = 100.0f;
    float min_scale = 0.5f;
    float max_scale = 3.0f;
};

// Manages world-space billboard text instances.
// Each frame, projects them to screen space and emits glyph runs.
class WorldTextManager {
public:
    uint32_t addText();
    void removeText(uint32_t id);
    WorldTextInstance* getText(uint32_t id);

    void setViewProjection(const float* vp_4x4);
    void setConfig(const WorldTextConfig& cfg) { config_ = cfg; }
    void setViewport(float w, float h) { viewport_w_ = w; viewport_h_ = h; }

    // Project all visible texts and emit display items
    void buildDisplayItems(render::DisplayListBuilder& builder, render::TextShaper& shaper);

    size_t count() const { return texts_.size(); }

private:
    std::vector<WorldTextInstance> texts_;
    uint32_t next_id_ = 1;
    float vp_[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    WorldTextConfig config_;
    float viewport_w_ = 800.0f;
    float viewport_h_ = 600.0f;

    // Project world position to screen coordinates
    // Returns false if behind the camera
    bool projectToScreen(float wx, float wy, float wz,
                         float& sx, float& sy, float& depth) const;
};

} // namespace dong::world
