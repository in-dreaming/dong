/**
 * Minimal Dong Demo (GPU backend)
 */

#include "demo_gpu_platform.h"
#include "dong.h"

#include <stdio.h>

static const char* DEMO_HTML =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "  <style>\n"
    "    body {\n"
    "      font-family: sans-serif;\n"
    "      display: flex;\n"
    "      justify-content: center;\n"
    "      align-items: center;\n"
    "      height: 100vh;\n"
    "      margin: 0;\n"
    "      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
    "    }\n"
    "    .card {\n"
    "      background: white;\n"
    "      border-radius: 16px;\n"
    "      padding: 40px 60px;\n"
    "      text-align: center;\n"
    "    }\n"
    "  </style>\n"
    "</head>\n"
    "<body>\n"
    "  <div class='card'>\n"
    "    <h1>Hello Dong!</h1>\n"
    "    <p>GPU backend minimal demo</p>\n"
    "  </div>\n"
    "</body>\n"
    "</html>\n";

int main(void) {
    DongDemoGpu gpu;
    DongDemoGpuConfig cfg = {
        .title = "Minimal Dong Demo (GPU)",
        .width = 800,
        .height = 600,
    };
    if (!dong_demo_gpu_init(&gpu, &cfg)) {
        return 1;
    }

    dong_engine_t* engine = dong_demo_gpu_create_engine(&gpu);
    if (!engine) {
        dong_demo_gpu_shutdown(&gpu);
        return 1;
    }

    if (dong_engine_load_html(engine, DEMO_HTML) != DONG_OK) {
        dong_demo_gpu_destroy_engine(engine);
        dong_demo_gpu_shutdown(&gpu);
        return 1;
    }

    while (dong_demo_gpu_poll_events(&gpu, engine)) {
        dong_demo_gpu_present(&gpu, engine);
    }

    dong_demo_gpu_destroy_engine(engine);
    dong_demo_gpu_shutdown(&gpu);
    return 0;
}
