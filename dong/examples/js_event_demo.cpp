#include <dong.h>
#include <cstdio>
#include <cstdint>

int main() {
    printf("=== Dong Engine: JS Event Bridge Demo ===\n");

    // 1. Create context
    printf("[1] Creating Dong context...\n");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        printf("ERROR: Failed to create context\n");
        return 1;
    }
    printf("OK: Context created\n");

    // 2. Create view
    printf("[2] Creating view (800x600)...\n");
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    if (!view) {
        printf("ERROR: Failed to create view\n");
        dong_destroy_context(ctx);
        return 1;
    }
    printf("OK: View created\n");

    // 3. Load HTML with a clickable button
    printf("[3] Loading HTML...\n");
    const char* html = R"(
        <html>
        <head>
            <style>
                body {
                    background-color: #202020;
                    color: #ffffff;
                    font-family: Arial;
                    padding: 40px;
                }
                .card {
                    background-color: #303030;
                    padding: 16px;
                    border-radius: 8px;
                }
                #counter-label {
                    margin-bottom: 12px;
                }
                #counter-button {
                    background-color: #007bff;
                    color: #ffffff;
                    padding: 10px 18px;
                    border-radius: 4px;
                    cursor: pointer;
                }
            </style>
        </head>
        <body>
            <div class="card">
                <div id="counter-label">Clicks: 0</div>
                <button id="counter-button">Click me</button>
            </div>
        </body>
        </html>
    )";

    dong_view_load_html(view, html);
    printf("OK: HTML loaded\n");

    // 4. First update (build DOM/layout/render)
    printf("[4] Running initial update...\n");
    dong_view_update(view);
    printf("OK: Initial update complete\n");

    // 5. Install JS that uses JSON + event listeners
    printf("[5] Installing JavaScript event handlers...\n");
    const char* js_code = R"(
        console.log("[JS] JS environment ready");

        var state = { clicks: 0 };
        console.log("[JS] Initial state:", JSON.stringify(state));

        var button = document.getElementById("counter-button");
        var label = document.getElementById("counter-label");
        if (!button || !label) {
            console.error("[JS] Missing button or label element");
        } else {
            console.log("[JS] Button and label found");
            button.addEventListener("click", function (ev) {
                state.clicks++;
                label.textContent = "Clicks: " + state.clicks;
                console.log("[JS] Click handled:", state.clicks, "times, state =", JSON.stringify(state));
            });
        }
    )";

    if (!dong_view_eval(view, js_code)) {
        printf("WARNING: JS eval failed\n");
    } else {
        printf("OK: JS eval succeeded\n");
    }

    // 6. Trigger multiple synthetic clicks via C API -> View -> DOM hit-test -> JS
    printf("[6] Sending synthetic mouse events (3x: move + down + up)...\n");
    // Position roughly over the button area; if hit-test misses, View will
    // fall back to the first <button> element.
    const int32_t click_x = 160;
    const int32_t click_y = 140;
    for (int i = 0; i < 3; ++i) {
        printf("  - Synthetic click #%d at (%d,%d)\n", i + 1, click_x, click_y);
        dong_view_send_mouse_move(view, click_x, click_y);
        dong_view_send_mouse_down(view, 0);
        dong_view_send_mouse_up(view, 0);
    }
    printf("OK: Mouse events sent\n");

    // 7. Run another update so layout/render reflect JS DOM changes
    printf("[7] Running post-click update...\n");
    dong_view_update(view);
    printf("OK: Post-click update complete\n");

    // 8. Optionally sample center pixel just to prove we rendered something
    void* buffer = dong_view_get_pixel_buffer(view);
    if (buffer) {
        const int width = 800;
        const int height = 600;
        int cx = width / 2;
        int cy = height / 2;
        unsigned char* pixels = (unsigned char*)buffer;
        int idx = (cy * width + cx) * 4;
        printf("Center pixel RGBA = (%u,%u,%u,%u)\n",
               pixels[idx + 0], pixels[idx + 1], pixels[idx + 2], pixels[idx + 3]);
    } else {
        printf("WARNING: No pixel buffer available\n");
    }

    // 9. Cleanup
    printf("[9] Cleaning up...\n");
    dong_view_free(view);
    dong_destroy_context(ctx);
    printf("OK: Cleanup complete\n");

    printf("=== JS Event Bridge Demo Complete ===\n");
    return 0;
}
