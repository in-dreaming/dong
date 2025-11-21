/**
 * Dong Engine - Render Pipeline & Script Integration Demo
 */

#include <dong.h>
#include <cstdio>
#include <cstdint>

void printPixelBuffer(void* buffer, uint32_t width, uint32_t height) {
    if (!buffer) {
        printf("No pixel buffer\n");
        return;
    }

    uint32_t total_bytes = width * height * 4;
    printf("Pixel buffer size: %u x %u (%u bytes)\n", 
           width, height, total_bytes);
    
    uint8_t* pixels = reinterpret_cast<uint8_t*>(buffer);
    printf("First 5 pixels (RGBA):\n");
    for (int i = 0; i < 5 && i < (int)(width * height); ++i) {
        printf("  Pixel %d: R=%3d G=%3d B=%3d A=%3d\n",
               i, 
               pixels[i * 4 + 0],
               pixels[i * 4 + 1],
               pixels[i * 4 + 2],
               pixels[i * 4 + 3]);
    }
}

int main() {
    printf("=== Dong Engine: Render & Script Demo ===\n\n");

    // 1. Create context
    printf("[1] Creating Dong context...\n");
    dong_context_t* context = dong_create_context();
    if (!context) {
        printf("ERROR: Failed to create context\n");
        return 1;
    }
    printf("OK: Context created\n\n");

    // 2. Create view (800x600)
    printf("[2] Creating view (800x600)...\n");
    dong_view_t* view = dong_view_create(context, 800, 600);
    if (!view) {
        printf("ERROR: Failed to create view\n");
        dong_destroy_context(context);
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
                <h1>Dong Engine Demo</h1>
                <p>This demonstrates the Dong UI engine with Skia rendering and QuickJS scripting.</p>
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
    void* pixel_buffer = dong_view_get_pixel_buffer(view);
    printPixelBuffer(pixel_buffer, 800, 600);
    printf("\n");

    // 6. Execute JavaScript
    printf("[6] Executing JavaScript...\n");
    const char* js_code = R"(
        console.log("Hello from JavaScript!");
        console.log("Document is ready");
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
        printf("WARNING: JavaScript execution had errors\n");
    }
    printf("\n");

    // 7. Update again
    printf("[7] Updating view again...\n");
    dong_view_update(view);
    printf("OK: View updated\n\n");

    // 8. Test resize
    printf("[8] Resizing view to 1024x768...\n");
    dong_view_resize(view, 1024, 768);
    dong_view_update(view);
    pixel_buffer = dong_view_get_pixel_buffer(view);
    printPixelBuffer(pixel_buffer, 1024, 768);
    printf("\n");

    // 9. Cleanup
    printf("[9] Cleaning up...\n");
    dong_view_free(view);
    dong_destroy_context(context);
    printf("OK: Cleanup complete\n\n");

    printf("=== Demo Complete ===\n");
    return 0;
}
