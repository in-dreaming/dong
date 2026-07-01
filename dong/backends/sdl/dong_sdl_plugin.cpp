#include "dong_sdl_plugin.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "dong_sdl_platform.h"
#include "dong_gpu_driver.h"
#include "dong_sdl_gpu_formats.h"

#include <cstring>
#include <string>

// =============================================================================
// SDL Plugin Internal State
// =============================================================================

struct SDLPluginState {
    dong_sdl_plugin_config_t config;
    std::string last_error;

    // SDL resources
    SDL_Window* window = nullptr;
    SDL_GPUDevice* gpu_device = nullptr;

    // DongGPUDriver wrapper
    DongGPUDriver* gpu_driver = nullptr;

    // Video subsystem (FFmpeg integration)
    int video_initialized = 0;

    int initialized = 0;
};

// =============================================================================
// Plugin Callback Implementations
// =============================================================================

static void sdl_plugin_log(void* user, dong_log_level_t level, const char* msg) {
    (void)user;
    SDL_LogPriority priority = SDL_LOG_PRIORITY_INFO;
    switch (level) {
        case DONG_LOG_TRACE: priority = SDL_LOG_PRIORITY_VERBOSE; break;
        case DONG_LOG_DEBUG: priority = SDL_LOG_PRIORITY_DEBUG; break;
        case DONG_LOG_INFO:  priority = SDL_LOG_PRIORITY_INFO; break;
        case DONG_LOG_WARN:  priority = SDL_LOG_PRIORITY_WARN; break;
        case DONG_LOG_ERROR: priority = SDL_LOG_PRIORITY_ERROR; break;
    }
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, priority, "%s", msg);
}

static uint64_t sdl_plugin_now_ns(void* user) {
    (void)user;
    return SDL_GetTicksNS();
}

static dong_window_t* sdl_plugin_window_create(void* user, const dong_window_desc_t* desc) {
    SDLPluginState* state = static_cast<SDLPluginState*>(user);

    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (state->config.window_resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    SDL_Window* window = SDL_CreateWindow(
        desc->title ? desc->title : state->config.window_title,
        (int)desc->width,
        (int)desc->height,
        flags);

    if (!window) {
        state->last_error = SDL_GetError();
        return nullptr;
    }

    state->window = window;

    // Show window
    SDL_ShowWindow(window);

    // Return opaque handle (just the SDL_Window pointer)
    return reinterpret_cast<dong_window_t*>(window);
}

static void sdl_plugin_window_destroy(void* user, dong_window_t* window) {
    SDLPluginState* state = static_cast<SDLPluginState*>(user);
    SDL_Window* sdl_window = reinterpret_cast<SDL_Window*>(window);

    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
    }

    if (state->window == sdl_window) {
        state->window = nullptr;
    }
}

static void sdl_plugin_window_get_size(void* user, dong_window_t* window, uint32_t* out_w, uint32_t* out_h) {
    (void)user;
    SDL_Window* sdl_window = reinterpret_cast<SDL_Window*>(window);

    int w, h;
    SDL_GetWindowSize(sdl_window, &w, &h);

    if (out_w) *out_w = (uint32_t)w;
    if (out_h) *out_h = (uint32_t)h;
}

static int sdl_plugin_poll_event(void* user, dong_input_event_t* out_event) {
    SDLPluginState* state = static_cast<SDLPluginState*>(user);
    (void)state;

    SDL_Event sdl_event;
    if (!SDL_PollEvent(&sdl_event)) {
        return 0;
    }

    // Convert SDL event to dong event
    memset(out_event, 0, sizeof(*out_event));

    switch (sdl_event.type) {
        case SDL_EVENT_QUIT:
            out_event->type = DONG_INPUT_EVENT_QUIT;
            return 1;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            out_event->type = DONG_INPUT_EVENT_QUIT;
            return 1;

        case SDL_EVENT_WINDOW_RESIZED:
            out_event->type = DONG_INPUT_EVENT_WINDOW_RESIZE;
            out_event->a = (uint32_t)sdl_event.window.data1;  // width
            out_event->b = (uint32_t)sdl_event.window.data2;  // height
            return 1;

        case SDL_EVENT_MOUSE_MOTION:
            out_event->type = DONG_INPUT_EVENT_MOUSE_MOVE;
            out_event->x = sdl_event.motion.x;
            out_event->y = sdl_event.motion.y;
            out_event->dx = sdl_event.motion.xrel;
            out_event->dy = sdl_event.motion.yrel;
            return 1;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            out_event->type = DONG_INPUT_EVENT_MOUSE_BUTTON;
            out_event->x = sdl_event.button.x;
            out_event->y = sdl_event.button.y;
            out_event->a = sdl_event.button.button;  // button id
            out_event->b = (sdl_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;  // pressed
            return 1;

        case SDL_EVENT_MOUSE_WHEEL:
            out_event->type = DONG_INPUT_EVENT_MOUSE_WHEEL;
            out_event->x = sdl_event.wheel.mouse_x;
            out_event->y = sdl_event.wheel.mouse_y;
            out_event->dx = sdl_event.wheel.x;
            out_event->dy = sdl_event.wheel.y;
            return 1;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            out_event->type = DONG_INPUT_EVENT_KEY;
            out_event->a = sdl_event.key.key;        // key code
            out_event->b = sdl_event.key.scancode;   // scan code
            out_event->x = (sdl_event.type == SDL_EVENT_KEY_DOWN) ? 1.0f : 0.0f;  // pressed
            out_event->y = sdl_event.key.repeat ? 1.0f : 0.0f;  // repeat
            return 1;

        case SDL_EVENT_TEXT_INPUT:
            out_event->type = DONG_INPUT_EVENT_TEXT;
            out_event->text = sdl_event.text.text;
            return 1;

        default:
            return 0;
    }
}

// =============================================================================
// Renderer/GPU Callbacks
// =============================================================================

static dong_gpu_device_t* sdl_plugin_renderer_init(void* user, dong_window_t* window) {
    SDLPluginState* state = static_cast<SDLPluginState*>(user);
    SDL_Window* sdl_window = reinterpret_cast<SDL_Window*>(window);

    // Select shader format
#ifdef __APPLE__
    SDL_GPUShaderFormat format_flags = SDL_GPU_SHADERFORMAT_MSL;
#else
    SDL_GPUShaderFormat format_flags = dong_sdl_default_shader_formats();
#endif

    // Create GPU device
    SDL_GPUDevice* device = SDL_CreateGPUDevice(
        format_flags,
        state->config.gpu_debug_mode,
        nullptr);

    if (!device) {
        state->last_error = SDL_GetError();
        return nullptr;
    }

    // Claim window for GPU
    if (!SDL_ClaimWindowForGPUDevice(device, sdl_window)) {
        state->last_error = SDL_GetError();
        SDL_DestroyGPUDevice(device);
        return nullptr;
    }

    // Set swapchain parameters
    SDL_GPUPresentMode present_mode = state->config.gpu_prefer_mailbox
        ? SDL_GPU_PRESENTMODE_MAILBOX
        : SDL_GPU_PRESENTMODE_VSYNC;

    if (!SDL_SetGPUSwapchainParameters(
            device,
            sdl_window,
            SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            present_mode)) {
        // Fallback to VSYNC
        SDL_SetGPUSwapchainParameters(
            device,
            sdl_window,
            SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            SDL_GPU_PRESENTMODE_VSYNC);
    }

    state->gpu_device = device;

    // Create and register DongGPUDriver
    state->gpu_driver = dong_sdl_create_gpu_driver(device, sdl_window);
    if (state->gpu_driver) {
        DongPlatform* platform = dong_platform_get();
        dong_platform_set_gpu_driver(platform, state->gpu_driver);
    }

    // Return opaque handle
    return reinterpret_cast<dong_gpu_device_t*>(device);
}

static void sdl_plugin_renderer_shutdown(void* user, dong_gpu_device_t* device) {
    SDLPluginState* state = static_cast<SDLPluginState*>(user);
    SDL_GPUDevice* sdl_device = reinterpret_cast<SDL_GPUDevice*>(device);

    // Unregister and destroy GPUDriver
    if (state->gpu_driver) {
        DongPlatform* platform = dong_platform_get();
        dong_platform_set_gpu_driver(platform, nullptr);
        dong_sdl_destroy_gpu_driver(state->gpu_driver);
        state->gpu_driver = nullptr;
    }

    if (sdl_device) {
        SDL_DestroyGPUDevice(sdl_device);
    }

    if (state->gpu_device == sdl_device) {
        state->gpu_device = nullptr;
    }
}

static int sdl_plugin_renderer_begin_frame(void* user, dong_gpu_device_t* device) {
    (void)user;
    (void)device;
    // Frame begin is handled in execute()
    return 1;
}

static int sdl_plugin_renderer_submit(void* user, dong_gpu_device_t* device, const dong_renderer_cmd_stream_t* stream) {
    (void)user;
    SDL_GPUDevice* sdl_device = reinterpret_cast<SDL_GPUDevice*>(device);
    (void)sdl_device;
    (void)stream;

    // Note: In the current architecture, GPUCommandList is executed via
    // dong_gpu_execute() which uses the DongGPUDriver registered in Platform.
    // This callback is for future use when we migrate to a pure C plugin API.

    return 1;
}

static int sdl_plugin_renderer_end_frame(void* user, dong_gpu_device_t* device) {
    (void)user;
    SDL_GPUDevice* sdl_device = reinterpret_cast<SDL_GPUDevice*>(device);

    // Submit and present
    SDL_GPUCommandBuffer* cmd_buf = SDL_AcquireGPUCommandBuffer(sdl_device);
    if (!cmd_buf) {
        return 0;
    }

    SDL_SubmitGPUCommandBuffer(cmd_buf);
    return 1;
}

static void* sdl_plugin_get_native_window_handle(void* user, dong_window_t* window) {
    (void)user;
    return reinterpret_cast<void*>(window);  // SDL_Window pointer
}

static void* sdl_plugin_get_native_gpu_device(void* user, dong_gpu_device_t* device) {
    (void)user;
    return reinterpret_cast<void*>(device);  // SDL_GPUDevice pointer
}

// =============================================================================
// Video Callbacks (Stub - FFmpeg integration pending)
// =============================================================================

static dong_video_player_t* sdl_plugin_video_open(void* user, const char* url) {
    (void)user;
    (void)url;
    // FFmpeg integration pending
    return nullptr;
}

static void sdl_plugin_video_close(void* user, dong_video_player_t* player) {
    (void)user;
    (void)player;
}

static int sdl_plugin_video_get_metadata(void* user, dong_video_player_t* player, dong_video_metadata_t* out) {
    (void)user;
    (void)player;
    (void)out;
    return 0;
}

static int sdl_plugin_video_read_frame(void* user, dong_video_player_t* player, dong_video_frame_t* out_frame) {
    (void)user;
    (void)player;
    (void)out_frame;
    return 0;
}

static int sdl_plugin_video_seek(void* user, dong_video_player_t* player, double time_seconds) {
    (void)user;
    (void)player;
    (void)time_seconds;
    return 0;
}

// =============================================================================
// Plugin vtable (singleton)
// =============================================================================

static dong_plugin_vtable_t g_sdl_plugin_vtable = {
    .info = {
        .plugin_api_version = DONG_PLUGIN_API_VERSION,
        .capabilities =
            DONG_PLUGIN_CAP_LOG |
            DONG_PLUGIN_CAP_TIME |
            DONG_PLUGIN_CAP_WINDOW |
            DONG_PLUGIN_CAP_INPUT |
            DONG_PLUGIN_CAP_RENDERER
    },

    .log = sdl_plugin_log,
    .now_ns = sdl_plugin_now_ns,

    .window_create = sdl_plugin_window_create,
    .window_destroy = sdl_plugin_window_destroy,
    .window_get_size = sdl_plugin_window_get_size,

    .poll_event = sdl_plugin_poll_event,

    .renderer_init = sdl_plugin_renderer_init,
    .renderer_shutdown = sdl_plugin_renderer_shutdown,
    .renderer_begin_frame = sdl_plugin_renderer_begin_frame,
    .renderer_submit = sdl_plugin_renderer_submit,
    .renderer_end_frame = sdl_plugin_renderer_end_frame,

    .get_native_window_handle = sdl_plugin_get_native_window_handle,
    .get_native_gpu_device = sdl_plugin_get_native_gpu_device,

    .video_open = sdl_plugin_video_open,
    .video_close = sdl_plugin_video_close,
    .video_get_metadata = sdl_plugin_video_get_metadata,
    .video_read_frame = sdl_plugin_video_read_frame,
    .video_seek = sdl_plugin_video_seek,
};

// =============================================================================
// Public API Implementation
// =============================================================================

DONG_SDL_PLATFORM_API dong_plugin_vtable_t* dong_sdl_plugin_create(
    const dong_sdl_plugin_config_t* config) {

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return nullptr;
    }

    // Allocate state
    SDLPluginState* state = new (std::nothrow) SDLPluginState();
    if (!state) {
        SDL_Quit();
        return nullptr;
    }

    // Store config
    if (config) {
        state->config = *config;
    } else {
        state->config = dong_sdl_plugin_config_default();
    }

    state->initialized = 1;

    // Set user data pointer in vtable (we'll use a copy)
    dong_plugin_vtable_t* vtable = new (std::nothrow) dong_plugin_vtable_t(g_sdl_plugin_vtable);
    if (!vtable) {
        delete state;
        SDL_Quit();
        return nullptr;
    }

    // Store state pointer (we'll need to access it in callbacks)
    // Note: In a real implementation, we'd store this in a map or use a different approach
    // For now, we'll use a static pointer (not thread-safe, but sufficient for single instance)
    static SDLPluginState* s_current_state = nullptr;
    s_current_state = state;

    return vtable;
}

DONG_SDL_PLATFORM_API void dong_sdl_plugin_destroy(dong_plugin_vtable_t* plugin) {
    if (!plugin) return;

    // Cleanup state
    static SDLPluginState* s_current_state = nullptr;
    if (s_current_state) {
        // Cleanup GPU
        if (s_current_state->gpu_driver) {
            DongPlatform* platform = dong_platform_get();
            dong_platform_set_gpu_driver(platform, nullptr);
            dong_sdl_destroy_gpu_driver(s_current_state->gpu_driver);
        }

        if (s_current_state->gpu_device) {
            SDL_DestroyGPUDevice(s_current_state->gpu_device);
        }

        // Cleanup window
        if (s_current_state->window) {
            SDL_DestroyWindow(s_current_state->window);
        }

        delete s_current_state;
        s_current_state = nullptr;
    }

    delete plugin;
    SDL_Quit();
}

DONG_SDL_PLATFORM_API int dong_sdl_plugin_available(void) {
    return SDL_WasInit(SDL_INIT_VIDEO) != 0 || SDL_InitSubSystem(SDL_INIT_VIDEO) != 0;
}

DONG_SDL_PLATFORM_API const char* dong_sdl_plugin_get_error(void) {
    static SDLPluginState* s_current_state = nullptr;
    if (s_current_state && !s_current_state->last_error.empty()) {
        return s_current_state->last_error.c_str();
    }
    return SDL_GetError();
}

DONG_SDL_PLATFORM_API void* dong_sdl_plugin_get_window_handle(dong_window_t* window) {
    return reinterpret_cast<void*>(window);
}

DONG_SDL_PLATFORM_API void* dong_sdl_plugin_get_gpu_device_handle(dong_gpu_device_t* device) {
    return reinterpret_cast<void*>(device);
}
