#include "dong_gpu_app_platform.h"

#include "dong_backend.h"
#include "dong_gpu_backend_driver.h"
#include "dong_gpu_image_decoder.h"
#include "dong_platform.h"
#include "dong.h"
#include "dong_ui_graph.h"

#include "gpu/gpu.h"
#include "gpu/platform/gpu_platform.h"
#include "gpu/platform/gpu_surface.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define DONG_MOUSE_BUTTON_LEFT 1
#define DONG_MOUSE_BUTTON_RIGHT 3

/* Dong's UI pipeline produces already display-ready (gamma-encoded) colors, not linear light
 * values. Configuring the swapchain with an _SRGB surface format makes the GPU's render-output
 * stage re-apply a linear->sRGB encode on every write, double-gamma-correcting the already-
 * encoded UI colors into a washed-out/hazy image. gpuSurfaceGetPreferredFormat() commonly
 * returns an _SRGB variant, so strip it back to the plain UNORM equivalent before configuring
 * the surface (mirrors gpu_gpu_driver.hpp's stripSrgbFormat, kept local since this is plain C). */
static GpuFormat dong_gpu_app_strip_srgb_format(GpuFormat fmt) {
    switch (fmt) {
    case GPU_FORMAT_RGBA8_UNORM_SRGB: return GPU_FORMAT_RGBA8_UNORM;
    case GPU_FORMAT_BGRA8_UNORM_SRGB: return GPU_FORMAT_BGRA8_UNORM;
    default: return fmt;
    }
}

static uint32_t map_gpu_key_to_scancode(uint32_t keycode) {
#if defined(_WIN32)
    switch (keycode) {
    case 'W':
    case 'w': return 26;
    case 'A':
    case 'a': return 4;
    case 'S':
    case 's': return 22;
    case 'D':
    case 'd': return 7;
    case 'Q':
    case 'q': return 20;
    case 'E':
    case 'e': return 8;
    case VK_SPACE: return 44;
    case VK_LCONTROL:
    case VK_RCONTROL: return 224;
    case VK_LSHIFT:
    case VK_RSHIFT: return 225;
    default: return 0;
    }
#else
    (void)keycode;
    return 0;
#endif
}

static uint32_t normalize_keycode(uint32_t keycode) {
    if (keycode >= 'A' && keycode <= 'Z') {
        return keycode + ('a' - 'A');
    }
    return keycode;
}

static int poll_windows_text_event(DongGpuAppPlatform* ctx, dong_app_event_t* out_event) {
#if defined(_WIN32)
    if (!ctx || !ctx->text_input_enabled || !ctx->window || !out_event) {
        return 0;
    }
    HWND hwnd = (HWND)gpuWindowGetHandle((GpuWindow)ctx->window);
    if (!hwnd) {
        return 0;
    }
    MSG msg;
    if (PeekMessageA(&msg, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE)) {
        static char text_buf[8];
        if (msg.wParam < 128) {
            text_buf[0] = (char)msg.wParam;
            text_buf[1] = '\0';
            out_event->type = DONG_APP_EVENT_TEXT;
            out_event->text.text = text_buf;
            return 1;
        }
    }
#else
    (void)ctx;
    (void)out_event;
#endif
    return 0;
}

int dong_gpu_app_platform_init(DongGpuAppPlatform* ctx, const DongGpuAppPlatformConfig* cfg) {
    if (!ctx || !cfg) {
        return 0;
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->width = cfg->width > 0 ? cfg->width : 800;
    ctx->height = cfg->height > 0 ? cfg->height : 600;

    fprintf(stderr, "[DongApp] backend=gpu (in-dreaming/gpu native)\n");

    if (gpuPlatformInit() != GPU_SUCCESS) {
        fprintf(stderr, "[DongGpuAppPlatform] gpuPlatformInit failed\n");
        return 0;
    }
    ctx->platform_ready = 1;

    GpuWindow window = NULL;
    GpuWindowDesc win_desc = {
        .title = cfg->title ? cfg->title : "Dong",
        .width = ctx->width,
        .height = ctx->height,
        .vsync = cfg->vsync ? true : false,
        .resizable = cfg->resizable ? true : false,
        .fullscreen = cfg->fullscreen ? true : false,
    };
    if (gpuCreateWindow(&win_desc, &window) != GPU_SUCCESS) {
        fprintf(stderr, "[DongGpuAppPlatform] gpuCreateWindow failed\n");
        return 0;
    }
    /* Enables SDL_EVENT_TEXT_INPUT generation, matching the SDL backend's
     * SDL_StartTextInput() call at app creation (appcore/src/app.c). */
    gpuWindowStartTextInput(window);

    ctx->width = gpuWindowGetWidthInPixels(window);
    ctx->height = gpuWindowGetHeightInPixels(window);
    if (ctx->width == 0 || ctx->height == 0) {
        ctx->width = win_desc.width;
        ctx->height = win_desc.height;
    }

    GpuDevice device = NULL;
    GpuDeviceDesc dev_desc = {.appName = "dong_app"};
    if (gpuCreateDevice(&dev_desc, &device) != GPU_SUCCESS) {
        fprintf(stderr, "[DongGpuAppPlatform] gpuCreateDevice failed\n");
        gpuDestroyWindow(window);
        return 0;
    }

    GpuSurface surface = NULL;
    if (gpuCreateSurface(device, window, GPU_SURFACE_TYPE_VULKAN, &surface) != GPU_SUCCESS) {
        fprintf(stderr, "[DongGpuAppPlatform] gpuCreateSurface failed\n");
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    ctx->surface_fmt = (uint32_t)dong_gpu_app_strip_srgb_format(gpuSurfaceGetPreferredFormat(surface));
    if (gpuSurfaceConfigure(surface, ctx->width, ctx->height, (GpuFormat)ctx->surface_fmt, win_desc.vsync) !=
        GPU_SUCCESS) {
        fprintf(stderr, "[DongGpuAppPlatform] gpuSurfaceConfigure failed\n");
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    GpuCommandQueue queue = NULL;
    if (gpuGetQueue(device, GPU_QUEUE_TYPE_GRAPHICS, &queue) != GPU_SUCCESS) {
        fprintf(stderr, "[DongGpuAppPlatform] gpuGetQueue failed\n");
        gpuSurfaceUnconfigure(surface);
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    DongGPUDriver* driver = dong_gpu_backend_create_driver();
    if (!driver) {
        fprintf(stderr, "[DongGpuAppPlatform] dong_gpu_backend_create_driver failed\n");
        gpuSurfaceUnconfigure(surface);
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    dong_gpu_driver_set_external_device(driver, device, 0);
    dong_gpu_driver_set_external_swapchain(driver, surface, queue, 0);
    dong_gpu_driver_set_external_window(driver, window);
    dong_gpu_driver_set_embedded_mode(driver, 0);
    if (!dong_gpu_driver_initialize(driver)) {
        fprintf(stderr, "[DongGpuAppPlatform] dong_gpu_driver_initialize failed\n");
        dong_gpu_backend_destroy_driver(driver);
        gpuSurfaceUnconfigure(surface);
        gpuDestroySurface(device, surface);
        gpuDestroyDevice(device);
        gpuDestroyWindow(window);
        return 0;
    }

    DongPlatform* platform = dong_platform_get();
    if (!platform) {
        fprintf(stderr, "[DongGpuAppPlatform] dong_platform_get failed\n");
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
        fprintf(stderr, "[DongGpuAppPlatform] dong_gpu_image_decoder_create failed (images will not decode)\n");
    }

    ctx->window = window;
    ctx->device = device;
    ctx->surface = surface;
    ctx->queue = queue;
    ctx->driver = driver;
    ctx->image_decoder = image_decoder;
    ctx->text_input_enabled = 1;
    return 1;
}

void dong_gpu_app_platform_shutdown(DongGpuAppPlatform* ctx) {
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

int dong_gpu_app_platform_poll_event(DongGpuAppPlatform* ctx, dong_app_event_t* out_event,
                                     dong_engine_t* engine) {
    if (!ctx || !out_event) {
        return 0;
    }

    memset(out_event, 0, sizeof(*out_event));
    out_event->type = DONG_APP_EVENT_NONE;

    GpuPlatformEvent ev;
    if (!gpuPollEvent(&ev)) {
        if (poll_windows_text_event(ctx, out_event)) {
            return ctx->quit ? 0 : 1;
        }
        return ctx->quit ? 0 : 1;
    }

    switch (ev.type) {
    case GPU_PLATFORM_EVENT_QUIT:
        ctx->quit = 1;
        out_event->type = DONG_APP_EVENT_QUIT;
        break;
    case GPU_PLATFORM_EVENT_RESIZE:
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
        out_event->type = DONG_APP_EVENT_WINDOW_RESIZED;
        out_event->window_resized.width = ctx->width;
        out_event->window_resized.height = ctx->height;
        if (engine) {
            (void)dong_engine_resize(engine, ctx->width, ctx->height);
        }
        break;
    case GPU_PLATFORM_EVENT_MOUSE_MOVE:
        out_event->type = DONG_APP_EVENT_MOUSE_MOVE;
        out_event->mouse_move.x = ev.mouse.x;
        out_event->mouse_move.y = ev.mouse.y;
        break;
    case GPU_PLATFORM_EVENT_MOUSE_BUTTON_DOWN:
    case GPU_PLATFORM_EVENT_MOUSE_BUTTON_UP:
        out_event->type = DONG_APP_EVENT_MOUSE_BUTTON;
        out_event->mouse_button.button = (int32_t)ev.mouse.button;
        out_event->mouse_button.pressed = (ev.type == GPU_PLATFORM_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;
        out_event->mouse_button.x = ev.mouse.x;
        out_event->mouse_button.y = ev.mouse.y;
        if (ev.mouse.button == DONG_MOUSE_BUTTON_RIGHT) {
            ctx->right_mouse_down = out_event->mouse_button.pressed;
        }
        break;
    case GPU_PLATFORM_EVENT_KEY_DOWN:
    case GPU_PLATFORM_EVENT_KEY_UP:
        out_event->type = DONG_APP_EVENT_KEY;
        out_event->key.key_code = normalize_keycode(ev.key.keycode);
        out_event->key.scancode = map_gpu_key_to_scancode(ev.key.keycode);
        out_event->key.pressed = (ev.type == GPU_PLATFORM_EVENT_KEY_DOWN) ? 1 : 0;
        out_event->key.repeat = 0;
        break;
    case GPU_PLATFORM_EVENT_MOUSE_WHEEL:
        out_event->type = DONG_APP_EVENT_MOUSE_WHEEL;
        out_event->mouse_wheel.x = ev.wheel.x;
        out_event->mouse_wheel.y = ev.wheel.y;
        out_event->mouse_wheel.delta_x = ev.wheel.delta_x;
        out_event->mouse_wheel.delta_y = ev.wheel.delta_y;
        break;
    case GPU_PLATFORM_EVENT_TEXT_INPUT:
        if (ctx->text_input_enabled) {
            out_event->type = DONG_APP_EVENT_TEXT;
            /* ev.text_input.text lives in a local stack GpuPlatformEvent here, but out_event
             * must remain valid for the caller after this function returns - copy into a
             * static buffer (single-threaded event loop, consumed before next poll call). */
            static char text_buf[sizeof(ev.text_input.text)];
            strncpy(text_buf, ev.text_input.text, sizeof(text_buf) - 1);
            text_buf[sizeof(text_buf) - 1] = '\0';
            out_event->text.text = text_buf;
        }
        break;
    default:
        break;
    }

    return ctx->quit ? 0 : 1;
}

void dong_gpu_app_platform_present_clear(DongGpuAppPlatform* ctx) {
    if (!ctx || !ctx->device || !ctx->surface || !ctx->queue) {
        return;
    }

    GpuSurfaceTexture backbuffer = NULL;
    if (gpuSurfaceAcquireNextImage((GpuSurface)ctx->surface, &backbuffer) != GPU_SUCCESS) {
        return;
    }

    GpuCommandEncoder encoder = gpuBeginCommandEncoder((GpuDevice)ctx->device, (GpuCommandQueue)ctx->queue);
    if (!encoder) {
        gpuSurfaceTextureRelease(backbuffer);
        return;
    }

    GpuRenderPassColorAttachment color = {0};
    color.attachment = backbuffer;
    color.loadOp = GPU_LOAD_OP_CLEAR;
    color.storeOp = GPU_STORE_OP_STORE;
    color.clearValue[0] = 0.1f;
    color.clearValue[1] = 0.1f;
    color.clearValue[2] = 0.1f;
    color.clearValue[3] = 1.0f;

    GpuRenderPassDesc rp = {0};
    rp.colorAttachmentCount = 1;
    rp.colorAttachments = &color;

    GpuRenderPassEncoder pass = gpuCmdBeginRenderPass(encoder, &rp);
    if (pass) {
        gpuCmdEndRenderPass(pass);
    }

    GpuCommandBuffer cmd = gpuFinishCommandEncoder(encoder);
    if (cmd) {
        gpuQueueSubmit((GpuCommandQueue)ctx->queue, 1, &cmd);
    }

    gpuSurfacePresent((GpuSurface)ctx->surface);
    gpuSurfaceTextureRelease(backbuffer);
}

void dong_gpu_app_platform_set_text_input(DongGpuAppPlatform* ctx, int enable) {
    if (!ctx) {
        return;
    }
    ctx->text_input_enabled = enable ? 1 : 0;
}
