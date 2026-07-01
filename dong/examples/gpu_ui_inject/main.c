/* Tier-2: host-owned GpuGraph with Dong UI pass injection. */
#include "dong_gpu_backend_driver.h"
#include "dong_gpu_driver.h"
#include "dong_ui_graph.h"

#include "gpu/gpu.h"
#include "gpu/platform/gpu_platform.h"
#include "gpu/platform/gpu_surface.h"
#include "gpu/rendergraph/gpu_render_graph.h"

#include <stdio.h>
#include <stdint.h>

static void host_clear_pass(GpuGraphPassContext* ctx, void* user_data) {
    (void)ctx;
    (void)user_data;
}

/* Dong's UI pipeline produces already display-ready (gamma-encoded) colors, not linear light
 * values. Configuring the swapchain with an _SRGB surface format makes the GPU's render-output
 * stage re-apply a linear->sRGB encode on every write, double-gamma-correcting the already-
 * encoded UI colors into a washed-out/hazy image. gpuSurfaceGetPreferredFormat() commonly
 * returns an _SRGB variant, so strip it back to the plain UNORM equivalent before configuring
 * the surface. */
static GpuFormat strip_srgb_format(GpuFormat fmt) {
    switch (fmt) {
    case GPU_FORMAT_RGBA8_UNORM_SRGB: return GPU_FORMAT_RGBA8_UNORM;
    case GPU_FORMAT_BGRA8_UNORM_SRGB: return GPU_FORMAT_BGRA8_UNORM;
    default: return fmt;
    }
}

static int run_embedded_tier2(DongGPUDriver* driver,
                              GpuDevice device,
                              GpuCommandQueue queue,
                              GpuSurface surface) {
    DongUiGraphContext* ui_ctx = dong_gpu_backend_get_ui_graph(driver);
    if (!ui_ctx) {
        fprintf(stderr, "dong_gpu_backend_get_ui_graph failed\n");
        return 1;
    }

    GpuSurfaceTexture backbuffer = NULL;
    if (gpuSurfaceAcquireNextImage(surface, &backbuffer) != GPU_SUCCESS) {
        return 0;
    }

    GpuGraph graph = NULL;
    if (gpuGraphCreate(device, &graph) != GPU_SUCCESS) {
        gpuSurfaceTextureRelease(backbuffer);
        return 0;
    }

    GpuGraphResource backbuffer_res = gpuGraphImportSurfaceTexture(graph, backbuffer, "backbuffer");

    GpuGraphPass host_pass = gpuGraphAddRenderPass(graph, "Host.Clear");
    GpuGraphColorAttachment host_ca = {
        .resource = backbuffer_res,
        .loadOp = GPU_LOAD_OP_CLEAR,
        .storeOp = GPU_STORE_OP_STORE,
        .clearColor = {0.05f, 0.08f, 0.12f, 1.0f},
    };
    gpuGraphPassSetColorAttachments(host_pass, 1, &host_ca);
    gpuGraphPassSetCallback(host_pass, host_clear_pass, NULL);

    ui_ctx->host_graph = graph;
    ui_ctx->device = device;
    ui_ctx->driver = driver;
    ui_ctx->composite_target = (void*)(uintptr_t)backbuffer_res;
    ui_ctx->composite_mode = DONG_UI_COMPOSITE_OVERLAY;

    dong_ui_graph_prepare(ui_ctx, NULL);
    dong_ui_graph_add_passes(ui_ctx);

    GpuGraphPass present = gpuGraphAddRenderPass(graph, "Present");
    gpuGraphPassPresent(present, backbuffer_res);

    if (gpuGraphCompile(graph) == GPU_SUCCESS) {
        gpuGraphExecute(graph, queue);
    }
    gpuGraphDestroy(graph);

    gpuSurfacePresent(surface);
    gpuSurfaceTextureRelease(backbuffer);
    return 0;
}

int main(void) {
    if (gpuPlatformInit() != GPU_SUCCESS) {
        fprintf(stderr, "gpuPlatformInit failed\n");
        return 1;
    }

    GpuWindow window = NULL;
    GpuWindowDesc win_desc = {
        .title = "gpu_ui_inject",
        .width = 800,
        .height = 600,
        .vsync = true,
        .resizable = true,
    };
    if (gpuCreateWindow(&win_desc, &window) != GPU_SUCCESS) {
        fprintf(stderr, "gpuCreateWindow failed\n");
        gpuPlatformShutdown();
        return 1;
    }

    GpuDevice device = NULL;
    GpuDeviceDesc dev_desc = {.appName = "gpu_ui_inject", .enableDebugLayer = true};
    if (gpuCreateDevice(&dev_desc, &device) != GPU_SUCCESS) {
        fprintf(stderr, "gpuCreateDevice failed\n");
        gpuDestroyWindow(window);
        gpuPlatformShutdown();
        return 1;
    }

    GpuSurface surface = NULL;
    if (gpuCreateSurface(device, window, GPU_SURFACE_TYPE_VULKAN, &surface) != GPU_SUCCESS) {
        fprintf(stderr, "gpuCreateSurface failed\n");
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        gpuPlatformShutdown();
        return 1;
    }

    GpuFormat fmt = strip_srgb_format(gpuSurfaceGetPreferredFormat(surface));
    if (gpuSurfaceConfigure(surface, win_desc.width, win_desc.height, fmt, true) != GPU_SUCCESS) {
        fprintf(stderr, "gpuSurfaceConfigure failed\n");
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        gpuPlatformShutdown();
        return 1;
    }

    GpuCommandQueue queue = NULL;
    if (gpuGetQueue(device, GPU_QUEUE_TYPE_GRAPHICS, &queue) != GPU_SUCCESS) {
        fprintf(stderr, "gpuGetQueue failed\n");
        gpuSurfaceUnconfigure(surface);
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        gpuPlatformShutdown();
        return 1;
    }

    DongGPUDriver* driver = dong_gpu_backend_create_driver();
    if (!driver) {
        fprintf(stderr, "dong_gpu_backend_create_driver failed\n");
        return 1;
    }
    dong_gpu_driver_set_external_device(driver, device, 0);
    dong_gpu_driver_set_embedded_mode(driver, 1);
    if (!dong_gpu_driver_initialize(driver)) {
        fprintf(stderr, "dong_gpu_driver_initialize failed\n");
        dong_gpu_backend_destroy_driver(driver);
        return 1;
    }

    fprintf(stderr, "gpu_ui_inject: Tier-2 host graph + Dong UI passes (120 frames)\n");

    uint32_t frame = 0;
    GpuPlatformEvent ev;
    int quit = 0;
    while (frame < 120 && !quit) {
        while (gpuPollEvent(&ev)) {
            if (ev.type == GPU_PLATFORM_EVENT_QUIT) {
                quit = 1;
                break;
            }
            if (ev.type == GPU_PLATFORM_EVENT_RESIZE) {
                gpuSurfaceConfigure(surface, ev.resize.width, ev.resize.height, fmt, true);
            }
        }
        if (quit) {
            break;
        }
        if (run_embedded_tier2(driver, device, queue, surface) != 0) {
            break;
        }
        frame++;
    }

    fprintf(stderr, "Rendered %u frames\n", frame);
    gpuQueueWaitOnHost(queue);
    dong_gpu_driver_shutdown(driver);
    dong_gpu_backend_destroy_driver(driver);
    gpuSurfaceUnconfigure(surface);
    gpuDestroySurface(device, surface);
    gpuDestroyDevice(device);
    gpuDestroyWindow(window);
    gpuPlatformShutdown();
    return 0;
}
