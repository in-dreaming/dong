#include "dong.h"
#include "dong_plugin_api.h"
#include "dong_legacy_view.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static void print_err(const char* msg) {
    std::fprintf(stderr, "[dong_app] %s\n", msg ? msg : "(null)");
}

static void print_usage() {
    std::fprintf(stderr, "Usage: dong_app [--html <file.html>] [--width <w>] [--height <h>]\n");
}

static std::string read_file_content(const char* path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return "";
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int main(int argc, char** argv) {
    const char* html_file = nullptr;
    uint32_t width = 960;
    uint32_t height = 540;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--html") == 0 && i + 1 < argc) {
            html_file = argv[++i];
        } else if (std::strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            width = (uint32_t)std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            height = (uint32_t)std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    std::string html_content;
    if (html_file) {
        html_content = read_file_content(html_file);
        if (html_content.empty()) {
            std::fprintf(stderr, "[dong_app] Failed to read HTML file: %s\n", html_file);
            return 2;
        }
    } else {
        // Default demo HTML
        html_content = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <style>
        body {
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            color: #eee;
            font-family: sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            text-align: center;
            padding: 40px;
            background: rgba(255,255,255,0.05);
            border-radius: 16px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
        }
        h1 {
            font-size: 48px;
            margin-bottom: 16px;
            color: #00d4ff;
        }
        p {
            font-size: 18px;
            opacity: 0.8;
        }
        button {
            margin-top: 24px;
            padding: 12px 32px;
            font-size: 16px;
            background: #7b2cbf;
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
        }
        button:hover {
            background: #9d4edd;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Dong UI Engine</h1>
        <p>A lightweight, embeddable HTML/CSS rendering engine</p>
        <button id="btn">Click Me</button>
    </div>
</body>
</html>
)HTML";
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::fprintf(stderr, "[dong_app] SDL_Init failed: %s\n", SDL_GetError());
        return 2;
    }

#if !defined(_WIN32)
    print_err("Only Windows dynamic loader is implemented for now.");
    SDL_Quit();
    return 2;
#else
    HMODULE mod = LoadLibraryA("dong_plugin_sdl.dll");
    if (!mod) {
        print_err("LoadLibraryA(dong_plugin_sdl.dll) failed. Make sure the DLL is next to dong_app.exe");
        SDL_Quit();
        return 2;
    }

    auto get_api = reinterpret_cast<dong_plugin_get_api_fn>(GetProcAddress(mod, "dong_plugin_get_api"));
    if (!get_api) {
        print_err("GetProcAddress(dong_plugin_get_api) failed");
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    const dong_plugin_vtable_t* plugin = get_api();
    if (!plugin) {
        print_err("dong_plugin_get_api returned null");
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    if (plugin->info.plugin_api_version != DONG_PLUGIN_API_VERSION) {
        std::fprintf(stderr, "[dong_app] Plugin API version mismatch: expected %u, got %u\n",
                     DONG_PLUGIN_API_VERSION, plugin->info.plugin_api_version);
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    // Create window via plugin
    dong_window_desc_t win_desc = {};
    win_desc.title = "Dong App";
    win_desc.width = width;
    win_desc.height = height;

    dong_window_t* window = nullptr;
    if (plugin->window_create) {
        window = plugin->window_create(nullptr, &win_desc);
    }
    if (!window) {
        print_err("Failed to create window via plugin");
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    // Initialize renderer via plugin
    dong_gpu_device_t* gpu_device = nullptr;
    if (plugin->renderer_init) {
        gpu_device = plugin->renderer_init(nullptr, window);
    }
    if (!gpu_device) {
        print_err("Failed to initialize renderer via plugin");
        plugin->window_destroy(nullptr, window);
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    // Get native handles
    void* native_device = plugin->get_native_gpu_device ? plugin->get_native_gpu_device(nullptr, gpu_device) : nullptr;
    void* native_window = plugin->get_native_window_handle ? plugin->get_native_window_handle(nullptr, window) : nullptr;

    if (!native_device || !native_window) {
        print_err("Failed to get native GPU device or window handle");
        plugin->renderer_shutdown(nullptr, gpu_device);
        plugin->window_destroy(nullptr, window);
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    // Create legacy View for rendering
    dong_view_t* view = dong_view_create(nullptr, width, height);
    if (!view) {
        print_err("Failed to create dong_view");
        plugin->renderer_shutdown(nullptr, gpu_device);
        plugin->window_destroy(nullptr, window);
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    // Set external GPU device for rendering
    dong_view_set_external_gpu_device(view, native_device, native_window);

    // Load HTML
    dong_view_load_html(view, html_content.c_str());

    std::printf("[dong_app] View created, entering main loop...\n");

    bool running = true;
    while (running) {
        dong_input_event_t ev = {};
        while (plugin->poll_event && plugin->poll_event(nullptr, &ev)) {
            switch (ev.type) {
            case DONG_INPUT_EVENT_QUIT:
                running = false;
                break;
            case DONG_INPUT_EVENT_MOUSE_MOVE:
                dong_view_send_mouse_move(view, (int32_t)ev.x, (int32_t)ev.y);
                break;
            case DONG_INPUT_EVENT_MOUSE_BUTTON:
                if (ev.b != 0) {
                    dong_view_send_mouse_down(view, (int32_t)ev.a);
                } else {
                    dong_view_send_mouse_up(view, (int32_t)ev.a);
                }
                break;
            case DONG_INPUT_EVENT_MOUSE_WHEEL:
                dong_view_send_mouse_wheel(view, ev.dx, ev.dy);
                break;
            case DONG_INPUT_EVENT_KEY:
                if (ev.b != 0) {
                    dong_view_send_key_down(view, ev.a);
                } else {
                    dong_view_send_key_up(view, ev.a);
                }
                break;
            case DONG_INPUT_EVENT_TEXT:
                if (ev.text) {
                    dong_view_send_text_input(view, ev.text);
                }
                break;
            case DONG_INPUT_EVENT_WINDOW_RESIZE:
                dong_view_resize(view, ev.a, ev.b);
                break;
            default:
                break;
            }
        }

        if (!running) break;

        // Update and render via View
        dong_view_update(view);

        SDL_Delay(16); // ~60 FPS cap
    }

    std::printf("[dong_app] Shutting down...\n");

    dong_view_destroy(view);
    plugin->renderer_shutdown(nullptr, gpu_device);
    plugin->window_destroy(nullptr, window);
    FreeLibrary(mod);
    SDL_Quit();
    return 0;
#endif
}
