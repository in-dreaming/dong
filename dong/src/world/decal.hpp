#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace dong::world {

struct DecalInstance {
    uint32_t id = 0;
    float world_x = 0.0f, world_y = 0.0f, world_z = 0.0f;
    float dir_x = 0.0f, dir_y = -1.0f, dir_z = 0.0f;  // projection direction (default: down)
    float width = 1.0f, height = 1.0f;
    float depth = 1.0f;
    float color_r = 1.0f, color_g = 1.0f, color_b = 1.0f, color_a = 1.0f;
    std::string texture_path;
    float normal_threshold = 0.5f;
    float rotation = 0.0f;
    bool visible = true;
};

// Manages world-space decal instances.
// Each frame, emits decal data through the DongDrawList C ABI for host consumption.
class DecalManager {
public:
    uint32_t addDecal();
    void removeDecal(uint32_t id);
    DecalInstance* getDecal(uint32_t id);

    const std::vector<DecalInstance>& getDecals() const { return decals_; }
    size_t count() const { return decals_.size(); }

private:
    std::vector<DecalInstance> decals_;
    uint32_t next_id_ = 1;
};

} // namespace dong::world
