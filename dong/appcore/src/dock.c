// dock.c - Multi-Window Docking System implementation
//
// Manages multiple OS windows, each with a binary split tree layout.
// Each pane renders to an offscreen texture, then composited via blit.

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
// Hit-test callback for borderless windows
// =============================================================================

// Forward declaration: find a leaf at the top row (y==0) whose title bar area
// contains the given point. Returns the leaf if found and the point is in the
// title bar (not over a tab or button), NULL otherwise.
static dock_node_t* dock_find_title_bar_draggable(
    dock_node_t* node, int32_t mx, int32_t my, const dong_dock_t* dock);

// Forward declaration for dock_tab_width (defined later)
static uint32_t dock_tab_width(const dock_node_t* node);

static SDL_HitTestResult SDLCALL dock_hit_test_callback(
    SDL_Window* win, const SDL_Point* area, void* data)
{
    dong_dock_t* dock = (dong_dock_t*)data;
    if (!dock) return SDL_HITTEST_NORMAL;

    // Find which dock window this is
    dong_dock_window_t* dw = NULL;
    for (int i = 0; i < dock->window_count; i++) {
        if (dock->windows[i].alive && dock->windows[i].sdl_window == win) {
            dw = &dock->windows[i];
            break;
        }
    }
    if (!dw) return SDL_HITTEST_NORMAL;

    int32_t mx = area->x;
    int32_t my = area->y;
    int32_t w = (int32_t)dw->width;
    int32_t h = (int32_t)dw->height;
    int32_t border = DOCK_RESIZE_BORDER;

    // Edge resize zones
    int at_left   = mx < border;
    int at_right  = mx >= w - border;
    int at_top    = my < border;
    int at_bottom = my >= h - border;

    if (at_top && at_left)     return SDL_HITTEST_RESIZE_TOPLEFT;
    if (at_top && at_right)    return SDL_HITTEST_RESIZE_TOPRIGHT;
    if (at_bottom && at_left)  return SDL_HITTEST_RESIZE_BOTTOMLEFT;
    if (at_bottom && at_right) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
    if (at_left)               return SDL_HITTEST_RESIZE_LEFT;
    if (at_right)              return SDL_HITTEST_RESIZE_RIGHT;
    if (at_top)                return SDL_HITTEST_RESIZE_TOP;
    if (at_bottom)             return SDL_HITTEST_RESIZE_BOTTOM;

    // Ensure layout is up-to-date for hit-testing
    if (dw->root)
        dock_node_layout(dw->root, 0, 0, dw->width, dw->height);

    // Check if over a split divider → NORMAL (let SDL deliver mouse events)
    if (dw->root) {
        dock_node_t* div_hit = dock_node_hit_test_divider(dw->root, mx, my);
        if (div_hit) {
            SDL_SetCursor(div_hit->type == DOCK_NODE_SPLIT_H
                ? dock->cursor_ew_resize
                : dock->cursor_ns_resize);
            return SDL_HITTEST_NORMAL;
        }
    }

    // Not over divider → restore default cursor
    SDL_SetCursor(dock->cursor_default);

    // Check if in title bar area (not over tabs/buttons) → draggable
    if (dw->root) {
        dock_node_t* draggable = dock_find_title_bar_draggable(dw->root, mx, my, dock);
        if (draggable) return SDL_HITTEST_DRAGGABLE;
    }

    return SDL_HITTEST_NORMAL;
}

// Helper: check if point is in a leaf's title bar and NOT over a tab or button
static dock_node_t* dock_find_title_bar_draggable(
    dock_node_t* node, int32_t mx, int32_t my, const dong_dock_t* dock)
{
    if (!node) return NULL;

    if (node->type == DOCK_NODE_LEAF) {
        // Check if point is within this leaf's title bar
        if (mx < node->x || mx >= node->x + (int32_t)node->w) return NULL;
        if (my < node->y || my >= node->y + DOCK_TITLE_BAR_HEIGHT) return NULL;

        // For non-primary windows with a single pane, the entire title bar
        // should initiate dock-drag (not OS window drag) so that the detached
        // pane can be docked back into other windows.
        {
            dong_dock_window_t* dw = NULL;
            for (int i = 0; i < dock->window_count; i++) {
                if (!dock->windows[i].alive) continue;
                if (dock->windows[i].root) {
                    dock_node_t* found = dock_node_hit_test(dock->windows[i].root, mx, my);
                    if (found == node) { dw = &dock->windows[i]; break; }
                }
            }
            if (dw && !dw->is_primary && dw->root &&
                dw->root->type == DOCK_NODE_LEAF && dw->root->pane_count == 1) {
                // Check if over window-level buttons - those still need to work
                int32_t bx = (int32_t)dw->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;
                int32_t by = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
                for (int b = 0; b < 3; b++) {
                    if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                        my >= by && my < by + DOCK_BTN_SIZE) {
                        return NULL; // over window button
                    }
                    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);
                }
                // Return NULL so SDL returns HITTEST_NORMAL, letting our
                // dock drag system handle the click instead of OS window drag
                return NULL;
            }
        }

        // Check if over a tab → NORMAL (let SDL handle tab click/drag)
        int32_t local_x = mx - node->x - DOCK_TAB_PAD;
        uint32_t tab_w = dock_tab_width(node);
        if (tab_w > 0 && local_x >= 0) {
            int tab_idx = local_x / (int32_t)(tab_w + DOCK_TAB_PAD);
            if (tab_idx < node->pane_count) return NULL; // over a tab
        }

        // Check if over per-leaf close button
        int32_t btn_x = node->x + (int32_t)node->w - DOCK_BTN_PAD - DOCK_BTN_SIZE;
        int32_t btn_y = node->y + (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
        if (mx >= btn_x && mx < btn_x + DOCK_BTN_SIZE &&
            my >= btn_y && my < btn_y + DOCK_BTN_SIZE) {
            return NULL; // over leaf close button
        }

        // Check if over window-level buttons (top-right corner of window)
        // Window buttons are at the very top-right, only if this leaf starts at y=0
        if (node->y == 0) {
            dong_dock_window_t* dw = NULL;
            for (int i = 0; i < dock->window_count; i++) {
                if (!dock->windows[i].alive) continue;
                // This leaf belongs to this window if it fits within window bounds
                if (dock->windows[i].root) {
                    dock_node_t* found = dock_node_hit_test(dock->windows[i].root, mx, my);
                    if (found == node) { dw = &dock->windows[i]; break; }
                }
            }
            if (dw) {
                // Window buttons: close, maximize, minimize at top-right
                int32_t bx = (int32_t)dw->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;
                int32_t by = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
                for (int b = 0; b < 3; b++) {
                    if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                        my >= by && my < by + DOCK_BTN_SIZE) {
                        return NULL; // over window button
                    }
                    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);
                }
            }
        }

        return node; // in title bar, not over button/tab → draggable
    }

    // Split node: recurse
    dock_node_t* r = dock_find_title_bar_draggable(node->children[0], mx, my, dock);
    if (r) return r;
    return dock_find_title_bar_draggable(node->children[1], mx, my, dock);
}

// =============================================================================
// Helpers
// =============================================================================

// Create a 1x1 RGBA texture filled with a solid color.
// Used for title bar / tab / indicator rendering via SDL_BlitGPUTexture stretch.
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

// Blit a 1x1 solid texture stretched to a rectangle on the swapchain.
static void dock_blit_solid(SDL_GPUCommandBuffer* cmd,
                             SDL_GPUTexture* src_tex,
                             SDL_GPUTexture* dst_tex,
                             int32_t x, int32_t y,
                             uint32_t w, uint32_t h)
{
    if (w == 0 || h == 0) return;
    SDL_GPUBlitInfo blit;
    SDL_zero(blit);
    blit.source.texture = src_tex;
    blit.source.w       = 1;
    blit.source.h       = 1;
    blit.destination.texture = dst_tex;
    blit.destination.x  = (uint32_t)x;
    blit.destination.y  = (uint32_t)y;
    blit.destination.w  = w;
    blit.destination.h  = h;
    blit.load_op        = SDL_GPU_LOADOP_LOAD;
    blit.filter         = SDL_GPU_FILTER_NEAREST;
    SDL_BlitGPUTexture(cmd, &blit);
}

static void dock_extract_dir(const char* path, char* out, size_t out_sz) {
    if (!out || out_sz == 0) return;
    out[0] = 0;
    if (!path) return;
    const char* ls = strrchr(path, '/');
    const char* lb = strrchr(path, '\\');
    const char* sep = ls;
    if (!sep || (lb && lb > sep)) sep = lb;
    if (!sep) return;
    size_t len = (size_t)(sep - path);
    if (len >= out_sz) len = out_sz - 1;
    memcpy(out, path, len);
    out[len] = 0;
}

static SDL_GPUTexture* dock_ensure_offscreen_texture(
    SDL_GPUDevice* device, SDL_GPUTexture* cached,
    uint32_t* cached_w, uint32_t* cached_h,
    uint32_t width, uint32_t height)
{
    if (!device) return NULL;
    if (cached && *cached_w == width && *cached_h == height) return cached;
    if (cached) SDL_ReleaseGPUTexture(device, cached);

    SDL_GPUTextureCreateInfo ti = {0};
    ti.type = SDL_GPU_TEXTURETYPE_2D;
    ti.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    ti.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    ti.width = width;
    ti.height = height;
    ti.layer_count_or_depth = 1;
    ti.num_levels = 1;
    ti.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUTexture* tex = SDL_CreateGPUTexture(device, &ti);
    if (tex) {
        *cached_w = width;
        *cached_h = height;
    }
    return tex;
}

static int dock_alloc_pane_slot(dong_dock_t* dock) {
    // Try to find a dead slot
    for (int i = 0; i < dock->pane_count; i++) {
        if (!dock->panes[i].alive) return i;
    }
    // Grow array
    if (dock->pane_count >= dock->pane_cap) {
        int new_cap = dock->pane_cap ? dock->pane_cap * 2 : 4;
        dong_dock_pane_t* arr = (dong_dock_pane_t*)realloc(
            dock->panes, sizeof(dong_dock_pane_t) * new_cap);
        if (!arr) return -1;
        dock->panes = arr;
        dock->pane_cap = new_cap;
    }
    int idx = dock->pane_count++;
    memset(&dock->panes[idx], 0, sizeof(dong_dock_pane_t));
    return idx;
}

static int dock_alloc_window_slot(dong_dock_t* dock) {
    for (int i = 0; i < dock->window_count; i++) {
        if (!dock->windows[i].alive) return i;
    }
    if (dock->window_count >= dock->window_cap) {
        int new_cap = dock->window_cap ? dock->window_cap * 2 : 4;
        dong_dock_window_t* arr = (dong_dock_window_t*)realloc(
            dock->windows, sizeof(dong_dock_window_t) * new_cap);
        if (!arr) return -1;
        dock->windows = arr;
        dock->window_cap = new_cap;
    }
    int idx = dock->window_count++;
    memset(&dock->windows[idx], 0, sizeof(dong_dock_window_t));
    return idx;
}

static dong_dock_window_t* dock_find_window_by_sdl_id(dong_dock_t* dock, SDL_WindowID wid) {
    for (int i = 0; i < dock->window_count; i++) {
        if (!dock->windows[i].alive) continue;
        if (SDL_GetWindowID(dock->windows[i].sdl_window) == wid)
            return &dock->windows[i];
    }
    return NULL;
}

static dong_engine_t* dock_create_engine(dong_dock_t* dock,
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

    dong_sdl_platform_shutdown();

    if (dock->gpu_device) {
        SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, dock->primary_window);
        SDL_DestroyGPUDevice(dock->gpu_device);
    }
    if (dock->primary_window) SDL_DestroyWindow(dock->primary_window);

    SDL_Quit();
    free(dock);
}

// =============================================================================
// Pane Management
// =============================================================================

DONG_APPCORE_API dong_dock_pane_t* dong_dock_add_pane(
    dong_dock_t* dock, const dong_dock_pane_config_t* config)
{
    if (!dock || !config) return NULL;

    int pi = dock_alloc_pane_slot(dock);
    if (pi < 0) return NULL;

    dong_dock_pane_t* pane = &dock->panes[pi];
    pane->alive = 1;
    pane->index = pi;
    pane->window_index = 0; // primary window

    if (config->title) {
        strncpy(pane->title, config->title, DOCK_MAX_TITLE - 1);
        pane->title[DOCK_MAX_TITLE - 1] = 0;
    }

    uint32_t pw = config->width  ? config->width  : dock->windows[0].width;
    uint32_t ph = config->height ? config->height : dock->windows[0].height;

    pane->engine = dock_create_engine(dock, pw, ph, config->shared_js, config->view_name);
    if (!pane->engine) {
        pane->alive = 0;
        return NULL;
    }

    if (config->resource_root) {
        strncpy(pane->resource_root, config->resource_root, sizeof(pane->resource_root) - 1);
        dong_engine_set_resource_root(pane->engine, config->resource_root);
    }

    if (config->html) {
        dong_engine_load_html(pane->engine, config->html);
    } else if (config->html_file) {
        char dir[1024];
        dock_extract_dir(config->html_file, dir, sizeof(dir));
        if (dir[0]) dong_engine_set_resource_root(pane->engine, dir);

        FILE* f = fopen(config->html_file, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* buf = (char*)malloc(sz + 1);
            if (buf) {
                size_t rd = fread(buf, 1, sz, f);
                buf[rd] = 0;
                dong_engine_load_html(pane->engine, buf);
                free(buf);
            }
            fclose(f);
        }
    }

    // Insert into primary window's split tree
    dong_dock_window_t* wn = &dock->windows[0];
    if (!wn->root) {
        wn->root = dock_node_new_leaf(pi);
    } else {
        // Find a leaf and add as tab
        dock_node_t* leaf = wn->root;
        while (leaf->type != DOCK_NODE_LEAF)
            leaf = leaf->children[0];
        dock_node_add_tab(leaf, pi);
    }

    pane->node = dock_node_find_leaf(wn->root, pi);
    if (dock->focused_pane_index < 0) dock->focused_pane_index = pi;

    return pane;
}

DONG_APPCORE_API dong_dock_pane_t* dong_dock_split(
    dong_dock_t* dock, dong_dock_pane_t* neighbor,
    dong_dock_edge_t edge, float ratio,
    const dong_dock_pane_config_t* config)
{
    if (!dock || !neighbor || !config) return NULL;
    if (!neighbor->alive) return NULL;

    int pi = dock_alloc_pane_slot(dock);
    if (pi < 0) return NULL;

    dong_dock_pane_t* pane = &dock->panes[pi];
    pane->alive = 1;
    pane->index = pi;
    pane->window_index = neighbor->window_index;

    if (config->title) {
        strncpy(pane->title, config->title, DOCK_MAX_TITLE - 1);
        pane->title[DOCK_MAX_TITLE - 1] = 0;
    }

    uint32_t pw = config->width  ? config->width  : 400;
    uint32_t ph = config->height ? config->height : 300;

    pane->engine = dock_create_engine(dock, pw, ph, config->shared_js, config->view_name);
    if (!pane->engine) {
        pane->alive = 0;
        return NULL;
    }

    if (config->resource_root) {
        strncpy(pane->resource_root, config->resource_root, sizeof(pane->resource_root) - 1);
        dong_engine_set_resource_root(pane->engine, config->resource_root);
    }

    if (config->html) {
        dong_engine_load_html(pane->engine, config->html);
    } else if (config->html_file) {
        char dir[1024];
        dock_extract_dir(config->html_file, dir, sizeof(dir));
        if (dir[0]) dong_engine_set_resource_root(pane->engine, dir);

        FILE* f = fopen(config->html_file, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* buf = (char*)malloc(sz + 1);
            if (buf) {
                size_t rd = fread(buf, 1, sz, f);
                buf[rd] = 0;
                dong_engine_load_html(pane->engine, buf);
                free(buf);
            }
            fclose(f);
        }
    }

    // Split in the tree
    dock_node_t* leaf = neighbor->node;
    if (!leaf) leaf = dock_node_find_leaf(
        dock->windows[neighbor->window_index].root, neighbor->index);

    if (edge == DONG_DOCK_TAB) {
        dock_node_add_tab(leaf, pi);
        pane->node = leaf;
    } else {
        dock_node_t* new_leaf = dock_node_split(leaf, edge, pi,
                                                  ratio > 0.0f ? ratio : 0.5f);
        pane->node = new_leaf;
        // Update neighbor's node pointer (may have moved due to in-place conversion)
        neighbor->node = dock_node_find_leaf(
            dock->windows[neighbor->window_index].root, neighbor->index);
    }

    return pane;
}

DONG_APPCORE_API void dong_dock_remove_pane(dong_dock_t* dock, dong_dock_pane_t* pane) {
    if (!dock || !pane || !pane->alive) return;

    int pi = pane->index;

    // Callback
    if (dock->pane_close_cb) {
        if (!dock->pane_close_cb(pane, dock->pane_close_ud))
            return; // vetoed
    }

    // Remove from split tree
    dong_dock_window_t* wn = &dock->windows[pane->window_index];
    if (wn->alive && wn->root) {
        dock_node_remove_pane(wn->root, pi);
    }

    // Destroy engine and texture
    if (pane->engine) dong_engine_destroy(pane->engine);
    if (pane->offscreen_tex)
        SDL_ReleaseGPUTexture(dock->gpu_device, pane->offscreen_tex);
    if (pane->title_tex)
        SDL_ReleaseGPUTexture(dock->gpu_device, pane->title_tex);

    pane->alive = 0;
    pane->engine = NULL;
    pane->offscreen_tex = NULL;
    pane->title_tex = NULL;
    pane->node = NULL;

    // Update focus
    if (dock->focused_pane_index == pi) {
        dock->focused_pane_index = -1;
        for (int i = 0; i < dock->pane_count; i++) {
            if (dock->panes[i].alive) {
                dock->focused_pane_index = i;
                break;
            }
        }
    }
}

// =============================================================================
// Event translation (mirrors app.c pattern)
// =============================================================================

static dong_app_event_t dock_translate_sdl_event(const SDL_Event* e) {
    dong_app_event_t out;
    memset(&out, 0, sizeof(out));
    out.type = DONG_APP_EVENT_NONE;
    if (!e) return out;

    switch (e->type) {
    case SDL_EVENT_QUIT:
        out.type = DONG_APP_EVENT_QUIT;
        return out;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        out.type = DONG_APP_EVENT_QUIT;
        return out;
    case SDL_EVENT_WINDOW_RESIZED:
        out.type = DONG_APP_EVENT_WINDOW_RESIZED;
        out.window_resized.width  = (uint32_t)e->window.data1;
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
        out.mouse_button.button  = (int32_t)e->button.button;
        out.mouse_button.pressed = (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
        out.mouse_button.x = (int32_t)e->button.x;
        out.mouse_button.y = (int32_t)e->button.y;
        return out;
    case SDL_EVENT_MOUSE_WHEEL:
        out.type = DONG_APP_EVENT_MOUSE_WHEEL;
        out.mouse_wheel.x = (int32_t)e->wheel.mouse_x;
        out.mouse_wheel.y = (int32_t)e->wheel.mouse_y;
        out.mouse_wheel.delta_x =  e->wheel.x;
        out.mouse_wheel.delta_y = -e->wheel.y; // flip: positive = scroll down
        return out;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        out.type = DONG_APP_EVENT_KEY;
        out.key.key_code = (uint32_t)e->key.key;
        out.key.scancode = (uint32_t)e->key.scancode;
        out.key.pressed  = (e->type == SDL_EVENT_KEY_DOWN) ? 1 : 0;
        out.key.repeat   = e->key.repeat ? 1 : 0;
        return out;
    case SDL_EVENT_TEXT_INPUT:
        out.type = DONG_APP_EVENT_TEXT;
        out.text.text = e->text.text;
        return out;
    case SDL_EVENT_TEXT_EDITING:
        out.type = DONG_APP_EVENT_TEXT_EDITING;
        out.text_editing.text = e->edit.text;
        out.text_editing.cursor = e->edit.start;
        out.text_editing.selection_length = e->edit.length;
        return out;
    default:
        return out;
    }
}

static void dock_forward_event_to_engine(dong_engine_t* engine,
                                          const dong_app_event_t* ev) {
    if (!engine || !ev) return;
    switch (ev->type) {
    case DONG_APP_EVENT_WINDOW_RESIZED:
        dong_engine_resize(engine, ev->window_resized.width, ev->window_resized.height);
        break;
    case DONG_APP_EVENT_MOUSE_MOVE:
        dong_engine_send_mouse_move(engine, ev->mouse_move.x, ev->mouse_move.y);
        break;
    case DONG_APP_EVENT_MOUSE_BUTTON:
        // Send mouse_move first for accurate hit-test
        dong_engine_send_mouse_move(engine, ev->mouse_button.x, ev->mouse_button.y);
        dong_engine_send_mouse_button(engine, ev->mouse_button.button, ev->mouse_button.pressed);
        break;
    case DONG_APP_EVENT_MOUSE_WHEEL:
        dong_engine_send_mouse_move(engine, ev->mouse_wheel.x, ev->mouse_wheel.y);
        dong_engine_send_mouse_wheel(engine, ev->mouse_wheel.delta_x, ev->mouse_wheel.delta_y);
        break;
    case DONG_APP_EVENT_KEY:
        dong_engine_send_key(engine, ev->key.key_code, ev->key.pressed);
        break;
    case DONG_APP_EVENT_TEXT:
        dong_engine_send_text(engine, ev->text.text);
        break;
    case DONG_APP_EVENT_TEXT_EDITING:
        dong_engine_send_text_editing(engine, ev->text_editing.text,
                                       ev->text_editing.cursor,
                                       ev->text_editing.selection_length);
        break;
    default:
        break;
    }
}

// Get the SDL_WindowID from an SDL_Event (returns 0 if no window context)
static SDL_WindowID dock_event_window_id(const SDL_Event* e) {
    switch (e->type) {
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return e->window.windowID;
    case SDL_EVENT_MOUSE_MOTION:
        return e->motion.windowID;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        return e->button.windowID;
    case SDL_EVENT_MOUSE_WHEEL:
        return e->wheel.windowID;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        return e->key.windowID;
    case SDL_EVENT_TEXT_INPUT:
        return e->text.windowID;
    case SDL_EVENT_TEXT_EDITING:
        return e->edit.windowID;
    default:
        return 0;
    }
}

// =============================================================================
// Embedded 8x16 bitmap font (printable ASCII 32-126, 1 bit per pixel)
// Each glyph = 16 bytes (8 pixels wide x 16 rows)
// =============================================================================

static const uint8_t s_font_8x16[95][16] = {
    // 32 (space)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 33 !
    {0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    // 34 "
    {0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 35 #
    {0x00,0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00},
    // 36 $
    {0x18,0x18,0x7C,0xC6,0xC2,0xC0,0x7C,0x06,0x06,0x86,0xC6,0x7C,0x18,0x18,0x00,0x00},
    // 37 %
    {0x00,0x00,0x00,0x00,0xC2,0xC6,0x0C,0x18,0x30,0x60,0xC6,0x86,0x00,0x00,0x00,0x00},
    // 38 &
    {0x00,0x00,0x38,0x6C,0x6C,0x38,0x76,0xDC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    // 39 '
    {0x00,0x30,0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 40 (
    {0x00,0x00,0x0C,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x18,0x0C,0x00,0x00,0x00,0x00},
    // 41 )
    {0x00,0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x18,0x30,0x00,0x00,0x00,0x00},
    // 42 *
    {0x00,0x00,0x00,0x00,0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,0x00},
    // 43 +
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00},
    // 44 ,
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00},
    // 45 -
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 46 .
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    // 47 /
    {0x00,0x00,0x00,0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00},
    // 48 0
    {0x00,0x00,0x7C,0xC6,0xC6,0xCE,0xDE,0xF6,0xE6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 49 1
    {0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00},
    // 50 2
    {0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,0x60,0xC0,0xC6,0xFE,0x00,0x00,0x00,0x00},
    // 51 3
    {0x00,0x00,0x7C,0xC6,0x06,0x06,0x3C,0x06,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 52 4
    {0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00},
    // 53 5
    {0x00,0x00,0xFE,0xC0,0xC0,0xC0,0xFC,0x06,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 54 6
    {0x00,0x00,0x38,0x60,0xC0,0xC0,0xFC,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 55 7
    {0x00,0x00,0xFE,0xC6,0x06,0x06,0x0C,0x18,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00},
    // 56 8
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 57 9
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,0x06,0x06,0x0C,0x78,0x00,0x00,0x00,0x00},
    // 58 :
    {0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
    // 59 ;
    {0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,0x00},
    // 60 <
    {0x00,0x00,0x00,0x06,0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x06,0x00,0x00,0x00,0x00},
    // 61 =
    {0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 62 >
    {0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00},
    // 63 ?
    {0x00,0x00,0x7C,0xC6,0xC6,0x0C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    // 64 @
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xDE,0xDE,0xDE,0xDC,0xC0,0x7C,0x00,0x00,0x00,0x00},
    // 65 A
    {0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    // 66 B
    {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0x66,0xFC,0x00,0x00,0x00,0x00},
    // 67 C
    {0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xC0,0xC0,0xC2,0x66,0x3C,0x00,0x00,0x00,0x00},
    // 68 D
    {0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,0x00},
    // 69 E
    {0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00},
    // 70 F
    {0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00},
    // 71 G
    {0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xDE,0xC6,0xC6,0x66,0x3A,0x00,0x00,0x00,0x00},
    // 72 H
    {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    // 73 I
    {0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    // 74 J
    {0x00,0x00,0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0xCC,0xCC,0xCC,0x78,0x00,0x00,0x00,0x00},
    // 75 K
    {0x00,0x00,0xE6,0x66,0x66,0x6C,0x78,0x78,0x6C,0x66,0x66,0xE6,0x00,0x00,0x00,0x00},
    // 76 L
    {0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,0x00},
    // 77 M
    {0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    // 78 N
    {0x00,0x00,0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,0x00},
    // 79 O
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 80 P
    {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00},
    // 81 Q
    {0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x0C,0x0E,0x00,0x00},
    // 82 R
    {0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x6C,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00},
    // 83 S
    {0x00,0x00,0x7C,0xC6,0xC6,0x60,0x38,0x0C,0x06,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 84 T
    {0x00,0x00,0x7E,0x7E,0x5A,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    // 85 U
    {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 86 V
    {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00,0x00,0x00,0x00},
    // 87 W
    {0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xD6,0xD6,0xD6,0xFE,0xEE,0x6C,0x00,0x00,0x00,0x00},
    // 88 X
    {0x00,0x00,0xC6,0xC6,0x6C,0x7C,0x38,0x38,0x7C,0x6C,0xC6,0xC6,0x00,0x00,0x00,0x00},
    // 89 Y
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    // 90 Z
    {0x00,0x00,0xFE,0xC6,0x86,0x0C,0x18,0x30,0x60,0xC2,0xC6,0xFE,0x00,0x00,0x00,0x00},
    // 91 [
    {0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,0x00,0x00,0x00},
    // 92 backslash
    {0x00,0x00,0x00,0x80,0xC0,0xE0,0x70,0x38,0x1C,0x0E,0x06,0x02,0x00,0x00,0x00,0x00},
    // 93 ]
    {0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,0x00},
    // 94 ^
    {0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 95 _
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00},
    // 96 `
    {0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 97 a
    {0x00,0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    // 98 b
    {0x00,0x00,0xE0,0x60,0x60,0x78,0x6C,0x66,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00},
    // 99 c
    {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 100 d
    {0x00,0x00,0x1C,0x0C,0x0C,0x3C,0x6C,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    // 101 e
    {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 102 f
    {0x00,0x00,0x1C,0x36,0x32,0x30,0x78,0x30,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00},
    // 103 g
    {0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,0x00},
    // 104 h
    {0x00,0x00,0xE0,0x60,0x60,0x6C,0x76,0x66,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,0x00},
    // 105 i
    {0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    // 106 j
    {0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00},
    // 107 k
    {0x00,0x00,0xE0,0x60,0x60,0x66,0x6C,0x78,0x78,0x6C,0x66,0xE6,0x00,0x00,0x00,0x00},
    // 108 l
    {0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    // 109 m
    {0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xD6,0xD6,0xC6,0x00,0x00,0x00,0x00},
    // 110 n
    {0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00},
    // 111 o
    {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 112 p
    {0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00},
    // 113 q
    {0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,0x00},
    // 114 r
    {0x00,0x00,0x00,0x00,0x00,0xDC,0x76,0x66,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,0x00},
    // 115 s
    {0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00,0x00,0x00,0x00},
    // 116 t
    {0x00,0x00,0x10,0x30,0x30,0xFC,0x30,0x30,0x30,0x30,0x36,0x1C,0x00,0x00,0x00,0x00},
    // 117 u
    {0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,0x00},
    // 118 v
    {0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,0x00},
    // 119 w
    {0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xD6,0xD6,0xD6,0xFE,0x6C,0x00,0x00,0x00,0x00},
    // 120 x
    {0x00,0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00},
    // 121 y
    {0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,0x00},
    // 122 z
    {0x00,0x00,0x00,0x00,0x00,0xFE,0xCC,0x18,0x30,0x60,0xC6,0xFE,0x00,0x00,0x00,0x00},
    // 123 {
    {0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x18,0x0E,0x00,0x00,0x00,0x00},
    // 124 |
    {0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00},
    // 125 }
    {0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,0x18,0x18,0x18,0x70,0x00,0x00,0x00,0x00},
    // 126 ~
    {0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

// Render a pane's title to a GPU texture (cached, re-rendered on change)
static void dock_render_title_texture(dong_dock_t* dock, dong_dock_pane_t* pane) {
    if (!pane->alive || !pane->title[0]) return;

    // Skip if title hasn't changed
    if (pane->title_tex && strcmp(pane->title, pane->title_rendered) == 0) return;

    // Release old texture
    if (pane->title_tex) {
        SDL_ReleaseGPUTexture(dock->gpu_device, pane->title_tex);
        pane->title_tex = NULL;
    }

    size_t len = strlen(pane->title);
    if (len > 32) len = 32; // clamp to reasonable length
    uint32_t tw = (uint32_t)(len * 8);   // 1x scale: 8px per char
    uint32_t th = 16;                     // 1x scale: 16px height
    if (tw == 0) return;

    // Render to RGBA buffer at 1x scale
    uint32_t buf_size = tw * th * 4;
    uint8_t* buf = (uint8_t*)malloc(buf_size);
    if (!buf) return;

    // Fill with transparent (will be blitted over already-drawn tab bg)
    // Use matching background so blit (which has no alpha blend) looks clean
    for (uint32_t i = 0; i < tw * th; i++) {
        buf[i * 4 + 0] = 0x3c; // R  (matches tex_tab_active #3c3c3c)
        buf[i * 4 + 1] = 0x3c; // G
        buf[i * 4 + 2] = 0x3c; // B
        buf[i * 4 + 3] = 0xff; // A
    }

    for (size_t ci = 0; ci < len; ci++) {
        unsigned char ch = (unsigned char)pane->title[ci];
        if (ch < 32 || ch > 126) ch = '?';
        const uint8_t* glyph = s_font_8x16[ch - 32];

        for (int row = 0; row < 16; row++) {
            uint8_t bits = glyph[row];
            for (int col = 0; col < 8; col++) {
                if (bits & (0x80 >> col)) {
                    uint32_t px = (uint32_t)(ci * 8 + col);
                    uint32_t py = (uint32_t)row;
                    uint32_t off = (py * tw + px) * 4;
                    buf[off + 0] = 0xCC; // R (light gray)
                    buf[off + 1] = 0xCC; // G
                    buf[off + 2] = 0xCC; // B
                    buf[off + 3] = 0xFF; // A
                }
            }
        }
    }

    // Create GPU texture
    SDL_GPUTextureCreateInfo ti = {0};
    ti.type = SDL_GPU_TEXTURETYPE_2D;
    ti.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    ti.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    ti.width = tw;
    ti.height = th;
    ti.layer_count_or_depth = 1;
    ti.num_levels = 1;
    ti.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUTexture* tex = SDL_CreateGPUTexture(dock->gpu_device, &ti);
    if (!tex) { free(buf); return; }

    // Upload via transfer buffer
    SDL_GPUTransferBufferCreateInfo tbi = {0};
    tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbi.size = buf_size;
    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(dock->gpu_device, &tbi);
    if (!tb) {
        SDL_ReleaseGPUTexture(dock->gpu_device, tex);
        free(buf);
        return;
    }

    uint8_t* ptr = (uint8_t*)SDL_MapGPUTransferBuffer(dock->gpu_device, tb, 0);
    if (!ptr) {
        SDL_ReleaseGPUTransferBuffer(dock->gpu_device, tb);
        SDL_ReleaseGPUTexture(dock->gpu_device, tex);
        free(buf);
        return;
    }
    memcpy(ptr, buf, buf_size);
    SDL_UnmapGPUTransferBuffer(dock->gpu_device, tb);
    free(buf);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(dock->gpu_device);
    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureTransferInfo src = {0};
    src.transfer_buffer = tb;
    src.offset = 0;
    src.pixels_per_row = tw;
    src.rows_per_layer = th;

    SDL_GPUTextureRegion dst = {0};
    dst.texture = tex;
    dst.w = tw;
    dst.h = th;
    dst.d = 1;

    SDL_UploadToGPUTexture(cp, &src, &dst, 0);
    SDL_EndGPUCopyPass(cp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(dock->gpu_device, tb);

    pane->title_tex = tex;
    pane->title_tex_w = tw;
    pane->title_tex_h = th;
    strncpy(pane->title_rendered, pane->title, DOCK_MAX_TITLE - 1);
    pane->title_rendered[DOCK_MAX_TITLE - 1] = 0;
}

// =============================================================================
// Tab width computation
// =============================================================================

static uint32_t dock_tab_width(const dock_node_t* node) {
    if (node->pane_count <= 0) return 0;
    uint32_t avail = node->w - DOCK_TAB_PAD;
    uint32_t tw = avail / (uint32_t)node->pane_count;
    if (tw < DOCK_TAB_MIN_WIDTH) tw = DOCK_TAB_MIN_WIDTH;
    if (tw > DOCK_TAB_MAX_WIDTH) tw = DOCK_TAB_MAX_WIDTH;
    return tw;
}

// =============================================================================
// Drag state machine
// =============================================================================

// Compute drop zone from mouse position relative to a leaf node
static dock_drop_zone_t dock_compute_drop_zone(
    const dock_node_t* node, int32_t mx, int32_t my)
{
    if (!node || node->type != DOCK_NODE_LEAF) return DOCK_DROP_NONE;

    int32_t lx = mx - node->x;
    int32_t ly = my - node->y;
    float fx = (float)lx / (float)node->w;
    float fy = (float)ly / (float)node->h;

    // Title bar area → TAB
    if (ly < DOCK_TITLE_BAR_HEIGHT) return DOCK_DROP_TAB;

    // Edge fractions
    if (fx < DOCK_DROP_EDGE_FRAC)       return DOCK_DROP_LEFT;
    if (fx > 1.0f - DOCK_DROP_EDGE_FRAC) return DOCK_DROP_RIGHT;
    if (fy < DOCK_DROP_EDGE_FRAC)       return DOCK_DROP_TOP;
    if (fy > 1.0f - DOCK_DROP_EDGE_FRAC) return DOCK_DROP_BOTTOM;

    // Center → TAB
    return DOCK_DROP_TAB;
}

// Compute the indicator rectangle for a drop zone
static void dock_compute_drop_rect(const dock_node_t* node,
    dock_drop_zone_t zone,
    int32_t* rx, int32_t* ry, uint32_t* rw, uint32_t* rh)
{
    int32_t cx, cy;
    uint32_t cw, ch;
    dock_leaf_content_rect(node, &cx, &cy, &cw, &ch);

    switch (zone) {
    case DOCK_DROP_TAB:
        *rx = node->x; *ry = node->y;
        *rw = node->w; *rh = DOCK_TITLE_BAR_HEIGHT;
        break;
    case DOCK_DROP_LEFT:
        *rx = cx; *ry = cy;
        *rw = cw / 2; *rh = ch;
        break;
    case DOCK_DROP_RIGHT:
        *rx = cx + (int32_t)(cw / 2); *ry = cy;
        *rw = cw - cw / 2; *rh = ch;
        break;
    case DOCK_DROP_TOP:
        *rx = cx; *ry = cy;
        *rw = cw; *rh = ch / 2;
        break;
    case DOCK_DROP_BOTTOM:
        *rx = cx; *ry = cy + (int32_t)(ch / 2);
        *rw = cw; *rh = ch - ch / 2;
        break;
    default:
        *rx = 0; *ry = 0; *rw = 0; *rh = 0;
        break;
    }
}

// Update drag: PENDING→ACTIVE on threshold, compute drop zone across ALL windows
static void dock_update_drag(dong_dock_t* dock) {
    dock_drag_t* d = &dock->drag;

    // Get global mouse position for cross-window hit-testing
    float gx, gy;
    SDL_GetGlobalMouseState(&gx, &gy);

    if (d->state == DOCK_DRAG_PENDING) {
        int32_t dx = (int32_t)gx - d->start_global_x;
        int32_t dy = (int32_t)gy - d->start_global_y;
        if (dx * dx + dy * dy >= DOCK_DRAG_THRESHOLD * DOCK_DRAG_THRESHOLD) {
            d->state = DOCK_DRAG_ACTIVE;
            SDL_CaptureMouse(true);
        } else {
            return;
        }
    }

    // Hit-test all alive windows
    for (int i = 0; i < dock->window_count; i++) {
        dong_dock_window_t* wn = &dock->windows[i];
        if (!wn->alive || !wn->sdl_window || !wn->root) continue;

        int wx, wy;
        SDL_GetWindowPosition(wn->sdl_window, &wx, &wy);
        int32_t lx = (int32_t)gx - wx;
        int32_t ly = (int32_t)gy - wy;

        if (lx < 0 || ly < 0 || lx >= (int32_t)wn->width || ly >= (int32_t)wn->height)
            continue;

        // Layout tree for up-to-date hit coordinates
        dock_node_layout(wn->root, 0, 0, wn->width, wn->height);

        // Check window-edge band first (window-level split)
        // Only matters when root is a split (single leaf = same as leaf drop)
        int32_t band = DOCK_WIN_EDGE_BAND;
        if (wn->root->type != DOCK_NODE_LEAF) {
            dock_drop_zone_t win_zone = DOCK_DROP_NONE;
            if (lx < band)
                win_zone = DOCK_DROP_WIN_LEFT;
            else if (lx >= (int32_t)wn->width - band)
                win_zone = DOCK_DROP_WIN_RIGHT;
            else if (ly < band)
                win_zone = DOCK_DROP_WIN_TOP;
            else if (ly >= (int32_t)wn->height - band)
                win_zone = DOCK_DROP_WIN_BOTTOM;

            if (win_zone != DOCK_DROP_NONE) {
                d->hover_window_index = wn->index;
                d->current_x = lx;
                d->current_y = ly;
                d->drop_zone = win_zone;

                // Compute drop rect as half the window area on the appropriate side
                switch (win_zone) {
                case DOCK_DROP_WIN_LEFT:
                    d->target_x = 0;
                    d->target_y = 0;
                    d->target_w = wn->width / 2;
                    d->target_h = wn->height;
                    break;
                case DOCK_DROP_WIN_RIGHT:
                    d->target_x = (int32_t)(wn->width / 2);
                    d->target_y = 0;
                    d->target_w = wn->width - wn->width / 2;
                    d->target_h = wn->height;
                    break;
                case DOCK_DROP_WIN_TOP:
                    d->target_x = 0;
                    d->target_y = 0;
                    d->target_w = wn->width;
                    d->target_h = wn->height / 2;
                    break;
                case DOCK_DROP_WIN_BOTTOM:
                    d->target_x = 0;
                    d->target_y = (int32_t)(wn->height / 2);
                    d->target_w = wn->width;
                    d->target_h = wn->height - wn->height / 2;
                    break;
                default:
                    break;
                }
                return;
            }
        }

        // Fall through to leaf-level hit test
        dock_node_t* hover = dock_node_hit_test(wn->root, lx, ly);
        if (hover && hover->type == DOCK_NODE_LEAF && hover->pane_count > 0) {
            d->hover_window_index = wn->index;
            d->current_x = lx;
            d->current_y = ly;
            d->drop_zone = dock_compute_drop_zone(hover, lx, ly);

            // Check for tab reorder within same leaf
            dock_node_t* src_leaf = NULL;
            if (d->source_pane_index >= 0 &&
                d->source_window_index == wn->index) {
                src_leaf = dock_node_find_leaf(wn->root, d->source_pane_index);
            }
            if (src_leaf == hover && hover->pane_count > 1 &&
                d->drop_zone == DOCK_DROP_TAB)
            {
                // Compute insertion position from mouse X
                uint32_t tab_w = dock_tab_width(hover);
                int32_t rel_x = lx - hover->x - DOCK_TAB_PAD;
                int insert_pos;
                if (rel_x < 0) {
                    insert_pos = 0;
                } else {
                    insert_pos = (rel_x + (int32_t)(tab_w + DOCK_TAB_PAD) / 2)
                                 / (int32_t)(tab_w + DOCK_TAB_PAD);
                    if (insert_pos > hover->pane_count)
                        insert_pos = hover->pane_count;
                }
                d->reorder_insert_pos = insert_pos;
                // Suppress normal drop indicator for reorder
                d->target_w = 0;
                d->target_h = 0;
                return;
            }

            d->reorder_insert_pos = -1;
            dock_compute_drop_rect(hover, d->drop_zone,
                &d->target_x, &d->target_y, &d->target_w, &d->target_h);
            return;
        }
    }

    // Not over any window → DOCK_DROP_NONE (will detach on release)
    d->hover_window_index = -1;
    d->drop_zone = DOCK_DROP_NONE;
    d->target_w = 0;
    d->target_h = 0;
    d->reorder_insert_pos = -1;
}

// Convert drop zone to dock edge
static dong_dock_edge_t dock_zone_to_edge(dock_drop_zone_t zone) {
    switch (zone) {
    case DOCK_DROP_LEFT:      return DONG_DOCK_LEFT;
    case DOCK_DROP_RIGHT:     return DONG_DOCK_RIGHT;
    case DOCK_DROP_TOP:       return DONG_DOCK_TOP;
    case DOCK_DROP_BOTTOM:    return DONG_DOCK_BOTTOM;
    case DOCK_DROP_TAB:       return DONG_DOCK_TAB;
    case DOCK_DROP_WIN_LEFT:  return DONG_DOCK_LEFT;
    case DOCK_DROP_WIN_RIGHT: return DONG_DOCK_RIGHT;
    case DOCK_DROP_WIN_TOP:   return DONG_DOCK_TOP;
    case DOCK_DROP_WIN_BOTTOM:return DONG_DOCK_BOTTOM;
    default:                  return DONG_DOCK_TAB;
    }
}

// Finish drag: perform action based on drop zone
static void dock_finish_drag(dong_dock_t* dock) {
    dock_drag_t* d = &dock->drag;

    // Release mouse capture
    if (d->state == DOCK_DRAG_ACTIVE)
        SDL_CaptureMouse(false);

    if (d->state != DOCK_DRAG_ACTIVE) {
        // Never exceeded threshold, treat as simple click (already handled)
        d->state = DOCK_DRAG_NONE;
        d->source_pane_index = -1;
        d->reorder_insert_pos = -1;
        return;
    }

    int src_pi = d->source_pane_index;
    if (src_pi < 0 || src_pi >= dock->pane_count || !dock->panes[src_pi].alive) {
        d->state = DOCK_DRAG_NONE;
        d->source_pane_index = -1;
        d->reorder_insert_pos = -1;
        return;
    }

    dong_dock_pane_t* src_pane = &dock->panes[src_pi];

    // Find hover target (hover_window_index and current_x/y set by dock_update_drag)
    int hover_wi = d->hover_window_index;
    dong_dock_window_t* hover_win = NULL;
    dock_node_t* hover_node = NULL;
    if (hover_wi >= 0 && hover_wi < dock->window_count && dock->windows[hover_wi].alive) {
        hover_win = &dock->windows[hover_wi];
        hover_node = hover_win->root
            ? dock_node_hit_test(hover_win->root, d->current_x, d->current_y)
            : NULL;
    }

    // Handle window-level drops
    if (d->drop_zone >= DOCK_DROP_WIN_LEFT && d->drop_zone <= DOCK_DROP_WIN_BOTTOM
        && hover_win && hover_win->root) {
        // Remove pane from source
        dong_dock_window_t* old_win = &dock->windows[src_pane->window_index];
        if (old_win->alive && old_win->root) {
            dock_node_remove_pane(old_win->root, src_pi);
            if (old_win->root->type == DOCK_NODE_LEAF && old_win->root->pane_count == 0) {
                dock_node_free(old_win->root);
                old_win->root = NULL;
                if (!old_win->is_primary) {
                    SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, old_win->sdl_window);
                    SDL_DestroyWindow(old_win->sdl_window);
                    old_win->sdl_window = NULL;
                    old_win->alive = 0;
                }
            }
        }

        // Split at window root level
        dong_dock_edge_t edge = dock_zone_to_edge(d->drop_zone);
        dock_node_t* new_leaf = dock_node_split_root(
            &hover_win->root, edge, src_pi, 0.5f);

        // Update pane metadata
        src_pane->window_index = hover_win->index;
        src_pane->node = new_leaf;
    } else if (d->drop_zone == DOCK_DROP_NONE || !hover_node ||
        hover_node->type != DOCK_NODE_LEAF || hover_node->pane_count == 0) {
        // No valid target: detach to new window at global mouse position
        float gx, gy;
        SDL_GetGlobalMouseState(&gx, &gy);
        dong_dock_detach(dock, src_pane, (int32_t)gx - 50, (int32_t)gy - 10, 0, 0);
    } else {
        // Leaf-level drop
        int target_pi = hover_node->pane_indices[hover_node->active_tab];
        dock_node_t* src_leaf = dock_node_find_leaf(
            dock->windows[src_pane->window_index].root, src_pi);

        if (src_leaf == hover_node) {
            // Same leaf: reorder tabs if insertion position differs
            if (d->reorder_insert_pos >= 0 && hover_node->pane_count > 1) {
                // Find current index of dragged pane in this leaf
                int cur = -1;
                for (int i = 0; i < hover_node->pane_count; i++) {
                    if (hover_node->pane_indices[i] == src_pi) { cur = i; break; }
                }
                int insert = d->reorder_insert_pos;
                // Adjust: if inserting after the current position, the removal
                // shifts indices down, so target becomes insert-1
                if (cur >= 0 && insert != cur && insert != cur + 1) {
                    // Remove from current position
                    int pi_val = hover_node->pane_indices[cur];
                    if (insert > cur) insert--; // adjust for removal shift
                    // Shift elements
                    if (cur < insert) {
                        for (int i = cur; i < insert; i++)
                            hover_node->pane_indices[i] = hover_node->pane_indices[i + 1];
                    } else {
                        for (int i = cur; i > insert; i--)
                            hover_node->pane_indices[i] = hover_node->pane_indices[i - 1];
                    }
                    hover_node->pane_indices[insert] = pi_val;
                    hover_node->active_tab = insert;
                }
            }
        } else if (target_pi >= 0 && target_pi < dock->pane_count &&
                   dock->panes[target_pi].alive) {
            dong_dock_pane_t* target_pane = &dock->panes[target_pi];
            dong_dock_edge_t edge = dock_zone_to_edge(d->drop_zone);
            dong_dock_attach(dock, src_pane, target_pane, edge);
        }
    }

    // Reset drag state
    d->state = DOCK_DRAG_NONE;
    d->source_pane_index = -1;
    d->source_tab_index = -1;
    d->drop_zone = DOCK_DROP_NONE;
    d->target_w = 0;
    d->target_h = 0;
    d->reorder_insert_pos = -1;
}

// =============================================================================
// Poll Events
// =============================================================================

DONG_APPCORE_API int dong_dock_poll_events(dong_dock_t* dock) {
    if (!dock) return 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            dock->running = 0;
            return 0;
        }

        SDL_WindowID wid = dock_event_window_id(&e);
        dong_dock_window_t* win = wid ? dock_find_window_by_sdl_id(dock, wid) : NULL;

        // Window close
        if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && win) {
            if (win->is_primary) {
                dock->running = 0;
                return 0;
            }
            // Close secondary window: callback
            if (dock->window_close_cb) {
                if (!dock->window_close_cb(win, dock->window_close_ud))
                    continue; // vetoed
            }
            // Destroy all panes in this window
            for (int i = 0; i < dock->pane_count; i++) {
                dong_dock_pane_t* p = &dock->panes[i];
                if (!p->alive || p->window_index != win->index) continue;
                if (p->engine) dong_engine_destroy(p->engine);
                if (p->offscreen_tex)
                    SDL_ReleaseGPUTexture(dock->gpu_device, p->offscreen_tex);
                if (p->title_tex)
                    SDL_ReleaseGPUTexture(dock->gpu_device, p->title_tex);
                p->alive = 0;
                p->engine = NULL;
                p->offscreen_tex = NULL;
                p->title_tex = NULL;
            }
            dock_node_free(win->root);
            win->root = NULL;
            SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, win->sdl_window);
            SDL_DestroyWindow(win->sdl_window);
            win->sdl_window = NULL;
            win->alive = 0;
            continue;
        }

        // Window resize
        if (e.type == SDL_EVENT_WINDOW_RESIZED && win) {
            win->width  = (uint32_t)e.window.data1;
            win->height = (uint32_t)e.window.data2;
            continue;
        }

        if (!win) continue;

        dong_app_event_t ev = dock_translate_sdl_event(&e);
        if (ev.type == DONG_APP_EVENT_NONE) continue;

        // Mouse events: hit-test to find target pane, convert to local coords
        if (ev.type == DONG_APP_EVENT_MOUSE_MOVE ||
            ev.type == DONG_APP_EVENT_MOUSE_BUTTON ||
            ev.type == DONG_APP_EVENT_MOUSE_WHEEL)
        {
            int32_t mx, my;
            if (ev.type == DONG_APP_EVENT_MOUSE_MOVE) {
                mx = ev.mouse_move.x; my = ev.mouse_move.y;
            } else if (ev.type == DONG_APP_EVENT_MOUSE_BUTTON) {
                mx = ev.mouse_button.x; my = ev.mouse_button.y;
            } else {
                mx = ev.mouse_wheel.x; my = ev.mouse_wheel.y;
            }

            // Layout the tree to ensure coordinates are up to date
            if (win->root)
                dock_node_layout(win->root, 0, 0, win->width, win->height);

            // --- Divider drag interaction (priority over tab drag) ---
            if (dock->divider.state == DOCK_DIV_DRAGGING) {
                if (ev.type == DONG_APP_EVENT_MOUSE_MOVE) {
                    // Update split ratio from mouse position
                    dock_node_t* dn = dock->divider.node;
                    if (dn) {
                        float new_ratio;
                        if (dn->type == DOCK_NODE_SPLIT_H) {
                            new_ratio = (float)(mx - dn->x) / (float)dn->w;
                        } else {
                            new_ratio = (float)(my - dn->y) / (float)dn->h;
                        }
                        if (new_ratio < 0.05f) new_ratio = 0.05f;
                        if (new_ratio > 0.95f) new_ratio = 0.95f;
                        dn->ratio = new_ratio;
                    }
                    continue; // consume
                }
                if (ev.type == DONG_APP_EVENT_MOUSE_BUTTON &&
                    ev.mouse_button.button == 1 &&
                    ev.mouse_button.pressed == 0) {
                    // Mouse up: end divider drag (keep last_click fields for double-click)
                    dock->divider.state = DOCK_DIV_IDLE;
                    dock->divider.node = NULL;
                    continue; // consume
                }
            }

            // Divider click detection (only when not tab-dragging and not divider-dragging)
            if (dock->drag.state == DOCK_DRAG_NONE &&
                dock->divider.state != DOCK_DIV_DRAGGING &&
                ev.type == DONG_APP_EVENT_MOUSE_BUTTON &&
                ev.mouse_button.button == 1 &&
                ev.mouse_button.pressed == 1)
            {
                dock_node_t* div_hit = win->root
                    ? dock_node_hit_test_divider(win->root, mx, my) : NULL;

                if (div_hit) {
                    // Double-click detection: reset ratio to 0.5
                    uint64_t now_ms = SDL_GetTicks();
                    if (dock->divider.last_click_node == div_hit &&
                        now_ms - dock->divider.last_click_time < 400) {
                        div_hit->ratio = 0.5f;
                        dock->divider.last_click_time = 0;
                        dock->divider.last_click_node = NULL;
                    } else {
                        // Start divider drag
                        dock->divider.state = DOCK_DIV_DRAGGING;
                        dock->divider.node = div_hit;
                        dock->divider.window_index = win->index;
                        dock->divider.last_click_time = now_ms;
                        dock->divider.last_click_node = div_hit;
                    }
                    continue; // consume
                }
            }

            // --- Handle active drag (mouse move / mouse up) ---
            // Uses global mouse state to hit-test ALL windows
            if (dock->drag.state != DOCK_DRAG_NONE) {
                if (ev.type == DONG_APP_EVENT_MOUSE_MOVE) {
                    dock_update_drag(dock);
                    continue; // consume
                }
                if (ev.type == DONG_APP_EVENT_MOUSE_BUTTON &&
                    ev.mouse_button.button == 1 &&
                    ev.mouse_button.pressed == 0) {
                    dock_update_drag(dock); // final position update
                    dock_finish_drag(dock);
                    continue; // consume
                }
            }

            dock_node_t* hit = win->root ? dock_node_hit_test(win->root, mx, my) : NULL;
            if (hit && hit->pane_count > 0) {
                int32_t local_y = my - hit->y;

                // --- Title bar area ---
                if (local_y < DOCK_TITLE_BAR_HEIGHT) {
                    if (ev.type == DONG_APP_EVENT_MOUSE_BUTTON &&
                        ev.mouse_button.button == 1 &&
                        ev.mouse_button.pressed == 1)
                    {
                        // Check window-level buttons first (top-right corner)
                        {
                            int32_t by = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
                            int32_t bx = (int32_t)win->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;

                            // Close button (rightmost)
                            if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                                my >= by && my < by + DOCK_BTN_SIZE) {
                                if (win->is_primary) {
                                    dock->running = 0;
                                    return 0;
                                } else {
                                    // Synthesize window close event
                                    SDL_Event close_ev;
                                    SDL_zero(close_ev);
                                    close_ev.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
                                    close_ev.window.windowID = SDL_GetWindowID(win->sdl_window);
                                    SDL_PushEvent(&close_ev);
                                }
                                continue;
                            }
                            bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);

                            // Maximize button
                            if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                                my >= by && my < by + DOCK_BTN_SIZE) {
                                Uint32 flags = SDL_GetWindowFlags(win->sdl_window);
                                if (flags & SDL_WINDOW_MAXIMIZED)
                                    SDL_RestoreWindow(win->sdl_window);
                                else
                                    SDL_MaximizeWindow(win->sdl_window);
                                continue;
                            }
                            bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);

                            // Minimize button
                            if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                                my >= by && my < by + DOCK_BTN_SIZE) {
                                SDL_MinimizeWindow(win->sdl_window);
                                continue;
                            }
                        }

                        // Check per-leaf close button
                        {
                            int32_t bx = hit->x + (int32_t)hit->w - DOCK_BTN_PAD - DOCK_BTN_SIZE;
                            int32_t by = hit->y + (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
                            if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                                my >= by && my < by + DOCK_BTN_SIZE) {
                                // Close active tab in this leaf
                                int pi = hit->pane_indices[hit->active_tab];
                                if (pi >= 0 && pi < dock->pane_count &&
                                    dock->panes[pi].alive) {
                                    dong_dock_remove_pane(dock, &dock->panes[pi]);
                                }
                                continue;
                            }
                        }

                        // Compute which tab was clicked
                        int32_t local_x = mx - hit->x - DOCK_TAB_PAD;
                        uint32_t tab_w = dock_tab_width(hit);
                        int tab_idx = -1;
                        if (local_x >= 0 && tab_w > 0) {
                            tab_idx = local_x / (int32_t)(tab_w + DOCK_TAB_PAD);
                            if (tab_idx >= hit->pane_count) tab_idx = -1;
                        }

                        if (tab_idx >= 0) {
                            // Activate tab
                            hit->active_tab = tab_idx;
                            int pi = hit->pane_indices[tab_idx];
                            dock->focused_pane_index = pi;

                            // Begin drag pending
                            dock->drag.state = DOCK_DRAG_PENDING;
                            dock->drag.source_pane_index = pi;
                            dock->drag.source_window_index = win->index;
                            dock->drag.source_tab_index = tab_idx;
                            dock->drag.start_x = mx;
                            dock->drag.start_y = my;
                            dock->drag.current_x = mx;
                            dock->drag.current_y = my;
                            dock->drag.hover_window_index = win->index;
                            dock->drag.drop_zone = DOCK_DROP_NONE;
                            // Record global mouse for cross-window threshold
                            {
                                float gx0, gy0;
                                SDL_GetGlobalMouseState(&gx0, &gy0);
                                dock->drag.start_global_x = (int32_t)gx0;
                                dock->drag.start_global_y = (int32_t)gy0;
                            }
                        } else if (!win->is_primary && win->root &&
                                   win->root->type == DOCK_NODE_LEAF &&
                                   win->root->pane_count == 1) {
                            // Non-primary single-pane window: clicking anywhere
                            // in the title bar initiates a dock drag so user can
                            // dock this detached pane into another window
                            int pi = hit->pane_indices[0];
                            dock->focused_pane_index = pi;

                            dock->drag.state = DOCK_DRAG_PENDING;
                            dock->drag.source_pane_index = pi;
                            dock->drag.source_window_index = win->index;
                            dock->drag.source_tab_index = 0;
                            dock->drag.start_x = mx;
                            dock->drag.start_y = my;
                            dock->drag.current_x = mx;
                            dock->drag.current_y = my;
                            dock->drag.hover_window_index = win->index;
                            dock->drag.drop_zone = DOCK_DROP_NONE;
                            {
                                float gx0, gy0;
                                SDL_GetGlobalMouseState(&gx0, &gy0);
                                dock->drag.start_global_x = (int32_t)gx0;
                                dock->drag.start_global_y = (int32_t)gy0;
                            }
                        }
                    }
                    continue; // title bar clicks don't go to engine
                }

                // --- Content area (below title bar) ---
                int pi = hit->pane_indices[hit->active_tab];
                if (pi >= 0 && pi < dock->pane_count && dock->panes[pi].alive) {
                    dong_dock_pane_t* target = &dock->panes[pi];

                    // Convert to pane-local coordinates (relative to content rect)
                    int32_t lx = mx - hit->x;
                    int32_t ly = my - hit->y - DOCK_TITLE_BAR_HEIGHT;

                    dong_app_event_t local = ev;
                    if (local.type == DONG_APP_EVENT_MOUSE_MOVE) {
                        local.mouse_move.x = lx;
                        local.mouse_move.y = ly;
                    } else if (local.type == DONG_APP_EVENT_MOUSE_BUTTON) {
                        local.mouse_button.x = lx;
                        local.mouse_button.y = ly;
                        dock->focused_pane_index = pi; // click focuses pane
                    } else {
                        local.mouse_wheel.x = lx;
                        local.mouse_wheel.y = ly;
                    }

                    dock_forward_event_to_engine(target->engine, &local);
                }
            }
        }
        // Key/text events: route to focused pane
        else if (ev.type == DONG_APP_EVENT_KEY ||
                 ev.type == DONG_APP_EVENT_TEXT ||
                 ev.type == DONG_APP_EVENT_TEXT_EDITING)
        {
            if (dock->focused_pane_index >= 0 &&
                dock->focused_pane_index < dock->pane_count &&
                dock->panes[dock->focused_pane_index].alive)
            {
                dock_forward_event_to_engine(
                    dock->panes[dock->focused_pane_index].engine, &ev);
            }
        }
    }

    // Frame timing
    uint64_t now  = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();
    dock->delta_time = (float)(now - dock->last_frame_time) / (float)freq;
    dock->last_frame_time = now;

    return dock->running;
}

// =============================================================================
// Rendering
// =============================================================================

// Recursively render all active panes in a window's split tree
static void dock_render_tree(dong_dock_t* dock, dock_node_t* node) {
    if (!node) return;

    if (node->type == DOCK_NODE_LEAF) {
        if (node->pane_count <= 0) return;
        int pi = node->pane_indices[node->active_tab];
        if (pi < 0 || pi >= dock->pane_count) return;
        dong_dock_pane_t* pane = &dock->panes[pi];
        if (!pane->alive || !pane->engine) return;

        // Content area = leaf area minus title bar
        int32_t cx, cy;
        uint32_t cw, ch;
        dock_leaf_content_rect(node, &cx, &cy, &cw, &ch);
        if (cw == 0) cw = 1;
        if (ch == 0) ch = 1;

        // Ensure offscreen texture sized to content region
        pane->offscreen_tex = dock_ensure_offscreen_texture(
            dock->gpu_device, pane->offscreen_tex,
            &pane->tex_w, &pane->tex_h, cw, ch);
        if (!pane->offscreen_tex) return;

        // Resize engine to content dimensions
        dong_engine_resize(pane->engine, cw, ch);

        // Offscreen render
        dong_gpu_begin_frame_offscreen(
            dock->driver, (DongGPUTexture)pane->offscreen_tex, cw, ch);
        dong_engine_tick(pane->engine);
        dong_gpu_end_frame_offscreen(dock->driver);
        return;
    }

    dock_render_tree(dock, node->children[0]);
    dock_render_tree(dock, node->children[1]);
}

// Blit all visible panes to swapchain
static void dock_blit_tree(dock_node_t* node, dong_dock_t* dock,
                            SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain) {
    if (!node) return;

    if (node->type == DOCK_NODE_LEAF) {
        if (node->pane_count <= 0) return;

        // --- Title bar background ---
        dock_blit_solid(cmd, dock->tex_tab_bg, swapchain,
                        node->x, node->y,
                        node->w, DOCK_TITLE_BAR_HEIGHT);

        // --- Individual tab rects ---
        uint32_t tab_w = dock_tab_width(node);
        for (int t = 0; t < node->pane_count; t++) {
            int pi = node->pane_indices[t];
            if (pi < 0 || pi >= dock->pane_count) continue;

            int32_t tx = node->x + DOCK_TAB_PAD + (int32_t)(t * (tab_w + DOCK_TAB_PAD));
            int32_t ty = node->y + 2;
            uint32_t th = DOCK_TITLE_BAR_HEIGHT - 4;

            SDL_GPUTexture* tab_tex = (t == node->active_tab)
                ? dock->tex_tab_active
                : dock->tex_tab_bg;
            dock_blit_solid(cmd, tab_tex, swapchain, tx, ty, tab_w, th);

            // Blit title text centered vertically in the tab
            dong_dock_pane_t* tab_pane = &dock->panes[pi];
            if (tab_pane->alive) {
                dock_render_title_texture(dock, tab_pane);
                if (tab_pane->title_tex && tab_pane->title_tex_w > 0) {
                    uint32_t text_w = tab_pane->title_tex_w;
                    uint32_t text_h = tab_pane->title_tex_h;
                    // Clamp text width to tab width with small padding
                    if (text_w > tab_w - 4) text_w = tab_w - 4;
                    int32_t text_x = tx + 2;
                    int32_t text_y = ty + (int32_t)(th - text_h) / 2;

                    SDL_GPUBlitInfo tblit;
                    SDL_zero(tblit);
                    tblit.source.texture = tab_pane->title_tex;
                    tblit.source.w       = text_w;
                    tblit.source.h       = text_h;
                    tblit.destination.texture = swapchain;
                    tblit.destination.x  = (uint32_t)text_x;
                    tblit.destination.y  = (uint32_t)text_y;
                    tblit.destination.w  = text_w;
                    tblit.destination.h  = text_h;
                    tblit.load_op        = SDL_GPU_LOADOP_LOAD;
                    tblit.filter         = SDL_GPU_FILTER_NEAREST;
                    SDL_BlitGPUTexture(cmd, &tblit);
                }
            }
        }

        // --- Per-leaf close button (right edge of title bar) ---
        {
            int32_t bx = node->x + (int32_t)node->w - DOCK_BTN_PAD - DOCK_BTN_SIZE;
            int32_t by = node->y + (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
            dock_blit_solid(cmd, dock->tex_btn_close, swapchain,
                            bx, by, DOCK_BTN_SIZE, DOCK_BTN_SIZE);
        }

        // --- Pane content (below title bar) ---
        int pi = node->pane_indices[node->active_tab];
        if (pi < 0 || pi >= dock->pane_count) return;
        dong_dock_pane_t* pane = &dock->panes[pi];
        if (!pane->alive || !pane->offscreen_tex) return;

        int32_t cx, cy;
        uint32_t cw, ch;
        dock_leaf_content_rect(node, &cx, &cy, &cw, &ch);

        SDL_GPUBlitInfo blit;
        SDL_zero(blit);
        blit.source.texture = pane->offscreen_tex;
        blit.source.w       = pane->tex_w;
        blit.source.h       = pane->tex_h;
        blit.destination.texture = swapchain;
        blit.destination.x  = (uint32_t)cx;
        blit.destination.y  = (uint32_t)cy;
        blit.destination.w  = cw;
        blit.destination.h  = ch;
        blit.load_op        = SDL_GPU_LOADOP_LOAD;
        blit.filter         = SDL_GPU_FILTER_LINEAR;
        SDL_BlitGPUTexture(cmd, &blit);
        return;
    }

    dock_blit_tree(node->children[0], dock, cmd, swapchain);
    dock_blit_tree(node->children[1], dock, cmd, swapchain);
}

// Blit window-level min/max/close buttons at top-right corner
static void dock_blit_window_buttons(dong_dock_t* dock,
    SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain,
    dong_dock_window_t* win)
{
    int32_t by = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
    int32_t bx = (int32_t)win->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;

    // Close button (rightmost, red)
    dock_blit_solid(cmd, dock->tex_btn_close, swapchain,
                    bx, by, DOCK_BTN_SIZE, DOCK_BTN_SIZE);
    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);

    // Maximize button
    dock_blit_solid(cmd, dock->tex_btn_maximize, swapchain,
                    bx, by, DOCK_BTN_SIZE, DOCK_BTN_SIZE);
    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);

    // Minimize button
    dock_blit_solid(cmd, dock->tex_btn_minimize, swapchain,
                    bx, by, DOCK_BTN_SIZE, DOCK_BTN_SIZE);
}

// Render divider lines between split children (makes dividers discoverable)
static void dock_blit_dividers(dock_node_t* node, dong_dock_t* dock,
                                SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain) {
    if (!node || node->type == DOCK_NODE_LEAF) return;

    // Render this node's divider
    if (node->type == DOCK_NODE_SPLIT_H) {
        int32_t dx = node->children[0]->x + (int32_t)node->children[0]->w - 1;
        dock_blit_solid(cmd, dock->tex_tab_active, swapchain,
                        dx, node->y, DOCK_DIVIDER_THICKNESS, node->h);
    } else { // SPLIT_V
        int32_t dy = node->children[0]->y + (int32_t)node->children[0]->h - 1;
        dock_blit_solid(cmd, dock->tex_tab_active, swapchain,
                        node->x, dy, node->w, DOCK_DIVIDER_THICKNESS);
    }

    // Recurse
    dock_blit_dividers(node->children[0], dock, cmd, swapchain);
    dock_blit_dividers(node->children[1], dock, cmd, swapchain);
}

// Blit drop indicator overlay during active drag
static void dock_blit_drop_indicator(dong_dock_t* dock,
    SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain,
    dong_dock_window_t* win)
{
    if (dock->drag.state != DOCK_DRAG_ACTIVE) return;
    if (dock->drag.hover_window_index != win->index) return;
    if (dock->drag.drop_zone == DOCK_DROP_NONE) return;
    if (dock->drag.target_w == 0 || dock->drag.target_h == 0) return;

    dock_blit_solid(cmd, dock->tex_drop_indicator, swapchain,
                    dock->drag.target_x, dock->drag.target_y,
                    dock->drag.target_w, dock->drag.target_h);
}

// Blit tab reorder insertion indicator (vertical line between tabs)
static void dock_blit_reorder_indicator(dong_dock_t* dock,
    SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain,
    dong_dock_window_t* win)
{
    if (dock->drag.state != DOCK_DRAG_ACTIVE) return;
    if (dock->drag.reorder_insert_pos < 0) return;
    if (dock->drag.hover_window_index != win->index) return;

    int src_pi = dock->drag.source_pane_index;
    if (src_pi < 0 || src_pi >= dock->pane_count) return;

    dock_node_t* leaf = dock_node_find_leaf(win->root, src_pi);
    if (!leaf || leaf->type != DOCK_NODE_LEAF || leaf->pane_count <= 1) return;

    uint32_t tab_w = dock_tab_width(leaf);
    int pos = dock->drag.reorder_insert_pos;

    // X position of the insertion line
    int32_t ix = leaf->x + DOCK_TAB_PAD + (int32_t)(pos * (tab_w + DOCK_TAB_PAD))
                 - (int32_t)(DOCK_TAB_PAD / 2);
    int32_t iy = leaf->y + 2;
    uint32_t ih = DOCK_TITLE_BAR_HEIGHT - 4;

    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    ix, iy, 3, ih);
}

// Blit focus border around the focused pane's leaf node
static void dock_blit_focus_border(dong_dock_t* dock,
    SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain,
    dong_dock_window_t* win)
{
    if (dock->focused_pane_index < 0) return;
    if (dock->focused_pane_index >= dock->pane_count) return;
    dong_dock_pane_t* fp = &dock->panes[dock->focused_pane_index];
    if (!fp->alive || fp->window_index != win->index) return;

    dock_node_t* leaf = dock_node_find_leaf(win->root, dock->focused_pane_index);
    if (!leaf) return;

    int32_t bw = DOCK_FOCUS_BORDER_W;
    int32_t nx = leaf->x, ny = leaf->y;
    uint32_t nw = leaf->w, nh = leaf->h;

    // Top edge
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    nx, ny, nw, (uint32_t)bw);
    // Bottom edge
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    nx, ny + (int32_t)nh - bw, nw, (uint32_t)bw);
    // Left edge
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    nx, ny + bw, (uint32_t)bw, nh - 2 * (uint32_t)bw);
    // Right edge
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    nx + (int32_t)nw - bw, ny + bw,
                    (uint32_t)bw, nh - 2 * (uint32_t)bw);
}

DONG_APPCORE_API void dong_dock_render(dong_dock_t* dock) {
    if (!dock) return;

    // For each alive window: layout, offscreen render all panes, composite
    for (int wi = 0; wi < dock->window_count; wi++) {
        dong_dock_window_t* wn = &dock->windows[wi];
        if (!wn->alive || !wn->sdl_window || !wn->root) continue;

        // Skip minimized windows
        Uint32 wflags = SDL_GetWindowFlags(wn->sdl_window);
        if (wflags & SDL_WINDOW_MINIMIZED) continue;

        // Re-query actual window size (handles restore from minimize)
        {
            int ww = 0, wh = 0;
            SDL_GetWindowSize(wn->sdl_window, &ww, &wh);
            if (ww > 0 && wh > 0) {
                wn->width  = (uint32_t)ww;
                wn->height = (uint32_t)wh;
            }
        }
        if (wn->width == 0 || wn->height == 0) continue;

        // Layout
        dock_node_layout(wn->root, 0, 0, wn->width, wn->height);

        // Offscreen render all panes
        dock_render_tree(dock, wn->root);

        // Composite to swapchain
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(dock->gpu_device);
        if (!cmd) continue;

        SDL_GPUTexture* swapchain = NULL;
        uint32_t sw = 0, sh = 0;
        if (!SDL_AcquireGPUSwapchainTexture(cmd, wn->sdl_window,
                                              &swapchain, &sw, &sh) || !swapchain) {
            SDL_CancelGPUCommandBuffer(cmd);
            continue;
        }

        // Clear
        {
            SDL_GPUColorTargetInfo cti;
            SDL_zero(cti);
            cti.texture  = swapchain;
            cti.load_op  = SDL_GPU_LOADOP_CLEAR;
            cti.store_op = SDL_GPU_STOREOP_STORE;
            cti.clear_color.r = 0.08f;
            cti.clear_color.g = 0.08f;
            cti.clear_color.b = 0.12f;
            cti.clear_color.a = 1.0f;
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &cti, 1, NULL);
            if (pass) SDL_EndGPURenderPass(pass);
        }

        // Blit panes
        dock_blit_tree(wn->root, dock, cmd, swapchain);

        // Blit divider lines between split children
        dock_blit_dividers(wn->root, dock, cmd, swapchain);

        // Window-level buttons (min/max/close at top-right)
        dock_blit_window_buttons(dock, cmd, swapchain, wn);

        // Focus border
        dock_blit_focus_border(dock, cmd, swapchain, wn);

        // Drop indicator (during drag)
        dock_blit_drop_indicator(dock, cmd, swapchain, wn);

        // Tab reorder indicator (during same-leaf drag)
        dock_blit_reorder_indicator(dock, cmd, swapchain, wn);

        SDL_SubmitGPUCommandBuffer(cmd);
    }
}

// =============================================================================
// Main Loop helpers
// =============================================================================

DONG_APPCORE_API int dong_dock_is_running(dong_dock_t* dock) {
    return dock ? dock->running : 0;
}

DONG_APPCORE_API float dong_dock_get_delta_time(dong_dock_t* dock) {
    return dock ? dock->delta_time : 0.016f;
}

DONG_APPCORE_API void dong_dock_run(dong_dock_t* dock,
                                     dong_dock_tick_fn tick, void* user_data) {
    if (!dock) return;
    while (dock->running) {
        if (!dong_dock_poll_events(dock)) break;
        float dt = dong_dock_get_delta_time(dock);
        if (tick) tick(dock, dt, user_data);
        dong_dock_render(dock);
    }
}

// =============================================================================
// Detach / Attach
// =============================================================================

DONG_APPCORE_API dong_dock_window_t* dong_dock_detach(
    dong_dock_t* dock, dong_dock_pane_t* pane,
    int window_x, int window_y,
    uint32_t window_w, uint32_t window_h)
{
    if (!dock || !pane || !pane->alive) return NULL;

    int pi = pane->index;
    uint32_t w = window_w ? window_w : (pane->tex_w ? pane->tex_w : 640);
    uint32_t h = window_h ? window_h : (pane->tex_h ? pane->tex_h : 480);

    // Remove from current window's tree
    dong_dock_window_t* old_win = &dock->windows[pane->window_index];
    if (old_win->alive && old_win->root) {
        dock_node_remove_pane(old_win->root, pi);
        // If root became empty leaf with 0 panes, clean up
        if (old_win->root->type == DOCK_NODE_LEAF && old_win->root->pane_count == 0) {
            dock_node_free(old_win->root);
            old_win->root = NULL;
            // Destroy empty secondary window
            if (!old_win->is_primary) {
                SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, old_win->sdl_window);
                SDL_DestroyWindow(old_win->sdl_window);
                old_win->sdl_window = NULL;
                old_win->alive = 0;
            }
        }
    }

    // Create new SDL window (borderless)
    SDL_Window* sdl_win = SDL_CreateWindow(
        pane->title[0] ? pane->title : "Detached",
        w, h, SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
    if (!sdl_win) {
        fprintf(stderr, "[DongDock] Failed to create detach window: %s\n", SDL_GetError());
        return NULL;
    }

    if (window_x || window_y)
        SDL_SetWindowPosition(sdl_win, window_x, window_y);

    if (!SDL_ClaimWindowForGPUDevice(dock->gpu_device, sdl_win)) {
        fprintf(stderr, "[DongDock] ClaimWindow failed for detach: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_win);
        return NULL;
    }

    // Allocate window slot
    int wi = dock_alloc_window_slot(dock);
    if (wi < 0) {
        SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, sdl_win);
        SDL_DestroyWindow(sdl_win);
        return NULL;
    }

    dong_dock_window_t* nw = &dock->windows[wi];
    nw->alive = 1;
    nw->index = wi;
    nw->sdl_window = sdl_win;
    nw->width = w;
    nw->height = h;
    nw->is_primary = 0;
    nw->root = dock_node_new_leaf(pi);

    // Update pane
    pane->window_index = wi;
    pane->node = nw->root;

    // Register hit-test for borderless window
    SDL_SetWindowHitTest(sdl_win, dock_hit_test_callback, dock);

    SDL_StartTextInput(sdl_win);

    return nw;
}

DONG_APPCORE_API void dong_dock_attach(
    dong_dock_t* dock, dong_dock_pane_t* pane,
    dong_dock_pane_t* target, dong_dock_edge_t edge)
{
    if (!dock || !pane || !target || !pane->alive || !target->alive) return;

    int pi = pane->index;
    int old_wi = pane->window_index;

    // Remove from old window
    dong_dock_window_t* old_win = &dock->windows[old_wi];
    if (old_win->alive && old_win->root) {
        dock_node_remove_pane(old_win->root, pi);
        if (old_win->root->type == DOCK_NODE_LEAF && old_win->root->pane_count == 0) {
            dock_node_free(old_win->root);
            old_win->root = NULL;
            // Destroy empty secondary window
            if (!old_win->is_primary) {
                SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, old_win->sdl_window);
                SDL_DestroyWindow(old_win->sdl_window);
                old_win->sdl_window = NULL;
                old_win->alive = 0;
            }
        }
    }

    // Insert into target's tree
    pane->window_index = target->window_index;
    dock_node_t* tleaf = dock_node_find_leaf(
        dock->windows[target->window_index].root, target->index);

    if (!tleaf) return; // target leaf not found (shouldn't happen)

    if (edge == DONG_DOCK_TAB) {
        dock_node_add_tab(tleaf, pi);
        pane->node = tleaf;
    } else {
        dock_node_t* nl = dock_node_split(tleaf, edge, pi, 0.5f);
        pane->node = nl;
        target->node = dock_node_find_leaf(
            dock->windows[target->window_index].root, target->index);
    }
}

// =============================================================================
// Pane Properties
// =============================================================================

DONG_APPCORE_API dong_engine_t* dong_dock_pane_get_engine(dong_dock_pane_t* pane) {
    return (pane && pane->alive) ? pane->engine : NULL;
}

DONG_APPCORE_API int dong_dock_pane_load_html(dong_dock_pane_t* pane, const char* html) {
    if (!pane || !pane->alive || !pane->engine || !html) return 0;
    return (dong_engine_load_html(pane->engine, html) == DONG_OK) ? 1 : 0;
}

DONG_APPCORE_API int dong_dock_pane_load_html_file(dong_dock_pane_t* pane, const char* path) {
    if (!pane || !pane->alive || !pane->engine || !path) return 0;

    char dir[1024];
    dock_extract_dir(path, dir, sizeof(dir));
    if (dir[0]) dong_engine_set_resource_root(pane->engine, dir);

    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(sz + 1);
    if (!buf) { fclose(f); return 0; }
    size_t rd = fread(buf, 1, sz, f);
    buf[rd] = 0;
    fclose(f);
    int ok = dong_dock_pane_load_html(pane, buf);
    free(buf);
    return ok;
}

DONG_APPCORE_API void dong_dock_pane_set_title(dong_dock_pane_t* pane, const char* title) {
    if (!pane || !pane->alive) return;
    if (title) {
        strncpy(pane->title, title, DOCK_MAX_TITLE - 1);
        pane->title[DOCK_MAX_TITLE - 1] = 0;
    }
}

DONG_APPCORE_API const char* dong_dock_pane_get_title(dong_dock_pane_t* pane) {
    return (pane && pane->alive) ? pane->title : "";
}

DONG_APPCORE_API void dong_dock_pane_set_resource_root(dong_dock_pane_t* pane, const char* root) {
    if (!pane || !pane->alive || !pane->engine || !root) return;
    strncpy(pane->resource_root, root, sizeof(pane->resource_root) - 1);
    dong_engine_set_resource_root(pane->engine, root);
}

DONG_APPCORE_API int dong_dock_pane_eval_script(dong_dock_pane_t* pane, const char* script) {
    if (!pane || !pane->alive || !pane->engine || !script) return 0;
    return (dong_engine_eval_script(pane->engine, script) == DONG_OK) ? 1 : 0;
}

// =============================================================================
// Split Ratio
// =============================================================================

DONG_APPCORE_API void dong_dock_set_split_ratio(
    dong_dock_t* dock, dong_dock_pane_t* pane, float ratio)
{
    if (!dock || !pane || !pane->alive) return;
    dock_node_t* leaf = pane->node;
    if (!leaf) return;
    dock_node_t* parent = leaf->parent;
    if (!parent || parent->type == DOCK_NODE_LEAF) return;
    if (ratio < 0.05f) ratio = 0.05f;
    if (ratio > 0.95f) ratio = 0.95f;
    parent->ratio = ratio;
}

// =============================================================================
// Window Access
// =============================================================================

DONG_APPCORE_API int dong_dock_get_window_count(dong_dock_t* dock) {
    if (!dock) return 0;
    int n = 0;
    for (int i = 0; i < dock->window_count; i++)
        if (dock->windows[i].alive) n++;
    return n;
}

DONG_APPCORE_API dong_dock_window_t* dong_dock_get_window(dong_dock_t* dock, int index) {
    if (!dock || index < 0 || index >= dock->window_count) return NULL;
    return dock->windows[index].alive ? &dock->windows[index] : NULL;
}

DONG_APPCORE_API dong_dock_window_t* dong_dock_get_primary_window(dong_dock_t* dock) {
    return dock ? &dock->windows[0] : NULL;
}

DONG_APPCORE_API void* dong_dock_window_get_native(dong_dock_window_t* win) {
    return (win && win->alive) ? win->sdl_window : NULL;
}

// =============================================================================
// Callbacks
// =============================================================================

DONG_APPCORE_API void dong_dock_set_pane_close_callback(
    dong_dock_t* dock, dong_dock_pane_close_fn cb, void* ud)
{
    if (!dock) return;
    dock->pane_close_cb = cb;
    dock->pane_close_ud = ud;
}

DONG_APPCORE_API void dong_dock_set_window_close_callback(
    dong_dock_t* dock, dong_dock_window_close_fn cb, void* ud)
{
    if (!dock) return;
    dock->window_close_cb = cb;
    dock->window_close_ud = ud;
}

// =============================================================================
// Layout Persistence (stubs - P1)
// =============================================================================

DONG_APPCORE_API int dong_dock_save_layout(dong_dock_t* dock, const char* path) {
    (void)dock; (void)path;
    fprintf(stderr, "[DongDock] save_layout: not yet implemented\n");
    return 0;
}

DONG_APPCORE_API int dong_dock_load_layout(dong_dock_t* dock, const char* path,
                                            dong_dock_pane_created_fn on_pane,
                                            void* user_data) {
    (void)dock; (void)path; (void)on_pane; (void)user_data;
    fprintf(stderr, "[DongDock] load_layout: not yet implemented\n");
    return 0;
}
