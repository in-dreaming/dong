/**
 * Minimal Dong Demo
 *
 * This example demonstrates the simplest possible Dong application:
 * - Create a window with GPU support
 * - Load HTML content
 * - Run the main loop
 *
 * Total: ~50 lines of code (vs 1500+ in the legacy approach)
 */

#include "dong_app.h"
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
    "      box-shadow: 0 20px 60px rgba(0,0,0,0.3);\n"
    "      text-align: center;\n"
    "    }\n"
    "    h1 { color: #333; margin: 0 0 10px 0; }\n"
    "    p { color: #666; margin: 0; }\n"
    "  </style>\n"
    "</head>\n"
    "<body>\n"
    "  <div class='card'>\n"
    "    <h1>Hello Dong!</h1>\n"
    "    <p>GPU-accelerated HTML/CSS rendering</p>\n"
    "  </div>\n"
    "</body>\n"
    "</html>\n";

int main(void) {
    // Configure the application
    dong_app_config_t config = {0};
    config.title = "Minimal Dong Demo";
    config.width = 800;
    config.height = 600;
    config.enable_dong = 1;
    config.resizable = 1;

    // Create the application
    dong_app_t* app = dong_app_create(&config);
    if (!app) {
        fprintf(stderr, "Failed to create application\n");
        return 1;
    }

    // Load HTML content (using new simplified API)
    dong_app_load_html(app, DEMO_HTML);

    // Run the main loop (blocks until window is closed)
    dong_app_run(app, NULL, NULL);

    // Cleanup
    dong_app_destroy(app);
    return 0;
}
