// dock_pane_api.c - Pane-level Dock API helpers

#include "dock_internal.h"

#include <stdlib.h>
#include <string.h>

int dock_alloc_pane_slot(dong_dock_t* dock) {
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

DONG_APPCORE_API void dong_dock_remove_pane(dong_dock_t* dock, dong_dock_pane_t* pane) {
    if (!dock || !pane || !pane->alive) return;
    int pi = pane->index;

    // Callback veto
    if (dock->pane_close_cb) {
        if (!dock->pane_close_cb(pane, dock->pane_close_ud))
            return; // vetoed
    }

    // Remove from split tree
    dong_dock_window_t* wn = &dock->windows[pane->window_index];
    dock_window_remove_pane_index(dock, wn, pi);

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
