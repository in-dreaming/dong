#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"

using dong::platform::SDL3Window;

namespace {

bool writeBMP(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data) {
    FILE* f = std::fopen(filename, "wb");
    if (!f) {
        return false;
    }

    const uint32_t pixel_data_bytes = width * height * 3u;
    const uint32_t file_size = 54u + pixel_data_bytes;

    uint8_t bmp_file_header[14] = {
        'B', 'M',
        0, 0, 0, 0,
        0, 0,
        0, 0,
        54, 0, 0, 0,
    };
    bmp_file_header[2] = static_cast<uint8_t>(file_size);
    bmp_file_header[3] = static_cast<uint8_t>(file_size >> 8);
    bmp_file_header[4] = static_cast<uint8_t>(file_size >> 16);
    bmp_file_header[5] = static_cast<uint8_t>(file_size >> 24);

    uint8_t bmp_info_header[40] = {
        40, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 0,
        24, 0,
    };
    bmp_info_header[4]  = static_cast<uint8_t>(width);
    bmp_info_header[5]  = static_cast<uint8_t>(width >> 8);
    bmp_info_header[6]  = static_cast<uint8_t>(width >> 16);
    bmp_info_header[7]  = static_cast<uint8_t>(width >> 24);
    bmp_info_header[8]  = static_cast<uint8_t>(height);
    bmp_info_header[9]  = static_cast<uint8_t>(height >> 8);
    bmp_info_header[10] = static_cast<uint8_t>(height >> 16);
    bmp_info_header[11] = static_cast<uint8_t>(height >> 24);

    std::fwrite(bmp_file_header, 1, sizeof(bmp_file_header), f);
    std::fwrite(bmp_info_header, 1, sizeof(bmp_info_header), f);

    for (int32_t y = static_cast<int32_t>(height) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint32_t src_index = (static_cast<uint32_t>(y) * width + x) * 4u;
            const uint8_t rgb[3] = {
                rgba_data[src_index + 2],
                rgba_data[src_index + 1],
                rgba_data[src_index + 0],
            };
            std::fwrite(rgb, 1, sizeof(rgb), f);
        }
    }

    std::fclose(f);
    return true;
}

void appendCString(std::string& out, const char* text) {
    out.append(text);
}

void appendHexEntity(std::string& out, uint32_t codepoint) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "&#x%04X;", static_cast<unsigned int>(codepoint));
    out.append(buffer);
}

std::string buildGlyphStressHtml() {
    std::string html;

    appendCString(html,
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "  <meta charset=\"utf-8\" />\n"
        "  <title>Glyph Atlas Stress Test</title>\n"
        "  <style>\n"
        "    body {\n"
        "      margin: 0;\n"
        "      padding: 16px;\n"
        "      background-color: #0b1120;\n"
        "      color: #e5e7eb;\n"
        "      font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Text', 'Segoe UI', sans-serif;\n"
        "    }\n"
        "    .page {\n"
        "      width: 1100px;\n"
        "      margin: 0 auto;\n"
        "    }\n"
        "    .title {\n"
        "      font-size: 20px;\n"
        "      font-weight: 600;\n"
        "      margin-bottom: 4px;\n"
        "    }\n"
        "    .subtitle {\n"
        "      font-size: 13px;\n"
        "      color: #9ca3af;\n"
        "      margin-bottom: 10px;\n"
        "    }\n"
        "    .section {\n"
        "      margin-bottom: 16px;\n"
        "      padding: 10px 12px;\n"
        "      border-radius: 10px;\n"
        "      background-color: #020617;\n"
        "      box-shadow: 0 0 0 1px rgba(148, 163, 184, 0.40);\n"
        "    }\n"
        "    .row {\n"
        "      font-size: 13px;\n"
        "      line-height: 1.6;\n"
        "      white-space: nowrap;\n"
        "    }\n"
        "    .row.alt {\n"
        "      font-size: 16px;\n"
        "    }\n"
        "    .row.tight {\n"
        "      line-height: 1.2;\n"
        "    }\n"
        "    .label {\n"
        "      display: inline-block;\n"
        "      min-width: 130px;\n"
        "      font-size: 11px;\n"
        "      text-transform: uppercase;\n"
        "      letter-spacing: 0.08em;\n"
        "      color: #64748b;\n"
        "    }\n"
        "    .latin {\n"
        "      font-family: 'SF Pro Text', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;\n"
        "    }\n"
        "    .mono {\n"
        "      font-family: Menlo, Consolas, monospace;\n"
        "    }\n"
        "    .cjk {\n"
        "      font-size: 18px;\n"
        "      line-height: 1.4;\n"
        "    }\n"
        "    .grid {\n"
        "      margin-top: 8px;\n"
        "      display: flex;\n"
        "      flex-direction: column;\n"
        "      gap: 2px;\n"
        "    }\n"
        "  </style>\n"
        "</head>\n"
        "<body>\n"
        "  <div class=\"page\">\n"
        "    <div class=\"title\">Glyph Atlas Stress Test</div>\n"
        "    <div class=\"subtitle\">大量不同字号和字符集的文本行，用于压测 GlyphAtlas 多页与淘汰</div>\n"
        "    <div class=\"section\">\n"
        "      <div class=\"row latin\"><span class=\"label\">Latin-1</span> The quick brown fox jumps over the lazy dog 0123456789 !@#$%^&*()</div>\n"
        "      <div class=\"row latin alt\"><span class=\"label\">Latin-2</span> Sphinx of black quartz, judge my vow. CAFÉ, naïve, façade, über, señor.</div>\n"
        "      <div class=\"row latin tight\"><span class=\"label\">Mixed</span> 这是一段中英文混排文本 ABC abc 123，用于 baseline 与 glyph 组合测试。</div>\n"
        "      <div class=\"row mono\"><span class=\"label\">Monospace</span> 0x0000 0x00FF 0x1F600  monospace sample line (baseline marker)</div>\n"
        "    </div>\n"
        "    <div class=\"section\">\n"
        "      <div class=\"row\"><span class=\"label\">CJK Grid</span> 从 U+4E00 开始生成大量 CJK 字符，每行 64 个，合计数千个，逼近或超过单个 GlyphAtlas 页容量，触发多页分配与 LRU 淘汰。</div>\n"
        "      <div class=\"grid cjk\">\n");

    const uint32_t start_codepoint = 0x4E00u;
    const uint32_t total_codepoints = 6144u;
    const uint32_t per_line = 64u;
    uint32_t current = start_codepoint;
    const uint32_t end_codepoint = start_codepoint + total_codepoints;

    while (current < end_codepoint) {
        appendCString(html, "        <div class=\"row\">");
        for (uint32_t i = 0; i < per_line && current < end_codepoint; ++i, ++current) {
            appendHexEntity(html, current);
        }
        appendCString(html, "</div>\n");
    }

    appendCString(html,
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "</body>\n"
        "</html>\n");

    return html;
}

} // namespace

int main() {
    std::printf("=== GPU Screenshot Demo - Glyph Atlas Stress Test ===\n");

    SDL3Window::CreateInfo create_info{};
    create_info.title = "GPU Screenshot Demo - Glyph Atlas Stress";
    create_info.width = 1200;
    create_info.height = 800;
    create_info.use_gpu = true;
    create_info.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(create_info)) {
        std::printf("ERROR: Failed to initialize SDL3Window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        std::printf("ERROR: GPU device is null\n");
        return 1;
    }

    dong_context_t* context = dong_create_context();
    if (!context) {
        std::printf("ERROR: Failed to create dong context\n");
        return 1;
    }

    dong_view_t* view = dong_view_create(context, create_info.width, create_info.height);
    if (!view) {
        std::printf("ERROR: Failed to create dong view\n");
        dong_destroy_context(context);
        return 1;
    }

    dong_view_set_external_gpu_device(view,
                                      static_cast<void*>(device),
                                      static_cast<void*>(window.getHandle()));

    std::string html = buildGlyphStressHtml();
    std::printf("[Load] HTML size: %zu bytes\n", static_cast<size_t>(html.size()));
    dong_view_load_html(view, html.c_str());

    std::printf("[Render] Updating view once...\n");
    dong_view_update(view);
    SDL_Delay(500);

    const uint32_t width = static_cast<uint32_t>(create_info.width);
    const uint32_t height = static_cast<uint32_t>(create_info.height);
    std::vector<uint8_t> pixels(width * height * 4u);

    std::printf("[Render] Rendering offscreen (%ux%u)...\n", width, height);
    if (!dong_view_render_offscreen(view,
                                    static_cast<void*>(device),
                                    width,
                                    height,
                                    pixels.data())) {
        std::printf("ERROR: dong_view_render_offscreen failed\n");
        dong_view_destroy(view);
        dong_destroy_context(context);
        return 1;
    }

    const char* output_path = "zig-out/tmp/gpu_screenshot_glyph_stress.bmp";
    if (writeBMP(output_path, width, height, pixels.data())) {
        std::printf("[Save] Saved glyph stress screenshot to %s\n", output_path);
    } else {
        std::printf("ERROR: Failed to save BMP to %s\n", output_path);
    }

    dong_view_destroy(view);
    dong_destroy_context(context);

    std::printf("[Done] Glyph stress demo finished.\n");
    return 0;
}
