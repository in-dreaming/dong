/**
 * HTML Render Test - 渲染 HTML 文件到贴图并保存为 BMP
 *
 * 用法:
 *   html_render_test <html_file> [output.bmp] [width] [height] [frames]
 *   html_render_test <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update]
 *
 * 说明:
 * - html_file 既支持绝对路径，也支持相对路径；相对路径会优先按“可执行文件目录”解析（便于 zig build 直接运行）。
 * - frames > 1 时会逐帧导出独立 BMP。
 */

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"
#include "core/profiler.h"

using dong::platform::SDL3Window;
namespace fs = std::filesystem;

// BMP 写入函数
bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        SDL_Log("ERROR: Cannot open file for writing: %s", filename);
        return false;
    }

    uint32_t row_size = ((width * 3 + 3) / 4) * 4;  // 4-byte aligned
    uint32_t pixel_data_size = row_size * height;
    uint32_t filesize = 54 + pixel_data_size;
    
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

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    // BMP 从底部开始存储
    std::vector<uint8_t> row(row_size, 0);
    for (int y = static_cast<int>(height) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t idx = (y * width + x) * 4;
            row[x * 3 + 0] = rgba_data[idx + 2];  // B
            row[x * 3 + 1] = rgba_data[idx + 1];  // G
            row[x * 3 + 2] = rgba_data[idx + 0];  // R
        }
        fwrite(row.data(), 1, row_size, f);
    }

    fclose(f);
    return true;
}

// 读取文件内容
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 获取可用的测试 HTML 文件列表
std::vector<std::string> listTestFiles(const std::string& dir) {
    std::vector<std::string> files;
    try {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() == ".html") {
                files.push_back(entry.path().filename().string());
            }
        }
    } catch (...) {
        // 目录不存在
    }
    std::sort(files.begin(), files.end());
    return files;
}

static uint32_t parseU32OrDefault(const char* s, uint32_t default_value) {
    if (!s || !*s) return default_value;
    char* end = nullptr;
    unsigned long v = std::strtoul(s, &end, 10);
    if (end == s) return default_value;
    if (v > 0xFFFFFFFFu) return default_value;
    return static_cast<uint32_t>(v);
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
        char last = output_file.back();
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

void printUsage(const char* prog) {
    SDL_Log("Usage:");
    SDL_Log("  %s <html_file> [output.bmp] [width] [height] [frames]", prog);
    SDL_Log("  %s <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update] [--profile <trace.json>]", prog);
    SDL_Log("");
    SDL_Log("Arguments:");
    SDL_Log("  html_file   - Path to HTML file (relative to exe or absolute)");
    SDL_Log("  output.bmp  - Output BMP file or output directory (default: zig-out/tmp/render_test.bmp)");
    SDL_Log("  width       - Render width (default: 800)");
    SDL_Log("  height      - Render height (default: 600)");
    SDL_Log("  frames      - Number of frames (default: 1)");
    SDL_Log("");
    SDL_Log("Options:");
    SDL_Log("  --frames N           - Render N frames (overrides positional frames)");
    SDL_Log("  --frame-ms MS        - Sleep MS milliseconds between frames (default: 0)");
    SDL_Log("  --no-update          - Do NOT call dong_view_update() between frames");
    SDL_Log("  --profile <file.json>- Dump profiler trace to file (Chrome Trace format)");
    SDL_Log("");
    SDL_Log("Output rule when frames > 1:");
    SDL_Log("  - If output ends with .bmp, will write: <stem>_f000.bmp, <stem>_f001.bmp, ...");
    SDL_Log("  - If output is a directory, will write: <html_stem>_f000.bmp, <html_stem>_f001.bmp, ...");
    SDL_Log("");
    SDL_Log("Available test files in data/tests/:");

    auto files = listTestFiles("data/tests");
    if (files.empty()) {
        files = listTestFiles("zig-out/bin/data/tests");
    }
    if (files.empty()) {
        if (const char* base = SDL_GetBasePath()) {
            fs::path exe_dir(base);
            files = listTestFiles((exe_dir / "data/tests").string());
        }
    }
    for (const auto& f : files) {
        SDL_Log("  - data/tests/%s", f.c_str());
    }
}

int main(int argc, char* argv[]) {
    SDL_Log("=== HTML Render Test ===");

    // 解析参数
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string html_file = argv[1];

    // Backward compatible positional args:
    //   html_render_test <html_file> [output.bmp] [width] [height] [frames]
    std::string output_file = "zig-out/tmp/render_test.bmp";
    bool output_specified = false;
    uint32_t width = 800;
    uint32_t height = 600;
    uint32_t frames = 1;
    uint32_t frame_ms = 0;
    bool do_update = true;
    std::string profile_output;  // profiler trace output file

    int positional_index = 0;
    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];

        if (a == "-h" || a == "--help") {
            printUsage(argv[0]);
            return 0;
        }

        if (a == "--no-update") {
            do_update = false;
            continue;
        }

        if (a == "--frames") {
            if (i + 1 >= argc) {
                SDL_Log("ERROR: --frames requires a value");
                return 1;
            }
            frames = parseU32OrDefault(argv[++i], frames);
            continue;
        }

        if (a == "--frame-ms") {
            if (i + 1 >= argc) {
                SDL_Log("ERROR: --frame-ms requires a value");
                return 1;
            }
            frame_ms = parseU32OrDefault(argv[++i], frame_ms);
            continue;
        }

        if (a == "--profile") {
            if (i + 1 >= argc) {
                SDL_Log("ERROR: --profile requires a file path");
                return 1;
            }
            profile_output = argv[++i];
            continue;
        }

        // positional
        switch (positional_index) {
            case 0:
                output_file = a;
                output_specified = true;
                break;
            case 1:
                width = parseU32OrDefault(argv[i], width);
                break;
            case 2:
                height = parseU32OrDefault(argv[i], height);
                break;
            case 3:
                frames = parseU32OrDefault(argv[i], frames);
                break;
            default:
                SDL_Log("ERROR: Too many positional arguments: %s", a.c_str());
                printUsage(argv[0]);
                return 1;
        }
        positional_index++;
    }

    if (width == 0 || height == 0) {
        SDL_Log("ERROR: Invalid dimensions");
        return 1;
    }
    if (frames == 0) {
        SDL_Log("ERROR: Invalid frames: 0");
        return 1;
    }

    // 读取 HTML 文件（兼容 zig build：工作目录通常是仓库根目录，而测试文件在 zig-out/bin/data 下）
    fs::path exe_dir;
    if (const char* base = SDL_GetBasePath()) {
        exe_dir = fs::path(base);
    }

    auto resolveExistingPath = [&](const fs::path& p) -> fs::path {
        std::error_code ec;
        if (fs::exists(p, ec)) return p;

        if (!p.is_absolute()) {
            if (!exe_dir.empty()) {
                // 1) <exe_dir>/<relative>
                fs::path c1 = exe_dir / p;
                if (fs::exists(c1, ec)) return c1;

                // 2) <exe_dir>/data/<relative>  (兼容传 tests/xxx.html 等情况)
                fs::path c2 = exe_dir / "data" / p;
                if (fs::exists(c2, ec)) return c2;
            }

            // 3) <repo_root>/zig-out/bin/<relative>
            fs::path c3 = fs::path("zig-out/bin") / p;
            if (fs::exists(c3, ec)) return c3;

            // 4) <repo_root>/zig-out/bin/data/<relative>
            fs::path c4 = fs::path("zig-out/bin/data") / p;
            if (fs::exists(c4, ec)) return c4;
        }

        return p;
    };

    const fs::path html_path = resolveExistingPath(fs::path(html_file));
    html_file = html_path.string();

    // 如果用户没显式指定 output，并且当前 exe 在 zig-out/bin 下：
    // 把默认输出落到 ../tmp，避免从 zig-out/bin 运行时出现 zig-out/bin/zig-out/tmp 的二次嵌套。
    if (!output_specified && output_file == "zig-out/tmp/render_test.bmp" && !exe_dir.empty()) {
        std::error_code ec;
        if (fs::exists(exe_dir / "data", ec)) {
            output_file = (exe_dir / ".." / "tmp" / "render_test.bmp").lexically_normal().string();
        }
    }

    std::string html_content = readFile(html_file);
    if (html_content.empty()) {
        SDL_Log("ERROR: Cannot read HTML file: %s", html_file.c_str());
        return 1;
    }

    SDL_Log("[Input]  HTML: %s (%zu bytes)", html_file.c_str(), html_content.size());
    SDL_Log("[Render] Size: %ux%u frames=%u frame_ms=%u update=%s",
            width, height, frames, frame_ms, do_update ? "true" : "false");
    if (!profile_output.empty()) {
        SDL_Log("[Profile] Output: %s", profile_output.c_str());
        dong_profiler_init();
    }

    std::string html_stem = fs::path(html_file).stem().string();
    fs::path first_output_path = getFrameOutputPath(output_file, html_stem, 0, frames);
    SDL_Log("[Output] BMP:  %s", first_output_path.string().c_str());
    if (frames > 1) {
        fs::path last_output_path = getFrameOutputPath(output_file, html_stem, frames - 1, frames);
        SDL_Log("[Output] ...  %s", last_output_path.string().c_str());
    }

    // 创建 SDL 窗口
    SDL3Window::CreateInfo ci{};
    ci.title = "HTML Render Test";
    ci.width = width;
    ci.height = height;
    ci.use_gpu = true;
    ci.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(ci)) {
        SDL_Log("ERROR: Failed to initialize SDL3Window: %s", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        SDL_Log("ERROR: GPU device is null");
        return 1;
    }

    // 创建 Dong 上下文
    SDL_Log("[Init] Creating dong context...");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        SDL_Log("ERROR: Failed to create dong context");
        return 1;
    }

    SDL_Log("[Init] Creating dong view...");
    dong_view_t* view = dong_view_create(ctx, width, height);
    if (!view) {
        SDL_Log("ERROR: Failed to create dong view");
        dong_destroy_context(ctx);
        return 1;
    }

    // 设置 GPU 渲染模式
    SDL_Log("[Init] Enabling GPU render mode...");
    dong_view_set_external_gpu_device(view,
                                      static_cast<void*>(device),
                                      static_cast<void*>(window.getHandle()));

    // 资源根目录：让相对路径（../images/bg.png）按 HTML 文件所在目录解析
    {
        std::string resource_root = fs::absolute(fs::path(html_file)).parent_path().string();
        SDL_Log("[Load] Resource root: %s", resource_root.c_str());
        dong_view_set_resource_root(view, resource_root.c_str());
    }

    // 加载 HTML
    SDL_Log("[Load] Loading HTML...");
    SDL_Log("[Debug] HTML content length: %zu", html_content.length());
    SDL_Log("[Debug] About to call dong_view_load_html...");
    dong_view_load_html(view, html_content.c_str());
    SDL_Log("[Load] HTML loaded successfully");


    SDL_Log("[Debug] About to create pixel buffer...");
    // 渲染到像素缓冲区（逐帧导出）
    std::vector<uint8_t> pixels(width * height * 4);
    SDL_Log("[Debug] Pixel buffer created, size=%zu", pixels.size());

    auto logPixelStats = [&](uint32_t frame_index) {
        int total = static_cast<int>(width * height);
        int black = 0, colored = 0, white = 0;
        for (int i = 0; i < total; ++i) {
            int idx = i * 4;
            int brightness = (pixels[idx] + pixels[idx + 1] + pixels[idx + 2]) / 3;
            if (brightness < 50) black++;
            else if (brightness > 200) white++;
            else colored++;
        }
        SDL_Log("[Stats] Frame %u: black=%d(%.1f%%) colored=%d(%.1f%%) white=%d(%.1f%%)",
                frame_index,
                black, 100.0f * black / total,
                colored, 100.0f * colored / total,
                white, 100.0f * white / total);
    };

    SDL_Log("[Render] Rendering offscreen...");
    for (uint32_t fi = 0; fi < frames; ++fi) {
        DONG_PROFILE_SCOPE_CAT("RenderFrame", "frame");
        SDL_Log("[Render] Frame %u: do_update=%d", fi, do_update ? 1 : 0);

        // Optional: inject a synthetic click for tab switching, etc.
        // Env format: DONG_TEST_CLICK="x,y" or "x,y,button" (SDL button code; left=1)
        // Note: We inject on frame 1 so the initial script execution can attach event listeners on frame 0.
        if (fi == 1) {
            if (const char* s = std::getenv("DONG_TEST_CLICK")) {
                int x = 0, y = 0, button = 1;
                int n = std::sscanf(s, "%d,%d,%d", &x, &y, &button);
                if (n >= 2) {
                    dong_view_send_mouse_move(view, x, y);
                    dong_view_send_mouse_down(view, button);
                    dong_view_send_mouse_up(view, button);
                }
            }
        }


        // Optional: inject a synthetic mouse wheel between frames for scroll testing.
        // Env format: DONG_TEST_WHEEL="x,y,dy" (dy uses dong convention: positive = scroll down)
        if (fi > 0) {
            if (const char* s = std::getenv("DONG_TEST_WHEEL")) {
                int x = 0, y = 0;
                float dy = 0.0f;
                if (std::sscanf(s, "%d,%d,%f", &x, &y, &dy) == 3) {
                    dong_view_send_mouse_move(view, x, y);
                    dong_view_send_mouse_wheel(view, 0.0f, dy);
                }
            }
        }

        if (do_update) {
            SDL_Log("[Render] Calling dong_view_update...");
            dong_view_update(view);
            SDL_Log("[Render] dong_view_update completed");
        }




        if (!dong_view_render_offscreen(view, static_cast<void*>(device), width, height, pixels.data())) {

            SDL_Log("ERROR: dong_view_render_offscreen failed (frame=%u)", fi);
            dong_view_destroy(view);
            dong_destroy_context(ctx);
            return 1;
        }

        fs::path out_path = getFrameOutputPath(output_file, html_stem, fi, frames);
        ensureParentDir(out_path);
        if (!writeBMP(out_path.string().c_str(), width, height, pixels.data())) {
            SDL_Log("ERROR: Failed to save BMP (frame=%u): %s", fi, out_path.string().c_str());
            dong_view_destroy(view);
            dong_destroy_context(ctx);
            return 1;
        }

        // 为了避免日志爆炸：只输出首帧/末帧统计
        if (fi == 0 || fi + 1 == frames) {
            logPixelStats(fi);
        }

        if (frame_ms > 0 && fi + 1 < frames) {
            SDL_Delay(frame_ms);
        }
    }

    SDL_Log("[Save] Saved %u frame(s)", frames);

    // Dump profiler trace if requested
    if (!profile_output.empty()) {
        ensureParentDir(fs::path(profile_output));
        if (dong_profiler_dump(profile_output.c_str()) == 0) {
            SDL_Log("[Profile] Trace saved to: %s", profile_output.c_str());
        } else {
            SDL_Log("[Profile] ERROR: Failed to save trace to: %s", profile_output.c_str());
        }
    }

    // 清理
    SDL_Log("[Cleanup] Shutting down...");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    fs::path out0 = getFrameOutputPath(output_file, html_stem, 0, frames);
    SDL_Log("[Done] Output: %s", out0.string().c_str());
    if (frames > 1) {
        fs::path outN = getFrameOutputPath(output_file, html_stem, frames - 1, frames);
        SDL_Log("[Done] Output: %s", outN.string().c_str());
    }
    return 0;
}
