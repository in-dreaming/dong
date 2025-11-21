#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <iomanip>
#include <cstdio>
#include <dong.h>

// Simple integration test for Dong engine
int main() {
    printf("=== Dong Engine: Integration Demo ===\n\n");

    // 1. Create context
    printf("[1] Creating context...\n");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        printf("ERROR: Failed to create context\n");
        return 1;
    }
    printf("OK: Context created\n\n");

    // 2. Create view
    printf("[2] Creating view (800x600)...\n");
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    if (!view) {
        printf("ERROR: Failed to create view\n");
        dong_destroy_context(ctx);
        return 1;
    }
    printf("OK: View created\n\n");

    // 3. Load HTML
    printf("[3] Loading HTML...\n");
    const char* html = R"(
        <html>
        <head>
            <style>
                body { 
                    background-color: white;
                    font-family: Arial;
                    padding: 20px;
                }
                .container {
                    background-color: #f0f0f0;
                    padding: 10px;
                    border-radius: 5px;
                }
                h1 {
                    color: #333;
                    font-size: 24px;
                }
                .button {
                    background-color: #007bff;
                    color: white;
                    padding: 10px 20px;
                    border-radius: 3px;
                    cursor: pointer;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>Dong Engine Integration Test</h1>
                <p>This demonstrates the full pipeline: HTML parse -&gt; Layout -&gt; Render -&gt; Script</p>
                <button class="button">Click me!</button>
            </div>
        </body>
        </html>
    )";

    dong_view_load_html(view, html);
    printf("OK: HTML loaded\n\n");

    // 4. Update (layout + render)
    printf("[4] Updating view (layout + render)...\n");
    dong_view_update(view);
    printf("OK: View updated\n\n");

    // 5. Get pixel buffer
    printf("[5] Getting pixel buffer...\n");
    void* buffer = dong_view_get_pixel_buffer(view);
    if (buffer) {
        printf("OK: Pixel buffer obtained (800x600 = 1920000 bytes)\n");
    } else {
        printf("WARNING: No pixel buffer available\n");
    }
    printf("\n");

    // 6. Execute JavaScript
    printf("[6] Executing JavaScript...\n");
    const char* js_code = R"(
        console.log("Hello from JavaScript!");
        var heading = document.querySelector("h1");
        if (heading) {
            console.log("Found heading element");
        }
        var buttons = document.getElementsByTagName("button");
        console.log("Found buttons");
    )";

    bool js_result = dong_view_eval(view, js_code);
    if (js_result) {
        printf("OK: JavaScript executed successfully\n");
    } else {
        printf("WARNING: JavaScript execution returned false\n");
    }
    printf("\n");

    // 7. Test resize
    printf("[7] Resizing view to 1024x768...\n");
    dong_view_resize(view, 1024, 768);
    dong_view_update(view);
    buffer = dong_view_get_pixel_buffer(view);
    if (buffer) {
        printf("OK: View resized and rendered\n");
    }
    printf("\n");

    // 8. Cleanup
    printf("[8] Cleaning up...\n");
    dong_view_free(view);
    dong_destroy_context(ctx);
    printf("OK: Cleanup complete\n\n");

    printf("=== Integration Test Complete ===\n");
    return 0;
}
