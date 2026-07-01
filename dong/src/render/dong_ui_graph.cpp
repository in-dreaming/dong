#include "dong_ui_graph.h"
#include "dong_gpu_driver.h"
#include "ui_pass_compiler.hpp"

#include "../../src/render/gpu_ir.hpp"

#include <cstdlib>
#include <cstring>

struct DongUiGraphInternal {
    const void* command_list = nullptr;
    DongUiPassBundle bundle{};
    DongHostViewDrawFn host_view_fn = nullptr;
    void* host_view_userdata = nullptr;
    DongUiGraphResource output_resource = 0;
};

void dong_ui_graph_context_init(DongUiGraphContext* ctx) {
    if (!ctx) {
        return;
    }
    if (!ctx->_internal) {
        ctx->_internal = new DongUiGraphInternal();
    }
}

void dong_ui_graph_prepare(DongUiGraphContext* ctx, const void* command_list) {
    if (!ctx) {
        return;
    }
    dong_ui_graph_context_init(ctx);
    auto* internal = static_cast<DongUiGraphInternal*>(ctx->_internal);
    internal->command_list = command_list;
    const auto* list = static_cast<const dong::render::GPUCommandList*>(command_list);
    dong::render::UiPassCompiler compiler;
    internal->bundle = list ? compiler.compile(*list) : DongUiPassBundle{};
}

void dong_ui_graph_add_passes(DongUiGraphContext* ctx) {
    if (!ctx || !ctx->driver || !ctx->host_graph) {
        return;
    }
    dong_ui_graph_context_init(ctx);
    auto* internal = static_cast<DongUiGraphInternal*>(ctx->_internal);
    internal->output_resource = ctx->composite_target
        ? (DongUiGraphResource)(uintptr_t)ctx->composite_target
        : 0;

    dong_gpu_ui_graph_add_passes(ctx->driver, ctx);
}

DongUiGraphResource dong_ui_graph_get_output(const DongUiGraphContext* ctx) {
    if (!ctx || !ctx->_internal) {
        return 0;
    }
    return static_cast<const DongUiGraphInternal*>(ctx->_internal)->output_resource;
}

const DongUiPassBundle* dong_ui_graph_get_pass_bundle(const DongUiGraphContext* ctx) {
    if (!ctx || !ctx->_internal) {
        return nullptr;
    }
    return &static_cast<const DongUiGraphInternal*>(ctx->_internal)->bundle;
}

void dong_ui_graph_set_host_view_callback(DongUiGraphContext* ctx,
                                          DongHostViewDrawFn fn,
                                          void* userdata) {
    if (!ctx) {
        return;
    }
    dong_ui_graph_context_init(ctx);
    auto* internal = static_cast<DongUiGraphInternal*>(ctx->_internal);
    internal->host_view_fn = fn;
    internal->host_view_userdata = userdata;
}

void dong_ui_graph_reset(DongUiGraphContext* ctx) {
    if (!ctx) {
        return;
    }
    if (ctx->_internal) {
        delete static_cast<DongUiGraphInternal*>(ctx->_internal);
        ctx->_internal = nullptr;
    }
}

void dong_gtc_graph_add_passes(DongGtcGraphContext* gtc_ctx) {
    (void)gtc_ctx;
}

void dong_gpu_driver_set_external_device(DongGPUDriver* driver,
                                         DongHostGpuDevice device,
                                         int owns_device) {
    if (!driver) {
        return;
    }
    driver->external_device = device;
    driver->owns_external_device = owns_device ? 1 : 0;
}

void dong_gpu_driver_set_embedded_mode(DongGPUDriver* driver, int embedded) {
    if (!driver) {
        return;
    }
    driver->embedded_mode = embedded ? 1 : 0;
}

int dong_gpu_driver_is_embedded_mode(const DongGPUDriver* driver) {
    return (driver && driver->embedded_mode) ? 1 : 0;
}
