#include "decal.hpp"
#include <algorithm>

namespace dong::world {

uint32_t DecalManager::addDecal() {
    DecalInstance inst;
    inst.id = next_id_++;
    decals_.push_back(std::move(inst));
    return decals_.back().id;
}

void DecalManager::removeDecal(uint32_t id) {
    decals_.erase(std::remove_if(decals_.begin(), decals_.end(),
        [id](const DecalInstance& d) { return d.id == id; }),
        decals_.end());
}

DecalInstance* DecalManager::getDecal(uint32_t id) {
    for (auto& d : decals_) {
        if (d.id == id) return &d;
    }
    return nullptr;
}

} // namespace dong::world
