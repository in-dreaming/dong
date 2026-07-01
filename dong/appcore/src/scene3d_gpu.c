#if defined(DONG_BACKEND_GPU)

#include "dong_scene3d.h"
#include "dong_math.h"
#include "dong.h"
#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "scene3d_world_gpu.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define MAX_SCREENS 64
#define MAX_PATH_LEN 1024
#define DONG_MB_LEFT 1
#define DONG_MB_RIGHT 3

#ifndef DONG_GPU_SHADER_DIR
#define DONG_GPU_SHADER_DIR "shaders"
#endif

typedef struct { dong_vec3_t origin; dong_vec3_t dir; } dong_ray_t;

struct dong_screen3d_t {
    dong_engine_t* engine;
    DongGPUTexture texture;
    float pos_x, pos_y, pos_z;
    float yaw;
    float pitch;
    float screen_width, screen_height;
    uint32_t tex_width, tex_height;
    int hovered;
    int focused;
    int32_t mouse_x;
    int32_t mouse_y;
    int dirty;
    int warmup_updates_remaining;
    char resource_root[MAX_PATH_LEN];
    char debug_name[256];
};

struct dong_scene3d_t {
    dong_app_t* app;
    void* device;
    void* surface;
    void* queue;
    DongGPUDriver* driver;
    Scene3DWorldGpu* world;
    uint32_t surface_fmt;
    dong_screen3d_t* screens[MAX_SCREENS];
    int screen_count;
    char resource_root[MAX_PATH_LEN];
    float cam_x, cam_y, cam_z;
    float cam_yaw, cam_pitch;
    float cam_speed, cam_sensitivity;
    int cam_controls_enabled;
    int key_w, key_a, key_s, key_d;
    int key_q, key_e;
    int key_space;
    int key_lctrl, key_rctrl;
    int key_lshift, key_rshift;
    int right_mouse_down;
    int32_t last_mouse_x;
    int32_t last_mouse_y;
    float bg_r, bg_g, bg_b, bg_a;
    int depth_test_enabled;
    int hovered_idx;
    int focused_idx;
    int pressed_idx;
    int32_t pressed_x;
    int32_t pressed_y;
    int last_sent_mouse_screen;
    int32_t last_sent_mouse_x;
    int32_t last_sent_mouse_y;
    int dirty;
    int initial_full_update_done;
    int bg_rr_cursor;
    double scheduler_start_time;
};

static double get_time_sec(void) {
#if defined(_WIN32)
    static LARGE_INTEGER freq = {0};
    static LARGE_INTEGER start = {0};
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
    }
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - start.QuadPart) / (double)freq.QuadPart;
#else
    return 0.0;
#endif
}

static void copy_string(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) return;
    if (!src) {
        dst[0] = 0;
        return;
    }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = 0;
}

static char* read_text_file(const char* path) {
    if (!path || !path[0]) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(f);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t n = fread(buf, 1, (size_t)sz, f);
    buf[n] = 0;
    fclose(f);
    return buf;
}

static void build_full_path(const char* root, const char* file, char* out, size_t out_size) {
    if (!out || out_size == 0) return;
    out[0] = 0;
    if (!file || !file[0]) return;
    if (root && root[0]) snprintf(out, out_size, "%s/%s", root, file);
    else copy_string(out, out_size, file);
}

static void extract_dir_from_path(const char* path, char* out_dir, size_t out_size) {
    if (!out_dir || out_size == 0) return;
    out_dir[0] = 0;
    if (!path || !path[0]) return;
    copy_string(out_dir, out_size, path);
    char* last_sep = strrchr(out_dir, '/');
    char* backslash = strrchr(out_dir, '\\');
    if (!last_sep || (backslash && backslash > last_sep)) last_sep = backslash;
    if (last_sep) *last_sep = 0;
    else out_dir[0] = 0;
}

static void set_screen_resource_root(dong_screen3d_t* screen, const char* root) {
    if (!screen || !screen->engine || !root || !root[0]) return;
    copy_string(screen->resource_root, sizeof(screen->resource_root), root);
    (void)dong_engine_set_resource_root(screen->engine, screen->resource_root);
}

static dong_engine_t* create_screen_engine(dong_scene3d_t* scene, uint32_t w, uint32_t h) {
    dong_engine_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.api_version = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.width = w;
    desc.height = h;
    dong_engine_t* engine = NULL;
    if (dong_engine_create(&desc, &engine) != DONG_OK || !engine) return NULL;
    if (dong_engine_set_gpu(engine, scene->device, dong_app_get_window(scene->app)) != DONG_OK) {
        dong_engine_destroy(engine);
        return NULL;
    }
    return engine;
}

static DongGPUTexture ensure_offscreen_texture(dong_scene3d_t* scene, dong_screen3d_t* scr) {
    if (!scene || !scene->driver || !scr) return NULL;
    if (scr->texture) return scr->texture;
    DongGPUTextureDesc desc = {0};
    desc.width = scr->tex_width;
    desc.height = scr->tex_height;
    desc.format = DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM;
    desc.usage = DONG_GPU_TEXTURE_USAGE_COLOR_TARGET | DONG_GPU_TEXTURE_USAGE_SAMPLER;
    scr->texture = dong_gpu_create_texture(scene->driver, &desc);
    return scr->texture;
}

static int update_screen_texture(dong_scene3d_t* scene, int idx) {
    dong_screen3d_t* scr = (scene && idx >= 0 && idx < scene->screen_count) ? scene->screens[idx] : NULL;
    if (!scr || !scr->engine || !scene || !scene->driver) return 0;
    DongGPUTexture target = ensure_offscreen_texture(scene, scr);
    if (!target) return 0;
    if (!dong_gpu_begin_frame_offscreen(scene->driver, target, scr->tex_width, scr->tex_height)) return 0;
    if (dong_engine_tick(scr->engine) != DONG_OK) {
        dong_gpu_end_frame_offscreen(scene->driver);
        return 0;
    }
    dong_gpu_end_frame_offscreen(scene->driver);
    scr->dirty = 0;
    return 1;
}

static void update_camera_key(dong_scene3d_t* scene, uint32_t key, int down) {
    if (!scene) return;
    const int v = down ? 1 : 0;
    if (key == 'w') scene->key_w = v;
    else if (key == 'a') scene->key_a = v;
    else if (key == 's') scene->key_s = v;
    else if (key == 'd') scene->key_d = v;
    else if (key == 'q') scene->key_q = v;
    else if (key == 'e') scene->key_e = v;
    else if (key == ' ') scene->key_space = v;
#if defined(_WIN32)
    else if (key == VK_LCONTROL || key == VK_RCONTROL) {
        scene->key_lctrl = (key == VK_LCONTROL) ? v : scene->key_lctrl;
        scene->key_rctrl = (key == VK_RCONTROL) ? v : scene->key_rctrl;
    } else if (key == VK_LSHIFT || key == VK_RSHIFT) {
        scene->key_lshift = (key == VK_LSHIFT) ? v : scene->key_lshift;
        scene->key_rshift = (key == VK_RSHIFT) ? v : scene->key_rshift;
    }
#endif
}

static void update_camera_controls(dong_scene3d_t* scene, float dt) {
    if (!scene || !scene->cam_controls_enabled) return;
    if (scene->right_mouse_down) {
        int32_t mx = 0, my = 0;
        dong_app_get_mouse_position(scene->app, &mx, &my);
        if (scene->last_mouse_x != (int32_t)0x80000000) {
            float dx = (float)(mx - scene->last_mouse_x);
            float dy = (float)(my - scene->last_mouse_y);
            scene->cam_yaw -= dx * scene->cam_sensitivity;
            scene->cam_pitch -= dy * scene->cam_sensitivity;
            if (scene->cam_pitch > 1.5f) scene->cam_pitch = 1.5f;
            if (scene->cam_pitch < -1.5f) scene->cam_pitch = -1.5f;
            scene->dirty = 1;
        }
        scene->last_mouse_x = mx;
        scene->last_mouse_y = my;
    } else {
        scene->last_mouse_x = (int32_t)0x80000000;
    }

    float spd = scene->cam_speed * dt;
    if (scene->key_lshift || scene->key_rshift) spd *= 2.0f;
    float fx = sinf(scene->cam_yaw), fz = cosf(scene->cam_yaw);
    float rx = -cosf(scene->cam_yaw), rz = sinf(scene->cam_yaw);
    if (scene->key_w) { scene->cam_x += fx * spd; scene->cam_z += fz * spd; scene->dirty = 1; }
    if (scene->key_s) { scene->cam_x -= fx * spd; scene->cam_z -= fz * spd; scene->dirty = 1; }
    if (scene->key_d) { scene->cam_x += rx * spd; scene->cam_z += rz * spd; scene->dirty = 1; }
    if (scene->key_a) { scene->cam_x -= rx * spd; scene->cam_z -= rz * spd; scene->dirty = 1; }
    if (scene->key_space || scene->key_e) { scene->cam_y += spd; scene->dirty = 1; }
    if (scene->key_lctrl || scene->key_rctrl || scene->key_q) { scene->cam_y -= spd; scene->dirty = 1; }
}

static dong_ray_t make_mouse_ray(const dong_scene3d_t* scene, int32_t mx, int32_t my, uint32_t w, uint32_t h) {
    dong_ray_t ray;
    ray.origin = dong_vec3(scene->cam_x, scene->cam_y, scene->cam_z);
    float ndc_x = (2.0f * (float)mx / (float)w) - 1.0f;
    float ndc_y = 1.0f - (2.0f * (float)my / (float)h);
    float aspect = (float)w / (float)h;
    dong_vec3_t eye = ray.origin;
    dong_vec3_t fwd = dong_vec3(cosf(scene->cam_pitch) * sinf(scene->cam_yaw), sinf(scene->cam_pitch),
                                cosf(scene->cam_pitch) * cosf(scene->cam_yaw));
    dong_vec3_t target = dong_vec3_add(eye, fwd);
    dong_mat4_t view = dong_mat4_look_at(eye, target, dong_vec3(0, 1, 0));
    dong_mat4_t proj = dong_mat4_perspective(DONG_DEG_TO_RAD(60.0f), aspect, 0.1f, 1000.0f);
    dong_mat4_t inv_vp = dong_mat4_inverse(dong_mat4_multiply(proj, view));
    dong_vec4_t clip_far = dong_vec4(ndc_x, ndc_y, 1.0f, 1.0f);
    dong_vec4_t far4 = dong_mat4_transform(inv_vp, clip_far);
    dong_vec3_t far_p = eye;
    if (fabsf(far4.w) > 1e-5f) far_p = dong_vec3(far4.x / far4.w, far4.y / far4.w, far4.z / far4.w);
    ray.dir = dong_vec3_normalize(dong_vec3_sub(far_p, eye));
    return ray;
}

static int ray_hit_screen(const dong_ray_t* ray, const dong_screen3d_t* scr, float* out_u, float* out_v) {
    const float sw = scr->screen_width, sh = scr->screen_height;
    const float hw = sw * 0.5f, hh = sh * 0.5f;
    dong_mat4_t R = dong_mat4_multiply(dong_mat4_rotate_y(scr->yaw), dong_mat4_rotate_x(scr->pitch));
    dong_mat4_t invR = dong_mat4_inverse(R);
    dong_vec3_t o1 = dong_vec3_sub(ray->origin, dong_vec3(scr->pos_x, scr->pos_y, scr->pos_z));
    dong_vec4_t w4 = dong_mat4_transform(invR, dong_vec4(o1.x, o1.y, o1.z, 1.0f));
    dong_vec4_t e4 = dong_mat4_transform(invR, dong_vec4(ray->dir.x, ray->dir.y, ray->dir.z, 0.0f));
    if (fabsf(e4.z) < 1e-5f) return 0;
    float t = -w4.z / e4.z;
    if (t < 0.0f) return 0;
    float px = w4.x + t * e4.x, py = w4.y + t * e4.y;
    float vx = px / sw, vy = py / sh;
    if (vx < -0.5f || vx > 0.5f || vy < -0.5f || vy > 0.5f) return 0;
    *out_u = (px + hw) / sw;
    *out_v = (py + hh) / sh;
    return 1;
}

static int pick_hovered_screen(const dong_scene3d_t* scene, const dong_ray_t* ray, float* out_u, float* out_v) {
    float best_u = 0, best_v = 0;
    int best = -1;
    for (int i = 0; i < scene->screen_count; i++) {
        dong_screen3d_t* scr = scene->screens[i];
        if (!scr) continue;
        float u, v;
        if (ray_hit_screen(ray, scr, &u, &v)) {
            best = i;
            best_u = u;
            best_v = v;
        }
    }
    if (best >= 0) {
        *out_u = best_u;
        *out_v = best_v;
    }
    return best;
}

static void send_mouse_move_to_screen(dong_scene3d_t* scene, int idx, int32_t x, int32_t y) {
    if (idx < 0 || idx >= scene->screen_count) return;
    dong_screen3d_t* scr = scene->screens[idx];
    if (!scr || !scr->engine) return;
    scr->mouse_x = x;
    scr->mouse_y = y;
    dong_engine_send_mouse_move(scr->engine, x, y);
    scr->dirty = 1;
}

static void update_hover_from_mouse(dong_scene3d_t* scene, int32_t mx, int32_t my, uint32_t w, uint32_t h) {
    if (!scene || w == 0 || h == 0) return;
    dong_ray_t ray = make_mouse_ray(scene, mx, my, w, h);
    float u = 0, v = 0;
    int hovered = pick_hovered_screen(scene, &ray, &u, &v);
    for (int i = 0; i < scene->screen_count; i++) {
        if (scene->screens[i]) scene->screens[i]->hovered = 0;
    }
    scene->hovered_idx = hovered;
    if (hovered < 0) return;
    dong_screen3d_t* scr = scene->screens[hovered];
    scr->hovered = 1;
    int32_t sx = (int32_t)(u * (float)scr->tex_width);
    int32_t sy = (int32_t)((1.0f - v) * (float)scr->tex_height);
    send_mouse_move_to_screen(scene, hovered, sx, sy);
}

DONG_APPCORE_API dong_scene3d_t* dong_scene3d_create(dong_app_t* app) {
    if (!app) return NULL;
    dong_scene3d_t* scene = (dong_scene3d_t*)calloc(1, sizeof(dong_scene3d_t));
    if (!scene) return NULL;
    scene->app = app;
    scene->device = dong_app_get_gpu_device(app);
    scene->surface = dong_app_get_gpu_surface(app);
    scene->queue = dong_app_get_gpu_queue(app);
    scene->surface_fmt = dong_app_get_gpu_surface_format(app);
    scene->driver = dong_platform_get_gpu_driver(dong_platform_get());
    if (!scene->device || !scene->surface || !scene->queue || !scene->driver) {
        free(scene);
        return NULL;
    }
    scene->cam_x = 0;
    scene->cam_y = 2.5f;
    scene->cam_z = 5;
    scene->cam_yaw = -DONG_PI;
    scene->cam_pitch = 0;
    scene->cam_speed = 5;
    scene->cam_sensitivity = 0.002f;
    scene->cam_controls_enabled = 1;
    scene->bg_r = 0.1f;
    scene->bg_g = 0.1f;
    scene->bg_b = 0.15f;
    scene->bg_a = 1.0f;
    scene->depth_test_enabled = 1;
    scene->hovered_idx = scene->focused_idx = scene->pressed_idx = -1;
    scene->last_mouse_x = (int32_t)0x80000000;
    scene->dirty = 1;
    scene->world = scene3d_world_gpu_create(scene->device, scene->surface_fmt, DONG_GPU_SHADER_DIR);
    if (!scene->world) {
        free(scene);
        return NULL;
    }
    printf("[Scene3D/GPU] Created\n");
    return scene;
}

DONG_APPCORE_API void dong_scene3d_destroy(dong_scene3d_t* scene) {
    if (!scene) return;
    for (int i = 0; i < scene->screen_count; i++) {
        dong_screen3d_t* scr = scene->screens[i];
        if (!scr) continue;
        if (scr->texture && scene->driver) dong_gpu_destroy_texture(scene->driver, scr->texture);
        if (scr->engine) dong_engine_destroy(scr->engine);
        free(scr);
    }
    if (scene->world) scene3d_world_gpu_destroy(scene->world);
    free(scene);
}

DONG_APPCORE_API dong_screen3d_t* dong_scene3d_add_screen(dong_scene3d_t* scene, const dong_screen3d_config_t* config) {
    if (!scene || !config || scene->screen_count >= MAX_SCREENS) return NULL;
    dong_screen3d_t* scr = (dong_screen3d_t*)calloc(1, sizeof(dong_screen3d_t));
    if (!scr) return NULL;
    scr->tex_width = config->width > 0 ? config->width : 800;
    scr->tex_height = config->height > 0 ? config->height : 600;
    scr->screen_width = config->screen_width > 0 ? config->screen_width : 4.0f;
    scr->screen_height = config->screen_height > 0 ? config->screen_height : 3.0f;
    scr->pos_x = config->pos_x;
    scr->pos_y = config->pos_y;
    scr->pos_z = config->pos_z;
    scr->yaw = config->yaw;
    scr->pitch = config->pitch;
    scr->dirty = 1;
    scr->engine = create_screen_engine(scene, scr->tex_width, scr->tex_height);
    if (!scr->engine) {
        free(scr);
        return NULL;
    }
    const char* base_root = config->resource_root ? config->resource_root : scene->resource_root;
    if (config->html_file) {
        char full[MAX_PATH_LEN];
        char file_dir[MAX_PATH_LEN];
        build_full_path(base_root, config->html_file, full, sizeof(full));
        extract_dir_from_path(full, file_dir, sizeof(file_dir));
        // Relative asset paths inside the HTML (e.g. "../images/bg.png") are resolved
        // against the per-screen engine's resource root, so it must point at the HTML
        // file's own directory (not just the scene-wide data root).
        if (file_dir[0]) {
            set_screen_resource_root(scr, file_dir);
        } else if (base_root && base_root[0]) {
            set_screen_resource_root(scr, base_root);
        }
        copy_string(scr->debug_name, sizeof(scr->debug_name), config->html_file);
        char* html = read_text_file(full);
        if (html) {
            dong_engine_load_html(scr->engine, html);
            free(html);
        }
    } else if (config->html_content) {
        if (base_root && base_root[0]) {
            set_screen_resource_root(scr, base_root);
        }
        dong_engine_load_html(scr->engine, config->html_content);
        copy_string(scr->debug_name, sizeof(scr->debug_name), "inline");
    }
    scene->screens[scene->screen_count++] = scr;
    scene->dirty = 1;
    return scr;
}

DONG_APPCORE_API dong_screen3d_t* dong_scene3d_add_screen_simple(dong_scene3d_t* scene, const char* html_file,
    uint32_t width, uint32_t height, float pos_x, float pos_y, float pos_z) {
    dong_screen3d_config_t cfg = {0};
    cfg.html_file = html_file;
    cfg.width = width;
    cfg.height = height;
    cfg.pos_x = pos_x;
    cfg.pos_y = pos_y;
    cfg.pos_z = pos_z;
    return dong_scene3d_add_screen(scene, &cfg);
}

DONG_APPCORE_API void dong_scene3d_remove_screen(dong_scene3d_t* scene, dong_screen3d_t* screen) {
    (void)scene;
    (void)screen;
}

DONG_APPCORE_API int dong_scene3d_get_screen_count(dong_scene3d_t* scene) { return scene ? scene->screen_count : 0; }
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_get_screen(dong_scene3d_t* scene, int idx) {
    return (scene && idx >= 0 && idx < scene->screen_count) ? scene->screens[idx] : NULL;
}

DONG_APPCORE_API void dong_screen3d_set_position(dong_screen3d_t* s, float x, float y, float z) {
    if (s) { s->pos_x = x; s->pos_y = y; s->pos_z = z; }
}
DONG_APPCORE_API void dong_screen3d_set_yaw(dong_screen3d_t* s, float yaw) { if (s) s->yaw = yaw; }
DONG_APPCORE_API void dong_screen3d_set_pitch(dong_screen3d_t* s, float pitch) { if (s) s->pitch = pitch; }
DONG_APPCORE_API int dong_screen3d_eval_script(dong_screen3d_t* s, const char* js) {
    return (s && s->engine && js && dong_engine_eval_script(s->engine, js) == DONG_OK) ? 1 : 0;
}
DONG_APPCORE_API void* dong_screen3d_get_view(dong_screen3d_t* s) { return s ? s->engine : NULL; }

DONG_APPCORE_API void dong_scene3d_get_camera_position(dong_scene3d_t* s, float* x, float* y, float* z) {
    if (s) { if (x) *x = s->cam_x; if (y) *y = s->cam_y; if (z) *z = s->cam_z; }
}
DONG_APPCORE_API void dong_scene3d_set_camera_position(dong_scene3d_t* s, float x, float y, float z) {
    if (s) { s->cam_x = x; s->cam_y = y; s->cam_z = z; s->dirty = 1; }
}
DONG_APPCORE_API void dong_scene3d_get_camera_rotation(dong_scene3d_t* s, float* yaw, float* pitch) {
    if (s) { if (yaw) *yaw = s->cam_yaw; if (pitch) *pitch = s->cam_pitch; }
}
DONG_APPCORE_API void dong_scene3d_set_camera_rotation(dong_scene3d_t* s, float yaw, float pitch) {
    if (s) { s->cam_yaw = yaw; s->cam_pitch = pitch; s->dirty = 1; }
}
DONG_APPCORE_API void dong_scene3d_set_camera_controls_enabled(dong_scene3d_t* s, int e) { if (s) s->cam_controls_enabled = e; }
DONG_APPCORE_API void dong_scene3d_set_camera_speed(dong_scene3d_t* s, float spd) { if (s) s->cam_speed = spd; }
DONG_APPCORE_API void dong_scene3d_set_camera_sensitivity(dong_scene3d_t* s, float sens) { if (s) s->cam_sensitivity = sens; }
DONG_APPCORE_API void dong_scene3d_set_background_color(dong_scene3d_t* s, float r, float g, float b, float a) {
    if (s) { s->bg_r = r; s->bg_g = g; s->bg_b = b; s->bg_a = a; }
}
DONG_APPCORE_API void dong_scene3d_set_depth_test_enabled(dong_scene3d_t* s, int e) { if (s) s->depth_test_enabled = e; }
DONG_APPCORE_API void dong_scene3d_set_resource_root(dong_scene3d_t* s, const char* root) {
    if (s && root) copy_string(s->resource_root, sizeof(s->resource_root), root);
}
DONG_APPCORE_API void dong_scene3d_arrange_screens(dong_scene3d_t* scene, float spacing_x, float spacing_y, int max_per_row) {
    (void)spacing_x;
    (void)spacing_y;
    (void)max_per_row;
    (void)scene;
}

DONG_APPCORE_API dong_overlay_t* dong_scene3d_add_overlay(dong_scene3d_t* scene, const char* html, uint32_t w, uint32_t h) {
    (void)scene; (void)html; (void)w; (void)h;
    return NULL;
}
DONG_APPCORE_API dong_overlay_t* dong_scene3d_add_overlay_file(dong_scene3d_t* scene, const char* html_file, uint32_t w, uint32_t h) {
    (void)scene; (void)html_file; (void)w; (void)h;
    return NULL;
}
DONG_APPCORE_API dong_overlay_t* dong_scene3d_get_overlay(dong_scene3d_t* scene, int idx) {
    (void)scene; (void)idx;
    return NULL;
}
DONG_APPCORE_API int dong_scene3d_get_overlay_count(dong_scene3d_t* scene) { (void)scene; return 0; }

DONG_APPCORE_API void dong_scene3d_process_event(dong_scene3d_t* scene, const dong_app_event_t* event) {
    if (!scene || !event) return;
    scene->dirty = 1;
    uint32_t w = 0, h = 0;
    dong_app_get_size(scene->app, &w, &h);
    switch (event->type) {
    case DONG_APP_EVENT_MOUSE_MOVE:
        if (!scene->right_mouse_down) update_hover_from_mouse(scene, event->mouse_move.x, event->mouse_move.y, w, h);
        break;
    case DONG_APP_EVENT_MOUSE_BUTTON:
        if (event->mouse_button.button == DONG_MB_RIGHT) {
            scene->right_mouse_down = event->mouse_button.pressed;
            scene->last_mouse_x = (int32_t)0x80000000;
            break;
        }
        if (event->mouse_button.button == DONG_MB_LEFT) {
            update_hover_from_mouse(scene, event->mouse_button.x, event->mouse_button.y, w, h);
            if (scene->hovered_idx >= 0 && scene->screens[scene->hovered_idx]) {
                dong_screen3d_t* scr = scene->screens[scene->hovered_idx];
                scene->focused_idx = scene->hovered_idx;
                send_mouse_move_to_screen(scene, scene->hovered_idx, scr->mouse_x, scr->mouse_y);
                dong_engine_send_mouse_button(scr->engine, DONG_MB_LEFT, event->mouse_button.pressed);
                scr->dirty = 1;
            }
        }
        break;
    case DONG_APP_EVENT_MOUSE_WHEEL:
        if (scene->hovered_idx >= 0 && scene->screens[scene->hovered_idx]) {
            dong_screen3d_t* scr = scene->screens[scene->hovered_idx];
            dong_engine_send_mouse_wheel(scr->engine, event->mouse_wheel.delta_x * 3.0f, event->mouse_wheel.delta_y * 3.0f);
            scr->dirty = 1;
        }
        break;
    case DONG_APP_EVENT_KEY:
        update_camera_key(scene, event->key.key_code, event->key.pressed);
        if (scene->focused_idx >= 0 && scene->screens[scene->focused_idx]) {
            dong_engine_send_key(scene->screens[scene->focused_idx]->engine, event->key.key_code, event->key.pressed);
            scene->screens[scene->focused_idx]->dirty = 1;
        }
        break;
    case DONG_APP_EVENT_TEXT:
        if (scene->focused_idx >= 0 && scene->screens[scene->focused_idx] && event->text.text) {
            dong_engine_send_text(scene->screens[scene->focused_idx]->engine, event->text.text);
            scene->screens[scene->focused_idx]->dirty = 1;
        }
        break;
    default:
        break;
    }
}

DONG_APPCORE_API dong_screen3d_t* dong_scene3d_handle_input(dong_scene3d_t* scene) {
    (void)scene;
    return NULL;
}

DONG_APPCORE_API void dong_scene3d_update(dong_scene3d_t* scene, float dt) {
    if (!scene) return;
    update_camera_controls(scene, dt);
    if (!scene->initial_full_update_done) {
        for (int i = 0; i < scene->screen_count; i++) update_screen_texture(scene, i);
        scene->initial_full_update_done = 1;
    } else {
        if (scene->hovered_idx >= 0 && scene->screens[scene->hovered_idx] &&
            scene->screens[scene->hovered_idx]->dirty) {
            update_screen_texture(scene, scene->hovered_idx);
        }
        int idx = scene->bg_rr_cursor % (scene->screen_count > 0 ? scene->screen_count : 1);
        if (scene->screen_count > 0 && scene->screens[idx] && scene->screens[idx]->dirty) {
            update_screen_texture(scene, idx);
        }
        scene->bg_rr_cursor++;
    }
}

DONG_APPCORE_API void dong_scene3d_render(dong_scene3d_t* scene) {
    if (!scene || !scene->world) return;
    if (scene->initial_full_update_done && !scene->dirty) return;

    uint32_t sw = 0, sh = 0;
    dong_app_get_size(scene->app, &sw, &sh);
    float aspect = (float)sw / (float)sh;
    dong_vec3_t eye = dong_vec3(scene->cam_x, scene->cam_y, scene->cam_z);
    dong_vec3_t fwd = dong_vec3(cosf(scene->cam_pitch) * sinf(scene->cam_yaw), sinf(scene->cam_pitch),
                                cosf(scene->cam_pitch) * cosf(scene->cam_yaw));
    dong_mat4_t view = dong_mat4_look_at(eye, dong_vec3_add(eye, fwd), dong_vec3(0, 1, 0));
    dong_mat4_t proj = dong_mat4_perspective(DONG_DEG_TO_RAD(60.0f), aspect, 0.1f, 1000.0f);
    dong_mat4_t vp = dong_mat4_multiply(proj, view);

    Scene3DWorldScreenDraw draws[MAX_SCREENS];
    int draw_count = 0;
    for (int i = 0; i < scene->screen_count; i++) {
        dong_screen3d_t* scr = scene->screens[i];
        if (!scr || !scr->texture) continue;
        dong_mat4_t scale = dong_mat4_scale(scr->screen_width, scr->screen_height, 1.0f);
        dong_mat4_t rotate = dong_mat4_multiply(dong_mat4_rotate_y(scr->yaw), dong_mat4_rotate_x(scr->pitch));
        dong_mat4_t translate = dong_mat4_translate(scr->pos_x, scr->pos_y, scr->pos_z);
        dong_mat4_t model = dong_mat4_multiply(translate, dong_mat4_multiply(rotate, scale));
        dong_mat4_t mvp = dong_mat4_multiply(vp, model);
        Scene3DWorldScreenDraw* d = &draws[draw_count++];
        d->texture = scr->texture;
        memcpy(d->mvp, mvp.m, sizeof(d->mvp));
        memcpy(d->model, model.m, sizeof(d->model));
        d->highlighted = scr->hovered;
    }

    Scene3DWorldFrameDesc frame = {0};
    frame.driver = scene->driver;
    frame.device = scene->device;
    frame.queue = scene->queue;
    frame.surface = scene->surface;
    frame.width = sw;
    frame.height = sh;
    frame.bg_r = scene->bg_r;
    frame.bg_g = scene->bg_g;
    frame.bg_b = scene->bg_b;
    frame.bg_a = scene->bg_a;
    frame.depth_test = scene->depth_test_enabled;
    frame.screen_count = draw_count;
    frame.screens = draws;
    if (scene3d_world_gpu_render(scene->world, &frame)) {
        scene->dirty = 0;
    }
}

#endif
