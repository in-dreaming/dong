#ifndef DONG_H
#define DONG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dong_context_t dong_context_t;
typedef struct dong_view_t dong_view_t;

// 1. Initialization
dong_context_t* dong_create_context(void);
void dong_destroy_context(dong_context_t* ctx);

// 2. View Management
dong_view_t* dong_view_create(dong_context_t* ctx, uint32_t width, uint32_t height);
void dong_view_destroy(dong_view_t* view);
void dong_view_free(dong_view_t* view);
void dong_view_load_html(dong_view_t* view, const char* html);
void dong_view_resize(dong_view_t* view, uint32_t width, uint32_t height);

// 3. Update/Render Pipeline
void dong_view_update(dong_view_t* view);

// 4. Render Output
void* dong_view_get_pixel_buffer(dong_view_t* view);
uint32_t dong_view_get_texture_id(dong_view_t* view);

// 5. Input Events (host -> engine -> DOM -> JS)
// Mouse coordinates are in view space pixels.
// Supported JS event types: "mousemove", "mousedown", "mouseup", "click", "keydown", "keyup".
void dong_view_send_mouse_move(dong_view_t* view, int32_t x, int32_t y);
void dong_view_send_mouse_down(dong_view_t* view, int32_t button);
void dong_view_send_mouse_up(dong_view_t* view, int32_t button);
void dong_view_send_key_down(dong_view_t* view, uint32_t key_code);
void dong_view_send_key_up(dong_view_t* view, uint32_t key_code);

// 6. Rendering mode control
// When use_gpu is true, the view will use the SDL_gpu-based GPU backend
// instead of the CPU Skia backend for rendering.
void dong_view_set_render_mode(dong_view_t* view, bool use_gpu);

// 7. JS Interaction
// Evaluate JavaScript code in the view's scripting context.
// Returns true on success, false if evaluation fails.
bool dong_view_eval(dong_view_t* view, const char* script);
// Execute script and return a stringified result. Currently returns an empty
// string but still evaluates the script for side effects.
const char* dong_view_eval_return(dong_view_t* view, const char* script);

#ifdef __cplusplus
}
#endif

#endif // DONG_H
