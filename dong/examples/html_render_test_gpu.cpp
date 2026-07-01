/**
 * GPU backend headless HTML render test (in-dreaming/gpu, no dong_sdl_backend).
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

#include <dong.h>
#include <dong_platform.h>
#include <dong_gpu_driver.h>
#include <dong_gpu_backend_driver.h>
#include <dong_ui_graph.h>

#include "gpu/gpu.h"

namespace fs = std::filesystem;

static bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba) {
    if (!filename || !rgba) {
        return false;
    }
    const uint32_t row_bytes = ((width * 3 + 3) / 4) * 4;
    const uint32_t pixel_bytes = row_bytes * height;
    const uint32_t file_size = 14 + 40 + pixel_bytes;

    FILE* f = fopen(filename, "wb");
    if (!f) {
        return false;
    }

    uint8_t header[14] = {'B', 'M'};
    header[2] = (uint8_t)(file_size);
    header[3] = (uint8_t)(file_size >> 8);
    header[4] = (uint8_t)(file_size >> 16);
    header[5] = (uint8_t)(file_size >> 24);
    header[10] = 54;
    fwrite(header, 1, 14, f);

    uint8_t info[40] = {0};
    info[0] = 40;
    info[4] = (uint8_t)(width);
    info[5] = (uint8_t)(width >> 8);
    info[6] = (uint8_t)(width >> 16);
    info[7] = (uint8_t)(width >> 24);
    info[8] = (uint8_t)(height);
    info[9] = (uint8_t)(height >> 8);
    info[10] = (uint8_t)(height >> 16);
    info[11] = (uint8_t)(height >> 24);
    info[12] = 1;
    info[14] = 24;
    fwrite(info, 1, 40, f);

    std::vector<uint8_t> row(row_bytes, 0);
    for (int32_t y = (int32_t)height - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* p = rgba + ((uint32_t)y * width + x) * 4;
            row[x * 3 + 0] = p[2];
            row[x * 3 + 1] = p[1];
            row[x * 3 + 2] = p[0];
        }
        fwrite(row.data(), 1, row_bytes, f);
    }
    fclose(f);
    return true;
}

static std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static fs::path resolveHtmlPath(const char* html_file) {
    fs::path p(html_file);
    std::error_code ec;
    if (fs::exists(p, ec)) {
        return p;
    }
    fs::path c1 = fs::path("zig-out/bin") / p;
    if (fs::exists(c1, ec)) {
        return c1;
    }
    fs::path c2 = fs::path("zig-out/bin/data") / p;
    if (fs::exists(c2, ec)) {
        return c2;
    }
    return p;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: html_render_test <html_file> [output.bmp] [width] [height]\n");
        return 1;
    }

    const fs::path html_path = resolveHtmlPath(argv[1]);
    const std::string html_content = readFile(html_path);
    if (html_content.empty()) {
        fprintf(stderr, "ERROR: cannot read HTML: %s\n", html_path.string().c_str());
        return 1;
    }

    const char* output_file = argc > 2 ? argv[2] : "output.bmp";
    const uint32_t width = argc > 3 ? (uint32_t)atoi(argv[3]) : 800;
    const uint32_t height = argc > 4 ? (uint32_t)atoi(argv[4]) : 600;

    if (gpuPlatformInit() != GPU_SUCCESS) {
        fprintf(stderr, "ERROR: gpuPlatformInit failed\n");
        return 1;
    }

    GpuDevice device = nullptr;
    GpuDeviceDesc dev_desc{.appName = "html_render_test"};
    if (gpuCreateDevice(&dev_desc, &device) != GPU_SUCCESS) {
        fprintf(stderr, "ERROR: gpuCreateDevice failed\n");
        gpuPlatformShutdown();
        return 1;
    }

    GpuWindow window = nullptr;
    GpuWindowDesc win_desc = {.title = "html_render_test", .width = 1, .height = 1, .vsync = false};
    if (gpuCreateWindow(&win_desc, &window) != GPU_SUCCESS) {
        fprintf(stderr, "ERROR: gpuCreateWindow failed\n");
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    DongGPUDriver* driver = dong_gpu_backend_create_driver();
    if (!driver) {
        fprintf(stderr, "ERROR: dong_gpu_backend_create_driver failed\n");
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    dong_gpu_driver_set_external_device(driver, device, 0);
    dong_gpu_driver_set_embedded_mode(driver, 0);
    if (!dong_gpu_driver_initialize(driver)) {
        fprintf(stderr, "ERROR: dong_gpu_driver_initialize failed\n");
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    DongPlatform* platform = dong_platform_get();
    dong_platform_set_gpu_driver(platform, driver);

    const std::string resource_root = fs::absolute(html_path).parent_path().string();
    dong_gpu_set_resource_root(driver, resource_root.c_str());

    dong_engine_desc_t desc{};
    desc.api_version = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.width = width;
    desc.height = height;

    dong_engine_t* engine = nullptr;
    if (dong_engine_create(&desc, &engine) != DONG_OK || !engine) {
        fprintf(stderr, "ERROR: dong_engine_create failed\n");
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    if (!resource_root.empty()) {
        (void)dong_engine_set_resource_root(engine, resource_root.c_str());
    }
    if (dong_engine_set_gpu(engine, device, window) != DONG_OK) {
        fprintf(stderr, "ERROR: dong_engine_set_gpu failed\n");
        dong_engine_destroy(engine);
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }
    if (dong_engine_load_html(engine, html_content.c_str()) != DONG_OK) {
        fprintf(stderr, "ERROR: dong_engine_load_html failed\n");
        dong_engine_destroy(engine);
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    DongGPUTextureDesc tex_desc{};
    tex_desc.width = width;
    tex_desc.height = height;
    tex_desc.format = DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM;
    tex_desc.usage = DONG_GPU_TEXTURE_USAGE_COLOR_TARGET | DONG_GPU_TEXTURE_USAGE_SAMPLER |
                     DONG_GPU_TEXTURE_USAGE_TRANSFER_SRC;
    tex_desc.mip_levels = 1;
    tex_desc.debug_name = "html_render_test_rt";
    DongGPUTexture rt = dong_gpu_create_texture(driver, &tex_desc);
    if (!rt) {
        fprintf(stderr, "ERROR: create offscreen texture failed\n");
        dong_engine_destroy(engine);
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    if (!dong_gpu_begin_frame_offscreen(driver, rt, width, height)) {
        fprintf(stderr, "ERROR: begin_frame_offscreen failed\n");
        dong_gpu_destroy_texture(driver, rt);
        dong_engine_destroy(engine);
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    dong_gpu_prepare_resources(driver, nullptr);
    if (dong_engine_tick(engine) != DONG_OK) {
        fprintf(stderr, "WARN: dong_engine_tick returned error\n");
    }
    dong_gpu_end_frame_offscreen(driver);
    dong_gpu_wait_for_gpu(driver);

    std::vector<uint8_t> pixels((size_t)width * height * 4);
    if (dong_gpu_backend_readback_texture_rgba(driver, rt, width, height, pixels.data(), pixels.size()) != 0) {
        fprintf(stderr, "ERROR: texture readback failed\n");
        dong_gpu_destroy_texture(driver, rt);
        dong_engine_destroy(engine);
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    if (!writeBMP(output_file, width, height, pixels.data())) {
        fprintf(stderr, "ERROR: write BMP failed: %s\n", output_file);
        dong_gpu_destroy_texture(driver, rt);
        dong_engine_destroy(engine);
        dong_gpu_driver_shutdown(driver);
        dong_gpu_backend_destroy_driver(driver);
        gpuDestroyDevice(device);
        gpuPlatformShutdown();
        return 1;
    }

    fprintf(stderr, "[html_render_test] wrote %s (%ux%u)\n", output_file, width, height);

    dong_gpu_destroy_texture(driver, rt);
    dong_engine_destroy(engine);
    dong_platform_set_gpu_driver(platform, nullptr);
    dong_gpu_driver_shutdown(driver);
    dong_gpu_backend_destroy_driver(driver);
    gpuDestroyWindow(window);
    gpuDestroyDevice(device);
    gpuPlatformShutdown();
    return 0;
}
