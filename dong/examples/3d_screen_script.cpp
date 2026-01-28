/**
 * Dong Engine 3D Multi-Screen Script Demo
 * 
 * 演示多个 HTML 屏幕的自动排列和交互
 * 支持 n 个 HTML 文件，用数组声明文件路径和窗口大小，自动排列屏幕
 * 
 * 控制说明：
 * - 右键按住 + 鼠标移动：控制视角
 * - WASD：前后左右移动
 * - Q/E 或 空格/Ctrl：上下移动
 * - Shift：加速移动
 * - 左键点击屏幕：与 HTML 交互
 * - ESC：退出
 */

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <cstring>
#include <string>
#include <csignal>
#include <limits>
#include <fstream>
#include <sstream>


#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_loadso.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "utils/math3d.hpp"
#include "utils/camera.hpp"
#include "utils/gpu_utils.hpp"
#include "utils/dong_utils.hpp"
#include "core/profiler.h"

// #region agent log debug-triangle shaders
// Minimal pipeline to prove "we can draw to swapchain at all" independent of:
// - vertex buffer content for 3D meshes
// - MVP matrices / clip-space conventions
// - depth/culling states
// If this doesn't show up in the screenshot, the issue is below our scene code.
static const char* kVertexShaderDebugTriangle = R"(
struct VSInput {
    float2 pos : TEXCOORD0;
};
struct VSOutput {
    float4 position : SV_Position;
};
VSOutput main(VSInput input) {
    VSOutput o;
    o.position = float4(input.pos.xy, 0.0, 1.0);
    return o;
}
)";

static const char* kFragmentShaderDebugTriangle = R"(
float4 main() : SV_Target0 {
    return float4(1.0, 0.0, 0.0, 1.0); // solid red
}
)";

// Debug mesh pipeline: use Vertex3D position directly (no uniforms).
// This helps distinguish "MVP/uniforms wrong" vs "VB upload/binding wrong".
static const char* kVertexShaderDebugMeshNoMVP = R"(
struct VSInput {
    float3 position : TEXCOORD0;
};
struct VSOutput {
    float4 position : SV_Position;
};
VSOutput main(VSInput input) {
    VSOutput o;
    // Put the shape in the bottom-right corner so it won't cover the scene.
    // (Also helps confirm VB upload/bind works without drowning out the real render.)
    float2 p = input.position.xy * 0.18 + float2(0.65, -0.65);
    o.position = float4(p, 0.5, 1.0);
    return o;
}
)";

static const char* kFragmentShaderDebugMeshNoMVP = R"(
float4 main() : SV_Target0 {
    return float4(0.0, 1.0, 0.0, 1.0); // solid green
}
)";
// #endregion

// Debug logging helper
static void debug_log(const char* location, const char* message, const char* hypothesis_id, const char* data_json = nullptr) {
    try {
        // Use raw string to avoid accidental escape sequences like \m, \a, etc.
        std::ofstream log_file(R"(d:\mix\agents\game\indr\dong\.cursor\debug.log)", std::ios::app);
        if (log_file.is_open()) {
            std::stringstream ss;
            ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            ss << ",\"location\":\"" << location << "\"";
            ss << ",\"message\":\"" << message << "\"";
            ss << ",\"hypothesisId\":\"" << hypothesis_id << "\"";
            ss << ",\"sessionId\":\"debug-session\"";
            ss << ",\"runId\":\"run1\"";
            if (data_json) {
                ss << ",\"data\":" << data_json;
            }
            ss << "}\n";
            log_file << ss.str();
            log_file.close();
        } else {
            // Fallback: also try to log to stderr
            fprintf(stderr, "[DEBUG] Failed to open log file: %s\n", location);
        }
    } catch (...) {
        // Ignore logging errors
    }
}

// #region agent log viewport/scissor compat
// Some environments in this repo end up loading an SDL3.dll that does not export
// SDL_SetGPUViewport / SDL_SetGPUScissor, causing STATUS_ENTRYPOINT_NOT_FOUND at launch
// if we link to them directly. However, when these are missing, SDL_GPU's default
// viewport/scissor can be unset on some backends and clip all draws (clear still works).
// We therefore resolve the symbols dynamically and call them when available.
typedef void (SDLCALL *PFN_SDL_SetGPUViewport)(SDL_GPURenderPass* pass, const SDL_GPUViewport* viewport);
typedef void (SDLCALL *PFN_SDL_SetGPUScissor)(SDL_GPURenderPass* pass, const SDL_Rect* rect);

static PFN_SDL_SetGPUViewport g_pfnSetGPUViewport = nullptr;
static PFN_SDL_SetGPUScissor  g_pfnSetGPUScissor  = nullptr;
static bool g_tried_load_viewport_scissor = false;
static bool g_logged_viewport_scissor_status = false;

static void ensureViewportScissorFnsLoaded() {
    if (g_tried_load_viewport_scissor) return;
    g_tried_load_viewport_scissor = true;

    const char* lib =
#if defined(_WIN32)
        "SDL3.dll";
#elif defined(__APPLE__)
        "libSDL3.dylib";
#else
        "libSDL3.so";
#endif

    SDL_SharedObject* sdlobj = SDL_LoadObject(lib);
    if (!sdlobj) {
        if (!g_logged_viewport_scissor_status) {
            std::stringstream ss;
            ss << "{\"lib\":\"" << lib << "\",\"loadObjectOk\":false,\"err\":\"" << SDL_GetError() << "\"}";
            debug_log("3d_screen_script.cpp:viewport", "Failed to load SDL3 shared object for viewport/scissor", "H13", ss.str().c_str());
            g_logged_viewport_scissor_status = true;
        }
        return;
    }

    g_pfnSetGPUViewport = (PFN_SDL_SetGPUViewport)SDL_LoadFunction(sdlobj, "SDL_SetGPUViewport");
    g_pfnSetGPUScissor  = (PFN_SDL_SetGPUScissor)SDL_LoadFunction(sdlobj, "SDL_SetGPUScissor");

    if (!g_logged_viewport_scissor_status) {
        std::stringstream ss;
        ss << "{\"lib\":\"" << lib << "\",\"loadObjectOk\":true";
        ss << ",\"hasSetGPUViewport\":" << (g_pfnSetGPUViewport ? 1 : 0);
        ss << ",\"hasSetGPUScissor\":" << (g_pfnSetGPUScissor ? 1 : 0);
        ss << "}";
        debug_log("3d_screen_script.cpp:viewport", "Viewport/scissor symbol probe", "H13", ss.str().c_str());
        g_logged_viewport_scissor_status = true;
    }
}

static void setFullscreenViewportScissor(SDL_GPURenderPass* pass, uint32_t sw, uint32_t sh) {
    ensureViewportScissorFnsLoaded();
    if (!pass) return;

    if (g_pfnSetGPUViewport) {
        SDL_GPUViewport vp{};
        vp.x = 0.0f;
        vp.y = 0.0f;
        vp.w = (float)sw;
        vp.h = (float)sh;
        vp.min_depth = 0.0f;
        vp.max_depth = 1.0f;
        g_pfnSetGPUViewport(pass, &vp);
    }

    if (g_pfnSetGPUScissor) {
        SDL_Rect sc{};
        sc.x = 0;
        sc.y = 0;
        sc.w = (int)sw;
        sc.h = (int)sh;
        g_pfnSetGPUScissor(pass, &sc);
    }

    static int logged_calls = 0;
    if (logged_calls < 3) {
        std::stringstream ss;
        ss << "{\"sw\":" << sw << ",\"sh\":" << sh;
        ss << ",\"calledViewport\":" << (g_pfnSetGPUViewport ? 1 : 0);
        ss << ",\"calledScissor\":" << (g_pfnSetGPUScissor ? 1 : 0);
        ss << ",\"pass\":" << (void*)pass << "}";
        debug_log("3d_screen_script.cpp:viewport", "Applied fullscreen viewport/scissor (if available)", "H13", ss.str().c_str());
        logged_calls++;
    }
}
// #endregion

// Simple BMP writer (kept for fallback)
static bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    SDL_Log("[writeBMP] Opening file: %s (w=%u h=%u)", filename, width, height);
    fflush(stdout);
    FILE* f = fopen(filename, "wb");
    if (!f) {
        SDL_Log("[writeBMP] ERROR: Failed to open file: %s (errno=%d)", filename, errno);
        fflush(stdout);
        return false;
    }
    SDL_Log("[writeBMP] File opened successfully");
    fflush(stdout);

    uint32_t filesize = 54 + width * height * 3;
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

    for (int y = static_cast<int>(height) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t idx = (y * width + x) * 4;
            uint8_t rgb[3] = {
                rgba_data[idx + 2],
                rgba_data[idx + 1],
                rgba_data[idx + 0],
            };
            fwrite(rgb, 3, 1, f);
        }
    }

    fclose(f);
    SDL_Log("[writeBMP] File written and closed successfully");
    fflush(stdout);
    return true;
}

#if defined(_WIN32)
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

static bool writePNG_WIC(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    SDL_Log("[writePNG] Using WIC: %s (w=%u h=%u)", filename, width, height);

    // Convert UTF-8 (or ANSI) path to wide; filename here is ASCII path in our demo.
    wchar_t wpath[MAX_PATH];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath, MAX_PATH);
    if (wlen <= 0) {
        // Fallback to ANSI
        wlen = MultiByteToWideChar(CP_ACP, 0, filename, -1, wpath, MAX_PATH);
        if (wlen <= 0) {
            SDL_Log("[writePNG] MultiByteToWideChar failed");
            return false;
        }
    }

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool did_init = SUCCEEDED(hr);
    if (hr == RPC_E_CHANGED_MODE) {
        // Someone initialized COM with different mode; continue.
        hr = S_OK;
    }
    if (FAILED(hr)) {
        SDL_Log("[writePNG] CoInitializeEx failed (0x%08X)", (unsigned)hr);
        return false;
    }

    IWICImagingFactory* factory = nullptr;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr) || !factory) {
        SDL_Log("[writePNG] CoCreateInstance(WICImagingFactory) failed (0x%08X)", (unsigned)hr);
        if (did_init) CoUninitialize();
        return false;
    }

    IWICStream* stream = nullptr;
    hr = factory->CreateStream(&stream);
    if (SUCCEEDED(hr)) {
        hr = stream->InitializeFromFilename(wpath, GENERIC_WRITE);
    }
    if (FAILED(hr) || !stream) {
        SDL_Log("[writePNG] Create/Init stream failed (0x%08X)", (unsigned)hr);
        if (stream) stream->Release();
        factory->Release();
        if (did_init) CoUninitialize();
        return false;
    }

    IWICBitmapEncoder* encoder = nullptr;
    hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
    if (SUCCEEDED(hr)) {
        hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
    }
    if (FAILED(hr) || !encoder) {
        SDL_Log("[writePNG] Create/Init encoder failed (0x%08X)", (unsigned)hr);
        if (encoder) encoder->Release();
        stream->Release();
        factory->Release();
        if (did_init) CoUninitialize();
        return false;
    }

    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* props = nullptr;
    hr = encoder->CreateNewFrame(&frame, &props);
    if (SUCCEEDED(hr)) {
        hr = frame->Initialize(props);
    }
    if (props) props->Release();
    if (FAILED(hr) || !frame) {
        SDL_Log("[writePNG] Create/Init frame failed (0x%08X)", (unsigned)hr);
        if (frame) frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        if (did_init) CoUninitialize();
        return false;
    }

    hr = frame->SetSize(width, height);
    if (FAILED(hr)) {
        SDL_Log("[writePNG] SetSize failed (0x%08X)", (unsigned)hr);
    }

    // Our pixels are RGBA8; write as 32bpp BGRA to WIC.
    // Convert line-by-line with a temp buffer to avoid large extra allocation.
    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    hr = frame->SetPixelFormat(&fmt);
    if (FAILED(hr)) {
        SDL_Log("[writePNG] SetPixelFormat failed (0x%08X)", (unsigned)hr);
    }

    if (FAILED(hr)) {
        frame->Release();
        encoder->Release();
        stream->Release();
        factory->Release();
        if (did_init) CoUninitialize();
        return false;
    }

    std::vector<uint8_t> bgra(width * 4);
    const UINT stride = (UINT)(width * 4);

    for (uint32_t y = 0; y < height; ++y) {
        const uint8_t* src = rgba_data + (size_t)y * (size_t)width * 4;
        uint8_t* dst = bgra.data();
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t r = src[x * 4 + 0];
            const uint8_t g = src[x * 4 + 1];
            const uint8_t b = src[x * 4 + 2];
            const uint8_t a = src[x * 4 + 3];
            dst[x * 4 + 0] = b;
            dst[x * 4 + 1] = g;
            dst[x * 4 + 2] = r;
            dst[x * 4 + 3] = a;
        }
        hr = frame->WritePixels(1, stride, stride, bgra.data());
        if (FAILED(hr)) {
            SDL_Log("[writePNG] WritePixels failed at y=%u (0x%08X)", y, (unsigned)hr);
            break;
        }
    }

    if (SUCCEEDED(hr)) {
        hr = frame->Commit();
    }
    if (SUCCEEDED(hr)) {
        hr = encoder->Commit();
    }

    frame->Release();
    encoder->Release();
    stream->Release();
    factory->Release();

    if (did_init) {
        CoUninitialize();
    }

    if (FAILED(hr)) {
        SDL_Log("[writePNG] Commit failed (0x%08X)", (unsigned)hr);
        return false;
    }

    SDL_Log("[writePNG] Wrote PNG successfully: %s", filename);
    return true;
}
#endif

static bool writeImageAuto(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    if (!filename) return false;
    const char* ext = std::strrchr(filename, '.');
    if (ext) {
        if (SDL_strcasecmp(ext, ".png") == 0) {
#if defined(_WIN32)
            return writePNG_WIC(filename, width, height, rgba_data);
#else
            SDL_Log("[writePNG] PNG requested but no writer available on this platform; falling back to BMP");
            return writeBMP(filename, width, height, rgba_data);
#endif
        }
    }
    return writeBMP(filename, width, height, rgba_data);
}

// 保存 swapchain 纹理到图片
static bool saveSwapchainToImage(SDL_GPUDevice* device, SDL_GPUTexture* swapchain, 
                                  uint32_t width, uint32_t height, const char* filename) {
    SDL_Log("[saveSwapchainToImage] Entry: device=%p swapchain=%p w=%u h=%u filename=%s", 
            (void*)device, (void*)swapchain, width, height, filename ? filename : "null");
    fflush(stdout);
    if (!device || !swapchain) {
        SDL_Log("[saveSwapchainToImage] ERROR: Invalid parameters");
        fflush(stdout);
        return false;
    }
    
    // 创建下载缓冲区
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = width * height * 4;
    
    SDL_GPUTransferBuffer* download_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
    if (!download_buffer) {
        SDL_Log("Failed to create download buffer: %s", SDL_GetError());
        return false;
    }
    
    // 获取命令缓冲区
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
        return false;
    }
    
    // 开始拷贝 pass
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    
    SDL_GPUTextureRegion src_region{};
    src_region.texture = swapchain;
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
    
    // Wait for completion.
    // NOTE: Avoid fence APIs for compatibility with older SDL3.dll builds.
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(device);
    
    // 映射并读取数据
    void* mapped = SDL_MapGPUTransferBuffer(device, download_buffer, false);
    if (!mapped) {
        SDL_Log("Failed to map download buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
        return false;
    }
    
    std::vector<uint8_t> pixels(width * height * 4);
    std::memcpy(pixels.data(), mapped, width * height * 4);

    // Diagnostic: Check first few pixels to verify readback works
    {
        SDL_Log("[Screenshot] First 4 pixels RGBA: [%u,%u,%u,%u] [%u,%u,%u,%u] [%u,%u,%u,%u] [%u,%u,%u,%u]",
                pixels[0], pixels[1], pixels[2], pixels[3],
                pixels[4], pixels[5], pixels[6], pixels[7],
                pixels[8], pixels[9], pixels[10], pixels[11],
                pixels[12], pixels[13], pixels[14], pixels[15]);
    }
    
    SDL_UnmapGPUTransferBuffer(device, download_buffer);
    SDL_ReleaseGPUTransferBuffer(device, download_buffer);
    
    // Save image (PNG preferred if filename ends with .png)
    SDL_Log("[saveSwapchainToImage] Writing image file: %s", filename);
    fflush(stdout);
    bool result = writeImageAuto(filename, width, height, pixels.data());
    SDL_Log("[saveSwapchainToImage] writeImageAuto returned: %s", result ? "true" : "false");
    fflush(stdout);
    
    if (result) {
        // 像素统计
        int total = static_cast<int>(width * height);
        int black = 0, gray = 0, white = 0, colored = 0;
        for (int i = 0; i < total; ++i) {
            int idx = i * 4;
            int r = pixels[idx];
            int g = pixels[idx + 1];
            int b = pixels[idx + 2];
            int brightness = (r + g + b) / 3;
            if (brightness < 50) black++;
            else if (brightness > 200) white++;
            else gray++;
            // 检查是否有非灰色像素（说明有内容）
            if (std::abs(r - g) > 10 || std::abs(g - b) > 10 || std::abs(r - b) > 10) {
                colored++;
            }
        }
        SDL_Log("[Screenshot] Saved to %s", filename);
        SDL_Log("[Screenshot] Pixels: black=%d(%.1f%%) gray=%d(%.1f%%) white=%d(%.1f%%) colored=%d(%.1f%%)",
                black, 100.0*black/total, gray, 100.0*gray/total, white, 100.0*white/total, 
                colored, 100.0*colored/total);
    }
    
    return result;
}

// Capture the current swapchain texture *within the same command buffer*.
// This avoids relying on swapchain texture validity after submit/present.
struct PendingCapture {
    SDL_GPUTransferBuffer* download_buffer = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    std::string filename;
};

// Enqueue a GPU texture -> transfer buffer download *into the current command buffer*.
// IMPORTANT: This must happen before the command buffer is submitted.
// We deliberately do NOT submit/wait/map here.
static bool enqueueSwapchainCapture(
    SDL_GPUDevice* device,
    SDL_GPUCommandBuffer* cmd,
    SDL_GPUTexture* swapchain,
    uint32_t width,
    uint32_t height,
    const char* filename,
    PendingCapture* out
) {
    if (!out) return false;

    // #region agent log
    {
        std::stringstream ss;
        ss << "{\"device\":" << (void*)device
           << ",\"cmd\":" << (void*)cmd
           << ",\"swapchain\":" << (void*)swapchain
           << ",\"w\":" << width
           << ",\"h\":" << height
           << ",\"filename\":\"" << (filename ? filename : "null") << "\"}";
        debug_log("3d_screen_script.cpp:enqueueSwapchainCapture", "Capture enqueue start", "H12", ss.str().c_str());
    }
    // #endregion

    if (!device || !cmd || !swapchain || width == 0 || height == 0 || !filename || !filename[0]) {
        debug_log("3d_screen_script.cpp:enqueueSwapchainCapture", "Capture invalid params", "H12", "{}");
        return false;
    }

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = width * height * 4;

    SDL_GPUTransferBuffer* download_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
    if (!download_buffer) {
        debug_log("3d_screen_script.cpp:enqueueSwapchainCapture", "Create download buffer failed", "H12", "{}");
        return false;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTextureRegion src_region{};
    src_region.texture = swapchain;
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

    out->download_buffer = download_buffer;
    out->width = width;
    out->height = height;
    out->filename = filename;

    debug_log("3d_screen_script.cpp:enqueueSwapchainCapture", "Capture enqueued", "H12", "{}");
    return true;
}

// After the command buffer is submitted and GPU work is complete, call this to write the image.
static bool finalizeSwapchainCapture(SDL_GPUDevice* device, PendingCapture* cap) {
    if (!cap || !cap->download_buffer || cap->width == 0 || cap->height == 0 || cap->filename.empty()) return false;

    void* mapped = SDL_MapGPUTransferBuffer(device, cap->download_buffer, false);
    if (!mapped) {
        SDL_ReleaseGPUTransferBuffer(device, cap->download_buffer);
        cap->download_buffer = nullptr;
        debug_log("3d_screen_script.cpp:finalizeSwapchainCapture", "Map download buffer failed", "H12", "{}");
        return false;
    }

    std::vector<uint8_t> pixels(cap->width * cap->height * 4);
    std::memcpy(pixels.data(), mapped, pixels.size());
    SDL_UnmapGPUTransferBuffer(device, cap->download_buffer);
    SDL_ReleaseGPUTransferBuffer(device, cap->download_buffer);
    cap->download_buffer = nullptr;

    // Compute diagnostics BEFORE we mutate pixels with the stamp.
    int total = (int)(cap->width * cap->height);
    int white = 0;
    int black = 0;
    for (int i = 0; i < total; ++i) {
        const int idx = i * 4;
        const int b = (pixels[idx + 0] + pixels[idx + 1] + pixels[idx + 2]) / 3;
        if (b > 240) white++;
        if (b < 15) black++;
    }

    const int first_r = (int)pixels[0];
    const int first_g = (int)pixels[1];
    const int first_b = (int)pixels[2];
    const int first_a = (int)pixels[3];

    const uint32_t cx = cap->width / 2;
    const uint32_t cy = cap->height / 2;
    const uint32_t cidx = (cy * cap->width + cx) * 4;
    const int center_r = (int)pixels[cidx + 0];
    const int center_g = (int)pixels[cidx + 1];
    const int center_b = (int)pixels[cidx + 2];
    const int center_a = (int)pixels[cidx + 3];

    const uint32_t sx = (cap->width > 80) ? 80 : (cap->width - 1);
    const uint32_t sy = (cap->height > 80) ? 80 : (cap->height - 1);
    const uint32_t sidx = (sy * cap->width + sx) * 4;
    const int sample_r = (int)pixels[sidx + 0];
    const int sample_g = (int)pixels[sidx + 1];
    const int sample_b = (int)pixels[sidx + 2];
    const int sample_a = (int)pixels[sidx + 3];

    // Deterministic stamp (top-left 32x32 magenta) to verify screenshot pipeline.
    {
        const uint32_t stamp_w = (cap->width < 32) ? cap->width : 32;
        const uint32_t stamp_h = (cap->height < 32) ? cap->height : 32;
        for (uint32_t y = 0; y < stamp_h; ++y) {
            for (uint32_t x = 0; x < stamp_w; ++x) {
                const uint32_t idx = (y * cap->width + x) * 4;
                pixels[idx + 0] = 255;
                pixels[idx + 1] = 0;
                pixels[idx + 2] = 255;
                pixels[idx + 3] = 255;
            }
        }
    }

    const bool ok = writeImageAuto(cap->filename.c_str(), cap->width, cap->height, pixels.data());

    // #region agent log
    {
        std::stringstream ss;
        ss << "{\"ok\":" << (ok ? 1 : 0)
           << ",\"whitePct\":" << (total ? (100.0 * (double)white / (double)total) : 0.0)
           << ",\"blackPct\":" << (total ? (100.0 * (double)black / (double)total) : 0.0)
           << ",\"firstPixel\":[" << first_r << "," << first_g << "," << first_b << "," << first_a << "]"
           << ",\"centerPixel\":[" << center_r << "," << center_g << "," << center_b << "," << center_a << "]"
           << ",\"samplePixel\":[" << sample_r << "," << sample_g << "," << sample_b << "," << sample_a << "]"
           << ",\"filename\":\"" << cap->filename << "\"}";
        debug_log("3d_screen_script.cpp:finalizeSwapchainCapture", "Capture done", "H12", ss.str().c_str());
    }
    // #endregion

    return ok;
}

using namespace dong::utils;

// Global for signal handler and atexit
static std::string g_profile_output;

// SDL_Log 默认写到 stderr；PowerShell 会把 native stderr 当成 error record。
// 这里把 SDL_Log 重定向到 stdout，避免运行 demo 时误报“异常”。
static void SDLCALL logToStdout(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    (void)userdata;
    (void)category;
    (void)priority;
    if (!message) return;
    std::fprintf(stdout, "%s\n", message);
    std::fflush(stdout);
}

static void dumpProfilerAtExit() {
    if (!g_profile_output.empty()) {
        printf("[Profile] atexit: dumping profiler trace...\n");
        fflush(stdout);
        int result = dong_profiler_dump(g_profile_output.c_str());
        printf("[Profile] atexit: dump result=%d, file=%s\n", result, g_profile_output.c_str());
        fflush(stdout);
    }
}

static void signalHandler(int signum) {
    printf("[Profile] Signal %d received\n", signum);
    fflush(stdout);
    dumpProfilerAtExit();
    std::_Exit(signum);  // Use _Exit to avoid calling atexit handlers twice
}

// ============================================================================
// HUD 系统
// ============================================================================

struct HUD {
    HtmlScreen3D html;
    SDL_GPUBuffer* quadVB = nullptr;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    
    bool showHelp = false;
    float fps = 0.0f;
    float fpsAccum = 0.0f;
    int frameCount = 0;
    float fpsUpdateTimer = 0.0f;
    
    bool init(dong_context_t* ctx, SDL_GPUDevice* device, SDL_Window* window,
              const char* htmlContent, uint32_t w, uint32_t h,
              SDL_GPUShader* vsHUD, SDL_GPUShader* fsHUD,
              SDL_GPUTextureFormat swapchainFormat,
              const char* resourceRoot = nullptr) {
        // 初始化 HTML 渲染
        if (!html.init(ctx, device, window, htmlContent, w, h, resourceRoot)) {
            return false;
        }

        
        // 创建 HUD 顶点缓冲区
        SDL_GPUBufferCreateInfo vbInfo{};
        vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        vbInfo.size = sizeof(VertexHUD) * 6;
        quadVB = SDL_CreateGPUBuffer(device, &vbInfo);
        if (!quadVB) return false;
        
        // 创建 HUD 管线
        SDL_GPUVertexBufferDescription vbDesc{};
        vbDesc.slot = 0;
        vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        vbDesc.pitch = sizeof(VertexHUD);
        
        SDL_GPUVertexAttribute attrs[2] = {};
        attrs[0].buffer_slot = 0;
        attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attrs[0].location = 0;
        attrs[0].offset = 0;
        attrs[1].buffer_slot = 0;
        attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attrs[1].location = 1;
        attrs[1].offset = sizeof(float) * 2;
        
        SDL_GPUColorTargetDescription colorDesc{};
        colorDesc.format = swapchainFormat;  // 使用实际 swapchain 格式
        // HUD blending: use standard straight-alpha blending.
        // Most paths in this repo assume straight alpha (see other examples and engine-side pipelines).
        // The HUD fragment shader already clamps RGB when alpha is near 0 to avoid "white RGB" washing the scene.
        colorDesc.blend_state.enable_blend = true;
        colorDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        colorDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        
        SDL_GPUGraphicsPipelineCreateInfo pipeInfo{};
        pipeInfo.vertex_shader = vsHUD;
        pipeInfo.fragment_shader = fsHUD;
        pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipeInfo.target_info.num_color_targets = 1;
        pipeInfo.target_info.color_target_descriptions = &colorDesc;
        pipeInfo.target_info.has_depth_stencil_target = false;
        pipeInfo.vertex_input_state.num_vertex_buffers = 1;
        pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDesc;
        pipeInfo.vertex_input_state.num_vertex_attributes = 2;
        pipeInfo.vertex_input_state.vertex_attributes = attrs;
        pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
        pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
        
        pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
        return pipeline != nullptr;
    }
    
    void update(SDL_GPUDevice* device, float dt) {
        // 更新 FPS
        frameCount++;
        fpsAccum += dt;
        fpsUpdateTimer += dt;
        
        // 每 1.0 秒更新一次 FPS 显示（降低频率，更容易观察）
        if (fpsUpdateTimer >= 1.0f) {
            fps = frameCount / fpsAccum;
            
            // 更新 HTML 中的 FPS 显示
            char js[128];
            int fps_int = (int)fps;
            snprintf(js, sizeof(js), 
                "var e=document.getElementById('fps-value');if(e){e.textContent='%d';}",
                fps_int);
            
            bool eval_result = html.eval(js);
            
            // Debug: 打印 FPS 更新
            SDL_Log("[HUD] FPS updated: %d (eval=%d, frameCount=%d, timer=%.2f)", 
                    fps_int, eval_result ? 1 : 0, frameCount, fpsUpdateTimer);
            
            fpsAccum = 0;
            frameCount = 0;
            fpsUpdateTimer = 0;
        }
        
        // 渲染 HTML 到纹理
        html.update(device);
    }
    
    void toggleHelp() {
        showHelp = !showHelp;
        html.eval("toggleHelp()");
    }
    
    bool eval(const char* js_code) {
        return html.eval(js_code);
    }
    
    void render(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* cmd,
                SDL_GPUSampler* sampler) {
        // #region agent log
        static int hud_render_count = 0;
        if (hud_render_count < 3) {
            std::ofstream log_file("d:\\mix\\agents\\game\\indr\\dong\\.cursor\\debug.log", std::ios::app);
            if (log_file.is_open()) {
                std::stringstream ss;
                ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                ss << ",\"location\":\"3d_screen_script.cpp:376\",\"message\":\"HUD render entry\",\"hypothesisId\":\"H8\",\"sessionId\":\"debug-session\",\"runId\":\"run1\"";
                ss << ",\"data\":{\"hasTexture\":" << (html.renderTexture != nullptr) << ",\"hasPipeline\":" << (pipeline != nullptr) << ",\"texture\":" << (void*)html.renderTexture << "}}\n";
                log_file << ss.str();
                log_file.close();
            }
            hud_render_count++;
        }
        // #endregion

        if (!html.renderTexture || !pipeline) return;

        SDL_BindGPUGraphicsPipeline(pass, pipeline);

        SDL_GPUBufferBinding binding{quadVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);

        SDL_GPUTextureSamplerBinding texBinding{html.renderTexture, sampler};
        SDL_BindGPUFragmentSamplers(pass, 0, &texBinding, 1);

        // Diagnostic: if HUD is overwriting the whole scene due to missing alpha/blend issues,
        // forcing a low alpha should make the 3D scene visible behind it.
        const bool force_hud_half_alpha = (std::getenv("DONG_DEBUG_HUD_HALF_ALPHA") != nullptr);

        UniformsHUD uniforms{};
        uniforms.color[0] = 1.0f;
        uniforms.color[1] = 1.0f;
        uniforms.color[2] = 1.0f;
        uniforms.color[3] = force_hud_half_alpha ? 0.25f : 1.0f;
        // Workaround switch: if HUD background becomes accidentally opaque-white and overwrites the whole scene,
        // enable keying via DONG_HUD_KEY_WHITE_BG=1. Keep it OFF by default to avoid hiding legitimate HUD content.
        const char* keyEnv = std::getenv("DONG_HUD_KEY_WHITE_BG");
        const bool keyWhite = (keyEnv && std::strcmp(keyEnv, "0") != 0);
        uniforms.keyWhiteBg = keyWhite ? 1.0f : 0.0f;

        SDL_PushGPUFragmentUniformData(cmd, 0, &uniforms, sizeof(uniforms));

        // Debug: log uniform data
        static int uniform_log_count = 0;
        if (uniform_log_count < 3) {
            SDL_Log("[HUD] Pushing fragment uniform buffer: color=(%.2f,%.2f,%.2f,%.2f) keyWhiteBg=%.2f",
                    uniforms.color[0], uniforms.color[1], uniforms.color[2], uniforms.color[3],
                    uniforms.keyWhiteBg);
            uniform_log_count++;
        }

        SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
    }
    
    void cleanup(SDL_GPUDevice* device) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
            pipeline = nullptr;
        }
        if (quadVB) {
            SDL_ReleaseGPUBuffer(device, quadVB);
            quadVB = nullptr;
        }
        html.cleanup(device);
    }
};

// HTML 屏幕配置
struct ScreenConfig {
    const char* htmlFile;
    uint32_t rtWidth;
    uint32_t rtHeight;
    float width;   // 3D 世界中的宽度
    float height;  // 3D 世界中的高度
};

// 屏幕信息（扩展 HtmlScreen3D）
struct Screen3D {
    HtmlScreen3D html;
    Vec3 position;
    float yaw = 0;
    float width = 3.0f;
    float height = 4.0f;
    uint32_t rtWidth = 800;
    uint32_t rtHeight = 1280;
    bool hovered = false;
    bool focused = false;
    bool isVideo = false;
};


// 获取屏幕的世界变换矩阵
Mat4 getScreenTransform(const Screen3D& screen) {
    return Mat4::translate(screen.position) * Mat4::rotateY(screen.yaw);
}

// 获取屏幕的 Quad3D（用于射线检测）
Quad3D getScreenQuad(const Screen3D& screen) {
    Vec3 right = Vec3{std::cos(screen.yaw), 0, -std::sin(screen.yaw)};
    Vec3 up = Vec3{0, 1, 0};
    return Quad3D(screen.position, right, up, screen.width, screen.height);
}

// 自动排列屏幕位置
// 从左下角往右上角分布，避免叠加
void arrangeScreens(Screen3D* screens, int numScreens, float spacingX = 4.0f, float spacingY = 2.8f) {
    if (numScreens <= 0) return;
    
    // 计算排列方式：优先横向排列，超过一定数量时分行
    int maxPerRow = 5;  // 每行最多5个屏幕
    int totalRows = (numScreens + maxPerRow - 1) / maxPerRow;
    
    // 基准 Y：让整个阵列垂直居中于相机高度附近
    // 相机初始高度 2.5f，让阵列中心大约在 2.5f
    float baseY = 2.5f + 3.0f - (totalRows - 1) * spacingY * 0.5f;
    
    for (int i = 0; i < numScreens; i++) {
        int row = i / maxPerRow;
        int col = i % maxPerRow;
        int screensInThisRow = (std::min)(maxPerRow, numScreens - row * maxPerRow);
        
        // 计算该行的起始X位置（居中对齐）
        float rowWidth = (screensInThisRow - 1) * spacingX;
        float startX = -rowWidth * 0.5f;
        
        // 设置位置：row 增加时 Y 增加（从下往上排）
        screens[i].position.x = startX + col * spacingX;
        screens[i].position.y = baseY + row * spacingY;
        screens[i].position.z = -3.0f;  // 稍微远一点，避免太近
        
        // 设置朝向（稍微向内倾斜）
        float centerX = 0.0f;
        float deltaX = screens[i].position.x - centerX;
        screens[i].yaw = -deltaX * 0.08f;  // 向中心倾斜
        
        // 限制倾斜角度
        screens[i].yaw = (std::max)(-0.4f, (std::min)(0.4f, screens[i].yaw));
    }
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    std::string profile_output;
    std::string present_mode_cli;      // mailbox|vsync|immediate
    int frames_in_flight_cli = 0;      // 1..3

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--profile" || arg == "--profiler") && i + 1 < argc) {
            profile_output = argv[++i];

        } else if (arg == "--present-mode" && i + 1 < argc) {
            present_mode_cli = argv[++i];
        } else if (arg == "--frames-in-flight" && i + 1 < argc) {
            frames_in_flight_cli = std::atoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            printf("Usage: %s [--profile <trace.json>] [--present-mode mailbox|vsync|immediate] [--frames-in-flight 1..3]\n", argv[0]);
            printf("Options:\n");
            printf("  --profile <file.json>           Dump profiler trace to file (Chrome Trace format)\n");
            printf("  --present-mode <mode>           Swapchain present mode (default: immediate)\n");
            printf("  --frames-in-flight <1..3>       Allowed frames in flight (default: 2)\n");
            printf("Env overrides (same as plugin):\n");
            printf("  DONG_GPU_PRESENT_MODE=mailbox|vsync|immediate\n");
            printf("  DONG_GPU_FRAMES_IN_FLIGHT=1|2|3\n");
            return 0;
        }
    }

    // Env -> defaults -> CLI override
    std::string present_mode_str = "immediate";
    if (const char* v = std::getenv("DONG_GPU_PRESENT_MODE")) {
        if (v && *v) {
            present_mode_str = v;
        }
    }
    if (!present_mode_cli.empty()) {
        present_mode_str = present_mode_cli;
    }

    int frames_in_flight = 2;
    if (const char* v = std::getenv("DONG_GPU_FRAMES_IN_FLIGHT")) {
        if (v && *v) {
            frames_in_flight = std::atoi(v);
        }
    }
    if (frames_in_flight_cli != 0) {
        frames_in_flight = frames_in_flight_cli;
    }
    if (frames_in_flight < 1) frames_in_flight = 1;
    if (frames_in_flight > 3) frames_in_flight = 3;

    auto parse_present_mode = [](const std::string& s, bool* out_valid) -> SDL_GPUPresentMode {
        if (out_valid) *out_valid = true;
        if (s == "mailbox") return SDL_GPU_PRESENTMODE_MAILBOX;
        if (s == "vsync") return SDL_GPU_PRESENTMODE_VSYNC;
        if (s == "immediate") return SDL_GPU_PRESENTMODE_IMMEDIATE;
        if (out_valid) *out_valid = false;
        return SDL_GPU_PRESENTMODE_IMMEDIATE;
    };

    bool present_mode_valid = true;
    const SDL_GPUPresentMode requested_present_mode = parse_present_mode(present_mode_str, &present_mode_valid);
    if (!present_mode_valid) {
        // 注意：这里发生在 SDL_Init 之前，用 printf。
        printf("[Swapchain] invalid present-mode '%s' (use mailbox|vsync|immediate), falling back to immediate\n", present_mode_str.c_str());
    }



    // 初始化 profiler (before SDL_Init, use printf)
    if (!profile_output.empty()) {
        printf("[Profile] Profiler enabled, output: %s\n", profile_output.c_str());
        fflush(stdout);
        g_profile_output = profile_output;  // Save for atexit/signal handler
        dong_profiler_init();
        
        // Register atexit handler - will be called on any normal exit
        std::atexit(dumpProfilerAtExit);
        
        // Install signal handlers for Ctrl+C etc
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
    }

    // 注意：SDL_Init 之前不要用 SDL_Log（默认会走 stderr）。
    printf("=== Dong 3D Multi-Screen Script Demo ===\n");
    printf("This demo supports multiple HTML screens with automatic arrangement\n");
    printf("Controls: RMB+Mouse=Look, WASD=Move, Q/E=Up/Down, Shift=Sprint, LMB=Interact, ESC=Exit\n");
    fflush(stdout);
    
    // #region agent log
    debug_log("3d_screen_script.cpp:558", "Program starting", "H11", "{}");
    debug_log("3d_screen_script.cpp:558", "Build marker", "H17", "{\"marker\":\"H17-20260127-2347\"}");
    // #endregion

    // 配置要显示的 HTML 屏幕
    ScreenConfig screenConfigs[] = {
        {"screen1_script.html", 800, 1280, 3.0f, 4.8f},
        {"screen2_script.html", 960, 640, 3.0f, 2.0f},
        {"feature_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/cursor_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/image_test.html", 960, 640, 3.0f, 2.0f},

        {"tests/background_attachment_fixed_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/background_clip_test.html",960, 640, 3.0f, 2.0f},
        {"tests/background_origin_test.html", 960, 640, 3.0f, 2.0f},
        {"tests/cursor_test.html",960, 640, 3.0f, 2.0f},
        {"tests/font_style_test.html",960, 640, 3.0f, 2.0f},
        {"tests/getcomputedstyle_smoke_test.html",960, 640, 3.0f, 2.0f},
        {"tests/image_test.html",960, 640, 3.0f, 2.0f},
        {"tests/outline_test.html",960, 640, 3.0f, 2.0f},
        {"tests/queryselector_complex_test.html",960, 640, 3.0f, 2.0f},
        {"tests/stylesheets_deleterule_test.html",960, 640, 3.0f, 2.0f},
        {"tests/stylesheets_insertrule_test.html",960, 640, 3.0f, 2.0f},
        {"tests/text_decoration_test.html",960, 640, 3.0f, 2.0f},
        {"tests/text_shadow_test.html",960, 640, 3.0f, 2.0f},
        {"tests/text_wrap_test.html",960, 640, 3.0f, 2.0f},
        {"tests/transform_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_play_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_events_test.html",960, 640, 3.0f, 2.0f},
        {"video/video_acceptance.html",960, 640, 3.0f, 2.0f},
        {"video/video_js_api_smoke_test.html",960, 640, 3.0f, 2.0f},
        // 可以继续添加更多屏幕...
    };
    
    const int numScreens = sizeof(screenConfigs) / sizeof(screenConfigs[0]);
    printf("Configured %d screens\n", numScreens);
    fflush(stdout);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // 把 SDL_Log 输出从 stderr 重定向到 stdout。
    SDL_SetLogOutputFunction(logToStdout, nullptr);
    
    if (!SDL_ShaderCross_Init()) {
        SDL_Log("SDL_ShaderCross_Init failed: %s", SDL_GetError());
        return 1;
    }

    //const int WIN_W = 2560, WIN_H = 1440;
    const int WIN_W = 2000, WIN_H = 1000;
    SDL_Window* window = SDL_CreateWindow("Dong 3D Multi-Screen Script Demo", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        false, nullptr);
    if (!device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        return 1;
    }

    // Swapchain tuning (demo side): present mode + frames-in-flight.
    // 目的：避免 profiler 被 VSYNC/acquire wait 掩盖真实热点。
    {
        SDL_SetGPUAllowedFramesInFlight(device, (Uint32)frames_in_flight);

        auto presentModeName = [](SDL_GPUPresentMode m) -> const char* {
            switch (m) {
                case SDL_GPU_PRESENTMODE_IMMEDIATE: return "immediate";
                case SDL_GPU_PRESENTMODE_MAILBOX:   return "mailbox";
                case SDL_GPU_PRESENTMODE_VSYNC:     return "vsync";
                default:                            return "unknown";
            }
        };

        auto trySetPresentMode = [&](SDL_GPUPresentMode mode) -> bool {
            return SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, mode);
        };

        SDL_GPUPresentMode final_mode = requested_present_mode;
        if (!trySetPresentMode(final_mode)) {
            SDL_Log("[Swapchain] request present-mode=%s failed (%s)", presentModeName(final_mode), SDL_GetError());

            // 优先回退 MAILBOX（通常比 VSYNC 更不容易被节流），最后回退 VSYNC。
            if (final_mode != SDL_GPU_PRESENTMODE_MAILBOX && trySetPresentMode(SDL_GPU_PRESENTMODE_MAILBOX)) {
                final_mode = SDL_GPU_PRESENTMODE_MAILBOX;
            } else {
                (void)trySetPresentMode(SDL_GPU_PRESENTMODE_VSYNC);
                final_mode = SDL_GPU_PRESENTMODE_VSYNC;
            }
        }

        SDL_Log("[Swapchain] present-mode=%s frames-in-flight=%d", presentModeName(final_mode), frames_in_flight);
    }

    // 创建 Dong 上下文

    dong_context_t* dongCtx = dong_create_context();
    if (!dongCtx) {
        SDL_Log("Failed to create dong context");
        return 1;
    }

    // 编译着色器
    SDL_GPUShader* vs3d = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShader3D, "main");
    SDL_GPUShader* fsCube = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShader3D, "main");
    SDL_GPUShader* fsGrid = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderGrid, "main");
    SDL_GPUShader* vsTextured = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShaderTextured, "main");
    SDL_GPUShader* fsTextured = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderTextured, "main", 1, 1);
    SDL_GPUShader* vsHUD = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShaderHUD, "main", 0, 0);
    SDL_GPUShader* fsHUD = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderHUD, "main", 1, 1);
    SDL_GPUShader* vsDbgTri = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShaderDebugTriangle, "main");
    SDL_GPUShader* fsDbgTri = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderDebugTriangle, "main");
    SDL_GPUShader* vsDbgMesh = compileShader(device, SDL_GPU_SHADERSTAGE_VERTEX, kVertexShaderDebugMeshNoMVP, "main");
    SDL_GPUShader* fsDbgMesh = compileShader(device, SDL_GPU_SHADERSTAGE_FRAGMENT, kFragmentShaderDebugMeshNoMVP, "main");
    
    if (!vs3d || !fsCube || !fsGrid || !vsTextured || !fsTextured || !vsHUD || !fsHUD || !vsDbgTri || !fsDbgTri || !vsDbgMesh || !fsDbgMesh) {
        SDL_Log("Failed to compile shaders");
        // #region agent log
        debug_log("3d_screen_script.cpp:710", "Shader compilation failed", "H11", "{}");
        // #endregion
        return 1;
    }
    
    // #region agent log
    debug_log("3d_screen_script.cpp:715", "Shaders compiled successfully", "H11", "{}");
    // #endregion

    // 3D 顶点布局
    SDL_GPUVertexBufferDescription vbDesc3D{};
    vbDesc3D.slot = 0;
    vbDesc3D.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDesc3D.pitch = sizeof(Vertex3D);

    SDL_GPUVertexAttribute attrs3D[3] = {};
    attrs3D[0].buffer_slot = 0; attrs3D[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs3D[0].location = 0; attrs3D[0].offset = 0;
    attrs3D[1].buffer_slot = 0; attrs3D[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs3D[1].location = 1; attrs3D[1].offset = sizeof(float) * 3;
    attrs3D[2].buffer_slot = 0; attrs3D[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrs3D[2].location = 2; attrs3D[2].offset = sizeof(float) * 6;

    // 纹理顶点布局
    SDL_GPUVertexBufferDescription vbDescUV{};
    vbDescUV.slot = 0;
    vbDescUV.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDescUV.pitch = sizeof(VertexUV);

    SDL_GPUVertexAttribute attrsUV[2] = {};
    attrsUV[0].buffer_slot = 0; attrsUV[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; attrsUV[0].location = 0; attrsUV[0].offset = 0;
    attrsUV[1].buffer_slot = 0; attrsUV[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; attrsUV[1].location = 1; attrsUV[1].offset = sizeof(float) * 3;

    // 获取 swapchain 实际格式（可能是 BGRA 或 RGBA，取决于平台/驱动）
    const SDL_GPUTextureFormat swapchainFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
    SDL_Log("[Pipeline] Swapchain format: %d", (int)swapchainFormat);

    // 立方体管线
    SDL_GPUColorTargetDescription colorDesc{};
    colorDesc.format = swapchainFormat;  // 使用 swapchain 实际格式，而非硬编码
    colorDesc.blend_state.enable_blend = false;

    SDL_GPUGraphicsPipelineCreateInfo pipeInfo{};
    pipeInfo.vertex_shader = vs3d;
    pipeInfo.fragment_shader = fsCube;
    pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeInfo.target_info.num_color_targets = 1;
    pipeInfo.target_info.color_target_descriptions = &colorDesc;
    pipeInfo.target_info.has_depth_stencil_target = true;
    pipeInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    pipeInfo.depth_stencil_state.enable_depth_test = true;
    pipeInfo.depth_stencil_state.enable_depth_write = true;
    pipeInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    pipeInfo.vertex_input_state.num_vertex_buffers = 1;
    pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDesc3D;
    pipeInfo.vertex_input_state.num_vertex_attributes = 3;
    pipeInfo.vertex_input_state.vertex_attributes = attrs3D;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    pipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipeInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    SDL_GPUGraphicsPipeline* cubePipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

    // 网格管线
    colorDesc.blend_state.enable_blend = true;
    colorDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    colorDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    colorDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    colorDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    
    pipeInfo.fragment_shader = fsGrid;
    pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    
    SDL_GPUGraphicsPipeline* gridPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

    // 屏幕纹理管线
    colorDesc.blend_state.enable_blend = true;
    pipeInfo.vertex_shader = vsTextured;
    pipeInfo.fragment_shader = fsTextured;
    pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDescUV;
    pipeInfo.vertex_input_state.num_vertex_attributes = 2;
    pipeInfo.vertex_input_state.vertex_attributes = attrsUV;
    pipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    // 屏幕需要深度测试（避免被其他物体遮挡），但不需要深度写入（避免遮挡后续物体）
    pipeInfo.depth_stencil_state.enable_depth_test = true;
    pipeInfo.depth_stencil_state.enable_depth_write = false;  // 不写入深度，避免遮挡后续渲染
    pipeInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    
    SDL_GPUGraphicsPipeline* screenPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);

    // Debug triangle pipeline (draws in NDC without uniforms)
    struct DebugTriVertex { float x, y; };
    SDL_GPUVertexBufferDescription vbDescDbg{};
    vbDescDbg.slot = 0;
    vbDescDbg.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDescDbg.pitch = sizeof(DebugTriVertex);
    SDL_GPUVertexAttribute attrsDbg[1] = {};
    attrsDbg[0].buffer_slot = 0;
    attrsDbg[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrsDbg[0].location = 0;
    attrsDbg[0].offset = 0;

    SDL_GPUGraphicsPipelineCreateInfo dbgPipeInfo{};
    dbgPipeInfo.vertex_shader = vsDbgTri;
    dbgPipeInfo.fragment_shader = fsDbgTri;
    dbgPipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    dbgPipeInfo.target_info.num_color_targets = 1;
    // Important: don't inherit blending from other pipelines.
    SDL_GPUColorTargetDescription dbgColorDesc = colorDesc;
    dbgColorDesc.blend_state.enable_blend = false;
    dbgPipeInfo.target_info.color_target_descriptions = &dbgColorDesc;
    // Important: match the render pass' depth target but DO NOT write depth,
    // otherwise this triangle can occlude the entire 3D scene.
    dbgPipeInfo.target_info.has_depth_stencil_target = true;
    dbgPipeInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    dbgPipeInfo.depth_stencil_state.enable_depth_test = false;
    dbgPipeInfo.depth_stencil_state.enable_depth_write = false;
    dbgPipeInfo.vertex_input_state.num_vertex_buffers = 1;
    dbgPipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDescDbg;
    dbgPipeInfo.vertex_input_state.num_vertex_attributes = 1;
    dbgPipeInfo.vertex_input_state.vertex_attributes = attrsDbg;
    dbgPipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    dbgPipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    dbgPipeInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    SDL_GPUGraphicsPipeline* dbgTriPipeline = SDL_CreateGPUGraphicsPipeline(device, &dbgPipeInfo);

    // Debug mesh pipeline: Vertex3D position -> clip (no MVP)
    SDL_GPUVertexBufferDescription vbDescDbgMesh{};
    vbDescDbgMesh.slot = 0;
    vbDescDbgMesh.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDescDbgMesh.pitch = sizeof(Vertex3D);
    SDL_GPUVertexAttribute attrsDbgMesh[1] = {};
    attrsDbgMesh[0].buffer_slot = 0;
    attrsDbgMesh[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attrsDbgMesh[0].location = 0;
    attrsDbgMesh[0].offset = 0;

    SDL_GPUGraphicsPipelineCreateInfo dbgMeshPipeInfo{};
    dbgMeshPipeInfo.vertex_shader = vsDbgMesh;
    dbgMeshPipeInfo.fragment_shader = fsDbgMesh;
    dbgMeshPipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    dbgMeshPipeInfo.target_info.num_color_targets = 1;
    dbgMeshPipeInfo.target_info.color_target_descriptions = &dbgColorDesc;
    dbgMeshPipeInfo.target_info.has_depth_stencil_target = true;
    dbgMeshPipeInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    dbgMeshPipeInfo.depth_stencil_state.enable_depth_test = false;
    dbgMeshPipeInfo.depth_stencil_state.enable_depth_write = false;
    dbgMeshPipeInfo.vertex_input_state.num_vertex_buffers = 1;
    dbgMeshPipeInfo.vertex_input_state.vertex_buffer_descriptions = &vbDescDbgMesh;
    dbgMeshPipeInfo.vertex_input_state.num_vertex_attributes = 1;
    dbgMeshPipeInfo.vertex_input_state.vertex_attributes = attrsDbgMesh;
    dbgMeshPipeInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    dbgMeshPipeInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    dbgMeshPipeInfo.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    SDL_GPUGraphicsPipeline* dbgMeshPipeline = SDL_CreateGPUGraphicsPipeline(device, &dbgMeshPipeInfo);

    SDL_GPUBufferCreateInfo dbgVBInfo{};
    dbgVBInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    dbgVBInfo.size = (Uint32)(3 * sizeof(DebugTriVertex));
    SDL_GPUBuffer* dbgTriVB = SDL_CreateGPUBuffer(device, &dbgVBInfo);

    if (!cubePipeline || !gridPipeline || !screenPipeline || !dbgTriPipeline || !dbgTriVB || !dbgMeshPipeline) {
        SDL_Log("Failed to create pipelines");
        return 1;
    }

    // 创建采样器
    SDL_GPUSamplerCreateInfo samplerInfo{};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &samplerInfo);

    // 生成几何体
    auto cubeVerts = generateCube();
    auto gridVerts = generateGrid();
    
    // 创建顶点缓冲区
    SDL_GPUBufferCreateInfo vbInfo{};
    vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    
    vbInfo.size = (Uint32)(cubeVerts.size() * sizeof(Vertex3D));
    SDL_GPUBuffer* cubeVB = SDL_CreateGPUBuffer(device, &vbInfo);
    
    vbInfo.size = (Uint32)(gridVerts.size() * sizeof(Vertex3D));
    SDL_GPUBuffer* gridVB = SDL_CreateGPUBuffer(device, &vbInfo);

    // 读取 HTML 文件并创建屏幕
    std::vector<std::string> htmlContents(numScreens);
    std::vector<std::string> htmlPaths(numScreens);
    std::vector<std::string> htmlRoots(numScreens);
    std::vector<Screen3D> screens(numScreens);

    for (int i = 0; i < numScreens; i++) {
        htmlPaths[i] = getDataPath(screenConfigs[i].htmlFile);
        htmlContents[i] = readFile(htmlPaths[i].c_str());
        if (htmlContents[i].empty()) {
            SDL_Log("Warning: Failed to load %s, using fallback HTML", screenConfigs[i].htmlFile);
            htmlContents[i] = "<html><body><h1>File not found: " + std::string(screenConfigs[i].htmlFile) + "</h1></body></html>";
            htmlRoots[i] = getDataPath("");
        } else {
            SDL_Log("Loaded %s (%zu bytes)", screenConfigs[i].htmlFile, htmlContents[i].size());
            try {
                htmlRoots[i] = std::filesystem::path(htmlPaths[i]).parent_path().string();
            } catch (...) {
                htmlRoots[i] = getDataPath("");
            }
        }

        // 设置屏幕属性
        screens[i].width = screenConfigs[i].width;
        screens[i].height = screenConfigs[i].height;
        screens[i].rtWidth = screenConfigs[i].rtWidth;
        screens[i].rtHeight = screenConfigs[i].rtHeight;

        // 粗分类：video/ 目录下认为是视频用例（用于更新调度）。
        screens[i].isVideo = (std::strncmp(screenConfigs[i].htmlFile, "video/", 6) == 0);
    }


    
    // 自动排列屏幕
    arrangeScreens(screens.data(), numScreens);
    
    SDL_Log("Screen arrangement:");
    for (int i = 0; i < numScreens; i++) {
        SDL_Log("  Screen %d: pos=(%.1f,%.1f,%.1f) yaw=%.2f size=%.1fx%.1f rt=%dx%d", 
                i, screens[i].position.x, screens[i].position.y, screens[i].position.z,
                screens[i].yaw, screens[i].width, screens[i].height,
                screens[i].rtWidth, screens[i].rtHeight);
    }

    // 初始化 HTML 屏幕
    // 注意：JS 代码现在通过 HTML 中的 <script> 标签自动执行，无需手动 eval
    for (int i = 0; i < numScreens; i++) {
        if (!screens[i].html.init(dongCtx, device, window, htmlContents[i].c_str(),
                                   screens[i].rtWidth, screens[i].rtHeight,
                                   htmlRoots[i].c_str())) {

            SDL_Log("Failed to init HTML screen %d (%s)", i, screenConfigs[i].htmlFile);
            return 1;
        }
        
        // 给每个 View 打上可读的 profile 名（只用于性能分析/日志，不影响功能）
        dong_view_set_debug_name(screens[i].html.view, screenConfigs[i].htmlFile);

        // 设置 3D 属性
        screens[i].html.width = screens[i].width;
        screens[i].html.height = screens[i].height;

        
        // 创建顶点缓冲区
        vbInfo.size = sizeof(VertexUV) * 6;
        screens[i].html.quadVB = SDL_CreateGPUBuffer(device, &vbInfo);
    }

    // 无需手动执行 JS - <script> 标签会在 load_html 时自动执行
    SDL_Log("HTML screens initialized - <script> tags executed automatically");

    // 初始化 HUD
    HUD hud;
    const std::string hudPath = getDataPath("hud.html");
    std::string hudHtml = readFile(hudPath.c_str());
    std::string hudRoot;
    try {
        hudRoot = std::filesystem::path(hudPath).parent_path().string();
    } catch (...) {
        hudRoot = getDataPath("");
    }

    if (hudHtml.empty()) {
        SDL_Log("Warning: Failed to load hud.html");
        hudHtml = "<html><body style='background:transparent'><div style='color:white;position:absolute;top:10px;left:10px'>FPS: --</div></body></html>";
    }
    if (!hud.init(dongCtx, device, window, hudHtml.c_str(), WIN_W, WIN_H, vsHUD, fsHUD, swapchainFormat, hudRoot.c_str())) {
        SDL_Log("Failed to init HUD");
        // #region agent log
        debug_log("3d_screen_script.cpp:910", "HUD init failed", "H11", "{}");
        // #endregion
        return 1;
    }
    dong_view_set_debug_name(hud.html.view, "hud");
    SDL_Log("HUD initialized");
    
    // 设置初始 FPS 值 - 使用直接的 JavaScript
    bool init_result = hud.eval("var e=document.getElementById('fps-value');if(e){e.textContent='INIT';e.style.color='#ffff00';e.style.background='rgba(0,0,0,0.7)';}");
    SDL_Log("HUD initial FPS set, result=%d", init_result ? 1 : 0);
    
    // 强制更新一次 HUD 纹理
    hud.update(device, 0.0f);
    SDL_Log("HUD initial update done");
    
    // #region agent log
    debug_log("3d_screen_script.cpp:916", "HUD initialized successfully", "H11", "{}");
    // #endregion


    // 相机
    FPSCamera camera;
    camera.position = Vec3{0.0f, 2.5f, 8.0f};
    camera.yaw = -3.14159f;
    camera.pitch = -0.1f;

    InputState input;
    
    SDL_GPUTexture* depthTexture = nullptr;
    uint32_t depthW = 0, depthH = 0;
    
    float mainCubeRotation = 0.0f;
    int focusedScreen = -1;  // 当前聚焦的屏幕 (-1 表示无)

    // Track press target so mouse-up goes to the same screen even if cursor moves away.
    int pressedScreen = -1;
    int32_t pressedX = 0;
    int32_t pressedY = 0;

    // If press+release happen in the same frame, delay mouse-up by 1 frame so :active is visible.
    bool pendingMouseUp = false;
    int pendingMouseUpScreen = -1;
    int32_t pendingMouseUpX = 0;
    int32_t pendingMouseUpY = 0;

    // Track last hovered screen so we can clear :hover when cursor leaves all screens.
    int lastHoveredScreen = -1;

    // 只有鼠标坐标真的变化时才把 move 转发给 HTML（否则会导致不必要的 repaint/重建）。
    int lastSentMouseScreen = -1;
    int32_t lastSentMouseX = (std::numeric_limits<int32_t>::min)();
    int32_t lastSentMouseY = (std::numeric_limits<int32_t>::min)();


    auto lastTime = std::chrono::high_resolution_clock::now();
    auto startTime = std::chrono::high_resolution_clock::now();
    bool screenshot_taken = false;

    bool running = true;
    int winW = WIN_W, winH = WIN_H;

    // 启用文本输入
    SDL_StartTextInput(window);
    
    // #region agent log
    {
        std::ofstream log_file("d:\\mix\\agents\\game\\indr\\dong\\.cursor\\debug.log", std::ios::app);
        if (log_file.is_open()) {
            std::stringstream ss;
            ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            ss << ",\"location\":\"3d_screen_script.cpp:950\",\"message\":\"Before main loop\",\"hypothesisId\":\"H11\",\"sessionId\":\"debug-session\",\"runId\":\"run1\"";
            ss << ",\"data\":{\"numScreens\":" << numScreens << ",\"running\":" << (running ? 1 : 0) << "}}\n";
            log_file << ss.str();
            log_file.close();
        }
    }
    // #endregion

    // 帧率/卡顿排查：不要再强制每帧 SDL_Delay(16) 双重限速。
    // - 默认不 sleep（让 swapchain/vsync 自己决定节奏）
    // - 如需手动限帧：设置环境变量 DONG_FRAME_SLEEP_MS=N
    const int frame_sleep_ms = []() -> int {
        const char* v = std::getenv("DONG_FRAME_SLEEP_MS");
        if (!v || !*v) return 0;
        const int ms = std::atoi(v);
        return ms < 0 ? 0 : ms;
    }();

    // swapchain acquire：默认阻塞（稳定、接近 vsync 行为）。
    // 如需减少 acquire 等待：DONG_GPU_SWAPCHAIN_NOWAIT=1（可能跳帧/撕裂）。
    const bool swapchain_nowait = (std::getenv("DONG_GPU_SWAPCHAIN_NOWAIT") != nullptr);


    // 自动化性能采样：预热后清空 profiler，只采样固定时长后自动退出。
    // - 默认仅在传了 --profile 时启用（避免影响日常交互使用）
    // - 可用环境变量覆盖：
    //   - DONG_BENCH_AUTOSTOP=0  关闭自动退出
    //   - DONG_BENCH_WARMUP_MS=N 预热 N ms（默认 2000）
    //   - DONG_BENCH_RUN_MS=N    采样 N ms（默认 5000）
    const bool bench_has_profile = !g_profile_output.empty();
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

    const auto bench_t0 = std::chrono::steady_clock::now();
    bool bench_measuring = false;
    auto bench_t_measure = bench_t0;

    // 多屏更新调度：把 20+ 屏的离屏更新摊到多帧里。
    // - hovered/focused：每帧更新（交互优先）
    // - video：按上限帧率更新（默认 30fps）
    // - background/static：每帧轮询少量（默认 2 个）
    const int bg_updates_per_frame = []() -> int {
        const char* v = std::getenv("DONG_BG_UPDATES_PER_FRAME");
        if (!v || !*v) return 2;
        const int n = std::atoi(v);
        return n < 0 ? 0 : n;
    }();
    const int video_max_fps = []() -> int {
        const char* v = std::getenv("DONG_VIDEO_MAX_FPS");
        if (!v || !*v) return 30;
        const int n = std::atoi(v);
        return n < 0 ? 0 : n;
    }();

    const auto scheduler_t0 = std::chrono::steady_clock::now();
    std::vector<double> next_video_update_sec(numScreens, 0.0);
    std::vector<uint64_t> updated_stamp(numScreens, 0);
    uint64_t stamp = 1;
    int bg_rr_cursor = 0;
    bool initial_full_update_done = false;

    // #region agent log
    {
        std::ofstream log_file("d:\\mix\\agents\\game\\indr\\dong\\.cursor\\debug.log", std::ios::app);
        if (log_file.is_open()) {
            std::stringstream ss;
            ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            ss << ",\"location\":\"3d_screen_script.cpp:1029\",\"message\":\"Entering main loop\",\"hypothesisId\":\"H11\",\"sessionId\":\"debug-session\",\"runId\":\"run1\"";
            ss << ",\"data\":{\"running\":" << (running ? 1 : 0) << "}}\n";
            log_file << ss.str();
            log_file.close();
        }
    }

    // Print SDL_GPU_LOADOP enum values (diagnostic for HUD pass LOAD semantics)
    // NOTE: We also log it during the first few frames inside the loop to ensure it lands in debug.log.
    {
        const int v_load = (int)SDL_GPU_LOADOP_LOAD;
        const int v_clear = (int)SDL_GPU_LOADOP_CLEAR;
        const int v_dontcare = (int)SDL_GPU_LOADOP_DONT_CARE;

        const int v_store = (int)SDL_GPU_STOREOP_STORE;
        const int v_store_dontcare = (int)SDL_GPU_STOREOP_DONT_CARE;

        SDL_Log("[Diag] SDL_GPU_LOADOP values: LOAD=%d CLEAR=%d DONT_CARE=%d", v_load, v_clear, v_dontcare);
        SDL_Log("[Diag] SDL_GPU_STOREOP values: STORE=%d DONT_CARE=%d", v_store, v_store_dontcare);

        {
            std::stringstream ss;
            ss << "{\"loadop_load\":" << v_load
               << ",\"loadop_clear\":" << v_clear
               << ",\"loadop_dontcare\":" << v_dontcare
               << "}";
            debug_log("3d_screen_script.cpp:loadop", "SDL_GPU_LOADOP enum values", "H16", ss.str().c_str());
        }

        {
            std::stringstream ss;
            ss << "{\"storeop_store\":" << v_store
               << ",\"storeop_dontcare\":" << v_store_dontcare
               << "}";
            debug_log("3d_screen_script.cpp:storeop", "SDL_GPU_STOREOP enum values", "H16", ss.str().c_str());
        }
    }
    // #endregion

    while (running) {

        // #region agent log
        static int loop_iteration = 0;
        if (loop_iteration < 5) {
            std::ofstream log_file("d:\\mix\\agents\\game\\indr\\dong\\.cursor\\debug.log", std::ios::app);
            if (log_file.is_open()) {
                {
                    std::stringstream ss;
                    ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    ss << ",\"location\":\"3d_screen_script.cpp:1032\",\"message\":\"Loop iteration\",\"hypothesisId\":\"H11\",\"sessionId\":\"debug-session\",\"runId\":\"run1\"";
                    ss << ",\"data\":{\"iteration\":" << loop_iteration << ",\"running\":" << (running ? 1 : 0) << "}}\n";
                    log_file << ss.str();
                }

                // Also print LOADOP enum values early to guarantee they appear in debug.log.
                if (loop_iteration < 3) {
                    const int v_load = (int)SDL_GPU_LOADOP_LOAD;
                    const int v_clear = (int)SDL_GPU_LOADOP_CLEAR;
                    const int v_dontcare = (int)SDL_GPU_LOADOP_DONT_CARE;

                    std::stringstream ss;
                    ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    ss << ",\"location\":\"3d_screen_script.cpp:1032\",\"message\":\"SDL_GPU_LOADOP enum values\",\"hypothesisId\":\"H16\",\"sessionId\":\"debug-session\",\"runId\":\"run1\"";
                    ss << ",\"data\":{\"loadop_load\":" << v_load << ",\"loadop_clear\":" << v_clear << ",\"loadop_dontcare\":" << v_dontcare << "}}\n";
                    log_file << ss.str();
                }

                log_file.close();
            }
        }
        loop_iteration++;
        // #endregion
        // 先处理自动采样窗口（保证开始采样的那一帧也能被记录到）
        if (bench_autostop) {
            const auto tnow = std::chrono::steady_clock::now();
            const uint64_t elapsed_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(tnow - bench_t0).count();

            if (!bench_measuring && elapsed_ms >= bench_warmup_ms) {
                bench_measuring = true;
                bench_t_measure = tnow;

                // 清空并重置时间戳：把启动/预热阶段的长耗时从 trace 里剔除。
                dong_profiler_init();
                SDL_Log("[Bench] measuring started (warmup_ms=%llu run_ms=%llu)",
                        (unsigned long long)bench_warmup_ms, (unsigned long long)bench_run_ms);
            }

            if (bench_measuring) {
                const uint64_t measure_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(tnow - bench_t_measure).count();
                if (bench_run_ms > 0 && measure_ms >= bench_run_ms) {
                    SDL_Log("[Bench] measuring finished (%llu ms), exiting...", (unsigned long long)measure_ms);
                    running = false;
                }
            }
        }

        DONG_PROFILE_SCOPE_CAT("Frame", "frame");

        auto now = std::chrono::high_resolution_clock::now();

        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        
        mainCubeRotation += dt * 0.5f;

        input.resetFrameState();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            input.handleEvent(event);
            
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                SDL_Log("=== Quit event received (type=%d) ===", event.type);
                // #region agent log
                debug_log("3d_screen_script.cpp:1071", "Quit event received", "H11", "{}");
                // #endregion
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                SDL_Log("=== ESC pressed, exiting ===");
                // #region agent log
                debug_log("3d_screen_script.cpp:1074", "ESC pressed", "H11", "{}");
                // #endregion
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && 
                       (event.key.scancode == SDL_SCANCODE_F1 || event.key.scancode == SDL_SCANCODE_H)) {
                // F1 或 H 键切换帮助面板
                hud.toggleHelp();
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &winW, &winH);
            } else if (event.type == SDL_EVENT_TEXT_INPUT) {
                // 转发文本输入到聚焦的屏幕
                if (focusedScreen >= 0) {
                    screens[focusedScreen].html.sendTextInput(event.text.text);
                }
            } else if (event.type == SDL_EVENT_KEY_DOWN && !input.right_mouse_down) {
                // 转发按键到聚焦的屏幕
                if (focusedScreen >= 0) {
                    screens[focusedScreen].html.sendKeyDown(event.key.scancode);
                }
            } else if (event.type == SDL_EVENT_KEY_UP && !input.right_mouse_down) {
                if (focusedScreen >= 0) {
                    screens[focusedScreen].html.sendKeyUp(event.key.scancode);
                }
            }
        }

        // 只有在右键按住且没有聚焦屏幕时才控制相机
        // 当屏幕聚焦时，键盘输入应该被屏幕劫持
        bool cameraControlEnabled = input.right_mouse_down || focusedScreen < 0;
        if (input.right_mouse_down) {
            camera.update(dt, input.keys, true, input.mouse_delta_x, input.mouse_delta_y);
        } else if (focusedScreen < 0) {
            // 只有没有聚焦屏幕时才允许 WASD 移动相机
            camera.update(dt, input.keys, false, 0, 0);
        }
        // 只有右键按住时才锁定鼠标
        SDL_SetWindowRelativeMouseMode(window, input.right_mouse_down);

        // 射线检测屏幕悬停和点击
        Ray hoverRay = camera.pixelToRay(input.mouse_x, input.mouse_y, winW, winH);
        
        int hoveredScreen = -1;
        Vec2 hoveredUV{-1, -1};
        
        int32_t hoveredScreenX = 0;
        int32_t hoveredScreenY = 0;

        for (int i = 0; i < numScreens; i++) {
            Quad3D quad = getScreenQuad(screens[i]);
            Vec2 uv = quad.intersect(hoverRay);
            screens[i].hovered = (uv.x >= 0);
            
            if (screens[i].hovered) {
                hoveredScreen = i;
                hoveredUV = uv;

                // 转换 UV 到屏幕像素坐标
                // 注意：UV 的 v 是从下到上（0=底部，1=顶部）
                // 但 HTML 坐标系 Y 是从上到下（0=顶部）
                // 所以需要翻转 Y 坐标
                hoveredScreenX = (int32_t)(uv.x * screens[i].rtWidth);
                hoveredScreenY = (int32_t)((1.0f - uv.y) * screens[i].rtHeight);
            }
        }

        // 只在坐标变化时才转发 move；否则会导致 HTML 侧每帧都置脏并触发昂贵的 rebuild。
        if (hoveredScreen >= 0) {
            if (hoveredScreen != lastSentMouseScreen || hoveredScreenX != lastSentMouseX || hoveredScreenY != lastSentMouseY) {
                screens[hoveredScreen].html.sendMouseMove(hoveredScreenX, hoveredScreenY);
                lastSentMouseScreen = hoveredScreen;
                lastSentMouseX = hoveredScreenX;
                lastSentMouseY = hoveredScreenY;
            }
        }

        // 当鼠标离开所有屏幕时，给上一块屏幕发一次“屏幕外”的 move 以清除 :hover。
        if (hoveredScreen < 0 && lastHoveredScreen >= 0) {
            screens[lastHoveredScreen].html.sendMouseMove(-1, -1);
            lastHoveredScreen = -1;
            lastSentMouseScreen = -1;
            lastSentMouseX = (std::numeric_limits<int32_t>::min)();
            lastSentMouseY = (std::numeric_limits<int32_t>::min)();
        } else if (hoveredScreen >= 0) {
            lastHoveredScreen = hoveredScreen;
        }


        // CSS cursor：由宿主读取 hit-test 位置的 computed cursor 并设置系统光标。
        // 注意：右键相机控制时启用 relative mouse mode，避免在此期间反复 Show/Set cursor。
        if (!input.right_mouse_down) {
            const char* css_cursor = "auto";
            if (hoveredScreen >= 0) {
                css_cursor = dong_view_get_cursor_at(screens[hoveredScreen].html.view, hoveredScreenX, hoveredScreenY);
            }
            applyCSSCursor(css_cursor);
        }

        // If we delayed mouse-up from last frame, deliver it now (after a frame rendered with :active).

        if (pendingMouseUp && pendingMouseUpScreen >= 0) {
            screens[pendingMouseUpScreen].html.sendMouseMove(pendingMouseUpX, pendingMouseUpY);
            screens[pendingMouseUpScreen].html.sendMouseUp(1);
            pendingMouseUp = false;
            pendingMouseUpScreen = -1;
        }

        // 处理鼠标按下
        if (input.left_mouse_pressed) {
            if (hoveredScreen >= 0) {
                // 点击屏幕，设置焦点
                focusedScreen = hoveredScreen;
                
                for (int i = 0; i < numScreens; i++) {
                    screens[i].focused = (i == focusedScreen);
                }

                // Capture press target so release goes to the same screen.
                pressedScreen = hoveredScreen;
                pressedX = hoveredScreenX;
                pressedY = hoveredScreenY;
                
                // 发送鼠标按下事件
                screens[hoveredScreen].html.sendMouseDown(1);
            } else {
                // 点击屏幕外，取消焦点
                focusedScreen = -1;
                for (int i = 0; i < numScreens; i++) {
                    screens[i].focused = false;
                }
                pressedScreen = -1;
            }
        }

        // 处理鼠标释放：始终发送给按下时命中的屏幕（否则会导致 :active 不稳定/卡住）。
        if (input.left_mouse_released) {
            const int targetScreen = (pressedScreen >= 0) ? pressedScreen : hoveredScreen;
            const int32_t upX = (pressedScreen >= 0) ? pressedX : hoveredScreenX;
            const int32_t upY = (pressedScreen >= 0) ? pressedY : hoveredScreenY;

            if (targetScreen >= 0) {
                if (input.left_mouse_pressed) {
                    // press+release in same frame: delay mouse-up by 1 frame for visible :active.
                    pendingMouseUp = true;
                    pendingMouseUpScreen = targetScreen;
                    pendingMouseUpX = upX;
                    pendingMouseUpY = upY;
                } else {
                    screens[targetScreen].html.sendMouseMove(upX, upY);
                    screens[targetScreen].html.sendMouseUp(1);
                }
            }

            pressedScreen = -1;
        }


        // 处理滚轮 - SDL 滚轮值通常是 -1 或 1
        // view.cpp 中已经有 kScrollSpeed = 20.0f，这里只需要小幅放大
        if (hoveredScreen >= 0 && (input.mouse_wheel_x != 0 || input.mouse_wheel_y != 0)) {
            constexpr float kWheelMultiplier = 3.0f;
            screens[hoveredScreen].html.sendMouseWheel(
                input.mouse_wheel_x * kWheelMultiplier, 
                input.mouse_wheel_y * kWheelMultiplier);
        }

        // 更新 HTML 屏幕（离屏渲染）
        // 注意：屏幕纹理会被主场景采样；不更新则复用上一帧纹理，仍保持“可见”。
        {
            DONG_PROFILE_SCOPE_CAT("OffscreenUpdates", "render");

            const double now_sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - scheduler_t0).count();
            const double video_interval = (video_max_fps > 0) ? (1.0 / (double)video_max_fps) : 0.0;
            const uint64_t cur_stamp = stamp++;

            auto markUpdated = [&](int idx) {
                if (idx < 0 || idx >= numScreens) return;
                if (updated_stamp[idx] == cur_stamp) return;

                if (bench_has_profile) {
                    char name[256];
                    std::snprintf(name, sizeof(name), "ScreenUpdate:%d:%s", idx, screenConfigs[idx].htmlFile);
                    ::dong::ProfilerScope __scope(name, screens[idx].isVideo ? "video" : "render");

                    screens[idx].html.update(device);
                } else {
                    screens[idx].html.update(device);
                }

                updated_stamp[idx] = cur_stamp;
            };


            if (!initial_full_update_done) {
                // 首帧先把所有屏幕都渲一次，避免大面积“空纹理”。
                for (int i = 0; i < numScreens; ++i) {
                    markUpdated(i);
                }
                initial_full_update_done = true;
            } else {
                // 交互屏：每帧
                if (hoveredScreen >= 0) {
                    markUpdated(hoveredScreen);
                }
                if (focusedScreen >= 0) {
                    markUpdated(focusedScreen);
                }

                // 视频屏：限频
                if (video_max_fps <= 0) {
                    // <=0 表示不限频（每帧更新）
                    for (int i = 0; i < numScreens; ++i) {
                        if (screens[i].isVideo) {
                            markUpdated(i);
                        }
                    }
                } else {
                    for (int i = 0; i < numScreens; ++i) {
                        if (!screens[i].isVideo) continue;
                        if (now_sec >= next_video_update_sec[i]) {
                            markUpdated(i);
                            next_video_update_sec[i] = now_sec + video_interval;
                        }
                    }
                }

                // 背景/静态：轮询摊到多帧
                int bg_updated = 0;
                const int bg_budget = bg_updates_per_frame;
                for (int attempts = 0; attempts < numScreens && bg_updated < bg_budget; ++attempts) {
                    const int idx = (bg_rr_cursor++) % numScreens;
                    if (idx == hoveredScreen || idx == focusedScreen) continue;
                    if (screens[idx].isVideo) continue;
                    markUpdated(idx);
                    bg_updated++;
                }
            }
        }

        // 更新 HUD（轻量，默认每帧）
        {
            DONG_PROFILE_SCOPE_CAT("HUDUpdate", "render");
            hud.update(device, dt);
        }


        
        // 注意：不要每帧全局等待 GPU idle。
        // 提交顺序已经能保证后续对 HTML 纹理的采样依赖；每帧 WaitForGPUIdle 会把性能直接锁死。
        // 如需排查同步/资源竞争问题，可设置环境变量 DONG_DEBUG_WAIT_GPU_IDLE_EVERY_FRAME=1。
        static const bool kWaitGpuIdleEveryFrame = (std::getenv("DONG_DEBUG_WAIT_GPU_IDLE_EVERY_FRAME") != nullptr);
        if (kWaitGpuIdleEveryFrame) {
            SDL_WaitForGPUIdle(device);
        }


        SDL_GPUCommandBuffer* cmd = nullptr;
        {
            DONG_PROFILE_SCOPE_CAT("SDL_AcquireGPUCommandBuffer", "gpu");
            cmd = SDL_AcquireGPUCommandBuffer(device);
        }
        
        // #region agent log
        static int cmd_acquire_count = 0;
        if (cmd_acquire_count < 5) {
            std::ofstream log_file("d:\\mix\\agents\\game\\indr\\dong\\.cursor\\debug.log", std::ios::app);
            if (log_file.is_open()) {
                std::stringstream ss;
                ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                ss << ",\"location\":\"3d_screen_script.cpp:1331\",\"message\":\"Command buffer acquire\",\"hypothesisId\":\"H11\",\"sessionId\":\"debug-session\",\"runId\":\"run1\"";
                ss << ",\"data\":{\"cmd\":" << (void*)cmd << ",\"count\":" << cmd_acquire_count << "}}\n";
                log_file << ss.str();
                log_file.close();
            }
            cmd_acquire_count++;
        }
        // #endregion
        
        if (!cmd) {
            // cmd_buf 不可用时避免 busy-spin
            const int sleep_ms = (frame_sleep_ms > 0) ? frame_sleep_ms : 1;
            {
                DONG_PROFILE_SCOPE_CAT("SDL_Delay", "frame");
                SDL_Delay(sleep_ms);
            }
            continue;
        }



        // Copy pass - 上传顶点数据
        // 这些几何体顶点在 demo 中是静态的：只需要上传一次即可，避免每帧 copy/upload 造成不必要的 GPU submit 开销。
        static bool kUploadedStaticVBs = false;
        if (!kUploadedStaticVBs) {
            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            uploadVertices(device, copyPass, cubeVB, cubeVerts);
            uploadVertices(device, copyPass, gridVB, gridVerts);

            // Upload debug triangle vertices (NDC)
            {
                std::vector<DebugTriVertex> dbgVerts;
                dbgVerts.reserve(3);

                // Default: small corner triangle (non-invasive).
                // Optional: fullscreen triangle to prove swapchain write path.
                const bool fullscreen_dbg_tri = (std::getenv("DONG_DEBUG_FULLSCREEN_TRI") != nullptr);
                if (fullscreen_dbg_tri) {
                    dbgVerts.push_back(DebugTriVertex{-1.0f, -1.0f});
                    dbgVerts.push_back(DebugTriVertex{ 3.0f, -1.0f});
                    dbgVerts.push_back(DebugTriVertex{-1.0f,  3.0f});
                } else {
                    dbgVerts.push_back(DebugTriVertex{-0.98f,  0.98f});
                    dbgVerts.push_back(DebugTriVertex{-0.70f,  0.98f});
                    dbgVerts.push_back(DebugTriVertex{-0.98f,  0.70f});
                }

                uploadVertices(device, copyPass, dbgTriVB, dbgVerts);
            }

            // 上传屏幕四边形顶点（静态）
            for (int i = 0; i < numScreens; i++) {
                auto quadVerts = generateQuad(screens[i].width, screens[i].height);
                uploadVertices(device, copyPass, screens[i].html.quadVB, quadVerts);
            }

            // 上传 HUD 四边形顶点（静态）
            auto hudQuadVerts = generateHUDQuad();
            uploadVertices(device, copyPass, hud.quadVB, hudQuadVerts);

            SDL_EndGPUCopyPass(copyPass);
            kUploadedStaticVBs = true;
        }


        // ========== 渲染主场景 ==========
        SDL_GPUTexture* swapchain = nullptr;
        Uint32 sw = 0, sh = 0;
        {
            // swapchain acquire：默认阻塞（稳定）。
            // 如需减少卡顿/等待，可用 NOWAIT（可能跳帧/撕裂）：DONG_GPU_SWAPCHAIN_NOWAIT=1
            bool ok = false;
            if (!swapchain_nowait) {
                DONG_PROFILE_SCOPE_CAT("SDL_WaitAndAcquireGPUSwapchainTexture", "gpu");
                ok = SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapchain, &sw, &sh);
            } else {
                DONG_PROFILE_SCOPE_CAT("SDL_AcquireGPUSwapchainTexture", "gpu");
                ok = SDL_AcquireGPUSwapchainTexture(cmd, window, &swapchain, &sw, &sh);
            }


            if (!ok || !swapchain) {
                // #region agent log
                static int swapchain_fail_count = 0;
                if (swapchain_fail_count < 3) {
                    std::ofstream log_file("d:\\mix\\agents\\game\\indr\\dong\\.cursor\\debug.log", std::ios::app);
                    if (log_file.is_open()) {
                        std::stringstream ss;
                        ss << "{\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                        ss << ",\"location\":\"3d_screen_script.cpp:1375\",\"message\":\"Swapchain acquire failed\",\"hypothesisId\":\"H11\",\"sessionId\":\"debug-session\",\"runId\":\"run1\"";
                        ss << ",\"data\":{\"ok\":" << (ok ? 1 : 0) << ",\"swapchain\":" << (void*)swapchain << ",\"count\":" << swapchain_fail_count << "}}\n";
                        log_file << ss.str();
                        log_file.close();
                    }
                    swapchain_fail_count++;
                }
                // #endregion
                
                {
                    DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBuffer", "gpu");
                    SDL_SubmitGPUCommandBuffer(cmd);
                }
                // swapchain 不可用时避免 busy-spin
                {
                    const int sleep_ms = swapchain_nowait ? 1 : ((frame_sleep_ms > 0) ? frame_sleep_ms : 1);
                    DONG_PROFILE_SCOPE_CAT("SDL_Delay", "frame");
                    SDL_Delay(sleep_ms);

                }
                continue;
            }

        }





        // 深度纹理
        if (!depthTexture || depthW != sw || depthH != sh) {
            if (depthTexture) SDL_ReleaseGPUTexture(device, depthTexture);
            depthTexture = createDepthTexture(device, sw, sh);
            depthW = sw;
            depthH = sh;
            static bool logged_depth_once = false;
            if (!logged_depth_once) {
                SDL_Log("[Render] Created depth texture: %p (size: %ux%u)", (void*)depthTexture, sw, sh);
                logged_depth_once = true;
            }
        }

        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = swapchain;
        colorTarget.clear_color = SDL_FColor{0.1f, 0.1f, 0.15f, 1.0f};  // Restore normal dark background
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTarget.store_op = SDL_GPU_STOREOP_STORE; // MUST store swapchain

        SDL_GPUDepthStencilTargetInfo depthTarget{};
        depthTarget.texture = depthTexture;
        depthTarget.clear_depth = 1.0f;
        depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
        depthTarget.store_op = SDL_GPU_STOREOP_DONT_CARE;

        // #region agent log
        static int frame_debug = 0;
        if (frame_debug < 3) {
            std::stringstream ss;
            ss << "{\"clearColor\":{\"r\":" << colorTarget.clear_color.r << ",\"g\":" << colorTarget.clear_color.g << ",\"b\":" << colorTarget.clear_color.b << "},\"swapchain\":" << (void*)swapchain << ",\"sw\":" << sw << ",\"sh\":" << sh << ",\"colorStoreOp\":" << (int)colorTarget.store_op << "}";
            debug_log("3d_screen_script.cpp:1350", "Before BeginGPURenderPass", "H1", ss.str().c_str());
            frame_debug++;
        }
        // #endregion

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, &depthTarget);
        
        if (!pass) {
            SDL_Log("[Render ERROR] SDL_BeginGPURenderPass returned null! swapchain=%p depthTexture=%p", (void*)swapchain, (void*)depthTexture);
        }
        // Viewport/scissor: SDL3 GPU backends typically default to full-target viewport.
        // Some older experiments attempted to set viewport/scissor via dynamically loaded symbols,
        // but the signature/owner object can differ across SDL builds and may result in undefined behavior.
        // For 3D pass, rely on default full-target viewport unless explicitly enabled.
        const bool enable_pass_viewport_scissor = (std::getenv("DONG_ENABLE_PASS_VIEWPORT_SCISSOR") != nullptr);
        if (pass && enable_pass_viewport_scissor) {
            setFullscreenViewportScissor(pass, sw, sh);
        }


        // 首帧调试：打印关键渲染状态
        static bool logged_first_frame = false;
        if (!logged_first_frame && pass) {
            SDL_Log("[Render] First frame: pass=%p swapchain=%p(%ux%u) depth=%p", 
                    (void*)pass, (void*)swapchain, sw, sh, (void*)depthTexture);
            SDL_Log("[Render] Camera: pos=(%.2f,%.2f,%.2f) yaw=%.2f pitch=%.2f",
                    camera.position.x, camera.position.y, camera.position.z, camera.yaw, camera.pitch);
            logged_first_frame = true;
        }

        float aspect = (float)sw / (float)sh;
        Mat4 view = camera.getViewMatrix();
        Mat4 proj = camera.getProjectionMatrix(aspect);
        Mat4 vp = proj * view;

        // Debug draw: NDC triangle (proves swapchain write path). Default OFF.
        static const bool kDrawDbgTri = (std::getenv("DONG_DEBUG_DRAW_TRI") != nullptr);
        if (kDrawDbgTri && pass) {
            SDL_BindGPUGraphicsPipeline(pass, dbgTriPipeline);
            SDL_GPUBufferBinding dbgBinding{dbgTriVB, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &dbgBinding, 1);

            // #region agent log
            static int dbg_tri_logged = 0;
            if (dbg_tri_logged < 3) {
                std::stringstream ss;
                ss << "{\"pipeline\":" << (void*)dbgTriPipeline
                   << ",\"vb\":" << (void*)dbgTriVB
                   << ",\"pass\":" << (void*)pass
                   << ",\"sw\":" << sw << ",\"sh\":" << sh << "}";
                debug_log("3d_screen_script.cpp:dbgtri", "Drawing debug NDC triangle", "H14", ss.str().c_str());
                dbg_tri_logged++;
            }
            // #endregion

            SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
        }

        // Debug draw: draw cubeVB without MVP/uniforms (should appear as green shape near center).
        // Default OFF. Enable with DONG_DEBUG_DRAW_MESH=1
        static const bool kDrawDbgMesh = (std::getenv("DONG_DEBUG_DRAW_MESH") != nullptr);
        if (kDrawDbgMesh && pass) {
            SDL_BindGPUGraphicsPipeline(pass, dbgMeshPipeline);
            SDL_GPUBufferBinding vb{cubeVB, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

            // #region agent log
            static int dbg_mesh_logged = 0;
            if (dbg_mesh_logged < 3) {
                std::stringstream ss;
                ss << "{\"pipeline\":" << (void*)dbgMeshPipeline
                   << ",\"vb\":" << (void*)cubeVB
                   << ",\"verts\":" << cubeVerts.size()
                   << ",\"pass\":" << (void*)pass << "}";
                debug_log("3d_screen_script.cpp:dbgmesh", "Drawing debug mesh (no MVP) using cubeVB", "H15", ss.str().c_str());
                dbg_mesh_logged++;
            }
            // #endregion

            SDL_DrawGPUPrimitives(pass, (Uint32)cubeVerts.size(), 1, 0, 0);
        }

        // #region agent log
        if (frame_debug < 3) {
            std::stringstream ss;
            ss << "{\"aspect\":" << aspect << ",\"cameraPos\":{\"x\":" << camera.position.x << ",\"y\":" << camera.position.y << ",\"z\":" << camera.position.z << "},\"pass\":" << (void*)pass << "}";
            debug_log("3d_screen_script.cpp:1378", "After computing VP matrix", "H2", ss.str().c_str());
        }
        // #endregion

        // 绘制网格
        SDL_BindGPUGraphicsPipeline(pass, gridPipeline);
        SDL_GPUBufferBinding gridBinding{gridVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &gridBinding, 1);
        
        Uniforms3D gridUniforms{};
        Mat4 identity = Mat4::identity();
        memcpy(gridUniforms.mvp, vp.m, sizeof(float) * 16);
        memcpy(gridUniforms.model, identity.m, sizeof(float) * 16);
        gridUniforms.lightDir[0] = 0.5f; gridUniforms.lightDir[1] = 1.0f; gridUniforms.lightDir[2] = 0.3f;
        gridUniforms.ambient[0] = 0.3f; gridUniforms.ambient[1] = 0.3f; gridUniforms.ambient[2] = 0.3f;
        
        SDL_PushGPUVertexUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        SDL_PushGPUFragmentUniformData(cmd, 0, &gridUniforms, sizeof(gridUniforms));
        
        static bool logged_draw_calls = false;
        if (!logged_draw_calls) {
            SDL_Log("[Render] Draw: grid=%zu verts, cube=%zu verts", gridVerts.size(), cubeVerts.size());
            logged_draw_calls = true;
        }
        
        // #region agent log
        if (frame_debug < 3) {
            std::stringstream ss;
            ss << "{\"gridVerts\":" << gridVerts.size() << ",\"pipeline\":" << (void*)gridPipeline << ",\"vb\":" << (void*)gridVB;
            ss << ",\"mvp\":[";
            for (int i = 0; i < 16; i++) {
                if (i > 0) ss << ",";
                ss << vp.m[i];
            }
            ss << "]}";
            debug_log("3d_screen_script.cpp:1404", "Before DrawGPUPrimitives grid", "H3", ss.str().c_str());
        }
        // #endregion

        SDL_DrawGPUPrimitives(pass, (Uint32)gridVerts.size(), 1, 0, 0);

        // 绘制主立方体
        SDL_BindGPUGraphicsPipeline(pass, cubePipeline);
        SDL_GPUBufferBinding cubeBinding{cubeVB, 0};
        SDL_BindGPUVertexBuffers(pass, 0, &cubeBinding, 1);
        
        Mat4 mainCubeModel = Mat4::translate(Vec3{0, 0.5f, 0}) * Mat4::rotateY(mainCubeRotation);
        Mat4 mainCubeMVP = vp * mainCubeModel;
        
        Uniforms3D mainCubeUniforms{};
        memcpy(mainCubeUniforms.mvp, mainCubeMVP.m, sizeof(float) * 16);
        memcpy(mainCubeUniforms.model, mainCubeModel.m, sizeof(float) * 16);
        mainCubeUniforms.lightDir[0] = 0.5f; mainCubeUniforms.lightDir[1] = 1.0f; mainCubeUniforms.lightDir[2] = 0.3f;
        mainCubeUniforms.ambient[0] = 0.2f; mainCubeUniforms.ambient[1] = 0.2f; mainCubeUniforms.ambient[2] = 0.25f;
        
        SDL_PushGPUVertexUniformData(cmd, 0, &mainCubeUniforms, sizeof(mainCubeUniforms));
        SDL_PushGPUFragmentUniformData(cmd, 0, &mainCubeUniforms, sizeof(mainCubeUniforms));
        // #region agent log
        if (frame_debug < 3) {
            std::stringstream ss;
            ss << "{\"cubeVerts\":" << cubeVerts.size() << ",\"pipeline\":" << (void*)cubePipeline << "}";
            debug_log("3d_screen_script.cpp:1420", "Before DrawGPUPrimitives cube", "H3", ss.str().c_str());
        }
        // #endregion

        SDL_DrawGPUPrimitives(pass, (Uint32)cubeVerts.size(), 1, 0, 0);

        // 绘制 HTML 屏幕
        SDL_BindGPUGraphicsPipeline(pass, screenPipeline);
        
        static bool logged_screen_status = false;
        if (!logged_screen_status) {
            int valid_count = 0;
            for (int i = 0; i < numScreens; i++) {
                if (screens[i].html.renderTexture) valid_count++;
            }
            SDL_Log("[Render] Screen textures: %d/%d valid", valid_count, numScreens);
            // 打印前几个屏幕的位置和纹理状态
            for (int i = 0; i < (std::min)(5, numScreens); i++) {
                SDL_Log("[Render] Screen %d: pos=(%.2f,%.2f,%.2f) texture=%p", 
                        i, screens[i].position.x, screens[i].position.y, screens[i].position.z,
                        (void*)screens[i].html.renderTexture);
            }
            logged_screen_status = true;
        }
        
        for (int i = 0; i < numScreens; i++) {
            // #region agent log
            if (frame_debug < 3 && i < 3) {
                std::stringstream ss;
                ss << "{\"screenIndex\":" << i << ",\"hasTexture\":" << (screens[i].html.renderTexture != nullptr) << ",\"texture\":" << (void*)screens[i].html.renderTexture << ",\"pos\":{\"x\":" << screens[i].position.x << ",\"y\":" << screens[i].position.y << ",\"z\":" << screens[i].position.z << "}}";
                debug_log("3d_screen_script.cpp:1443", "Rendering screen", "H4", ss.str().c_str());
            }
            // #endregion

            if (!screens[i].html.renderTexture) {
                static int missing_count = 0;
                if (missing_count < 3) {
                    SDL_Log("[Render] Screen %d has no renderTexture!", i);
                    missing_count++;
                }
                continue;
            }
            
            SDL_GPUBufferBinding screenBinding{screens[i].html.quadVB, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &screenBinding, 1);
            
            SDL_GPUTextureSamplerBinding texBinding{screens[i].html.renderTexture, sampler};
            SDL_BindGPUFragmentSamplers(pass, 0, &texBinding, 1);
            
            Mat4 screenModel = getScreenTransform(screens[i]);
            Mat4 screenMVP = vp * screenModel;
            
            // #region agent log
            if (frame_debug < 3 && i < 3) {
                std::stringstream ss;
                ss << "{\"screenIndex\":" << i << ",\"mvp\":[";
                for (int j = 0; j < 16; j++) {
                    if (j > 0) ss << ",";
                    ss << screenMVP.m[j];
                }
                ss << "],\"screenPos\":{\"x\":" << screens[i].position.x << ",\"y\":" << screens[i].position.y << ",\"z\":" << screens[i].position.z << "}}";
                debug_log("3d_screen_script.cpp:1460", "Screen MVP computed", "H5", ss.str().c_str());
            }
            // #endregion
            
            UniformsTextured screenUniforms{};
            memcpy(screenUniforms.mvp, screenMVP.m, sizeof(float) * 16);
            memcpy(screenUniforms.model, screenModel.m, sizeof(float) * 16);
            screenUniforms.color[0] = 1.0f; screenUniforms.color[1] = 1.0f;
            screenUniforms.color[2] = 1.0f; screenUniforms.color[3] = 1.0f;
            screenUniforms.highlight[0] = screens[i].hovered ? 1.0f : 0.0f;
            screenUniforms.highlight[1] = screens[i].focused ? 1.0f : 0.0f;
            
            SDL_PushGPUVertexUniformData(cmd, 0, &screenUniforms, sizeof(screenUniforms));
            SDL_PushGPUFragmentUniformData(cmd, 0, &screenUniforms, sizeof(screenUniforms));
            
            // #region agent log
            if (frame_debug < 3 && i < 3) {
                std::stringstream ss;
                ss << "{\"screenIndex\":" << i << ",\"pipeline\":" << (void*)screenPipeline << ",\"vb\":" << (void*)screens[i].html.quadVB << "}";
                debug_log("3d_screen_script.cpp:1472", "Before DrawGPUPrimitives screen", "H6", ss.str().c_str());
            }
            // #endregion

            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
        }

        // Debug: optional fullscreen tint using the *HUD pipeline* (known-good) while still in the 3D pass.
        // NOTE: This MUST NOT be enabled by default, otherwise it can overwrite the 3D scene depending on HUD texture content.
        static const bool kEnable3DPassTint = (std::getenv("DONG_DEBUG_3D_PASS_TINT") != nullptr);
        if (kEnable3DPassTint && pass) {
            SDL_BindGPUGraphicsPipeline(pass, hud.pipeline);
            SDL_GPUBufferBinding binding{hud.quadVB, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);

            // Use a known-valid sampled texture to satisfy the shader's sampler binding.
            SDL_GPUTextureSamplerBinding texBinding{hud.html.renderTexture, sampler};
            SDL_BindGPUFragmentSamplers(pass, 0, &texBinding, 1);

            UniformsHUD u{};
            u.color[0] = 1.0f; u.color[1] = 0.0f; u.color[2] = 1.0f; u.color[3] = 0.35f; // translucent tint
            SDL_PushGPUFragmentUniformData(cmd, 0, &u, sizeof(u));
            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
        }

        SDL_EndGPURenderPass(pass);
        
        // #region agent log
        if (frame_debug < 3) {
            std::stringstream ss;
            ss << "{\"passEnded\":true,\"swapchain\":" << (void*)swapchain << "}";
            debug_log("3d_screen_script.cpp:1604", "After EndGPURenderPass (3D scene)", "H7", ss.str().c_str());
        }
        // #endregion

        // ========== 渲染 HUD（2D 覆盖层，无深度测试）==========
        // Debug: allow disabling HUD pass entirely to isolate white-screen cause.
        const bool disable_hud_for_test = (std::getenv("DONG_DEBUG_DISABLE_HUD") != nullptr);
        if (!disable_hud_for_test) {
            SDL_GPUColorTargetInfo hudColorTarget{};
            hudColorTarget.texture = swapchain;
            hudColorTarget.load_op = SDL_GPU_LOADOP_LOAD;  // Preserve 3D scene
            hudColorTarget.store_op = SDL_GPU_STOREOP_STORE; // MUST store, otherwise swapchain contents become undefined

            // Diagnostic switches (default OFF):
            // - DONG_DEBUG_HUD_FORCE_LOAD=1 : force LOAD (no clear)
            // - DONG_DEBUG_HUD_FORCE_CLEAR=1: force CLEAR to transparent black
            if (std::getenv("DONG_DEBUG_HUD_FORCE_LOAD")) {
                hudColorTarget.load_op = SDL_GPU_LOADOP_LOAD;
            }
            if (std::getenv("DONG_DEBUG_HUD_FORCE_CLEAR")) {
                hudColorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
                hudColorTarget.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f};
            }
            
            // #region agent log
            if (frame_debug < 3) {
                std::stringstream ss;
                ss << "{\"hudLoadOp\":" << (int)hudColorTarget.load_op
                   << ",\"hudStoreOp\":" << (int)hudColorTarget.store_op
                   << ",\"hudClearColor\":{\"r\":" << hudColorTarget.clear_color.r
                   << ",\"g\":" << hudColorTarget.clear_color.g
                   << ",\"b\":" << hudColorTarget.clear_color.b
                   << ",\"a\":" << hudColorTarget.clear_color.a
                   << "}"
                   << ",\"swapchain\":" << (void*)swapchain
                   << ",\"sw\":" << sw << ",\"sh\":" << sh
                   << "}";
                debug_log("3d_screen_script.cpp:1611", "Before HUD render pass", "H8", ss.str().c_str());
            }
            // #endregion
            
        SDL_GPURenderPass* hudPass = SDL_BeginGPURenderPass(cmd, &hudColorTarget, 1, nullptr);
        if (hudPass) {
            const bool enable_pass_viewport_scissor = (std::getenv("DONG_ENABLE_PASS_VIEWPORT_SCISSOR") != nullptr);
            if (enable_pass_viewport_scissor) {
                setFullscreenViewportScissor(hudPass, sw, sh);
            }

                // #region agent log
                if (frame_debug < 3) {
                    std::stringstream ss;
                    ss << "{\"hudPass\":" << (void*)hudPass << ",\"hudTexture\":" << (void*)hud.html.renderTexture << "}";
                    debug_log("3d_screen_script.cpp:1629", "Before HUD render", "H8", ss.str().c_str());
                }
                // #endregion
                
                // Diagnostic: restrict HUD to a small scissor rect to verify whether HUD is covering the whole screen.
                // If 3D becomes visible outside the scissor, it proves the white screen is caused by HUD full-screen draw.
                if (std::getenv("DONG_DEBUG_HUD_SCISSOR_256")) {
                    SDL_Rect sc{0, 0, 256, 256};
                    ensureViewportScissorFnsLoaded();
                    if (g_pfnSetGPUScissor) {
                        g_pfnSetGPUScissor(hudPass, &sc);
                    }
                }

                hud.render(hudPass, cmd, sampler);
                
                // #region agent log
                if (frame_debug < 3) {
                    debug_log("3d_screen_script.cpp:1630", "After HUD render", "H8", "{}");
                }
                // #endregion
                
                SDL_EndGPURenderPass(hudPass);
            } else {
                SDL_Log("[Render ERROR] SDL_BeginGPURenderPass (HUD) returned null! swapchain=%p(%ux%u)", (void*)swapchain, sw, sh);
            }
        } else {
            // When HUD is disabled, still run a minimal 2D pass that LOADs the swapchain and ends immediately.
            // On some backends, having a second pass (or at least touching the swapchain in a predictable way)
            // avoids presenting an uninitialized/cleared surface that looks like a white screen.
            SDL_GPUColorTargetInfo hudColorTarget{};
            hudColorTarget.texture = swapchain;
            hudColorTarget.load_op = SDL_GPU_LOADOP_LOAD;
            hudColorTarget.store_op = SDL_GPU_STOREOP_STORE; // MUST store

            SDL_GPURenderPass* hudPass = SDL_BeginGPURenderPass(cmd, &hudColorTarget, 1, nullptr);
            if (hudPass) {
                const bool enable_pass_viewport_scissor = (std::getenv("DONG_ENABLE_PASS_VIEWPORT_SCISSOR") != nullptr);
                if (enable_pass_viewport_scissor) {
                    setFullscreenViewportScissor(hudPass, sw, sh);
                }
                SDL_EndGPURenderPass(hudPass);
            } else {
                SDL_Log("[Render ERROR] SDL_BeginGPURenderPass (HUD noop) returned null! swapchain=%p(%ux%u)", (void*)swapchain, sw, sh);
            }

            // #region agent log
            if (frame_debug < 3) {
                debug_log("3d_screen_script.cpp:1640", "HUD disabled for testing (noop pass)", "H8", "{}");
            }
            // #endregion
        }


        {
            DONG_PROFILE_SCOPE_CAT("SDL_SubmitGPUCommandBuffer", "gpu");
            SDL_SubmitGPUCommandBuffer(cmd);
        }

        if (frame_sleep_ms > 0) {
            DONG_PROFILE_SCOPE_CAT("SDL_Delay", "frame");
            SDL_Delay(frame_sleep_ms);
        }


    }

    SDL_Log("=== Exiting main loop ===");
    fflush(stdout);

    // NOTE: Do not attempt extra swapchain capture during shutdown.
    // It is unreliable and makes debugging ambiguous. We capture deterministically in-frame.

    // 停止文本输入
    SDL_Log("[Cleanup] Stopping text input...");
    SDL_StopTextInput(window);

    // 清理
    SDL_Log("[Cleanup] Releasing depth texture...");
    if (depthTexture) SDL_ReleaseGPUTexture(device, depthTexture);
    
    SDL_Log("[Cleanup] Cleaning up HUD...");
    hud.cleanup(device);
    
    SDL_Log("[Cleanup] Cleaning up %d screens...", numScreens);
    for (int i = 0; i < numScreens; i++) {
        SDL_Log("[Cleanup] Screen %d...", i);
        screens[i].html.cleanup(device);
    }
    
    SDL_Log("[Cleanup] Releasing GPU buffers...");
    SDL_ReleaseGPUBuffer(device, cubeVB);
    SDL_ReleaseGPUBuffer(device, gridVB);
    SDL_ReleaseGPUSampler(device, sampler);
    SDL_Log("[Cleanup] Releasing pipelines...");
    SDL_ReleaseGPUGraphicsPipeline(device, cubePipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, gridPipeline);
    SDL_ReleaseGPUGraphicsPipeline(device, screenPipeline);
    SDL_Log("[Cleanup] Releasing shaders...");
    SDL_ReleaseGPUShader(device, vs3d);
    SDL_ReleaseGPUShader(device, fsCube);
    SDL_ReleaseGPUShader(device, fsGrid);
    SDL_ReleaseGPUShader(device, vsTextured);
    SDL_ReleaseGPUShader(device, fsTextured);
    SDL_ReleaseGPUShader(device, vsHUD);
    SDL_ReleaseGPUShader(device, fsHUD);
    
    SDL_Log("[Cleanup] Destroying dong context...");
    dong_destroy_context(dongCtx);
    
    // Dump profiler BEFORE destroying GPU device (in case it crashes)
    if (!g_profile_output.empty()) {
        SDL_Log("[Profile] Dumping profiler trace before GPU cleanup...");
        int result = dong_profiler_dump(g_profile_output.c_str());
        SDL_Log("[Profile] Dump result=%d, file=%s", result, g_profile_output.c_str());
        g_profile_output.clear();  // Prevent atexit from dumping again
    }
    
    SDL_Log("[Cleanup] Destroying GPU device...");
    SDL_DestroyGPUDevice(device);
    SDL_Log("[Cleanup] Destroying window...");
    SDL_DestroyWindow(window);
    SDL_Log("[Cleanup] ShaderCross quit...");
    SDL_ShaderCross_Quit();

    SDL_Log("[Profile] Normal exit path complete");

    SDL_Log("=== Demo Complete ===");
    SDL_Quit();

    // atexit(dumpProfilerAtExit) will be called after this
    return 0;
}
