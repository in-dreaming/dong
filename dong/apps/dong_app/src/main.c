/**
 * Dong App - Simplified HTML Viewer/Runner
 *
 * A simple application that loads and displays HTML files using
 * the new AppCore architecture.
 *
 * Usage:
 *   dong_app [--html <file.html>] [--width <w>] [--height <h>]
 */

#include "dong_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default demo HTML
static const char* DEFAULT_HTML =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <style>\n"
    "        body {\n"
    "            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);\n"
    "            color: #eee;\n"
    "            font-family: sans-serif;\n"
    "            display: flex;\n"
    "            justify-content: center;\n"
    "            align-items: center;\n"
    "            height: 100vh;\n"
    "            margin: 0;\n"
    "        }\n"
    "        .container {\n"
    "            text-align: center;\n"
    "            padding: 40px;\n"
    "            background: rgba(255,255,255,0.05);\n"
    "            border-radius: 16px;\n"
    "            box-shadow: 0 8px 32px rgba(0,0,0,0.3);\n"
    "        }\n"
    "        h1 {\n"
    "            font-size: 48px;\n"
    "            margin-bottom: 16px;\n"
    "            color: #00d4ff;\n"
    "        }\n"
    "        p {\n"
    "            font-size: 18px;\n"
    "            opacity: 0.8;\n"
    "        }\n"
    "        button {\n"
    "            margin-top: 24px;\n"
    "            padding: 12px 32px;\n"
    "            font-size: 16px;\n"
    "            background: #7b2cbf;\n"
    "            color: white;\n"
    "            border: none;\n"
    "            border-radius: 8px;\n"
    "            cursor: pointer;\n"
    "        }\n"
    "        button:hover {\n"
    "            background: #9d4edd;\n"
    "        }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <div class=\"container\">\n"
    "        <h1>Dong UI Engine</h1>\n"
    "        <p>A lightweight, embeddable HTML/CSS rendering engine</p>\n"
    "        <button id=\"btn\" onclick=\"alert('Hello from Dong!')\">Click Me</button>\n"
    "    </div>\n"
    "</body>\n"
    "</html>\n";

static void print_usage(void) {
    fprintf(stderr,
        "Usage: dong_app [--html <file.html>] [--width <w>] [--height <h>]\n"
        "\n"
        "Options:\n"
        "  --html <file>  HTML file to load\n"
        "  --width <w>    Window width (default: 960)\n"
        "  --height <h>   Window height (default: 540)\n"
        "  --help         Show this help\n");
}

int main(int argc, char** argv) {
    const char* html_file = NULL;
    uint32_t width = 960;
    uint32_t height = 540;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--html") == 0 && i + 1 < argc) {
            html_file = argv[++i];
        } else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            width = (uint32_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            height = (uint32_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    // Create application
    dong_app_config_t config = {0};
    config.title = html_file ? html_file : "Dong App";
    config.width = width;
    config.height = height;
    config.enable_dong = 1;
    config.resizable = 1;

    dong_app_t* app = dong_app_create(&config);
    if (!app) {
        fprintf(stderr, "[dong_app] Failed to create application\n");
        return 1;
    }

    // Load HTML content (using new simplified API)
    if (html_file) {
        if (!dong_app_load_html_file(app, html_file)) {
            fprintf(stderr, "[dong_app] Failed to load HTML file: %s\n", html_file);
            dong_app_destroy(app);
            return 1;
        }
        printf("[dong_app] Loaded: %s\n", html_file);
    } else {
        dong_app_load_html(app, DEFAULT_HTML);
    }

    printf("[dong_app] Running...\n");

    // Main loop
    dong_app_run(app, NULL, NULL);

    // Cleanup
    printf("[dong_app] Shutting down...\n");
    dong_app_destroy(app);

    return 0;
}
