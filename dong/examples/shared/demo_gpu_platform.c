#include "demo_gpu_platform.h"

#include "dong_backend.h"
#include "dong_platform.h"
#include "dong_plugin_api.h"

#include "dong_gpu_backend_driver.h"
#include "dong_gpu_image_decoder.h"
#include "dong_ui_graph.h"
#include "gpu/gpu.h"
#include "gpu/platform/gpu_platform.h"
#include "gpu/platform/gpu_surface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

/* Dong's UI pipeline produces already display-ready (gamma-encoded) colors, not linear light
 * values. Configuring the swapchain with an _SRGB surface format makes the GPU's render-output
 * stage re-apply a linear->sRGB encode on every write, double-gamma-correcting the already-
 * encoded UI colors into a washed-out/hazy image. gpuSurfaceGetPreferredFormat() commonly
 * returns an _SRGB variant, so strip it back to the plain UNORM equivalent before configuring
 * the surface (mirrors gpu_gpu_driver.hpp's stripSrgbFormat, kept local since this is plain C). */
static GpuFormat dong_demo_gpu_strip_srgb_format(GpuFormat fmt) {
    switch (fmt) {
    case GPU_FORMAT_RGBA8_UNORM_SRGB: return GPU_FORMAT_RGBA8_UNORM;
    case GPU_FORMAT_BGRA8_UNORM_SRGB: return GPU_FORMAT_BGRA8_UNORM;
    default: return fmt;
    }
}

static void extract_dir_from_path(const char* path, char* out_dir, size_t out_size) {
    if (!path || !out_dir || out_size == 0) {
        return;
    }
    strncpy(out_dir, path, out_size - 1);
    out_dir[out_size - 1] = '\0';
    char* slash = strrchr(out_dir, '/');
    char* backslash = strrchr(out_dir, '\\');
    char* sep = slash;
    if (backslash && (!sep || backslash > sep)) {
        sep = backslash;
    }
    if (sep) {
        *sep = '\0';
    } else {
        out_dir[0] = '\0';
    }
}

/* Optional plugin (video decoding via FFmpeg, etc.). Loaded from the executable's own
 * directory, mirroring appcore/src/app.c's try_load_plugin() for the SDL backend. Without
 * this, <video> elements never open a decoder and the GPU backend's already-implemented
 * external-image texture path (updateExternalImageRGBA/YUV420P) never receives frames. */
static const dong_plugin_vtable_t* dong_demo_gpu_try_load_plugin(void) {
#if defined(_WIN32)
    static const dong_plugin_vtable_t* s_vtable = NULL;
    static int s_attempted = 0;
    if (s_attempted) {
        return s_vtable;
    }
    s_attempted = 1;

    char exe_path[1024];
    DWORD len = GetModuleFileNameA(NULL, exe_path, (DWORD)sizeof(exe_path));
    if (len == 0 || len >= sizeof(exe_path)) {
        return NULL;
    }
    char dir[1024];
    extract_dir_from_path(exe_path, dir, sizeof(dir));

    char path[1024];
    snprintf(path, sizeof(path), "%s\\dong_plugin_sdl.dll", dir);

    HMODULE mod = LoadLibraryA(path);
    if (!mod) {
        fprintf(stderr, "[DongDemoGpu] Plugin not found: %s\n", path);
        return NULL;
    }

    typedef const dong_plugin_vtable_t* (*get_api_fn)(void);
    get_api_fn fn = (get_api_fn)GetProcAddress(mod, "dong_plugin_get_api");
    if (!fn) {
        fprintf(stderr, "[DongDemoGpu] Plugin missing symbol dong_plugin_get_api: %s\n", path);
        FreeLibrary(mod);
        return NULL;
    }

    s_vtable = fn();
    if (s_vtable) {
        fprintf(stderr, "[DongDemoGpu] Plugin loaded: %s\n", path);
    }
    return s_vtable;
#else
    return NULL;
#endif
}

int dong_demo_gpu_init(DongDemoGpu* ctx, const DongDemoGpuConfig* cfg) {
    if (!ctx || !cfg) {
        return 0;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->width = cfg->width > 0 ? cfg->width : 960;
    ctx->height = cfg->height > 0 ? cfg->height : 540;

    fprintf(stderr, "[Dong] backend=gpu (in-dreaming/gpu native)\n");

    if (gpuPlatformInit() != GPU_SUCCESS) {
        fprintf(stderr, "[DongDemoGpu] gpuPlatformInit failed\n");
        return 0;
    }
    ctx->platform_ready = 1;

    GpuWindow window = NULL;
    GpuWindowDesc win_desc = {
        .title = cfg->title ? cfg->title : "Dong",
        .width = ctx->width,
        .height = ctx->height,
        .vsync = true,
        .resizable = true,
    };
    if (gpuCreateWindow(&win_desc, &window) != GPU_SUCCESS) {
        fprintf(stderr, "[DongDemoGpu] gpuCreateWindow failed\n");
        return 0;
    }
    /* Enables SDL_EVENT_TEXT_INPUT generation, matching the SDL backend's
     * SDL_StartTextInput() call at app creation (appcore/src/app.c). Without this, <input>
     * elements never receive typed characters under the GPU backend. */
    gpuWindowStartTextInput(window);

    /* Swapchain + layout must use drawable pixels (DPI-aware), not logical window units. */
    ctx->width = gpuWindowGetWidthInPixels(window);
    ctx->height = gpuWindowGetHeightInPixels(window);
    if (ctx->width == 0 || ctx->height == 0) {
        ctx->width = win_desc.width;
        ctx->height = win_desc.height;
    }
    fprintf(stderr, "[DongDemoGpu] drawable size=%ux%u (logical=%ux%u)\n",
            ctx->width, ctx->height, win_desc.width, win_desc.height);

    GpuDevice device = NULL;
    GpuDeviceDesc dev_desc = {.appName = "dong_demo"};
    if (gpuCreateDevice(&dev_desc, &device) != GPU_SUCCESS) {
        fprintf(stderr, "[DongDemoGpu] gpuCreateDevice failed\n");
        gpuDestroyWindow(window);
        return 0;
    }

    GpuCommandQueue queue = NULL;
    if (gpuGetQueue(device, GPU_QUEUE_TYPE_GRAPHICS, &queue) != GPU_SUCCESS) {
        fprintf(stderr, "[DongDemoGpu] gpuGetQueue failed\n");
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    DongGPUDriver* driver = dong_gpu_backend_create_driver();
    if (!driver) {
        fprintf(stderr, "[DongDemoGpu] dong_gpu_backend_create_driver failed\n");
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    dong_gpu_driver_set_external_device(driver, device, 0);
    dong_gpu_driver_set_embedded_mode(driver, 0);

    GpuSurface surface = NULL;
    if (gpuCreateSurface(device, window, GPU_SURFACE_TYPE_VULKAN, &surface) != GPU_SUCCESS) {
        fprintf(stderr, "[DongDemoGpu] gpuCreateSurface failed\n");
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    ctx->surface_fmt = (uint32_t)dong_demo_gpu_strip_srgb_format(gpuSurfaceGetPreferredFormat(surface));
    if (gpuSurfaceConfigure(surface, ctx->width, ctx->height, (GpuFormat)ctx->surface_fmt, true) !=
        GPU_SUCCESS) {
        fprintf(stderr, "[DongDemoGpu] gpuSurfaceConfigure failed\n");
        gpuDestroySurface(device, surface);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    // Surface must be wired before initialize() so pipelines compile for the actual
    // swapchain format (D3D12 is typically BGRA8, not RGBA8).
    dong_gpu_driver_set_external_swapchain(driver, surface, queue, 0);
    dong_gpu_driver_set_external_window(driver, window);

    if (!dong_gpu_driver_initialize(driver)) {
        fprintf(stderr, "[DongDemoGpu] dong_gpu_driver_initialize failed\n");
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    DongPlatform* platform = dong_platform_get();
    if (!platform) {
        fprintf(stderr, "[DongDemoGpu] dong_platform_get failed\n");
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuSurfaceUnconfigure(surface);
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }
    dong_platform_set_gpu_driver(platform, driver);

    DongImageDecoder* image_decoder = dong_gpu_image_decoder_create();
    if (image_decoder) {
        dong_platform_set_image_decoder(platform, image_decoder);
    } else {
        fprintf(stderr, "[DongDemoGpu] dong_gpu_image_decoder_create failed (images will not decode)\n");
    }

    ctx->window = window;
    ctx->device = device;
    ctx->surface = surface;
    ctx->queue = queue;
    ctx->driver = driver;
    ctx->image_decoder = image_decoder;
    return 1;
}

void dong_demo_gpu_shutdown(DongDemoGpu* ctx) {
    if (!ctx) {
        return;
    }

    if (ctx->queue) {
        gpuQueueWaitOnHost((GpuCommandQueue)ctx->queue);
    }

    if (ctx->image_decoder) {
        DongPlatform* platform = dong_platform_get();
        if (platform) {
            dong_platform_set_image_decoder(platform, NULL);
        }
        dong_gpu_image_decoder_destroy((DongImageDecoder*)ctx->image_decoder);
        ctx->image_decoder = NULL;
    }

    if (ctx->driver) {
        DongPlatform* platform = dong_platform_get();
        if (platform) {
            dong_platform_set_gpu_driver(platform, NULL);
        }
        dong_gpu_driver_shutdown(ctx->driver);
        dong_gpu_backend_destroy_driver(ctx->driver);
        ctx->driver = NULL;
    }

    if (ctx->surface && ctx->device) {
        gpuSurfaceUnconfigure((GpuSurface)ctx->surface);
        gpuDestroySurface((GpuDevice)ctx->device, (GpuSurface)ctx->surface);
        ctx->surface = NULL;
    }
    if (ctx->device) {
        gpuDestroyDevice((GpuDevice)ctx->device);
        ctx->device = NULL;
    }
    if (ctx->window) {
        gpuDestroyWindow((GpuWindow)ctx->window);
        ctx->window = NULL;
    }
    if (ctx->platform_ready) {
        gpuPlatformShutdown();
        ctx->platform_ready = 0;
    }
    ctx->queue = NULL;
}

static void dong_demo_gpu_update_cursor(DongDemoGpu* ctx, dong_engine_t* engine) {
    if (!ctx || !ctx->window || !engine) {
        return;
    }
    const char* css_cursor = dong_engine_get_cursor_at(engine, ctx->mouse_x, ctx->mouse_y);
    gpuWindowSetCursor((GpuWindow)ctx->window, css_cursor);
}

int dong_demo_gpu_poll_events(DongDemoGpu* ctx, dong_engine_t* engine) {
    if (!ctx) {
        return 0;
    }

    GpuPlatformEvent ev;
    while (gpuPollEvent(&ev)) {
        if (ev.type == GPU_PLATFORM_EVENT_QUIT) {
            ctx->quit = 1;
            break;
        }
        if (ev.type == GPU_PLATFORM_EVENT_RESIZE) {
            if (ctx->window) {
                ctx->width = gpuWindowGetWidthInPixels((GpuWindow)ctx->window);
                ctx->height = gpuWindowGetHeightInPixels((GpuWindow)ctx->window);
            } else {
                ctx->width = ev.resize.width;
                ctx->height = ev.resize.height;
            }
            if (ctx->width == 0 || ctx->height == 0) {
                ctx->width = ev.resize.width;
                ctx->height = ev.resize.height;
            }
            if (ctx->surface) {
                gpuSurfaceConfigure((GpuSurface)ctx->surface, ctx->width, ctx->height,
                                    (GpuFormat)ctx->surface_fmt, true);
            }
            if (engine) {
                (void)dong_engine_resize(engine, ctx->width, ctx->height);
            }
        }
        if (engine) {
            switch (ev.type) {
            case GPU_PLATFORM_EVENT_MOUSE_MOVE:
                ctx->mouse_x = ev.mouse.x;
                ctx->mouse_y = ev.mouse.y;
                (void)dong_engine_send_mouse_move(engine, ev.mouse.x, ev.mouse.y);
                break;
            case GPU_PLATFORM_EVENT_MOUSE_BUTTON_DOWN:
            case GPU_PLATFORM_EVENT_MOUSE_BUTTON_UP:
                ctx->mouse_x = ev.mouse.x;
                ctx->mouse_y = ev.mouse.y;
                (void)dong_engine_send_mouse_move(engine, ev.mouse.x, ev.mouse.y);
                (void)dong_engine_send_mouse_button(
                    engine, ev.mouse.button, ev.type == GPU_PLATFORM_EVENT_MOUSE_BUTTON_DOWN);
                break;
            case GPU_PLATFORM_EVENT_KEY_DOWN:
            case GPU_PLATFORM_EVENT_KEY_UP:
                (void)dong_engine_send_key(engine, ev.key.keycode, ev.type == GPU_PLATFORM_EVENT_KEY_DOWN);
                break;
            case GPU_PLATFORM_EVENT_MOUSE_WHEEL:
                ctx->mouse_x = ev.wheel.x;
                ctx->mouse_y = ev.wheel.y;
                (void)dong_engine_send_mouse_move(engine, ev.wheel.x, ev.wheel.y);
                (void)dong_engine_send_mouse_wheel(engine, ev.wheel.delta_x, ev.wheel.delta_y);
                break;
            case GPU_PLATFORM_EVENT_TEXT_INPUT:
                (void)dong_engine_send_text(engine, ev.text_input.text);
                break;
            default:
                break;
            }
        }
    }
    dong_demo_gpu_update_cursor(ctx, engine);
    return ctx->quit ? 0 : 1;
}

void dong_demo_gpu_present(DongDemoGpu* ctx, dong_engine_t* engine) {
    (void)ctx;
    if (engine) {
        (void)dong_engine_tick(engine);
    }
}

dong_engine_t* dong_demo_gpu_create_engine(DongDemoGpu* ctx) {
    if (!ctx || !ctx->device || !ctx->window) {
        return NULL;
    }

    ctx->width = gpuWindowGetWidthInPixels((GpuWindow)ctx->window);
    ctx->height = gpuWindowGetHeightInPixels((GpuWindow)ctx->window);
    if (ctx->width == 0 || ctx->height == 0) {
        ctx->width = 960;
        ctx->height = 540;
    }

    dong_engine_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.api_version = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.plugin = dong_demo_gpu_try_load_plugin();
    desc.plugin_user = NULL;
    desc.html = NULL;
    desc.width = ctx->width;
    desc.height = ctx->height;

    dong_engine_t* engine = NULL;
    if (dong_engine_create(&desc, &engine) != DONG_OK || !engine) {
        fprintf(stderr, "[DongDemoGpu] dong_engine_create failed\n");
        return NULL;
    }

    if (dong_engine_set_gpu(engine, ctx->device, ctx->window) != DONG_OK) {
        fprintf(stderr, "[DongDemoGpu] dong_engine_set_gpu failed\n");
        dong_engine_destroy(engine);
        return NULL;
    }

    fprintf(stderr, "[DongDemoGpu] dong_engine created\n");
    return engine;
}

void dong_demo_gpu_destroy_engine(dong_engine_t* engine) {
    if (engine) {
        dong_engine_destroy(engine);
    }
}

int dong_demo_gpu_load_html_file(dong_engine_t* engine, const char* path) {
    if (!engine || !path) {
        return 0;
    }

    char dir[1024];
    extract_dir_from_path(path, dir, sizeof(dir));
    if (dir[0]) {
        (void)dong_engine_set_resource_root(engine, dir);
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[DongDemoGpu] Failed to open file: %s\n", path);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0) {
        fclose(f);
        return 0;
    }

    char* content = (char*)malloc((size_t)size + 1);
    if (!content) {
        fclose(f);
        return 0;
    }

    size_t read = fread(content, 1, (size_t)size, f);
    content[read] = '\0';
    fclose(f);

    int ok = (dong_engine_load_html(engine, content) == DONG_OK) ? 1 : 0;
    free(content);
    return ok;
}
