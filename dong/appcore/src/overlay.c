/**
 * Overlay Implementation - HUD and UI Layer
 *
 * Renders HTML-based overlays on top of the 3D scene.
 */

#include "dong_overlay.h"
#include "dong_view.h"
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

struct dong_overlay_t {
    dong_app_t* app;
    dong_context_t* ctx;
    dong_view_t* view;
    SDL_GPUDevice* device;
    SDL_Window* window;
    SDL_GPUTexture* texture;

    // GPU resources
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUSampler* sampler;
    SDL_GPUBuffer* quad_vb;

    uint32_t width, height;
    int32_t pos_x, pos_y;
    float opacity;
    int enabled;

    char resource_root[MAX_PATH_LEN];
};

// Forward declarations
static int create_overlay_pipeline(dong_overlay_t* overlay, SDL_GPUTextureFormat swapchain_fmt);
static void destroy_overlay_pipeline(dong_overlay_t* overlay);

// Compile shader helper
static SDL_GPUShader* compile_shader_ov(SDL_GPUDevice* dev, SDL_GPUShaderStage stage, const char* hlsl, int nSamp, int nUB) {
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

    // Create Dong context and view
    overlay->ctx = dong_create_context();
    if (!overlay->ctx) {
        free(overlay);
        return NULL;
    }

    overlay->view = dong_view_create(overlay->ctx, width, height);
    if (!overlay->view) {
        dong_destroy_context(overlay->ctx);
        free(overlay);
        return NULL;
    }

    dong_view_set_external_gpu_device(overlay->view, overlay->device, overlay->window);
    dong_view_set_debug_name(overlay->view, "overlay");

    // Create rendering pipeline
    SDL_GPUTextureFormat swfmt = SDL_GetGPUSwapchainTextureFormat(overlay->device, overlay->window);
    if (!create_overlay_pipeline(overlay, swfmt)) {
        dong_view_free(overlay->view);
        dong_destroy_context(overlay->ctx);
        free(overlay);
        return NULL;
    }

    printf("[Overlay] Created %ux%u\n", width, height);
    return overlay;
}

DONG_APPCORE_API void dong_overlay_destroy(dong_overlay_t* overlay) {
    if (!overlay) return;
    destroy_overlay_pipeline(overlay);
    if (overlay->view) dong_view_free(overlay->view);
    if (overlay->ctx) dong_destroy_context(overlay->ctx);
    free(overlay);
}

DONG_APPCORE_API int dong_overlay_load_html(dong_overlay_t* overlay, const char* html) {
    if (!overlay || !overlay->view || !html) return 0;
    dong_view_load_html(overlay->view, html);
    return 1;
}

DONG_APPCORE_API int dong_overlay_load_file(dong_overlay_t* overlay, const char* file_path) {
    if (!overlay || !overlay->view || !file_path) return 0;

    // Extract directory for resource root
    char dir[MAX_PATH_LEN];
    strncpy(dir, file_path, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = 0;
    char* last_sep = strrchr(dir, '/');
    if (!last_sep) last_sep = strrchr(dir, '\\');
    if (last_sep) *last_sep = 0;
    else dir[0] = 0;
    if (dir[0]) dong_view_set_resource_root(overlay->view, dir);

    FILE* f = fopen(file_path, "rb");
    if (!f) {
        printf("[Overlay] Failed to open %s\n", file_path);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc(sz + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }

    fread(buf, 1, sz, f);
    buf[sz] = 0;
    fclose(f);

    dong_view_load_html(overlay->view, buf);
    free(buf);

    printf("[Overlay] Loaded %s\n", file_path);
    return 1;
}

DONG_APPCORE_API void dong_overlay_set_resource_root(dong_overlay_t* overlay, const char* root) {
    if (!overlay || !overlay->view || !root) return;
    strncpy(overlay->resource_root, root, MAX_PATH_LEN - 1);
    overlay->resource_root[MAX_PATH_LEN - 1] = 0;
    dong_view_set_resource_root(overlay->view, root);
}

DONG_APPCORE_API int dong_overlay_eval_script(dong_overlay_t* overlay, const char* script) {
    if (!overlay || !overlay->view || !script) return 0;
    return dong_view_eval(overlay->view, script);
}

DONG_APPCORE_API void dong_overlay_update(dong_overlay_t* overlay, float dt) {
    (void)dt;
    if (!overlay || !overlay->view || !overlay->enabled) return;

    // NOTE: Do NOT call dong_view_update() here! It renders to swapchain.
    // dong_view_render_to_gpu_texture() handles layout calculation internally.
    SDL_GPUTexture* new_tex = (SDL_GPUTexture*)dong_view_render_to_gpu_texture(
        overlay->view, overlay->device, overlay->width, overlay->height);
    if (new_tex) {
        overlay->texture = new_tex;
    }
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
    if (overlay && overlay->view) dong_view_send_mouse_move(overlay->view, x, y);
}

DONG_APPCORE_API void dong_overlay_send_mouse_button(dong_overlay_t* overlay, int32_t button, int pressed) {
    if (!overlay || !overlay->view) return;
    if (pressed) dong_view_send_mouse_down(overlay->view, button);
    else dong_view_send_mouse_up(overlay->view, button);
}

DONG_APPCORE_API void dong_overlay_send_mouse_wheel(dong_overlay_t* overlay, float delta_x, float delta_y) {
    if (overlay && overlay->view) dong_view_send_mouse_wheel(overlay->view, delta_x, delta_y);
}

DONG_APPCORE_API void dong_overlay_send_key(dong_overlay_t* overlay, uint32_t key_code, int pressed) {
    if (!overlay || !overlay->view) return;
    if (pressed) dong_view_send_key_down(overlay->view, key_code);
    else dong_view_send_key_up(overlay->view, key_code);
}

DONG_APPCORE_API void dong_overlay_send_text(dong_overlay_t* overlay, const char* text) {
    if (overlay && overlay->view && text) dong_view_send_text_input(overlay->view, text);
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
    SDL_GPUShader* vs = compile_shader_ov(overlay->device, SDL_GPU_SHADERSTAGE_VERTEX, VS_HUD, 0, 0);
    SDL_GPUShader* fs = compile_shader_ov(overlay->device, SDL_GPU_SHADERSTAGE_FRAGMENT, FS_HUD, 1, 1);
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
