#include <dong.h>
#include <stdio.h>
#include <stdlib.h>

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
        "body { background-color: #202020; padding: 20px; }"
        "h1 { background-color: #ffcc00; color: #202020; font-size: 24px; padding: 8px; border-radius: 8px; }"
        "p { background-color: #336699; color: #ffffff; padding: 8px; border-radius: 4px; }"
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

        // Dump to a simple PPM image for visualization
        const int width = 800;
        const int height = 600;
        FILE* fp = fopen("simple_demo.ppm", "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", width, height);
            unsigned char* pixels = (unsigned char*)buffer;

            int cx = width / 2;
            int cy = height / 2;
            int cidx = (cy * width + cx) * 4;
            printf("Center pixel RGBA = (%u, %u, %u, %u)\n",
                   pixels[cidx + 0], pixels[cidx + 1], pixels[cidx + 2], pixels[cidx + 3]);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int idx = (y * width + x) * 4; // RGBA
                    unsigned char rgb[3] = {
                        pixels[idx + 0], // R
                        pixels[idx + 1], // G
                        pixels[idx + 2]  // B
                    };
                    fwrite(rgb, 1, 3, fp);
                }
            }
            fclose(fp);
            printf("Saved image to simple_demo.ppm\n");

            // Also convert to PNG via macOS 'sips' tool for easier viewing
            int rc = system("sips -s format png simple_demo.ppm --out simple_demo.png >/dev/null 2>&1");
            if (rc == 0) {
                printf("Saved image to simple_demo.png\n");
            } else {
                printf("WARNING: Failed to convert simple_demo.ppm to PNG (sips exit code %d)\n", rc);
            }
        } else {
            printf("WARNING: Failed to open simple_demo.ppm for writing\n");
        }
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
