#include "feature_test_framework.hpp"
#include <dong.h>
#include <dong_platform.h>
#include <dong_gpu_driver.h>
#include "dong_sdl_platform.h"
#include "dong_sdl_gpu_formats.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <map>
#include <cstring>
#include <cctype>


namespace fs = std::filesystem;

namespace dong::test {

// Static renderer state
static SDL_Window* s_window = nullptr;
static SDL_GPUDevice* s_device = nullptr;
static SDL_GPUTexture* s_offscreen_texture = nullptr;
static int s_offscreen_w = 0;
static int s_offscreen_h = 0;
static dong_engine_t* s_engine = nullptr;
static int s_current_width = 0;
static int s_current_height = 0;
static int s_platform_initialized = 0;

FeatureTestRunner::FeatureTestRunner() = default;

FeatureTestRunner::~FeatureTestRunner() {
    cleanupRenderer();
}

void FeatureTestRunner::addTest(const FeatureTestCase& test) {
    tests_.push_back(test);
}

void FeatureTestRunner::addTests(const std::vector<FeatureTestCase>& tests) {
    tests_.insert(tests_.end(), tests.begin(), tests.end());
}

static std::string findResourceRoot() {
    std::vector<fs::path> candidates;
    candidates.emplace_back("data/tests");
    candidates.emplace_back("zig-out/bin/data/tests");

    if (const char* base = SDL_GetBasePath()) {
        fs::path exe_dir(base);
        candidates.emplace_back(exe_dir / "data/tests");
        candidates.emplace_back(exe_dir / "../data/tests");
        candidates.emplace_back(exe_dir / "../../examples/data/tests");
    }

    std::error_code ec;
    for (const auto& p : candidates) {
        if (fs::exists(p, ec) && fs::is_directory(p, ec)) {
            return p.lexically_normal().string();
        }
    }

    return std::string();
}

static SDL_GPUTexture* ensureOffscreenTexture(SDL_GPUDevice* device, int width, int height) {
    if (!device) return nullptr;

    if (s_offscreen_texture && s_offscreen_w == width && s_offscreen_h == height) {
        return s_offscreen_texture;
    }

    if (s_offscreen_texture) {
        SDL_ReleaseGPUTexture(device, s_offscreen_texture);
        s_offscreen_texture = nullptr;
    }

    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;

    tex_info.width = static_cast<Uint32>(width);
    tex_info.height = static_cast<Uint32>(height);
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    s_offscreen_texture = SDL_CreateGPUTexture(device, &tex_info);
    if (!s_offscreen_texture) {
        SDL_Log("ERROR: Failed to create offscreen texture: %s", SDL_GetError());
        return nullptr;
    }

    s_offscreen_w = width;
    s_offscreen_h = height;
    return s_offscreen_texture;
}

static bool envFlagEnabled(const char* name, bool default_value) {
    const char* v = std::getenv(name);
    if (!v || !v[0]) return default_value;
    return (v[0] != '0');
}

static bool downloadIsBGRA(SDL_GPUDevice* device) {
    (void)device;
    if (envFlagEnabled("DONG_GPU_DOWNLOAD_BGRA", false)) {
        return true;
    }
    if (envFlagEnabled("DONG_GPU_DOWNLOAD_RGBA", false)) {
        return false;
    }
    return false;
}

static void fixupDownloadedPixelsToRGBA(SDL_GPUDevice* device, int width, int height, uint8_t* pixels) {
    if (!device || !pixels) return;
    if (!downloadIsBGRA(device)) return;

    const size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    for (size_t i = 0; i < pixel_count; ++i) {
        uint8_t* p = pixels + i * 4;
        std::swap(p[0], p[2]);
    }
}

static bool downloadTextureRGBA(SDL_GPUDevice* device, SDL_GPUTexture* texture,
                                int width, int height, uint8_t* out_pixels) {
    if (!device || !texture || !out_pixels) {
        return false;
    }

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = static_cast<uint32_t>(width * height * 4);

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
    src_region.w = static_cast<Uint32>(width);
    src_region.h = static_cast<Uint32>(height);
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

    std::memcpy(out_pixels, mapped, static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
    SDL_UnmapGPUTransferBuffer(device, download_buffer);
    SDL_ReleaseGPUTransferBuffer(device, download_buffer);

    fixupDownloadedPixelsToRGBA(device, width, height, out_pixels);
    return true;
}

static bool ensureEngineInitialized(int width, int height) {
    if (s_engine && s_current_width == width && s_current_height == height) {
        return true;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("ERROR: SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    s_window = SDL_CreateWindow("Feature Test", width, height, SDL_WINDOW_HIDDEN);
    if (!s_window) {
        SDL_Log("ERROR: SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    s_device = SDL_CreateGPUDevice(dong_sdl_default_shader_formats(), false, nullptr);
    if (!s_device) {
        SDL_Log("ERROR: SDL_CreateGPUDevice failed: %s", SDL_GetError());
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(s_device, s_window)) {
        SDL_Log("ERROR: SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
        return false;
    }

    if (!s_platform_initialized) {
        if (!dong_sdl_platform_init(static_cast<void*>(s_device), static_cast<void*>(s_window))) {
            SDL_Log("ERROR: dong_sdl_platform_init failed");
            return false;
        }
        s_platform_initialized = 1;
    }

    dong_engine_desc_t desc{};
    desc.api_version = DONG_API_VERSION;
    desc.width = static_cast<uint32_t>(width);
    desc.height = static_cast<uint32_t>(height);

    if (dong_engine_create(&desc, &s_engine) != DONG_OK || !s_engine) {
        SDL_Log("ERROR: dong_engine_create failed");
        return false;
    }

    if (dong_engine_set_gpu(s_engine, static_cast<void*>(s_device), static_cast<void*>(s_window)) != DONG_OK) {
        SDL_Log("ERROR: dong_engine_set_gpu failed");
        return false;
    }

    s_current_width = width;
    s_current_height = height;

    const std::string root = findResourceRoot();
    if (!root.empty()) {
        DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
        dong_gpu_set_resource_root(driver, root.c_str());
    }

    return true;
}

bool FeatureTestRunner::initializeRenderer(int width, int height) {
    cleanupRenderer();
    return ensureEngineInitialized(width, height);
}

void FeatureTestRunner::cleanupRenderer() {
    if (s_engine) {
        dong_engine_destroy(s_engine);
        s_engine = nullptr;
    }
    if (s_platform_initialized) {
        dong_sdl_platform_shutdown();
        s_platform_initialized = 0;
    }
    if (s_offscreen_texture && s_device) {
        SDL_ReleaseGPUTexture(s_device, s_offscreen_texture);
        s_offscreen_texture = nullptr;
    }
    if (s_device) {
        if (s_window) {
            SDL_ReleaseWindowFromGPUDevice(s_device, s_window);
        }
        SDL_DestroyGPUDevice(s_device);
        s_device = nullptr;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = nullptr;
    }
    SDL_Quit();
    s_current_width = 0;
    s_current_height = 0;
    s_offscreen_w = 0;
    s_offscreen_h = 0;
}

bool FeatureTestRunner::renderToPixels(const std::string& html, int width, int height,
                                       std::vector<uint8_t>& pixels) {
    if (!ensureEngineInitialized(width, height)) {
        return false;
    }

    if (s_current_width != width || s_current_height != height) {
        dong_engine_resize(s_engine, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        s_current_width = width;
        s_current_height = height;
    }

    if (dong_engine_load_html(s_engine, html.c_str()) != DONG_OK) {
        return false;
    }

    SDL_GPUTexture* target = ensureOffscreenTexture(s_device, width, height);
    if (!target) {
        return false;
    }

    DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
    if (!driver) {
        return false;
    }

    if (!dong_gpu_begin_frame_offscreen(driver, target, static_cast<uint32_t>(width), static_cast<uint32_t>(height))) {
        return false;
    }

    if (dong_engine_tick(s_engine) != DONG_OK) {
        dong_gpu_end_frame_offscreen(driver);
        return false;
    }

    dong_gpu_end_frame_offscreen(driver);

    pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
    return downloadTextureRGBA(s_device, target, width, height, pixels.data());
}

bool FeatureTestRunner::saveBMP(const std::string& path, int width, int height, const uint8_t* pixels) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    uint32_t filesize = 54 + width * height * 3;
    uint8_t bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize >> 8);
    bmpfileheader[4] = (uint8_t)(filesize >> 16);
    bmpfileheader[5] = (uint8_t)(filesize >> 24);

    uint8_t bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
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

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            uint8_t rgb[3] = {pixels[idx + 2], pixels[idx + 1], pixels[idx + 0]};
            fwrite(rgb, 3, 1, f);
        }
    }

    fclose(f);
    return true;
}

FeatureTestResult FeatureTestRunner::runTest(const FeatureTestCase& test, const std::string& output_dir) {
    FeatureTestResult result;
    result.name = test.name;
    result.category = test.category;
    result.description = test.description;

    std::filesystem::create_directories(output_dir);

    std::string safe_name = test.name;
    for (char& c : safe_name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') c = '_';
    }
    result.screenshot_path = output_dir + "/" + test.category + "_" + safe_name + ".bmp";

    std::vector<uint8_t> pixels;
    if (!renderToPixels(test.html, test.width, test.height, pixels)) {
        result.passed = false;
        result.error_message = "Failed to render HTML";
        return result;
    }

    if (!saveBMP(result.screenshot_path, test.width, test.height, pixels.data())) {
        result.passed = false;
        result.error_message = "Failed to save screenshot";
        return result;
    }

    if (test.validator) {
        result.passed = test.validator(pixels.data(), test.width, test.height);
        if (!result.passed) {
            result.error_message = "Validation failed";
        }
    } else {
        result.passed = true;
    }

    return result;
}

std::vector<FeatureTestResult> FeatureTestRunner::runAll(const std::string& output_dir) {
    std::vector<FeatureTestResult> results;

    SDL_Log("Running %zu feature tests...", tests_.size());

    for (size_t i = 0; i < tests_.size(); ++i) {
        SDL_Log("[%zu/%zu] Testing: %s - %s",
                i + 1, tests_.size(), tests_[i].category.c_str(), tests_[i].name.c_str());

        auto result = runTest(tests_[i], output_dir);
        results.push_back(result);

        SDL_Log("  %s: %s", result.passed ? "PASS" : "FAIL",
                result.error_message.empty() ? result.screenshot_path.c_str() : result.error_message.c_str());
    }

    int passed = 0, failed = 0;
    for (const auto& r : results) {
        if (r.passed) ++passed;
        else ++failed;
    }
    SDL_Log("\n=== Test Summary ===");
    SDL_Log("Passed: %d, Failed: %d, Total: %zu", passed, failed, results.size());

    return results;
}

void FeatureTestRunner::generateReport(const std::vector<FeatureTestResult>& results,
                                         const std::string& output_path) {
    std::ofstream file(output_path);
    if (!file) return;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    file << "<!DOCTYPE html>\n<html>\n<head>\n";
    file << "<meta charset=\"UTF-8\">\n";
    file << "<title>Feature Test Report</title>\n";
    file << "<style>\n";
    file << "body { font-family: Arial, sans-serif; margin: 20px; background: #1a1a2e; color: #eee; }\n";
    file << "h1 { color: #e94560; }\n";
    file << ".summary { background: #16213e; padding: 15px; border-radius: 8px; margin-bottom: 20px; }\n";
    file << ".test { background: #16213e; padding: 15px; border-radius: 8px; margin-bottom: 10px; }\n";
    file << ".pass { border-left: 4px solid #00ff88; }\n";
    file << ".fail { border-left: 4px solid #e94560; }\n";
    file << ".category { color: #00d4ff; font-size: 12px; }\n";
    file << ".name { font-weight: bold; margin: 5px 0; }\n";
    file << ".screenshot { max-width: 400px; margin-top: 10px; border-radius: 4px; }\n";
    file << ".error { color: #e94560; font-size: 12px; }\n";
    file << "</style>\n</head>\n<body>\n";

    file << "<h1>Feature Test Report</h1>\n";
    file << "<p>Generated: " << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "</p>\n";

    int passed = 0, failed = 0;
    for (const auto& r : results) {
        if (r.passed) ++passed;
        else ++failed;
    }

    file << "<div class=\"summary\">\n";
    file << "<p>Total Tests: " << results.size() << "</p>\n";
    file << "<p>Passed: " << passed << "</p>\n";
    file << "<p>Failed: " << failed << "</p>\n";
    file << "</div>\n";

    for (const auto& r : results) {
        file << "<div class=\"test " << (r.passed ? "pass" : "fail") << "\">\n";
        file << "<div class=\"category\">" << r.category << "</div>\n";
        file << "<div class=\"name\">" << r.name << "</div>\n";
        file << "<div class=\"description\">" << r.description << "</div>\n";
        if (r.passed) {
            file << "<img class=\"screenshot\" src=\"" << r.screenshot_path << "\" />\n";
        } else {
            file << "<div class=\"error\">" << r.error_message << "</div>\n";
        }
        file << "</div>\n";
    }

    file << "</body>\n</html>\n";
}

namespace tests {

static std::string readTextFile(const fs::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static std::vector<FeatureTestCase> buildTestsFromDir(const fs::path& dir) {
    std::vector<FeatureTestCase> tests;
    if (dir.empty()) {
        return tests;
    }

    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        return tests;
    }

    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (entry.path().extension() != ".html") continue;

        FeatureTestCase test;
        test.name = entry.path().stem().string();
        test.category = "auto";
        test.description = "Auto-loaded HTML test";
        test.width = 800;
        test.height = 600;
        test.html = readTextFile(entry.path());
        if (!test.html.empty()) {
            tests.push_back(std::move(test));
        }
    }

    return tests;
}

std::vector<FeatureTestCase> getAllTests() {
    const std::string root = findResourceRoot();
    return buildTestsFromDir(fs::path(root));
}

std::vector<FeatureTestCase> getBoxModelTests() { return {}; }
std::vector<FeatureTestCase> getFlexboxTests() { return {}; }
std::vector<FeatureTestCase> getTransformTests() { return {}; }
std::vector<FeatureTestCase> getAnimationTests() { return {}; }
std::vector<FeatureTestCase> getVisualEffectsTests() { return {}; }
std::vector<FeatureTestCase> getTextTests() { return {}; }
std::vector<FeatureTestCase> getDOMAPITests() { return {}; }

} // namespace tests

} // namespace dong::test

