#include "world_overlay.hpp"
#include <algorithm>

namespace dong::world {

uint32_t WorldOverlayManager::addOverlay(uint32_t render_w, uint32_t render_h) {
    WorldOverlayInstance inst;
    inst.id = next_id_++;
    inst.render_width = render_w;
    inst.render_height = render_h;
    overlays_.push_back(std::move(inst));
    return overlays_.back().id;
}

void WorldOverlayManager::removeOverlay(uint32_t id) {
    overlays_.erase(std::remove_if(overlays_.begin(), overlays_.end(),
        [id](const WorldOverlayInstance& o) { return o.id == id; }),
        overlays_.end());
}

WorldOverlayInstance* WorldOverlayManager::getOverlay(uint32_t id) {
    for (auto& o : overlays_) {
        if (o.id == id) return &o;
    }
    return nullptr;
}

} // namespace dong::world
