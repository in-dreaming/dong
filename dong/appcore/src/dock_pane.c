// dock_pane.c - pane creation/split orchestration

#include "dock_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void dock_init_pane_title(dong_dock_pane_t* pane, const char* title) {
    if (!title) return;
    strncpy(pane->title, title, DOCK_MAX_TITLE - 1);
    pane->title[DOCK_MAX_TITLE - 1] = 0;
}

static void dock_init_pane_content(
    dong_dock_pane_t* pane,
    const dong_dock_pane_config_t* config)
{
    if (config->resource_root) {
        strncpy(pane->resource_root, config->resource_root, sizeof(pane->resource_root) - 1);
        pane->resource_root[sizeof(pane->resource_root) - 1] = 0;
        dong_engine_set_resource_root(pane->engine, config->resource_root);
    }

    if (config->html) {
        dong_engine_load_html(pane->engine, config->html);
        return;
    }

    if (!config->html_file) return;

    char dir[1024];
    dock_extract_dir(config->html_file, dir, sizeof(dir));
    if (dir[0]) dong_engine_set_resource_root(pane->engine, dir);

    FILE* f = fopen(config->html_file, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)sz + 1);
    if (buf) {
        size_t rd = fread(buf, 1, (size_t)sz, f);
        buf[rd] = 0;
        dong_engine_load_html(pane->engine, buf);
        free(buf);
    }
    fclose(f);
}

static dong_dock_pane_t* dock_alloc_and_init_pane(
    dong_dock_t* dock,
    const dong_dock_pane_config_t* config,
    int window_index,
    uint32_t default_w,
    uint32_t default_h)
{
    int pi = dock_alloc_pane_slot(dock);
    if (pi < 0) return NULL;

    dong_dock_pane_t* pane = &dock->panes[pi];
    pane->alive = 1;
    pane->index = pi;
    pane->window_index = window_index;

    dock_init_pane_title(pane, config->title);

    uint32_t pw = config->width ? config->width : default_w;
    uint32_t ph = config->height ? config->height : default_h;

    pane->engine = dock_create_engine(dock, pw, ph, config->shared_js, config->view_name);
    if (!pane->engine) {
        pane->alive = 0;
        return NULL;
    }

    dock_init_pane_content(pane, config);
    return pane;
}

dong_dock_pane_t* dock_add_pane_impl(
    dong_dock_t* dock, const dong_dock_pane_config_t* config)
{
    if (!dock || !config) return NULL;

    dong_dock_pane_t* pane = dock_alloc_and_init_pane(
        dock, config, 0, dock->windows[0].width, dock->windows[0].height);
    if (!pane) return NULL;

    // Insert into primary window's split tree
    int pi = pane->index;
    dong_dock_window_t* wn = &dock->windows[0];
    if (!wn->root) {
        wn->root = dock_node_new_leaf(pi);
    } else {
        dock_node_t* leaf = wn->root;
        while (leaf->type != DOCK_NODE_LEAF) {
            leaf = leaf->children[0];
        }
        dock_node_add_tab(leaf, pi);
    }

    pane->node = dock_node_find_leaf(wn->root, pi);
    if (dock->focused_pane_index < 0) dock->focused_pane_index = pi;
    return pane;
}

dong_dock_pane_t* dock_split_impl(
    dong_dock_t* dock, dong_dock_pane_t* neighbor,
    dong_dock_edge_t edge, float ratio,
    const dong_dock_pane_config_t* config)
{
    if (!dock || !neighbor || !config || !neighbor->alive) return NULL;

    dong_dock_pane_t* pane = dock_alloc_and_init_pane(
        dock, config, neighbor->window_index, 400, 300);
    if (!pane) return NULL;

    int pi = pane->index;

    dock_node_t* leaf = neighbor->node;
    if (!leaf) {
        leaf = dock_node_find_leaf(
            dock->windows[neighbor->window_index].root, neighbor->index);
    }
    if (!leaf) {
        if (pane->engine) {
            dong_engine_destroy(pane->engine);
            pane->engine = NULL;
        }
        pane->alive = 0;
        return NULL;
    }

    if (edge == DONG_DOCK_TAB) {
        dock_node_add_tab(leaf, pi);
        pane->node = leaf;
    } else {
        dock_node_t* new_leaf = dock_node_split(
            leaf, edge, pi, ratio > 0.0f ? ratio : 0.5f);
        pane->node = new_leaf;
        neighbor->node = dock_node_find_leaf(
            dock->windows[neighbor->window_index].root, neighbor->index);
    }

    return pane;
}

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
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    buf[rd] = 0;
    fclose(f);
    int ok = dong_dock_pane_load_html(pane, buf);
    free(buf);
    return ok;
}

DONG_APPCORE_API void dong_dock_pane_set_title(dong_dock_pane_t* pane, const char* title) {
    if (!pane || !pane->alive || !title) return;
    strncpy(pane->title, title, DOCK_MAX_TITLE - 1);
    pane->title[DOCK_MAX_TITLE - 1] = 0;
}

DONG_APPCORE_API const char* dong_dock_pane_get_title(dong_dock_pane_t* pane) {
    return (pane && pane->alive) ? pane->title : "";
}

DONG_APPCORE_API void dong_dock_pane_set_resource_root(dong_dock_pane_t* pane, const char* root) {
    if (!pane || !pane->alive || !pane->engine || !root) return;
    strncpy(pane->resource_root, root, sizeof(pane->resource_root) - 1);
    pane->resource_root[sizeof(pane->resource_root) - 1] = 0;
    dong_engine_set_resource_root(pane->engine, root);
}

DONG_APPCORE_API int dong_dock_pane_eval_script(dong_dock_pane_t* pane, const char* script) {
    if (!pane || !pane->alive || !pane->engine || !script) return 0;
    return (dong_engine_eval_script(pane->engine, script) == DONG_OK) ? 1 : 0;
}
