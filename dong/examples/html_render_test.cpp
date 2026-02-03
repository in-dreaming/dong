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
#include <dong_platform.h>
#include <dong_gpu_driver.h>
#include "dong_sdl_platform.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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

static fs::path resolveExistingPath(const fs::path& html_file) {
    fs::path exe_dir;
    if (const char* base = SDL_GetBasePath()) {
        exe_dir = fs::path(base);
    }

    std::error_code ec;
    if (fs::exists(html_file, ec)) return html_file;

    if (!html_file.is_absolute()) {
        if (!exe_dir.empty()) {
            fs::path c1 = exe_dir / html_file;
            if (fs::exists(c1, ec)) return c1;

            fs::path c2 = exe_dir / "data" / html_file;
            if (fs::exists(c2, ec)) return c2;
        }

        fs::path c3 = fs::path("zig-out/bin") / html_file;
        if (fs::exists(c3, ec)) return c3;

        fs::path c4 = fs::path("zig-out/bin/data") / html_file;
        if (fs::exists(c4, ec)) return c4;
    }

    return html_file;
}

static SDL_GPUTexture* ensureOffscreenTexture(SDL_GPUDevice* device,
                                              SDL_GPUTexture* cached,
                                              uint32_t* cached_w,
                                              uint32_t* cached_h,
                                              uint32_t width,
                                              uint32_t height) {
    if (!device) return nullptr;
    if (cached && *cached_w == width && *cached_h == height) {
        return cached;
    }

    if (cached) {
        SDL_ReleaseGPUTexture(device, cached);
    }

    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;

    tex_info.width = width;
    tex_info.height = height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUTexture* created = SDL_CreateGPUTexture(device, &tex_info);
    if (!created) {
        SDL_Log("ERROR: Failed to create offscreen texture: %s", SDL_GetError());
        return nullptr;
    }

    *cached_w = width;
    *cached_h = height;
    return created;
}

static bool downloadTextureRGBA(SDL_GPUDevice* device, SDL_GPUTexture* texture,
                                uint32_t width, uint32_t height, uint8_t* out_pixels) {
    if (!device || !texture || !out_pixels) {
        return false;
    }

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = width * height * 4;

    SDL_GPUTransferBuffer* download_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
    if (!download_buffer) {
        SDL_Log("ERROR: Failed to create download buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        SDL_Log("ERROR: Failed to acquire command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
        return false;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTextureRegion src_region{};
    src_region.texture = texture;
    src_region.mip_level = 0;
    src_region.layer = 0;
    src_region.x = 0;
    src_region.y = 0;
    src_region.z = 0;
    src_region.w = width;
    src_region.h = height;
    src_region.d = 1;

    SDL_GPUTextureTransferInfo dst_transfer{};
    dst_transfer.transfer_buffer = download_buffer;
    dst_transfer.offset = 0;
    dst_transfer.pixels_per_row = 0;
    dst_transfer.rows_per_layer = 0;

    SDL_DownloadFromGPUTexture(copy_pass, &src_region, &dst_transfer);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
    if (fence) {
        SDL_GPUFence* fences[] = { fence };
        if (!SDL_WaitForGPUFences(device, true, fences, 1)) {
            SDL_Log("ERROR: SDL_WaitForGPUFences failed: %s", SDL_GetError());
            SDL_WaitForGPUIdle(device);
        }
        SDL_ReleaseGPUFence(device, fence);
    } else {
        SDL_Log("ERROR: SDL_SubmitGPUCommandBufferAndAcquireFence failed: %s", SDL_GetError());
        SDL_SubmitGPUCommandBuffer(cmd);
        SDL_WaitForGPUIdle(device);
    }

    void* mapped = SDL_MapGPUTransferBuffer(device, download_buffer, false);
    if (!mapped) {
        SDL_Log("ERROR: Failed to map download buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
        return false;
    }

    std::memcpy(out_pixels, mapped, width * height * 4);
    SDL_UnmapGPUTransferBuffer(device, download_buffer);
    SDL_ReleaseGPUTransferBuffer(device, download_buffer);
    return true;
}

static void injectSyntheticInputs(dong_engine_t* engine, uint32_t frame_index) {
    if (!engine) return;

    if (frame_index == 1) {
        if (const char* s = std::getenv("DONG_TEST_CLICK")) {
            int x = 0, y = 0, button = 1;
            int n = std::sscanf(s, "%d,%d,%d", &x, &y, &button);
            if (n >= 2) {
                dong_engine_send_mouse_move(engine, x, y);
                dong_engine_send_mouse_button(engine, button, 1);
                dong_engine_send_mouse_button(engine, button, 0);
            }
        }
    }

    if (frame_index > 0) {
        if (const char* s = std::getenv("DONG_TEST_WHEEL")) {
            int x = 0, y = 0;
            float dy = 0.0f;
            if (std::sscanf(s, "%d,%d,%f", &x, &y, &dy) == 3) {
                dong_engine_send_mouse_move(engine, x, y);
                dong_engine_send_mouse_wheel(engine, 0.0f, dy);
            }
        }
    }
}

static bool executeEngineFrame(dong_engine_t* engine, DongGPUDriver* driver,
                               bool do_update, const void** cached_cmd_list) {
    if (!engine || !driver) return false;

    if (do_update || !cached_cmd_list || !*cached_cmd_list) {
        if (dong_engine_tick(engine) != DONG_OK) {
            return false;
        }
        if (!do_update && cached_cmd_list) {
            *cached_cmd_list = dong_engine_get_command_list(engine);
        }
        return true;
    }

    return dong_gpu_execute(driver, *cached_cmd_list) != 0;
}

void printUsage(const char* prog) {
    SDL_Log("Usage:");
    SDL_Log("  %s <html_file> [output.bmp] [width] [height] [frames]", prog);
    SDL_Log("  %s <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update]", prog);

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
    SDL_Log("  --no-update          - Do NOT update scripts/layout between frames");

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
    SDL_Log("=== HTML Render Test (Engine) ===");

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string html_file = argv[1];

    std::string output_file = "zig-out/tmp/render_test.bmp";
    bool output_specified = false;
    uint32_t width = 800;
    uint32_t height = 600;
    uint32_t frames = 1;
    uint32_t frame_ms = 0;
    bool do_update = true;


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

    const fs::path html_path = resolveExistingPath(fs::path(html_file));
    html_file = html_path.string();

    fs::path exe_dir;
    if (const char* base = SDL_GetBasePath()) {
        exe_dir = fs::path(base);
    }
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


    std::string html_stem = fs::path(html_file).stem().string();
    fs::path first_output_path = getFrameOutputPath(output_file, html_stem, 0, frames);
    SDL_Log("[Output] BMP:  %s", first_output_path.string().c_str());
    if (frames > 1) {
        fs::path last_output_path = getFrameOutputPath(output_file, html_stem, frames - 1, frames);
        SDL_Log("[Output] ...  %s", last_output_path.string().c_str());
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("ERROR: SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("HTML Render Test", width, height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("ERROR: SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                                               false, nullptr);
    if (!device) {
        SDL_Log("ERROR: SDL_CreateGPUDevice failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("ERROR: SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!dong_sdl_platform_init(static_cast<void*>(device), static_cast<void*>(window))) {
        SDL_Log("ERROR: Failed to init SDL platform backend");
        SDL_ReleaseWindowFromGPUDevice(device, window);
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
    if (!driver) {
        SDL_Log("ERROR: Platform GPU driver is null");
        dong_sdl_platform_shutdown();
        return 1;
    }

    const std::string resource_root = fs::absolute(fs::path(html_file)).parent_path().string();
    dong_gpu_set_resource_root(driver, resource_root.c_str());

    dong_engine_desc_t desc{};
    desc.api_version = DONG_API_VERSION;
    desc.width = width;
    desc.height = height;

    dong_engine_t* engine = nullptr;
    if (dong_engine_create(&desc, &engine) != DONG_OK || !engine) {
        SDL_Log("ERROR: Failed to create dong engine");
        dong_sdl_platform_shutdown();
        return 1;
    }

    if (dong_engine_set_gpu(engine, static_cast<void*>(device), static_cast<void*>(window)) != DONG_OK) {

        SDL_Log("ERROR: Failed to set GPU device for engine");
        dong_engine_destroy(engine);
        dong_sdl_platform_shutdown();
        return 1;
    }

    if (dong_engine_load_html(engine, html_content.c_str()) != DONG_OK) {
        SDL_Log("ERROR: Failed to load HTML");
        dong_engine_destroy(engine);
        dong_sdl_platform_shutdown();
        return 1;
    }

    SDL_GPUTexture* offscreen_texture = nullptr;
    uint32_t offscreen_w = 0;
    uint32_t offscreen_h = 0;
    std::vector<uint8_t> pixels(width * height * 4);
    const void* cached_cmd_list = nullptr;

    for (uint32_t fi = 0; fi < frames; ++fi) {
        SDL_Log("[Render] Frame %u: do_update=%d", fi, do_update ? 1 : 0);


        injectSyntheticInputs(engine, fi);

        offscreen_texture = ensureOffscreenTexture(device, offscreen_texture, &offscreen_w, &offscreen_h, width, height);
        if (!offscreen_texture) {
            SDL_Log("ERROR: Failed to ensure offscreen texture");
            break;
        }

        if (!dong_gpu_begin_frame_offscreen(driver, offscreen_texture, width, height)) {
            SDL_Log("ERROR: begin_frame_offscreen failed");
            break;
        }

        if (!executeEngineFrame(engine, driver, do_update, &cached_cmd_list)) {
            SDL_Log("ERROR: Engine render failed");
            dong_gpu_end_frame_offscreen(driver);
            break;
        }

        dong_gpu_end_frame_offscreen(driver);

        if (!downloadTextureRGBA(device, offscreen_texture, width, height, pixels.data())) {
            SDL_Log("ERROR: Failed to read back pixels");
            break;
        }

        fs::path out_path = getFrameOutputPath(output_file, html_stem, fi, frames);
        ensureParentDir(out_path);
        if (!writeBMP(out_path.string().c_str(), width, height, pixels.data())) {
            SDL_Log("ERROR: Failed to save BMP (frame=%u): %s", fi, out_path.string().c_str());
            break;
        }

        if (frame_ms > 0 && fi + 1 < frames) {
            SDL_Delay(frame_ms);
        }
    }

    SDL_Log("[Save] Saved %u frame(s)", frames);



    if (offscreen_texture) {
        SDL_ReleaseGPUTexture(device, offscreen_texture);
    }

    dong_engine_destroy(engine);
    dong_sdl_platform_shutdown();

    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    fs::path out0 = getFrameOutputPath(output_file, html_stem, 0, frames);

    SDL_Log("[Done] Output: %s", out0.string().c_str());
    if (frames > 1) {
        fs::path outN = getFrameOutputPath(output_file, html_stem, frames - 1, frames);
        SDL_Log("[Done] Output: %s", outN.string().c_str());
    }
    return 0;
}
