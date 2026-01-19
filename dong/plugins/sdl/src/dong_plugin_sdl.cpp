#include "dong_plugin_api.h"
#include "video_ffmpeg.h"

#include <SDL3/SDL.h>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>

#include <cstring>
#include <cstdlib>


namespace {

// =============================================================================
// Log
// =============================================================================
static void sdl_log(void* /*user*/, dong_log_level_t level, const char* msg) {
    if (!msg) {
        msg = "";
    }

    const char* prefix = "[dong]";
    switch (level) {
        case DONG_LOG_TRACE: prefix = "[dong][TRACE]"; break;
        case DONG_LOG_DEBUG: prefix = "[dong][DEBUG]"; break;
        case DONG_LOG_INFO:  prefix = "[dong][INFO]";  break;
        case DONG_LOG_WARN:  prefix = "[dong][WARN]";  break;
        case DONG_LOG_ERROR: prefix = "[dong][ERROR]"; break;
        default: break;
    }

    SDL_Log("%s %s", prefix, msg);
}

// =============================================================================
// Time
// =============================================================================
static uint64_t sdl_now_ns(void* /*user*/) {
    const uint64_t freq = (uint64_t) SDL_GetPerformanceFrequency();
    const uint64_t cnt = (uint64_t) SDL_GetPerformanceCounter();
    if (freq == 0) {
        return 0;
    }
    return (cnt * 1000000000ull) / freq;
}

// =============================================================================
// Window
// =============================================================================
static dong_window_t* sdl_window_create(void* /*user*/, const dong_window_desc_t* desc) {
    if (!desc) {
        return nullptr;
    }

    const char* title = desc->title ? desc->title : "Dong";
    int w = (int)desc->width;
    int h = (int)desc->height;
    if (w <= 0) w = 960;
    if (h <= 0) h = 540;

    SDL_Window* window = SDL_CreateWindow(title, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (!window) {
        SDL_Log("[dong_plugin_sdl] SDL_CreateWindow failed: %s", SDL_GetError());
        return nullptr;
    }

    return reinterpret_cast<dong_window_t*>(window);
}

static void sdl_window_destroy(void* /*user*/, dong_window_t* window) {
    if (window) {
        SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(window));
    }
}

static void sdl_window_get_size(void* /*user*/, dong_window_t* window, uint32_t* out_w, uint32_t* out_h) {
    if (!window) {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return;
    }
    int w = 0, h = 0;
    SDL_GetWindowSize(reinterpret_cast<SDL_Window*>(window), &w, &h);
    if (out_w) *out_w = (uint32_t)w;
    if (out_h) *out_h = (uint32_t)h;
}

// =============================================================================
// Input
// =============================================================================
static int sdl_poll_event(void* /*user*/, dong_input_event_t* out_event) {
    if (!out_event) {
        return 0;
    }
    std::memset(out_event, 0, sizeof(*out_event));

    SDL_Event e;
    if (!SDL_PollEvent(&e)) {
        return 0;
    }

    switch (e.type) {
        case SDL_EVENT_QUIT:
            out_event->type = DONG_INPUT_EVENT_QUIT;
            return 1;

        case SDL_EVENT_WINDOW_RESIZED: {
            out_event->type = DONG_INPUT_EVENT_WINDOW_RESIZE;
            SDL_Window* win = SDL_GetWindowFromID(e.window.windowID);
            int drawable_w = 0;
            int drawable_h = 0;
            if (win) {
                SDL_GetWindowSizeInPixels(win, &drawable_w, &drawable_h);
            }
            // 优先使用像素尺寸（drawable），避免高 DPI 下 View 与 swapchain 尺寸不一致
            if (drawable_w > 0 && drawable_h > 0) {
                out_event->a = (uint32_t)drawable_w;
                out_event->b = (uint32_t)drawable_h;
            } else {
                out_event->a = (uint32_t)e.window.data1; // width (logical)
                out_event->b = (uint32_t)e.window.data2; // height (logical)
            }
            return 1;
        }


        case SDL_EVENT_MOUSE_MOTION: {
            out_event->type = DONG_INPUT_EVENT_MOUSE_MOVE;
            SDL_Window* win = SDL_GetWindowFromID(e.motion.windowID);
            float dpr = 1.0f;
            if (win) {
                int logical_w = 0, logical_h = 0;
                int drawable_w = 0, drawable_h = 0;
                SDL_GetWindowSize(win, &logical_w, &logical_h);
                SDL_GetWindowSizeInPixels(win, &drawable_w, &drawable_h);
                if (logical_w > 0 && drawable_w > 0) {
                    dpr = (float)drawable_w / (float)logical_w;
                }
            }
            out_event->x = (float)e.motion.x * dpr;
            out_event->y = (float)e.motion.y * dpr;
            return 1;
        }


        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            out_event->type = DONG_INPUT_EVENT_MOUSE_BUTTON;
            out_event->a = (uint32_t)e.button.button; // button
            out_event->b = (uint32_t)(e.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1u : 0u); // down?

            SDL_Window* win = SDL_GetWindowFromID(e.button.windowID);
            float dpr = 1.0f;
            if (win) {
                int logical_w = 0, logical_h = 0;
                int drawable_w = 0, drawable_h = 0;
                SDL_GetWindowSize(win, &logical_w, &logical_h);
                SDL_GetWindowSizeInPixels(win, &drawable_w, &drawable_h);
                if (logical_w > 0 && drawable_w > 0) {
                    dpr = (float)drawable_w / (float)logical_w;
                }
            }
            out_event->x = (float)e.button.x * dpr;
            out_event->y = (float)e.button.y * dpr;
            return 1;
        }


        case SDL_EVENT_MOUSE_WHEEL: {
            out_event->type = DONG_INPUT_EVENT_MOUSE_WHEEL;
            // SDL: wheel.y 正值通常表示“向上”；dong 约定 delta_y 正值=向下（内容向上移动，scroll_y 增加）
            // 注意：wheel delta 不是像素坐标，不应按 DPR 缩放。
            out_event->dx = (float)e.wheel.x;
            out_event->dy = (float)(-e.wheel.y);
            return 1;
        }



        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            out_event->type = DONG_INPUT_EVENT_KEY;
            out_event->a = (uint32_t) e.key.key;
            out_event->b = (uint32_t) (e.type == SDL_EVENT_KEY_DOWN ? 1u : 0u);
            return 1;

        case SDL_EVENT_TEXT_INPUT:
            out_event->type = DONG_INPUT_EVENT_TEXT;
            out_event->text = e.text.text;
            return 1;

        default:
            break;
    }

    return 0;
}

// =============================================================================
// Renderer
// =============================================================================
struct SDLGPUDevice {
    SDL_GPUDevice* device = nullptr;
    SDL_Window* window = nullptr;
};

static dong_gpu_device_t* sdl_renderer_init(void* /*user*/, dong_window_t* window) {
    if (!window) {
        SDL_Log("[dong_plugin_sdl] renderer_init: window is null");
        return nullptr;
    }

    SDL_Window* sdl_window = reinterpret_cast<SDL_Window*>(window);

    SDL_GPUDevice* device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        true, // debug mode
        nullptr
    );

    if (!device) {
        SDL_Log("[dong_plugin_sdl] SDL_CreateGPUDevice failed: %s", SDL_GetError());
        return nullptr;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, sdl_window)) {
        SDL_Log("[dong_plugin_sdl] SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
        SDL_DestroyGPUDevice(device);
        return nullptr;
    }

    // 允许通过环境变量控制 swapchain present mode / frames-in-flight（性能排查常用）。
    // - DONG_GPU_PRESENT_MODE=mailbox|vsync|immediate
    // - DONG_GPU_FRAMES_IN_FLIGHT=1|2|3
    if (const char* v = std::getenv("DONG_GPU_FRAMES_IN_FLIGHT")) {
        const int n = std::atoi(v);
        if (n >= 1 && n <= 3) {
            SDL_SetGPUAllowedFramesInFlight(device, (Uint32)n);
            SDL_Log("[dong_plugin_sdl] AllowedFramesInFlight=%d", n);
        } else {
            SDL_Log("[dong_plugin_sdl] Invalid DONG_GPU_FRAMES_IN_FLIGHT=%s (expected 1..3)", v);
        }
    }

    SDL_GPUPresentMode mode = SDL_GPU_PRESENTMODE_MAILBOX;
    bool mode_from_env = false;
    if (const char* pm = std::getenv("DONG_GPU_PRESENT_MODE")) {
        mode_from_env = true;
        if (std::strcmp(pm, "mailbox") == 0) {
            mode = SDL_GPU_PRESENTMODE_MAILBOX;
        } else if (std::strcmp(pm, "vsync") == 0) {
            mode = SDL_GPU_PRESENTMODE_VSYNC;
        } else if (std::strcmp(pm, "immediate") == 0) {
            mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
        } else {
            SDL_Log("[dong_plugin_sdl] Invalid DONG_GPU_PRESENT_MODE=%s (use mailbox|vsync|immediate)", pm);
            mode_from_env = false;
            mode = SDL_GPU_PRESENTMODE_MAILBOX;
        }
    }

    if (mode_from_env) {
        if (!SDL_SetGPUSwapchainParameters(device, sdl_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, mode)) {
            SDL_Log("[dong_plugin_sdl] PresentMode request failed, falling back to VSYNC");
            SDL_SetGPUSwapchainParameters(device, sdl_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
        }
    } else {
        // 默认策略：优先 MAILBOX（避免某些平台/windowed vsync 被节流到 30Hz），不支持则回退 VSYNC。
        if (!SDL_SetGPUSwapchainParameters(device, sdl_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX)) {
            SDL_Log("[dong_plugin_sdl] MAILBOX present mode not supported, falling back to VSYNC");
            SDL_SetGPUSwapchainParameters(device, sdl_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
        }
    }


    auto* ctx = new SDLGPUDevice();
    ctx->device = device;
    ctx->window = sdl_window;

    SDL_Log("[dong_plugin_sdl] GPU device created successfully");

    return reinterpret_cast<dong_gpu_device_t*>(ctx);
}

static void sdl_renderer_shutdown(void* /*user*/, dong_gpu_device_t* device) {
    if (!device) return;
    auto* ctx = reinterpret_cast<SDLGPUDevice*>(device);
    if (ctx->device) {
        SDL_ReleaseWindowFromGPUDevice(ctx->device, ctx->window);
        SDL_DestroyGPUDevice(ctx->device);
    }
    delete ctx;
}

static int sdl_renderer_begin_frame(void* /*user*/, dong_gpu_device_t* device) {
    if (!device) return 0;
    // Frame begin is handled in submit for now
    return 1;
}

static int sdl_renderer_submit(void* /*user*/, dong_gpu_device_t* device, const dong_renderer_cmd_stream_t* /*stream*/) {
    if (!device) return 0;
    auto* ctx = reinterpret_cast<SDLGPUDevice*>(device);

    // For now, just clear the screen with a color
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(ctx->device);
    if (!cmd) {
        SDL_Log("[dong_plugin_sdl] SDL_AcquireGPUCommandBuffer failed");
        return 0;
    }

    SDL_GPUTexture* swapchain_texture = nullptr;
    uint32_t sw_w = 0, sw_h = 0;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, ctx->window, &swapchain_texture, &sw_w, &sw_h)) {
        SDL_Log("[dong_plugin_sdl] SDL_WaitAndAcquireGPUSwapchainTexture failed");
        SDL_SubmitGPUCommandBuffer(cmd);
        return 0;
    }

    if (swapchain_texture) {
        SDL_GPUColorTargetInfo color_target = {};
        color_target.texture = swapchain_texture;
        color_target.load_op = SDL_GPU_LOADOP_CLEAR;
        color_target.store_op = SDL_GPU_STOREOP_STORE;
        color_target.clear_color = {0.1f, 0.1f, 0.15f, 1.0f};

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_target, 1, nullptr);
        if (pass) {
            // TODO: Execute command stream here
            SDL_EndGPURenderPass(pass);
        }
    }

    SDL_SubmitGPUCommandBuffer(cmd);
    return 1;
}

static int sdl_renderer_end_frame(void* /*user*/, dong_gpu_device_t* /*device*/) {
    // Frame end is handled in submit for now
    return 1;
}

// =============================================================================
// Native handles
// =============================================================================
static void* sdl_get_native_window_handle(void* /*user*/, dong_window_t* window) {
    return reinterpret_cast<void*>(window);
}

static void* sdl_get_native_gpu_device(void* /*user*/, dong_gpu_device_t* device) {
    if (!device) return nullptr;
    auto* ctx = reinterpret_cast<SDLGPUDevice*>(device);
    return ctx->device;
}

// =============================================================================
// vtable
// =============================================================================

static constexpr dong_plugin_caps_t k_caps =
    (dong_plugin_caps_t)(DONG_PLUGIN_CAP_LOG |
                         DONG_PLUGIN_CAP_TIME |
                         DONG_PLUGIN_CAP_WINDOW |
                         DONG_PLUGIN_CAP_INPUT |
                         DONG_PLUGIN_CAP_RENDERER
#if defined(DONG_PLUGIN_SDL_HAS_FFMPEG) && DONG_PLUGIN_SDL_HAS_FFMPEG
                         | DONG_PLUGIN_CAP_VIDEO
#endif
    );

static const dong_plugin_vtable_t g_vtable = {
    /*info*/ {
        DONG_PLUGIN_API_VERSION,
        k_caps
    },

    /*log*/ sdl_log,
    /*now_ns*/ sdl_now_ns,

    /*window_create*/ sdl_window_create,
    /*window_destroy*/ sdl_window_destroy,
    /*window_get_size*/ sdl_window_get_size,

    /*poll_event*/ sdl_poll_event,

    /*renderer_init*/ sdl_renderer_init,
    /*renderer_shutdown*/ sdl_renderer_shutdown,
    /*renderer_begin_frame*/ sdl_renderer_begin_frame,
    /*renderer_submit*/ sdl_renderer_submit,
    /*renderer_end_frame*/ sdl_renderer_end_frame,

    /*get_native_window_handle*/ sdl_get_native_window_handle,
    /*get_native_gpu_device*/ sdl_get_native_gpu_device,

#if defined(DONG_PLUGIN_SDL_HAS_FFMPEG) && DONG_PLUGIN_SDL_HAS_FFMPEG
    /*video_open*/ sdl_video_open,
    /*video_close*/ sdl_video_close,
    /*video_get_metadata*/ sdl_video_get_metadata,
    /*video_read_frame*/ sdl_video_read_frame,
    /*video_seek*/ sdl_video_seek,
#else
    /*video_open*/ nullptr,
    /*video_close*/ nullptr,
    /*video_get_metadata*/ nullptr,
    /*video_read_frame*/ nullptr,
    /*video_seek*/ nullptr,
#endif
};


} // namespace

extern "C" {

#if defined(_WIN32)
__declspec(dllexport)
#endif
const dong_plugin_vtable_t* dong_plugin_get_api(void) {
    return &g_vtable;
}

} // extern "C"
