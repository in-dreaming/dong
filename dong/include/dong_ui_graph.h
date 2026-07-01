#ifndef DONG_UI_GRAPH_H
#define DONG_UI_GRAPH_H

#include "dong_ui_pass_bundle.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct DongGPUDriver;
typedef struct DongGPUDriver DongGPUDriver;

/* Opaque host graph handles (cast to GpuGraph/GpuDevice when using in-dreaming/gpu). */
typedef void* DongHostGpuGraph;
typedef void* DongHostGpuDevice;
typedef void* DongHostGpuGraphResource;
typedef uint32_t DongUiGraphResource;

typedef enum DongUiCompositeMode {
    DONG_UI_COMPOSITE_OVERLAY = 0,
    DONG_UI_COMPOSITE_OFFSCREEN,
    DONG_UI_COMPOSITE_UNDER_UI_HOOK,
} DongUiCompositeMode;

typedef struct DongUiDrawScope {
    float clip_x, clip_y, clip_w, clip_h;
    float transform[6];
    float opacity;
} DongUiDrawScope;

typedef void (*DongHostViewDrawFn)(uint32_t host_view_id,
                                    const DongUiDrawScope* scope,
                                    void* userdata);

typedef struct DongUiGraphContext {
    DongHostGpuGraph host_graph;
    DongHostGpuDevice device;
    DongGPUDriver* driver;
    DongHostGpuGraphResource composite_target;
    DongHostGpuGraphResource depth_target;
    DongUiCompositeMode composite_mode;
    float viewport_x, viewport_y, viewport_w, viewport_h;
    void* _internal;
} DongUiGraphContext;

typedef struct DongGtcGraphContext {
    void* gtc_context;
    DongHostGpuGraph host_graph;
    DongHostGpuDevice device;
} DongGtcGraphContext;

void dong_ui_graph_context_init(DongUiGraphContext* ctx);

void dong_ui_graph_prepare(DongUiGraphContext* ctx, const void* command_list);
void dong_ui_graph_add_passes(DongUiGraphContext* ctx);
DongUiGraphResource dong_ui_graph_get_output(const DongUiGraphContext* ctx);
const DongUiPassBundle* dong_ui_graph_get_pass_bundle(const DongUiGraphContext* ctx);

void dong_ui_graph_set_host_view_callback(DongUiGraphContext* ctx,
                                            DongHostViewDrawFn fn,
                                            void* userdata);

void dong_ui_graph_reset(DongUiGraphContext* ctx);

void dong_gtc_graph_add_passes(DongGtcGraphContext* gtc_ctx);

void dong_gpu_driver_set_external_device(DongGPUDriver* driver,
                                         DongHostGpuDevice device,
                                         int owns_device);
void dong_gpu_driver_set_external_swapchain(DongGPUDriver* driver,
                                            void* surface,
                                            void* queue,
                                            int owns_surface);
void dong_gpu_driver_set_embedded_mode(DongGPUDriver* driver, int embedded);
int dong_gpu_driver_is_embedded_mode(const DongGPUDriver* driver);

#ifdef __cplusplus
}
#endif

#endif /* DONG_UI_GRAPH_H */
