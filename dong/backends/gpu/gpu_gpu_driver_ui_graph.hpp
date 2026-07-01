#pragma once

#include "dong_ui_pass_bundle.h"

#ifdef DONG_HAS_IN_DREAMING_GPU
struct GpuGraph_t;
typedef struct GpuGraph_t* GpuGraph;
typedef uint32_t GpuGraphResource;
#endif

namespace dong::gpu_backend {

class GpuGPUDriverImpl;

#ifdef DONG_HAS_IN_DREAMING_GPU
void gpu_register_ui_passes_on_graph(GpuGraph graph,
                                     const DongUiPassBundle* bundle,
                                     GpuGraphResource color_target,
                                     GpuGPUDriverImpl* impl);
#endif

void gpu_register_ui_graph_vtable(struct DongGPUDriverVTable* vtable);

} // namespace dong::gpu_backend
