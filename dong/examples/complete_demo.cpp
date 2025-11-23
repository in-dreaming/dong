#include <cstdio>
#include <cstdint>
#include <dong.h>

// A "full pipeline" demo for Dong:
// HTML + CSS (cascade & flexbox) -> DOM -> ComputedStyle -> Yoga layout -> Skia render.
int main() {
    std::printf("=== Dong Engine: Complete DOM/CSS/Layout Demo ===\n\n");

    // 1. Create engine context
    std::printf("[1] Creating context...\n");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        std::printf("ERROR: Failed to create context\n");
        return 1;
    }
    std::printf("OK: Context created\n\n");

    // 2. Create a view (surface) for rendering
    const uint32_t initial_width = 960;
    const uint32_t initial_height = 600;
    std::printf("[2] Creating view (%ux%u)...\n", initial_width, initial_height);
    dong_view_t* view = dong_view_create(ctx, initial_width, initial_height);
    if (!view) {
        std::printf("ERROR: Failed to create view\n");
        dong_destroy_context(ctx);
        return 1;
    }
    std::printf("OK: View created\n\n");

    // 3. HTML + CSS that exercises:
    //    - <style> block (global stylesheet)
    //    - class/id selectors, cascade & specificity
    //    - flex layout (row/column, flex-grow)
    //    - margins/padding/typography
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <meta charset="utf-8" />
            <title>Dong Complete Demo</title>
            <style>
                html, body {
                    margin: 0;
                    padding: 0;
                    width: 100%;
                    height: 100%;
                }

                body {
                    background-color: #f5f5f5;
                    font-family: Arial, sans-serif;
                    display: flex;
                    justify-content: center;
                    align-items: flex-start;
                    padding: 24px;
                }

                .app {
                    display: flex;
                    flex-direction: column;
                    gap: 16px;
                    width: 100%;
                    max-width: 800px;
                }

                .header {
                    background-color: #ffffff;
                    padding: 16px 20px;
                    border-radius: 8px;
                    margin-bottom: 4px;
                }

                .header h1 {
                    margin: 0;
                    font-size: 24px;
                    color: #333333;
                }

                .subtitle {
                    margin-top: 8px;
                    color: #666666;
                    font-size: 14px;
                }

                .row {
                    display: flex;
                    gap: 16px;
                }

                .card {
                    flex: 1;
                    background-color: #ffffff;
                    border-radius: 8px;
                    padding: 16px 18px;
                }

                .card h2 {
                    margin: 0 0 8px 0;
                    font-size: 18px;
                }

                .card p {
                    margin: 4px 0;
                    color: #444444;
                    line-height: 1.5;
                }

                .pill {
                    display: inline-block;
                    padding: 4px 10px;
                    border-radius: 999px;
                    background-color: #e3f2fd;
                    color: #1565c0;
                    font-size: 12px;
                    margin-right: 4px;
                }

                .actions {
                    margin-top: 12px;
                    display: flex;
                    gap: 8px;
                }

                .button {
                    padding: 8px 16px;
                    border-radius: 4px;
                    border: none;
                    cursor: pointer;
                    font-size: 14px;
                }

                .button.primary {
                    background-color: #1976d2;
                    color: #ffffff;
                }

                .button.secondary {
                    background-color: #eeeeee;
                    color: #333333;
                }

                footer {
                    margin-top: 8px;
                    font-size: 12px;
                    color: #999999;
                    text-align: center;
                }
            </style>
        </head>
        <body>
            <div class="app">
                <div class="header">
                    <h1>Dong Engine: DOM / CSS / Layout Demo</h1>
                    <div class="subtitle">
                        Full pipeline: HTML & CSS &rarr; DOM &rarr; ComputedStyle &rarr; Yoga layout &rarr; Skia render
                    </div>
                </div>

                <div class="row">
                    <div class="card" id="left-card">
                        <h2>Layout</h2>
                        <p><span class="pill">Flexbox</span><span class="pill">Block</span></p>
                        <p>
                            This column demonstrates flex layout. The two cards share space using
                            <code>flex: 1</code> and gap between them is controlled by <code>.row</code>.
                        </p>
                        <p>
                            Resize the view in the host application to see Yoga recalculate positions
                            and sizes while respecting margins and paddings.
                        </p>
                    </div>

                    <div class="card" id="right-card">
                        <h2>Styling &amp; Script</h2>
                        <p>
                            Styles come from a &lt;style&gt; block using class and element selectors.
                            Inline styles could override them if present.
                        </p>
                        <div class="actions">
                            <button class="button primary" id="primaryBtn">Primary Action</button>
                            <button class="button secondary" id="secondaryBtn">Secondary</button>
                        </div>
                        <p id="status" style="margin-top: 10px; color: #777777;">
                            Click the buttons to see JavaScript integration logs.
                        </p>
                    </div>
                </div>

                <footer>
                    &copy; 2024 Dong Engine &mdash; Lexbor + Yoga + Skia + QuickJS
                </footer>
            </div>

            <script>
                (function() {
                    console.log("[Demo] Script running inside Dong view.");

                    var primary = document.getElementById("primaryBtn");
                    var secondary = document.getElementById("secondaryBtn");
                    var status = document.getElementById("status");

                    function setStatus(text) {
                        if (status) {
                            status.textContent = text;
                        }
                        console.log("[Demo] " + text);
                    }

                    if (primary) {
                        primary.addEventListener("click", function() {
                            setStatus("Primary button clicked.");
                        });
                    }

                    if (secondary) {
                        secondary.addEventListener("click", function() {
                            setStatus("Secondary button clicked.");
                        });
                    }

                    // Log computed style of one card to prove CSS is applied.
                    var card = document.getElementById("left-card");
                    if (card && card.getComputedStyle) {
                        var cs = card.getComputedStyle();
                        console.log("[Demo] Left card background:", cs.backgroundColor);
                        console.log("[Demo] Left card padding-top:", cs.padding_top);
                    }
                })();
            </script>
        </body>
        </html>
    )";

    std::printf("[3] Loading HTML into view...\n");
    dong_view_load_html(view, html);
    std::printf("OK: HTML parsed and DOM tree built\n\n");

    // 4. Run one update to drive script tasks, layout and render
    std::printf("[4] Running initial update (layout + render)...\n");
    dong_view_update(view);
    std::printf("OK: Layout calculated and frame rendered\n\n");

    // 5. (Optional) Sample a few pixels from the CPU buffer as a sanity check
    std::printf("[5] Sampling pixel buffer...\n");
    void* buf = dong_view_get_pixel_buffer(view);
    if (!buf) {
        std::printf("WARNING: No pixel buffer available (maybe using GPU mode).\n\n");
    } else {
        const uint8_t* p = static_cast<const uint8_t*>(buf);
        std::uint64_t checksum = 0;
        const std::size_t total = static_cast<std::size_t>(initial_width) * initial_height * 4;
        for (std::size_t i = 0; i < total; i += 97) {
            checksum += p[i];
        }
        std::printf("Top-left pixel RGBA = (%u, %u, %u, %u)\n", p[0], p[1], p[2], p[3]);
        std::printf("Sample checksum = %llu\n", (unsigned long long)checksum);

        // Additionally, dump the current frame to a simple PPM file so it can be
        // inspected visually with any image viewer that supports PPM format.
        const char* filename = "complete_demo.ppm";
        std::FILE* fp = std::fopen(filename, "wb");
        if (!fp) {
            std::printf("WARNING: Failed to open %s for writing.\n\n", filename);
        } else {
            // P6 binary PPM: RGB, 8 bits per channel.
            std::fprintf(fp, "P6\n%u %u\n255\n", initial_width, initial_height);
            for (uint32_t y = 0; y < initial_height; ++y) {
                for (uint32_t x = 0; x < initial_width; ++x) {
                    std::size_t idx = (static_cast<std::size_t>(y) * initial_width + x) * 4;
                    unsigned char rgb[3] = { p[idx + 0], p[idx + 1], p[idx + 2] };
                    std::fwrite(rgb, 1, 3, fp);
                }
            }
            std::fclose(fp);
            std::printf("PPM snapshot saved to %s\n\n", filename);
        }
    }

    // 6. Execute a small JS snippet to prove querySelector / DOM API works
    std::printf("[6] Executing a verification JavaScript snippet...\n");
    const char* verify_js = R"(
        var heading = document.querySelector("h1");
        if (heading) {
            console.log("[Verify] Found heading: " + heading.textContent);
        } else {
            console.log("[Verify] Heading not found.");
        }
    )";
    bool js_ok = dong_view_eval(view, verify_js);
    std::printf("JS eval result: %s\n\n", js_ok ? "true" : "false");

    // 7. Resize the view to exercise layout recomputation
    const uint32_t new_width = 1280;
    const uint32_t new_height = 720;
    std::printf("[7] Resizing view to %ux%u and updating...\n", new_width, new_height);
    dong_view_resize(view, new_width, new_height);
    dong_view_update(view);
    std::printf("OK: View resized and re-laid out.\n\n");

    // 8. Cleanup
    std::printf("[8] Cleaning up...\n");
    dong_view_free(view);
    dong_destroy_context(ctx);
    std::printf("OK: Resources released\n\n");

    std::printf("=== Complete DOM/CSS/Layout Demo Finished ===\n");
    return 0;
}
