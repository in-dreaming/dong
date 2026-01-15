#include "dong.h"
#include "dong_plugin_api.h"
#include "dong_legacy_view.h"
#include "../../../src/core/profiler.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <filesystem>


#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// Cursor management
static std::unordered_map<std::string, SDL_Cursor*> cursor_cache;
static std::string current_cursor_name;

static SDL_Cursor* getOrCreateCursor(const std::string& cursor_name) {
    auto it = cursor_cache.find(cursor_name);
    if (it != cursor_cache.end()) {
        return it->second;
    }
    
    SDL_SystemCursor sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;
    
    if (cursor_name == "pointer" || cursor_name == "hand") {
        sdl_cursor = SDL_SYSTEM_CURSOR_POINTER;
    } else if (cursor_name == "text" || cursor_name == "ibeam") {
        sdl_cursor = SDL_SYSTEM_CURSOR_TEXT;
    } else if (cursor_name == "move" || cursor_name == "all-scroll") {
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;
    } else if (cursor_name == "wait") {
        sdl_cursor = SDL_SYSTEM_CURSOR_WAIT;
    } else if (cursor_name == "progress") {
        sdl_cursor = SDL_SYSTEM_CURSOR_PROGRESS;
    } else if (cursor_name == "crosshair") {
        sdl_cursor = SDL_SYSTEM_CURSOR_CROSSHAIR;
    } else if (cursor_name == "not-allowed" || cursor_name == "no-drop") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    } else if (cursor_name == "n-resize" || cursor_name == "s-resize" || cursor_name == "ns-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NS_RESIZE;
    } else if (cursor_name == "e-resize" || cursor_name == "w-resize" || cursor_name == "ew-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_EW_RESIZE;
    } else if (cursor_name == "ne-resize" || cursor_name == "sw-resize" || cursor_name == "nesw-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NESW_RESIZE;
    } else if (cursor_name == "nw-resize" || cursor_name == "se-resize" || cursor_name == "nwse-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    } else if (cursor_name == "grab" || cursor_name == "grabbing") {
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;  // SDL3 doesn't have grab cursor
    }
    
    SDL_Cursor* cursor = SDL_CreateSystemCursor(sdl_cursor);
    if (cursor) {
        cursor_cache[cursor_name] = cursor;
    }
    return cursor;
}

static void updateCursor(dong_view_t* view, int32_t x, int32_t y) {
    const char* cursor_name = dong_view_get_cursor_at(view, x, y);
    if (!cursor_name) cursor_name = "auto";
    
    std::string name(cursor_name);
    if (name == current_cursor_name) {
        return;  // No change needed
    }
    
    current_cursor_name = name;
    
    if (name == "none") {
        SDL_HideCursor();
        return;
    }
    
    SDL_ShowCursor();
    SDL_Cursor* cursor = getOrCreateCursor(name);
    if (cursor) {
        SDL_SetCursor(cursor);
    }
}

static void cleanupCursors() {
    for (auto& pair : cursor_cache) {
        if (pair.second) {
            SDL_DestroyCursor(pair.second);
        }
    }
    cursor_cache.clear();
}

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
    // CMake+MinGW typically produces `libdong_plugin_sdl.dll`, while MSVC builds may produce
    // `dong_plugin_sdl.dll`. Try both so the app works across toolchains.
    HMODULE mod = nullptr;
    const char* plugin_names[] = {
        "libdong_plugin_sdl.dll",
        "dong_plugin_sdl.dll",
    };
    for (const char* name : plugin_names) {
        mod = LoadLibraryA(name);
        if (mod) break;
    }

    if (!mod) {
        print_err("LoadLibraryA failed. Make sure dong_plugin_sdl DLL is next to dong_app.exe (tried libdong_plugin_sdl.dll, dong_plugin_sdl.dll)");
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
    // IMPORTANT: use drawable(pixel) size so layout/render match swapchain on high-DPI displays.
    uint32_t view_w = width;
    uint32_t view_h = height;
    {
        SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(native_window);
        int pix_w = 0;
        int pix_h = 0;
        if (sdl_win && SDL_GetWindowSizeInPixels(sdl_win, &pix_w, &pix_h) && pix_w > 0 && pix_h > 0) {
            view_w = (uint32_t)pix_w;
            view_h = (uint32_t)pix_h;
        }
    }
    dong_view_t* view = dong_view_create(nullptr, view_w, view_h);

    if (!view) {
        print_err("Failed to create dong_view");
        plugin->renderer_shutdown(nullptr, gpu_device);
        plugin->window_destroy(nullptr, window);
        FreeLibrary(mod);
        SDL_Quit();
        return 2;
    }

    // Inject plugin API (enables optional subsystems like video)
    dong_view_set_plugin_api(view, plugin, nullptr);

    // Set external GPU device for rendering
    dong_view_set_external_gpu_device(view, native_device, native_window);

    // Resolve relative resource paths against the HTML file directory.
    // This makes url("../images/bg.png") stable regardless of current working directory.
    if (html_file) {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path abs_html = fs::absolute(fs::path(html_file), ec);
        if (!ec) {
            std::string resource_root = abs_html.parent_path().string();
            std::printf("[dong_app] Resource root: %s\n", resource_root.c_str());
            dong_view_set_resource_root(view, resource_root.c_str());
        }
    }

    // Load HTML
    dong_view_load_html(view, html_content.c_str());


    std::printf("[dong_app] View created, entering main loop...\n");

    dong_profiler_init();
    
    bool running = true;
    while (running) {
        DONG_PROFILE_FRAME();
        dong_input_event_t ev = {};
        while (plugin->poll_event && plugin->poll_event(nullptr, &ev)) {
            switch (ev.type) {
            case DONG_INPUT_EVENT_QUIT:
                running = false;
                break;
            case DONG_INPUT_EVENT_MOUSE_MOVE:
                dong_view_send_mouse_move(view, (int32_t)ev.x, (int32_t)ev.y);
                updateCursor(view, (int32_t)ev.x, (int32_t)ev.y);
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

    // Dump profiler trace
    dong_profiler_dump("dong_trace.json");
    dong_profiler_shutdown();
    
    cleanupCursors();
    dong_view_destroy(view);
    plugin->renderer_shutdown(nullptr, gpu_device);
    plugin->window_destroy(nullptr, window);
    FreeLibrary(mod);
    SDL_Quit();
    return 0;
#endif
}
