/**
 * Dock Demo
 *
 * Demonstrates the dong_dock docking system with title bars and drag-to-dock:
 *   - 3 panes: left panel with bottom console split, right content panel
 *   - Title bars with tab switching
 *   - Drag tabs to rearrange: edge = split, title bar = tab, outside = detach
 *   - Focus border around active pane
 */

#include "dong_dock.h"
#include <stdio.h>

static const char* HTML_PANEL_A =
    "<!DOCTYPE html>"
    "<html><head><style>"
    "body { margin:0; padding:20px; font-family:sans-serif;"
    "       background:#1a1a2e; color:#eee; }"
    "h2 { color:#e94560; margin-top:0; }"
    "p { color:#aaa; font-size:13px; line-height:1.8; }"
    ".hint { background:#0f3460; padding:8px 12px; border-radius:6px;"
    "        margin:8px 0; font-size:12px; color:#ccc; }"
    "</style></head><body>"
    "<h2>Panel A</h2>"
    "<div class='hint'>Drag tabs to rearrange panes</div>"
    "<div class='hint'>Drag to edge = split</div>"
    "<div class='hint'>Drag to title bar = add as tab</div>"
    "<div class='hint'>Drag outside = detach window</div>"
    "</body></html>";

static const char* HTML_PANEL_B =
    "<!DOCTYPE html>"
    "<html><head><style>"
    "body { margin:0; padding:20px; font-family:sans-serif;"
    "       background:#16213e; color:#eee; }"
    "h2 { color:#00b4d8; margin-top:0; }"
    ".card { background:#0f3460; padding:16px; border-radius:8px;"
    "        margin:12px 0; }"
    ".card h3 { margin:0 0 8px 0; color:#e94560; }"
    ".card p { margin:0; color:#aaa; font-size:13px; }"
    "</style></head><body>"
    "<h2>Panel B</h2>"
    "<div class='card'><h3>Content 1</h3><p>This panel can be dragged.</p></div>"
    "<div class='card'><h3>Content 2</h3><p>Try dragging its tab to the left panel.</p></div>"
    "</body></html>";

static const char* HTML_CONSOLE =
    "<!DOCTYPE html>"
    "<html><head><style>"
    "body { margin:0; padding:16px; font-family:monospace;"
    "       background:#0d1117; color:#58a6ff; font-size:13px; }"
    "div { margin:2px 0; }"
    "</style></head><body>"
    "<div>[LOG] Console pane ready.</div>"
    "<div>[LOG] Drag this tab to rearrange or detach.</div>"
    "</body></html>";

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    dong_dock_config_t cfg = {0};
    cfg.title  = "Dong Dock Demo";
    cfg.width  = 1280;
    cfg.height = 720;

    dong_dock_t* dock = dong_dock_create(&cfg);
    if (!dock) {
        fprintf(stderr, "Failed to create dock\n");
        return 1;
    }

    // Panel A (fills window initially)
    dong_dock_pane_config_t a_cfg = {0};
    a_cfg.title = "Panel A";
    a_cfg.html  = HTML_PANEL_A;
    dong_dock_pane_t* pane_a = dong_dock_add_pane(dock, &a_cfg);

    // Panel B (split right of A, 35/65)
    dong_dock_pane_config_t b_cfg = {0};
    b_cfg.title = "Panel B";
    b_cfg.html  = HTML_PANEL_B;
    dong_dock_pane_t* pane_b = dong_dock_split(dock, pane_a, DONG_DOCK_RIGHT, 0.35f, &b_cfg);

    // Console (split below A, 70/30)
    dong_dock_pane_config_t c_cfg = {0};
    c_cfg.title = "Console";
    c_cfg.html  = HTML_CONSOLE;
    dong_dock_split(dock, pane_a, DONG_DOCK_BOTTOM, 0.7f, &c_cfg);

    if (!pane_a || !pane_b) {
        fprintf(stderr, "Failed to create panes\n");
        dong_dock_destroy(dock);
        return 1;
    }

    printf("Dock demo running.\n");
    printf("  Drag tabs to rearrange, split, or detach.\n");
    printf("  Close window to exit.\n");
    fflush(stdout);

    dong_dock_run(dock, NULL, NULL);

    dong_dock_destroy(dock);
    printf("Dock demo finished.\n");
    return 0;
}
