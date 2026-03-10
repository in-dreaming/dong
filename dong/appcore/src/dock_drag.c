// dock_drag.c - Dock tab sizing and drag state machine

#include "dock_internal.h"

// =============================================================================
// Tab width computation
// =============================================================================

uint32_t dock_tab_width(const dock_node_t* node) {
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

    // Title bar area -> TAB
    if (ly < DOCK_TITLE_BAR_HEIGHT) return DOCK_DROP_TAB;

    // Edge fractions
    if (fx < DOCK_DROP_EDGE_FRAC)       return DOCK_DROP_LEFT;
    if (fx > 1.0f - DOCK_DROP_EDGE_FRAC) return DOCK_DROP_RIGHT;
    if (fy < DOCK_DROP_EDGE_FRAC)       return DOCK_DROP_TOP;
    if (fy > 1.0f - DOCK_DROP_EDGE_FRAC) return DOCK_DROP_BOTTOM;

    // Center -> TAB
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

// Update drag: PENDING->ACTIVE on threshold, compute drop zone across ALL windows
void dock_update_drag(dong_dock_t* dock) {
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

    // Check if the source is a non-primary single-pane window (detached pane).
    // If so, move the window to follow the mouse in real-time instead of
    // treating itself as a drop target.
    int src_is_detached = 0;
    dong_dock_window_t* src_win = NULL;
    if (d->source_window_index >= 0 && d->source_window_index < dock->window_count) {
        src_win = &dock->windows[d->source_window_index];
        if (src_win->alive && !src_win->is_primary &&
            src_win->root && src_win->root->type == DOCK_NODE_LEAF &&
            src_win->root->pane_count == 1) {
            src_is_detached = 1;
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

        // If hovering our own detached window, move it with the mouse
        // instead of computing drop zones on itself
        if (src_is_detached && wn->index == d->source_window_index) {
            int32_t new_x = (int32_t)gx - d->start_x;
            int32_t new_y = (int32_t)gy - d->start_y;
            SDL_SetWindowPosition(wn->sdl_window, new_x, new_y);
            // No drop target while over own window
            d->hover_window_index = -1;
            d->drop_zone = DOCK_DROP_NONE;
            d->target_w = 0;
            d->target_h = 0;
            d->reorder_insert_pos = -1;
            return;
        }

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

    // Not over any window -> DOCK_DROP_NONE (will detach on release)
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
void dock_finish_drag(dong_dock_t* dock) {
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
        dock_window_remove_pane_index(dock, old_win, src_pi);

        // Split at window root level
        dong_dock_edge_t edge = dock_zone_to_edge(d->drop_zone);
        dock_node_t* new_leaf = dock_node_split_root(
            &hover_win->root, edge, src_pi, 0.5f);

        // Update pane metadata
        src_pane->window_index = hover_win->index;
        src_pane->node = new_leaf;
    } else if (d->drop_zone == DOCK_DROP_NONE || !hover_node ||
        hover_node->type != DOCK_NODE_LEAF || hover_node->pane_count == 0) {
        // No valid target.
        // If source is already a detached single-pane window, it was moved
        // in real-time by dock_update_drag — just keep it where it is.
        dong_dock_window_t* sw = &dock->windows[src_pane->window_index];
        int already_detached = (!sw->is_primary && sw->root &&
            sw->root->type == DOCK_NODE_LEAF && sw->root->pane_count == 1);
        if (!already_detached) {
            // Detach to new window at global mouse position.
            // Use start_x/start_y as cursor offset within the new window
            // so the tab appears anchored where the user grabbed it.
            float gx, gy;
            SDL_GetGlobalMouseState(&gx, &gy);
            int32_t det_x = (int32_t)gx - d->start_x;
            int32_t det_y = (int32_t)gy - d->start_y;

            // Reasonable detach window size: use source pane content size,
            // clamped to [320x240, 800x600]
            uint32_t det_w = 0, det_h = 0;
            if (src_pane->tex_w > 0 && src_pane->tex_h > 0) {
                det_w = src_pane->tex_w;
                det_h = src_pane->tex_h + DOCK_TITLE_BAR_HEIGHT;
            }
            if (det_w < 320) det_w = 320;
            if (det_h < 240) det_h = 240;
            if (det_w > 800) det_w = 800;
            if (det_h > 600) det_h = 600;

            dong_dock_detach(dock, src_pane, det_x, det_y, det_w, det_h);
        }
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
