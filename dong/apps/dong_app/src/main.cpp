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
#include <vector>


#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
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
    std::fprintf(stderr,
                 "Usage: dong_app [--html <file.html>] [--width <w>] [--height <h>] "
                 "[--frames <n>] [--frame-ms <ms>] [--out <out.bmp|out_dir>] "
                 "[--click <x,y[,button]>] [--click-frame <n>] "
                 "[--trace-out <file.json>] [--profile <file.json>] "
                 "[--layer-cache <0|1>] [--debug-layer-cache <0|1>]\n");
    std::fprintf(stderr,
                 "\n"
                 "Frame dump rule when --frames N is set:\n"
                 "  - If --out ends with .bmp, will write: <stem>_f000.bmp, <stem>_f001.bmp, ...\n"
                 "  - If --out is a directory, will write: <html_stem>_f000.bmp, <html_stem>_f001.bmp, ...\n"
                 "\n"
                 "Default --out (when omitted and --frames>0): zig-out/tmp/<html_stem>.bmp (resolved from exe dir)\n");
}

namespace fs = std::filesystem;

static bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = std::fopen(filename, "wb");
    if (!f) {
        std::fprintf(stderr, "[dong_app] ERROR: Cannot open file for writing: %s\n", filename);
        return false;
    }

    const uint32_t row_size = ((width * 3 + 3) / 4) * 4; // 4-byte aligned
    const uint32_t pixel_data_size = row_size * height;
    const uint32_t filesize = 54 + pixel_data_size;

    uint8_t bmpfileheader[14] = {
        'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0
    };
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize >> 8);
    bmpfileheader[4] = (uint8_t)(filesize >> 16);
    bmpfileheader[5] = (uint8_t)(filesize >> 24);

    uint8_t bmpinfoheader[40] = {
        40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0
    };
    bmpinfoheader[4]  = (uint8_t)(width);
    bmpinfoheader[5]  = (uint8_t)(width >> 8);
    bmpinfoheader[6]  = (uint8_t)(width >> 16);
    bmpinfoheader[7]  = (uint8_t)(width >> 24);
    bmpinfoheader[8]  = (uint8_t)(height);
    bmpinfoheader[9]  = (uint8_t)(height >> 8);
    bmpinfoheader[10] = (uint8_t)(height >> 16);
    bmpinfoheader[11] = (uint8_t)(height >> 24);

    std::fwrite(bmpfileheader, 1, 14, f);
    std::fwrite(bmpinfoheader, 1, 40, f);

    // BMP stores rows bottom-up.
    std::vector<uint8_t> row(row_size, 0);
    for (int y = static_cast<int>(height) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint32_t idx = (static_cast<uint32_t>(y) * width + x) * 4;
            row[x * 3 + 0] = rgba_data[idx + 2]; // B
            row[x * 3 + 1] = rgba_data[idx + 1]; // G
            row[x * 3 + 2] = rgba_data[idx + 0]; // R
        }
        std::fwrite(row.data(), 1, row_size, f);
    }

    std::fclose(f);
    return true;
}

static int frameIndexWidth(uint32_t frames) {
    if (frames <= 1) return 1;
    return static_cast<int>(std::to_string(frames - 1).size());
}

static fs::path getFrameOutputPath(const std::string& output_file, const std::string& html_stem,
                                   uint32_t frame_index, uint32_t frames) {
    fs::path out_path(output_file);
    if (frames <= 1) return out_path;

    const int pad = frameIndexWidth(frames);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%0*u", pad, static_cast<unsigned>(frame_index));
    const std::string suffix = std::string("_f") + buf;

    std::error_code ec;
    bool treat_as_dir = false;
    if (!output_file.empty()) {
        const char last = output_file.back();
        if (last == '/' || last == '\\') {
            treat_as_dir = true;
        }
    }
    if (fs::exists(out_path, ec) && fs::is_directory(out_path, ec)) {
        treat_as_dir = true;
    }
    if (!out_path.has_extension()) {
        treat_as_dir = true;
    }

    const std::string ext = out_path.has_extension() ? out_path.extension().string() : ".bmp";

    if (treat_as_dir) {
        return out_path / fs::path(html_stem + suffix + ext);
    }

    fs::path parent = out_path.parent_path();
    const std::string base = out_path.stem().string();
    return parent / fs::path(base + suffix + ext);
}

static void ensureParentDir(const fs::path& p) {
    std::error_code ec;
    fs::path parent = p.parent_path();
    if (!parent.empty()) {
        fs::create_directories(parent, ec);
    }
}

#if defined(_WIN32)
static fs::path getExeDir() {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, (DWORD)sizeof(buf));
    if (n == 0 || n >= sizeof(buf)) {
        return fs::current_path();
    }
    std::error_code ec;
    fs::path p = fs::path(std::string(buf, buf + n)).parent_path();
    if (!p.empty()) return p;
    return fs::current_path(ec);
}
#else
static fs::path getExeDir() { return fs::current_path(); }
#endif


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

    // 默认开启隔离图层缓存：交互模式会每帧渲染 swapchain，
    // 如果 transform/isolation 每帧都离屏重栅格，会造成明显卡顿。
    bool layer_cache_enabled = true;
    bool layer_cache_flag_set = false;

    bool debug_layer_cache = false;
    bool debug_layer_cache_flag_set = false;

    int max_frames = 0;      // 0 => run until quit
    int frame_ms = 16;       // cap frame rate (interactive default)
    bool frame_ms_flag_set = false;

    // Enable profile/bench mode (for scripts/tools/auto_profile_loop.py)
    bool profile_enabled = false;


    bool inject_click = false;
    int click_x = 0;
    int click_y = 0;
    int click_button = 1;
    int click_frame = 1;

    std::string trace_out = "dong_trace.json";

    // If --frames is set, dong_app will dump per-frame BMPs using dong_view_render_offscreen.
    // This is used to validate low-frequency flicker / video playback deterministically.
    std::string frame_out;
    bool frame_out_flag_set = false;


    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--html") == 0 && i + 1 < argc) {
            html_file = argv[++i];
        } else if (std::strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            width = (uint32_t)std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            height = (uint32_t)std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--layer-cache") == 0 && i + 1 < argc) {
            layer_cache_enabled = (std::atoi(argv[++i]) != 0);
            layer_cache_flag_set = true;
        } else if (std::strcmp(argv[i], "--debug-layer-cache") == 0 && i + 1 < argc) {
            debug_layer_cache = (std::atoi(argv[++i]) != 0);
            debug_layer_cache_flag_set = true;
        } else if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            max_frames = std::atoi(argv[++i]);
            if (max_frames < 0) max_frames = 0;
        } else if (std::strcmp(argv[i], "--frame-ms") == 0 && i + 1 < argc) {
            frame_ms = std::atoi(argv[++i]);
            if (frame_ms < 0) frame_ms = 0;
            frame_ms_flag_set = true;

        } else if (std::strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            frame_out = argv[++i];
            frame_out_flag_set = true;

        } else if (std::strcmp(argv[i], "--click") == 0 && i + 1 < argc) {


            const char* s = argv[++i];
            int x = 0, y = 0, b = 1;
            int n = std::sscanf(s, "%d,%d,%d", &x, &y, &b);
            if (n >= 2) {
                inject_click = true;
                click_x = x;
                click_y = y;
                click_button = (n >= 3) ? b : 1;
            }
        } else if (std::strcmp(argv[i], "--click-frame") == 0 && i + 1 < argc) {
            click_frame = std::atoi(argv[++i]);
            if (click_frame < 0) click_frame = 0;

        } else if (std::strcmp(argv[i], "--trace-out") == 0 && i + 1 < argc) {
            trace_out = argv[++i];

        } else if (std::strcmp(argv[i], "--profile") == 0 && i + 1 < argc) {
            // For scripts/tools/auto_profile_loop.py
            trace_out = argv[++i];
            profile_enabled = true;

        } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }



    }

    // Profile mode: default to no manual sleep so swapchain/present mode controls pacing.
    if (profile_enabled && !frame_ms_flag_set) {
        frame_ms = 0;
    }

    const std::string html_stem = html_file ? fs::path(html_file).stem().string() : std::string("dong_app");

    // Default frame dump path (when omitted): put output under ../tmp next to zig-out/bin.
    // Avoid nested zig-out/bin/zig-out/tmp when launched from zig-out/bin.
    if (max_frames > 0 && !frame_out_flag_set) {
        const fs::path exe_dir = getExeDir();
        const fs::path default_out = (exe_dir / ".." / "tmp" / fs::path(html_stem + ".bmp")).lexically_normal();
        frame_out = default_out.string();
    }


#if defined(_WIN32)

    // GPUDriverFactory 通过环境变量读取开关（见 CreateGPUDriver）。
    // dong_app 作为交互 demo，默认打开 layer cache。
    _putenv_s("DONG_LAYER_CACHE", layer_cache_enabled ? "1" : "0");
    if (debug_layer_cache_flag_set) {
        _putenv_s("DONG_DEBUG_LAYER_CACHE", debug_layer_cache ? "1" : "0");
    }
#else
    (void)layer_cache_flag_set;
    (void)debug_layer_cache_flag_set;
#endif

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

    SDL_Window* sdl_win = reinterpret_cast<SDL_Window*>(native_window);

    // Create legacy View for rendering
    // IMPORTANT: use drawable(pixel) size so layout/render match swapchain on high-DPI displays.
    uint32_t view_w = width;
    uint32_t view_h = height;
    {
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

    // Base window title (plus FPS overlay updated in the main loop)
    std::string base_title = "dong_app";
    if (html_file) {
        try {
            namespace fs = std::filesystem;
            base_title += " - ";
            base_title += fs::path(html_file).filename().string();
        } catch (...) {
            // ignore
        }
    }
    if (sdl_win) {
        SDL_SetWindowTitle(sdl_win, base_title.c_str());
    }

    std::printf("[dong_app] View created, entering main loop...\n");

    dong_profiler_init();

    // FPS measurement
    const Uint64 perf_freq = SDL_GetPerformanceFrequency();
    Uint64 fps_last_counter = SDL_GetPerformanceCounter();
    int fps_frame_count = 0;
    constexpr double kTitleUpdateSec = 0.5;

    // 自动化性能采样：预热后清空 profiler，只采样固定时长后自动退出。
    // - 默认仅在传了 --profile 时启用（避免影响日常交互使用）
    // - 可用环境变量覆盖：
    //   - DONG_BENCH_AUTOSTOP=0  关闭自动退出
    //   - DONG_BENCH_WARMUP_MS=N 预热 N ms（默认 2000）
    //   - DONG_BENCH_RUN_MS=N    采样 N ms（默认 5000）
    const bool bench_has_profile = profile_enabled;
    const bool bench_autostop = bench_has_profile && ([]() {
        const char* v = std::getenv("DONG_BENCH_AUTOSTOP");
        return !(v && std::strcmp(v, "0") == 0);
    })();
    const uint64_t bench_warmup_ms = []() -> uint64_t {
        const char* v = std::getenv("DONG_BENCH_WARMUP_MS");
        if (!v || !*v) return 2000;
        const long long ms = std::atoll(v);
        return ms <= 0 ? 0ull : static_cast<uint64_t>(ms);
    }();
    const uint64_t bench_run_ms = []() -> uint64_t {
        const char* v = std::getenv("DONG_BENCH_RUN_MS");
        if (!v || !*v) return 5000;
        const long long ms = std::atoll(v);
        return ms <= 0 ? 0ull : static_cast<uint64_t>(ms);
    }();

    const Uint64 bench_t0_ms = SDL_GetTicks();
    bool bench_measuring = false;
    Uint64 bench_t_measure_ms = bench_t0_ms;

    bool running = true;
    int frame_index = 0;

    std::vector<uint8_t> capture_pixels;
    if (max_frames > 0 && !frame_out.empty()) {
        const fs::path first_out = getFrameOutputPath(frame_out, html_stem, 0u, static_cast<uint32_t>(max_frames));
        const fs::path last_out = getFrameOutputPath(frame_out, html_stem,
                                                     static_cast<uint32_t>(std::max(0, max_frames - 1)),
                                                     static_cast<uint32_t>(max_frames));
        std::printf("[dong_app] Frame dump: %s\n", first_out.string().c_str());
        if (max_frames > 1) {
            std::printf("[dong_app] Frame dump: ... %s\n", last_out.string().c_str());
        }
    }

    while (running) {
        const Uint64 frame_start_ms = SDL_GetTicks();


        // 先处理自动采样窗口（保证开始采样的那一帧也能被记录到）

        if (bench_autostop) {
            const Uint64 now_ms = SDL_GetTicks();
            const uint64_t elapsed_ms = (uint64_t)(now_ms - bench_t0_ms);

            if (!bench_measuring && elapsed_ms >= bench_warmup_ms) {
                bench_measuring = true;
                bench_t_measure_ms = now_ms;

                // 清空并重置时间戳：把启动/预热阶段的长耗时从 trace 里剔除。
                dong_profiler_init();
                SDL_Log("[Bench] measuring started (warmup_ms=%llu run_ms=%llu)",
                        (unsigned long long)bench_warmup_ms, (unsigned long long)bench_run_ms);
            }

            if (bench_measuring) {
                const uint64_t measure_ms = (uint64_t)(now_ms - bench_t_measure_ms);
                if (bench_run_ms > 0 && measure_ms >= bench_run_ms) {
                    SDL_Log("[Bench] measuring finished (%llu ms), exiting...", (unsigned long long)measure_ms);
                    running = false;
                }
            }
        }

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

        // Optional: inject a synthetic click (useful for deterministic profiling).
        if (inject_click && frame_index == click_frame) {
            dong_view_send_mouse_move(view, (int32_t)click_x, (int32_t)click_y);
            updateCursor(view, (int32_t)click_x, (int32_t)click_y);
            dong_view_send_mouse_down(view, (int32_t)click_button);
            dong_view_send_mouse_up(view, (int32_t)click_button);
        }

        // Update and render via View
        dong_view_update(view);

        // Dump a CPU screenshot for this frame (offscreen render).
        if (max_frames > 0 && !frame_out.empty()) {
            if (capture_pixels.empty()) {
                capture_pixels.resize(static_cast<size_t>(view_w) * static_cast<size_t>(view_h) * 4);
            }

            if (dong_view_render_offscreen(view, native_device, view_w, view_h, capture_pixels.data())) {
                const fs::path out_path = getFrameOutputPath(frame_out, html_stem,
                                                            static_cast<uint32_t>(frame_index),
                                                            static_cast<uint32_t>(max_frames));
                ensureParentDir(out_path);
                if (!writeBMP(out_path.string().c_str(), view_w, view_h, capture_pixels.data())) {
                    std::fprintf(stderr, "[dong_app] ERROR: Failed to save BMP: %s\n", out_path.string().c_str());
                }
            } else {
                std::fprintf(stderr, "[dong_app] ERROR: dong_view_render_offscreen failed (frame=%d)\n", frame_index);
            }
        }

        // FPS in window title (update at a low frequency to avoid overhead)

        fps_frame_count++;
        if (sdl_win && perf_freq > 0) {
            const Uint64 now = SDL_GetPerformanceCounter();
            const double elapsed_sec = (double)(now - fps_last_counter) / (double)perf_freq;
            if (elapsed_sec >= kTitleUpdateSec) {
                const double fps = (elapsed_sec > 1e-9) ? ((double)fps_frame_count / elapsed_sec) : 0.0;
                char buf[256];
                std::snprintf(buf, sizeof(buf), "%s | %.1f FPS", base_title.c_str(), fps);
                SDL_SetWindowTitle(sdl_win, buf);
                fps_last_counter = now;
                fps_frame_count = 0;
            }
        }

        if (frame_ms > 0) {
            const Uint64 frame_end_ms = SDL_GetTicks();
            const Uint64 work_ms = (frame_end_ms >= frame_start_ms) ? (frame_end_ms - frame_start_ms) : 0ull;
            const Uint64 target_ms = (Uint64)frame_ms;
            if (work_ms < target_ms) {
                SDL_Delay((Uint32)(target_ms - work_ms));
            }
        }


        frame_index++;
        if (max_frames > 0 && frame_index >= max_frames) {
            running = false;
        }

    }


    std::printf("[dong_app] Shutting down...\n");

    // Dump profiler trace
    dong_profiler_dump(trace_out.c_str());

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
