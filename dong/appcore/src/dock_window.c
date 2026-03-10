// dock_window.c - Dock window slot and lookup helpers

#include "dock_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Find a leaf title bar region that should trigger OS draggable hit-test.
static dock_node_t* dock_find_title_bar_draggable(
    dock_node_t* node, int32_t mx, int32_t my, const dong_dock_t* dock)
{
    if (!node) return NULL;

    if (node->type == DOCK_NODE_LEAF) {
        if (mx < node->x || mx >= node->x + (int32_t)node->w) return NULL;
        if (my < node->y || my >= node->y + DOCK_TITLE_BAR_HEIGHT) return NULL;

        // Detached single-pane window: leave dragging to dock system.
        {
            dong_dock_window_t* dw = NULL;
            for (int i = 0; i < dock->window_count; i++) {
                if (!dock->windows[i].alive) continue;
                if (dock->windows[i].root) {
                    dock_node_t* found = dock_node_hit_test(dock->windows[i].root, mx, my);
                    if (found == node) {
                        dw = &dock->windows[i];
                        break;
                    }
                }
            }
            if (dw && !dw->is_primary && dw->root &&
                dw->root->type == DOCK_NODE_LEAF && dw->root->pane_count == 1) {
                int32_t bx = (int32_t)dw->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;
                int32_t by = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
                for (int b = 0; b < 3; b++) {
                    if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                        my >= by && my < by + DOCK_BTN_SIZE) {
                        return NULL;
                    }
                    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);
                }
                return NULL;
            }
        }

        int32_t local_x = mx - node->x - DOCK_TAB_PAD;
        uint32_t tab_w = dock_tab_width(node);
        if (tab_w > 0 && local_x >= 0) {
            int tab_idx = local_x / (int32_t)(tab_w + DOCK_TAB_PAD);
            if (tab_idx < node->pane_count) return NULL;
        }

        if (node->y == 0) {
            dong_dock_window_t* dw = NULL;
            for (int i = 0; i < dock->window_count; i++) {
                if (!dock->windows[i].alive) continue;
                if (dock->windows[i].root) {
                    dock_node_t* found = dock_node_hit_test(dock->windows[i].root, mx, my);
                    if (found == node) {
                        dw = &dock->windows[i];
                        break;
                    }
                }
            }
            if (dw) {
                int32_t bx = (int32_t)dw->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;
                int32_t by = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
                for (int b = 0; b < 3; b++) {
                    if (mx >= bx && mx < bx + DOCK_BTN_SIZE &&
                        my >= by && my < by + DOCK_BTN_SIZE) {
                        return NULL;
                    }
                    bx -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);
                }
            }
        }

        return node;
    }

    dock_node_t* r = dock_find_title_bar_draggable(node->children[0], mx, my, dock);
    if (r) return r;
    return dock_find_title_bar_draggable(node->children[1], mx, my, dock);
}

SDL_HitTestResult SDLCALL dock_hit_test_callback(
    SDL_Window* win, const SDL_Point* area, void* data)
{
    dong_dock_t* dock = (dong_dock_t*)data;
    if (!dock) return SDL_HITTEST_NORMAL;

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

    int at_left = mx < border;
    int at_right = mx >= w - border;
    int at_top = my < border;
    int at_bottom = my >= h - border;

    if (at_top && at_left) return SDL_HITTEST_RESIZE_TOPLEFT;
    if (at_top && at_right) return SDL_HITTEST_RESIZE_TOPRIGHT;
    if (at_bottom && at_left) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
    if (at_bottom && at_right) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
    if (at_left) return SDL_HITTEST_RESIZE_LEFT;
    if (at_right) return SDL_HITTEST_RESIZE_RIGHT;
    if (at_top) return SDL_HITTEST_RESIZE_TOP;
    if (at_bottom) return SDL_HITTEST_RESIZE_BOTTOM;

    if (dw->root) {
        dock_node_t* div_hit = dock_node_hit_test_divider(dw->root, mx, my);
        if (div_hit) {
            SDL_SetCursor(div_hit->type == DOCK_NODE_SPLIT_H
                              ? dock->cursor_ew_resize
                              : dock->cursor_ns_resize);
            return SDL_HITTEST_NORMAL;
        }
    }

    SDL_SetCursor(dock->cursor_default);

    if (dw->root) {
        dock_node_t* draggable = dock_find_title_bar_draggable(dw->root, mx, my, dock);
        if (draggable) return SDL_HITTEST_DRAGGABLE;
    }

    return SDL_HITTEST_NORMAL;
}

int dock_alloc_window_slot(dong_dock_t* dock) {
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

dong_dock_window_t* dock_find_window_by_sdl_id(dong_dock_t* dock, SDL_WindowID wid) {
    for (int i = 0; i < dock->window_count; i++) {
        if (!dock->windows[i].alive) continue;
        if (SDL_GetWindowID(dock->windows[i].sdl_window) == wid)
            return &dock->windows[i];
    }
    return NULL;
}

void dock_window_remove_pane_index(dong_dock_t* dock, dong_dock_window_t* win, int pane_index) {
    if (!dock || !win || !win->alive || !win->root) return;

    dock_node_remove_pane(win->root, pane_index);
    if (win->root->type == DOCK_NODE_LEAF && win->root->pane_count == 0) {
        dock_node_free(win->root);
        win->root = NULL;
        if (!win->is_primary && win->sdl_window) {
            SDL_ReleaseWindowFromGPUDevice(dock->gpu_device, win->sdl_window);
            SDL_DestroyWindow(win->sdl_window);
            win->sdl_window = NULL;
            win->alive = 0;
        }
    }
}

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
    dock_window_remove_pane_index(dock, old_win, pi);

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
    dock_window_remove_pane_index(dock, old_win, pi);

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
