#include "../../include/dong_world_text.h"
#include "world_text.hpp"
#include "../core/engine_view.hpp"
#include "../render/text_shaper.hpp"

// The world text manager is stored per-engine as a simple static singleton for now.
// Future: move into EngineView::Impl.
static dong::world::WorldTextManager s_world_text_mgr;
static dong::render::TextShaper s_world_text_shaper;

struct dong_world_text {
    uint32_t id;
};

dong_world_text_t* dong_world_text_create(dong_engine_t* engine) {
    (void)engine;
    auto* wt = new dong_world_text();
    wt->id = s_world_text_mgr.addText();
    return wt;
}

void dong_world_text_destroy(dong_world_text_t* wt) {
    if (wt) {
        s_world_text_mgr.removeText(wt->id);
        delete wt;
    }
}

void dong_world_text_set_text(dong_world_text_t* wt, const char* text) {
    if (!wt) return;
    auto* inst = s_world_text_mgr.getText(wt->id);
    if (inst && text) inst->text = text;
}

void dong_world_text_set_position(dong_world_text_t* wt, float x, float y, float z) {
    if (!wt) return;
    auto* inst = s_world_text_mgr.getText(wt->id);
    if (inst) { inst->world_x = x; inst->world_y = y; inst->world_z = z; }
}

void dong_world_text_set_color(dong_world_text_t* wt, float r, float g, float b, float a) {
    if (!wt) return;
    auto* inst = s_world_text_mgr.getText(wt->id);
    if (inst) { inst->color_r = r; inst->color_g = g; inst->color_b = b; inst->color_a = a; }
}

void dong_world_text_set_font_size(dong_world_text_t* wt, float size) {
    if (!wt) return;
    auto* inst = s_world_text_mgr.getText(wt->id);
    if (inst) inst->font_size = size;
}

void dong_world_text_set_font_family(dong_world_text_t* wt, const char* family) {
    if (!wt) return;
    auto* inst = s_world_text_mgr.getText(wt->id);
    if (inst && family) inst->font_family = family;
}

void dong_world_text_set_visible(dong_world_text_t* wt, int visible) {
    if (!wt) return;
    auto* inst = s_world_text_mgr.getText(wt->id);
    if (inst) inst->visible = (visible != 0);
}

void dong_engine_set_world_text_vp(dong_engine_t* engine, const float* vp_matrix_4x4) {
    (void)engine;
    s_world_text_mgr.setViewProjection(vp_matrix_4x4);
}

void dong_engine_set_world_text_config(dong_engine_t* engine, const dong_world_text_config_t* config) {
    (void)engine;
    if (!config) return;
    dong::world::WorldTextConfig cfg;
    cfg.fade_start = config->fade_start_distance;
    cfg.fade_end = config->fade_end_distance;
    cfg.min_scale = config->min_scale;
    cfg.max_scale = config->max_scale;
    s_world_text_mgr.setConfig(cfg);
}

// Internal: called by engine tick to build world text display items
namespace dong::world {
    void buildWorldTextOverlay(render::DisplayListBuilder& builder, float vw, float vh) {
        s_world_text_mgr.setViewport(vw, vh);
        s_world_text_mgr.buildDisplayItems(builder, s_world_text_shaper);
    }
    bool hasWorldTexts() { return s_world_text_mgr.count() > 0; }
}
