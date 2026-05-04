/**
 * Dong Engine HDR Demo
 *
 * Demonstrates HDR (High Dynamic Range) rendering support with:
 * - Runtime HDR detection and configuration
 * - HDR vs SDR comparison
 * - Interactive luminance demonstration
 *
 * HDR Extended Linear uses R16G16B16A16_FLOAT format allowing values > 1.0
 * for brighter-than-white highlights on HDR displays.
 */

#include "dong_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("=== Dong Engine HDR Demo ===\n");
    printf("Testing HDR Extended Linear (R16G16B16A16_FLOAT) rendering\n\n");

    // Check for environment variable overrides
    const char* hdr_env = getenv("DONG_HDR");
    const char* luminance_env = getenv("DONG_HDR_LUMINANCE");

    printf("Environment:\n");
    printf("  DONG_HDR=%s\n", hdr_env ? hdr_env : "(not set)");
    printf("  DONG_HDR_LUMINANCE=%s\n", luminance_env ? luminance_env : "(not set)");
    printf("\n");

    // Create application with HDR enabled
    dong_app_config_t config = {0};
    config.title = "Dong HDR Demo";
    config.width = 900;
    config.height = 1200;
    config.enable_dong = 1;
    config.resizable = 1;
    config.enable_hdr = 1;  // Request HDR mode
    config.hdr_max_luminance = luminance_env ? (float)atof(luminance_env) : 1000.0f;

    dong_app_t* app = dong_app_create(&config);
    if (!app) {
        fprintf(stderr, "Failed to create application\n");
        return 1;
    }

    // Check HDR status
    int hdr_enabled = dong_app_is_hdr_enabled(app);
    float max_luminance = dong_app_get_hdr_max_luminance(app);

    printf("HDR Status:\n");
    if (hdr_enabled) {
        printf("  Mode: HDR Extended Linear (R16G16B16A16_FLOAT)\n");
        printf("  Max Luminance: %.0f nits\n", max_luminance);
    } else {
        printf("  Mode: SDR (R8G8B8A8_UNORM)\n");
        printf("  Note: HDR was requested but not available\n");
    }
    printf("\n");

    // Build HTML with HDR status embedded
    char html_with_status[16384];
    const char* status_class = hdr_enabled ? "status-hdr" : "status-sdr";
    const char* status_text = hdr_enabled
        ? "HDR mode active!"
        : "Running in SDR mode (HDR not available or not supported)";

    // Generate modified HTML with status already set
    snprintf(html_with_status, sizeof(html_with_status),
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <style>\n"
        "        * { box-sizing: border-box; }\n"
        "        body {\n"
        "            margin: 0;\n"
        "            padding: 20px;\n"
        "            background-color: #0a0a0f;\n"
        "            color: #ffffff;\n"
        "            font-family: Arial, sans-serif;\n"
        "        }\n"
        "        h1 {\n"
        "            font-size: 32px;\n"
        "            color: #00d4ff;\n"
        "            margin-bottom: 10px;\n"
        "        }\n"
        "        .status {\n"
        "            font-size: 18px;\n"
        "            margin-bottom: 20px;\n"
        "            padding: 12px;\n"
        "            border-radius: 8px;\n"
        "        }\n"
        "        .status-hdr {\n"
        "            background-color: #1a3a1a;\n"
        "            color: #00ff00;\n"
        "        }\n"
        "        .status-sdr {\n"
        "            background-color: #3a3a1a;\n"
        "            color: #ffaa00;\n"
        "        }\n"
        "        .section {\n"
        "            background-color: #141420;\n"
        "            border-radius: 12px;\n"
        "            padding: 20px;\n"
        "            margin-bottom: 20px;\n"
        "        }\n"
        "        .section-title {\n"
        "            font-size: 20px;\n"
        "            color: #e94560;\n"
        "            margin-bottom: 16px;\n"
        "        }\n"
        "        .color-grid {\n"
        "            display: flex;\n"
        "            flex-wrap: wrap;\n"
        "            gap: 12px;\n"
        "        }\n"
        "        .color-swatch {\n"
        "            width: 80px;\n"
        "            height: 80px;\n"
        "            border-radius: 8px;\n"
        "            display: flex;\n"
        "            align-items: center;\n"
        "            justify-content: center;\n"
        "            font-size: 12px;\n"
        "            color: #000000;\n"
        "            text-shadow: 0 0 2px rgba(255,255,255,0.8);\n"
        "        }\n"
        "        .luminance-bar {\n"
        "            height: 60px;\n"
        "            border-radius: 8px;\n"
        "            margin-bottom: 12px;\n"
        "            display: flex;\n"
        "            align-items: center;\n"
        "            justify-content: center;\n"
        "            font-size: 14px;\n"
        "        }\n"
        "        .gradient-bar {\n"
        "            height: 40px;\n"
        "            border-radius: 8px;\n"
        "            margin-bottom: 8px;\n"
        "        }\n"
        "        .info {\n"
        "            font-size: 14px;\n"
        "            color: #94a3b8;\n"
        "            margin-top: 16px;\n"
        "            line-height: 1.6;\n"
        "        }\n"
        "        .note {\n"
        "            font-size: 12px;\n"
        "            color: #64748b;\n"
        "            font-style: italic;\n"
        "            margin-top: 8px;\n"
        "        }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Dong HDR Demo</h1>\n"
        "    <div class=\"status %s\">%s</div>\n"
        "\n"
        "    <div class=\"section\">\n"
        "        <div class=\"section-title\">Color Swatches</div>\n"
        "        <div class=\"color-grid\">\n"
        "            <div class=\"color-swatch\" style=\"background-color: #ff0000;\">Red</div>\n"
        "            <div class=\"color-swatch\" style=\"background-color: #00ff00;\">Green</div>\n"
        "            <div class=\"color-swatch\" style=\"background-color: #0000ff; color: #ffffff;\">Blue</div>\n"
        "            <div class=\"color-swatch\" style=\"background-color: #ffff00;\">Yellow</div>\n"
        "            <div class=\"color-swatch\" style=\"background-color: #ff00ff;\">Magenta</div>\n"
        "            <div class=\"color-swatch\" style=\"background-color: #00ffff;\">Cyan</div>\n"
        "            <div class=\"color-swatch\" style=\"background-color: #ffffff;\">White</div>\n"
        "            <div class=\"color-swatch\" style=\"background-color: #808080; color: #ffffff;\">Gray</div>\n"
        "        </div>\n"
        "        <div class=\"info\">\n"
        "            Standard sRGB colors. On HDR displays, these should appear identical to SDR mode.\n"
        "        </div>\n"
        "    </div>\n"
        "\n"
        "    <div class=\"section\">\n"
        "        <div class=\"section-title\">Luminance Levels</div>\n"
        "        <div class=\"luminance-bar\" style=\"background-color: #1a1a1a; color: #ffffff;\">10%% (Dark)</div>\n"
        "        <div class=\"luminance-bar\" style=\"background-color: #4d4d4d; color: #ffffff;\">30%%</div>\n"
        "        <div class=\"luminance-bar\" style=\"background-color: #808080; color: #000000;\">50%%</div>\n"
        "        <div class=\"luminance-bar\" style=\"background-color: #b3b3b3; color: #000000;\">70%%</div>\n"
        "        <div class=\"luminance-bar\" style=\"background-color: #e6e6e6; color: #000000;\">90%%</div>\n"
        "        <div class=\"luminance-bar\" style=\"background-color: #ffffff; color: #000000;\">100%% (SDR White)</div>\n"
        "        <div class=\"info\">\n"
        "            On HDR displays, the 100%% white bar represents the SDR reference white level.\n"
        "        </div>\n"
        "    </div>\n"
        "\n"
        "    <div class=\"section\">\n"
        "        <div class=\"section-title\">Grayscale Gradient</div>\n"
        "        <div class=\"gradient-bar\" style=\"background: linear-gradient(to right, #000000, #ffffff);\"></div>\n"
        "        <div class=\"info\">\n"
        "            Smooth gradient from black to white. In HDR mode, this tests tone mapping.\n"
        "        </div>\n"
        "    </div>\n"
        "\n"
        "    <div class=\"section\">\n"
        "        <div class=\"section-title\">Color Gradients</div>\n"
        "        <div class=\"gradient-bar\" style=\"background: linear-gradient(to right, #000000, #ff0000);\"></div>\n"
        "        <div class=\"gradient-bar\" style=\"background: linear-gradient(to right, #000000, #00ff00);\"></div>\n"
        "        <div class=\"gradient-bar\" style=\"background: linear-gradient(to right, #000000, #0000ff);\"></div>\n"
        "        <div class=\"info\">\n"
        "            RGB channel gradients. Check for smooth transitions without banding.\n"
        "        </div>\n"
        "    </div>\n"
        "\n"
        "    <div class=\"section\">\n"
        "        <div class=\"section-title\">HDR Information</div>\n"
        "        <div class=\"info\">\n"
        "            HDR Extended Linear uses R16G16B16A16_FLOAT format with 16-bit per channel.\n"
        "        </div>\n"
        "        <div class=\"note\">\n"
        "            Environment variables: DONG_HDR=1 to force HDR, DONG_HDR_LUMINANCE for max nits.\n"
        "        </div>\n"
        "    </div>\n"
        "</body>\n"
        "</html>\n",
        status_class, status_text);

    // Load HTML content
    dong_app_load_html(app, html_with_status);

    printf("Instructions:\n");
    printf("  - Compare color accuracy and gradients\n");
    printf("  - On HDR displays, whites should be brighter and gradients smoother\n");
    printf("  - Press ESC or close window to exit\n");
    printf("\n");

    // Run main loop
    dong_app_run(app, NULL, NULL);

    // Cleanup
    printf("Shutting down...\n");
    dong_app_destroy(app);

    printf("=== HDR Demo Complete ===\n");
    return 0;
}
