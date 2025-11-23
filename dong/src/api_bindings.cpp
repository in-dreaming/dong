#include "dong.h"
#include "core/context.hpp"
#include "core/view.hpp"

using DongContext = dong::Context;
using DongView = dong::View;

extern "C" {

dong_context_t* dong_create_context(void) {
    return reinterpret_cast<dong_context_t*>(new DongContext());
}

void dong_destroy_context(dong_context_t* ctx) {
    delete reinterpret_cast<DongContext*>(ctx);
}

dong_view_t* dong_view_create(dong_context_t* ctx, uint32_t width, uint32_t height) {
    (void)ctx; // Unused for now
    return reinterpret_cast<dong_view_t*>(new DongView(width, height));
}

void dong_view_destroy(dong_view_t* view) {
    // Alias to dong_view_free for backward compatibility
    dong_view_free(view);
}

void dong_view_free(dong_view_t* view) {
    if (!view) return;
    delete reinterpret_cast<DongView*>(view);
}

void dong_view_load_html(dong_view_t* view, const char* html) {
    reinterpret_cast<DongView*>(view)->load_html(html);
}

void dong_view_resize(dong_view_t* view, uint32_t width, uint32_t height) {
    reinterpret_cast<DongView*>(view)->resize(width, height);
}

void dong_view_update(dong_view_t* view) {
    reinterpret_cast<DongView*>(view)->update();
}

void* dong_view_get_pixel_buffer(dong_view_t* view) {
    return reinterpret_cast<DongView*>(view)->get_pixel_buffer();
}

uint32_t dong_view_get_texture_id(dong_view_t* view) {
    (void)view;
    return 0;
}

void dong_view_send_mouse_move(dong_view_t* view, int32_t x, int32_t y) {
    if (!view) return;
    reinterpret_cast<DongView*>(view)->handle_mouse_move(x, y);
}

void dong_view_send_mouse_down(dong_view_t* view, int32_t button) {
    if (!view) return;
    reinterpret_cast<DongView*>(view)->handle_mouse_down(button);
}

void dong_view_send_mouse_up(dong_view_t* view, int32_t button) {
    if (!view) return;
    reinterpret_cast<DongView*>(view)->handle_mouse_up(button);
}

void dong_view_send_key_down(dong_view_t* view, uint32_t key_code) {
    if (!view) return;
    reinterpret_cast<DongView*>(view)->handle_key_down(key_code);
}

void dong_view_send_key_up(dong_view_t* view, uint32_t key_code) {
    if (!view) return;
    reinterpret_cast<DongView*>(view)->handle_key_up(key_code);
}

bool dong_view_eval(dong_view_t* view, const char* script) {
    if (!view || !script) return false;
    return reinterpret_cast<DongView*>(view)->eval_script(script);
}

const char* dong_view_eval_return(dong_view_t* view, const char* script) {
    if (!view || !script) return "";
    // TODO: Implement return value capture from JavaScript
    reinterpret_cast<DongView*>(view)->eval_script(script);
    return "";
}

} // extern "C"
