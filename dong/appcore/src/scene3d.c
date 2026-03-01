/**
 * Scene3D Implementation - 3D scene with HTML screens
 */

#include "dong_scene3d.h"
#include "dong_math.h"
#include "dong.h"
#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_plugin_api.h"
#include "dong_global_shared.h"

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

// Plugin loading for video support
static const dong_plugin_vtable_t* s_plugin_vtable = NULL;
static void* s_plugin_module = NULL;

static const dong_plugin_vtable_t* try_load_plugin(void) {
    if (s_plugin_vtable) return s_plugin_vtable;
    if (s_plugin_module) return NULL;

    const char* filename =
#if defined(_WIN32)
        "dong_plugin_sdl.dll";
#elif defined(__APPLE__)
        "libdong_plugin_sdl.dylib";
#else
        "libdong_plugin_sdl.so";
#endif

    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        s_plugin_module = (void*)1;
        return NULL;
    }

    char path[MAX_PATH_LEN];
    SDL_snprintf(path, sizeof(path), "%s%s", base_path, filename);

    s_plugin_module = SDL_LoadObject(path);
    if (!s_plugin_module) {
        s_plugin_module = (void*)1;
        return NULL;
    }

    typedef const dong_plugin_vtable_t* (*get_api_fn)(void);
    get_api_fn fn = (get_api_fn)SDL_LoadFunction((SDL_SharedObject*)s_plugin_module, "dong_plugin_get_api");
    if (!fn) {
        SDL_UnloadObject((SDL_SharedObject*)s_plugin_module);
        s_plugin_module = (void*)1;
        return NULL;
    }

    s_plugin_vtable = fn();
    return s_plugin_vtable;
}

static size_t strnlen_upto(const char* s, size_t max_len) {
    if (!s) return 0;
    size_t n = 0;
    while (n < max_len && s[n]) {
        n++;
    }
    return n;
}

static void copy_string(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) return;
    if (!src || !src[0]) {
        dst[0] = 0;
        return;
    }

    const size_t max_copy = dst_size - 1;
    const size_t n = strnlen_upto(src, max_copy);
    if ((const char*)dst != src) {
        memmove(dst, src, n);
    }
    dst[n] = 0;
}

static void extract_dir_from_path(const char* path, char* out_dir, size_t out_size) {
    if (!out_dir || out_size == 0) return;
    out_dir[0] = 0;
    if (!path || !path[0]) return;

    copy_string(out_dir, out_size, path);
    char* last_sep = strrchr(out_dir, '/');
    if (!last_sep) last_sep = strrchr(out_dir, '\\');
    if (last_sep) {
        *last_sep = 0;
    } else {
        out_dir[0] = 0;
    }
}

static void build_full_path(const char* root, const char* file, char* out_path, size_t out_size) {
    if (!out_path || out_size == 0) return;
    out_path[0] = 0;
    if (!file || !file[0]) return;

    if (root && root[0]) {
        SDL_snprintf(out_path, out_size, "%s/%s", root, file);
    } else {
        copy_string(out_path, out_size, file);
    }
}

static char* read_text_file(const char* path, size_t* out_size) {
    if (out_size) *out_size = 0;
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

    size_t read = fread(buf, 1, (size_t)sz, f);
    buf[read] = 0;
    fclose(f);

    if (out_size) *out_size = read;
    return buf;
}

// Uniform structures (must match shader)


typedef struct {
    float mvp[16];
    float model[16];
    float color[4];
    float highlight[4];
} UniformsTextured;

// Internal structures
struct dong_screen3d_t {
    dong_engine_t* engine;
    SDL_GPUTexture* texture;
    float pos_x, pos_y, pos_z;
    float yaw;
    float screen_width, screen_height;
    uint32_t tex_width, tex_height;

    int hovered;
    int focused;

    // Cached mouse position in this screen's pixel space (for cursor queries, click routing)
    int32_t mouse_x;
    int32_t mouse_y;

    int is_video;
    double next_update_time;

    // Dirty flag for optimization - only re-render when content changes
    int dirty;

    // Warmup: 某些资源（图片/字体/video poster）可能在首次 tick 后异步就绪。
    // 3D 多屏为了省性能不会每帧都 tick 静态页面，因此给每个 screen 一个短暂的 warmup 更新窗口。
    int warmup_updates_remaining;

    char resource_root[MAX_PATH_LEN];
    char debug_name[256];
};


// dong_overlay_t is defined in overlay.c

struct dong_scene3d_t {
    dong_app_t* app;
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

    // Camera input state (event-driven, avoids relying on SDL_GetKeyboardState timing)
    int key_w, key_a, key_s, key_d;
    int key_q, key_e;
    int key_space;
    int key_lctrl, key_rctrl;
    int key_lshift, key_rshift;
    int skip_next_rel_mouse_delta;

    // Background
    float bg_r, bg_g, bg_b, bg_a;
    int depth_test_enabled;

    // GPU resources for 3D screens
    SDL_GPUGraphicsPipeline* screen_pipeline;
    SDL_GPUSampler* sampler;
    SDL_GPUBuffer* quad_vb;
    SDL_GPUTexture* depth_texture;
    SDL_GPUTextureFormat depth_format;
    uint32_t depth_width, depth_height;

    // GPU resources for HUD overlay
    SDL_GPUGraphicsPipeline* hud_pipeline;
    SDL_GPUBuffer* hud_quad_vb;

    // Scheduler state for offscreen updates
    double scheduler_start_time;
    int bg_rr_cursor;
    int initial_full_update_done;

    // 只有在内容发生变化时才需要提交到 swapchain；否则会被 swapchain/backbuffer 语义节流到几十~一百多 FPS。
    int dirty;

    // Input routing state
    int hovered_idx;
    int focused_idx;
    int pressed_idx;
    int32_t pressed_x;
    int32_t pressed_y;
    int right_mouse_down;
    int last_sent_mouse_screen;

    int32_t last_sent_mouse_x;
    int32_t last_sent_mouse_y;
};

static dong_mat4_t build_solo_screen_model(const dong_scene3d_t* scene, const dong_screen3d_t* scr) {
    if (!scene || !scr) {
        return dong_mat4_identity();
    }

    // Place the screen directly in front of the camera, facing the camera.
    const float yaw = scene->cam_yaw;
    const float pitch = scene->cam_pitch;

    dong_vec3_t eye = dong_vec3(scene->cam_x, scene->cam_y, scene->cam_z);
    dong_vec3_t fwd = dong_vec3(
        cosf(pitch) * sinf(yaw),
        sinf(pitch),
        cosf(pitch) * cosf(yaw));

    const float dist = 3.0f;
    dong_vec3_t pos = dong_vec3_add(eye, dong_vec3_scale(fwd, dist));

    dong_mat4_t scale = dong_mat4_scale(scr->screen_width * 2.0f, scr->screen_height * 2.0f, 1.0f);
    dong_mat4_t rotate = dong_mat4_rotate_y(yaw + DONG_PI);
    dong_mat4_t translate = dong_mat4_translate(pos.x, pos.y, pos.z);
    return dong_mat4_multiply(translate, dong_mat4_multiply(rotate, scale));
}

static void set_screen_resource_root(dong_screen3d_t* screen, const char* root) {
    if (!screen || !screen->engine || !root || !root[0]) return;
    copy_string(screen->resource_root, sizeof(screen->resource_root), root);
    (void)dong_engine_set_resource_root(screen->engine, screen->resource_root);
}

static dong_engine_t* create_screen_engine(dong_scene3d_t* scene, uint32_t width, uint32_t height) {
    if (!scene || !scene->device || !scene->window) return NULL;

    dong_engine_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.api_version = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.plugin = try_load_plugin();
    desc.plugin_user = NULL;
    desc.width = width;
    desc.height = height;

    dong_engine_t* engine = NULL;
    if (dong_engine_create(&desc, &engine) != DONG_OK || !engine) {
        return NULL;
    }
    if (dong_engine_set_gpu(engine, scene->device, scene->window) != DONG_OK) {
        dong_engine_destroy(engine);
        return NULL;
    }

    return engine;
}

static SDL_GPUTexture* ensure_offscreen_texture(SDL_GPUDevice* device,
                                                SDL_GPUTexture* cached,
                                                uint32_t* cached_w,
                                                uint32_t* cached_h,
                                                uint32_t width,
                                                uint32_t height) {
    if (!device) return NULL;
    if (cached && *cached_w == width && *cached_h == height) {
        return cached;
    }

    if (cached) {
        SDL_ReleaseGPUTexture(device, cached);
    }

    SDL_GPUTextureCreateInfo tex_info = {0};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    tex_info.width = width;
    tex_info.height = height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    SDL_GPUTexture* created = SDL_CreateGPUTexture(device, &tex_info);
    if (!created) {
        return NULL;
    }

    *cached_w = width;
    *cached_h = height;
    return created;
}

static int env_i32_or_default(const char* name, int default_value) {
    const char* s = getenv(name);
    if (!s || !s[0]) return default_value;
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (end == s) return default_value;
    return (int)v;
}

static int get_solo_screen_idx(void) {
    return env_i32_or_default("DONG_SCENE3D_SOLO_SCREEN", -1);
}

static int write_bmp24(const char* filename, uint32_t width, uint32_t height, const uint8_t* rgba_data, int input_is_bgra) {
    if (!filename || !filename[0] || !rgba_data || width == 0 || height == 0) return 0;
    (void)input_is_bgra;

    FILE* f = fopen(filename, "wb");
    if (!f) return 0;

    const uint32_t row_size = ((width * 3 + 3) / 4) * 4;
    const uint32_t pixel_data_size = row_size * height;
    const uint32_t filesize = 54 + pixel_data_size;

    uint8_t bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    bmpfileheader[2] = (uint8_t)(filesize);
    bmpfileheader[3] = (uint8_t)(filesize >> 8);
    bmpfileheader[4] = (uint8_t)(filesize >> 16);
    bmpfileheader[5] = (uint8_t)(filesize >> 24);

    uint8_t bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    bmpinfoheader[4]  = (uint8_t)(width);
    bmpinfoheader[5]  = (uint8_t)(width >> 8);
    bmpinfoheader[6]  = (uint8_t)(width >> 16);
    bmpinfoheader[7]  = (uint8_t)(width >> 24);
    bmpinfoheader[8]  = (uint8_t)(height);
    bmpinfoheader[9]  = (uint8_t)(height >> 8);
    bmpinfoheader[10] = (uint8_t)(height >> 16);
    bmpinfoheader[11] = (uint8_t)(height >> 24);

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    uint8_t* row = (uint8_t*)calloc(1, row_size);
    if (!row) {
        fclose(f);
        return 0;
    }

    for (int y = (int)height - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint32_t idx = ((uint32_t)y * width + x) * 4;
            if (input_is_bgra) {
                row[x * 3 + 0] = rgba_data[idx + 0];
                row[x * 3 + 1] = rgba_data[idx + 1];
                row[x * 3 + 2] = rgba_data[idx + 2];
            } else {
                row[x * 3 + 0] = rgba_data[idx + 2];
                row[x * 3 + 1] = rgba_data[idx + 1];
                row[x * 3 + 2] = rgba_data[idx + 0];
            }
        }
        fwrite(row, 1, row_size, f);
    }

    free(row);
    fclose(f);
    return 1;
}

static int read_gpu_texture_rgba8(SDL_GPUDevice* dev,
                                 SDL_GPUTexture* tex,
                                 uint32_t width,
                                 uint32_t height,
                                 uint8_t** out_rgba,
                                 uint32_t* out_size) {
    if (out_rgba) *out_rgba = NULL;
    if (out_size) *out_size = 0;
    if (!dev || !tex || width == 0 || height == 0 || !out_rgba || !out_size) return 0;

    const uint32_t size = width * height * 4;

    SDL_GPUTransferBufferCreateInfo tb_info = {0};
    tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    tb_info.size = size;

    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(dev, &tb_info);
    if (!transfer) return 0;

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(dev);
    if (!cmd) {
        SDL_ReleaseGPUTransferBuffer(dev, transfer);
        return 0;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    if (!copy_pass) {
        SDL_CancelGPUCommandBuffer(cmd);
        SDL_ReleaseGPUTransferBuffer(dev, transfer);
        return 0;
    }

    SDL_GPUTextureRegion src = {0};
    src.texture = tex;
    src.x = 0;
    src.y = 0;
    src.w = width;
    src.h = height;
    src.d = 1;

    SDL_GPUTextureTransferInfo dst = {0};
    dst.transfer_buffer = transfer;
    dst.offset = 0;
    dst.pixels_per_row = width;
    dst.rows_per_layer = height;

    SDL_DownloadFromGPUTexture(copy_pass, &src, &dst);
    SDL_EndGPUCopyPass(copy_pass);

    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
    if (!fence) {
        SDL_ReleaseGPUTransferBuffer(dev, transfer);
        return 0;
    }

    while (!SDL_QueryGPUFence(dev, fence)) {
        SDL_Delay(1);
    }
    SDL_ReleaseGPUFence(dev, fence);

    void* mapped = SDL_MapGPUTransferBuffer(dev, transfer, false);
    if (!mapped) {
        SDL_ReleaseGPUTransferBuffer(dev, transfer);
        return 0;
    }

    uint8_t* buf = (uint8_t*)malloc(size);
    if (!buf) {
        SDL_UnmapGPUTransferBuffer(dev, transfer);
        SDL_ReleaseGPUTransferBuffer(dev, transfer);
        return 0;
    }

    memcpy(buf, mapped, size);
    SDL_UnmapGPUTransferBuffer(dev, transfer);
    SDL_ReleaseGPUTransferBuffer(dev, transfer);

    *out_rgba = buf;
    *out_size = size;
    return 1;
}

static void maybe_dump_screen_texture(dong_scene3d_t* scene, int idx, SDL_GPUTexture* tex, uint32_t w, uint32_t h) {
    static int done = 0;
    if (done) return;

    const int want = env_i32_or_default("DONG_SCENE3D_DUMP_SCREEN", -1);
    if (want < 0 || want != idx) return;

    const char* out = getenv("DONG_SCENE3D_DUMP_OUT");
    if (!out || !out[0]) out = "/tmp/scene3d_screen_dump.bmp";

    uint8_t* rgba = NULL;
    uint32_t size = 0;
    if (!read_gpu_texture_rgba8(scene->device, tex, w, h, &rgba, &size) || !rgba) {
        printf("[Scene3D] dump failed (idx=%d)\n", idx);
        return;
    }

    if (write_bmp24(out, w, h, rgba, 0)) {
        printf("[Scene3D] dumped screen %d to %s (%ux%u)\n", idx, out, w, h);
        done = 1;
    }

    free(rgba);
}

static void maybe_dump_swapchain_texture(dong_scene3d_t* scene, SDL_GPUTexture* tex, uint32_t w, uint32_t h, int input_is_bgra) {
    static int done = 0;
    static int frame_no = 0;
    if (done) return;

    const int want = env_i32_or_default("DONG_SCENE3D_DUMP_FRAME", 0);
    if (!want) return;

    frame_no++;
    const int after = env_i32_or_default("DONG_SCENE3D_DUMP_FRAME_AFTER", 0);
    if (frame_no < after) return;

    const char* out = getenv("DONG_SCENE3D_DUMP_FRAME_OUT");
    if (!out || !out[0]) out = "/tmp/scene3d_frame_dump.bmp";

    uint8_t* rgba = NULL;
    uint32_t size = 0;
    if (!read_gpu_texture_rgba8(scene->device, tex, w, h, &rgba, &size) || !rgba) {
        printf("[Scene3D] frame dump failed\n");
        return;
    }

    if (write_bmp24(out, w, h, rgba, input_is_bgra)) {
        printf("[Scene3D] dumped frame to %s (%ux%u)\n", out, w, h);
        done = 1;
    }

    free(rgba);
}

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
    "    // Key out near-white opaque background only (avoid killing antialiased text).\n"
    "    if (c.a > 0.98 && rgbEnergy > 0.98 && abs(c.r - c.g) < 0.03 && abs(c.g - c.b) < 0.03) {\n"
    "        c = float4(0.0, 0.0, 0.0, 0.0);\n"
    "    }\n"
    "    return c * uColor;\n"
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

    // Initialize GlobalShared for multi-view resource sharing
    // This enables shared GlyphAtlas across all screens, saving ~1GB GPU memory for 23 screens
    if (!dong_global_shared_is_initialized()) {
        DongPlatform* platform = dong_platform_get();
        DongGPUDriver* driver = platform ? dong_platform_get_gpu_driver(platform) : NULL;
        if (driver) {
            if (dong_global_shared_initialize(driver)) {
                printf("[Scene3D] GlobalShared initialized for resource sharing\n");
            } else {
                printf("[Scene3D] Warning: Failed to initialize GlobalShared\n");
            }
        }
    }

    // Defaults
    scene->cam_x = 0; scene->cam_y = 2.5f; scene->cam_z = 5;
    scene->cam_yaw = -DONG_PI; scene->cam_pitch = 0;
    scene->cam_speed = 5; scene->cam_sensitivity = 0.002f;
    scene->cam_controls_enabled = 1;
    scene->bg_r = 0.1f; scene->bg_g = 0.1f; scene->bg_b = 0.15f; scene->bg_a = 1;
    scene->depth_test_enabled = 1;

    // Input defaults
    scene->hovered_idx = -1;
    scene->focused_idx = -1;
    scene->pressed_idx = -1;
    scene->last_sent_mouse_screen = -1;
    scene->last_sent_mouse_x = (int32_t)0x80000000;
    scene->last_sent_mouse_y = (int32_t)0x80000000;

    // 首帧需要渲染一次把初始内容提交到 swapchain。
    scene->dirty = 1;

    if (!create_pipelines(scene)) {
        free(scene);
        return NULL;
    }

    // Log GlobalShared stats
    if (dong_global_shared_is_initialized()) {
        dong_global_shared_log_stats();
    }

    printf("[Scene3D] Created\n");
    return scene;
}

DONG_APPCORE_API void dong_scene3d_destroy(dong_scene3d_t* scene) {
    if (!scene) return;
    
    // Destroy all screens (this will release GlobalShared references via SDLGPUDriver destruction)
    for (int i = 0; i < scene->screen_count; i++) {
        if (scene->screens[i]) {
            if (scene->screens[i]->texture) {
                SDL_ReleaseGPUTexture(scene->device, scene->screens[i]->texture);
                scene->screens[i]->texture = NULL;
            }
            if (scene->screens[i]->engine) {
                dong_engine_destroy(scene->screens[i]->engine);
                scene->screens[i]->engine = NULL;
            }
            free(scene->screens[i]);
        }
    }

    for (int i = 0; i < scene->overlay_count; i++) {
        if (scene->overlays[i]) {
            dong_overlay_destroy(scene->overlays[i]);
        }
    }
    destroy_pipelines(scene);
    
    // Shutdown GlobalShared after all engines are destroyed
    // This will log a warning if there are still active references
    if (dong_global_shared_is_initialized()) {
        dong_global_shared_log_stats();
        dong_global_shared_shutdown();
        printf("[Scene3D] GlobalShared shutdown\n");
    }
    
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
    screen->resource_root[0] = 0;

    screen->engine = create_screen_engine(scene, screen->tex_width, screen->tex_height);
    if (!screen->engine) {
        free(screen);
        return NULL;
    }

    // Initialize optimization fields
    screen->dirty = 1;  // Mark as dirty to ensure first render
    screen->warmup_updates_remaining = 0;
    screen->next_update_time = 0.0;

    const char* base_root = config->resource_root;
    if (!base_root && scene->resource_root[0]) {
        base_root = scene->resource_root;
    }

    if (config->html_file) {
        char full_path[MAX_PATH_LEN];
        char file_dir[MAX_PATH_LEN];
        build_full_path(base_root, config->html_file, full_path, sizeof(full_path));
        extract_dir_from_path(full_path, file_dir, sizeof(file_dir));

        screen->is_video = (strncmp(config->html_file, "video/", 6) == 0);
        copy_string(screen->debug_name, sizeof(screen->debug_name), config->html_file);

        if (file_dir[0]) {
            set_screen_resource_root(screen, file_dir);
        } else if (base_root && base_root[0]) {
            set_screen_resource_root(screen, base_root);
        }

        size_t size = 0;
        char* buf = read_text_file(full_path, &size);
        if (buf && size > 0) {
            if (dong_engine_load_html(screen->engine, buf) == DONG_OK) {
                printf("[Scene3D] Loaded screen: %s (%ux%u)\n", full_path, screen->tex_width, screen->tex_height);
            }
        } else {
            printf("[Scene3D] Warning: Failed to open %s\n", full_path);
        }
        free(buf);
    } else if (config->html_content) {
        if (base_root && base_root[0]) {
            set_screen_resource_root(screen, base_root);
        }
        (void)dong_engine_load_html(screen->engine, config->html_content);
        copy_string(screen->debug_name, sizeof(screen->debug_name), "inline");
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
            if (screen->texture) {
                SDL_ReleaseGPUTexture(scene->device, screen->texture);
                screen->texture = NULL;
            }
            if (screen->engine) {
                dong_engine_destroy(screen->engine);
                screen->engine = NULL;
            }
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
    if (!s || !s->engine || !js) return 0;
    return (dong_engine_eval_script(s->engine, js) == DONG_OK) ? 1 : 0;
}
DONG_APPCORE_API void* dong_screen3d_get_view(dong_screen3d_t* s) { return s ? s->engine : NULL; }


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

// =============================================================================
// Input routing (ray cast)
// =============================================================================

typedef struct {
    dong_vec3_t origin;
    dong_vec3_t dir;
} dong_ray_t;

static dong_ray_t make_mouse_ray(const dong_scene3d_t* scene, int32_t mx, int32_t my, uint32_t w, uint32_t h) {
    dong_ray_t ray;
    ray.origin = dong_vec3(scene->cam_x, scene->cam_y, scene->cam_z);

    float ndc_x = (2.0f * (float)mx / (float)w) - 1.0f;
    float ndc_y = 1.0f - (2.0f * (float)my / (float)h);

    const float aspect = (float)w / (float)h;

    dong_vec3_t eye = ray.origin;
    dong_vec3_t fwd = dong_vec3(
        cosf(scene->cam_pitch) * sinf(scene->cam_yaw),
        sinf(scene->cam_pitch),
        cosf(scene->cam_pitch) * cosf(scene->cam_yaw)
    );
    dong_vec3_t target = dong_vec3_add(eye, fwd);

    dong_mat4_t view = dong_mat4_look_at(eye, target, dong_vec3(0, 1, 0));
    dong_mat4_t proj = dong_mat4_perspective(DONG_DEG_TO_RAD(60.0f), aspect, 0.1f, 1000.0f);
    dong_mat4_t vp = dong_mat4_multiply(proj, view);
    dong_mat4_t inv_vp = dong_mat4_inverse(vp);

    // Our perspective matrix maps to NDC z in [-1, 1]. Use far plane at z=1.
    dong_vec4_t clip_far = dong_vec4(ndc_x, ndc_y, 1.0f, 1.0f);
    dong_vec4_t far4 = dong_mat4_transform(inv_vp, clip_far);

    dong_vec3_t far_p = eye;
    if (fabsf(far4.w) > 1e-5f) {
        far_p = dong_vec3(far4.x / far4.w, far4.y / far4.w, far4.z / far4.w);
    }

    ray.dir = dong_vec3_normalize(dong_vec3_sub(far_p, eye));
    return ray;
}

// Return uv where u=0..1 (left->right), v=0..1 (bottom->top)
static int ray_hit_screen(const dong_ray_t* ray, const dong_screen3d_t* scr, float* out_t, float* out_u, float* out_v) {
    const float hw = scr->screen_width * 0.5f;
    const float hh = scr->screen_height * 0.5f;

    const float c = cosf(scr->yaw);
    const float s = sinf(scr->yaw);

    dong_vec3_t o = dong_vec3_sub(ray->origin, dong_vec3(scr->pos_x, scr->pos_y, scr->pos_z));

    // Transform into screen-local space using inverse rotation (-yaw)
    dong_vec3_t d_local;
    d_local.x = c * ray->dir.x - s * ray->dir.z;
    d_local.y = ray->dir.y;
    d_local.z = s * ray->dir.x + c * ray->dir.z;

    dong_vec3_t o_local;
    o_local.x = c * o.x - s * o.z;
    o_local.y = o.y;
    o_local.z = s * o.x + c * o.z;


    if (fabsf(d_local.z) < 1e-5f) return 0;

    const float t = -o_local.z / d_local.z;
    if (t < 0.0f) return 0;

    const float ix = o_local.x + t * d_local.x;
    const float iy = o_local.y + t * d_local.y;

    if (ix < -hw || ix > hw || iy < -hh || iy > hh) return 0;

    *out_t = t;
    *out_u = (ix + hw) / scr->screen_width;
    *out_v = (iy + hh) / scr->screen_height;
    return 1;
}

static SDL_SystemCursor map_css_cursor_to_sdl(const char* cursor_name_cstr) {
    const char* n = (cursor_name_cstr && cursor_name_cstr[0]) ? cursor_name_cstr : "auto";

    if (strcmp(n, "text") == 0) {
        return SDL_SYSTEM_CURSOR_TEXT;
    }
    if (strcmp(n, "pointer") == 0) {
        return SDL_SYSTEM_CURSOR_POINTER;
    }
    if (strcmp(n, "move") == 0) {
        return SDL_SYSTEM_CURSOR_MOVE;
    }
    if (strcmp(n, "wait") == 0 || strcmp(n, "progress") == 0) {
        return SDL_SYSTEM_CURSOR_WAIT;
    }
    if (strcmp(n, "crosshair") == 0) {
        return SDL_SYSTEM_CURSOR_CROSSHAIR;
    }
    if (strcmp(n, "not-allowed") == 0) {
        return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    }
    if (strcmp(n, "n-resize") == 0 || strcmp(n, "s-resize") == 0 || strcmp(n, "ns-resize") == 0) {
        return SDL_SYSTEM_CURSOR_NS_RESIZE;
    }
    if (strcmp(n, "e-resize") == 0 || strcmp(n, "w-resize") == 0 || strcmp(n, "ew-resize") == 0) {
        return SDL_SYSTEM_CURSOR_EW_RESIZE;
    }
    if (strcmp(n, "ne-resize") == 0 || strcmp(n, "sw-resize") == 0 || strcmp(n, "nesw-resize") == 0) {
        return SDL_SYSTEM_CURSOR_NESW_RESIZE;
    }
    if (strcmp(n, "nw-resize") == 0 || strcmp(n, "se-resize") == 0 || strcmp(n, "nwse-resize") == 0) {
        return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    }
    if (strcmp(n, "grab") == 0 || strcmp(n, "grabbing") == 0) {
        // SDL3 没有 grab/grabbing，使用 move 代替
        return SDL_SYSTEM_CURSOR_MOVE;
    }

    return SDL_SYSTEM_CURSOR_DEFAULT;
}

static void apply_css_cursor(const char* cursor_name_cstr) {
    const char* n = (cursor_name_cstr && cursor_name_cstr[0]) ? cursor_name_cstr : "auto";

    // Avoid churn if unchanged
    static char s_last[64] = {0};
    if (strncmp(s_last, n, sizeof(s_last) - 1) == 0) {
        return;
    }
    strncpy(s_last, n, sizeof(s_last) - 1);
    s_last[sizeof(s_last) - 1] = 0;

    if (strcmp(n, "none") == 0) {
        SDL_HideCursor();
        return;
    }

    SDL_ShowCursor();

    SDL_SystemCursor sys = map_css_cursor_to_sdl(n);
    static SDL_Cursor* s_cache[SDL_SYSTEM_CURSOR_COUNT];
    if (!s_cache[sys]) {
        s_cache[sys] = SDL_CreateSystemCursor(sys);
    }
    if (s_cache[sys]) {
        SDL_SetCursor(s_cache[sys]);
    }
}

static void clear_hovered_flags(dong_scene3d_t* scene) {
    for (int i = 0; i < scene->screen_count; i++) {
        if (scene->screens[i]) {
            scene->screens[i]->hovered = 0;
        }
    }
}

static int pick_hovered_screen(const dong_scene3d_t* scene, const dong_ray_t* ray, float* out_u, float* out_v) {
    float best_t = 1e30f;
    int best_idx = -1;
    float best_u = 0.0f;
    float best_v = 0.0f;

    for (int i = 0; i < scene->screen_count; i++) {
        dong_screen3d_t* scr = scene->screens[i];
        if (!scr) continue;

        float t, u, v;
        if (ray_hit_screen(ray, scr, &t, &u, &v) && t < best_t) {
            best_t = t;
            best_idx = i;
            best_u = u;
            best_v = v;
        }
    }

    if (best_idx >= 0) {
        *out_u = best_u;
        *out_v = best_v;
    }
    return best_idx;
}

static void send_mouse_move_to_screen(dong_scene3d_t* scene, int idx, int32_t x, int32_t y) {
    if (idx < 0 || idx >= scene->screen_count) return;
    dong_screen3d_t* scr = scene->screens[idx];
    if (!scr || !scr->engine) return;

    scr->mouse_x = x;
    scr->mouse_y = y;

    if (scene->last_sent_mouse_screen != idx ||
        scene->last_sent_mouse_x != x ||
        scene->last_sent_mouse_y != y) {
        dong_engine_send_mouse_move(scr->engine, x, y);
        scene->last_sent_mouse_screen = idx;
        scene->last_sent_mouse_x = x;
        scene->last_sent_mouse_y = y;
        // Mark screen as dirty since mouse move may trigger hover effects
        scr->dirty = 1;
    }
}

static void send_mouse_move_to_screen_force(dong_scene3d_t* scene, int idx, int32_t x, int32_t y) {
    if (idx < 0 || idx >= scene->screen_count) return;
    dong_screen3d_t* scr = scene->screens[idx];
    if (!scr || !scr->engine) return;

    scr->mouse_x = x;
    scr->mouse_y = y;
    dong_engine_send_mouse_move(scr->engine, x, y);
    scene->last_sent_mouse_screen = idx;
    scene->last_sent_mouse_x = x;
    scene->last_sent_mouse_y = y;
    scr->dirty = 1;
}


static void update_hover_from_mouse(dong_scene3d_t* scene, int32_t mx, int32_t my, uint32_t w, uint32_t h) {
    if (!scene || w == 0 || h == 0) return;

    dong_ray_t ray = make_mouse_ray(scene, mx, my, w, h);
    float u = 0.0f, v = 0.0f;
    const int hovered = pick_hovered_screen(scene, &ray, &u, &v);

    clear_hovered_flags(scene);

    if (hovered < 0) {
        if (scene->hovered_idx >= 0 && scene->hovered_idx < scene->screen_count) {
            send_mouse_move_to_screen(scene, scene->hovered_idx, -1, -1);
        }
        scene->hovered_idx = -1;

        if (!scene->right_mouse_down) {
            apply_css_cursor("auto");
        }
        return;
    }

    dong_screen3d_t* scr = scene->screens[hovered];
    scr->hovered = 1;

    const int32_t sx = (int32_t)(u * (float)scr->tex_width);
    const int32_t sy = (int32_t)((1.0f - v) * (float)scr->tex_height);
    send_mouse_move_to_screen(scene, hovered, sx, sy);

    if (!scene->right_mouse_down) {
        const char* css_cursor = scr->engine ? dong_engine_get_cursor_at(scr->engine, sx, sy) : "auto";
        apply_css_cursor(css_cursor);
    }


    scene->hovered_idx = hovered;
}

static void set_focused_screen(dong_scene3d_t* scene, int idx) {
    scene->focused_idx = idx;
    for (int i = 0; i < scene->screen_count; i++) {
        if (scene->screens[i]) {
            scene->screens[i]->focused = (i == idx);
        }
    }
}

static void handle_left_down(dong_scene3d_t* scene) {
    if (!scene) return;

    if (scene->hovered_idx >= 0 && scene->hovered_idx < scene->screen_count) {
        dong_screen3d_t* scr = scene->screens[scene->hovered_idx];
        set_focused_screen(scene, scene->hovered_idx);

        scene->pressed_idx = scene->hovered_idx;
        scene->pressed_x = scr->mouse_x;
        scene->pressed_y = scr->mouse_y;

        // 与 dong_app 2D 路径保持一致：按下前总是同步一次 mousemove，确保 hit-test 坐标新鲜。
        send_mouse_move_to_screen_force(scene, scene->pressed_idx, scene->pressed_x, scene->pressed_y);
        if (scr->engine) {
            dong_engine_send_mouse_button(scr->engine, SDL_BUTTON_LEFT, 1);
        }
        // Mark as dirty since click may change UI state
        scr->dirty = 1;
        return;

    }

    set_focused_screen(scene, -1);
    scene->pressed_idx = -1;
}

static void handle_left_up(dong_scene3d_t* scene) {
    if (!scene) return;

    int target = (scene->pressed_idx >= 0) ? scene->pressed_idx : scene->hovered_idx;
    if (target >= 0 && target < scene->screen_count) {
        dong_screen3d_t* scr = scene->screens[target];
        const int32_t upx = (scene->pressed_idx >= 0) ? scene->pressed_x : scr->mouse_x;
        const int32_t upy = (scene->pressed_idx >= 0) ? scene->pressed_y : scr->mouse_y;

        // 与 dong_app 2D 路径保持一致：抬起前总是同步 mousemove。
        send_mouse_move_to_screen_force(scene, target, upx, upy);
        if (scr->engine) {
            dong_engine_send_mouse_button(scr->engine, SDL_BUTTON_LEFT, 0);
        }
        // Mark as dirty since mouse up may trigger click handlers
        scr->dirty = 1;
    }


    scene->pressed_idx = -1;
}

static void handle_wheel(dong_scene3d_t* scene, float dx, float dy) {
    if (!scene) return;
    if (scene->hovered_idx < 0 || scene->hovered_idx >= scene->screen_count) return;

    dong_screen3d_t* scr = scene->screens[scene->hovered_idx];
    if (!scr || !scr->engine) return;

    // 与 2D app 路径对齐：滚轮前先同步当前位置，避免滚动命中目标错误。
    send_mouse_move_to_screen_force(scene, scene->hovered_idx, scr->mouse_x, scr->mouse_y);

    const float kWheelMultiplier = 3.0f;
    // dong_app 已经对 SDL wheel 做了方向校正，这里保持一致即可。
    dong_engine_send_mouse_wheel(scr->engine, dx * kWheelMultiplier, dy * kWheelMultiplier);
    // Mark as dirty since scrolling changes content position
    scr->dirty = 1;
}


static void update_camera_key_state_scancode(dong_scene3d_t* scene, uint32_t scancode, int down) {
    if (!scene) return;

    const int v = down ? 1 : 0;
    switch (scancode) {
        case SDL_SCANCODE_W: scene->key_w = v; break;
        case SDL_SCANCODE_A: scene->key_a = v; break;
        case SDL_SCANCODE_S: scene->key_s = v; break;
        case SDL_SCANCODE_D: scene->key_d = v; break;
        case SDL_SCANCODE_Q: scene->key_q = v; break;
        case SDL_SCANCODE_E: scene->key_e = v; break;
        case SDL_SCANCODE_SPACE: scene->key_space = v; break;
        case SDL_SCANCODE_LCTRL: scene->key_lctrl = v; break;
        case SDL_SCANCODE_RCTRL: scene->key_rctrl = v; break;
        case SDL_SCANCODE_LSHIFT: scene->key_lshift = v; break;
        case SDL_SCANCODE_RSHIFT: scene->key_rshift = v; break;
        default: break;
    }
}

static uint32_t normalize_ascii_keycode(uint32_t key) {
    if (key >= 'A' && key <= 'Z') {
        return key + ('a' - 'A');
    }
    return key;
}

static void update_camera_key_state_fallback_keycode(dong_scene3d_t* scene, uint32_t key, int down) {
    if (!scene) return;

    const int v = down ? 1 : 0;
    switch (normalize_ascii_keycode(key)) {
        case 'w': scene->key_w = v; break;
        case 'a': scene->key_a = v; break;
        case 's': scene->key_s = v; break;
        case 'd': scene->key_d = v; break;
        case 'q': scene->key_q = v; break;
        case 'e': scene->key_e = v; break;
        case SDLK_SPACE: scene->key_space = v; break;
        case SDLK_LCTRL: scene->key_lctrl = v; break;
        case SDLK_RCTRL: scene->key_rctrl = v; break;
        case SDLK_LSHIFT: scene->key_lshift = v; break;
        case SDLK_RSHIFT: scene->key_rshift = v; break;
        default: break;
    }
}

static int is_camera_shift_down(const dong_scene3d_t* scene) {
    return scene && (scene->key_lshift || scene->key_rshift);
}

static int debug_input_enabled(void) {
    const char* v = getenv("DONG_DEBUG_INPUT");
    return (v && v[0] && v[0] != '0');
}

static int is_camera_ctrl_down(const dong_scene3d_t* scene) {
    return scene && (scene->key_lctrl || scene->key_rctrl);
}

static void set_fps_input_mode(dong_scene3d_t* scene, int enable) {
    if (!scene || !scene->window) return;

    // FPS-mode semantics: grab keyboard+mouse, enable relative mouse, and disable text input
    // (IME/text input can interfere with letter keys on some platforms).
    if (enable) {
        (void)SDL_StopTextInput(scene->window);
    } else {
        (void)SDL_StartTextInput(scene->window);
    }

    (void)SDL_RaiseWindow(scene->window);

    const bool want = enable ? true : false;
    const bool ok_kbd = SDL_SetWindowKeyboardGrab(scene->window, want);
    const bool ok_mouse = SDL_SetWindowMouseGrab(scene->window, want);
    const bool ok_rel = SDL_SetWindowRelativeMouseMode(scene->window, want);

    scene->skip_next_rel_mouse_delta = enable ? 1 : 0;

    float dx = 0.0f, dy = 0.0f;
    SDL_GetRelativeMouseState(&dx, &dy);

    if (debug_input_enabled()) {
        fprintf(stderr,
                "[Scene3D][FPSInput] enable=%d kbd_grab=%d mouse_grab=%d rel=%d err=%s\n",
                enable ? 1 : 0,
                ok_kbd ? 1 : 0,
                ok_mouse ? 1 : 0,
                ok_rel ? 1 : 0,
                SDL_GetError());
    }
}

static void handle_key(dong_scene3d_t* scene, uint32_t key, int down) {
    if (!scene) return;
    if (scene->focused_idx < 0 || scene->focused_idx >= scene->screen_count) return;

    dong_screen3d_t* scr = scene->screens[scene->focused_idx];
    if (!scr || !scr->engine) return;

    dong_engine_send_key(scr->engine, key, down ? 1 : 0);
    // Mark as dirty since key input may change UI state
    scr->dirty = 1;
}

static void handle_text(dong_scene3d_t* scene, const char* text) {
    if (!scene || !text) return;
    if (scene->focused_idx < 0 || scene->focused_idx >= scene->screen_count) return;

    dong_screen3d_t* scr = scene->screens[scene->focused_idx];
    if (!scr || !scr->engine) return;

    dong_engine_send_text(scr->engine, text);
    // Mark as dirty since text input changes content
    scr->dirty = 1;
}

static void handle_text_editing(dong_scene3d_t* scene, const char* text,
                                int32_t cursor, int32_t selection_length) {
    if (!scene) return;
    if (scene->focused_idx < 0 || scene->focused_idx >= scene->screen_count) return;

    dong_screen3d_t* scr = scene->screens[scene->focused_idx];
    if (!scr || !scr->engine) return;

    dong_engine_send_text_editing(scr->engine, text, cursor, selection_length);
    scr->dirty = 1;
}


DONG_APPCORE_API void dong_scene3d_process_event(dong_scene3d_t* scene, const dong_app_event_t* event) {
    if (!scene || !event) return;

    uint32_t w = 0, h = 0;
    dong_app_get_size(scene->app, &w, &h);

    // 输入事件通常会引起 hover/focus/滚动/相机等变化，需要重新提交到 swapchain。
    scene->dirty = 1;

    switch (event->type) {
        case DONG_APP_EVENT_MOUSE_MOVE:
            update_hover_from_mouse(scene, event->mouse_move.x, event->mouse_move.y, w, h);
            break;
        case DONG_APP_EVENT_MOUSE_BUTTON:
            if (event->mouse_button.button == SDL_BUTTON_RIGHT) {
                scene->right_mouse_down = event->mouse_button.pressed ? 1 : 0;
                set_fps_input_mode(scene, event->mouse_button.pressed ? 1 : 0);
                break;
            }
            if (event->mouse_button.button == SDL_BUTTON_LEFT) {
                update_hover_from_mouse(scene, event->mouse_button.x, event->mouse_button.y, w, h);
                if (event->mouse_button.pressed) handle_left_down(scene);
                else handle_left_up(scene);
            }
            break;
        case DONG_APP_EVENT_MOUSE_WHEEL:
            handle_wheel(scene, event->mouse_wheel.delta_x, event->mouse_wheel.delta_y);
            break;
        case DONG_APP_EVENT_KEY: {
            const int down = event->key.pressed ? 1 : 0;

            if (debug_input_enabled()) {
                fprintf(stderr,
                        "[Scene3D][Key] key=0x%x sc=0x%x down=%d rep=%d\n",
                        (unsigned)event->key.key_code,
                        (unsigned)event->key.scancode,
                        down,
                        event->key.repeat);
            }

            if (event->key.scancode) {
                update_camera_key_state_scancode(scene, event->key.scancode, down);
            } else {
                update_camera_key_state_fallback_keycode(scene, event->key.key_code, down);
            }

            handle_key(scene, event->key.key_code, down);
            break;
        }
        case DONG_APP_EVENT_TEXT:
            handle_text(scene, event->text.text);
            break;
        case DONG_APP_EVENT_TEXT_EDITING:
            handle_text_editing(scene, event->text_editing.text,
                                event->text_editing.cursor,
                                event->text_editing.selection_length);
            break;
        default:
            break;
    }
}


// Input handling (deprecated)
DONG_APPCORE_API dong_screen3d_t* dong_scene3d_handle_input(dong_scene3d_t* scene) {
    (void)scene;
    return NULL;
}

static int update_screen_texture(dong_scene3d_t* scene, int idx) {
    dong_screen3d_t* scr = (scene && idx >= 0 && idx < scene->screen_count) ? scene->screens[idx] : NULL;
    if (!scr || !scr->engine || !scene) return 0;

    const int debug = (getenv("DONG_DEBUG_SCENE3D_OFFSCREEN") != NULL);
    if (debug) {
        printf("[Scene3D] update_screen_texture idx=%d name=%s size=%ux%u\n",
               idx,
               scr->debug_name[0] ? scr->debug_name : "(unnamed)",
               scr->tex_width,
               scr->tex_height);
    }

    if (scr->resource_root[0]) {
        set_screen_resource_root(scr, scr->resource_root);
    }

    DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
    if (!driver) {
        if (debug) printf("[Scene3D] update_screen_texture: driver is NULL\n");
        return 0;
    }

    SDL_GPUTexture* target = ensure_offscreen_texture(
        scene->device, scr->texture, &scr->tex_width, &scr->tex_height, scr->tex_width, scr->tex_height);
    if (!target) {
        if (debug) printf("[Scene3D] update_screen_texture: target is NULL\n");
        return 0;
    }

    if (!dong_gpu_begin_frame_offscreen(driver, target, scr->tex_width, scr->tex_height)) {
        if (debug) printf("[Scene3D] begin_frame_offscreen failed (idx=%d)\n", idx);
        return 0;
    }


    if (dong_engine_tick(scr->engine) != DONG_OK) {
        if (debug) printf("[Scene3D] engine_tick failed (idx=%d)\n", idx);
        dong_gpu_end_frame_offscreen(driver);
        return 0;
    }


    dong_gpu_end_frame_offscreen(driver);
    scr->texture = target;

    maybe_dump_screen_texture(scene, idx, target, scr->tex_width, scr->tex_height);
    return 1;
}


static double get_scene_time_sec(dong_scene3d_t* scene) {
    if (!scene) return 0.0;
    if (scene->scheduler_start_time == 0.0) {
        scene->scheduler_start_time = (double)SDL_GetTicks() / 1000.0;
    }
    return (double)SDL_GetTicks() / 1000.0 - scene->scheduler_start_time;
}

static void update_camera_controls(dong_scene3d_t* scene, float dt) {
    if (!scene || !scene->cam_controls_enabled) return;

    if (scene->right_mouse_down) {
        float dx = 0.0f, dy = 0.0f;
        SDL_GetRelativeMouseState(&dx, &dy);

        if (scene->skip_next_rel_mouse_delta) {
            scene->skip_next_rel_mouse_delta = 0;
        } else {
            scene->cam_yaw -= dx * scene->cam_sensitivity;
            scene->cam_pitch -= dy * scene->cam_sensitivity;
            if (scene->cam_pitch > 1.5f) scene->cam_pitch = 1.5f;
            if (scene->cam_pitch < -1.5f) scene->cam_pitch = -1.5f;
        }
    }

    float spd = scene->cam_speed * dt;
    if (is_camera_shift_down(scene)) spd *= 2.0f;

    float fx = sinf(scene->cam_yaw), fz = cosf(scene->cam_yaw);
    float rx = -cosf(scene->cam_yaw), rz = sinf(scene->cam_yaw);

    if (scene->key_w) { scene->cam_x += fx * spd; scene->cam_z += fz * spd; }
    if (scene->key_s) { scene->cam_x -= fx * spd; scene->cam_z -= fz * spd; }
    if (scene->key_d) { scene->cam_x += rx * spd; scene->cam_z += rz * spd; }
    if (scene->key_a) { scene->cam_x -= rx * spd; scene->cam_z -= rz * spd; }
    if (scene->key_space || scene->key_e) scene->cam_y += spd;
    if (is_camera_ctrl_down(scene) || scene->key_q) scene->cam_y -= spd;
}

// Calculate distance from camera to screen for LOD
static float get_screen_distance_sq(dong_scene3d_t* scene, dong_screen3d_t* scr) {
    if (!scene || !scr) return 1000000.0f;
    float dx = scr->pos_x - scene->cam_x;
    float dy = scr->pos_y - scene->cam_y;
    float dz = scr->pos_z - scene->cam_z;
    return dx*dx + dy*dy + dz*dz;
}

static void mark_screen_updated(dong_screen3d_t* scr,
                                double now_sec,
                                double video_interval,
                                double static_refresh_interval,
                                int consume_warmup) {
    if (!scr) return;
    scr->dirty = 0;
    if (consume_warmup && scr->warmup_updates_remaining > 0) {
        scr->warmup_updates_remaining -= 1;
    }
    scr->next_update_time = now_sec + (scr->is_video ? video_interval : static_refresh_interval);
}

static int update_screens_scheduled(dong_scene3d_t* scene) {
    if (!scene) return 0;

    const double now_sec = get_scene_time_sec(scene);
    // Video screens: 30fps
    const double video_interval = 1.0 / 30.0;
    const int max_updates_per_frame = 1;  // Only 1 update per frame for background screens

    // Warmup: 在启动后的短时间内，多 tick 几次静态页面，确保异步资源能落到画面上。
    const int warmup_extra_updates = 2;

    // Static refresh: 即使没有输入事件，静态页面也需要偶尔 tick 一次
    // 来把“延迟就绪”的资源（图片/字体等）落到屏幕上。
    // 注意：必须用真实时间节流；用帧计数在高 FPS 下会把“10 秒一次”变成“几百毫秒一次”。
    const double static_refresh_interval = 10.0;

    // Video LOD: 只有靠近相机的视频屏幕才后台推进（默认 6m，可通过环境变量调整）。
    const float video_active_dist = (float)env_i32_or_default("DONG_SCENE3D_VIDEO_ACTIVE_DIST", 6);
    const float video_active_dist_sq = video_active_dist * video_active_dist;

    // First frame: warm up screens once
    if (!scene->initial_full_update_done) {
        int updates_total = 0;
        for (int i = 0; i < scene->screen_count; i++) {
            dong_screen3d_t* scr = scene->screens[i];
            if (!scr) continue;

            // 视频屏幕很重：默认只预热“离相机近”的视频，避免启动阶段卡很久。
            if (scr->is_video) {
                const float dist_sq = get_screen_distance_sq(scene, scr);
                if (dist_sq > video_active_dist_sq) {
                    continue;
                }
            }

            if (update_screen_texture(scene, i)) {
                ++updates_total;
                scr->warmup_updates_remaining = warmup_extra_updates;
                mark_screen_updated(scr, now_sec, video_interval, static_refresh_interval, 0);
            }
        }
        scene->initial_full_update_done = 1;
        return updates_total;
    }

    int updates_this_frame = 0;

    // Priority 1: Hovered screen - update when dirty, during warmup, or on periodic refresh
    if (scene->hovered_idx >= 0) {
        dong_screen3d_t* scr = scene->screens[scene->hovered_idx];
        if (scr && (scr->dirty || scr->warmup_updates_remaining > 0 || now_sec >= scr->next_update_time)) {
            if (update_screen_texture(scene, scene->hovered_idx)) {
                mark_screen_updated(scr, now_sec, video_interval, static_refresh_interval, 1);
                updates_this_frame++;
            }
        }
    }

    // Priority 2: Focused screen - update when dirty, during warmup, or on periodic refresh
    if (scene->focused_idx >= 0 && scene->focused_idx != scene->hovered_idx) {
        dong_screen3d_t* scr = scene->screens[scene->focused_idx];
        if (scr && (scr->dirty || scr->warmup_updates_remaining > 0 || now_sec >= scr->next_update_time)) {
            if (update_screen_texture(scene, scene->focused_idx)) {
                mark_screen_updated(scr, now_sec, video_interval, static_refresh_interval, 1);
                updates_this_frame++;
            }
        }
    }

    // Priority 3: Background video screens
    // 注意：多个视频同时 30fps 会把“离屏 tick”成本放大到很夸张，拖垮整体帧率。
    // 这里做一个非常直接的 LOD：只有当屏幕足够接近相机时才后台推进视频。

    for (int i = 0; i < scene->screen_count && updates_this_frame < max_updates_per_frame; i++) {
        dong_screen3d_t* scr = scene->screens[i];
        if (!scr || !scr->is_video) continue;
        if (i == scene->hovered_idx || i == scene->focused_idx) continue;

        const float dist_sq = get_screen_distance_sq(scene, scr);
        if (dist_sq > video_active_dist_sq && scr->warmup_updates_remaining <= 0) {
            continue;
        }

        if (scr->warmup_updates_remaining > 0 || now_sec >= scr->next_update_time) {
            if (update_screen_texture(scene, i)) {
                mark_screen_updated(scr, now_sec, video_interval, static_refresh_interval, 1);
                updates_this_frame++;
            }
        }
    }

    // Priority 4: Background static screens - update when dirty or during warmup
    // Static HTML pages don't need periodic updates after warmup.
    if (updates_this_frame < max_updates_per_frame) {
        for (int i = 0; i < scene->screen_count; i++) {
            if (updates_this_frame >= max_updates_per_frame) break;

            int idx = (scene->bg_rr_cursor + i) % scene->screen_count;
            if (idx == scene->hovered_idx || idx == scene->focused_idx) continue;

            dong_screen3d_t* scr = scene->screens[idx];
            if (!scr || scr->is_video) continue;

            if (scr->dirty || scr->warmup_updates_remaining > 0 || now_sec >= scr->next_update_time) {
                if (update_screen_texture(scene, idx)) {
                    mark_screen_updated(scr, now_sec, video_interval, static_refresh_interval, 1);
                    updates_this_frame++;
                }
            }
        }
        scene->bg_rr_cursor++;
    }

    return updates_this_frame;
}

// Update
DONG_APPCORE_API void dong_scene3d_update(dong_scene3d_t* scene, float dt) {
    if (!scene) return;

    const float old_cam_x = scene->cam_x;
    const float old_cam_y = scene->cam_y;
    const float old_cam_z = scene->cam_z;
    const float old_cam_yaw = scene->cam_yaw;
    const float old_cam_pitch = scene->cam_pitch;

    update_camera_controls(scene, dt);

    if (fabsf(scene->cam_x - old_cam_x) > 1e-6f || fabsf(scene->cam_y - old_cam_y) > 1e-6f ||
        fabsf(scene->cam_z - old_cam_z) > 1e-6f || fabsf(scene->cam_yaw - old_cam_yaw) > 1e-6f ||
        fabsf(scene->cam_pitch - old_cam_pitch) > 1e-6f) {
        scene->dirty = 1;
    }

    const int screen_updates = update_screens_scheduled(scene);
    if (screen_updates > 0) {
        scene->dirty = 1;
    }

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

    // 无变化时跳过 swapchain 提交：避免 swapchain/backbuffer 的节流把“空转帧率”锁死。
    if (scene->initial_full_update_done && !scene->dirty) {
        return;
    }

    // Ensure depth texture
    if (!scene->depth_texture || scene->depth_width != sw || scene->depth_height != sh) {
        if (scene->depth_texture) SDL_ReleaseGPUTexture(scene->device, scene->depth_texture);
        SDL_GPUTextureCreateInfo dti = {0};
        dti.type = SDL_GPU_TEXTURETYPE_2D;
        dti.format = scene->depth_format;
        dti.width = sw; dti.height = sh;
        dti.layer_count_or_depth = 1; dti.num_levels = 1;
        dti.sample_count = SDL_GPU_SAMPLECOUNT_1;
        dti.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
        scene->depth_texture = SDL_CreateGPUTexture(scene->device, &dti);
        scene->depth_width = sw; scene->depth_height = sh;
    }

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(scene->device);
    if (!cmd) return;

    SDL_GPUTexture* swapchain = NULL;
    Uint32 swapchain_w = 0, swapchain_h = 0;

    // 默认用非阻塞 acquire，避免无意把渲染帧率锁到显示器刷新率附近。
    // 如需更强的同步语义（用于调试或特定后端问题），可设置：DONG_SCENE3D_SWAPCHAIN_WAIT=1
    const int wait_swapchain = env_i32_or_default("DONG_SCENE3D_SWAPCHAIN_WAIT", 0);
    const bool ok = wait_swapchain
        ? SDL_WaitAndAcquireGPUSwapchainTexture(cmd, scene->window, &swapchain, &swapchain_w, &swapchain_h)
        : SDL_AcquireGPUSwapchainTexture(cmd, scene->window, &swapchain, &swapchain_w, &swapchain_h);
    if (!ok) {
        SDL_CancelGPUCommandBuffer(cmd);
        return;
    }
    if (!swapchain) {
        // Swapchain not available this frame, submit empty command buffer
        SDL_SubmitGPUCommandBuffer(cmd);
        return;
    }

    // Optional: render into an intermediate texture so we can safely read back a frame.
    // Swapchain textures must only be referenced by the command buffer that acquired them.
    SDL_GPUTexture* capture_tex = NULL;
    SDL_GPUTexture* color_target = swapchain;
    const int want_dump_frame = env_i32_or_default("DONG_SCENE3D_DUMP_FRAME", 0);
    if (want_dump_frame) {
        SDL_GPUTextureCreateInfo ti = {0};
        ti.type = SDL_GPU_TEXTURETYPE_2D;
        ti.format = SDL_GetGPUSwapchainTextureFormat(scene->device, scene->window);
        ti.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        ti.width = swapchain_w;
        ti.height = swapchain_h;
        ti.layer_count_or_depth = 1;
        ti.num_levels = 1;
        ti.sample_count = SDL_GPU_SAMPLECOUNT_1;

        capture_tex = SDL_CreateGPUTexture(scene->device, &ti);
        if (capture_tex) {
            color_target = capture_tex;
        }
    }

    if (swapchain && scene->screen_pipeline) {
        SDL_GPUColorTargetInfo cti = {0};
        cti.texture = color_target;
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

            const int solo_idx = get_solo_screen_idx();

            // Render each screen
            for (int i = 0; i < scene->screen_count; i++) {
                dong_screen3d_t* scr = scene->screens[i];
                if (!scr || !scr->texture) continue;
                if (solo_idx >= 0 && i != solo_idx) continue;

                dong_mat4_t model;
                if (solo_idx >= 0) {
                    model = build_solo_screen_model(scene, scr);
                } else {
                    // Model matrix: translate * rotate * scale
                    // Scale by screen_width and screen_height to size the unit quad
                    dong_mat4_t scale = dong_mat4_scale(scr->screen_width, scr->screen_height, 1.0f);
                    dong_mat4_t rotate = dong_mat4_rotate_y(scr->yaw);
                    dong_mat4_t translate = dong_mat4_translate(scr->pos_x, scr->pos_y, scr->pos_z);
                    model = dong_mat4_multiply(translate, dong_mat4_multiply(rotate, scale));
                }

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
            hud_cti.texture = color_target;
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

    if (capture_tex) {
        // Copy the rendered frame to the swapchain for presentation.
        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
        if (copy_pass) {
            SDL_GPUTextureLocation src = {0};
            src.texture = capture_tex;
            SDL_GPUTextureLocation dst = {0};
            dst.texture = swapchain;
            SDL_CopyGPUTextureToTexture(copy_pass, &src, &dst, swapchain_w, swapchain_h, 1, false);
            SDL_EndGPUCopyPass(copy_pass);
        }

        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
        if (fence) {
            while (!SDL_QueryGPUFence(scene->device, fence)) {
                SDL_Delay(1);
            }
            SDL_ReleaseGPUFence(scene->device, fence);
        }

        int capture_is_bgra = 0;
        SDL_GPUTextureFormat fmt = SDL_GetGPUSwapchainTextureFormat(scene->device, scene->window);
        if (fmt == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM || fmt == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB) {
            capture_is_bgra = 1;
        }

        maybe_dump_swapchain_texture(scene, capture_tex, swapchain_w, swapchain_h, capture_is_bgra);
        SDL_ReleaseGPUTexture(scene->device, capture_tex);
        return;
    }

    SDL_SubmitGPUCommandBuffer(cmd);
    scene->dirty = 0;
}

// Pipeline creation
static SDL_ShaderCross_ShaderStage to_shadercross_stage(SDL_GPUShaderStage stage) {
    switch (stage) {
    case SDL_GPU_SHADERSTAGE_VERTEX:
        return SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
    case SDL_GPU_SHADERSTAGE_FRAGMENT:
        return SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    default:
        return SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    }
}

static SDL_GPUShader* compile_shader(SDL_GPUDevice* dev,
                                    SDL_GPUShaderStage stage,
                                    const char* hlsl,
                                    const char* debug_name,
                                    int expected_samplers,
                                    int expected_uniform_buffers) {
    if (!dev || !hlsl) return NULL;

    SDL_ShaderCross_ShaderStage sc_stage = to_shadercross_stage(stage);

    SDL_ShaderCross_HLSL_Info hlsl_info = {0};
    hlsl_info.source = hlsl;
    hlsl_info.entrypoint = "main";
    hlsl_info.shader_stage = sc_stage;

    size_t spirv_size = 0;
    void* spirv_data = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_size);
    if (!spirv_data || spirv_size == 0) {
        printf("[Scene3D] ShaderCross HLSL->SPIRV failed (%s)\n", SDL_GetError());
        if (spirv_data) SDL_free(spirv_data);
        return NULL;
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    if (debug_name && debug_name[0]) {
        SDL_SetStringProperty(props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_NAME_STRING, debug_name);
    }

    SDL_ShaderCross_GraphicsShaderMetadata* metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(
        (const Uint8*)spirv_data, spirv_size, props);
    if (!metadata) {
        printf("[Scene3D] ShaderCross SPIRV reflection failed (%s)\n", SDL_GetError());
        SDL_free(spirv_data);
        SDL_DestroyProperties(props);
        return NULL;
    }

    if (expected_samplers >= 0 && (int)metadata->resource_info.num_samplers != expected_samplers) {
        printf("[Scene3D] Warning: shader '%s' samplers=%u expected=%d\n",
               debug_name ? debug_name : "<unnamed>", metadata->resource_info.num_samplers, expected_samplers);
    }
    if (expected_uniform_buffers >= 0 && (int)metadata->resource_info.num_uniform_buffers != expected_uniform_buffers) {
        printf("[Scene3D] Warning: shader '%s' uniform_buffers=%u expected=%d\n",
               debug_name ? debug_name : "<unnamed>", metadata->resource_info.num_uniform_buffers, expected_uniform_buffers);
    }

    SDL_ShaderCross_SPIRV_Info spirv_info = {0};
    spirv_info.bytecode = (const Uint8*)spirv_data;
    spirv_info.bytecode_size = spirv_size;
    spirv_info.entrypoint = "main";
    spirv_info.shader_stage = sc_stage;
    spirv_info.props = props;

    SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
        dev, &spirv_info, &metadata->resource_info, props);
    if (!shader) {
        printf("[Scene3D] ShaderCross SPIRV->GPU shader failed (%s)\n", SDL_GetError());
    }

    SDL_free(metadata);
    SDL_free(spirv_data);
    SDL_DestroyProperties(props);
    return shader;
}

static SDL_GPUTextureFormat choose_depth_format(SDL_GPUDevice* device) {
    if (!device) return SDL_GPU_TEXTUREFORMAT_INVALID;

    const SDL_GPUTextureUsageFlags usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    const SDL_GPUTextureType type = SDL_GPU_TEXTURETYPE_2D;

    const SDL_GPUTextureFormat candidates[] = {
        SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        SDL_GPU_TEXTUREFORMAT_D16_UNORM,
        SDL_GPU_TEXTUREFORMAT_D24_UNORM,
    };

    for (size_t i = 0; i < (sizeof(candidates) / sizeof(candidates[0])); i++) {
        if (SDL_GPUTextureSupportsFormat(device, candidates[i], type, usage)) {
            return candidates[i];
        }
    }

    return SDL_GPU_TEXTUREFORMAT_INVALID;
}

static int create_pipelines(dong_scene3d_t* scene) {
    if (!SDL_ShaderCross_Init()) {
        printf("[Scene3D] SDL_ShaderCross_Init failed (%s)\n", SDL_GetError());
        return 0;
    }

    // Sampler
    SDL_GPUSamplerCreateInfo sai = {0};
    sai.min_filter = sai.mag_filter = SDL_GPU_FILTER_LINEAR;
    sai.address_mode_u = sai.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    scene->sampler = SDL_CreateGPUSampler(scene->device, &sai);
    if (!scene->sampler) {
        printf("[Scene3D] Failed to create sampler (%s)\n", SDL_GetError());
        return 0;
    }

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
    if (!scene->quad_vb) {
        printf("[Scene3D] Failed to create quad vertex buffer (%s)\n", SDL_GetError());
        return 0;
    }

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
    SDL_GPUShader* vs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_VERTEX, VS_TEXTURED,
                                       "scene3d/VS_TEXTURED", 0, 1);
    SDL_GPUShader* fs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_FRAGMENT, FS_TEXTURED,
                                       "scene3d/FS_TEXTURED", 1, 1);
    if (!vs || !fs) {
        if (vs) SDL_ReleaseGPUShader(scene->device, vs);
        if (fs) SDL_ReleaseGPUShader(scene->device, fs);
        return 0;
    }

    // Pipeline
    SDL_GPUTextureFormat swfmt = SDL_GetGPUSwapchainTextureFormat(scene->device, scene->window);

    scene->depth_format = choose_depth_format(scene->device);
    if (scene->depth_format == SDL_GPU_TEXTUREFORMAT_INVALID) {
        printf("[Scene3D] No supported depth format on this GPU backend\n");
        return 0;
    }

    SDL_GPUVertexBufferDescription vbd = {0};
    vbd.slot = 0; vbd.pitch = 5 * sizeof(float); vbd.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attrs[2] = {0};
    attrs[0].location = 0; attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attrs[1].location = 1; attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; attrs[1].offset = 12;

    SDL_GPUColorTargetDescription ctd = {0};
    ctd.format = swfmt;
    // 3D screen 是“显示器面板”，语义上应当是不透明的。
    // 这里关闭 blending，让屏幕内容不受纹理 alpha 影响（可避免文字 alpha 异常导致整屏文字消失）。
    ctd.blend_state.enable_blend = 0;

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
    pci.target_info.depth_stencil_format = scene->depth_format;
    pci.depth_stencil_state.enable_depth_test = 1;
    pci.depth_stencil_state.enable_depth_write = 1;
    pci.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    pci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    scene->screen_pipeline = SDL_CreateGPUGraphicsPipeline(scene->device, &pci);

    SDL_ReleaseGPUShader(scene->device, vs);
    SDL_ReleaseGPUShader(scene->device, fs);

    if (!scene->screen_pipeline) {
        printf("[Scene3D] Failed to create screen pipeline (%s)\n", SDL_GetError());
        return 0;
    }

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
    SDL_GPUShader* hud_vs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_VERTEX, VS_HUD,
                                           "scene3d/VS_HUD", 0, 0);
    SDL_GPUShader* hud_fs = compile_shader(scene->device, SDL_GPU_SHADERSTAGE_FRAGMENT, FS_HUD,
                                           "scene3d/FS_HUD", 1, 1);
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
