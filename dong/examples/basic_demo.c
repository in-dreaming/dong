#include <dong.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Dong UI Engine - Hello World Demo\n\n");

    // Create context
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        printf("Failed to create context\n");
        return 1;
    }

    // Create view
    dong_view_t* view = dong_view_create(ctx, 800, 600);
    if (!view) {
        printf("Failed to create view\n");
        dong_destroy_context(ctx);
        return 1;
    }

    // Load simple HTML + CSS
    const char* html =
        "<html>"
        "<head><style>"
        "body { background-color: #202020; padding: 40px; }"
        "h1 { background-color: #ffcc00; color: #202020; font-size: 32px; padding: 12px 16px; border-radius: 8px; }"
        "p { background-color: #336699; color: #ffffff; padding: 10px 16px; border-radius: 4px; margin-top: 16px; }"
        "</style></head>"
        "<body>"
        "<h1>Hello Dong!</h1>"
        "<p>This is your first Dong UI Engine page.</p>"
        "</body>"
        "</html>";
    dong_view_load_html(view, html);

    // Update and render
    dong_view_update(view);

    // Get pixel buffer and dump to PNG via PPM
    void* buffer = dong_view_get_pixel_buffer(view);
    if (buffer) {
        const int width = 800;
        const int height = 600;
        unsigned char* pixels = (unsigned char*)buffer;

        // Debug: sample a grid of pixels around expected text positions
        int sample_coords[][2] = {
            // h1 baseline scan (y ≈ 63)
            {56, 63}, {80, 63}, {104, 63}, {128, 63}, {160, 63},
            // h1 inside/background reference
            {56, 50}, {80, 50},
            {56, 80}, {80, 80},
            // p baseline scan (y ≈ 96)
            {56, 96}, {80, 96}, {104, 96}, {128, 96}, {160, 96},
            // p inside/background reference
            {56, 90}, {80, 90},
            {400, 300} // center of screen
        };
        int sample_count = (int)(sizeof(sample_coords) / sizeof(sample_coords[0]));

        printf("Sampled pixels (x,y -> RGBA):\n");
        for (int i = 0; i < sample_count; ++i) {
            int sx = sample_coords[i][0];
            int sy = sample_coords[i][1];
            if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                int idx = (sy * width + sx) * 4;
                unsigned char r = pixels[idx + 0];
                unsigned char g = pixels[idx + 1];
                unsigned char b = pixels[idx + 2];
                unsigned char a = pixels[idx + 3];
                printf("  (%3d,%3d) -> (%3u,%3u,%3u,%3u)\n", sx, sy, r, g, b, a);
            }
        }

        FILE* fp = fopen("hello_demo.ppm", "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", width, height);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int idx = (y * width + x) * 4; // RGBA
                    unsigned char rgb[3] = {
                        pixels[idx + 0],
                        pixels[idx + 1],
                        pixels[idx + 2]
                    };
                    fwrite(rgb, 1, 3, fp);
                }
            }
            fclose(fp);
            printf("Saved image to hello_demo.ppm\n");

            int rc = system("sips -s format png hello_demo.ppm --out hello_demo.png >/dev/null 2>&1");
            if (rc == 0) {
                printf("Saved image to hello_demo.png\n");
            } else {
                printf("WARNING: Failed to convert hello_demo.ppm to PNG (sips exit code %d)\n", rc);
            }
        } else {
            printf("WARNING: Failed to open hello_demo.ppm for writing\n");
        }
    } else {
        printf("WARNING: No pixel buffer available\n");
    }

    printf("Demo completed successfully\n");

    // Cleanup
    dong_view_destroy(view);
    dong_destroy_context(ctx);

    return 0;
}
