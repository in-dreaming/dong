/**
 * HTML Render Test - 渲染 HTML 文件到贴图并保存为 BMP
 *
 * 用法:
 *   html_render_test <html_file> [output.bmp] [width] [height] [frames]
 *   html_render_test <html_file> [output.bmp] [width] [height] --frames N [--frame-ms MS] [--no-update]
 *
 * 两帧对比 + 横向拼接（contenteditable / execCommand 等）:
 *   html_render_test page.html out.bmp 800 600 2 ^
 *     --eval-after-frame0-file snippets/ce_bold_after_frame0.js --stitch-horizontal [--stitch-output merged.bmp]
 * - 第 1 帧：操作前；执行脚本后第 2 帧：操作后；仍写出 out_f000.bmp / out_f001.bmp。
 * - --stitch-horizontal 生成宽为 2*width 的拼接图：若输出参数为具体 *.bmp 文件路径，则写入该路径；否则写入 <stem>_stitched.bmp。
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
#include <dong_plugin_api.h>
#include "dong_sdl_platform.h"


#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace fs = std::filesystem;

// Optional: load SDL plugin for video support.
static const dong_plugin_vtable_t* try_load_plugin() {
    static const dong_plugin_vtable_t* s_plugin_vtable = NULL;
    static void* s_plugin_module = NULL;
    if (s_plugin_vtable) return s_plugin_vtable;
    if (s_plugin_module) return NULL;

    const char* filename =
#if defined(_WIN32)
        "dong_plugin_sdl.dll";
#elif defined(__APPLE__)
        "libdong_plugin_sdl.dylib";
#else
        "libdong_plugin_sdl.so";
#endif

    const char* base_path = SDL_GetBasePath();
    if (!base_path) return NULL;

    char path[1024];
    SDL_snprintf(path, sizeof(path), "%s%s", base_path, filename);

    s_plugin_module = SDL_LoadObject(path);
    if (!s_plugin_module) {
        SDL_Log("[html_render_test] plugin not found: %s", path);
        s_plugin_module = (void*)1;
        return NULL;
    }

    typedef const dong_plugin_vtable_t* (*get_api_fn)(void);
    get_api_fn fn = (get_api_fn)SDL_LoadFunction((SDL_SharedObject*)s_plugin_module, "dong_plugin_get_api");
    if (!fn) {
        SDL_Log("[html_render_test] plugin missing symbol: dong_plugin_get_api");
        SDL_UnloadObject((SDL_SharedObject*)s_plugin_module);
        s_plugin_module = (void*)1;
        return NULL;
    }

    s_plugin_vtable = fn();
    if (s_plugin_vtable) {
        SDL_Log("[html_render_test] plugin loaded: %s", path);
    }
    return s_plugin_vtable;
}

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

// 左右拼接两帧 RGBA（同宽高），输出宽 2*width。
static bool stitchHorizontalRGBA(uint32_t width, uint32_t height,
                                 const uint8_t* left, const uint8_t* right,
                                 std::vector<uint8_t>& out_rgba) {
    if (!left || !right || width == 0 || height == 0) {
        return false;
    }
    out_rgba.resize(static_cast<size_t>(width) * 2u * static_cast<size_t>(height) * 4u);
    for (uint32_t y = 0; y < height; ++y) {
        const size_t row = static_cast<size_t>(y) * static_cast<size_t>(width) * 4u;
        const size_t dst = static_cast<size_t>(y) * static_cast<size_t>(width) * 2u * 4u;
        std::memcpy(out_rgba.data() + dst, left + row, static_cast<size_t>(width) * 4u);
        std::memcpy(out_rgba.data() + dst + static_cast<size_t>(width) * 4u, right + row,
                    static_cast<size_t>(width) * 4u);
    }
    return true;
}

static fs::path defaultStitchedOutputPath(const std::string& output_file) {
    fs::path out(output_file);
    fs::path parent = out.parent_path();
    const std::string stem = out.stem().string();
    const std::string ext = out.has_extension() ? out.extension().string() : ".bmp";
    return parent / fs::path(stem + "_stitched" + ext);
}

// Multi-frame writes <stem>_f0.bmp etc.; user often passes the same path they will open.
// If they pass a concrete file (e.g. ce_pair.bmp), write the stitched before|after there
// (width*2 x height). Otherwise use <stem>_stitched.bmp (e.g. directory output).
static fs::path resolveStitchedBMPPath(const std::string& stitch_output_path,
                                       const std::string& output_file) {
    if (!stitch_output_path.empty()) {
        return fs::path(stitch_output_path);
    }
    fs::path of(output_file);
    std::error_code ec;
    if (of.has_extension() && !fs::is_directory(of, ec)) {
        return of;
    }
    return defaultStitchedOutputPath(output_file);
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

// Resolve eval script path: HTML 同目录、cwd、可执行目录/data。
static fs::path resolveScriptPath(const fs::path& html_path, const fs::path& script_ref) {
    std::error_code ec;
    if (script_ref.is_absolute() && fs::exists(script_ref, ec)) {
        return script_ref;
    }
    fs::path c1 = html_path.parent_path() / script_ref;
    if (fs::exists(c1, ec)) {
        return c1;
    }
    if (fs::exists(script_ref, ec)) {
        return fs::weakly_canonical(script_ref, ec);
    }
    if (const char* base = SDL_GetBasePath()) {
        fs::path exe(base);
        fs::path c2 = exe / script_ref;
        if (fs::exists(c2, ec)) {
            return c2;
        }
        fs::path c3 = exe / "data" / script_ref;
        if (fs::exists(c3, ec)) {
            return c3;
        }
    }
    return script_ref;
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

static bool envFlagEnabled(const char* name, bool default_value) {
    const char* v = std::getenv(name);
    if (!v || !v[0]) return default_value;
    return (v[0] != '0');
}

static bool downloadIsBGRA(SDL_GPUDevice* device) {
    (void)device;
    // Default to assuming RGBA for offscreen downloads. Some backends may need
    // manual override for diagnostic captures.
    if (envFlagEnabled("DONG_GPU_DOWNLOAD_BGRA", false)) {
        return true;
    }
    if (envFlagEnabled("DONG_GPU_DOWNLOAD_RGBA", false)) {
        return false;
    }
    return false;
}

static void logPixelRGBA(const char* label, const uint8_t* pixels, uint32_t width, uint32_t x, uint32_t y) {
    if (!label || !pixels || width == 0) return;
    const size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 4;
    SDL_Log("[download] %s (%u,%u) bytes=%u,%u,%u,%u", label, x, y,
            (unsigned)pixels[idx + 0], (unsigned)pixels[idx + 1], (unsigned)pixels[idx + 2], (unsigned)pixels[idx + 3]);
}

static void fixupDownloadedPixelsToRGBA(SDL_GPUDevice* device, uint32_t width, uint32_t height, uint8_t* pixels) {
    if (!device || !pixels) return;
    if (!downloadIsBGRA(device)) return;

    const bool debug = envFlagEnabled("DONG_GPU_DOWNLOAD_DEBUG", false);
    if (debug) {
        const char* driver = SDL_GetGPUDeviceDriver(device);
        SDL_Log("[download] driver=%s fixup=BGRA->RGBA", driver ? driver : "(null)");

        const uint32_t sx = (width > 401) ? 400 : (width ? (width - 1) : 0);
        const uint32_t sy = (height > 261) ? 260 : (height ? (height - 1) : 0);
        logPixelRGBA("before mid", pixels, width, width / 2, height / 2);
        logPixelRGBA("before sample", pixels, width, sx, sy);
    }

    const size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    for (size_t i = 0; i < pixel_count; ++i) {
        uint8_t* p = pixels + i * 4;
        std::swap(p[0], p[2]);
    }

    if (debug) {
        const uint32_t sx = (width > 401) ? 400 : (width ? (width - 1) : 0);
        const uint32_t sy = (height > 261) ? 260 : (height ? (height - 1) : 0);
        logPixelRGBA("after mid", pixels, width, width / 2, height / 2);
        logPixelRGBA("after sample", pixels, width, sx, sy);
    }
}

static uint32_t alignUpU32(uint32_t v, uint32_t align) {
    if (align == 0) return v;
    return (v + (align - 1)) & ~(align - 1);
}

static bool shouldPadDownloadRows256(SDL_GPUDevice* device) {
    if (envFlagEnabled("DONG_GPU_DOWNLOAD_PAD256", false)) {
        return true;
    }

    // D3D12 readbacks often require 256-byte row pitch alignment; SDL tries to
    // hide this via a deferred CPU copy, but relying on that timing is fragile.
    // We proactively pad rows on D3D12 to make the GPU copy direct and deterministic.
    const char* driver = device ? SDL_GetGPUDeviceDriver(device) : nullptr;
    if (!driver) return false;
    return std::strcmp(driver, "direct3d12") == 0;
}

struct DownloadLayout {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bytes_per_row = 0;
    uint32_t padded_bytes_per_row = 0;
    uint32_t padded_pixels_per_row = 0;
    uint32_t buffer_size = 0;
};

static DownloadLayout computeDownloadLayout(SDL_GPUDevice* device, uint32_t width, uint32_t height) {
    DownloadLayout layout;
    layout.width = width;
    layout.height = height;
    layout.bytes_per_row = width * 4;

    layout.padded_bytes_per_row = layout.bytes_per_row;
    if (shouldPadDownloadRows256(device)) {
        layout.padded_bytes_per_row = alignUpU32(layout.bytes_per_row, 256);
    }

    layout.padded_pixels_per_row = (layout.padded_bytes_per_row / 4);
    layout.buffer_size = layout.padded_bytes_per_row * height;
    return layout;
}

static void copyMappedDownloadToTightRGBA(const void* mapped, const DownloadLayout& layout, uint8_t* out_pixels) {
    if (!mapped || !out_pixels) return;

    const uint8_t* src = static_cast<const uint8_t*>(mapped);
    const size_t src_row = static_cast<size_t>(layout.padded_bytes_per_row);
    const size_t dst_row = static_cast<size_t>(layout.bytes_per_row);

    for (uint32_t y = 0; y < layout.height; ++y) {
        const uint8_t* src_row_ptr = src + static_cast<size_t>(y) * src_row;
        uint8_t* dst_row_ptr = out_pixels + static_cast<size_t>(y) * dst_row;
        std::memcpy(dst_row_ptr, src_row_ptr, dst_row);
    }
}

static bool downloadTextureRGBA(SDL_GPUDevice* device, SDL_GPUTexture* texture,
                                uint32_t width, uint32_t height, uint8_t* out_pixels) {
    if (!device || !texture || !out_pixels) {
        return false;
    }

    const DownloadLayout layout = computeDownloadLayout(device, width, height);

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = layout.buffer_size;

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
    dst_transfer.pixels_per_row = layout.padded_pixels_per_row;
    dst_transfer.rows_per_layer = height;

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

    copyMappedDownloadToTightRGBA(mapped, layout, out_pixels);
    SDL_UnmapGPUTransferBuffer(device, download_buffer);
    SDL_ReleaseGPUTransferBuffer(device, download_buffer);

    fixupDownloadedPixelsToRGBA(device, width, height, out_pixels);
    return true;
}


static uint32_t parseEnvU32(const char* name, uint32_t default_value) {
    if (const char* s = std::getenv(name)) {
        unsigned v = 0;
        if (std::sscanf(s, "%u", &v) == 1) {
            return static_cast<uint32_t>(v);
        }
    }
    return default_value;
}

static uint32_t getClickFrameIndex(uint32_t total_frames) {
    // If we only render one frame, inject at frame 0 so one-shot captures can
    // exercise click-driven UI (e.g. select dropdown open state).
    if (total_frames <= 1) return 0;
    return parseEnvU32("DONG_TEST_CLICK_FRAME", 1);
}

static void injectSyntheticInputs(dong_engine_t* engine, uint32_t frame_index, uint32_t total_frames) {
    if (!engine) return;

    const uint32_t click_frame = getClickFrameIndex(total_frames);
    if (frame_index == click_frame) {
        if (const char* seq = std::getenv("DONG_TEST_CLICKS")) {
            // Format: "x,y" or "x,y,button"; multiple clicks separated by ';'
            std::string s(seq);
            size_t start = 0;
            while (start < s.size()) {
                size_t end = s.find(';', start);
                if (end == std::string::npos) end = s.size();

                std::string token = s.substr(start, end - start);
                int x = 0, y = 0, button = 1;
                int n = std::sscanf(token.c_str(), "%d,%d,%d", &x, &y, &button);
                if (n >= 2) {
                    SDL_Log("[TestClick] Injecting click at (%d, %d) button=%d", x, y, button);
                    dong_engine_send_mouse_move(engine, x, y);
                    dong_engine_send_mouse_button(engine, button, 1);
                    dong_engine_send_mouse_button(engine, button, 0);
                }

                start = end + 1;
            }
        } else if (const char* s = std::getenv("DONG_TEST_CLICK")) {
            int x = 0, y = 0, button = 1;
            int n = std::sscanf(s, "%d,%d,%d", &x, &y, &button);
            if (n >= 2) {
                SDL_Log("[TestClick] Injecting click at (%d, %d) button=%d", x, y, button);
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
    SDL_Log("  --eval-after-frame0-file PATH - After saving frame 0, run JS from file (needs frames>=2)");
    SDL_Log("  --stitch-horizontal  - After 2 frames, write stitched BMP (2*width): see resolve rules below");
    SDL_Log("  --stitch-output PATH - Stitched BMP path (optional; overrides default target)");
    SDL_Log("  Stitched path: if output is a .bmp file (not dir), stitched -> that file; else -> <stem>_stitched.bmp");

    SDL_Log("");
    SDL_Log("Test input injection (env vars):");
    SDL_Log("  DONG_TEST_CLICK=x,y[,button]");
    SDL_Log("  DONG_TEST_CLICKS=x1,y1[,...];x2,y2[,...]");
    SDL_Log("  DONG_TEST_CLICK_FRAME=N (default: 1; forced to 0 when frames==1)");
    SDL_Log("  DONG_TEST_WHEEL=x,y,dy (wheel injects on frame_index > 0)");

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
    fprintf(stderr, "[DEBUG] html_render_test starting...\n");
    fflush(stderr);
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
    std::string eval_after_frame0_file;
    bool stitch_horizontal = false;
    std::string stitch_output_path;

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

        if (a == "--eval-after-frame0-file") {
            if (i + 1 >= argc) {
                SDL_Log("ERROR: --eval-after-frame0-file requires a path");
                return 1;
            }
            eval_after_frame0_file = argv[++i];
            continue;
        }

        if (a == "--stitch-horizontal") {
            stitch_horizontal = true;
            continue;
        }

        if (a == "--stitch-output") {
            if (i + 1 >= argc) {
                SDL_Log("ERROR: --stitch-output requires a path");
                return 1;
            }
            stitch_output_path = argv[++i];
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

    if (!eval_after_frame0_file.empty() && frames == 1) {
        frames = 2;
        SDL_Log("[Config] --eval-after-frame0-file: frames set to 2");
    }
    if (!eval_after_frame0_file.empty() && frames < 2) {
        SDL_Log("ERROR: --eval-after-frame0-file requires at least 2 frames");
        return 1;
    }
    if (stitch_horizontal && frames != 2) {
        SDL_Log("ERROR: --stitch-horizontal requires exactly 2 frames (got %u)", frames);
        return 1;
    }
    if (!eval_after_frame0_file.empty() && !do_update) {
        SDL_Log("ERROR: --eval-after-frame0-file is incompatible with --no-update");
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
    if (stitch_horizontal && frames == 2) {
        fs::path sp = resolveStitchedBMPPath(stitch_output_path, output_file);
        SDL_Log("[Output] Stitched before|after -> %s", sp.string().c_str());
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
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.plugin = try_load_plugin();
    desc.plugin_user = NULL;
    desc.width = width;
    desc.height = height;

    dong_engine_t* engine = nullptr;
    if (dong_engine_create(&desc, &engine) != DONG_OK || !engine) {
        SDL_Log("ERROR: Failed to create dong engine");
        dong_sdl_platform_shutdown();
        return 1;
    }

    if (!resource_root.empty()) {
        (void)dong_engine_set_resource_root(engine, resource_root.c_str());
    }

    if (dong_engine_set_gpu(engine, static_cast<void*>(device), static_cast<void*>(window)) != DONG_OK) {

        SDL_Log("ERROR: Failed to set GPU device for engine");
        dong_engine_destroy(engine);
        dong_sdl_platform_shutdown();
        return 1;
    }


    // Apply text renderer mode from environment variable
    if (const char* tr_env = std::getenv("DONG_TEXT_RENDERER")) {
        std::string tr_val(tr_env);
        while (!tr_val.empty() && (tr_val.back() == ' ' || tr_val.back() == '\t'))
            tr_val.pop_back();
        std::transform(tr_val.begin(), tr_val.end(), tr_val.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        dong_text_renderer_mode_t tr_mode = DONG_TEXT_RENDERER_AUTO;
        if (tr_val == "slug") {
            tr_mode = DONG_TEXT_RENDERER_SLUG;
        } else if (tr_val == "msdf") {
            tr_mode = DONG_TEXT_RENDERER_MSDF;
        }
        dong_engine_set_text_renderer_mode(engine, tr_mode);
        SDL_Log("[Config] Text renderer mode: %s (from DONG_TEXT_RENDERER='%s')",
                tr_mode == DONG_TEXT_RENDERER_SLUG ? "Slug" :
                tr_mode == DONG_TEXT_RENDERER_MSDF ? "MSDF" : "Auto", tr_val.c_str());
    }

    if (dong_engine_load_html(engine, html_content.c_str()) != DONG_OK) {
        SDL_Log("ERROR: Failed to load HTML");
        dong_engine_destroy(engine);
        dong_sdl_platform_shutdown();
        return 1;
    }

    std::string eval_after_frame0_script;
    if (!eval_after_frame0_file.empty()) {
        const fs::path sp = resolveScriptPath(html_path, fs::path(eval_after_frame0_file));
        std::error_code ec;
        if (!fs::exists(sp, ec)) {
            SDL_Log("ERROR: eval script not found: %s", sp.string().c_str());
            dong_engine_destroy(engine);
            dong_sdl_platform_shutdown();
            return 1;
        }
        eval_after_frame0_script = readFile(sp.string());
        if (eval_after_frame0_script.empty()) {
            SDL_Log("ERROR: eval script is empty: %s", sp.string().c_str());
            dong_engine_destroy(engine);
            dong_sdl_platform_shutdown();
            return 1;
        }
        SDL_Log("[Eval] after frame 0: %zu bytes from %s",
                eval_after_frame0_script.size(), sp.string().c_str());
    }

    SDL_GPUTexture* offscreen_texture = nullptr;
    uint32_t offscreen_w = 0;
    uint32_t offscreen_h = 0;
    std::vector<uint8_t> pixels(width * height * 4);
    std::vector<uint8_t> pixels_frame0;
    const void* cached_cmd_list = nullptr;
    const bool need_stitch = stitch_horizontal && frames == 2;
    uint32_t frames_saved = 0;
    std::string stitched_file_written;

    for (uint32_t fi = 0; fi < frames; ++fi) {
        SDL_Log("[Render] Frame %u: do_update=%d", fi, do_update ? 1 : 0);


        injectSyntheticInputs(engine, fi, frames);

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
        frames_saved = fi + 1;

        if (need_stitch && fi == 0) {
            pixels_frame0 = pixels;
        }

        if (fi == 0 && !eval_after_frame0_script.empty()) {
            cached_cmd_list = nullptr;
            if (dong_engine_eval_script(engine, eval_after_frame0_script.c_str()) != DONG_OK) {
                SDL_Log("WARN: dong_engine_eval_script after frame 0 failed");
            }
        }

        if (frame_ms > 0 && fi + 1 < frames) {
            SDL_Delay(frame_ms);
        }
    }

    if (need_stitch && frames_saved == 2 && pixels_frame0.size() == pixels.size()) {
        std::vector<uint8_t> stitched;
        if (!stitchHorizontalRGBA(width, height, pixels_frame0.data(), pixels.data(), stitched)) {
            SDL_Log("ERROR: stitchHorizontalRGBA failed");
        } else {
            const fs::path sp = resolveStitchedBMPPath(stitch_output_path, output_file);
            ensureParentDir(sp);
            if (!writeBMP(sp.string().c_str(), width * 2, height, stitched.data())) {
                SDL_Log("ERROR: Failed to write stitched BMP: %s", sp.string().c_str());
            } else {
                SDL_Log("[Stitch] %s (%ux%u) left=frame0 right=frame1", sp.string().c_str(),
                        width * 2, height);
                stitched_file_written = sp.string();
            }
        }
    }

    SDL_Log("[Save] Saved %u frame(s)", frames_saved);



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
        if (!stitched_file_written.empty()) {
            SDL_Log("[Done] Before|after (stitched, open this): %s", stitched_file_written.c_str());
        } else if (need_stitch) {
            SDL_Log("[Done] WARN: stitched BMP was not written (check errors above)");
        }
    }
    return 0;
}
