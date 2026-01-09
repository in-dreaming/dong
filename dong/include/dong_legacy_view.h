#ifndef DONG_LEGACY_VIEW_H
#define DONG_LEGACY_VIEW_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dong_context_t dong_context_t;
typedef struct dong_view_t dong_view_t;

// NOTE: This is the legacy C API kept for transition.
// It is expected to be removed or moved behind build options after pluginization.

// 1. Initialization
// (Legacy) Context is currently not used by views.
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
// Supported JS event types: "mousemove", "mousedown", "mouseup", "click", "wheel",
//                           "keydown", "keyup", "input", "focus", "blur".
void dong_view_send_mouse_move(dong_view_t* view, int32_t x, int32_t y);
void dong_view_send_mouse_down(dong_view_t* view, int32_t button);
void dong_view_send_mouse_up(dong_view_t* view, int32_t button);
void dong_view_send_mouse_wheel(dong_view_t* view, float delta_x, float delta_y);
void dong_view_send_key_down(dong_view_t* view, uint32_t key_code);
void dong_view_send_key_up(dong_view_t* view, uint32_t key_code);
void dong_view_send_text_input(dong_view_t* view, const char* text);

// 6. Rendering mode control
// When use_gpu is true, the view will use the SDL_gpu-based GPU backend
// instead of the CPU backend for rendering.
void dong_view_set_render_mode(dong_view_t* view, bool use_gpu);

// Set an external GPU device and window for rendering.
// device: SDL_GPUDevice* pointer (cast from void*)
// window: SDL_Window* pointer (cast from void*)
void dong_view_set_external_gpu_device(dong_view_t* view, void* device, void* window);

// 7. JS Interaction
bool dong_view_eval(dong_view_t* view, const char* script);
const char* dong_view_eval_return(dong_view_t* view, const char* script);

// Offscreen rendering support (底层接口)
// Render to a GPU texture (SDL_GPUTexture*)
// Returns the GPU texture on success, NULL on failure
// Caller is responsible for releasing the texture with SDL_ReleaseGPUTexture()
void* dong_view_render_to_gpu_texture(dong_view_t* view, void* gpu_device,
                                      uint32_t width, uint32_t height);

// Offscreen rendering support (上层接口)
// Render to an offscreen texture and read back pixels
// Returns true on success, pixels will be written to out_pixels (RGBA format)
// out_pixels must be pre-allocated with size width * height * 4
bool dong_view_render_offscreen(dong_view_t* view, void* gpu_device,
                                uint32_t width, uint32_t height,
                                uint8_t* out_pixels);

#ifdef __cplusplus
}
#endif

#endif // DONG_LEGACY_VIEW_H
