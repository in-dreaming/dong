/**
 * Dong App (GPU backend) - HTML viewer using in-dreaming/gpu.
 *
 * Built when CMake DONG_BACKEND=gpu; installed as dong_app.exe.
 *
 * Usage:
 *   dong_app [--html <file.html>] [--width <w>] [--height <h>]
 */

#include "demo_gpu_platform.h"
#include "dong.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    "        }\n"
    "        h1 { color: #00d4ff; }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <div class=\"container\">\n"
    "        <h1>Dong UI Engine</h1>\n"
    "        <p>GPU backend (in-dreaming/gpu)</p>\n"
    "    </div>\n"
    "</body>\n"
    "</html>\n";

static void print_usage(void) {
    fprintf(stderr,
            "Usage: dong_app [--html <file.html>] [--width <w>] [--height <h>]\n"
            "\n"
            "GPU backend build. For SDL backend demos use: zig build -Dbackend=sdl\n");
}

int main(int argc, char** argv) {
    const char* html_file = NULL;
    uint32_t width = 960;
    uint32_t height = 540;

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

    DongDemoGpu gpu;
    DongDemoGpuConfig cfg = {
        .title = html_file ? html_file : "Dong App (GPU)",
        .width = width,
        .height = height,
    };
    if (!dong_demo_gpu_init(&gpu, &cfg)) {
        return 1;
    }

    dong_engine_t* engine = dong_demo_gpu_create_engine(&gpu);
    if (!engine) {
        dong_demo_gpu_shutdown(&gpu);
        return 1;
    }

    if (html_file) {
        if (!dong_demo_gpu_load_html_file(engine, html_file)) {
            fprintf(stderr, "[dong_app] Failed to load HTML file: %s\n", html_file);
            dong_demo_gpu_destroy_engine(engine);
            dong_demo_gpu_shutdown(&gpu);
            return 1;
        }
        printf("[dong_app] Loaded: %s\n", html_file);
    } else {
        if (dong_engine_load_html(engine, DEFAULT_HTML) != DONG_OK) {
            fprintf(stderr, "[dong_app] Failed to load default HTML\n");
            dong_demo_gpu_destroy_engine(engine);
            dong_demo_gpu_shutdown(&gpu);
            return 1;
        }
    }

    printf("[dong_app] Running (GPU backend)...\n");

    const char* aq_env = getenv("DONG_AUTO_QUIT");
    int auto_quit_frames = aq_env ? atoi(aq_env) : 0;
    int frame = 0;

    while (dong_demo_gpu_poll_events(&gpu, engine)) {
        dong_demo_gpu_present(&gpu, engine);
        if (auto_quit_frames > 0 && ++frame >= auto_quit_frames) {
            printf("[dong_app] Auto-quit after %d frames\n", frame);
            break;
        }
    }

    printf("[dong_app] Shutting down...\n");
    dong_demo_gpu_destroy_engine(engine);
    dong_demo_gpu_shutdown(&gpu);
    return 0;
}
