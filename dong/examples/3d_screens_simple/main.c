/**
 * Dong Engine - 3D HTML Screens Demo (Complete)
 *
 * This example fully replicates the 3d_screen_script.cpp functionality
 * using the new AppCore Scene3D API - ~200 lines vs 1500+ in the legacy approach.
 *
 * Features:
 *   - Multiple HTML test screens with automatic arrangement
 *   - First-person camera controls (WASD + mouse)
 *   - HTML/CSS/JS interaction on 3D screens
 *   - HUD overlay with FPS display and help panel
 *
 * Controls:
 *   - Right mouse + mouse: Look around
 *   - WASD: Move forward/back/left/right
 *   - Space/E: Move up, Ctrl/Q: Move down
 *   - Shift: Sprint
 *   - Left click: Interact with HTML screens
 *   - F1/H: Toggle help panel
 *   - ESC: Exit
 */

#include "dong_app.h"
#include "dong_scene3d.h"
#include "dong_overlay.h"
#include "dong.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static dong_scene3d_t* g_scene = NULL;
static int g_toggle_help_requested = 0;

static void on_app_event(void* user_data, const dong_app_event_t* event) {
    (void)user_data;
    if (g_scene) {
        dong_scene3d_process_event(g_scene, event);
    }
    if (event && event->type == DONG_APP_EVENT_KEY && event->key.pressed) {
        if (event->key.key_code == SDLK_F1 || event->key.key_code == SDLK_H) {
            g_toggle_help_requested = 1;
        }


    }
}



// Screen configuration structure (mirrors ScreenConfig from legacy)
typedef struct {
    const char* html_file;
    uint32_t rt_width;
    uint32_t rt_height;
    float screen_width;
    float screen_height;
} ScreenConfig;

// All test screens from 3d_screen_script.cpp
static const ScreenConfig SCREEN_CONFIGS[] = {
    {"screen1_script.html", 800, 1280, 3.0f, 4.8f},
    {"screen2_script.html", 960, 640, 3.0f, 2.0f},
    {"feature_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/cursor_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/image_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/background_attachment_fixed_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/background_clip_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/background_origin_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/font_style_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/outline_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/queryselector_complex_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/stylesheets_deleterule_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/stylesheets_insertrule_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/text_decoration_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/text_shadow_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/text_wrap_test.html", 960, 640, 3.0f, 2.0f},
    {"tests/transform_test.html", 960, 640, 3.0f, 2.0f},
    {"video/video_play_test.html", 960, 640, 3.0f, 2.0f},
    {"video/video_test.html", 960, 640, 3.0f, 2.0f},
    {"video/video_events_test.html", 960, 640, 3.0f, 2.0f},
    {"video/video_acceptance.html", 960, 640, 3.0f, 2.0f},
    {"video/video_js_api_smoke_test.html", 960, 640, 3.0f, 2.0f},
};

static const int NUM_SCREENS = sizeof(SCREEN_CONFIGS) / sizeof(SCREEN_CONFIGS[0]);

static int is_data_dir(const char* dir) {
    if (!dir || !dir[0]) return 0;
    char test[1024];
    snprintf(test, sizeof(test), "%s/screen1_script.html", dir);
    FILE* f = fopen(test, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void try_set_data_path(const char* candidate, char* out, size_t out_size) {
    if (!candidate || !candidate[0] || out[0]) return;
    if (!is_data_dir(candidate)) return;
    strncpy(out, candidate, out_size - 1);
    out[out_size - 1] = 0;
    printf("[Scene3D] Found data directory: %s\n", out);
}

static void try_set_data_path_from_base(const char* base, const char* suffix, char* out, size_t out_size) {
    if (!base || !base[0] || !suffix || !suffix[0] || out[0]) return;
    char candidate[1024];
    snprintf(candidate, sizeof(candidate), "%s%s", base, suffix);
    try_set_data_path(candidate, out, out_size);
}

// Get data path (looks for examples/data relative to executable)
static const char* get_data_path(void) {
    static char path[1024] = {0};
    if (path[0] == 0) {
        const char* env_root = getenv("DONG_DATA_ROOT");
        try_set_data_path(env_root, path, sizeof(path));

        const char* candidates[] = {
            "data",
            "../data",
            "examples/data",
            "../examples/data",
            "../../examples/data",
            "dong/examples/data",
            "../dong/examples/data",
        };
        for (int i = 0; i < (int)(sizeof(candidates) / sizeof(candidates[0])); i++) {
            try_set_data_path(candidates[i], path, sizeof(path));
        }

        const char* base = SDL_GetBasePath();
        if (base) {
            try_set_data_path_from_base(base, "data", path, sizeof(path));
            try_set_data_path_from_base(base, "../data", path, sizeof(path));
            try_set_data_path_from_base(base, "../examples/data", path, sizeof(path));
            try_set_data_path_from_base(base, "../../examples/data", path, sizeof(path));
        }

        if (path[0] == 0) {
            strcpy(path, "data");
        }
    }
    return path;
}


// Fallback HUD HTML (used if hud.html not found)
static const char* FALLBACK_HUD =
    "<html><body style='background:transparent;font-family:Arial;'>"
    "<div id='fps' style='position:absolute;top:12px;left:12px;color:#00ff88;font-size:18px;text-shadow:1px 1px 0 rgba(0,0,0,0.9);'>"
    "FPS: <span id='fps-value' style='color:#ffff00;font-weight:bold;background:rgba(0,0,0,0.5);padding:2px 6px;border-radius:3px;'>0</span>"
    "</div>"
    "<div id='help-hint' style='position:absolute;bottom:12px;left:12px;font-size:14px;color:white;text-shadow:1px 1px 0 rgba(0,0,0,0.9);background:rgba(0,0,0,0.5);padding:6px 12px;border-radius:4px;'>"
    "Press F1 or H for help"
    "</div>"
    "<script>function toggleHelp(){}</script>"
    "</body></html>";

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("=== Dong 3D Multi-Screen Demo (AppCore) ===\n");
    printf("This demo supports %d HTML screens with automatic arrangement\n", NUM_SCREENS);
    printf("Controls: RMB+Mouse=Look, WASD=Move, Space/E=Up, Ctrl/Q=Down, Shift=Sprint, F1/H=Help, ESC=Exit\n");

    // 为了与 ./dong_app --html 的表现保持一致，这里不再默认禁用脚本。
    // 如需禁用：自行设置环境变量 DONG_DISABLE_SCRIPTS=1。

    // Create application
    dong_app_config_t config = {0};
    config.title = "Dong 3D Multi-Screen Demo";
    config.width = 2000;
    config.height = 1000;
    config.enable_dong = 1;
    config.resizable = 1;

    dong_app_t* app = dong_app_create(&config);
    if (!app) {
        fprintf(stderr, "Failed to create application\n");
        return 1;
    }

    // Create 3D scene
    dong_scene3d_t* scene = dong_scene3d_create(app);
    if (!scene) {
        fprintf(stderr, "Failed to create 3D scene\n");
        dong_app_destroy(app);
        return 1;
    }
    g_scene = scene;

    // Forward SDL input events to Scene3D (mouse/keyboard/text)
    dong_app_set_event_callback(app, on_app_event, NULL);

    // Enable SDL text input so input fields can receive SDL_EVENT_TEXT_INPUT
    dong_app_enable_text_input(app, 1);

    // Set resource root for HTML file resolution
    const char* data_path = get_data_path();
    dong_scene3d_set_resource_root(scene, data_path);

    // Add all test screens
    printf("Loading %d HTML screens...\n", NUM_SCREENS);
    for (int i = 0; i < NUM_SCREENS; i++) {
        dong_screen3d_config_t scfg = {0};
        scfg.html_file = SCREEN_CONFIGS[i].html_file;
        scfg.width = SCREEN_CONFIGS[i].rt_width;
        scfg.height = SCREEN_CONFIGS[i].rt_height;
        scfg.screen_width = SCREEN_CONFIGS[i].screen_width;
        scfg.screen_height = SCREEN_CONFIGS[i].screen_height;
        // Position will be set by arrange_screens

        dong_screen3d_t* screen = dong_scene3d_add_screen(scene, &scfg);
        if (!screen) {
            printf("[Scene3D] Warning: Failed to add screen %d (%s)\n", i, SCREEN_CONFIGS[i].html_file);
        }
    }

    // Automatically arrange screens in a grid
    dong_scene3d_arrange_screens(scene, 4.0f, 2.8f, 5);

    // Apply text renderer mode from environment variable to all screens
    {
        const char* tr_env = getenv("DONG_TEXT_RENDERER");
        if (tr_env) {
            dong_text_renderer_mode_t mode = DONG_TEXT_RENDERER_AUTO;
            if (strcmp(tr_env, "slug") == 0 || strcmp(tr_env, "Slug") == 0 || strcmp(tr_env, "SLUG") == 0) {
                mode = DONG_TEXT_RENDERER_SLUG;
            } else if (strcmp(tr_env, "msdf") == 0 || strcmp(tr_env, "Msdf") == 0 || strcmp(tr_env, "MSDF") == 0) {
                mode = DONG_TEXT_RENDERER_MSDF;
            }
            for (int i = 0; i < dong_scene3d_get_screen_count(scene); i++) {
                dong_screen3d_t* scr = dong_scene3d_get_screen(scene, i);
                dong_engine_t* eng = (dong_engine_t*)dong_screen3d_get_view(scr);
                if (eng) dong_engine_set_text_renderer_mode(eng, mode);
            }
            printf("[Scene3D] Text renderer: %s (from DONG_TEXT_RENDERER='%s')\n",
                   mode == DONG_TEXT_RENDERER_SLUG ? "Slug" :
                   mode == DONG_TEXT_RENDERER_MSDF ? "MSDF" : "Auto", tr_env);
        }
    }

    // Print screen arrangement
    printf("Screen arrangement:\n");
    int count = dong_scene3d_get_screen_count(scene);
    for (int i = 0; i < count; i++) {
        dong_screen3d_t* scr = dong_scene3d_get_screen(scene, i);
        if (scr) {
            printf("  Screen %d: %s\n", i, SCREEN_CONFIGS[i].html_file);
        }
    }
    printf("Created 3D scene with %d screens\n", dong_scene3d_get_screen_count(scene));

    // Create HUD overlay (managed by scene3d)
    dong_overlay_t* hud = dong_scene3d_add_overlay_file(scene, "hud.html", 0, 0);
    if (!hud) {
        // Fallback to inline HUD
        hud = dong_scene3d_add_overlay(scene, FALLBACK_HUD, 0, 0);
        if (hud) {
            printf("[HUD] Using fallback HUD\n");
        }
    } else {
        printf("[HUD] Loaded hud.html\n");
    }

    // FPS tracking
    int frame_count = 0;
    float fps_timer = 0.0f;
    float fps = 0.0f;
    float total_time = 0.0f;
    int total_frames = 0;

    // Main loop
    while (dong_app_is_running(app)) {

        if (!dong_app_poll_events(app)) break;

        float dt = dong_app_get_delta_time(app);

        // Handle F1/H for help toggle (event-driven)
        if (g_toggle_help_requested) {
            g_toggle_help_requested = 0;
            if (hud) {
                dong_overlay_eval_script(hud, "if(typeof toggleHelp==='function')toggleHelp();");
            }
        }


        // Update FPS display
        frame_count++;
        total_frames++;
        fps_timer += dt;
        total_time += dt;
        if (fps_timer >= 1.0f) {
            fps = (float)frame_count / fps_timer;
            frame_count = 0;
            fps_timer = 0.0f;

            // Update FPS in HUD
            if (hud) {
                char js[256];
                snprintf(js, sizeof(js),
                    "var e=document.getElementById('fps-value');if(e)e.textContent='%d';",
                    (int)fps);
                dong_overlay_eval_script(hud, js);
            }
            // Also print to console for performance monitoring
            printf("[FPS] %.1f\n", fps);
            fflush(stdout);
        }

        // Auto-exit after 10 seconds for benchmark (uncomment for testing)
        // if (total_time >= 10.0f) {
        //     printf("\n[BENCHMARK] Total frames: %d, Time: %.2fs, Average FPS: %.1f\n",
        //            total_frames, total_time, (float)total_frames / total_time);
        //     break;
        // }

        // Update scene (camera controls, HTML texture updates, overlay updates)
        dong_scene3d_update(scene, dt);

        // Render 3D scene + HUD overlays (all in one pass)
        // Note: dong_scene3d_render handles swapchain acquisition and submission internally,e'e'e'eeeee
        // so we don't need to call dong_app_present here.
        dong_scene3d_render(scene);

        // dong_app_present is intentionally NOT called here because dong_scene3d_render
        // already acquires and submits to the swapchain. Calling both would cause conflicts.
    }

    // Cleanup (scene3d manages overlays, no need to destroy separately)
    printf("Shutting down...\n");
    dong_scene3d_destroy(scene);
    dong_app_destroy(app);

    printf("=== 3D Multi-Screen Demo Complete ===\n");
    return 0;
}
