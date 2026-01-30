/**
 * Scene3D Implementation - 3D scene with HTML screens
 */

#include "dong_scene3d.h"
#include "dong_math.h"
#include "dong.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MAX_SCREENS 64
#define MAX_OVERLAYS 8
#define MAX_PATH_LEN 1024

// Uniform structures (must match shader)
typedef struct {
    float mvp[16];
    float model[16];
    float color[4];
    float highlight[4];
} UniformsTextured;

// Internal structures
struct dong_screen3d_t {
    dong_view_t* view;
    SDL_GPUTexture* texture;
    float pos_x, pos_y, pos_z;
    float yaw;
    float screen_width, screen_height;
    uint32_t tex_width, tex_height;
    int hovered, focused;
    char debug_name[256];
};

// dong_overlay_t is defined in overlay.c

struct dong_scene3d_t {
    dong_app_t* app;
    dong_context_t* ctx;
    SDL_GPUDevice* device;
    SDL_Window* window;

    dong_screen3d_t* screens[MAX_SCREENS];
    int screen_count;

    dong_overlay_t* overlays[MAX_OVERLAYS];
    int overlay_count;

    // Resource root for HTML file resolution
    char resource_root[MAX_PATH_LEN];

    // Camera
    float cam_x, cam_y, cam_z;
    float cam_yaw, cam_pitch;
    float cam_speed, cam_sensitivity;
    int cam_controls_enabled;

    // Background
    float bg_r, bg_g, bg_b, bg_a;
    int depth_test_enabled;

    // GPU resources for 3D screens
    SDL_GPUGraphicsPipeline* screen_pipeline;
    SDL_GPUSampler* sampler;
    SDL_GPUBuffer* quad_vb;
    SDL_GPUTexture* depth_texture;
    uint32_t depth_width, depth_height;

    // GPU resources for HUD overlay
    SDL_GPUGraphicsPipeline* hud_pipeline;
    SDL_GPUBuffer* hud_quad_vb;
};

// Forward declarations
static int create_pipelines(dong_scene3d_t* scene);
static void destroy_pipelines(dong_scene3d_t* scene);

// Shaders
static const char* VS_TEXTURED =
    "struct VSInput { float3 position : TEXCOORD0; float2 uv : TEXCOORD1; };\n"
    "struct VSOutput { float4 position : SV_Position; float2 uv : TEXCOORD0; };\n"
    "cbuffer Uniforms : register(b0, space1) {\n"
    "    float4x4 uMVP; float4x4 uModel; float4 uColor; float4 uHighlight;\n"
    "};\n"
    "VSOutput main(VSInput input) {\n"
    "    VSOutput o; o.position = mul(uMVP, float4(input.position, 1.0));\n"
    "    o.uv = input.uv; return o;\n"
    "}\n";

static const char* FS_TEXTURED =
    "Texture2D tex : register(t0, space2);\n"
    "SamplerState texSampler : register(s0, space2);\n"
    "cbuffer Uniforms : register(b0, space3) {\n"
    "    float4x4 uMVP; float4x4 uModel; float4 uColor; float4 uHighlight;\n"
    "};\n"
    "struct PSInput { float4 position : SV_Position; float2 uv : TEXCOORD0; };\n"
    "float4 main(PSInput input) : SV_Target0 {\n"
    "    float4 c = tex.Sample(texSampler, input.uv) * uColor;\n"
    "    if (uHighlight.x > 0.5) {\n"
    "        float b = 0.02;\n"
    "        if (input.uv.x < b || input.uv.x > 1.0-b || input.uv.y < b || input.uv.y > 1.0-b)\n"
    "            c.rgb = lerp(c.rgb, float3(0.3,0.6,1.0), 0.7);\n"
    "    }\n"
    "    return c;\n"
    "}\n";

// HUD shaders (fullscreen quad, no depth, alpha blending with white background keying)
static const char* VS_HUD =
    "struct VSInput { float2 position : TEXCOORD0; float2 uv : TEXCOORD1; };\n"
    "struct VSOutput { float4 position : SV_Position; float2 uv : TEXCOORD0; };\n"
    "VSOutput main(VSInput input) {\n"
    "    VSOutput o;\n"
    "    o.position = float4(input.position, 0.0, 1.0);\n"
    "    o.uv = input.uv;\n"
    "    return o;\n"
    "}\n";

static const char* FS_HUD =
    "Texture2D tex : register(t0, space2);\n"
    "SamplerState texSampler : register(s0, space2);\n"
    "cbuffer Uniforms : register(b0, space3) { float4 uColor; };\n"
    "struct PSInput { float4 position : SV_Position; float2 uv : TEXCOORD0; };\n"
    "float4 main(PSInput input) : SV_Target0 {\n"
    "    float4 c = tex.Sample(texSampler, input.uv);\n"
    "    float rgbEnergy = dot(c.rgb, float3(0.3333, 0.3333, 0.3333));\n"
    "    // Case 1: Low alpha + bright RGB = transparent background\n"
    "    if (c.a < 0.1 && rgbEnergy > 0.7) {\n"
    "        c = float4(0.0, 0.0, 0.0, 0.0);\n"
    "    }\n"
    "    // Case 2: Near-white with high alpha but no saturation (gray/white background)\n"
    "    else if (c.a > 0.9 && rgbEnergy > 0.95 && \n"
    "             abs(c.r - c.g) < 0.05 && abs(c.g - c.b) < 0.05) {\n"
    "        c = float4(0.0, 0.0, 0.0, 0.0);\n"
    "    }\n"
    "    // Case 3: Promote alpha for actual colored content (not grayscale background)\n"
    "    else if (c.a <= 0.001 && rgbEnergy > 0.01 && rgbEnergy <= 0.7) {\n"
    "        c.a = 1.0;\n"
    "    }\n"
    "    float4 outc = c * uColor;\n"
    "    // Premultiply alpha for correct blending\n"
    "    outc.rgb *= outc.a;\n"
    "    // Final safety: clamp RGB to 0 for fully transparent pixels\n"
    "    if (outc.a <= 0.001) {\n"
    "        outc.rgb = float3(0.0, 0.0, 0.0);\n"
    "    }\n"
    "    return outc;\n"
    "}\n";

// Scene lifecycle
DONG_APPCORE_API dong_scene3d_t* dong_scene3d_create(dong_app_t* app) {
    if (!app) return NULL;

    dong_scene3d_t* scene = (dong_scene3d_t*)calloc(1, sizeof(dong_scene3d_t));
    if (!scene) return NULL;

    scene->app = app;
    scene->device = (SDL_GPUDevice*)dong_app_get_gpu_device(app);
    scene->window = (SDL_Window*)dong_app_get_window(app);
    if (!scene->device || !scene->window) { free(scene); return NULL; }

    scene->ctx = dong_create_context();
    if (!scene->ctx) { free(scene); return NULL; }

    // Defaults
    scene->cam_x = 0; scene->cam_y = 2.5f; scene->cam_z = 5;
    scene->cam_yaw = -DONG_PI; scene->cam_pitch = 0;
    scene->cam_speed = 5; scene->cam_sensitivity = 0.002f;
    scene->cam_controls_enabled = 1;
    scene->bg_r = 0.1f; scene->bg_g = 0.1f; scene->bg_b = 0.15f; scene->bg_a = 1;
    scene->depth_test_enabled = 1;

    if (!create_pipelines(scene)) {
        dong_destroy_context(scene->ctx);
        free(scene);
        return NULL;
    }

    printf("[Scene3D] Created\n");
    return scene;
}

DONG_APPCORE_API void dong_scene3d_destroy(dong_scene3d_t* scene) {
    if (!scene) return;
    for (int i = 0; i < scene->screen_count; i++) {
        if (scene->screens[i]) {
            if (scene->screens[i]->view) dong_view_free(scene->screens[i]->view);
            free(scene->screens[i]);
        }
    }
    for (int i = 0; i < scene->overlay_count; i++) {
        if (scene->overlays[i]) {
            dong_overlay_destroy(scene->overlays[i]);
        }
    }
    destroy_pipelines(scene);
    if (scene->ctx) dong_destroy_context(scene->ctx);
    free(scene);
}

// Screen management
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_add_screen(dong_scene3d_t* scene, const dong_screen3d_config_t* config) {
    if (!scene || !config || scene->screen_count >= MAX_SCREENS) return NULL;

    dong_screen3d_t* screen = (dong_screen3d_t*)calloc(1, sizeof(dong_screen3d_t));
    if (!screen) return NULL;

    screen->tex_width = config->width > 0 ? config->width : 800;
    screen->tex_height = config->height > 0 ? config->height : 600;
    screen->screen_width = config->screen_width > 0 ? config->screen_width : 4.0f;
    screen->screen_height = config->screen_height > 0 ? config->screen_height : 3.0f;
    screen->pos_x = config->pos_x;
    screen->pos_y = config->pos_y;
    screen->pos_z = config->pos_z;
    screen->yaw = config->yaw;

    screen->view = dong_view_create(scene->ctx, screen->tex_width, screen->tex_height);
    if (!screen->view) { free(screen); return NULL; }

    dong_view_set_external_gpu_device(screen->view, scene->device, scene->window);

    // Determine resource root
    const char* res_root = config->resource_root;
    if (!res_root && scene->resource_root[0]) {
        res_root = scene->resource_root;
    }

    if (config->html_file) {
        // Build full path
        char full_path[MAX_PATH_LEN];
        if (res_root && res_root[0]) {
            snprintf(full_path, sizeof(full_path), "%s/%s", res_root, config->html_file);
        } else {
            strncpy(full_path, config->html_file, sizeof(full_path) - 1);
            full_path[sizeof(full_path) - 1] = 0;
        }

        // Set debug name
        strncpy(screen->debug_name, config->html_file, sizeof(screen->debug_name) - 1);
        dong_view_set_debug_name(screen->view, config->html_file);

        // Extract directory from full_path for resource root
        char file_dir[MAX_PATH_LEN];
        strncpy(file_dir, full_path, sizeof(file_dir) - 1);
        file_dir[sizeof(file_dir) - 1] = 0;
        char* last_sep = strrchr(file_dir, '/');
        if (!last_sep) last_sep = strrchr(file_dir, '\\');
        if (last_sep) *last_sep = 0;
        else file_dir[0] = 0;
        if (file_dir[0]) dong_view_set_resource_root(screen->view, file_dir);

        FILE* f = fopen(full_path, "rb");
        if (f) {
            fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
            char* buf = (char*)malloc(sz + 1);
            if (buf) {
                fread(buf, 1, sz, f);
                buf[sz] = 0;
                dong_view_load_html(screen->view, buf);
                free(buf);
            }
            fclose(f);
            printf("[Scene3D] Loaded screen: %s (%ux%u)\n", full_path, screen->tex_width, screen->tex_height);
        } else {
            printf("[Scene3D] Warning: Failed to open %s\n", full_path);
        }
    } else if (config->html_content) {
        dong_view_load_html(screen->view, config->html_content);
        strncpy(screen->debug_name, "inline", sizeof(screen->debug_name) - 1);
    }

    scene->screens[scene->screen_count++] = screen;
    return screen;
}

DONG_APPCORE_API dong_screen3d_t* dong_scene3d_add_screen_simple(dong_scene3d_t* scene, const char* html_file,
    uint32_t width, uint32_t height, float pos_x, float pos_y, float pos_z) {
    dong_screen3d_config_t cfg = {0};
    cfg.html_file = html_file;
    cfg.width = width; cfg.height = height;
    cfg.pos_x = pos_x; cfg.pos_y = pos_y; cfg.pos_z = pos_z;
    return dong_scene3d_add_screen(scene, &cfg);
}

DONG_APPCORE_API void dong_scene3d_remove_screen(dong_scene3d_t* scene, dong_screen3d_t* screen) {
    if (!scene || !screen) return;
    for (int i = 0; i < scene->screen_count; i++) {
        if (scene->screens[i] == screen) {
            if (screen->view) dong_view_free(screen->view);
            free(screen);
            for (int j = i; j < scene->screen_count - 1; j++) scene->screens[j] = scene->screens[j+1];
            scene->screen_count--;
            break;
        }
    }
}

DONG_APPCORE_API int dong_scene3d_get_screen_count(dong_scene3d_t* scene) { return scene ? scene->screen_count : 0; }
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_get_screen(dong_scene3d_t* scene, int idx) {
    return (scene && idx >= 0 && idx < scene->screen_count) ? scene->screens[idx] : NULL;
}

// Screen properties
DONG_APPCORE_API void dong_screen3d_set_position(dong_screen3d_t* s, float x, float y, float z) {
    if (s) { s->pos_x = x; s->pos_y = y; s->pos_z = z; }
}
DONG_APPCORE_API void dong_screen3d_set_yaw(dong_screen3d_t* s, float yaw) { if (s) s->yaw = yaw; }
DONG_APPCORE_API int dong_screen3d_eval_script(dong_screen3d_t* s, const char* js) {
    return (s && s->view && js) ? dong_view_eval(s->view, js) : 0;
}
DONG_APPCORE_API void* dong_screen3d_get_view(dong_screen3d_t* s) { return s ? s->view : NULL; }

// Camera
DONG_APPCORE_API void dong_scene3d_get_camera_position(dong_scene3d_t* s, float* x, float* y, float* z) {
    if (s) { if (x) *x = s->cam_x; if (y) *y = s->cam_y; if (z) *z = s->cam_z; }
}
DONG_APPCORE_API void dong_scene3d_set_camera_position(dong_scene3d_t* s, float x, float y, float z) {
    if (s) { s->cam_x = x; s->cam_y = y; s->cam_z = z; }
}
DONG_APPCORE_API void dong_scene3d_get_camera_rotation(dong_scene3d_t* s, float* yaw, float* pitch) {
    if (s) { if (yaw) *yaw = s->cam_yaw; if (pitch) *pitch = s->cam_pitch; }
}
DONG_APPCORE_API void dong_scene3d_set_camera_rotation(dong_scene3d_t* s, float yaw, float pitch) {
    if (s) { s->cam_yaw = yaw; s->cam_pitch = pitch; }
}
DONG_APPCORE_API void dong_scene3d_set_camera_controls_enabled(dong_scene3d_t* s, int e) { if (s) s->cam_controls_enabled = e; }
DONG_APPCORE_API void dong_scene3d_set_camera_speed(dong_scene3d_t* s, float spd) { if (s) s->cam_speed = spd; }
DONG_APPCORE_API void dong_scene3d_set_camera_sensitivity(dong_scene3d_t* s, float sens) { if (s) s->cam_sensitivity = sens; }

// Config
DONG_APPCORE_API void dong_scene3d_set_background_color(dong_scene3d_t* s, float r, float g, float b, float a) {
    if (s) { s->bg_r = r; s->bg_g = g; s->bg_b = b; s->bg_a = a; }
}
DONG_APPCORE_API void dong_scene3d_set_depth_test_enabled(dong_scene3d_t* s, int e) { if (s) s->depth_test_enabled = e; }

DONG_APPCORE_API void dong_scene3d_set_resource_root(dong_scene3d_t* s, const char* root) {
    if (s && root) {
        strncpy(s->resource_root, root, MAX_PATH_LEN - 1);
        s->resource_root[MAX_PATH_LEN - 1] = 0;
    }
}

// Automatic screen arrangement (replicates 3d_screen_script logic)
DONG_APPCORE_API void dong_scene3d_arrange_screens(dong_scene3d_t* scene, float spacing_x, float spacing_y, int max_per_row) {
    if (!scene || scene->screen_count <= 0) return;

    if (spacing_x <= 0) spacing_x = 4.0f;
    if (spacing_y <= 0) spacing_y = 2.8f;
    if (max_per_row <= 0) max_per_row = 5;

    int total_rows = (scene->screen_count + max_per_row - 1) / max_per_row;
    float base_y = 2.5f + 3.0f - (total_rows - 1) * spacing_y * 0.5f;

    for (int i = 0; i < scene->screen_count; i++) {
        dong_screen3d_t* scr = scene->screens[i];
        if (!scr) continue;

        int row = i / max_per_row;
        int col = i % max_per_row;
        int screens_in_row = max_per_row;
        if (scene->screen_count - row * max_per_row < max_per_row) {
            screens_in_row = scene->screen_count - row * max_per_row;
        }

        float row_width = (screens_in_row - 1) * spacing_x;
        float start_x = -row_width * 0.5f;

        scr->pos_x = start_x + col * spacing_x;
        scr->pos_y = base_y + row * spacing_y;
        scr->pos_z = -3.0f;

        // Slight yaw toward center
        float delta_x = scr->pos_x;
        scr->yaw = -delta_x * 0.08f;
        if (scr->yaw > 0.4f) scr->yaw = 0.4f;
        if (scr->yaw < -0.4f) scr->yaw = -0.4f;
    }

    printf("[Scene3D] Arranged %d screens in %d rows\n", scene->screen_count, total_rows);
}

// Overlay management
DONG_APPCORE_API dong_overlay_t* dong_scene3d_add_overlay(dong_scene3d_t* scene, const char* html, uint32_t w, uint32_t h) {
    if (!scene || scene->overlay_count >= MAX_OVERLAYS) return NULL;

    // Get window size if not specified
    if (w == 0 || h == 0) {
        uint32_t ww, wh;
        dong_app_get_size(scene->app, &ww, &wh);
        if (w == 0) w = ww;
        if (h == 0) h = wh;
    }

    dong_overlay_t* overlay = dong_overlay_create(scene->app, w, h);
    if (!overlay) return NULL;

    // Set resource root if available
    if (scene->resource_root[0]) {
        dong_overlay_set_resource_root(overlay, scene->resource_root);
    }

    // Load HTML content
    if (html) {
        dong_overlay_load_html(overlay, html);
    }

    scene->overlays[scene->overlay_count++] = overlay;
    printf("[Scene3D] Added overlay %d (%ux%u)\n", scene->overlay_count - 1, w, h);
    return overlay;
}

DONG_APPCORE_API dong_overlay_t* dong_scene3d_add_overlay_file(dong_scene3d_t* scene, const char* html_file, uint32_t w, uint32_t h) {
    if (!scene || !html_file || scene->overlay_count >= MAX_OVERLAYS) return NULL;

    // Get window size if not specified
    if (w == 0 || h == 0) {
        uint32_t ww, wh;
        dong_app_get_size(scene->app, &ww, &wh);
        if (w == 0) w = ww;
        if (h == 0) h = wh;
    }

    dong_overlay_t* overlay = dong_overlay_create(scene->app, w, h);
    if (!overlay) return NULL;

    // Set resource root if available
    if (scene->resource_root[0]) {
        dong_overlay_set_resource_root(overlay, scene->resource_root);
    }

    // Build full path
    char full_path[MAX_PATH_LEN];
    if (scene->resource_root[0]) {
        snprintf(full_path, sizeof(full_path), "%s/%s", scene->resource_root, html_file);
    } else {
        strncpy(full_path, html_file, sizeof(full_path) - 1);
        full_path[sizeof(full_path) - 1] = 0;
    }

    // Load HTML from file
    FILE* f = fopen(full_path, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* buf = (char*)malloc(sz + 1);
        if (buf) {
            fread(buf, 1, sz, f);
            buf[sz] = 0;
            dong_overlay_load_html(overlay, buf);
            free(buf);
            printf("[Scene3D] Loaded overlay from %s (%ux%u)\n", full_path, w, h);
        }
        fclose(f);
    } else {
        printf("[Scene3D] Warning: Failed to open overlay file %s\n", full_path);
        dong_overlay_destroy(overlay);
        return NULL;
    }

    scene->overlays[scene->overlay_count++] = overlay;
    return overlay;
}

DONG_APPCORE_API dong_overlay_t* dong_scene3d_get_overlay(dong_scene3d_t* scene, int idx) {
    return (scene && idx >= 0 && idx < scene->overlay_count) ? scene->overlays[idx] : NULL;
}

DONG_APPCORE_API int dong_scene3d_get_overlay_count(dong_scene3d_t* scene) {
    return scene ? scene->overlay_count : 0;
}

// Input handling
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_handle_input(dong_scene3d_t* scene) {
    (void)scene; return NULL; // TODO: ray casting
}

// Update
DONG_APPCORE_API void dong_scene3d_update(dong_scene3d_t* scene, float dt) {
    if (!scene) return;

    // Camera controls
    if (scene->cam_controls_enabled) {
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        Uint32 mouse = SDL_GetMouseState(NULL, NULL);
        if (mouse & SDL_BUTTON_RMASK) {
            float dx, dy;
            SDL_GetRelativeMouseState(&dx, &dy);
            scene->cam_yaw -= dx * scene->cam_sensitivity;
            scene->cam_pitch -= dy * scene->cam_sensitivity;
            if (scene->cam_pitch > 1.5f) scene->cam_pitch = 1.5f;
            if (scene->cam_pitch < -1.5f) scene->cam_pitch = -1.5f;
        }
        float spd = scene->cam_speed * dt;
        if (keys[SDL_SCANCODE_LSHIFT]) spd *= 2;
        float fx = sinf(scene->cam_yaw), fz = cosf(scene->cam_yaw);
        float rx = -cosf(scene->cam_yaw), rz = sinf(scene->cam_yaw);
        if (keys[SDL_SCANCODE_W]) { scene->cam_x += fx * spd; scene->cam_z += fz * spd; }
        if (keys[SDL_SCANCODE_S]) { scene->cam_x -= fx * spd; scene->cam_z -= fz * spd; }
        if (keys[SDL_SCANCODE_D]) { scene->cam_x += rx * spd; scene->cam_z += rz * spd; }
        if (keys[SDL_SCANCODE_A]) { scene->cam_x -= rx * spd; scene->cam_z -= rz * spd; }
        if (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_E]) scene->cam_y += spd;
        if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_Q]) scene->cam_y -= spd;
    }

    // Update screen textures
    for (int i = 0; i < scene->screen_count; i++) {
        dong_screen3d_t* scr = scene->screens[i];
        if (scr && scr->view) {
            // NOTE: Do NOT call dong_view_update() here! It renders to swapchain.
            // dong_view_render_to_gpu_texture() handles layout calculation internally.

            // Render to GPU texture (texture is managed internally by View)
            SDL_GPUTexture* new_tex = (SDL_GPUTexture*)dong_view_render_to_gpu_texture(
                scr->view, scene->device, scr->tex_width, scr->tex_height);

            // Only update texture if we got a valid one (keep old texture if render fails)
            if (new_tex) {
                scr->texture = new_tex;
            }
        }
    }

    // Update overlays (use overlay API functions)
    for (int i = 0; i < scene->overlay_count; i++) {
        dong_overlay_t* ov = scene->overlays[i];
        if (ov) {
            dong_overlay_update(ov, dt);
        }
    }
}

// Render
DONG_APPCORE_API void dong_scene3d_render(dong_scene3d_t* scene) {
    if (!scene || !scene->device || !scene->window) return;

    uint32_t sw, sh;
    dong_app_get_size(scene->app, &sw, &sh);
    if (sw == 0 || sh == 0) return;

    // Ensure depth texture
    if (!scene->depth_texture || scene->depth_width != sw || scene->depth_height != sh) {
        if (scene->depth_texture) SDL_ReleaseGPUTexture(scene->device, scene->depth_texture);
        SDL_GPUTextureCreateInfo dti = {0};
        dti.type = SDL_GPU_TEXTURETYPE_2D;
        dti.format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
        dti.width = sw; dti.height = sh;
        dti.layer_count_or_depth = 1; dti.num_levels = 1;
        dti.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        scene->depth_texture = SDL_CreateGPUTexture(scene->device, &dti);
        scene->depth_width = sw; scene->depth_height = sh;
    }

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(scene->device);
    if (!cmd) return;

    SDL_GPUTexture* swapchain = NULL;
    Uint32 swapchain_w = 0, swapchain_h = 0;
    // Use Wait version to ensure proper synchronization with offscreen renders
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, scene->window, &swapchain, &swapchain_w, &swapchain_h)) {
        SDL_CancelGPUCommandBuffer(cmd);
        return;
    }
    if (!swapchain) {
        // Swapchain not available this frame, submit empty command buffer
        SDL_SubmitGPUCommandBuffer(cmd);
        return;
    }

    if (swapchain && scene->screen_pipeline) {
        SDL_GPUColorTargetInfo cti = {0};
        cti.texture = swapchain;
        cti.load_op = SDL_GPU_LOADOP_CLEAR;
        cti.store_op = SDL_GPU_STOREOP_STORE;
        cti.clear_color.r = scene->bg_r;
        cti.clear_color.g = scene->bg_g;
        cti.clear_color.b = scene->bg_b;
        cti.clear_color.a = scene->bg_a;

        SDL_GPUDepthStencilTargetInfo dsti = {0};
        dsti.texture = scene->depth_texture;
        dsti.load_op = SDL_GPU_LOADOP_CLEAR;
        dsti.store_op = SDL_GPU_STOREOP_DONT_CARE;
        dsti.clear_depth = 1.0f;

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &cti, 1, scene->depth_test_enabled ? &dsti : NULL);
        if (pass) {
            SDL_BindGPUGraphicsPipeline(pass, scene->screen_pipeline);

            // Build VP matrix
            float aspect = (float)sw / (float)sh;
            dong_vec3_t eye = dong_vec3(scene->cam_x, scene->cam_y, scene->cam_z);
            dong_vec3_t fwd = dong_vec3(
                cosf(scene->cam_pitch) * sinf(scene->cam_yaw),
                sinf(scene->cam_pitch),
                cosf(scene->cam_pitch) * cosf(scene->cam_yaw));
            dong_vec3_t target = dong_vec3_add(eye, fwd);
            dong_mat4_t view = dong_mat4_look_at(eye, target, dong_vec3(0, 1, 0));
            dong_mat4_t proj = dong_mat4_perspective(DONG_DEG_TO_RAD(60), aspect, 0.1f, 1000.0f);
            dong_mat4_t vp = dong_mat4_multiply(proj, view);

            // Render each screen
            for (int i = 0; i < scene->screen_count; i++) {
                dong_screen3d_t* scr = scene->screens[i];
                if (!scr || !scr->texture) continue;

                // Model matrix: translate * rotate * scale
                // Scale by screen_width and screen_height to size the unit quad
                dong_mat4_t scale = dong_mat4_scale(scr->screen_width, scr->screen_height, 1.0f);
                dong_mat4_t rotate = dong_mat4_rotate_y(scr->yaw);
                dong_mat4_t translate = dong_mat4_translate(scr->pos_x, scr->pos_y, scr->pos_z);
                dong_mat4_t model = dong_mat4_multiply(translate, dong_mat4_multiply(rotate, scale));
                dong_mat4_t mvp = dong_mat4_multiply(vp, model);

                UniformsTextured u = {0};
                memcpy(u.mvp, mvp.m, 64);
                memcpy(u.model, model.m, 64);
                u.color[0] = u.color[1] = u.color[2] = u.color[3] = 1.0f;
                u.highlight[0] = scr->hovered ? 1.0f : 0.0f;

                SDL_PushGPUVertexUniformData(cmd, 0, &u, sizeof(u));
                SDL_PushGPUFragmentUniformData(cmd, 0, &u, sizeof(u));

                SDL_GPUBufferBinding vbb = {scene->quad_vb, 0};
                SDL_BindGPUVertexBuffers(pass, 0, &vbb, 1);

                SDL_GPUTextureSamplerBinding tsb = {scr->texture, scene->sampler};
                SDL_BindGPUFragmentSamplers(pass, 0, &tsb, 1);

                SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
            }

            SDL_EndGPURenderPass(pass);
        }

        // Render HUD overlays on top (same command buffer, new render pass without depth)
        if (scene->overlay_count > 0 && scene->hud_pipeline && scene->hud_quad_vb) {
            static int hud_debug_count = 0;
            if (hud_debug_count < 5) {
                printf("[Scene3D] HUD render: overlay_count=%d, pipeline=%p, quad_vb=%p\n",
                       scene->overlay_count, (void*)scene->hud_pipeline, (void*)scene->hud_quad_vb);
                hud_debug_count++;
            }

            SDL_GPUColorTargetInfo hud_cti = {0};
            hud_cti.texture = swapchain;
            hud_cti.load_op = SDL_GPU_LOADOP_LOAD;  // Preserve 3D scene
            hud_cti.store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass* hud_pass = SDL_BeginGPURenderPass(cmd, &hud_cti, 1, NULL);
            if (hud_pass) {
                SDL_BindGPUGraphicsPipeline(hud_pass, scene->hud_pipeline);

                SDL_GPUBufferBinding hud_vbb = {scene->hud_quad_vb, 0};
                SDL_BindGPUVertexBuffers(hud_pass, 0, &hud_vbb, 1);

                for (int i = 0; i < scene->overlay_count; i++) {
                    dong_overlay_t* ov = scene->overlays[i];
                    SDL_GPUTexture* ov_tex = ov ? (SDL_GPUTexture*)dong_overlay_get_texture(ov) : NULL;

                    static int tex_debug_count = 0;
                    if (tex_debug_count < 5) {
                        printf("[Scene3D] HUD overlay[%d]: ov=%p, tex=%p\n", i, (void*)ov, (void*)ov_tex);
                        tex_debug_count++;
                    }

                    if (!ov_tex) continue;

                    float hud_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                    SDL_PushGPUFragmentUniformData(cmd, 0, hud_color, sizeof(hud_color));

                    SDL_GPUTextureSamplerBinding hud_tsb = {ov_tex, scene->sampler};
                    SDL_BindGPUFragmentSamplers(hud_pass, 0, &hud_tsb, 1);

                    SDL_DrawGPUPrimitives(hud_pass, 6, 1, 0, 0);
                }

                SDL_EndGPURenderPass(hud_pass);
            }
        }
    }

    SDL_SubmitGPUCommandBuffer(cmd);
}

// Pipeline creation
static SDL_GPUShader* compile_shader(SDL_GPUDevice* dev, SDL_GPUShaderStage stage, const char* hlsl, int nSamp, int nUB) {
    SDL_GPUShaderFormat fmts = SDL_GetGPUShaderFormats(dev);
    SDL_ShaderCross_HLSL_Info info = {0};
    info.source = hlsl;
    info.entrypoint = "main";
    info.shader_stage = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;

    void* code = NULL; size_t sz = 0;
    SDL_GPUShaderFormat fmt = SDL_GPU_SHADERFORMAT_INVALID;

    if (fmts & SDL_GPU_SHADERFORMAT_SPIRV) {
        code = SDL_ShaderCross_CompileSPIRVFromHLSL(&info, &sz);
        fmt = SDL_GPU_SHADERFORMAT_SPIRV;
    } else if (fmts & SDL_GPU_SHADERFORMAT_DXIL) {
        code = SDL_ShaderCross_CompileDXILFromHLSL(&info, &sz);
        fmt = SDL_GPU_SHADERFORMAT_DXIL;
    }
    if (!code) return NULL;

    SDL_GPUShaderCreateInfo sci = {0};
    sci.code = code; sci.code_size = sz;
    sci.entrypoint = "main"; sci.format = fmt; sci.stage = stage;
    sci.num_samplers = nSamp; sci.num_uniform_buffers = nUB;
    SDL_GPUShader* sh = SDL_CreateGPUShader(dev, &sci);
    SDL_free(code);
    return sh;
}

static int create_pipelines(dong_scene3d_t* scene) {
    if (!SDL_ShaderCross_Init()) return 0;

    // Sampler
    SDL_GPUSamplerCreateInfo sai = {0};
    sai.min_filter = sai.mag_filter = SDL_GPU_FILTER_LINEAR;
    sai.address_mode_u = sai.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    scene->sampler = SDL_CreateGPUSampler(scene->device, &sai);

    // Quad VB - unit quad centered at origin, will be scaled by model matrix
    // Each screen uses its own screen_width/screen_height to scale this
    float quad[] = {
        -0.5f, -0.5f, 0, 0, 1,   0.5f, -0.5f, 0, 1, 1,   0.5f,  0.5f, 0, 1, 0,
        -0.5f, -0.5f, 0, 0, 1,   0.5f,  0.5f, 0, 1, 0,  -0.5f,  0.5f, 0, 0, 0
    };
    SDL_GPUBufferCreateInfo vbi = {0};
    vbi.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vbi.size = sizeof(quad);
    scene->quad_vb = SDL_CreateGPUBuffer(scene->device, &vbi);

    SDL_GPUTransferBufferCreateInfo tbi = {0};
    tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbi.size = sizeof(quad);
    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(scene->device, &tbi);
    void* ptr = SDL_MapGPUTransferBuffer(scene->device, tb, 0);
    memcpy(ptr, quad, sizeof(quad));
    SDL_UnmapGPUTransferBuffer(scene->device, tb);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(scene->device);
    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTransferBufferLocation src = {tb, 0};
    SDL_GPUBufferRegion dst = {scene->quad_vb, 0, sizeof(quad)};
    SDL_UploadToGPUBuffer(cp, &src, &dst, 0);
    SDL_EndGPUCopyPass(cp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(scene->device, tb);

    // Shaders
    SDL_GPUShader* vs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_VERTEX, VS_TEXTURED, 0, 1);
    SDL_GPUShader* fs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_FRAGMENT, FS_TEXTURED, 1, 1);
    if (!vs || !fs) {
        if (vs) SDL_ReleaseGPUShader(scene->device, vs);
        if (fs) SDL_ReleaseGPUShader(scene->device, fs);
        return 0;
    }

    // Pipeline
    SDL_GPUTextureFormat swfmt = SDL_GetGPUSwapchainTextureFormat(scene->device, scene->window);

    SDL_GPUVertexBufferDescription vbd = {0};
    vbd.slot = 0; vbd.pitch = 5 * sizeof(float); vbd.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attrs[2] = {0};
    attrs[0].location = 0; attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attrs[1].location = 1; attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; attrs[1].offset = 12;

    SDL_GPUColorTargetDescription ctd = {0};
    ctd.format = swfmt;
    ctd.blend_state.enable_blend = 1;
    ctd.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    ctd.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    ctd.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    ctd.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    ctd.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    ctd.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

    SDL_GPUGraphicsPipelineCreateInfo pci = {0};
    pci.vertex_shader = vs;
    pci.fragment_shader = fs;
    pci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pci.vertex_input_state.num_vertex_buffers = 1;
    pci.vertex_input_state.vertex_buffer_descriptions = &vbd;
    pci.vertex_input_state.num_vertex_attributes = 2;
    pci.vertex_input_state.vertex_attributes = attrs;
    pci.target_info.num_color_targets = 1;
    pci.target_info.color_target_descriptions = &ctd;
    pci.target_info.has_depth_stencil_target = 1;
    pci.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM;
    pci.depth_stencil_state.enable_depth_test = 1;
    pci.depth_stencil_state.enable_depth_write = 1;
    pci.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    pci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    scene->screen_pipeline = SDL_CreateGPUGraphicsPipeline(scene->device, &pci);

    SDL_ReleaseGPUShader(scene->device, vs);
    SDL_ReleaseGPUShader(scene->device, fs);

    if (!scene->screen_pipeline) return 0;

    // =========================================================================
    // HUD Pipeline (fullscreen quad, no depth, premultiplied alpha)
    // =========================================================================

    // HUD quad (NDC coordinates)
    float hud_quad[] = {
        -1, -1, 0, 1,   // bottom-left: pos.xy, uv.xy
         1, -1, 1, 1,   // bottom-right
         1,  1, 1, 0,   // top-right
        -1, -1, 0, 1,   // bottom-left
         1,  1, 1, 0,   // top-right
        -1,  1, 0, 0    // top-left
    };

    SDL_GPUBufferCreateInfo hud_vbi = {0};
    hud_vbi.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    hud_vbi.size = sizeof(hud_quad);
    scene->hud_quad_vb = SDL_CreateGPUBuffer(scene->device, &hud_vbi);

    SDL_GPUTransferBufferCreateInfo hud_tbi = {0};
    hud_tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    hud_tbi.size = sizeof(hud_quad);
    SDL_GPUTransferBuffer* hud_tb = SDL_CreateGPUTransferBuffer(scene->device, &hud_tbi);
    void* hud_ptr = SDL_MapGPUTransferBuffer(scene->device, hud_tb, 0);
    memcpy(hud_ptr, hud_quad, sizeof(hud_quad));
    SDL_UnmapGPUTransferBuffer(scene->device, hud_tb);

    SDL_GPUCommandBuffer* hud_cmd = SDL_AcquireGPUCommandBuffer(scene->device);
    SDL_GPUCopyPass* hud_cp = SDL_BeginGPUCopyPass(hud_cmd);
    SDL_GPUTransferBufferLocation hud_src = {hud_tb, 0};
    SDL_GPUBufferRegion hud_dst = {scene->hud_quad_vb, 0, sizeof(hud_quad)};
    SDL_UploadToGPUBuffer(hud_cp, &hud_src, &hud_dst, 0);
    SDL_EndGPUCopyPass(hud_cp);
    SDL_SubmitGPUCommandBuffer(hud_cmd);
    SDL_ReleaseGPUTransferBuffer(scene->device, hud_tb);

    // HUD shaders
    SDL_GPUShader* hud_vs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_VERTEX, VS_HUD, 0, 0);
    SDL_GPUShader* hud_fs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_FRAGMENT, FS_HUD, 1, 1);
    if (!hud_vs || !hud_fs) {
        if (hud_vs) SDL_ReleaseGPUShader(scene->device, hud_vs);
        if (hud_fs) SDL_ReleaseGPUShader(scene->device, hud_fs);
        printf("[Scene3D] Warning: HUD shader compilation failed\n");
        return 1;  // Still return success (3D works, HUD doesn't)
    }

    // HUD vertex layout (pos.xy + uv.xy = 4 floats)
    SDL_GPUVertexBufferDescription hud_vbd = {0};
    hud_vbd.slot = 0;
    hud_vbd.pitch = 4 * sizeof(float);
    hud_vbd.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute hud_attrs[2] = {0};
    hud_attrs[0].location = 0;
    hud_attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    hud_attrs[0].offset = 0;
    hud_attrs[1].location = 1;
    hud_attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    hud_attrs[1].offset = 8;

    // HUD blend state (premultiplied alpha)
    SDL_GPUColorTargetDescription hud_ctd = {0};
    hud_ctd.format = swfmt;
    hud_ctd.blend_state.enable_blend = 1;
    hud_ctd.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    hud_ctd.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    hud_ctd.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    hud_ctd.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    hud_ctd.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    hud_ctd.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

    SDL_GPUGraphicsPipelineCreateInfo hud_pci = {0};
    hud_pci.vertex_shader = hud_vs;
    hud_pci.fragment_shader = hud_fs;
    hud_pci.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    hud_pci.vertex_input_state.num_vertex_buffers = 1;
    hud_pci.vertex_input_state.vertex_buffer_descriptions = &hud_vbd;
    hud_pci.vertex_input_state.num_vertex_attributes = 2;
    hud_pci.vertex_input_state.vertex_attributes = hud_attrs;
    hud_pci.target_info.num_color_targets = 1;
    hud_pci.target_info.color_target_descriptions = &hud_ctd;
    hud_pci.target_info.has_depth_stencil_target = 0;  // No depth for HUD
    hud_pci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    hud_pci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    scene->hud_pipeline = SDL_CreateGPUGraphicsPipeline(scene->device, &hud_pci);

    SDL_ReleaseGPUShader(scene->device, hud_vs);
    SDL_ReleaseGPUShader(scene->device, hud_fs);

    if (!scene->hud_pipeline) {
        printf("[Scene3D] Warning: HUD pipeline creation failed\n");
    }

    return 1;
}

static void destroy_pipelines(dong_scene3d_t* scene) {
    if (scene->screen_pipeline) SDL_ReleaseGPUGraphicsPipeline(scene->device, scene->screen_pipeline);
    if (scene->hud_pipeline) SDL_ReleaseGPUGraphicsPipeline(scene->device, scene->hud_pipeline);
    if (scene->sampler) SDL_ReleaseGPUSampler(scene->device, scene->sampler);
    if (scene->quad_vb) SDL_ReleaseGPUBuffer(scene->device, scene->quad_vb);
    if (scene->hud_quad_vb) SDL_ReleaseGPUBuffer(scene->device, scene->hud_quad_vb);
    if (scene->depth_texture) SDL_ReleaseGPUTexture(scene->device, scene->depth_texture);
}
