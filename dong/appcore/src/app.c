// DongApp Implementation
// High-level SDL3 application framework + dong_engine_* integration.

#include "dong_app.h"

#include "dong.h"
#include "dong_plugin_api.h"
#include "dong_sdl_platform.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_PATH_LEN 1024

// Plugin loading (optional: video, etc.)
static const dong_plugin_vtable_t* s_plugin_vtable = NULL;

static void* s_plugin_module = NULL;

static const dong_plugin_vtable_t* try_load_plugin(void) {
    if (s_plugin_vtable) return s_plugin_vtable;
    if (s_plugin_module) return NULL;

    const char* filename =
#if defined(_WIN32)
        "dong_plugin_sdl.dll";
#elif defined(__APPLE__)
        "libdong_plugin_sdl.dylib";
#else
        "libdong_plugin_sdl.so";
#endif

    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        printf("[DongApp] SDL_GetBasePath failed: %s\n", SDL_GetError());
        return NULL;
    }


    char path[1024];
    SDL_snprintf(path, sizeof(path), "%s%s", base_path, filename);

    s_plugin_module = SDL_LoadObject(path);
    if (!s_plugin_module) {
        printf("[DongApp] Plugin not found: %s (%s)\n", path, SDL_GetError());
        s_plugin_module = (void*)1;
        return NULL;
    }


    typedef const dong_plugin_vtable_t* (*get_api_fn)(void);
    get_api_fn fn = (get_api_fn)SDL_LoadFunction((SDL_SharedObject*)s_plugin_module, "dong_plugin_get_api");
    if (!fn) {
        printf("[DongApp] Plugin missing symbol: %s\n", SDL_GetError());
        SDL_UnloadObject((SDL_SharedObject*)s_plugin_module);
        s_plugin_module = (void*)1;
        return NULL;
    }


    s_plugin_vtable = fn();
    if (s_plugin_vtable) {
        printf("[DongApp] Plugin loaded: %s\n", path);
    }
    return s_plugin_vtable;
}

static void extract_dir_from_path(const char* path, char* out_dir, size_t out_size) {
    if (!out_dir || out_size == 0) return;
    out_dir[0] = 0;
    if (!path) return;

    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* last_sep = last_slash;
    if (!last_sep || (last_backslash && last_backslash > last_sep)) {
        last_sep = last_backslash;
    }
    if (!last_sep) return;

    size_t len = (size_t)(last_sep - path);
    if (len >= out_size) len = out_size - 1;
    memcpy(out_dir, path, len);
    out_dir[len] = 0;
}

// =============================================================================
// Internal Structures
// =============================================================================


typedef struct dong_app_impl_t {
    SDL_Window* window;
    SDL_GPUDevice* gpu_device;

    uint32_t width;
    uint32_t height;
    int enable_dong;
    int running;
    int vsync;
    int enable_hdr;
    int hdr_enabled;            // Actual HDR status (may differ from enable_hdr if not supported)
    float hdr_max_luminance;    // Max luminance in nits

    uint64_t last_frame_time;
    float delta_time;

    int32_t mouse_x;
    int32_t mouse_y;

    dong_engine_t* engine;

    dong_app_event_callback_t event_callback;
    void* event_callback_user_data;
} dong_app_impl_t;

static void app_set_present_mode(dong_app_impl_t* app) {
    if (!app || !app->gpu_device || !app->window) {
        return;
    }

    SDL_GPUPresentMode mode = app->vsync ? SDL_GPU_PRESENTMODE_VSYNC : SDL_GPU_PRESENTMODE_IMMEDIATE;
    SDL_GPUSwapchainComposition composition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;

    // Try to enable HDR if requested
    app->hdr_enabled = 0;
    if (app->enable_hdr) {
        // Check for HDR Extended Linear support (R16G16B16A16_FLOAT)
        if (SDL_WindowSupportsGPUSwapchainComposition(app->gpu_device, app->window,
                                                       SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR)) {
            composition = SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR;
            app->hdr_enabled = 1;
            printf("[DongApp] HDR Extended Linear supported, enabling HDR mode\n");
        } else {
            printf("[DongApp] HDR Extended Linear not supported, falling back to SDR\n");
        }
    }

    if (!SDL_SetGPUSwapchainParameters(app->gpu_device, app->window, composition, mode)) {
        printf("[DongApp] Failed to set swapchain parameters: %s\n", SDL_GetError());
        // Fallback to SDR if HDR failed
        if (app->hdr_enabled) {
            app->hdr_enabled = 0;
            SDL_SetGPUSwapchainParameters(app->gpu_device, app->window,
                                          SDL_GPU_SWAPCHAINCOMPOSITION_SDR, mode);
            printf("[DongApp] Fell back to SDR mode\n");
        }
    }
}

static void app_send_event_callback(dong_app_impl_t* app, const dong_app_event_t* ev) {
    if (!app || !app->event_callback || !ev) {
        return;
    }
    app->event_callback(app->event_callback_user_data, ev);
}

static SDL_SystemCursor app_map_cursor(const char* cursor_name_cstr) {
    const char* n = (cursor_name_cstr && cursor_name_cstr[0]) ? cursor_name_cstr : "auto";

    if (strcmp(n, "text") == 0) return SDL_SYSTEM_CURSOR_TEXT;
    if (strcmp(n, "pointer") == 0) return SDL_SYSTEM_CURSOR_POINTER;
    if (strcmp(n, "move") == 0) return SDL_SYSTEM_CURSOR_MOVE;
    if (strcmp(n, "wait") == 0 || strcmp(n, "progress") == 0) return SDL_SYSTEM_CURSOR_WAIT;
    if (strcmp(n, "crosshair") == 0) return SDL_SYSTEM_CURSOR_CROSSHAIR;
    if (strcmp(n, "not-allowed") == 0) return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    if (strcmp(n, "n-resize") == 0 || strcmp(n, "s-resize") == 0 || strcmp(n, "ns-resize") == 0) {
        return SDL_SYSTEM_CURSOR_NS_RESIZE;
    }
    if (strcmp(n, "e-resize") == 0 || strcmp(n, "w-resize") == 0 || strcmp(n, "ew-resize") == 0) {
        return SDL_SYSTEM_CURSOR_EW_RESIZE;
    }
    if (strcmp(n, "ne-resize") == 0 || strcmp(n, "sw-resize") == 0 || strcmp(n, "nesw-resize") == 0) {
        return SDL_SYSTEM_CURSOR_NESW_RESIZE;
    }
    if (strcmp(n, "nw-resize") == 0 || strcmp(n, "se-resize") == 0 || strcmp(n, "nwse-resize") == 0) {
        return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    }
    if (strcmp(n, "grab") == 0 || strcmp(n, "grabbing") == 0) {
        return SDL_SYSTEM_CURSOR_MOVE;
    }

    return SDL_SYSTEM_CURSOR_DEFAULT;
}

static void app_apply_cursor(const char* cursor_name_cstr) {
    const char* n = (cursor_name_cstr && cursor_name_cstr[0]) ? cursor_name_cstr : "auto";

    static char s_last[64] = {0};
    if (strncmp(s_last, n, sizeof(s_last) - 1) == 0) {
        return;
    }
    strncpy(s_last, n, sizeof(s_last) - 1);
    s_last[sizeof(s_last) - 1] = 0;

    if (strcmp(n, "none") == 0) {
        SDL_HideCursor();
        return;
    }

    SDL_ShowCursor();

    SDL_SystemCursor sys = app_map_cursor(n);
    static SDL_Cursor* s_cache[SDL_SYSTEM_CURSOR_COUNT];
    if (!s_cache[sys]) {
        s_cache[sys] = SDL_CreateSystemCursor(sys);
    }
    if (s_cache[sys]) {
        SDL_SetCursor(s_cache[sys]);
    }
}

static void app_update_cursor(dong_app_impl_t* app) {
    if (!app || !app->engine) {
        return;
    }

    const char* css_cursor = dong_engine_get_cursor_at(app->engine, app->mouse_x, app->mouse_y);
    app_apply_cursor(css_cursor);
}


static void app_forward_event_to_engine(dong_app_impl_t* app, const dong_app_event_t* ev) {
    if (!app || !app->engine || !ev) {
        return;
    }

    switch (ev->type) {
        case DONG_APP_EVENT_WINDOW_RESIZED:
            (void)dong_engine_resize(app->engine, ev->window_resized.width, ev->window_resized.height);
            break;
        case DONG_APP_EVENT_MOUSE_MOVE:
            (void)dong_engine_send_mouse_move(app->engine, ev->mouse_move.x, ev->mouse_move.y);
            break;
        case DONG_APP_EVENT_MOUSE_BUTTON:
            // 重要：engine_view 的 mouse down/up 会基于最近一次 mouse_move 的坐标做 hit-test。
            // 如果用户“点击但鼠标没动”，仅发送 mouse_button 会导致 last_mouse_x/y 过期，出现点不开/点不准。
            (void)dong_engine_send_mouse_move(app->engine, ev->mouse_button.x, ev->mouse_button.y);
            (void)dong_engine_send_mouse_button(app->engine, ev->mouse_button.button, ev->mouse_button.pressed);
            break;
        case DONG_APP_EVENT_MOUSE_WHEEL:
            // 同上：滚轮滚动需要准确的鼠标位置来找到 scroll container。
            (void)dong_engine_send_mouse_move(app->engine, ev->mouse_wheel.x, ev->mouse_wheel.y);
            (void)dong_engine_send_mouse_wheel(app->engine, ev->mouse_wheel.delta_x, ev->mouse_wheel.delta_y);
            break;
        case DONG_APP_EVENT_KEY:
            (void)dong_engine_send_key(app->engine, ev->key.key_code, ev->key.pressed);
            break;
        case DONG_APP_EVENT_TEXT:
            (void)dong_engine_send_text(app->engine, ev->text.text);
            break;
        case DONG_APP_EVENT_TEXT_EDITING:
            (void)dong_engine_send_text_editing(app->engine, ev->text_editing.text,
                                                ev->text_editing.cursor,
                                                ev->text_editing.selection_length);
            break;
        default:
            break;
    }
}

static int debug_input_enabled(void) {
    const char* v = getenv("DONG_DEBUG_INPUT");
    return (v && v[0] && v[0] != '0');
}

static dong_app_event_t app_translate_sdl_event(dong_app_impl_t* app, const SDL_Event* e) {
    dong_app_event_t out;
    memset(&out, 0, sizeof(out));
    out.type = DONG_APP_EVENT_NONE;

    if (!app || !e) {
        return out;
    }

    switch (e->type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            out.type = DONG_APP_EVENT_QUIT;
            return out;

        case SDL_EVENT_WINDOW_RESIZED:
            out.type = DONG_APP_EVENT_WINDOW_RESIZED;
            out.window_resized.width = (uint32_t)e->window.data1;
            out.window_resized.height = (uint32_t)e->window.data2;
            return out;

        case SDL_EVENT_MOUSE_MOTION:
            out.type = DONG_APP_EVENT_MOUSE_MOVE;
            out.mouse_move.x = (int32_t)e->motion.x;
            out.mouse_move.y = (int32_t)e->motion.y;
            return out;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            out.type = DONG_APP_EVENT_MOUSE_BUTTON;
            out.mouse_button.button = (int32_t)e->button.button;
            out.mouse_button.pressed = (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
            out.mouse_button.x = (int32_t)e->button.x;
            out.mouse_button.y = (int32_t)e->button.y;
            return out;

        case SDL_EVENT_MOUSE_WHEEL:
            out.type = DONG_APP_EVENT_MOUSE_WHEEL;
            out.mouse_wheel.x = (int32_t)e->wheel.mouse_x;
            out.mouse_wheel.y = (int32_t)e->wheel.mouse_y;
            out.mouse_wheel.delta_x = e->wheel.x;
            out.mouse_wheel.delta_y = -e->wheel.y;
            return out;


        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            out.type = DONG_APP_EVENT_KEY;
            out.key.key_code = (uint32_t)e->key.key;
            out.key.scancode = (uint32_t)e->key.scancode;
            out.key.pressed = (e->type == SDL_EVENT_KEY_DOWN) ? 1 : 0;
            out.key.repeat = e->key.repeat ? 1 : 0;
            if (debug_input_enabled()) {
                const char* kname = SDL_GetKeyName((SDL_Keycode)out.key.key_code);
                const char* sname = SDL_GetScancodeName((SDL_Scancode)out.key.scancode);
                fprintf(stderr,
                        "[DongApp][Key] type=%s key=0x%x(%s) sc=0x%x(%s) down=%d rep=%d\n",
                        (e->type == SDL_EVENT_KEY_DOWN) ? "down" : "up",
                        (unsigned)out.key.key_code,
                        (kname && kname[0]) ? kname : "?",
                        (unsigned)out.key.scancode,
                        (sname && sname[0]) ? sname : "?",
                        out.key.pressed,
                        out.key.repeat);
            }
            return out;

        case SDL_EVENT_TEXT_INPUT:
            out.type = DONG_APP_EVENT_TEXT;
            out.text.text = e->text.text;
            if (debug_input_enabled()) {
                fprintf(stderr, "[DongApp][TextInput] %s\n", e->text.text ? e->text.text : "(null)");
            }
            return out;

        case SDL_EVENT_TEXT_EDITING:
            out.type = DONG_APP_EVENT_TEXT_EDITING;
            out.text_editing.text = e->edit.text;
            out.text_editing.cursor = e->edit.start;
            out.text_editing.selection_length = e->edit.length;
            if (debug_input_enabled()) {
                fprintf(stderr, "[DongApp][TextEditing] %s\n", e->edit.text ? e->edit.text : "(null)");
            }
            return out;

        default:
            return out;
    }
}

static int app_create_engine(dong_app_impl_t* app) {
    if (!app) {
        return 0;
    }

    if (!dong_sdl_platform_init(app->gpu_device, app->window)) {
        fprintf(stderr, "[DongApp] dong_sdl_platform_init failed\n");
        return 0;
    }

    dong_engine_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.api_version = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.plugin = try_load_plugin();
    desc.plugin_user = NULL;
    desc.html = NULL;
    desc.width = app->width;
    desc.height = app->height;

    if (dong_engine_create(&desc, &app->engine) != DONG_OK || !app->engine) {
        fprintf(stderr, "[DongApp] dong_engine_create failed\n");
        return 0;
    }

    if (dong_engine_set_gpu(app->engine, app->gpu_device, app->window) != DONG_OK) {
        fprintf(stderr, "[DongApp] dong_engine_set_gpu failed\n");
        return 0;
    }

    printf("[DongApp] dong_engine created (GPU enabled)\n");
    return 1;
}

// =============================================================================
// Public API
// =============================================================================

DONG_APPCORE_API dong_app_t* dong_app_create(const dong_app_config_t* config) {
    if (!config) return NULL;

    dong_app_impl_t* app = (dong_app_impl_t*)calloc(1, sizeof(dong_app_impl_t));
    if (!app) return NULL;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "[DongApp] SDL_Init failed: %s\n", SDL_GetError());
        free(app);
        return NULL;
    }

    app->width = config->width > 0 ? config->width : 800;
    app->height = config->height > 0 ? config->height : 600;
    app->enable_dong = config->enable_dong;
    app->vsync = config->vsync;
    app->enable_hdr = config->enable_hdr;
    app->hdr_enabled = 0;
    app->hdr_max_luminance = config->hdr_max_luminance > 0.0f ? config->hdr_max_luminance : 1000.0f;

    SDL_WindowFlags window_flags = 0;
    if (config->resizable) window_flags |= SDL_WINDOW_RESIZABLE;
    if (config->fullscreen) window_flags |= SDL_WINDOW_FULLSCREEN;

    const char* title = config->title ? config->title : "Dong Application";
    app->window = SDL_CreateWindow(title, app->width, app->height, window_flags);
    if (!app->window) {
        fprintf(stderr, "[DongApp] SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        free(app);
        return NULL;
    }

    app->gpu_device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        false,
        NULL);
    if (!app->gpu_device) {
        fprintf(stderr, "[DongApp] SDL_CreateGPUDevice failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        free(app);
        return NULL;
    }

    if (!SDL_ClaimWindowForGPUDevice(app->gpu_device, app->window)) {
        fprintf(stderr, "[DongApp] SDL_ClaimWindowForGPUDevice failed: %s\n", SDL_GetError());
        SDL_DestroyGPUDevice(app->gpu_device);
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        free(app);
        return NULL;
    }

    app_set_present_mode(app);

    if (app->enable_dong) {
        (void)app_create_engine(app);
    }

    app->last_frame_time = SDL_GetPerformanceCounter();
    app->delta_time = 0.016f;
    app->running = 1;

    return (dong_app_t*)app;
}

DONG_APPCORE_API void dong_app_destroy(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app) return;

    if (app->engine) {
        dong_engine_destroy(app->engine);
        app->engine = NULL;
    }

    dong_sdl_platform_shutdown();

    if (app->gpu_device) {
        SDL_DestroyGPUDevice(app->gpu_device);
        app->gpu_device = NULL;
    }

    if (app->window) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }

    SDL_Quit();
    free(app);
}

DONG_APPCORE_API int dong_app_is_running(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->running : 0;
}

DONG_APPCORE_API void dong_app_quit(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (app) app->running = 0;
}

DONG_APPCORE_API void dong_app_run(dong_app_t* app_handle, dong_app_tick_fn tick, void* user_data) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app) return;

    while (app->running) {
        if (!dong_app_poll_events(app_handle)) {
            break;
        }

        float dt = dong_app_get_delta_time(app_handle);
        if (tick) {
            tick(app_handle, dt, user_data);
        }

        dong_app_present(app_handle);
    }
}

DONG_APPCORE_API int dong_app_poll_events(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app) return 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        dong_app_event_t ev = app_translate_sdl_event(app, &e);

        if (ev.type == DONG_APP_EVENT_QUIT) {
            app->running = 0;
            return 0;
        }

        if (ev.type == DONG_APP_EVENT_WINDOW_RESIZED) {
            app->width = ev.window_resized.width;
            app->height = ev.window_resized.height;
        }

        if (ev.type == DONG_APP_EVENT_MOUSE_MOVE) {
            app->mouse_x = ev.mouse_move.x;
            app->mouse_y = ev.mouse_move.y;
        } else if (ev.type == DONG_APP_EVENT_MOUSE_BUTTON) {
            app->mouse_x = ev.mouse_button.x;
            app->mouse_y = ev.mouse_button.y;
        } else if (ev.type == DONG_APP_EVENT_MOUSE_WHEEL) {
            app->mouse_x = ev.mouse_wheel.x;
            app->mouse_y = ev.mouse_wheel.y;
        }

        if (ev.type != DONG_APP_EVENT_NONE) {
            app_forward_event_to_engine(app, &ev);
            app_send_event_callback(app, &ev);
        }
    }

    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();
    app->delta_time = (float)(now - app->last_frame_time) / (float)freq;
    app->last_frame_time = now;

    app_update_cursor(app);

    return app->running;
}


DONG_APPCORE_API float dong_app_get_delta_time(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->delta_time : 0.016f;
}

DONG_APPCORE_API int dong_app_begin_frame(dong_app_t* app_handle) {
    (void)app_handle;
    return 1;
}

DONG_APPCORE_API void dong_app_present(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !app->gpu_device || !app->window) return;

    if (app->engine) {
        // Tick engine (handles rendering to swapchain internally via dong_engine_set_gpu)
        (void)dong_engine_tick(app->engine);
        // Engine rendering is done, submit any remaining GPU work
        // Note: The engine uses dong_gpu_execute which handles submission
        return;
    }

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(app->gpu_device);
    if (!cmd) return;

    SDL_GPUTexture* swapchain_texture = NULL;
    if (!SDL_AcquireGPUSwapchainTexture(cmd, app->window, &swapchain_texture, NULL, NULL)) {
        SDL_CancelGPUCommandBuffer(cmd);
        return;
    }

    if (swapchain_texture) {
        SDL_GPUColorTargetInfo color_info;
        SDL_zero(color_info);
        color_info.texture = swapchain_texture;
        color_info.load_op = SDL_GPU_LOADOP_CLEAR;
        color_info.store_op = SDL_GPU_STOREOP_STORE;
        color_info.clear_color.r = 0.1f;
        color_info.clear_color.g = 0.1f;
        color_info.clear_color.b = 0.1f;
        color_info.clear_color.a = 1.0f;

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, NULL);
        if (pass) {
            SDL_EndGPURenderPass(pass);
        }
    }

    SDL_SubmitGPUCommandBuffer(cmd);
}

DONG_APPCORE_API void dong_app_get_size(dong_app_t* app_handle, uint32_t* out_width, uint32_t* out_height) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (out_width) *out_width = app ? app->width : 0;
    if (out_height) *out_height = app ? app->height : 0;
}

DONG_APPCORE_API void dong_app_get_mouse_position(dong_app_t* app_handle, int32_t* out_x, int32_t* out_y) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (out_x) *out_x = app ? app->mouse_x : 0;
    if (out_y) *out_y = app ? app->mouse_y : 0;
}

DONG_APPCORE_API dong_renderer_t* dong_app_get_renderer(dong_app_t* app_handle) {
    (void)app_handle;
    return NULL;
}

DONG_APPCORE_API void* dong_app_get_gpu_device(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->gpu_device : NULL;
}

DONG_APPCORE_API void* dong_app_get_window(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->window : NULL;
}

DONG_APPCORE_API void* dong_app_get_dong_engine(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->engine : NULL;
}

DONG_APPCORE_API int dong_app_load_html(dong_app_t* app_handle, const char* html) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !app->engine || !html) return 0;
    return (dong_engine_load_html(app->engine, html) == DONG_OK) ? 1 : 0;
}

DONG_APPCORE_API int dong_app_load_html_file(dong_app_t* app_handle, const char* path) {
    if (!app_handle || !path) return 0;

    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !app->engine) return 0;

    char dir[MAX_PATH_LEN];
    extract_dir_from_path(path, dir, sizeof(dir));
    if (dir[0]) {
        (void)dong_engine_set_resource_root(app->engine, dir);
    }

    FILE* f = fopen(path, "rb");

    if (!f) {
        fprintf(stderr, "[DongApp] Failed to open file: %s\n", path);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(f);
        return 0;
    }

    size_t read = fread(content, 1, size, f);
    content[read] = '\0';
    fclose(f);

    int result = dong_app_load_html(app_handle, content);
    free(content);
    return result;
}

DONG_APPCORE_API void dong_app_enable_text_input(dong_app_t* app_handle, int enable) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !app->window) return;

    if (enable) {
        SDL_StartTextInput(app->window);
    } else {
        SDL_StopTextInput(app->window);
    }
}

DONG_APPCORE_API void dong_app_set_event_callback(dong_app_t* app_handle, dong_app_event_callback_t callback, void* user_data) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app) return;
    app->event_callback = callback;
    app->event_callback_user_data = user_data;
}

DONG_APPCORE_API int dong_app_is_hdr_enabled(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->hdr_enabled : 0;
}

DONG_APPCORE_API float dong_app_get_hdr_max_luminance(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !app->hdr_enabled) return 0.0f;
    return app->hdr_max_luminance;
}
