// dock_events.c - Dock event translation and forwarding helpers

#include "dock_internal.h"

#include <string.h>

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
                        uint32_t dim; // total dimension along split axis
                        if (dn->type == DOCK_NODE_SPLIT_H) {
                            new_ratio = (float)(mx - dn->x) / (float)dn->w;
                            dim = dn->w;
                        } else {
                            new_ratio = (float)(my - dn->y) / (float)dn->h;
                            dim = dn->h;
                        }
                        // Clamp: both children must be >= DOCK_MIN_PANE_SIZE
                        float min_ratio = (dim > 0)
                            ? (float)DOCK_MIN_PANE_SIZE / (float)dim : 0.05f;
                        float max_ratio = 1.0f - min_ratio;
                        if (min_ratio > 0.45f) min_ratio = 0.45f; // failsafe
                        if (max_ratio < 0.55f) max_ratio = 0.55f;
                        if (new_ratio < min_ratio) new_ratio = min_ratio;
                        if (new_ratio > max_ratio) new_ratio = max_ratio;
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

            // --- Update hover state on mouse move ---
            if (ev.type == DONG_APP_EVENT_MOUSE_MOVE &&
                dock->drag.state == DOCK_DRAG_NONE &&
                dock->divider.state != DOCK_DIV_DRAGGING)
            {
                dock->hover.type = DOCK_HOVER_NONE;
                dock->hover.window_index = win->index;
                dock->hover.node = NULL;
                dock->hover.tab_index = -1;

                // Check divider hover
                dock_node_t* div_hit = win->root
                    ? dock_node_hit_test_divider(win->root, mx, my) : NULL;
                if (div_hit) {
                    dock->hover.type = DOCK_HOVER_DIVIDER;
                    dock->hover.node = div_hit;
                } else if (hit && hit->pane_count > 0 && (my - hit->y) < DOCK_TITLE_BAR_HEIGHT) {
                    // In title bar 闂?check window buttons first
                    int32_t by_btn = (DOCK_TITLE_BAR_HEIGHT - DOCK_BTN_SIZE) / 2;
                    int32_t bx_btn = (int32_t)win->width - DOCK_BTN_PAD - DOCK_BTN_SIZE;
                    if (mx >= bx_btn && mx < bx_btn + DOCK_BTN_SIZE &&
                        my >= by_btn && my < by_btn + DOCK_BTN_SIZE) {
                        dock->hover.type = DOCK_HOVER_WIN_CLOSE;
                    } else {
                        bx_btn -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);
                        if (mx >= bx_btn && mx < bx_btn + DOCK_BTN_SIZE &&
                            my >= by_btn && my < by_btn + DOCK_BTN_SIZE) {
                            dock->hover.type = DOCK_HOVER_WIN_MAXIMIZE;
                        } else {
                            bx_btn -= (DOCK_BTN_SIZE + DOCK_BTN_PAD);
                            if (mx >= bx_btn && mx < bx_btn + DOCK_BTN_SIZE &&
                                my >= by_btn && my < by_btn + DOCK_BTN_SIZE) {
                                dock->hover.type = DOCK_HOVER_WIN_MINIMIZE;
                            }
                        }
                    }

                    // Check per-tab areas (if not over a window button)
                    if (dock->hover.type == DOCK_HOVER_NONE) {
                        uint32_t tab_w = dock_tab_width(hit);
                        if (tab_w > 0) {
                            for (int t = 0; t < hit->pane_count; t++) {
                                int32_t tx = hit->x + DOCK_TAB_PAD + (int32_t)(t * (tab_w + DOCK_TAB_PAD));
                                int32_t ty_tab = hit->y + 2;
                                uint32_t th = DOCK_TITLE_BAR_HEIGHT - 4;
                                if (mx >= tx && mx < tx + (int32_t)tab_w &&
                                    my >= ty_tab && my < ty_tab + (int32_t)th) {
                                    // Inside this tab 闂?check close button
                                    int32_t cbx = tx + (int32_t)tab_w - DOCK_TAB_CLOSE_PAD - DOCK_TAB_CLOSE_SIZE;
                                    int32_t cby = ty_tab + (int32_t)(th - DOCK_TAB_CLOSE_SIZE) / 2;
                                    if (mx >= cbx && mx < cbx + DOCK_TAB_CLOSE_SIZE &&
                                        my >= cby && my < cby + DOCK_TAB_CLOSE_SIZE) {
                                        dock->hover.type = DOCK_HOVER_TAB_CLOSE;
                                    } else {
                                        dock->hover.type = DOCK_HOVER_TAB;
                                    }
                                    dock->hover.node = hit;
                                    dock->hover.tab_index = t;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

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

                        // Check per-tab close buttons
                        {
                            uint32_t tab_w = dock_tab_width(hit);
                            if (tab_w > 0) {
                                for (int t = 0; t < hit->pane_count; t++) {
                                    int32_t tx = hit->x + DOCK_TAB_PAD + (int32_t)(t * (tab_w + DOCK_TAB_PAD));
                                    int32_t ty = hit->y + 2;
                                    uint32_t th = DOCK_TITLE_BAR_HEIGHT - 4;
                                    int32_t cbx = tx + (int32_t)tab_w - DOCK_TAB_CLOSE_PAD - DOCK_TAB_CLOSE_SIZE;
                                    int32_t cby = ty + (int32_t)(th - DOCK_TAB_CLOSE_SIZE) / 2;
                                    if (mx >= cbx && mx < cbx + DOCK_TAB_CLOSE_SIZE &&
                                        my >= cby && my < cby + DOCK_TAB_CLOSE_SIZE) {
                                        int pi = hit->pane_indices[t];
                                        if (pi >= 0 && pi < dock->pane_count &&
                                            dock->panes[pi].alive) {
                                            dong_dock_remove_pane(dock, &dock->panes[pi]);
                                        }
                                        goto title_bar_handled;
                                    }
                                }
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
                    title_bar_handled:
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
