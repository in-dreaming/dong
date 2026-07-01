#if defined(DONG_BACKEND_GPU)

#include "dong_dock.h"

DONG_APPCORE_API dong_dock_t* dong_dock_create(const dong_dock_config_t* config) {
    (void)config;
    return NULL;
}

DONG_APPCORE_API void dong_dock_destroy(dong_dock_t* dock) { (void)dock; }

DONG_APPCORE_API int dong_dock_poll_events(dong_dock_t* dock) {
    (void)dock;
    return 0;
}

DONG_APPCORE_API void dong_dock_render(dong_dock_t* dock) { (void)dock; }

DONG_APPCORE_API int dong_dock_is_running(dong_dock_t* dock) {
    (void)dock;
    return 0;
}

DONG_APPCORE_API float dong_dock_get_delta_time(dong_dock_t* dock) {
    (void)dock;
    return 0.0f;
}

DONG_APPCORE_API void dong_dock_run(dong_dock_t* dock, dong_dock_tick_fn tick, void* user_data) {
    (void)dock;
    (void)tick;
    (void)user_data;
}

DONG_APPCORE_API dong_dock_pane_t* dong_dock_add_pane(dong_dock_t* dock, const dong_dock_pane_config_t* config) {
    (void)dock;
    (void)config;
    return NULL;
}

DONG_APPCORE_API dong_dock_pane_t* dong_dock_split(dong_dock_t* dock, dong_dock_pane_t* neighbor,
                                                     dong_dock_edge_t edge, float ratio,
                                                     const dong_dock_pane_config_t* config) {
    (void)dock;
    (void)neighbor;
    (void)edge;
    (void)ratio;
    (void)config;
    return NULL;
}

DONG_APPCORE_API void dong_dock_remove_pane(dong_dock_t* dock, dong_dock_pane_t* pane) {
    (void)dock;
    (void)pane;
}

DONG_APPCORE_API dong_dock_window_t* dong_dock_detach(dong_dock_t* dock, dong_dock_pane_t* pane, int window_x,
                                                      int window_y, uint32_t window_w, uint32_t window_h) {
    (void)dock;
    (void)pane;
    (void)window_x;
    (void)window_y;
    (void)window_w;
    (void)window_h;
    return NULL;
}

DONG_APPCORE_API void dong_dock_attach(dong_dock_t* dock, dong_dock_pane_t* pane, dong_dock_pane_t* target,
                                        dong_dock_edge_t edge) {
    (void)dock;
    (void)pane;
    (void)target;
    (void)edge;
}

DONG_APPCORE_API dong_engine_t* dong_dock_pane_get_engine(dong_dock_pane_t* pane) {
    (void)pane;
    return NULL;
}

DONG_APPCORE_API int dong_dock_pane_load_html(dong_dock_pane_t* pane, const char* html) {
    (void)pane;
    (void)html;
    return 0;
}

DONG_APPCORE_API int dong_dock_pane_load_html_file(dong_dock_pane_t* pane, const char* path) {
    (void)pane;
    (void)path;
    return 0;
}

DONG_APPCORE_API void dong_dock_pane_set_title(dong_dock_pane_t* pane, const char* title) {
    (void)pane;
    (void)title;
}

DONG_APPCORE_API const char* dong_dock_pane_get_title(dong_dock_pane_t* pane) {
    (void)pane;
    return "";
}

DONG_APPCORE_API void dong_dock_pane_set_resource_root(dong_dock_pane_t* pane, const char* root) {
    (void)pane;
    (void)root;
}

DONG_APPCORE_API int dong_dock_pane_eval_script(dong_dock_pane_t* pane, const char* script) {
    (void)pane;
    (void)script;
    return 0;
}

DONG_APPCORE_API void dong_dock_set_split_ratio(dong_dock_t* dock, dong_dock_pane_t* pane, float ratio) {
    (void)dock;
    (void)pane;
    (void)ratio;
}

DONG_APPCORE_API int dong_dock_get_window_count(dong_dock_t* dock) {
    (void)dock;
    return 0;
}

DONG_APPCORE_API dong_dock_window_t* dong_dock_get_window(dong_dock_t* dock, int index) {
    (void)dock;
    (void)index;
    return NULL;
}

DONG_APPCORE_API dong_dock_window_t* dong_dock_get_primary_window(dong_dock_t* dock) {
    (void)dock;
    return NULL;
}

DONG_APPCORE_API void* dong_dock_window_get_native(dong_dock_window_t* win) {
    (void)win;
    return NULL;
}

DONG_APPCORE_API int dong_dock_save_layout(dong_dock_t* dock, const char* path) {
    (void)dock;
    (void)path;
    return 0;
}

DONG_APPCORE_API int dong_dock_load_layout(dong_dock_t* dock, const char* path, dong_dock_pane_created_fn on_pane,
                                           void* user_data) {
    (void)dock;
    (void)path;
    (void)on_pane;
    (void)user_data;
    return 0;
}

DONG_APPCORE_API void dong_dock_set_pane_close_callback(dong_dock_t* dock, dong_dock_pane_close_fn callback,
                                                        void* user_data) {
    (void)dock;
    (void)callback;
    (void)user_data;
}

DONG_APPCORE_API void dong_dock_set_window_close_callback(dong_dock_t* dock, dong_dock_window_close_fn callback,
                                                          void* user_data) {
    (void)dock;
    (void)callback;
    (void)user_data;
}

#endif
