#include "../../include/dong_decal.h"
#include "decal.hpp"

// Global decal manager singleton (future: per-engine)
static dong::world::DecalManager s_decal_mgr;

struct dong_decal {
    uint32_t id;
};

dong_decal_t* dong_decal_create(dong_engine_t* engine) {
    (void)engine;
    auto* d = new dong_decal();
    d->id = s_decal_mgr.addDecal();
    return d;
}

void dong_decal_destroy(dong_decal_t* decal) {
    if (decal) {
        s_decal_mgr.removeDecal(decal->id);
        delete decal;
    }
}

void dong_decal_set_position(dong_decal_t* decal, float x, float y, float z) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) { inst->world_x = x; inst->world_y = y; inst->world_z = z; }
}

void dong_decal_set_direction(dong_decal_t* decal, float dx, float dy, float dz) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) { inst->dir_x = dx; inst->dir_y = dy; inst->dir_z = dz; }
}

void dong_decal_set_size(dong_decal_t* decal, float width, float height) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) { inst->width = width; inst->height = height; }
}

void dong_decal_set_depth(dong_decal_t* decal, float depth) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) inst->depth = depth;
}

void dong_decal_set_color(dong_decal_t* decal, float r, float g, float b, float a) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) { inst->color_r = r; inst->color_g = g; inst->color_b = b; inst->color_a = a; }
}

void dong_decal_set_texture(dong_decal_t* decal, const char* texture_path) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) inst->texture_path = texture_path ? texture_path : "";
}

void dong_decal_set_normal_threshold(dong_decal_t* decal, float threshold) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) inst->normal_threshold = threshold;
}

void dong_decal_set_visible(dong_decal_t* decal, int visible) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) inst->visible = (visible != 0);
}

void dong_decal_set_rotation(dong_decal_t* decal, float radians) {
    if (!decal) return;
    auto* inst = s_decal_mgr.getDecal(decal->id);
    if (inst) inst->rotation = radians;
}

// Internal: emit decal draw commands to the DongDrawList
namespace dong::world {
    const std::vector<DecalInstance>& getActiveDecals() {
        return s_decal_mgr.getDecals();
    }
    bool hasDecals() { return s_decal_mgr.count() > 0; }
}
