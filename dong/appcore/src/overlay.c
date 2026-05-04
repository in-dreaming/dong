/**
 * Overlay Implementation - HUD and UI Layer
 *
 * Renders HTML-based overlays on top of the 3D scene.
 */

#include "dong_overlay.h"
#include "dong.h"
#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_plugin_api.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define MAX_PATH_LEN 1024

// HUD shader sources
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
    "cbuffer Uniforms : register(b0, space3) { float4 uColor; float uKeyWhiteBg; float3 _pad; };\n"
    "struct PSInput { float4 position : SV_Position; float2 uv : TEXCOORD0; };\n"
    "float4 main(PSInput input) : SV_Target0 {\n"
    "    float4 c = tex.Sample(texSampler, input.uv);\n"
    "    // Premultiply alpha for correct blending\n"
    "    c.rgb *= c.a;\n"
    "    c *= uColor;\n"
    "    return c;\n"
    "}\n";

// Uniform structure (must match shader)
typedef struct {
    float color[4];
    float keyWhiteBg;
    float pad[3];
} UniformsHUD;

// Plugin loading (optional: video, etc.)
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

static void copy_string(char* dst, size_t dst_size, const char* src) {
    if (!dst || dst_size == 0) return;
    if (!src || !src[0]) {
        dst[0] = 0;
        return;
    }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = 0;
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

struct dong_overlay_t {
    dong_app_t* app;
    dong_engine_t* engine;
    SDL_GPUDevice* device;
    SDL_Window* window;
    SDL_GPUTexture* texture;

    // GPU resources
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUSampler* sampler;
    SDL_GPUBuffer* quad_vb;

    uint32_t width, height;
    uint32_t tex_width, tex_height;
    int32_t pos_x, pos_y;
    float opacity;
    int enabled;
    int dirty;                          // Needs re-render when content changes
    int initial_render_done;            // First frame rendered

    char resource_root[MAX_PATH_LEN];
};

static SDL_GPUTexture* ensure_offscreen_texture(dong_overlay_t* overlay, uint32_t width, uint32_t height) {
    if (!overlay || !overlay->device) return NULL;

    if (overlay->texture && overlay->tex_width == width && overlay->tex_height == height) {
        return overlay->texture;
    }

    if (overlay->texture) {
        SDL_ReleaseGPUTexture(overlay->device, overlay->texture);
        overlay->texture = NULL;
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

    overlay->texture = SDL_CreateGPUTexture(overlay->device, &tex_info);
    if (!overlay->texture) {
        return NULL;
    }

    overlay->tex_width = width;
    overlay->tex_height = height;
    return overlay->texture;
}

// Forward declarations
static int create_overlay_pipeline(dong_overlay_t* overlay, SDL_GPUTextureFormat swapchain_fmt);
static void destroy_overlay_pipeline(dong_overlay_t* overlay);


// Compile shader helper
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

static SDL_GPUShader* compile_shader_ov(SDL_GPUDevice* dev,
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
        printf("[Overlay] ShaderCross HLSL->SPIRV failed (%s)\n", SDL_GetError());
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
        printf("[Overlay] ShaderCross SPIRV reflection failed (%s)\n", SDL_GetError());
        SDL_free(spirv_data);
        SDL_DestroyProperties(props);
        return NULL;
    }

    if (expected_samplers >= 0 && (int)metadata->resource_info.num_samplers != expected_samplers) {
        printf("[Overlay] Warning: shader '%s' samplers=%u expected=%d\n",
               debug_name ? debug_name : "<unnamed>", metadata->resource_info.num_samplers, expected_samplers);
    }
    if (expected_uniform_buffers >= 0 && (int)metadata->resource_info.num_uniform_buffers != expected_uniform_buffers) {
        printf("[Overlay] Warning: shader '%s' uniform_buffers=%u expected=%d\n",
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
        printf("[Overlay] ShaderCross SPIRV->GPU shader failed (%s)\n", SDL_GetError());
    }

    SDL_free(metadata);
    SDL_free(spirv_data);
    SDL_DestroyProperties(props);
    return shader;
}

DONG_APPCORE_API dong_overlay_t* dong_overlay_create(dong_app_t* app, uint32_t width, uint32_t height) {
    if (!app) return NULL;

    dong_overlay_t* overlay = (dong_overlay_t*)calloc(1, sizeof(dong_overlay_t));
    if (!overlay) return NULL;

    overlay->app = app;
    overlay->device = (SDL_GPUDevice*)dong_app_get_gpu_device(app);
    overlay->window = (SDL_Window*)dong_app_get_window(app);
    if (!overlay->device || !overlay->window) {
        free(overlay);
        return NULL;
    }

    // Get window size if not specified
    if (width == 0 || height == 0) {
        uint32_t ww, wh;
        dong_app_get_size(app, &ww, &wh);
        if (width == 0) width = ww;
        if (height == 0) height = wh;
    }
    overlay->width = width;
    overlay->height = height;
    overlay->opacity = 1.0f;
    overlay->enabled = 1;

    overlay->tex_width = 0;
    overlay->tex_height = 0;

    dong_engine_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.api_version = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.plugin = try_load_plugin();
    desc.plugin_user = NULL;
    desc.width = width;
    desc.height = height;

    if (dong_engine_create(&desc, &overlay->engine) != DONG_OK || !overlay->engine) {
        free(overlay);
        return NULL;
    }

    if (dong_engine_set_gpu(overlay->engine, overlay->device, overlay->window) != DONG_OK) {
        dong_engine_destroy(overlay->engine);
        free(overlay);
        return NULL;
    }

    // Create rendering pipeline

    SDL_GPUTextureFormat swfmt = SDL_GetGPUSwapchainTextureFormat(overlay->device, overlay->window);
    if (!create_overlay_pipeline(overlay, swfmt)) {
        dong_engine_destroy(overlay->engine);
        overlay->engine = NULL;
        free(overlay);
        return NULL;
    }


    printf("[Overlay] Created %ux%u\n", width, height);
    return overlay;
}

DONG_APPCORE_API void dong_overlay_destroy(dong_overlay_t* overlay) {
    if (!overlay) return;
    destroy_overlay_pipeline(overlay);
    if (overlay->texture) {
        SDL_ReleaseGPUTexture(overlay->device, overlay->texture);
        overlay->texture = NULL;
    }
    if (overlay->engine) {
        dong_engine_destroy(overlay->engine);
        overlay->engine = NULL;
    }
    free(overlay);
}


DONG_APPCORE_API int dong_overlay_load_html(dong_overlay_t* overlay, const char* html) {
    if (!overlay || !overlay->engine || !html) return 0;
    return (dong_engine_load_html(overlay->engine, html) == DONG_OK) ? 1 : 0;
}


DONG_APPCORE_API int dong_overlay_load_file(dong_overlay_t* overlay, const char* file_path) {
    if (!overlay || !overlay->engine || !file_path) return 0;

    char dir[MAX_PATH_LEN];
    extract_dir_from_path(file_path, dir, sizeof(dir));
    if (dir[0]) {
        copy_string(overlay->resource_root, sizeof(overlay->resource_root), dir);
        (void)dong_engine_set_resource_root(overlay->engine, dir);
    }

    size_t size = 0;
    char* buf = read_text_file(file_path, &size);
    if (!buf || size == 0) {
        printf("[Overlay] Failed to open %s\n", file_path);
        free(buf);
        return 0;
    }

    int ok = (dong_engine_load_html(overlay->engine, buf) == DONG_OK) ? 1 : 0;
    free(buf);

    if (ok) {
        printf("[Overlay] Loaded %s\n", file_path);
    }
    return ok;
}


DONG_APPCORE_API void dong_overlay_set_resource_root(dong_overlay_t* overlay, const char* root) {
    if (!overlay || !overlay->engine || !root) return;
    copy_string(overlay->resource_root, sizeof(overlay->resource_root), root);
    (void)dong_engine_set_resource_root(overlay->engine, root);
}


DONG_APPCORE_API int dong_overlay_eval_script(dong_overlay_t* overlay, const char* script) {
    if (!overlay || !overlay->engine || !script) return 0;
    int result = (dong_engine_eval_script(overlay->engine, script) == DONG_OK) ? 1 : 0;
    if (result) {
        overlay->dirty = 1;  // Script may have changed content
    }
    return result;
}


DONG_APPCORE_API void dong_overlay_update(dong_overlay_t* overlay, float dt) {
    (void)dt;
    if (!overlay || !overlay->engine || !overlay->enabled) return;

    // Only render on first frame or when dirty
    if (overlay->initial_render_done && !overlay->dirty) {
        return;
    }

    DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
    if (!driver) return;

    SDL_GPUTexture* target = ensure_offscreen_texture(overlay, overlay->width, overlay->height);
    if (!target) return;

    if (!dong_gpu_begin_frame_offscreen(driver, target, overlay->width, overlay->height)) {
        return;
    }

    if (dong_engine_tick(overlay->engine) != DONG_OK) {
        dong_gpu_end_frame_offscreen(driver);
        return;
    }

    dong_gpu_end_frame_offscreen(driver);
    overlay->initial_render_done = 1;
    overlay->dirty = 0;
}


DONG_APPCORE_API void dong_overlay_render(dong_overlay_t* overlay) {
    if (!overlay || !overlay->enabled || !overlay->texture || !overlay->pipeline) return;

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(overlay->device);
    if (!cmd) return;

    SDL_GPUTexture* swapchain = NULL;
    if (!SDL_AcquireGPUSwapchainTexture(cmd, overlay->window, &swapchain, NULL, NULL)) {
        SDL_CancelGPUCommandBuffer(cmd);
        return;
    }

    if (swapchain) {
        SDL_GPUColorTargetInfo cti = {0};
        cti.texture = swapchain;
        cti.load_op = SDL_GPU_LOADOP_LOAD;  // Load existing content (the 3D scene)
        cti.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &cti, 1, NULL);
        if (pass) {
            SDL_BindGPUGraphicsPipeline(pass, overlay->pipeline);

            SDL_GPUBufferBinding vbb = {overlay->quad_vb, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &vbb, 1);

            SDL_GPUTextureSamplerBinding tsb = {overlay->texture, overlay->sampler};
            SDL_BindGPUFragmentSamplers(pass, 0, &tsb, 1);

            UniformsHUD uniforms = {0};
            uniforms.color[0] = 1.0f;
            uniforms.color[1] = 1.0f;
            uniforms.color[2] = 1.0f;
            uniforms.color[3] = overlay->opacity;
            uniforms.keyWhiteBg = 0.0f;

            SDL_PushGPUFragmentUniformData(cmd, 0, &uniforms, sizeof(uniforms));

            SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);

            SDL_EndGPURenderPass(pass);
        }
    }

    SDL_SubmitGPUCommandBuffer(cmd);
}

DONG_APPCORE_API int dong_overlay_hit_test(dong_overlay_t* overlay, int32_t x, int32_t y) {
    (void)overlay; (void)x; (void)y;
    // TODO: Check if pixel at (x,y) is non-transparent
    return 0;
}

DONG_APPCORE_API void dong_overlay_send_mouse_move(dong_overlay_t* overlay, int32_t x, int32_t y) {
    if (overlay && overlay->engine) {
        dong_engine_send_mouse_move(overlay->engine, x, y);
    }
}

DONG_APPCORE_API void dong_overlay_send_mouse_button(dong_overlay_t* overlay, int32_t button, int pressed) {
    if (!overlay || !overlay->engine) return;
    dong_engine_send_mouse_button(overlay->engine, button, pressed);
}

DONG_APPCORE_API void dong_overlay_send_mouse_wheel(dong_overlay_t* overlay, float delta_x, float delta_y) {
    if (overlay && overlay->engine) {
        dong_engine_send_mouse_wheel(overlay->engine, delta_x, delta_y);
    }
}

DONG_APPCORE_API void dong_overlay_send_key(dong_overlay_t* overlay, uint32_t key_code, int pressed) {
    if (!overlay || !overlay->engine) return;
    dong_engine_send_key(overlay->engine, key_code, pressed);
}

DONG_APPCORE_API void dong_overlay_send_text(dong_overlay_t* overlay, const char* text) {
    if (overlay && overlay->engine && text) {
        dong_engine_send_text(overlay->engine, text);
    }
}

DONG_APPCORE_API void dong_overlay_send_text_editing(dong_overlay_t* overlay,
                                                      const char* text,
                                                      int32_t cursor,
                                                      int32_t selection_length) {
    if (overlay && overlay->engine) {
        dong_engine_send_text_editing(overlay->engine, text, cursor, selection_length);
    }
}


DONG_APPCORE_API void dong_overlay_set_position(dong_overlay_t* overlay, int32_t x, int32_t y) {
    if (overlay) { overlay->pos_x = x; overlay->pos_y = y; }
}

DONG_APPCORE_API void dong_overlay_set_opacity(dong_overlay_t* overlay, float opacity) {
    if (overlay) overlay->opacity = opacity;
}

DONG_APPCORE_API void dong_overlay_set_enabled(dong_overlay_t* overlay, int enabled) {
    if (overlay) overlay->enabled = enabled;
}

DONG_APPCORE_API void* dong_overlay_get_texture(dong_overlay_t* overlay) {
    return overlay ? overlay->texture : NULL;
}

// Pipeline creation
static int create_overlay_pipeline(dong_overlay_t* overlay, SDL_GPUTextureFormat swapchain_fmt) {
    if (!SDL_ShaderCross_Init()) return 0;

    // Create sampler
    SDL_GPUSamplerCreateInfo sai = {0};
    sai.min_filter = sai.mag_filter = SDL_GPU_FILTER_LINEAR;
    sai.address_mode_u = sai.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    overlay->sampler = SDL_CreateGPUSampler(overlay->device, &sai);
    if (!overlay->sampler) return 0;

    // Create fullscreen quad vertex buffer
    // NDC coordinates: -1 to 1, UV: 0 to 1
    float quad[] = {
        -1, -1, 0, 1,   // bottom-left
         1, -1, 1, 1,   // bottom-right
         1,  1, 1, 0,   // top-right
        -1, -1, 0, 1,   // bottom-left
         1,  1, 1, 0,   // top-right
        -1,  1, 0, 0    // top-left
    };

    SDL_GPUBufferCreateInfo vbi = {0};
    vbi.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vbi.size = sizeof(quad);
    overlay->quad_vb = SDL_CreateGPUBuffer(overlay->device, &vbi);
    if (!overlay->quad_vb) return 0;

    // Upload quad data
    SDL_GPUTransferBufferCreateInfo tbi = {0};
    tbi.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbi.size = sizeof(quad);
    SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(overlay->device, &tbi);
    void* ptr = SDL_MapGPUTransferBuffer(overlay->device, tb, 0);
    memcpy(ptr, quad, sizeof(quad));
    SDL_UnmapGPUTransferBuffer(overlay->device, tb);

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(overlay->device);
    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTransferBufferLocation src = {tb, 0};
    SDL_GPUBufferRegion dst = {overlay->quad_vb, 0, sizeof(quad)};
    SDL_UploadToGPUBuffer(cp, &src, &dst, 0);
    SDL_EndGPUCopyPass(cp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(overlay->device, tb);

    // Compile shaders
    SDL_GPUShader* vs = compile_shader_ov(overlay->device, SDL_GPU_SHADERSTAGE_VERTEX, VS_HUD,
                                         "overlay/VS_HUD", 0, 0);
    SDL_GPUShader* fs = compile_shader_ov(overlay->device, SDL_GPU_SHADERSTAGE_FRAGMENT, FS_HUD,
                                         "overlay/FS_HUD", 1, 1);
    if (!vs || !fs) {
        if (vs) SDL_ReleaseGPUShader(overlay->device, vs);
        if (fs) SDL_ReleaseGPUShader(overlay->device, fs);
        return 0;
    }

    // Create pipeline
    SDL_GPUVertexBufferDescription vbd = {0};
    vbd.slot = 0;
    vbd.pitch = 4 * sizeof(float);  // pos.xy + uv.xy
    vbd.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attrs[2] = {0};
    attrs[0].location = 0;
    attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrs[0].offset = 0;
    attrs[1].location = 1;
    attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrs[1].offset = 8;

    SDL_GPUColorTargetDescription ctd = {0};
    ctd.format = swapchain_fmt;
    // Alpha blending for HUD overlay
    ctd.blend_state.enable_blend = 1;
    ctd.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;  // Premultiplied alpha
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
    pci.target_info.has_depth_stencil_target = 0;  // No depth test for HUD
    pci.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pci.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    overlay->pipeline = SDL_CreateGPUGraphicsPipeline(overlay->device, &pci);

    SDL_ReleaseGPUShader(overlay->device, vs);
    SDL_ReleaseGPUShader(overlay->device, fs);

    return overlay->pipeline != NULL;
}

static void destroy_overlay_pipeline(dong_overlay_t* overlay) {
    if (!overlay || !overlay->device) return;
    if (overlay->pipeline) {
        SDL_ReleaseGPUGraphicsPipeline(overlay->device, overlay->pipeline);
        overlay->pipeline = NULL;
    }
    if (overlay->sampler) {
        SDL_ReleaseGPUSampler(overlay->device, overlay->sampler);
        overlay->sampler = NULL;
    }
    if (overlay->quad_vb) {
        SDL_ReleaseGPUBuffer(overlay->device, overlay->quad_vb);
        overlay->quad_vb = NULL;
    }
}
