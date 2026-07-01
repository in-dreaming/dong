#ifndef DONG_UI_PASS_BUNDLE_H
#define DONG_UI_PASS_BUNDLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DongUiPassBundle {
    const void* command_list;
    uint32_t upload_byte_estimate;
    int has_upload;
    int has_layer_raster;
    int has_ui_main;
    int has_texture_compress;
    uint32_t draw_command_count;
} DongUiPassBundle;

#ifdef __cplusplus
}
#endif

#endif /* DONG_UI_PASS_BUNDLE_H */
