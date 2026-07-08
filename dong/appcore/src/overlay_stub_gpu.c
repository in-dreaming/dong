#if defined(DONG_BACKEND_GPU)

#include "dong_overlay.h"

DONG_APPCORE_API dong_overlay_t* dong_overlay_create(dong_app_t* app, uint32_t width, uint32_t height) {
    (void)app;
    (void)width;
    (void)height;
    return NULL;
}

DONG_APPCORE_API void dong_overlay_destroy(dong_overlay_t* overlay) { (void)overlay; }

DONG_APPCORE_API int dong_overlay_load_html(dong_overlay_t* overlay, const char* html) {
    (void)overlay;
    (void)html;
    return 0;
}

DONG_APPCORE_API int dong_overlay_load_file(dong_overlay_t* overlay, const char* file_path) {
    (void)overlay;
    (void)file_path;
    return 0;
}

DONG_APPCORE_API void dong_overlay_set_resource_root(dong_overlay_t* overlay, const char* root) {
    (void)overlay;
    (void)root;
}

DONG_APPCORE_API int dong_overlay_eval_script(dong_overlay_t* overlay, const char* script) {
    (void)overlay;
    (void)script;
    return 0;
}

DONG_APPCORE_API int dong_overlay_call_porffor_export(dong_overlay_t* overlay, const char* module_name,
                                                      const char* export_name) {
    (void)overlay;
    (void)module_name;
    (void)export_name;
    return 0;
}

DONG_APPCORE_API int dong_overlay_call_porffor_export1(dong_overlay_t* overlay, const char* module_name,
                                                       const char* export_name, double arg0) {
    (void)overlay;
    (void)module_name;
    (void)export_name;
    (void)arg0;
    return 0;
}

DONG_APPCORE_API void dong_overlay_update(dong_overlay_t* overlay, float dt) {
    (void)overlay;
    (void)dt;
}

DONG_APPCORE_API void dong_overlay_render(dong_overlay_t* overlay) { (void)overlay; }

DONG_APPCORE_API int dong_overlay_hit_test(dong_overlay_t* overlay, int32_t x, int32_t y) {
    (void)overlay;
    (void)x;
    (void)y;
    return 0;
}

DONG_APPCORE_API void dong_overlay_send_mouse_move(dong_overlay_t* overlay, int32_t x, int32_t y) {
    (void)overlay;
    (void)x;
    (void)y;
}

DONG_APPCORE_API void dong_overlay_send_mouse_button(dong_overlay_t* overlay, int32_t button, int pressed) {
    (void)overlay;
    (void)button;
    (void)pressed;
}

DONG_APPCORE_API void dong_overlay_send_mouse_wheel(dong_overlay_t* overlay, float delta_x, float delta_y) {
    (void)overlay;
    (void)delta_x;
    (void)delta_y;
}

DONG_APPCORE_API void dong_overlay_send_key(dong_overlay_t* overlay, uint32_t key_code, int pressed) {
    (void)overlay;
    (void)key_code;
    (void)pressed;
}

DONG_APPCORE_API void dong_overlay_send_text(dong_overlay_t* overlay, const char* text) {
    (void)overlay;
    (void)text;
}

DONG_APPCORE_API void dong_overlay_send_text_editing(dong_overlay_t* overlay, const char* text, int32_t cursor,
                                                     int32_t selection_length) {
    (void)overlay;
    (void)text;
    (void)cursor;
    (void)selection_length;
}

DONG_APPCORE_API void dong_overlay_set_position(dong_overlay_t* overlay, int32_t x, int32_t y) {
    (void)overlay;
    (void)x;
    (void)y;
}

DONG_APPCORE_API void dong_overlay_set_opacity(dong_overlay_t* overlay, float opacity) {
    (void)overlay;
    (void)opacity;
}

DONG_APPCORE_API void dong_overlay_set_enabled(dong_overlay_t* overlay, int enabled) {
    (void)overlay;
    (void)enabled;
}

DONG_APPCORE_API void* dong_overlay_get_texture(dong_overlay_t* overlay) {
    (void)overlay;
    return NULL;
}

#endif
