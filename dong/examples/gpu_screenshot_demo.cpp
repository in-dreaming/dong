#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"

using dong::platform::SDL3Window;

// 简单的 BMP 写入函数（与 gpu_screenshot_analysis 保持一致）
bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;

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
    return true;
}

int main() {
    std::printf("=== GPU Screenshot Demo - MSDF Offscreen ===\n");

    // 1. 创建带 GPU 的 SDL 窗口（复用现有 SDL3Window 封装）
    SDL3Window::CreateInfo ci{};
    ci.title = "GPU Screenshot Demo";
    ci.width = 800;
    ci.height = 400;
    ci.use_gpu = true;
    ci.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(ci)) {
        std::printf("ERROR: Failed to initialize SDL3Window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        std::printf("ERROR: GPU device is null\n");
        return 1;
    }

    // 2. 创建 Dong 上下文 + 视图
    std::printf("[Init] Creating dong context...\n");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create dong context\n");
        return 1;
    }

    std::printf("[Init] Creating dong view...\n");
    dong_view_t* view = dong_view_create(ctx, ci.width, ci.height);
    if (!view) {
        std::printf("ERROR: Failed to create dong view\n");
        dong_destroy_context(ctx);
        return 1;
    }

    // 3. 切换到 GPU 渲染模式
    std::printf("[Init] Enabling GPU render mode...\n");
    dong_view_set_external_gpu_device(view,
                                      static_cast<void*>(device),
                                      static_cast<void*>(window.getHandle()));

    // 4. 加载测试 HTML - 大字号 MSDF 文字（与原 demo 一致）
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    margin: 0;
                    padding: 16px;
                    background-color: #ffffff;
                    font-family: Arial, sans-serif;
                }

                .section-title {
                    font-size: 14px;
                    letter-spacing: 0.08em;
                    text-transform: uppercase;
                    color: #666666;
                    margin: 4px 0 6px;
                }

                .test-line {
                    margin: 6px 0;
                    padding: 6px 10px;
                    background: #f0f0f0;
                    border-radius: 4px;
                }

                .size-72 { font-size: 72px; color: #000000; }
                .size-48 { font-size: 48px; color: #ff0000; }
                .size-36 { font-size: 36px; color: #0000ff; }

                /* 内嵌滚动容器 + 圆角裁剪 */
                .scroll-card {
                    position: relative;
                    margin-top: 8px;
                    width: 360px;
                    height: 220px;
                    padding: 12px;
                    box-sizing: border-box;
                    border-radius: 16px;
                    background: #fafafa;
                    box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.04);
                }

                .scroll-card-header {
                    font-size: 13px;
                    font-weight: bold;
                    margin-bottom: 6px;
                    color: #333333;
                }

                .scroll-card-body {
                    position: relative;
                    margin-top: 4px;
                    border-radius: 12px;
                    overflow: auto;
                    height: 160px;
                    padding: 8px 10px;
                    box-sizing: border-box;
                    background: #ffffff;
                }

                .scroll-row {
                    font-size: 13px;
                    line-height: 1.6;
                    color: #222222;
                    white-space: nowrap;
                }

                .scroll-row strong {
                    font-weight: bold;
                    margin-right: 4px;
                }

                /* 弹窗 / Drawer overlay，使用 transform + opacity */
                .overlay-root {
                    position: absolute;
                    right: 16px;
                    top: 24px;
                    width: 340px;
                    height: 260px;
                    pointer-events: none;
                }

                .overlay-backdrop {
                    position: absolute;
                    inset: 0;
                    border-radius: 18px;
                    background: rgba(0, 0, 0, 0.04);
                    opacity: 0.9;
                }

                .overlay-modal {
                    position: absolute;
                    left: 18px;
                    top: 24px;
                    width: 304px;
                    height: 210px;
                    padding: 12px 14px;
                    box-sizing: border-box;
                    border-radius: 14px;
                    background: #ffffff;
                    box-shadow:
                        0 18px 45px rgba(15, 23, 42, 0.18),
                        0 0 0 1px rgba(148, 163, 184, 0.18);

                    /* 关键：让这一块作为独立 Layer，走 transform+opacity 合成 */
                    isolation: isolate;
                    opacity: 0.92;
                    transform: translateY(6px) scale(0.98);
                }

                .overlay-title {
                    font-size: 15px;
                    font-weight: bold;
                    margin-bottom: 4px;
                    color: #111827;
                }

                .overlay-subtitle {
                    font-size: 12px;
                    margin-bottom: 8px;
                    color: #6b7280;
                }

                .overlay-row {
                    font-size: 13px;
                    line-height: 1.5;
                    margin: 2px 0;
                    color: #111827;
                }

                .overlay-row code {
                    font-family: Menlo, Consolas, monospace;
                    background: #f3f4f6;
                    border-radius: 4px;
                    padding: 1px 3px;
                }
            </style>
        </head>
        <body>
            <!-- 区块 1：原始 MSDF 大字号基线测试 -->
            <div class="section-title">MSDF baseline</div>
            <div class="test-line size-72">ABC</div>
            <div class="test-line size-48">Test</div>
            <div class="test-line size-36">Hello</div>

            <!-- 区块 2：内嵌滚动容器 + 圆角裁剪，用于 Scroll + Clip + MSDF 组合测试 -->
            <div class="section-title">Scroll + rounded clip</div>
            <div class="scroll-card">
                <div class="scroll-card-header">Chat window (scrollable, clipped)</div>
                <div class="scroll-card-body">
                    <div class="scroll-row"><strong>Alice</strong> 这是一条用于测试 MSDF 渲染基线和滚动裁剪的示例消息。</div>
                    <div class="scroll-row"><strong>Bob</strong> 第二行文字，检查行高、字距以及圆角裁剪边缘。</div>
                    <div class="scroll-row"><strong>Alice</strong> 第三行，稍微长一点的文本以观察段落换行行为。</div>
                    <div class="scroll-row"><strong>Bob</strong> 第四行，再来一点英文 ABC xyz 123 mixed content.</div>
                    <div class="scroll-row"><strong>Alice</strong> 第五行，确保滚动区域内部有足够内容可以裁剪。</div>
                    <div class="scroll-row"><strong>Bob</strong> 第六行，继续堆叠文本，方便截图时观察 MSDF 边缘。</div>
                    <div class="scroll-row"><strong>Alice</strong> 第七行，最后一条测试消息。</div>
                </div>
            </div>

            <!-- 区块 3：Overlay 弹窗，使用 transform + opacity 走 LayerTree + 缓存 -->
            <div class="overlay-root">
                <div class="overlay-backdrop"></div>
                <div class="overlay-modal">
                    <div class="overlay-title">LayerTree &amp; Cache Preview</div>
                    <div class="overlay-subtitle">Overlay with transform + opacity</div>
                    <div class="overlay-row">此弹窗使用 <code>isolation: isolate</code> 形成独立图层。</div>
                    <div class="overlay-row">动画阶段只需调整 <code>transform</code> 和 <code>opacity</code>，</div>
                    <div class="overlay-row">内容不变时可以通过 Layer 缓存避免重栅格。</div>
                </div>
            </div>
        </body>
        </html>
    )";

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    // 5. 给 Dong 一点时间做首次布局/渲染（可选）
    std::printf("[Render] Letting view update once...\n");
    dong_view_update(view);
    SDL_Delay(200);

    // 6. 使用 dong_view_render_offscreen 渲染到 RGBA 缓冲
    const uint32_t width = ci.width;
    const uint32_t height = ci.height;
    std::vector<uint8_t> pixels(width * height * 4);

    std::printf("[Render] Rendering offscreen via dong_view_render_offscreen(%ux%u)...\n",
                width, height);
    if (!dong_view_render_offscreen(view,
                                    static_cast<void*>(device),
                                    width,
                                    height,
                                    pixels.data())) {
        std::printf("ERROR: dong_view_render_offscreen failed\n");

        std::printf("[Cleanup] Shutting down...\n");
        dong_view_destroy(view);
        dong_destroy_context(ctx);
        return 1;
    }

    // 7. 保存 BMP
    const char* output_file = "gpu_screenshot.bmp";
    if (writeBMP(output_file, width, height, pixels.data())) {
        std::printf("[Save] Saved to %s\n", output_file);
    } else {
        std::printf("ERROR: Failed to save BMP\n");
    }

    // 8. 简单像素统计（参考 gpu_screenshot_analysis 的风格）
    std::printf("\n=== PIXEL ANALYSIS ===\n");
    int total_pixels = static_cast<int>(width * height);
    int total_black = 0, total_white = 0, total_gray = 0;

    for (int i = 0; i < total_pixels; ++i) {
        int idx = i * 4;
        int brightness = (pixels[idx] + pixels[idx + 1] + pixels[idx + 2]) / 3;
        if (brightness < 50) total_black++;
        else if (brightness > 200) total_white++;
        else total_gray++;
    }

    std::printf("Total pixels: %d\n", total_pixels);
    std::printf("  Black (<50): %d (%.1f%%)\n", total_black, 100.0 * total_black / total_pixels);
    std::printf("  Gray (50-200): %d (%.1f%%)\n", total_gray, 100.0 * total_gray / total_pixels);
    std::printf("  White (>200): %d (%.1f%%)\n", total_white, 100.0 * total_white / total_pixels);

    if (total_white == total_pixels) {
        std::printf("\n⚠️  WARNING: Image is all white! Offscreen rendering produced blank output.\n");
    } else if (total_black > 100 || total_gray > 100) {
        std::printf("\n✅ SUCCESS: Text pixels detected in offscreen screenshot!\n");
    }

    // 9. 清理
    std::printf("\n[Cleanup] Shutting down...\n");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    std::printf("[Done] Check %s for the captured MSDF text.\n", output_file);
    return 0;
}
