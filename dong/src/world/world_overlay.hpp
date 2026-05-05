#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace dong::world {

struct WorldOverlayInstance {
    uint32_t id = 0;
    float world_x = 0.0f, world_y = 0.0f, world_z = 0.0f;
    float pitch_deg = 0.0f, yaw_deg = 0.0f, roll_deg = 0.0f;
    float world_width = 1.0f, world_height = 1.0f;
    uint32_t render_width = 512;
    uint32_t render_height = 256;
    std::string html_content;
    bool visible = true;
    bool occlusion_enabled = true;
    bool interactive = false;
};

// Manages world-space overlay instances.
// Each overlay represents a 3D quad with HTML content rendered onto it.
class WorldOverlayManager {
public:
    uint32_t addOverlay(uint32_t render_w, uint32_t render_h);
    void removeOverlay(uint32_t id);
    WorldOverlayInstance* getOverlay(uint32_t id);

    const std::vector<WorldOverlayInstance>& getOverlays() const { return overlays_; }
    size_t count() const { return overlays_.size(); }

private:
    std::vector<WorldOverlayInstance> overlays_;
    uint32_t next_id_ = 1;
};

} // namespace dong::world
