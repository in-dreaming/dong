/**
 * HTML Render Test - 渲染 HTML 文件到贴图并保存为 BMP
 * 
 * 用法:
 *   html_render_test <html_file> [output.bmp] [width] [height]
 * 
 * 示例:
 *   html_render_test data/tests/transform_test.html output.bmp 800 600
 *   html_render_test data/tests/cursor_test.html
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
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"

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

void printUsage(const char* prog) {
    SDL_Log("Usage: %s <html_file> [output.bmp] [width] [height]", prog);
    SDL_Log("");
    SDL_Log("Arguments:");
    SDL_Log("  html_file   - Path to HTML file (relative to exe or absolute)");
    SDL_Log("  output.bmp  - Output BMP file (default: zig-out/tmp/render_test.bmp)");
    SDL_Log("  width       - Render width (default: 800)");
    SDL_Log("  height      - Render height (default: 600)");
    SDL_Log("");
    SDL_Log("Available test files in data/tests/:");
    
    auto files = listTestFiles("data/tests");
    if (files.empty()) {
        files = listTestFiles("zig-out/bin/data/tests");
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
    std::string output_file = (argc > 2) ? argv[2] : "zig-out/tmp/render_test.bmp";
    uint32_t width = (argc > 3) ? static_cast<uint32_t>(std::atoi(argv[3])) : 800;
    uint32_t height = (argc > 4) ? static_cast<uint32_t>(std::atoi(argv[4])) : 600;

    if (width == 0 || height == 0) {
        SDL_Log("ERROR: Invalid dimensions");
        return 1;
    }

    // 读取 HTML 文件
    std::string html_content = readFile(html_file);
    if (html_content.empty()) {
        // 尝试相对于 data 目录
        html_content = readFile("data/" + html_file);
    }
    if (html_content.empty()) {
        // 尝试 zig-out/bin/data
        html_content = readFile("zig-out/bin/data/" + html_file);
    }
    if (html_content.empty()) {
        SDL_Log("ERROR: Cannot read HTML file: %s", html_file.c_str());
        return 1;
    }

    SDL_Log("[Input]  HTML: %s (%zu bytes)", html_file.c_str(), html_content.size());
    SDL_Log("[Output] BMP:  %s (%ux%u)", output_file.c_str(), width, height);

    // 确保输出目录存在
    fs::path output_path(output_file);
    if (output_path.has_parent_path()) {
        fs::create_directories(output_path.parent_path());
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

    // 加载 HTML
    SDL_Log("[Load] Loading HTML...");
    dong_view_load_html(view, html_content.c_str());
    SDL_Log("[Load] HTML loaded successfully");

    // 渲染到像素缓冲区
    std::vector<uint8_t> pixels(width * height * 4);
    SDL_Log("[Render] Rendering offscreen...");
    
    if (!dong_view_render_offscreen(view, static_cast<void*>(device), width, height, pixels.data())) {
        SDL_Log("ERROR: dong_view_render_offscreen failed");
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    // 保存 BMP
    if (writeBMP(output_file.c_str(), width, height, pixels.data())) {
        SDL_Log("[Save] Saved to %s", output_file.c_str());
    } else {
        SDL_Log("ERROR: Failed to save BMP");
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    // 像素统计
    int total = static_cast<int>(width * height);
    int black = 0, colored = 0, white = 0;
    for (int i = 0; i < total; ++i) {
        int idx = i * 4;
        int brightness = (pixels[idx] + pixels[idx + 1] + pixels[idx + 2]) / 3;
        if (brightness < 50) black++;
        else if (brightness > 200) white++;
        else colored++;
    }
    SDL_Log("[Stats] Pixels: black=%d(%.1f%%) colored=%d(%.1f%%) white=%d(%.1f%%)",
            black, 100.0*black/total, colored, 100.0*colored/total, white, 100.0*white/total);

    // 清理
    SDL_Log("[Cleanup] Shutting down...");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    SDL_Log("[Done] Output: %s", output_file.c_str());
    return 0;
}
