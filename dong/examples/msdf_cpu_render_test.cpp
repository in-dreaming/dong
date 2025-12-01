#include <cstdio>
#include <cstdint>
#include <vector>
#include <dong.h>

// 简单的 BMP 写入函数
bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    
    uint32_t filesize = 54 + width * height * 3;
    uint8_t bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize>>8);
    bmpfileheader[4] = (uint8_t)(filesize>>16);
    bmpfileheader[5] = (uint8_t)(filesize>>24);
    
    uint8_t bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    bmpinfoheader[4] = (uint8_t)(width);
    bmpinfoheader[5] = (uint8_t)(width>>8);
    bmpinfoheader[6] = (uint8_t)(width>>16);
    bmpinfoheader[7] = (uint8_t)(width>>24);
    bmpinfoheader[8] = (uint8_t)(height);
    bmpinfoheader[9] = (uint8_t)(height>>8);
    bmpinfoheader[10] = (uint8_t)(height>>16);
    bmpinfoheader[11] = (uint8_t)(height>>24);
    
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);
    
    for (int y = height - 1; y >= 0; y--) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t idx = (y * width + x) * 4;
            uint8_t rgb[3] = {rgba_data[idx + 2], rgba_data[idx + 1], rgba_data[idx + 0]};
            fwrite(rgb, 3, 1, f);
        }
    }
    
    fclose(f);
    return true;
}

int main() {
    std::printf("=== MSDF CPU Render Test - Analyze Pixels ===\n");

    // 1. 创建上下文
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create context\n");
        return 1;
    }

    // 2. 创建视图（CPU 渲染 - 默认模式）
    const uint32_t width = 800;
    const uint32_t height = 400;
    std::printf("[Init] Creating view %ux%u...\n", width, height);
    dong_view_t* view = dong_view_create(ctx, width, height);
    if (!view) {
        std::printf("ERROR: Failed to create view\n");
        dong_destroy_context(ctx);
        return 1;
    }

    // 3. 加载测试 HTML - 大字号以便观察
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    margin: 0;
                    padding: 20px;
                    background-color: #ffffff;
                    font-family: Arial, sans-serif;
                }
                
                .test-line {
                    margin: 15px 0;
                    padding: 10px;
                    background: #f0f0f0;
                }
                
                .size-72 { font-size: 72px; color: #000000; }
                .size-48 { font-size: 48px; color: #ff0000; }
                .size-36 { font-size: 36px; color: #0000ff; }
            </style>
        </head>
        <body>
            <div class="test-line size-72">ABC</div>
            <div class="test-line size-48">Test</div>
            <div class="test-line size-36">Hello</div>
        </body>
        </html>
    )";

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    // 4. 渲染（CPU 模式）
    std::printf("[Render] Rendering with CPU backend...\n");
    dong_view_update(view);

    // 5. 获取像素缓冲区
    std::printf("[Read] Getting pixel buffer...\n");
    void* buffer = dong_view_get_pixel_buffer(view);
    if (!buffer) {
        std::printf("ERROR: Failed to get pixel buffer (CPU render may be disabled)\n");
        std::printf("       Check view.cpp:189-192, CPU rendering path is commented out!\n");
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    uint8_t* pixels = (uint8_t*)buffer;

    // 6. 保存图片
    const char* output_file = "msdf_cpu_test.bmp";
    if (writeBMP(output_file, width, height, pixels)) {
        std::printf("[Save] Saved to %s\n", output_file);
    } else {
        std::printf("ERROR: Failed to save BMP\n");
    }

    // 7. 详细分析像素 - 这是关键！
    std::printf("\n=== COMPREHENSIVE PIXEL ANALYSIS ===\n");

    // 7.1 全局统计
    int total_pixels = width * height;
    int total_black = 0, total_white = 0, total_gray = 0;
    int total_red = 0, total_blue = 0;
    
    for (uint32_t i = 0; i < total_pixels; i++) {
        int idx = i * 4;
        uint8_t r = pixels[idx];
        uint8_t g = pixels[idx + 1];
        uint8_t b = pixels[idx + 2];
        
        int brightness = (r + g + b) / 3;
        
        if (brightness < 50) total_black++;
        else if (brightness > 200) total_white++;
        else total_gray++;
        
        // 检测彩色文字
        if (r > 200 && g < 100 && b < 100) total_red++;
        if (b > 200 && r < 100 && g < 100) total_blue++;
    }
    
    std::printf("Total pixels: %d\n", total_pixels);
    std::printf("  Black (<50): %d (%.1f%%)\n", total_black, 100.0*total_black/total_pixels);
    std::printf("  Gray (50-200): %d (%.1f%%)\n", total_gray, 100.0*total_gray/total_pixels);
    std::printf("  White (>200): %d (%.1f%%)\n", total_white, 100.0*total_white/total_pixels);
    std::printf("  Red pixels: %d (%.1f%%)\n", total_red, 100.0*total_red/total_pixels);
    std::printf("  Blue pixels: %d (%.1f%%)\n", total_blue, 100.0*total_blue/total_pixels);

    // 7.2 判断渲染状态
    std::printf("\n=== RENDERING STATUS ===\n");
    
    if (total_black == total_pixels) {
        std::printf("❌ FAILED: Image is completely black! No rendering occurred.\n");
        std::printf("   Possible causes:\n");
        std::printf("   - CPU render path is disabled in view.cpp\n");
        std::printf("   - Painter is not initialized\n");
        std::printf("   - Display list is empty\n");
    } else if (total_white == total_pixels) {
        std::printf("❌ FAILED: Image is completely white! Background only, no text.\n");
        std::printf("   Possible causes:\n");
        std::printf("   - Text layout failed\n");
        std::printf("   - Font loading failed\n");
        std::printf("   - Text color same as background\n");
    } else if (total_black < 100 && total_gray < 100) {
        std::printf("⚠️  WARNING: Very few dark pixels! Text may not be rendering.\n");
        std::printf("   Black: %d, Gray: %d\n", total_black, total_gray);
    } else {
        std::printf("✅ SUCCESS: Image contains text content!\n");
        std::printf("   - Found %d black pixels (text)\n", total_black);
        std::printf("   - Found %d gray pixels (anti-aliasing)\n", total_gray);
        
        if (total_red > 0) {
            std::printf("   - Found %d red pixels (colored text line 2)\n", total_red);
        }
        if (total_blue > 0) {
            std::printf("   - Found %d blue pixels (colored text line 3)\n", total_blue);
        }
    }

    // 7.3 区域采样 - 检查预期文字位置
    std::printf("\n=== REGIONAL SAMPLING ===\n");
    
    struct Region {
        const char* name;
        int x, y, w, h;
        int expected_black_percent;  // 预期黑色像素百分比
    };
    
    Region regions[] = {
        {"Line 1 (72px ABC)", 50, 30, 300, 100, 15},   // 大字应该有较多黑色像素
        {"Line 2 (48px Test)", 50, 140, 250, 70, 10},
        {"Line 3 (36px Hello)", 50, 230, 200, 60, 8},
        {"Background area", 700, 350, 90, 40, 0},      // 纯背景区域
    };
    
    for (const auto& region : regions) {
        int region_pixels = region.w * region.h;
        int region_black = 0;
        int region_colored = 0;
        
        for (int y = region.y; y < region.y + region.h && y < (int)height; y++) {
            for (int x = region.x; x < region.x + region.w && x < (int)width; x++) {
                int idx = (y * width + x) * 4;
                uint8_t r = pixels[idx];
                uint8_t g = pixels[idx + 1];
                uint8_t b = pixels[idx + 2];
                
                int brightness = (r + g + b) / 3;
                if (brightness < 50) region_black++;
                
                // 检测彩色
                if ((r > 150 && g < 100) || (b > 150 && r < 100)) {
                    region_colored++;
                }
            }
        }
        
        float black_percent = 100.0f * region_black / region_pixels;
        std::printf("%s:\n", region.name);
        std::printf("  Black pixels: %d / %d (%.1f%%)  [Expected: ~%d%%]\n",
                   region_black, region_pixels, black_percent, region.expected_black_percent);
        
        if (region_colored > 0) {
            std::printf("  Colored pixels: %d (%.1f%%)\n",
                       region_colored, 100.0f * region_colored / region_pixels);
        }
        
        // 判断该区域是否符合预期
        if (region.expected_black_percent > 0) {
            if (black_percent < region.expected_black_percent / 2) {
                std::printf("  ⚠️  WARNING: Too few black pixels! Text may not be rendering correctly.\n");
            } else if (black_percent > region.expected_black_percent * 2) {
                std::printf("  ⚠️  WARNING: Too many black pixels! May have rendering artifacts.\n");
            } else {
                std::printf("  ✅ OK: Black pixel ratio is reasonable.\n");
            }
        } else {
            if (black_percent > 5) {
                std::printf("  ⚠️  WARNING: Unexpected dark pixels in background area!\n");
            } else {
                std::printf("  ✅ OK: Background is clean.\n");
            }
        }
    }

    // 7.4 精确点采样
    std::printf("\n=== PRECISE POINT SAMPLING ===\n");
    
    struct SamplePoint {
        int x, y;
        const char* desc;
        bool should_be_text;  // 该点是否应该是文字（黑色/灰色）
    };
    
    SamplePoint samples[] = {
        {100, 70, "Inside 'A' (line 1)", true},
        {150, 70, "Inside 'B' (line 1)", true},
        {200, 70, "Inside 'C' (line 1)", true},
        {50, 30, "Top-left corner (background)", false},
        {100, 175, "Inside 'T' (line 2)", true},
        {150, 265, "Inside 'H' (line 3)", true},
        {750, 380, "Bottom-right corner (background)", false},
    };
    
    for (const auto& sample : samples) {
        if (sample.x >= (int)width || sample.y >= (int)height) continue;
        
        int idx = (sample.y * width + sample.x) * 4;
        uint8_t r = pixels[idx];
        uint8_t g = pixels[idx + 1];
        uint8_t b = pixels[idx + 2];
        int brightness = (r + g + b) / 3;
        
        std::printf("(%d, %d) - %s:\n", sample.x, sample.y, sample.desc);
        std::printf("  RGBA(%d, %d, %d, 255)  Brightness: %d\n", r, g, b, brightness);
        
        bool is_dark = (brightness < 128);
        if (sample.should_be_text) {
            if (is_dark) {
                std::printf("  ✅ OK: Dark pixel as expected (text rendered)\n");
            } else {
                std::printf("  ❌ FAILED: Should be text but pixel is too bright!\n");
            }
        } else {
            if (!is_dark) {
                std::printf("  ✅ OK: Bright pixel as expected (background)\n");
            } else {
                std::printf("  ⚠️  WARNING: Should be background but pixel is dark!\n");
            }
        }
    }

    // 8. 最终结论
    std::printf("\n=== FINAL VERDICT ===\n");
    
    int text_pixels = total_black + total_gray;
    float text_ratio = 100.0f * text_pixels / total_pixels;
    
    if (text_ratio < 1.0f) {
        std::printf("❌ MSDF RENDERING FAILED\n");
        std::printf("   Only %.2f%% pixels are text-related (expected >5%%)\n", text_ratio);
        std::printf("   MSDF text rendering is not working correctly!\n");
    } else if (text_ratio < 5.0f) {
        std::printf("⚠️  MSDF RENDERING DEGRADED\n");
        std::printf("   %.2f%% pixels are text-related (expected >5%%)\n", text_ratio);
        std::printf("   Text may be rendering but with issues.\n");
    } else {
        std::printf("✅ MSDF RENDERING WORKING\n");
        std::printf("   %.2f%% pixels are text-related\n", text_ratio);
        std::printf("   MSDF text is rendering successfully!\n");
    }

    // 9. 清理
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    std::printf("\n[Done] Check %s for visual verification\n", output_file);
    std::printf("       All pixel analysis completed above.\n");
    
    return 0;
}
