/**
 * Dong Engine - Render Pipeline & Script Integration Demo
 */

#include <dong.h>
#include <cstdio>
#include <cstdint>

void printPixelBuffer(void* buffer, uint32_t width, uint32_t height) {
    if (!buffer) {
        printf("No pixel buffer");
        return;
    }

    uint32_t total_bytes = width * height * 4;
    printf("Pixel buffer size: %u x %u (%u bytes)", 
           width, height, total_bytes);
    
    uint8_t* pixels = reinterpret_cast<uint8_t*>(buffer);
    printf("First 5 pixels (RGBA):");
    for (int i = 0; i < 5 && i < (int)(width * height); ++i) {
        printf("  Pixel %d: R=%3d G=%3d B=%3d A=%3d",
               i, 
               pixels[i * 4 + 0],
               pixels[i * 4 + 1],
               pixels[i * 4 + 2],
               pixels[i * 4 + 3]);
    }
}

int main() {
    printf("=== Dong Engine: Render & Script Demo ===");

    // 1. Create context
    printf("[1] Creating Dong context...");
    dong_context_t* context = dong_create_context();
    if (!context) {
        printf("ERROR: Failed to create context");
        return 1;
    }
    printf("OK: Context created");

    // 2. Create view (800x600)
    printf("[2] Creating view (800x600)...");
    dong_view_t* view = dong_view_create(context, 800, 600);
    if (!view) {
        printf("ERROR: Failed to create view");
        dong_destroy_context(context);
        return 1;
    }
    printf("OK: View created");

    // 3. Load HTML
    printf("[3] Loading HTML...");
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
                <button id=\"demo-button\" class=\"button\">Click me!</button>\n            </div>
        </body>
        </html>
    )";
    
    dong_view_load_html(view, html);
    printf("OK: HTML loaded");

    // 4. Update (layout + render)
    printf("[4] Updating view (layout + render)...");
    dong_view_update(view);
    printf("OK: View updated");

    // 5. Get pixel buffer
    printf("[5] Getting pixel buffer...");
    void* pixel_buffer = dong_view_get_pixel_buffer(view);
    printPixelBuffer(pixel_buffer, 800, 600);
    printf("");

    // 6. Execute JavaScript
    printf("[6] Executing JavaScript...");
    const char* js_code = R"(
        console.log("Hello from JavaScript!");
        console.log("Document is ready");

        // JSON + state demo
        var state = { clicks: 0, label: "Click me" };
        console.log("Initial state:", JSON.stringify(state));

        var heading = document.querySelector("h1");
        if (heading) {
            console.log("Found heading element");
        }
        var button = document.querySelector("button");
        if (button) {
            console.log("Found button element");
            button.addEventListener("click", function (ev) {
                state.clicks++;
                console.log("Button clicked", state.clicks, "times");
            });

            // For now, events are JS-only; manually invoke the handler once
            if (typeof button._simulateClick === "function") {
                button._simulateClick();
            }
        }
    )";
    
    bool js_result = dong_view_eval(view, js_code);
    if (js_result) {
        printf("OK: JavaScript executed successfully");
    } else {
        printf("WARNING: JavaScript execution had errors");
    }
    printf("");

    // 7. Update again
    printf("[7] Updating view again...");
    dong_view_update(view);
    printf("OK: View updated");

    // 8. Test resize
    printf("[8] Resizing view to 1024x768...");
    dong_view_resize(view, 1024, 768);
    dong_view_update(view);
    pixel_buffer = dong_view_get_pixel_buffer(view);
    printPixelBuffer(pixel_buffer, 1024, 768);
    printf("");

    // 9. Cleanup
    printf("[9] Cleaning up...");
    dong_view_free(view);
    dong_destroy_context(context);
    printf("OK: Cleanup complete");

    printf("=== Demo Complete ===");
    return 0;
}
