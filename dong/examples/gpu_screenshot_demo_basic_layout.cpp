#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstring>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"

using dong::platform::SDL3Window;

// 简单的 BMP 写入函数（与 gpu_screenshot_demo 保持一致）
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
    std::printf("=== GPU Screenshot Demo - Basic Layout / Text / Colors ===\n");

    SDL3Window::CreateInfo ci{};
    ci.title = "GPU Screenshot Demo - Basic Layout";
    ci.width = 900;
    ci.height = 520;
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

    std::printf("[Init] Enabling GPU render mode...\n");
    dong_view_set_external_gpu_device(view,
                                      static_cast<void*>(device),
                                      static_cast<void*>(window.getHandle()));

    // 基础布局 + 文本 + 颜色 对齐场景
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="utf-8" />
            <style>
                body {
                    margin: 0;
                    padding: 16px;
                    background-color: #f5f5f5;
                    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
                    color: #111827;
                }

                .page {
                    width: 760px;
                    margin: 0 auto;
                }

                .section-title {
                    font-size: 13px;
                    letter-spacing: 0.08em;
                    text-transform: uppercase;
                    color: #6b7280;
                    margin: 8px 0 6px;
                }

                .section-subtitle {
                    font-size: 12px;
                    color: #9ca3af;
                    margin: 0 0 6px;
                }

                /* 1. 布局几何：block / inline / inline-block / relative / absolute */
                .card {
                    margin-top: 8px;
                    padding: 12px 14px;
                    border-radius: 12px;
                    box-sizing: border-box;
                    background-color: #ffffff;
                    box-shadow: 0 0 0 1px rgba(15, 23, 42, 0.04);
                }

                .block-stack {
                    margin-bottom: 8px;
                }

                .block-box {
                    display: block;
                    box-sizing: border-box;
                    margin-bottom: 6px;
                    padding: 8px 10px;
                    border: 1px solid #e5e7eb;
                    border-radius: 6px;
                    background-color: #eff6ff;
                    color: #1d4ed8;
                    font-size: 13px;
                }

                .block-box.primary {
                    margin-left: 24px;
                    padding-left: 18px;
                    background-color: #dbeafe;
                    border-color: #bfdbfe;
                    color: #1d4ed8;
                }

                .inline-container {
                    margin-top: 4px;
                    font-size: 14px;
                    line-height: 1.6;
                    background-color: #f9fafb;
                    border-radius: 8px;
                    padding: 6px 10px;
                }

                .inline-label {
                    font-weight: 600;
                    color: #111827;
                }

                .inline-chip {
                    display: inline-block;
                    margin-left: 4px;
                    margin-right: 4px;
                    padding: 2px 6px;
                    border-radius: 999px;
                    background-color: #fee2e2;
                    color: #b91c1c;
                    font-size: 13px;
                    vertical-align: baseline;
                }

                .inline-chip.alt {
                    background-color: #e0f2fe;
                    color: #0369a1;
                }

                .inline-chip.small {
                    font-size: 11px;
                    padding: 1px 5px;
                }

                .inline-block-row {
                    margin-top: 6px;
                }

                .inline-block-box {
                    display: inline-block;
                    width: 72px;
                    height: 40px;
                    margin-right: 6px;
                    border-radius: 8px;
                    box-sizing: border-box;
                    padding: 6px 8px;
                    border: 1px solid #e5e7eb;
                    background-color: #ecfeff;
                    color: #0e7490;
                    font-size: 12px;
                    text-align: center;
                }

                .rel-abs-container {
                    margin-top: 8px;
                    display: flex;
                    gap: 12px;
                }

                .rel-box {
                    position: relative;
                    width: 200px;
                    height: 70px;
                    padding: 10px 12px;
                    box-sizing: border-box;
                    border-radius: 10px;
                    background-color: #fefce8;
                    border: 1px solid #facc15;
                    font-size: 13px;
                    color: #92400e;
                }

                .rel-box .abs-badge {
                    position: absolute;
                    right: -6px;
                    top: -6px;
                    padding: 2px 6px;
                    border-radius: 999px;
                    background-color: #f97316;
                    color: #ffffff;
                    font-size: 11px;
                    border: 1px solid rgba(248, 250, 252, 0.9);
                }

                .rel-box .baseline-marker {
                    position: absolute;
                    left: 10px;
                    bottom: 12px;
                    width: 80px;
                    height: 1px;
                    background-color: rgba(148, 163, 184, 0.9);
                }

                /* 2. 文本排版 / 字形 */
                .typography-card {
                    margin-top: 8px;
                    padding: 12px 14px 10px;
                    border-radius: 12px;
                    box-sizing: border-box;
                    background-color: #ffffff;
                    box-shadow: 0 0 0 1px rgba(15, 23, 42, 0.04);
                }

                .typo-row {
                    margin: 4px 0;
                }

                .typo-large {
                    font-size: 28px;
                    line-height: 1.25;
                    font-weight: 700;
                    letter-spacing: 0.02em;
                    color: #111827;
                }

                .typo-body {
                    font-size: 14px;
                    line-height: 1.7;
                    color: #374151;
                }

                .typo-body.tight {
                    line-height: 1.35;
                }

                .typo-caption {
                    font-size: 12px;
                    line-height: 1.5;
                    color: #6b7280;
                    letter-spacing: 0.06em;
                    text-transform: uppercase;
                }

                .typo-mono {
                    font-family: Menlo, Consolas, monospace;
                    font-size: 12px;
                    background-color: #f3f4f6;
                    border-radius: 6px;
                    padding: 4px 6px;
                    display: inline-block;
                    margin-top: 2px;
                    color: #111827;
                }

                .typo-compare-line {
                    margin-top: 6px;
                    padding: 4px 0;
                    border-top: 1px dashed rgba(209, 213, 219, 0.9);
                }

                .typo-compare-span {
                    display: inline-block;
                    margin-right: 8px;
                }

                .typo-compare-span.small {
                    font-size: 13px;
                }

                .typo-compare-span.big {
                    font-size: 18px;
                    font-weight: 600;
                }

                .typo-compare-span.spaced {
                    letter-spacing: 0.08em;
                }

                /* 3. 颜色 / 背景 / 边框 */
                .color-card {
                    margin-top: 8px;
                    padding: 12px 14px 10px;
                    border-radius: 12px;
                    box-sizing: border-box;
                    background-color: #ffffff;
                    box-shadow: 0 0 0 1px rgba(15, 23, 42, 0.04);
                }

                .color-row {
                    display: flex;
                    gap: 10px;
                    margin-top: 6px;
                }

                .color-swatch {
                    flex: 1;
                    min-width: 0;
                    padding: 8px 10px;
                    border-radius: 10px;
                    box-sizing: border-box;
                    font-size: 12px;
                }

                .color-swatch.primary {
                    background-color: #1d4ed8;
                    color: #eff6ff;
                }

                .color-swatch.muted {
                    background-color: #f3f4f6;
                    color: #4b5563;
                    border: 1px dashed #d1d5db;
                }

                .color-swatch.soft {
                    background-color: rgba(16, 185, 129, 0.08);
                    color: #047857;
                    border: 1px solid rgba(16, 185, 129, 0.4);
                }

                .color-swatch.shadow {
                    background-color: #ffffff;
                    color: #111827;
                    border-radius: 12px;
                    box-shadow:
                        0 12px 24px rgba(15, 23, 42, 0.16),
                        0 0 0 1px rgba(148, 163, 184, 0.40);
                }

                .color-label {
                    font-size: 11px;
                    text-transform: uppercase;
                    letter-spacing: 0.08em;
                    color: rgba(249, 250, 251, 0.9);
                }

                .color-code {
                    font-family: Menlo, Consolas, monospace;
                    font-size: 11px;
                    margin-top: 2px;
                    opacity: 0.9;
                }

                .color-caption {
                    font-size: 11px;
                    color: #4b5563;
                    margin-top: 2px;
                }
            </style>
        </head>
        <body>
            <div class="page">
                <!-- 1. 布局几何 -->
                <div class="section-title">1. Layout geometry</div>
                <div class="section-subtitle">block / inline / inline-block / relative / absolute</div>
                <div class="card">
                    <div class="block-stack">
                        <div class="block-box">Block A - margin / border / padding 基本盒模型</div>
                        <div class="block-box primary">Block B - 左侧增加 margin-left + padding-left</div>
                    </div>

                    <div class="inline-container">
                        <span class="inline-label">Inline text:</span>
                        <span>前后各有不同大小的 inline 元素，检查 baseline。</span>
                        <span class="inline-chip">chip-1</span>
                        <span class="inline-chip alt">chip-2</span>
                        <span class="inline-chip small">chip-3</span>
                    </div>

                    <div class="inline-block-row">
                        <div class="inline-block-box">IB-1</div>
                        <div class="inline-block-box">IB-2</div>
                        <div class="inline-block-box">IB-3</div>
                    </div>

                    <div class="rel-abs-container">
                        <div class="rel-box">
                            <div>position: relative; 内部角标 absolute。</div>
                            <div class="baseline-marker"></div>
                            <div class="abs-badge">ABS</div>
                        </div>
                    </div>
                </div>

                <!-- 2. 文本排版 / 字形 -->
                <div class="section-title">2. Typography</div>
                <div class="section-subtitle">font family / size / weight / line-height / letter-spacing</div>
                <div class="typography-card">
                    <div class="typo-row typo-caption">SECTION TITLE · BASELINE CHECK</div>
                    <div class="typo-row typo-large">The quick brown fox jumps over the lazy dog</div>
                    <div class="typo-row typo-body">
                        这是一段用于测试段落排版、行高和中英文混排的示例文本。The quick brown fox
                        jumps over the lazy dog 1234567890 ABC xyz。
                    </div>
                    <div class="typo-row typo-body tight">
                        行高较紧的第二段文本，用于对比 baseline 间距是否和 HTML 接近。
                    </div>
                    <div class="typo-row">
                        <span class="typo-mono">font-family: Menlo; font-size: 12px;</span>
                    </div>
                    <div class="typo-compare-line">
                        <span class="typo-compare-span small">Small text</span>
                        <span class="typo-compare-span big">BIG TEXT</span>
                        <span class="typo-compare-span spaced">SPACED LETTERS</span>
                    </div>
                </div>

                <!-- 3. 颜色 / 背景 / 边框 -->
                <div class="section-title">3. Colors & borders</div>
                <div class="section-subtitle">foreground / background / radius / border / shadow</div>
                <div class="color-card">
                    <div class="color-row">
                        <div class="color-swatch primary">
                            <div class="color-label">Primary</div>
                            <div class="color-code">#1D4ED8 on #EFF6FF</div>
                            <div class="color-caption">用于对齐前景/背景颜色和对比度。</div>
                        </div>
                        <div class="color-swatch muted">
                            <div class="color-label" style="color:#6b7280;">Muted</div>
                            <div class="color-code" style="color:#4b5563;">#F3F4F6 with dashed border</div>
                            <div class="color-caption">检查边框样式与圆角。</div>
                        </div>
                    </div>
                    <div class="color-row">
                        <div class="color-swatch soft">
                            <div class="color-label" style="color:#047857;">Soft alpha</div>
                            <div class="color-code">rgba(16,185,129,0.08)</div>
                            <div class="color-caption">用于测试透明背景叠加。</div>
                        </div>
                        <div class="color-swatch shadow">
                            <div class="color-label" style="color:#4b5563;">Shadow card</div>
                            <div class="color-code" style="color:#111827;">box-shadow + border</div>
                            <div class="color-caption">用于观察阴影形状和模糊半径。</div>
                        </div>
                    </div>
                </div>
            </div>
        </body>
        </html>
    )";

    std::printf("[Load] Loading HTML...\n");
    dong_view_load_html(view, html);

    std::printf("[Render] Letting view update once...\n");
    dong_view_update(view);
    SDL_Delay(200);

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

    const char* output_file = "zig-out/tmp/gpu_screenshot_basic_layout.bmp";
    if (writeBMP(output_file, width, height, pixels.data())) {
        std::printf("[Save] Saved to %s\n", output_file);
    } else {
        std::printf("ERROR: Failed to save BMP\n");
    }

    std::printf("\n[Cleanup] Shutting down...\n");
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    std::printf("[Done] Check %s for the captured layout/text/color reference.\n", output_file);
    return 0;
}
