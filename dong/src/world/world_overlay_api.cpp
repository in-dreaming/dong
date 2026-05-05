#include "../../include/dong_world_overlay.h"
#include "world_overlay.hpp"

static dong::world::WorldOverlayManager s_overlay_mgr;

struct dong_world_overlay {
    uint32_t id;
};

dong_world_overlay_t* dong_world_overlay_create(dong_engine_t* engine, uint32_t render_width, uint32_t render_height) {
    (void)engine;
    auto* ov = new dong_world_overlay();
    ov->id = s_overlay_mgr.addOverlay(render_width, render_height);
    return ov;
}

void dong_world_overlay_destroy(dong_world_overlay_t* overlay) {
    if (overlay) {
        s_overlay_mgr.removeOverlay(overlay->id);
        delete overlay;
    }
}

void dong_world_overlay_load_html(dong_world_overlay_t* overlay, const char* html) {
    if (!overlay) return;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    if (inst && html) inst->html_content = html;
}

void dong_world_overlay_set_position(dong_world_overlay_t* overlay, float x, float y, float z) {
    if (!overlay) return;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    if (inst) { inst->world_x = x; inst->world_y = y; inst->world_z = z; }
}

void dong_world_overlay_set_rotation(dong_world_overlay_t* overlay, float pitch_deg, float yaw_deg, float roll_deg) {
    if (!overlay) return;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    if (inst) { inst->pitch_deg = pitch_deg; inst->yaw_deg = yaw_deg; inst->roll_deg = roll_deg; }
}

void dong_world_overlay_set_size(dong_world_overlay_t* overlay, float width, float height) {
    if (!overlay) return;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    if (inst) { inst->world_width = width; inst->world_height = height; }
}

void dong_world_overlay_set_visible(dong_world_overlay_t* overlay, int visible) {
    if (!overlay) return;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    if (inst) inst->visible = (visible != 0);
}

void dong_world_overlay_set_occlusion(dong_world_overlay_t* overlay, int enable) {
    if (!overlay) return;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    if (inst) inst->occlusion_enabled = (enable != 0);
}

void dong_world_overlay_set_interactive(dong_world_overlay_t* overlay, int enable) {
    if (!overlay) return;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    if (inst) inst->interactive = (enable != 0);
}

void dong_world_overlay_send_mouse(dong_world_overlay_t* overlay,
                                    int32_t local_x, int32_t local_y,
                                    int32_t button, int pressed) {
    if (!overlay) return;
    // TODO: Forward to internal engine view when per-overlay views are implemented
    (void)local_x; (void)local_y; (void)button; (void)pressed;
}

void dong_world_overlay_tick(dong_world_overlay_t* overlay) {
    if (!overlay) return;
    // TODO: Tick internal engine view when per-overlay views are implemented
}

uint32_t dong_world_overlay_get_width(dong_world_overlay_t* overlay) {
    if (!overlay) return 0;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    return inst ? inst->render_width : 0;
}

uint32_t dong_world_overlay_get_height(dong_world_overlay_t* overlay) {
    if (!overlay) return 0;
    auto* inst = s_overlay_mgr.getOverlay(overlay->id);
    return inst ? inst->render_height : 0;
}

namespace dong::world {
    const std::vector<WorldOverlayInstance>& getActiveOverlays() {
        return s_overlay_mgr.getOverlays();
    }
    bool hasOverlays() { return s_overlay_mgr.count() > 0; }
}
