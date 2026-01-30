// DongApp Implementation
// Provides a high-level application framework using SDL3 and the Dong engine.

#include "dong_app.h"
#include "dong.h"
#include "dong_platform.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// Internal Structures
// =============================================================================

typedef struct dong_app_impl_t {
    // SDL handles
    SDL_Window* window;
    SDL_GPUDevice* gpu_device;

    // Configuration
    uint32_t width;
    uint32_t height;
    int enable_dong;
    int running;
    int vsync;

    // Timing
    uint64_t last_frame_time;
    float delta_time;

    // Input state
    int32_t mouse_x;
    int32_t mouse_y;
    uint32_t mouse_buttons;

    // Dong context and view (core View API for rendering)
    dong_context_t* dong_ctx;
    dong_view_t* dong_view;

    // Blit pipeline (for rendering texture to swapchain)
    SDL_GPUGraphicsPipeline* blit_pipeline;
    SDL_GPUSampler* blit_sampler;
} dong_app_impl_t;

// =============================================================================
// Blit Shader (renders texture to screen)
// =============================================================================

static const char* BLIT_VS_HLSL =
    "struct VSOutput {\n"
    "    float4 position : SV_Position;\n"
    "    float2 uv : TEXCOORD0;\n"
    "};\n"
    "VSOutput main(uint vid : SV_VertexID) {\n"
    "    VSOutput o;\n"
    "    // Fullscreen triangle\n"
    "    float2 uv = float2((vid << 1) & 2, vid & 2);\n"
    "    o.position = float4(uv * 2.0 - 1.0, 0.0, 1.0);\n"
    "    o.uv = float2(uv.x, 1.0 - uv.y);\n"
    "    return o;\n"
    "}\n";

static const char* BLIT_FS_HLSL =
    "Texture2D tex : register(t0, space2);\n"
    "SamplerState texSampler : register(s0, space2);\n"
    "float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target0 {\n"
    "    return tex.Sample(texSampler, uv);\n"
    "}\n";

// Forward declaration
static int create_blit_pipeline(dong_app_impl_t* app);

// =============================================================================
// Public API Implementation
// =============================================================================

DONG_APPCORE_API dong_app_t* dong_app_create(const dong_app_config_t* config) {
    if (!config) return NULL;

    dong_app_impl_t* app = (dong_app_impl_t*)calloc(1, sizeof(dong_app_impl_t));
    if (!app) return NULL;

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "[DongApp] SDL_Init failed: %s\n", SDL_GetError());
        free(app);
        return NULL;
    }

    // Store configuration
    app->width = config->width > 0 ? config->width : 800;
    app->height = config->height > 0 ? config->height : 600;
    app->enable_dong = config->enable_dong;
    app->vsync = config->vsync;

    // Create window
    SDL_WindowFlags window_flags = 0;
    if (config->resizable) {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    if (config->fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN;
    }

    const char* title = config->title ? config->title : "Dong Application";
    app->window = SDL_CreateWindow(title, app->width, app->height, window_flags);
    if (!app->window) {
        fprintf(stderr, "[DongApp] SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        free(app);
        return NULL;
    }

    // Create GPU device
    app->gpu_device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        false,  // debug mode
        NULL    // driver name (auto-select)
    );
    if (!app->gpu_device) {
        fprintf(stderr, "[DongApp] SDL_CreateGPUDevice failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        free(app);
        return NULL;
    }

    // Claim window for GPU
    if (!SDL_ClaimWindowForGPUDevice(app->gpu_device, app->window)) {
        fprintf(stderr, "[DongApp] SDL_ClaimWindowForGPUDevice failed: %s\n", SDL_GetError());
        SDL_DestroyGPUDevice(app->gpu_device);
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        free(app);
        return NULL;
    }

    // Initialize Dong using View API (which has full GPU rendering support)
    if (config->enable_dong) {
        app->dong_ctx = dong_create_context();
        if (!app->dong_ctx) {
            fprintf(stderr, "[DongApp] dong_create_context failed\n");
        } else {
            app->dong_view = dong_view_create(app->dong_ctx, app->width, app->height);
            if (!app->dong_view) {
                fprintf(stderr, "[DongApp] dong_view_create failed\n");
            } else {
                // Enable GPU rendering
                dong_view_set_external_gpu_device(app->dong_view, app->gpu_device, app->window);
                printf("[DongApp] Dong view created with GPU rendering\n");
            }
        }

        // Create blit pipeline for rendering texture to swapchain
        if (!create_blit_pipeline(app)) {
            fprintf(stderr, "[DongApp] Warning: Failed to create blit pipeline\n");
        }
    }

    // Initialize timing
    app->last_frame_time = SDL_GetPerformanceCounter();
    app->delta_time = 0.016f;  // Default 60fps
    app->running = 1;

    return (dong_app_t*)app;
}

DONG_APPCORE_API void dong_app_destroy(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app) return;

    // Destroy blit resources
    if (app->blit_pipeline) {
        SDL_ReleaseGPUGraphicsPipeline(app->gpu_device, app->blit_pipeline);
        app->blit_pipeline = NULL;
    }
    if (app->blit_sampler) {
        SDL_ReleaseGPUSampler(app->gpu_device, app->blit_sampler);
        app->blit_sampler = NULL;
    }

    // Destroy Dong view
    if (app->dong_view) {
        dong_view_free(app->dong_view);
        app->dong_view = NULL;
    }
    if (app->dong_ctx) {
        dong_destroy_context(app->dong_ctx);
        app->dong_ctx = NULL;
    }

    // Destroy GPU device (releases window claim)
    if (app->gpu_device) {
        SDL_DestroyGPUDevice(app->gpu_device);
        app->gpu_device = NULL;
    }

    // Destroy window
    if (app->window) {
        SDL_DestroyWindow(app->window);
        app->window = NULL;
    }

    SDL_Quit();
    free(app);
}

DONG_APPCORE_API int dong_app_is_running(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->running : 0;
}

DONG_APPCORE_API void dong_app_quit(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (app) {
        app->running = 0;
    }
}

DONG_APPCORE_API void dong_app_run(dong_app_t* app_handle, dong_app_tick_fn tick, void* user_data) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app) return;

    while (app->running) {
        if (!dong_app_poll_events(app_handle)) {
            break;
        }

        float dt = dong_app_get_delta_time(app_handle);

        if (tick) {
            tick(app_handle, dt, user_data);
        }

        dong_app_present(app_handle);
    }
}

DONG_APPCORE_API int dong_app_poll_events(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app) return 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                app->running = 0;
                return 0;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                app->running = 0;
                return 0;

            case SDL_EVENT_KEY_DOWN:
                // ESC to quit
                if (event.key.key == SDLK_ESCAPE) {
                    app->running = 0;
                    return 0;
                }
                if (app->dong_view) {
                    dong_view_send_key_down(app->dong_view, event.key.key);
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (app->dong_view) {
                    dong_view_send_key_up(app->dong_view, event.key.key);
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                app->width = event.window.data1;
                app->height = event.window.data2;
                if (app->dong_view) {
                    dong_view_resize(app->dong_view, app->width, app->height);
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                app->mouse_x = (int32_t)event.motion.x;
                app->mouse_y = (int32_t)event.motion.y;
                if (app->dong_view) {
                    dong_view_send_mouse_move(app->dong_view, app->mouse_x, app->mouse_y);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (app->dong_view) {
                    dong_view_send_mouse_down(app->dong_view, event.button.button);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (app->dong_view) {
                    dong_view_send_mouse_up(app->dong_view, event.button.button);
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                if (app->dong_view) {
                    // SDL wheel y positive = scroll up, dong expects positive = scroll down
                    dong_view_send_mouse_wheel(app->dong_view, event.wheel.x, -event.wheel.y);
                }
                break;

            case SDL_EVENT_TEXT_INPUT:
                if (app->dong_view) {
                    dong_view_send_text_input(app->dong_view, event.text.text);
                }
                break;
        }
    }

    // Update delta time
    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();
    app->delta_time = (float)(now - app->last_frame_time) / (float)freq;
    app->last_frame_time = now;

    return app->running;
}

DONG_APPCORE_API float dong_app_get_delta_time(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->delta_time : 0.016f;
}

DONG_APPCORE_API int dong_app_begin_frame(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !app->gpu_device) return 0;
    // Frame begin is handled implicitly by SDL_GPU
    return 1;
}

DONG_APPCORE_API void dong_app_present(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !app->gpu_device || !app->window) return;

    // Render Dong content to texture
    SDL_GPUTexture* dong_texture = NULL;
    if (app->dong_view) {
        dong_texture = (SDL_GPUTexture*)dong_view_render_to_gpu_texture(
            app->dong_view, app->gpu_device, app->width, app->height);
    }

    // Acquire command buffer and swapchain texture
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(app->gpu_device);
    if (!cmd) return;

    SDL_GPUTexture* swapchain_texture = NULL;
    if (!SDL_AcquireGPUSwapchainTexture(cmd, app->window, &swapchain_texture, NULL, NULL)) {
        SDL_CancelGPUCommandBuffer(cmd);
        return;
    }

    if (swapchain_texture) {
        SDL_GPUColorTargetInfo color_info = {0};
        color_info.texture = swapchain_texture;
        color_info.store_op = SDL_GPU_STOREOP_STORE;

        if (dong_texture && app->blit_pipeline && app->blit_sampler) {
            // Blit Dong texture to swapchain (no clear needed)
            color_info.load_op = SDL_GPU_LOADOP_DONT_CARE;

            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, NULL);
            if (pass) {
                SDL_BindGPUGraphicsPipeline(pass, app->blit_pipeline);

                SDL_GPUTextureSamplerBinding tex_binding = {0};
                tex_binding.texture = dong_texture;
                tex_binding.sampler = app->blit_sampler;
                SDL_BindGPUFragmentSamplers(pass, 0, &tex_binding, 1);

                SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
                SDL_EndGPURenderPass(pass);
            }
        } else {
            // No Dong content, clear to dark gray
            color_info.load_op = SDL_GPU_LOADOP_CLEAR;
            color_info.clear_color.r = 0.1f;
            color_info.clear_color.g = 0.1f;
            color_info.clear_color.b = 0.1f;
            color_info.clear_color.a = 1.0f;

            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, NULL);
            if (pass) {
                SDL_EndGPURenderPass(pass);
            }
        }
    }

    SDL_SubmitGPUCommandBuffer(cmd);
}

DONG_APPCORE_API void dong_app_get_size(dong_app_t* app_handle, uint32_t* out_width, uint32_t* out_height) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (out_width) *out_width = app ? app->width : 0;
    if (out_height) *out_height = app ? app->height : 0;
}

DONG_APPCORE_API void dong_app_get_mouse_position(dong_app_t* app_handle, int32_t* out_x, int32_t* out_y) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (out_x) *out_x = app ? app->mouse_x : 0;
    if (out_y) *out_y = app ? app->mouse_y : 0;
}

DONG_APPCORE_API dong_renderer_t* dong_app_get_renderer(dong_app_t* app_handle) {
    (void)app_handle;
    return NULL;  // Not implemented yet
}

DONG_APPCORE_API void* dong_app_get_gpu_device(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->gpu_device : NULL;
}

DONG_APPCORE_API void* dong_app_get_window(dong_app_t* app_handle) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->window : NULL;
}

DONG_APPCORE_API void* dong_app_get_dong_engine(dong_app_t* app_handle) {
    // Return dong_view for compatibility (internal View API)
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    return app ? app->dong_view : NULL;
}

DONG_APPCORE_API int dong_app_load_html(dong_app_t* app_handle, const char* html) {
    dong_app_impl_t* app = (dong_app_impl_t*)app_handle;
    if (!app || !html) return 0;

    if (app->dong_view) {
        dong_view_load_html(app->dong_view, html);
        return 1;
    }
    return 0;
}

DONG_APPCORE_API int dong_app_load_html_file(dong_app_t* app_handle, const char* path) {
    if (!path) return 0;

    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[DongApp] Failed to open file: %s\n", path);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(f);
        return 0;
    }

    size_t read = fread(content, 1, size, f);
    content[read] = '\0';
    fclose(f);

    int result = dong_app_load_html(app_handle, content);
    free(content);
    return result;
}

// =============================================================================
// Blit Pipeline Creation
// =============================================================================

// Need SDL_ShaderCross for HLSL compilation
#include <SDL3_shadercross/SDL_shadercross.h>

static SDL_GPUShader* compile_shader(SDL_GPUDevice* device, SDL_GPUShaderStage stage,
                                      const char* hlsl_source, const char* entry_point,
                                      int num_samplers, int num_uniform_buffers) {
    SDL_ShaderCross_HLSL_Info hlsl_info;
    SDL_zero(hlsl_info);
    hlsl_info.source = hlsl_source;
    hlsl_info.entrypoint = entry_point;
    hlsl_info.shader_stage = (stage == SDL_GPU_SHADERSTAGE_VERTEX)
                              ? SDL_SHADERCROSS_SHADERSTAGE_VERTEX
                              : SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
    hlsl_info.include_dir = NULL;
    hlsl_info.defines = NULL;
    hlsl_info.props = 0;

    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat target_format = SDL_GPU_SHADERFORMAT_INVALID;
    const char* format_name = "unknown";

    if (formats & SDL_GPU_SHADERFORMAT_DXIL) {
        target_format = SDL_GPU_SHADERFORMAT_DXIL;
        format_name = "DXIL";
    } else if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
        target_format = SDL_GPU_SHADERFORMAT_SPIRV;
        format_name = "SPIRV";
    } else if (formats & SDL_GPU_SHADERFORMAT_MSL) {
        target_format = SDL_GPU_SHADERFORMAT_MSL;
        format_name = "MSL";
    }

    if (target_format == SDL_GPU_SHADERFORMAT_INVALID) {
        fprintf(stderr, "[DongApp] No supported shader format found\n");
        return NULL;
    }

    SDL_GPUShaderCreateInfo shader_info;
    SDL_zero(shader_info);
    shader_info.stage = stage;
    shader_info.num_samplers = num_samplers;
    shader_info.num_uniform_buffers = num_uniform_buffers;
    shader_info.entrypoint = entry_point;
    shader_info.format = target_format;

    void* bytecode = NULL;
    size_t bytecode_size = 0;

    if (target_format == SDL_GPU_SHADERFORMAT_DXIL) {
        bytecode = SDL_ShaderCross_CompileDXILFromHLSL(&hlsl_info, &bytecode_size);
    } else if (target_format == SDL_GPU_SHADERFORMAT_SPIRV) {
        bytecode = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &bytecode_size);
    } else if (target_format == SDL_GPU_SHADERFORMAT_MSL) {
        // MSL requires SPIRV first, then transpile
        bytecode = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &bytecode_size);
    }

    if (!bytecode || bytecode_size == 0) {
        fprintf(stderr, "[DongApp] Failed to compile shader to %s\n", format_name);
        return NULL;
    }

    shader_info.code = bytecode;
    shader_info.code_size = bytecode_size;

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shader_info);
    SDL_free(bytecode);

    if (!shader) {
        fprintf(stderr, "[DongApp] SDL_CreateGPUShader failed: %s\n", SDL_GetError());
    }

    return shader;
}

static int create_blit_pipeline(dong_app_impl_t* app) {
    if (!app || !app->gpu_device || !app->window) return 0;

    // Initialize SDL_ShaderCross
    if (!SDL_ShaderCross_Init()) {
        fprintf(stderr, "[DongApp] SDL_ShaderCross_Init failed\n");
        return 0;
    }

    // Compile shaders
    SDL_GPUShader* vs = compile_shader(app->gpu_device, SDL_GPU_SHADERSTAGE_VERTEX,
                                        BLIT_VS_HLSL, "main", 0, 0);
    SDL_GPUShader* fs = compile_shader(app->gpu_device, SDL_GPU_SHADERSTAGE_FRAGMENT,
                                        BLIT_FS_HLSL, "main", 1, 0);

    if (!vs || !fs) {
        if (vs) SDL_ReleaseGPUShader(app->gpu_device, vs);
        if (fs) SDL_ReleaseGPUShader(app->gpu_device, fs);
        return 0;
    }

    // Create sampler
    SDL_GPUSamplerCreateInfo sampler_info = {0};
    sampler_info.min_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.mag_filter = SDL_GPU_FILTER_LINEAR;
    sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    app->blit_sampler = SDL_CreateGPUSampler(app->gpu_device, &sampler_info);

    // Create pipeline
    SDL_GPUTextureFormat swapchain_format = SDL_GetGPUSwapchainTextureFormat(app->gpu_device, app->window);

    SDL_GPUColorTargetDescription color_desc = {0};
    color_desc.format = swapchain_format;
    color_desc.blend_state.enable_blend = false;

    SDL_GPUGraphicsPipelineCreateInfo pipe_info = {0};
    pipe_info.vertex_shader = vs;
    pipe_info.fragment_shader = fs;
    pipe_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipe_info.target_info.num_color_targets = 1;
    pipe_info.target_info.color_target_descriptions = &color_desc;
    pipe_info.target_info.has_depth_stencil_target = false;
    pipe_info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pipe_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    app->blit_pipeline = SDL_CreateGPUGraphicsPipeline(app->gpu_device, &pipe_info);

    // Release shaders (no longer needed after pipeline creation)
    SDL_ReleaseGPUShader(app->gpu_device, vs);
    SDL_ReleaseGPUShader(app->gpu_device, fs);

    if (!app->blit_pipeline) {
        fprintf(stderr, "[DongApp] Failed to create blit pipeline: %s\n", SDL_GetError());
        return 0;
    }

    printf("[DongApp] Blit pipeline created successfully\n");
    return 1;
}
