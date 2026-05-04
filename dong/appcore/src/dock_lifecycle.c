// dock_lifecycle.c - Dock plugin/bootstrap/create/destroy

#include "dock_internal.h"
#include "dong.h"
#include "dong_plugin_api.h"
#include "dong_sdl_platform.h"
#include "dong_platform.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// Plugin loading (shared with app.c pattern)
// =============================================================================

static const dong_plugin_vtable_t* s_dock_plugin_vtable = NULL;
static void* s_dock_plugin_module = NULL;

static const dong_plugin_vtable_t* dock_try_load_plugin(void) {
    if (s_dock_plugin_vtable) return s_dock_plugin_vtable;
    if (s_dock_plugin_module) return NULL;

    const char* filename =
#if defined(_WIN32)
        "dong_plugin_sdl.dll";
#elif defined(__APPLE__)
        "libdong_plugin_sdl.dylib";
#else
        "libdong_plugin_sdl.so";
#endif

    const char* base_path = SDL_GetBasePath();
    if (!base_path) return NULL;

    char path[1024];
    SDL_snprintf(path, sizeof(path), "%s%s", base_path, filename);

    s_dock_plugin_module = SDL_LoadObject(path);
    if (!s_dock_plugin_module) {
        s_dock_plugin_module = (void*)1;
        return NULL;
    }

    typedef const dong_plugin_vtable_t* (*get_api_fn)(void);
    get_api_fn fn = (get_api_fn)SDL_LoadFunction(
        (SDL_SharedObject*)s_dock_plugin_module, "dong_plugin_get_api");
    if (!fn) {
        SDL_UnloadObject((SDL_SharedObject*)s_dock_plugin_module);
        s_dock_plugin_module = (void*)1;
        return NULL;
    }

    s_dock_plugin_vtable = fn();
    return s_dock_plugin_vtable;
}

// =============================================================================
// Helpers
// =============================================================================

static SDL_GPUTexture* dock_create_solid_texture(
    SDL_GPUDevice* device, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_GPUTextureCreateInfo ti = {0};
    ti.type = SDL_GPU_TEXTURETYPE_2D;
    ti.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    ti.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    ti.width = 1;
    ti.height = 1;
    ti.layer_count_or_depth = 1;
    ti.num_levels = 1;
    ti.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUTexture* tex = SDL_CreateGPUTexture(device, &ti);
    if (!tex) return NULL;

    // Upload the single pixel via transfer buffer
    SDL_GPUTransferBufferCreateInfo tbi = {0};
    tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbi.size = 4;
    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(device, &tbi);
    if (!tb) { SDL_ReleaseGPUTexture(device, tex); return NULL; }

    uint8_t* ptr = (uint8_t*)SDL_MapGPUTransferBuffer(device, tb, 0);
    if (!ptr) {
        SDL_ReleaseGPUTransferBuffer(device, tb);
        SDL_ReleaseGPUTexture(device, tex);
        return NULL;
    }
    ptr[0] = r; ptr[1] = g; ptr[2] = b; ptr[3] = a;
    SDL_UnmapGPUTransferBuffer(device, tb);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureTransferInfo src = {0};
    src.transfer_buffer = tb;
    src.offset = 0;
    src.pixels_per_row = 1;
    src.rows_per_layer = 1;

    SDL_GPUTextureRegion dst = {0};
    dst.texture = tex;
    dst.w = 1;
    dst.h = 1;
    dst.d = 1;

    SDL_UploadToGPUTexture(cp, &src, &dst, 0);
    SDL_EndGPUCopyPass(cp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(device, tb);

    return tex;
}

dong_engine_t* dock_create_engine(dong_dock_t* dock,
                                  uint32_t w, uint32_t h,
                                  dong_engine_t* shared_js,
                                  const char* view_name) {
    dong_engine_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.api_version = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.plugin = dock_try_load_plugin();
    desc.width = w;
    desc.height = h;

    dong_engine_t* engine = NULL;
    dong_result_t rc;

    if (shared_js) {
        rc = dong_engine_create_shared_js(&desc, shared_js, view_name, &engine);
    } else {
        rc = dong_engine_create(&desc, &engine);
    }

    if (rc != DONG_OK || !engine) {
        fprintf(stderr, "[DongDock] dong_engine_create failed\n");
        return NULL;
    }

    if (dong_engine_set_gpu(engine, dock->gpu_device, dock->primary_window) != DONG_OK) {
        fprintf(stderr, "[DongDock] dong_engine_set_gpu failed\n");
        dong_engine_destroy(engine);
        return NULL;
    }

    return engine;
}

// =============================================================================
// Dock Lifecycle
// =============================================================================

DONG_APPCORE_API dong_dock_t* dong_dock_create(const dong_dock_config_t* config) {
    if (!config) return NULL;

    dong_dock_t* dock = (dong_dock_t*)calloc(1, sizeof(dong_dock_t));
    if (!dock) return NULL;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "[DongDock] SDL_Init failed: %s\n", SDL_GetError());
        free(dock);
        return NULL;
    }

    uint32_t w = config->width  ? config->width  : 1280;
    uint32_t h = config->height ? config->height : 720;
    const char* title = config->title ? config->title : "Dong Dock";

    SDL_Window* win = SDL_CreateWindow(title, w, h,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
    if (!win) {
        fprintf(stderr, "[DongDock] SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        free(dock);
        return NULL;
    }

    SDL_GPUDevice* dev = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        false, NULL);
    if (!dev) {
        fprintf(stderr, "[DongDock] SDL_CreateGPUDevice failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        free(dock);
        return NULL;
    }

    if (!SDL_ClaimWindowForGPUDevice(dev, win)) {
        fprintf(stderr, "[DongDock] SDL_ClaimWindowForGPUDevice failed: %s\n", SDL_GetError());
        SDL_DestroyGPUDevice(dev);
        SDL_DestroyWindow(win);
        SDL_Quit();
        free(dock);
        return NULL;
    }

    // Platform init (once per process)
    if (!dong_sdl_platform_init(dev, win)) {
        fprintf(stderr, "[DongDock] dong_sdl_platform_init failed\n");
        SDL_DestroyGPUDevice(dev);
        SDL_DestroyWindow(win);
        SDL_Quit();
        free(dock);
        return NULL;
    }

    dock->gpu_device = dev;
    dock->driver = dong_platform_get_gpu_driver(dong_platform_get());
    dock->primary_window = win;
    dock->running = 1;
    dock->last_frame_time = SDL_GetPerformanceCounter();
    dock->delta_time = 0.016f;
    dock->focused_pane_index = -1;

    // Create chrome textures (1x1 solid colors)
    dock->tex_tab_bg         = dock_create_solid_texture(dev, 0x2b, 0x2b, 0x2b, 0xff);
    dock->tex_tab_active     = dock_create_solid_texture(dev, 0x3c, 0x3c, 0x3c, 0xff);
    dock->tex_drop_indicator = dock_create_solid_texture(dev, 0x22, 0x44, 0x88, 0xff);
    dock->tex_focus_border   = dock_create_solid_texture(dev, 0x00, 0x78, 0xd4, 0xff);
    dock->tex_btn_close      = dock_create_solid_texture(dev, 0xc4, 0x2b, 0x1c, 0xff);
    dock->tex_btn_maximize   = dock_create_solid_texture(dev, 0x55, 0x55, 0x55, 0xff);
    dock->tex_btn_minimize   = dock_create_solid_texture(dev, 0x55, 0x55, 0x55, 0xff);
    dock->tex_btn_hover      = dock_create_solid_texture(dev, 0x44, 0x44, 0x44, 0xff);
    dock->tex_divider        = dock_create_solid_texture(dev, 0x55, 0x55, 0x55, 0xff);
    dock->tex_divider_hover  = dock_create_solid_texture(dev, 0x00, 0x78, 0xd4, 0xff);
    dock->tex_close_hover_bg = dock_create_solid_texture(dev, 0xc4, 0x2b, 0x1c, 0xff);

    // Init hover state
    dock->hover.type = DOCK_HOVER_NONE;
    dock->hover.window_index = -1;
    dock->hover.node = NULL;
    dock->hover.tab_index = -1;

    // Init drag state
    dock->drag.state = DOCK_DRAG_NONE;
    dock->drag.source_pane_index = -1;
    dock->drag.source_tab_index = -1;
    dock->drag.hover_window_index = -1;
    dock->drag.reorder_insert_pos = -1;

    // Init divider drag state
    dock->divider.state = DOCK_DIV_IDLE;
    dock->divider.node = NULL;
    dock->divider.window_index = -1;
    dock->divider.last_click_time = 0;
    dock->divider.last_click_node = NULL;

    // Create cursors for divider resize
    dock->cursor_default   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    dock->cursor_ew_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
    dock->cursor_ns_resize = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);

    // Allocate primary window slot
    int wi = dock_alloc_window_slot(dock);
    if (wi < 0) {
        dong_sdl_platform_shutdown();
        SDL_DestroyGPUDevice(dev);
        SDL_DestroyWindow(win);
        SDL_Quit();
        free(dock);
        return NULL;
    }

    dock->windows[wi].alive = 1;
    dock->windows[wi].index = wi;
    dock->windows[wi].sdl_window = win;
    dock->windows[wi].root = NULL;
    dock->windows[wi].width = w;
    dock->windows[wi].height = h;
    dock->windows[wi].is_primary = 1;

    // Register hit-test for borderless window (enables OS drag + resize)
    SDL_SetWindowHitTest(win, dock_hit_test_callback, dock);

    printf("[DongDock] Created (%ux%u)\n", w, h);
    return dock;
}

DONG_APPCORE_API void dong_dock_destroy(dong_dock_t* dock) {
    if (!dock) return;

    // Destroy all panes
    for (int i = 0; i < dock->pane_count; i++) {
        dong_dock_pane_t* p = &dock->panes[i];
        if (!p->alive) continue;
        if (p->engine) dong_engine_destroy(p->engine);
        if (p->offscreen_tex)
            SDL_ReleaseGPUTexture(dock->gpu_device, p->offscreen_tex);
        if (p->title_tex)
            SDL_ReleaseGPUTexture(dock->gpu_device, p->title_tex);
        p->alive = 0;
    }
    free(dock->panes);

    // Destroy all windows (free split trees, destroy SDL windows)
    for (int i = 0; i < dock->window_count; i++) {
        dong_dock_window_t* wn = &dock->windows[i];
        if (!wn->alive) continue;
        dock_node_free(wn->root);
        if (!wn->is_primary && wn->sdl_window) {
            SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, wn->sdl_window);
            SDL_DestroyWindow(wn->sdl_window);
        }
        wn->alive = 0;
    }
    free(dock->windows);

    // Release cursors
    if (dock->cursor_default)   SDL_DestroyCursor(dock->cursor_default);
    if (dock->cursor_ew_resize) SDL_DestroyCursor(dock->cursor_ew_resize);
    if (dock->cursor_ns_resize) SDL_DestroyCursor(dock->cursor_ns_resize);

    // Release chrome textures
    if (dock->tex_tab_bg)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_tab_bg);
    if (dock->tex_tab_active)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_tab_active);
    if (dock->tex_drop_indicator)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_drop_indicator);
    if (dock->tex_focus_border)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_focus_border);
    if (dock->tex_btn_close)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_btn_close);
    if (dock->tex_btn_maximize)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_btn_maximize);
    if (dock->tex_btn_minimize)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_btn_minimize);
    if (dock->tex_btn_hover)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_btn_hover);
    if (dock->tex_divider)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_divider);
    if (dock->tex_divider_hover)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_divider_hover);
    if (dock->tex_close_hover_bg)
        SDL_ReleaseGPUTexture(dock->gpu_device, dock->tex_close_hover_bg);

    dong_sdl_platform_shutdown();

    if (dock->gpu_device) {
        SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, dock->primary_window);
        SDL_DestroyGPUDevice(dock->gpu_device);
    }
    if (dock->primary_window) SDL_DestroyWindow(dock->primary_window);

    SDL_Quit();
    free(dock);
}
