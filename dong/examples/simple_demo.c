#include <dong.h>
#include <stdio.h>

int main() {
    printf("=== Dong Engine: Simple Demo ===\n\n");

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
    const char* html =
        "<html>"
        "<head><style>"
        "body { background-color: white; padding: 20px; }"
        "h1 { color: #333; font-size: 24px; }"
        "</style></head>"
        "<body>"
        "<h1>Dong Engine Demo</h1>"
        "<p>Simple rendering test</p>"
        "</body>"
        "</html>";

    dong_view_load_html(view, html);
    printf("OK: HTML loaded\n\n");

    // 3.5 Run a simple JavaScript snippet
    printf("[3.5] Running JavaScript...\n");
    if (!dong_view_eval(view, "console.log('Hello from JS!')")) {
        printf("WARNING: JS eval failed\n\n");
    } else {
        printf("OK: JS eval succeeded\n\n");
    }

    // 4. Update (layout + render)
    printf("[4] Updating view...\n");
    dong_view_update(view);
    printf("OK: View updated\n\n");

    // 5. Get pixel buffer
    printf("[5] Getting pixel buffer...\n");
    void* buffer = dong_view_get_pixel_buffer(view);
    if (buffer) {
        printf("OK: Pixel buffer obtained\n");
    } else {
        printf("WARNING: No pixel buffer available\n");
    }
    printf("\n");

    // 6. Cleanup
    printf("[6] Cleaning up...\n");
    dong_view_free(view);
    dong_destroy_context(ctx);
    printf("OK: Cleanup complete\n\n");

    printf("=== Demo Complete ===\n");
    return 0;
}
