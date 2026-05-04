// dock_layout_io.c - layout persistence entrypoints

#include "dock_internal.h"

#include <stdio.h>

DONG_APPCORE_API int dong_dock_save_layout(dong_dock_t* dock, const char* path) {
    (void)dock;
    (void)path;
    fprintf(stderr, "[DongDock] save_layout: not yet implemented\n");
    return 0;
}

DONG_APPCORE_API int dong_dock_load_layout(dong_dock_t* dock, const char* path,
                                            dong_dock_pane_created_fn on_pane,
                                            void* user_data) {
    (void)dock;
    (void)path;
    (void)on_pane;
    (void)user_data;
    fprintf(stderr, "[DongDock] load_layout: not yet implemented\n");
    return 0;
}
