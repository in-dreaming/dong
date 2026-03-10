// dock_render.c - Dock rendering pipeline and composition

#include "dock_internal.h"

#include <string.h>

// =============================================================================
// Render Helpers
// =============================================================================

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

// =============================================================================
// Rendering
// =============================================================================

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
                            SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain,
                            dong_dock_window_t* win) {
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

            // Choose tab background: active > dragging(dim) > hover > default
            int is_drag_source = (dock->drag.state == DOCK_DRAG_ACTIVE &&
                                  pi == dock->drag.source_pane_index);
            int is_hovered = (dock->hover.node == node && dock->hover.tab_index == t &&
                              (dock->hover.type == DOCK_HOVER_TAB ||
                               dock->hover.type == DOCK_HOVER_TAB_CLOSE));
            SDL_GPUTexture* tab_tex;
            if (is_drag_source)
                tab_tex = dock->tex_tab_bg;  // dimmed: same as bar bg
            else if (t == node->active_tab)
                tab_tex = dock->tex_tab_active;
            else if (is_hovered)
                tab_tex = dock->tex_btn_hover;
            else
                tab_tex = dock->tex_tab_bg;
            dock_blit_solid(cmd, tab_tex, swapchain, tx, ty, tab_w, th);

            // Per-tab close button (right edge of tab)
            // Skip if overlapping window-level buttons area (top-right corner)
            {
                int32_t cbx = tx + (int32_t)tab_w - DOCK_TAB_CLOSE_PAD - DOCK_TAB_CLOSE_SIZE;
                int32_t cby = ty + (int32_t)(th - DOCK_TAB_CLOSE_SIZE) / 2;

                // Window buttons occupy rightmost ~(3*(BTN_SIZE+BTN_PAD)+BTN_PAD) px at y=0
                int skip_close = 0;
                if (node->y == 0) {
                    int32_t win_btn_left = (int32_t)win->width
                        - 3 * (DOCK_BTN_SIZE + DOCK_BTN_PAD) - DOCK_BTN_PAD;
                    if (cbx + DOCK_TAB_CLOSE_SIZE > win_btn_left)
                        skip_close = 1;
                }

                if (!skip_close && !is_drag_source) {
                    // Highlight close button on hover
                    int close_hovered = (dock->hover.node == node &&
                                         dock->hover.tab_index == t &&
                                         dock->hover.type == DOCK_HOVER_TAB_CLOSE);
                    SDL_GPUTexture* close_tex = close_hovered
                        ? dock->tex_close_hover_bg
                        : dock->tex_btn_close;
                    dock_blit_solid(cmd, close_tex, swapchain,
                                    cbx, cby, DOCK_TAB_CLOSE_SIZE, DOCK_TAB_CLOSE_SIZE);
                }
            }

            // Blit title text centered vertically in the tab
            // Leave room for close button on the right
            // Skip text for drag source tab (dimmed)
            dong_dock_pane_t* tab_pane = &dock->panes[pi];
            if (tab_pane->alive && !is_drag_source) {
                dock_render_title_texture(dock, tab_pane);
                if (tab_pane->title_tex && tab_pane->title_tex_w > 0) {
                    uint32_t text_w = tab_pane->title_tex_w;
                    uint32_t text_h = tab_pane->title_tex_h;
                    // Clamp text to tab width minus close button area
                    uint32_t max_text_w = tab_w - DOCK_TAB_CLOSE_SIZE - DOCK_TAB_CLOSE_PAD - 4;
                    if (text_w > max_text_w) text_w = max_text_w;
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

    dock_blit_tree(node->children[0], dock, cmd, swapchain, win);
    dock_blit_tree(node->children[1], dock, cmd, swapchain, win);
}

// Blit window-level min/max/close buttons at top-right corner
static void dock_blit_window_buttons(dong_dock_t* dock,
    SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain,
    dong_dock_window_t* win)
{
    int32_t by = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
    int32_t bx = (int32_t)win->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;
    int is_hover_win = (dock->hover.window_index == win->index);

    // Close button (rightmost, red)
    SDL_GPUTexture* close_tex = (is_hover_win && dock->hover.type == DOCK_HOVER_WIN_CLOSE)
        ? dock->tex_close_hover_bg : dock->tex_btn_close;
    dock_blit_solid(cmd, close_tex, swapchain,
                    bx, by, DOCK_BTN_SIZE, DOCK_BTN_SIZE);
    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);

    // Maximize button
    SDL_GPUTexture* max_tex = (is_hover_win && dock->hover.type == DOCK_HOVER_WIN_MAXIMIZE)
        ? dock->tex_btn_hover : dock->tex_btn_maximize;
    dock_blit_solid(cmd, max_tex, swapchain,
                    bx, by, DOCK_BTN_SIZE, DOCK_BTN_SIZE);
    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);

    // Minimize button
    SDL_GPUTexture* min_tex = (is_hover_win && dock->hover.type == DOCK_HOVER_WIN_MINIMIZE)
        ? dock->tex_btn_hover : dock->tex_btn_minimize;
    dock_blit_solid(cmd, min_tex, swapchain,
                    bx, by, DOCK_BTN_SIZE, DOCK_BTN_SIZE);
}

// Render divider lines between split children (makes dividers discoverable)
static void dock_blit_dividers(dock_node_t* node, dong_dock_t* dock,
                                SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain) {
    if (!node || node->type == DOCK_NODE_LEAF) return;

    // Choose divider color: hover/dragging = accent, default = gray
    int is_active = (dock->divider.state == DOCK_DIV_DRAGGING && dock->divider.node == node) ||
                    (dock->hover.type == DOCK_HOVER_DIVIDER && dock->hover.node == node);
    SDL_GPUTexture* div_tex = is_active ? dock->tex_divider_hover : dock->tex_divider;

    // Render this node's divider
    if (node->type == DOCK_NODE_SPLIT_H) {
        int32_t dx = node->children[0]->x + (int32_t)node->children[0]->w
                     - (int32_t)(DOCK_DIVIDER_THICKNESS / 2);
        dock_blit_solid(cmd, div_tex, swapchain,
                        dx, node->y, DOCK_DIVIDER_THICKNESS, node->h);
    } else { // SPLIT_V
        int32_t dy = node->children[0]->y + (int32_t)node->children[0]->h
                     - (int32_t)(DOCK_DIVIDER_THICKNESS / 2);
        dock_blit_solid(cmd, div_tex, swapchain,
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

// Blit drag ghost: a small tab-like rectangle at the mouse position during active drag
static void dock_blit_drag_ghost(dong_dock_t* dock,
    SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain,
    dong_dock_window_t* win)
{
    if (dock->drag.state != DOCK_DRAG_ACTIVE) return;
    int src_pi = dock->drag.source_pane_index;
    if (src_pi < 0 || src_pi >= dock->pane_count) return;
    dong_dock_pane_t* pane = &dock->panes[src_pi];
    if (!pane->alive) return;

    // For detached windows being moved, don't render ghost
    if (dock->drag.source_window_index >= 0 &&
        dock->drag.source_window_index < dock->window_count) {
        dong_dock_window_t* sw = &dock->windows[dock->drag.source_window_index];
        if (sw->alive && !sw->is_primary && sw->root &&
            sw->root->type == DOCK_NODE_LEAF && sw->root->pane_count == 1)
            return;
    }

    // Only render on the window the mouse is over (or source window if none)
    int target_wi = dock->drag.hover_window_index;
    if (target_wi < 0) target_wi = dock->drag.source_window_index;
    if (target_wi != win->index) return;

    // Ghost position: convert global mouse to window-local
    float gx, gy;
    SDL_GetGlobalMouseState(&gx, &gy);
    int wx, wy;
    SDL_GetWindowPosition(win->sdl_window, &wx, &wy);
    int32_t ghost_x = (int32_t)gx - wx - 40; // offset left of cursor
    int32_t ghost_y = (int32_t)gy - wy - 10;  // slightly above cursor
    uint32_t ghost_w = 120;
    uint32_t ghost_h = DOCK_TITLE_BAR_HEIGHT - 4;

    // Clamp to window bounds
    if (ghost_x < 0) ghost_x = 0;
    if (ghost_y < 0) ghost_y = 0;
    if (ghost_x + (int32_t)ghost_w > (int32_t)win->width)
        ghost_x = (int32_t)win->width - (int32_t)ghost_w;
    if (ghost_y + (int32_t)ghost_h > (int32_t)win->height)
        ghost_y = (int32_t)win->height - (int32_t)ghost_h;

    // Ghost tab background
    dock_blit_solid(cmd, dock->tex_tab_active, swapchain,
                    ghost_x, ghost_y, ghost_w, ghost_h);

    // Ghost border (1px accent color frame)
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    ghost_x, ghost_y, ghost_w, 1);
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    ghost_x, ghost_y + (int32_t)ghost_h - 1, ghost_w, 1);
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    ghost_x, ghost_y, 1, ghost_h);
    dock_blit_solid(cmd, dock->tex_focus_border, swapchain,
                    ghost_x + (int32_t)ghost_w - 1, ghost_y, 1, ghost_h);

    // Ghost title text
    dock_render_title_texture(dock, pane);
    if (pane->title_tex && pane->title_tex_w > 0) {
        uint32_t text_w = pane->title_tex_w;
        uint32_t text_h = pane->title_tex_h;
        if (text_w > ghost_w - 8) text_w = ghost_w - 8;
        int32_t text_x = ghost_x + 4;
        int32_t text_y = ghost_y + (int32_t)(ghost_h - text_h) / 2;

        SDL_GPUBlitInfo tblit;
        SDL_zero(tblit);
        tblit.source.texture = pane->title_tex;
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
        dock_blit_tree(wn->root, dock, cmd, swapchain, wn);

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

        // Drag ghost (floating tab at mouse during active drag)
        dock_blit_drag_ghost(dock, cmd, swapchain, wn);

        SDL_SubmitGPUCommandBuffer(cmd);
    }
}
