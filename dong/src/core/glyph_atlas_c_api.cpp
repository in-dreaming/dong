// Glyph Atlas C API Implementation
// Provides default vtable registration mechanism

#include "dong_glyph_atlas.h"
#include "../core/log.h"

namespace {
    const dong_glyph_atlas_vtable_t* g_glyph_atlas_vtable = nullptr;
    const dong_font_face_vtable_t* g_font_face_vtable = nullptr;
} // namespace

extern "C" {

void dong_glyph_atlas_set_vtable(const dong_glyph_atlas_vtable_t* vtable) {
    g_glyph_atlas_vtable = vtable;
}

void dong_font_face_set_vtable(const dong_font_face_vtable_t* vtable) {
    g_font_face_vtable = vtable;
}

const dong_glyph_atlas_vtable_t* dong_glyph_atlas_get_vtable(void) {
    return g_glyph_atlas_vtable;
}

const dong_font_face_vtable_t* dong_font_face_get_vtable(void) {
    return g_font_face_vtable;
}

dong_glyph_atlas_t* dong_glyph_atlas_create(void* gpu_driver, uint32_t width, uint32_t height) {
    if (!g_glyph_atlas_vtable || !g_glyph_atlas_vtable->create) {
        DONG_LOG_ERROR("GlyphAtlas: no vtable registered");
        return nullptr;
    }
    return g_glyph_atlas_vtable->create(gpu_driver, width, height);
}

void dong_glyph_atlas_destroy(dong_glyph_atlas_t* atlas) {
    if (!g_glyph_atlas_vtable || !g_glyph_atlas_vtable->destroy) {
        return;
    }
    g_glyph_atlas_vtable->destroy(atlas);
}

dong_font_face_t* dong_font_face_load(const char* path) {
    if (!g_font_face_vtable || !g_font_face_vtable->load_from_file) {
        DONG_LOG_ERROR("FontFace: no vtable registered");
        return nullptr;
    }
    return g_font_face_vtable->load_from_file(path, 0);
}

void dong_font_face_free(dong_font_face_t* face) {
    if (!g_font_face_vtable || !g_font_face_vtable->destroy) {
        return;
    }
    g_font_face_vtable->destroy(face);
}

} // extern "C"
