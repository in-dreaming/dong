// dock_api_misc.c - Lightweight Dock API helpers

#include "dock_internal.h"

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
