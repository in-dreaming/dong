#ifndef DOCK_INTERNAL_H
#define DOCK_INTERNAL_H

#include "dong_dock.h"
#include "dong_gpu_driver.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOCK_MAX_TITLE 128

// =============================================================================
// Title bar / tab bar constants
// =============================================================================

#define DOCK_TITLE_BAR_HEIGHT  36
#define DOCK_TAB_MIN_WIDTH     80
#define DOCK_TAB_MAX_WIDTH     240
#define DOCK_TAB_PAD           4
#define DOCK_DRAG_THRESHOLD    8
#define DOCK_DROP_EDGE_FRAC    0.25f
#define DOCK_FOCUS_BORDER_W    2

// Per-tab close button constants
#define DOCK_TAB_CLOSE_SIZE    14
#define DOCK_TAB_CLOSE_PAD     4

// Window control button constants
#define DOCK_BTN_SIZE          24
#define DOCK_BTN_PAD           6
#define DOCK_RESIZE_BORDER     6

// Window-edge detection band for window-level drops
#define DOCK_WIN_EDGE_BAND     40

// Divider hit-test constants
#define DOCK_DIVIDER_HALF_W    4   // 卤4px hit zone from divider center
#define DOCK_DIVIDER_THICKNESS 4   // Visual divider line thickness
#define DOCK_MIN_PANE_SIZE     120 // Minimum pane dimension in pixels

// =============================================================================
// Divider drag state
// =============================================================================

typedef enum {
    DOCK_DIV_IDLE,
    DOCK_DIV_HOVER,
    DOCK_DIV_DRAGGING,
} dock_divider_state_t;

typedef struct {
    dock_divider_state_t state;
    struct dock_node_t* node;         // The split node whose divider is being dragged
    int window_index;          // Which window the divider belongs to
    uint64_t last_click_time;  // For double-click detection (ms)
    struct dock_node_t* last_click_node; // Node clicked last time (for double-click)
} dock_divider_drag_t;

// =============================================================================
// Hover state (for visual feedback)
// =============================================================================

typedef enum {
    DOCK_HOVER_NONE,
    DOCK_HOVER_TAB,            // Hovering a tab (not over its close button)
    DOCK_HOVER_TAB_CLOSE,      // Hovering a tab's close button
    DOCK_HOVER_WIN_CLOSE,      // Hovering window close button
    DOCK_HOVER_WIN_MAXIMIZE,   // Hovering window maximize button
    DOCK_HOVER_WIN_MINIMIZE,   // Hovering window minimize button
    DOCK_HOVER_DIVIDER,        // Hovering a split divider
} dock_hover_type_t;

typedef struct {
    dock_hover_type_t type;
    int window_index;          // Which window
    struct dock_node_t* node;  // Which leaf or split node
    int tab_index;             // Which tab (for TAB / TAB_CLOSE)
} dock_hover_t;

// =============================================================================
// Drag state
// =============================================================================

typedef enum {
    DOCK_DRAG_NONE,
    DOCK_DRAG_PENDING,
    DOCK_DRAG_ACTIVE,
} dock_drag_state_t;

typedef enum {
    DOCK_DROP_NONE,
    DOCK_DROP_TAB,
    DOCK_DROP_LEFT,
    DOCK_DROP_RIGHT,
    DOCK_DROP_TOP,
    DOCK_DROP_BOTTOM,
    DOCK_DROP_WIN_LEFT,    // Window-level edges
    DOCK_DROP_WIN_RIGHT,
    DOCK_DROP_WIN_TOP,
    DOCK_DROP_WIN_BOTTOM,
} dock_drop_zone_t;

typedef struct {
    dock_drag_state_t state;
    int source_pane_index;      // Pane being dragged
    int source_window_index;    // Window it came from
    int source_tab_index;       // Tab index within its leaf at drag start
    int32_t start_x, start_y;  // Mouse position at drag start (window-local)
    int32_t start_global_x, start_global_y; // Global mouse at drag start
    int32_t current_x, current_y; // Current mouse position (window-local)
    int hover_window_index;     // Window mouse is over (-1 = none)
    dock_drop_zone_t drop_zone;
    // Computed target for rendering drop indicators
    int32_t target_x, target_y;
    uint32_t target_w, target_h;
    // Tab reorder within same leaf
    int reorder_insert_pos;     // -1 = not reordering, else insertion index
} dock_drag_t;

// =============================================================================
// Split tree node
// =============================================================================

typedef enum {
    DOCK_NODE_LEAF,
    DOCK_NODE_SPLIT_H,  // Horizontal split (left | right)
    DOCK_NODE_SPLIT_V,  // Vertical split (top / bottom)
} dock_node_type_t;

typedef struct dock_node_t {
    dock_node_type_t type;
    float ratio;                    // 0.0-1.0, left/top fraction
    struct dock_node_t* parent;

    union {
        struct {                    // SPLIT_H / SPLIT_V
            struct dock_node_t* children[2];
        };
        struct {                    // LEAF
            int* pane_indices;      // Tab group (>=1 pane index)
            int pane_count;
            int pane_cap;
            int active_tab;
        };
    };

    // Layout result (computed by dock_node_layout)
    int32_t x, y;
    uint32_t w, h;
} dock_node_t;

// =============================================================================
// Pane
// =============================================================================

struct dong_dock_pane_t {
    int alive;
    int index;
    dong_engine_t* engine;
    SDL_GPUTexture* offscreen_tex;
    uint32_t tex_w, tex_h;
    char title[DOCK_MAX_TITLE];
    char resource_root[1024];
    dock_node_t* node;              // Which leaf node contains this pane
    int window_index;               // Which window this pane belongs to

    // Cached title texture (bitmap font rendered)
    SDL_GPUTexture* title_tex;
    uint32_t title_tex_w, title_tex_h;
    char title_rendered[DOCK_MAX_TITLE]; // Last rendered title (dirty check)
};

// =============================================================================
// Window
// =============================================================================

struct dong_dock_window_t {
    int alive;
    int index;
    SDL_Window* sdl_window;
    dock_node_t* root;
    uint32_t width, height;
    int is_primary;
};

// =============================================================================
// Dock
// =============================================================================

struct dong_dock_t {
    SDL_GPUDevice* gpu_device;
    DongGPUDriver* driver;
    int running;
    float delta_time;
    uint64_t last_frame_time;

    dong_dock_window_t* windows;
    int window_count, window_cap;

    dong_dock_pane_t* panes;
    int pane_count, pane_cap;

    int focused_pane_index;         // -1 = none

    dong_dock_pane_close_fn pane_close_cb;
    void* pane_close_ud;
    dong_dock_window_close_fn window_close_cb;
    void* window_close_ud;

    // Primary window reference (always windows[0])
    SDL_Window* primary_window;

    // Chrome textures (1x1 solid colors, stretched via blit)
    SDL_GPUTexture* tex_tab_bg;         // #2b2b2b
    SDL_GPUTexture* tex_tab_active;     // #3c3c3c
    SDL_GPUTexture* tex_drop_indicator; // #224488
    SDL_GPUTexture* tex_focus_border;   // #0078d4
    SDL_GPUTexture* tex_btn_close;      // #c42b1c (red)
    SDL_GPUTexture* tex_btn_maximize;   // #555555
    SDL_GPUTexture* tex_btn_minimize;   // #555555
    SDL_GPUTexture* tex_btn_hover;      // #444444 (hover highlight)
    SDL_GPUTexture* tex_divider;        // #555555 (divider default)
    SDL_GPUTexture* tex_divider_hover;  // #0078d4 (divider hover/drag)
    SDL_GPUTexture* tex_close_hover_bg; // #c42b1c80 (tab close hover bg)

    // Drag state
    dock_drag_t drag;

    // Hover state (for visual feedback)
    dock_hover_t hover;

    // Divider drag state
    dock_divider_drag_t divider;

    // Cursors for divider resize
    SDL_Cursor* cursor_default;
    SDL_Cursor* cursor_ew_resize;  // For SPLIT_H dividers (vertical line)
    SDL_Cursor* cursor_ns_resize;  // For SPLIT_V dividers (horizontal line)
};

// =============================================================================
// Content rect helper (leaf area below title bar)
// =============================================================================

static inline void dock_leaf_content_rect(const dock_node_t* leaf,
    int32_t* cx, int32_t* cy, uint32_t* cw, uint32_t* ch)
{
    *cx = leaf->x;
    *cy = leaf->y + DOCK_TITLE_BAR_HEIGHT;
    *cw = leaf->w;
    *ch = (leaf->h > (uint32_t)DOCK_TITLE_BAR_HEIGHT)
        ? leaf->h - DOCK_TITLE_BAR_HEIGHT : 1;
}

// =============================================================================
// =============================================================================
// Dock drag helpers (dock_drag.c)
// =============================================================================

uint32_t dock_tab_width(const dock_node_t* node);
void dock_update_drag(dong_dock_t* dock);
void dock_finish_drag(dong_dock_t* dock);

// Dock window helpers (dock_window.c)
// =============================================================================

int dock_alloc_pane_slot(dong_dock_t* dock);
int dock_alloc_window_slot(dong_dock_t* dock);
dong_dock_window_t* dock_find_window_by_sdl_id(dong_dock_t* dock, SDL_WindowID wid);
void dock_window_remove_pane_index(dong_dock_t* dock, dong_dock_window_t* win, int pane_index);
SDL_HitTestResult SDLCALL dock_hit_test_callback(
    SDL_Window* win, const SDL_Point* area, void* data);

// Dock pane orchestration helpers (dock_pane.c)
// =============================================================================

dong_dock_pane_t* dock_add_pane_impl(
    dong_dock_t* dock, const dong_dock_pane_config_t* config);
dong_dock_pane_t* dock_split_impl(
    dong_dock_t* dock, dong_dock_pane_t* neighbor,
    dong_dock_edge_t edge, float ratio,
    const dong_dock_pane_config_t* config);

// Dock lifecycle helpers (dock_lifecycle.c)
// =============================================================================

dong_engine_t* dock_create_engine(dong_dock_t* dock,
                                  uint32_t w, uint32_t h,
                                  dong_engine_t* shared_js,
                                  const char* view_name);

// Dock title rendering (dock_title.c)
// =============================================================================

void dock_render_title_texture(dong_dock_t* dock, dong_dock_pane_t* pane);

// =============================================================================
// Split tree functions (dock_split.c)
// =============================================================================

dock_node_t* dock_node_new_leaf(int pane_index);
void dock_node_free(dock_node_t* node);
dock_node_t* dock_node_split(dock_node_t* leaf, dong_dock_edge_t edge,
                              int new_pane_index, float ratio);
dock_node_t* dock_node_split_root(dock_node_t** root_ptr, dong_dock_edge_t edge,
                                   int new_pane_index, float ratio);
int dock_node_remove_pane(dock_node_t* node, int pane_index);
void dock_node_add_tab(dock_node_t* leaf, int pane_index);
void dock_node_layout(dock_node_t* node, int32_t x, int32_t y, uint32_t w, uint32_t h);
dock_node_t* dock_node_find_leaf(dock_node_t* root, int pane_index);
dock_node_t* dock_node_hit_test(dock_node_t* root, int32_t mx, int32_t my);
dock_node_t* dock_node_hit_test_divider(dock_node_t* root, int32_t mx, int32_t my);

#ifdef __cplusplus
}
#endif

#endif // DOCK_INTERNAL_H
