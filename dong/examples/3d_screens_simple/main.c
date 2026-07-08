/**
 * Dong Engine - 3D HTML Screens Demo (floor layout + Porffor + curated cases)
 *
 * Features:
 *   - Porffor-ready component demos + engine feature tests laid on the XZ floor (pitch)
 *   - Vertical HTML "billboards" as zone titles
 *   - First-person camera (WASD + mouse); optional overview camera preset
 *   - HUD overlay with FPS / help
 *
 * Controls:
 *   - Right mouse + mouse: Look around
 *   - WASD: Move forward/back/left/right
 *   - Space/E: Up, Ctrl/Q: Down
 *   - Shift: Sprint
 *   - Left click: Interact with HTML screens
 *   - F1/H: Toggle HUD controls
 *   - ESC: Exit
 */

#include "dong_app.h"
#include "dong_math.h"
#include "dong_scene3d.h"
#include "dong_overlay.h"
#include "dong.h"
#if !defined(DONG_BACKEND_GPU)
#include <SDL3/SDL.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(DONG_BACKEND_GPU) && defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if defined(DONG_BACKEND_GPU)
#define DONG_KEY_F1 0x70
#define DONG_KEY_H 'h'
#else
#define DONG_KEY_F1 SDLK_F1
#define DONG_KEY_H SDLK_H
#endif

static dong_scene3d_t* g_scene = NULL;
static int g_help_requested = 0;

static void on_app_event(void* user_data, const dong_app_event_t* event) {
    (void)user_data;
    if (g_scene) {
        dong_scene3d_process_event(g_scene, event);
    }
    if (event && event->type == DONG_APP_EVENT_KEY && event->key.pressed) {
        if (event->key.key_code == DONG_KEY_F1 || event->key.key_code == DONG_KEY_H) {
            g_help_requested = 1;
        }
    }
}

typedef struct {
    const char* html_file;
    uint32_t rt_width;
    uint32_t rt_height;
    float screen_width;
    float screen_height;
} FloorPanel;

typedef struct {
    const char* zone_title;
    int panel_count;
    const FloorPanel* panels;
} ZoneRow;

static const FloorPanel k_component_showcase[] = {
    {"porf-counter/index.html", 960, 640, 4.5f, 2.8f},
    {"porf-game-ui/index.html", 960, 640, 4.5f, 2.8f},
    {"porf-todo-classic/index.html", 960, 640, 4.5f, 2.8f},
    {"feature_test.html", 960, 640, 4.5f, 2.8f},
    {"pretext/test_text_flow.html", 960, 640, 4.5f, 2.8f},
    {"pretext/test_text_flow_dynamic.html", 960, 640, 4.5f, 2.8f},
    {"pretext/test_dual_mode.html", 960, 640, 4.5f, 2.8f},
};

static const FloorPanel k_forms[] = {
    {"tests/test_select_basic.html", 960, 640, 4.2f, 2.6f},
    {"tests/test_submit_event.html", 960, 640, 4.2f, 2.6f},
    {"tests/test_input_value.html", 960, 640, 4.2f, 2.6f},
};

static const FloorPanel k_layout[] = {
    {"tests/test_sticky_scroll_top.html", 960, 640, 4.2f, 2.6f},
    {"tests/test_display_contents_layout.html", 960, 640, 4.2f, 2.6f},
    {"tests/test_aspect_ratio_flex.html", 960, 640, 4.2f, 2.6f},
};

static const FloorPanel k_text[] = {
    {"tests/text_wrap_test.html", 960, 640, 4.2f, 2.6f},
    {"pretext/test_text_flow_directdraw.html", 960, 640, 4.2f, 2.6f},
    {"tests/test_colr_emoji.html", 960, 640, 4.2f, 2.6f},
    {"tests/test_colr_v0_emoji.html", 960, 640, 4.2f, 2.6f},
};

static const ZoneRow ZONE_ROWS[] = {
    {"Components / Porffor", (int)(sizeof(k_component_showcase) / sizeof(k_component_showcase[0])), k_component_showcase},
    {"Forms", (int)(sizeof(k_forms) / sizeof(k_forms[0])), k_forms},
    {"Layout / CSS", (int)(sizeof(k_layout) / sizeof(k_layout[0])), k_layout},
    {"Text / Emoji", (int)(sizeof(k_text) / sizeof(k_text[0])), k_text},
};

static const int NUM_ZONE_ROWS = (int)(sizeof(ZONE_ROWS) / sizeof(ZONE_ROWS[0]));

static int is_data_dir(const char* dir) {
    if (!dir || !dir[0]) return 0;
    char test[1024];
    snprintf(test, sizeof(test), "%s/screen1_script.html", dir);
    FILE* f = fopen(test, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    snprintf(test, sizeof(test), "%s/preact-counter/index.html", dir);
    f = fopen(test, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
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

#if defined(DONG_BACKEND_GPU) && defined(_WIN32)
static void get_exe_dir(char* out, size_t out_size) {
    if (!out || out_size == 0) return;
    out[0] = 0;
    DWORD n = GetModuleFileNameA(NULL, out, (DWORD)out_size);
    if (n == 0 || n >= out_size) return;
    for (int i = (int)n - 1; i >= 0; --i) {
        if (out[i] == '\\' || out[i] == '/') {
            out[i] = 0;
            return;
        }
    }
}
#endif

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
        for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
            try_set_data_path(candidates[i], path, sizeof(path));
        }

#if defined(DONG_BACKEND_GPU) && defined(_WIN32)
        {
            char base[1024];
            get_exe_dir(base, sizeof(base));
            if (base[0]) {
                try_set_data_path_from_base(base, "/data", path, sizeof(path));
                try_set_data_path_from_base(base, "/../data", path, sizeof(path));
                try_set_data_path_from_base(base, "/../examples/data", path, sizeof(path));
                try_set_data_path_from_base(base, "/../../examples/data", path, sizeof(path));
            }
        }
#else
        const char* base = SDL_GetBasePath();
        if (base) {
            try_set_data_path_from_base(base, "data", path, sizeof(path));
            try_set_data_path_from_base(base, "../data", path, sizeof(path));
            try_set_data_path_from_base(base, "../examples/data", path, sizeof(path));
            try_set_data_path_from_base(base, "../../examples/data", path, sizeof(path));
        }
#endif

        if (path[0] == 0) {
            strcpy(path, "data");
        }
    }
    return path;
}

static void ensure_script_timeout_env(void) {
    if (getenv("DONG_SCRIPT_TIMEOUT_MS") && getenv("DONG_SCRIPT_TIMEOUT_MS")[0]) {
        return;
    }
#if defined(_WIN32)
    _putenv_s("DONG_SCRIPT_TIMEOUT_MS", "10000");
#else
    setenv("DONG_SCRIPT_TIMEOUT_MS", "10000", 0);
#endif
}

// 物理 swapchain 路径 MSAA（Scene3D 在 scene3d.c 中解析到 swapchain）；未设置时可由此默认开启
static void ensure_scene3d_msaa_env(void) {
    if (getenv("DONG_SCENE3D_MSAA") && getenv("DONG_SCENE3D_MSAA")[0]) {
        return;
    }
#if defined(_WIN32)
    _putenv_s("DONG_SCENE3D_MSAA", "4");
#else
    setenv("DONG_SCENE3D_MSAA", "4", 0);
#endif
}

static const char* HUD_MODULE = "scene3d_hud";

static const char* PORFFOR_HUD_FALLBACK =
    "<html><body data-porffor-module='scene3d_hud' style='background:transparent;font-family:Arial;'>"
    "<div id='fps' style='position:absolute;top:12px;left:12px;color:#00ff88;font-size:18px;text-shadow:1px 1px 0 rgba(0,0,0,0.9);'>"
    "FPS: <span id='fps-value' style='color:#ffff00;font-weight:bold;background:rgba(0,0,0,0.7);padding:2px 6px;border-radius:3px;'>0</span>"
    "</div>"
    "<div id='help-panel' style='opacity:0;position:absolute;top:80px;left:12px;font-size:14px;color:white;background:rgba(0,0,0,0.75);padding:10px 14px;border-radius:4px;line-height:1.5;'>"
    "<b>Controls</b><br/>RMB+Mouse look<br/>WASD move<br/>Space/E up<br/>Ctrl/Q down<br/>Shift sprint<br/>ESC exit"
    "</div>"
    "<div id='help-hint' style='position:absolute;bottom:12px;left:12px;font-size:14px;color:white;background:rgba(0,0,0,0.5);padding:6px 12px;border-radius:4px;'>Press F1 or H for help</div>"
    "</body></html>";

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    ensure_script_timeout_env();
    ensure_scene3d_msaa_env();

    int total_floor = 0;
    for (int r = 0; r < NUM_ZONE_ROWS; r++) {
        total_floor += ZONE_ROWS[r].panel_count;
    }

    printf("=== Dong 3D Floor Gallery (Porffor + curated tests) ===\n");
#if defined(DONG_BACKEND_GPU)
    printf("[Dong] backend=gpu (native in-dreaming/gpu Scene3D)\n");
#endif
    printf("Zones: %d, floor panels: %d (+ %d title billboards)\n", NUM_ZONE_ROWS, total_floor, NUM_ZONE_ROWS);
    printf("Controls: RMB+Mouse=Look, WASD=Move, Space/E=Up, Ctrl/Q=Down, Shift=Sprint, F1/H=Help, ESC=Exit\n");

    dong_app_config_t config = {0};
    config.title = "Dong 3D Floor Gallery";
    config.width = 2000;
    config.height = 1000;
    config.enable_dong = 1;
    config.resizable = 1;

    dong_app_t* app = dong_app_create(&config);
    if (!app) {
        fprintf(stderr, "Failed to create application\n");
        return 1;
    }

    dong_scene3d_t* scene = dong_scene3d_create(app);
    if (!scene) {
        fprintf(stderr, "Failed to create 3D scene\n");
        dong_app_destroy(app);
        return 1;
    }
    g_scene = scene;

    dong_app_set_event_callback(app, on_app_event, NULL);
    dong_app_enable_text_input(app, 1);

    const char* data_path = get_data_path();
    dong_scene3d_set_resource_root(scene, data_path);

    const float floor_y = 1.2f;
    const float row_step_z = 7.5f;
    const float col_spacing = 5.8f;
    const float first_row_z = 1.0f;
    const float label_x = -14.0f;
    const float label_y = 3.5f;
    // Easel-style tilt: -60° from horizontal (30° from vertical, angled toward viewer)
    const float floor_pitch = -DONG_PI * 0.333f;

    int screen_index = 0;

    for (int row = 0; row < NUM_ZONE_ROWS; row++) {
        const ZoneRow* zr = &ZONE_ROWS[row];
        float row_z = first_row_z - (float)row * row_step_z;

        {
            char label_html[512];
            snprintf(label_html, sizeof(label_html),
                "<html><head><meta charset=\"utf-8\"/></head>"
                "<body style=\"margin:0;background:#1a1a2e;color:#e8e8f0;"
                "font:bold 128px sans-serif;display:flex;align-items:center;justify-content:center;height:100%%;\">"
                "%s</body></html>",
                zr->zone_title);

            dong_screen3d_config_t lcfg = {0};
            lcfg.html_content = label_html;
            lcfg.width = 1024;
            lcfg.height = 320;
            lcfg.pos_x = label_x;
            lcfg.pos_y = label_y;
            lcfg.pos_z = row_z;
            lcfg.yaw = 0.0f;
            lcfg.pitch = 0.0f;
            lcfg.screen_width = 3.2f;
            lcfg.screen_height = 0.85f;

            dong_screen3d_t* lab = dong_scene3d_add_screen(scene, &lcfg);
            if (lab) {
                printf("  [%d] ZONE \"%s\" label @ (%.1f,%.1f,%.1f) yaw=0 pitch=0\n",
                       screen_index++, zr->zone_title, label_x, label_y, row_z);
            } else {
                printf("[Scene3D] Warning: failed zone label \"%s\"\n", zr->zone_title);
            }
        }

        int n = zr->panel_count;
        float row_width = (n > 1) ? (float)(n - 1) * col_spacing : 0.0f;
        float start_x = -row_width * 0.5f;

        for (int i = 0; i < n; i++) {
            const FloorPanel* p = &zr->panels[i];
            dong_screen3d_config_t scfg = {0};
            scfg.html_file = p->html_file;
            scfg.width = p->rt_width;
            scfg.height = p->rt_height;
            scfg.pos_x = start_x + (float)i * col_spacing;
            scfg.pos_y = floor_y;
            scfg.pos_z = row_z;
            scfg.yaw = 0.0f;
            scfg.pitch = floor_pitch;
            scfg.screen_width = p->screen_width;
            scfg.screen_height = p->screen_height;

            dong_screen3d_t* scr = dong_scene3d_add_screen(scene, &scfg);
            if (scr) {
                printf("  [%d] floor \"%s\" [%s] @ (%.1f,%.1f,%.1f) yaw=0 pitch=-pi/2 w=%.2f h=%.2f\n",
                       screen_index++, zr->zone_title, p->html_file,
                       scfg.pos_x, scfg.pos_y, scfg.pos_z, p->screen_width, p->screen_height);
            } else {
                printf("[Scene3D] Warning: failed screen %s\n", p->html_file);
            }
        }
    }

    dong_scene3d_set_camera_position(scene, 0.0f, 5.0f, 16.0f);
    dong_scene3d_set_camera_rotation(scene, -DONG_PI, -0.35f);

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
                if (eng) {
                    dong_engine_set_text_renderer_mode(eng, mode);
                }
            }
            printf("[Scene3D] Text renderer: %s (from DONG_TEXT_RENDERER='%s')\n",
                   mode == DONG_TEXT_RENDERER_SLUG ? "Slug" :
                   mode == DONG_TEXT_RENDERER_MSDF ? "MSDF" : "Auto", tr_env);
        }
    }

    printf("Created 3D scene with %d screens (resource root: %s)\n", dong_scene3d_get_screen_count(scene), data_path);

    dong_overlay_t* hud = dong_scene3d_add_overlay_file(scene, "hud.html", 0, 0);
    if (!hud) {
        hud = dong_scene3d_add_overlay(scene, PORFFOR_HUD_FALLBACK, 0, 0);
    }
    if (hud) {
        printf("[HUD] Using Porffor HUD module: %s\n", HUD_MODULE);
        (void)dong_overlay_call_porffor_export1(hud, HUD_MODULE, "setFps", 0.0);
    }

    int frame_count = 0;
    float fps_timer = 0.0f;
    float fps = 0.0f;
    const char* aq_env = getenv("DONG_AUTO_QUIT");
    int auto_quit_frames = aq_env ? atoi(aq_env) : 0;
    int total_frames = 0;

    while (dong_app_is_running(app)) {
        if (!dong_app_poll_events(app)) {
            break;
        }

        float dt = dong_app_get_delta_time(app);

        if (g_help_requested) {
            g_help_requested = 0;
            if (!hud || !dong_overlay_call_porffor_export(hud, HUD_MODULE, "toggleHelp")) {
                printf("Controls: RMB+Mouse=Look, WASD=Move, Space/E=Up, Ctrl/Q=Down, Shift=Sprint, ESC=Exit\n");
            }
        }

        frame_count++;
        fps_timer += dt;
        if (fps_timer >= 1.0f) {
            fps = (float)frame_count / fps_timer;
            frame_count = 0;
            fps_timer = 0.0f;

            if (hud) {
                (void)dong_overlay_call_porffor_export1(hud, HUD_MODULE, "setFps", (double)fps);
            }
            printf("[FPS] %.1f\n", fps);
            fflush(stdout);
        }

        dong_scene3d_update(scene, dt);
        dong_scene3d_render(scene);

        if (auto_quit_frames > 0 && ++total_frames >= auto_quit_frames) {
            printf("[Scene3D] Auto-quit after %d frames (DONG_AUTO_QUIT)\n", total_frames);
            break;
        }
    }

    printf("Shutting down...\n");
    dong_scene3d_destroy(scene);
    dong_app_destroy(app);

    printf("=== 3D Floor Gallery Complete ===\n");
    return 0;
}
