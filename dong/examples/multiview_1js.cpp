/**
 * Multi-View Shared JS Demo
 *
 * Demonstrates multiple EngineViews sharing a single JavaScript context.
 * - "main" view (left):    control panel with buttons and text input
 * - "display" view (right): display panel driven by JS from the main view
 *
 * Both views render to offscreen textures and are composited side-by-side
 * via SDL_BlitGPUTexture.
 *
 * JS API:
 *   document / window          -> main view (default, backward compatible)
 *   dong.getView("display")    -> display view's window object
 *     .document.getElementById -> display view's DOM
 */

#include "dong_app.h"
#include "dong.h"
#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_sdl_platform.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* HTML content                                                        */
/* ------------------------------------------------------------------ */

static const char* MAIN_HTML =
    "<!DOCTYPE html><html><head><style>"
    "* { box-sizing: border-box; }"
    "body { margin: 0; padding: 16px; background-color: #1e293b; color: #e2e8f0; font-family: Arial, sans-serif; }"
    "h2 { font-size: 20px; color: #38bdf8; margin: 0 0 16px 0; }"
    ".btn-row { display: flex; gap: 10px; margin-bottom: 12px; }"
    "button { padding: 10px 20px; background-color: #3b82f6; color: #fff; border-radius: 6px; font-size: 14px; }"
    "#status { margin-top: 12px; padding: 10px; background-color: #0f172a; border-radius: 6px; font-size: 13px; color: #94a3b8; }"
    "input { width: 100%; padding: 10px; background-color: #0f172a; color: #e2e8f0; border-radius: 6px; font-size: 14px; margin-bottom: 12px; }"
    "</style></head><body>"
    "<h2>Control Panel (main)</h2>"
    "<div class=\"btn-row\">"
    "  <button id=\"btn-hello\">Say Hello</button>"
    "  <button id=\"btn-time\">Show Time</button>"
    "  <button id=\"btn-clear\">Clear</button>"
    "</div>"
    "<input id=\"msg-input\" type=\"text\" placeholder=\"Type a message for display view...\" />"
    "<div id=\"status\">Ready.</div>"
    "</body></html>";

static const char* DISPLAY_HTML =
    "<!DOCTYPE html><html><head><style>"
    "* { box-sizing: border-box; }"
    "body { margin: 0; padding: 16px; background-color: #0f172a; color: #e2e8f0; font-family: Arial, sans-serif; }"
    "h2 { font-size: 20px; color: #a78bfa; margin: 0 0 16px 0; }"
    "#content { padding: 16px; background-color: #1e1b4b; border-radius: 8px; font-size: 16px; min-height: 80px; color: #c4b5fd; }"
    "#counter { margin-top: 12px; font-size: 13px; color: #64748b; }"
    "</style></head><body>"
    "<h2>Display Panel (display)</h2>"
    "<div id=\"content\">Waiting for input from control panel...</div>"
    "<div id=\"counter\">Messages received: 0</div>"
    "</body></html>";

static const char* SHARED_JS =
    "var msgCount = 0;\n"
    "var displayWin = dong.getView('display');\n"
    "var displayDoc = displayWin.document;\n"
    "\n"
    "function updateDisplay(text) {\n"
    "    msgCount++;\n"
    "    displayDoc.getElementById('content').textContent = text;\n"
    "    displayDoc.getElementById('counter').textContent = 'Messages received: ' + msgCount;\n"
    "    document.getElementById('status').textContent = 'Sent: ' + text;\n"
    "}\n"
    "\n"
    "document.getElementById('btn-hello').addEventListener('click', function() {\n"
    "    updateDisplay('Hello from the control panel!');\n"
    "});\n"
    "document.getElementById('btn-time').addEventListener('click', function() {\n"
    "    updateDisplay('Time: ' + new Date().toLocaleTimeString());\n"
    "});\n"
    "document.getElementById('btn-clear').addEventListener('click', function() {\n"
    "    displayDoc.getElementById('content').textContent = 'Waiting for input from control panel...';\n"
    "    document.getElementById('status').textContent = 'Cleared.';\n"
    "});\n"
    "document.getElementById('msg-input').addEventListener('input', function() {\n"
    "    var val = document.getElementById('msg-input').value;\n"
    "    if (val) updateDisplay(val);\n"
    "});\n"
    "document.getElementById('status').textContent = 'Shared JS loaded. Both views connected.';\n";

/* ------------------------------------------------------------------ */
/* Global state for event routing                                      */
/* ------------------------------------------------------------------ */

static const uint32_t WIN_W = 900, WIN_H = 500;
static const uint32_t VIEW_W = WIN_W / 2, VIEW_H = WIN_H;

static dong_engine_t* g_eng_main    = NULL;
static dong_engine_t* g_eng_display = NULL;

static dong_engine_t* pick_engine(int32_t x) {
    return (x < (int32_t)VIEW_W) ? g_eng_main : g_eng_display;
}

static int32_t to_local_x(dong_engine_t* eng, int32_t x) {
    return (eng == g_eng_main) ? x : x - (int32_t)VIEW_W;
}

static void event_callback(void* /*user_data*/, const dong_app_event_t* ev) {
    if (!ev) return;

    dong_engine_t* eng;
    int32_t lx;

    switch (ev->type) {
        case DONG_APP_EVENT_MOUSE_MOVE:
            eng = pick_engine(ev->mouse_move.x);
            lx  = to_local_x(eng, ev->mouse_move.x);
            dong_engine_send_mouse_move(eng, lx, ev->mouse_move.y);
            break;

        case DONG_APP_EVENT_MOUSE_BUTTON:
            eng = pick_engine(ev->mouse_button.x);
            lx  = to_local_x(eng, ev->mouse_button.x);
            dong_engine_send_mouse_move(eng, lx, ev->mouse_button.y);
            dong_engine_send_mouse_button(eng, ev->mouse_button.button, ev->mouse_button.pressed);
            break;

        case DONG_APP_EVENT_MOUSE_WHEEL:
            eng = pick_engine(ev->mouse_wheel.x);
            lx  = to_local_x(eng, ev->mouse_wheel.x);
            dong_engine_send_mouse_move(eng, lx, ev->mouse_wheel.y);
            dong_engine_send_mouse_wheel(eng, ev->mouse_wheel.delta_x, ev->mouse_wheel.delta_y);
            break;

        case DONG_APP_EVENT_KEY:
            dong_engine_send_key(g_eng_main, ev->key.key_code, ev->key.pressed);
            break;

        case DONG_APP_EVENT_TEXT:
            dong_engine_send_text(g_eng_main, ev->text.text);
            break;

        case DONG_APP_EVENT_TEXT_EDITING:
            dong_engine_send_text_editing(g_eng_main, ev->text_editing.text,
                                          ev->text_editing.cursor,
                                          ev->text_editing.selection_length);
            break;

        default:
            break;
    }
}

/* ------------------------------------------------------------------ */
/* Offscreen texture helper                                            */
/* ------------------------------------------------------------------ */

static SDL_GPUTexture* create_offscreen_texture(SDL_GPUDevice* dev, uint32_t w, uint32_t h) {
    SDL_GPUTextureCreateInfo ci;
    SDL_zero(ci);
    ci.type                 = SDL_GPU_TEXTURETYPE_2D;
    ci.format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    ci.usage                = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    ci.width                = w;
    ci.height               = h;
    ci.layer_count_or_depth = 1;
    ci.num_levels           = 1;
    ci.sample_count         = SDL_GPU_SAMPLECOUNT_1;
    return SDL_CreateGPUTexture(dev, &ci);
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    printf("=== Multi-View Shared JS Demo ===\n");

    /* 1. Create AppCore window (no built-in dong engine) */
    dong_app_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.title       = "Multi-View Shared JS";
    cfg.width       = WIN_W;
    cfg.height      = WIN_H;
    cfg.enable_dong = 0;
    cfg.resizable   = 0;

    dong_app_t* app = dong_app_create(&cfg);
    if (!app) { fprintf(stderr, "dong_app_create failed\n"); return 1; }

    SDL_GPUDevice* dev = (SDL_GPUDevice*)dong_app_get_gpu_device(app);
    SDL_Window*    win = (SDL_Window*)dong_app_get_window(app);

    /* 2. Initialize dong platform (normally done by enable_dong=1) */
    if (!dong_sdl_platform_init(dev, win)) {
        fprintf(stderr, "dong_sdl_platform_init failed\n");
        dong_app_destroy(app);
        return 1;
    }

    /* 3. Create primary engine ("main", owns JS) */
    dong_engine_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    desc.api_version        = DONG_API_VERSION;
    desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
    desc.width  = VIEW_W;
    desc.height = VIEW_H;

    if (dong_engine_create(&desc, &g_eng_main) != DONG_OK) {
        fprintf(stderr, "main engine create failed\n");
        dong_app_destroy(app); return 1;
    }
    dong_engine_set_gpu(g_eng_main, dev, win);

    /* 4. Create secondary engine ("display", shares JS) */
    if (dong_engine_create_shared_js(&desc, g_eng_main, "display", &g_eng_display) != DONG_OK) {
        fprintf(stderr, "display engine create failed\n");
        dong_engine_destroy(g_eng_main);
        dong_app_destroy(app); return 1;
    }
    dong_engine_set_gpu(g_eng_display, dev, win);

    /* 5. Load HTML and eval shared JS */
    dong_engine_load_html(g_eng_main, MAIN_HTML);
    dong_engine_load_html(g_eng_display, DISPLAY_HTML);
    dong_engine_eval_script(g_eng_main, SHARED_JS);

    /* 6. Create offscreen textures */
    SDL_GPUTexture* tex_main    = create_offscreen_texture(dev, VIEW_W, VIEW_H);
    SDL_GPUTexture* tex_display = create_offscreen_texture(dev, VIEW_W, VIEW_H);
    if (!tex_main || !tex_display) {
        fprintf(stderr, "offscreen texture creation failed\n");
        dong_engine_destroy(g_eng_display);
        dong_engine_destroy(g_eng_main);
        dong_app_destroy(app); return 1;
    }

    /* 7. Set up input routing via event callback */
    dong_app_set_event_callback(app, event_callback, NULL);
    dong_app_enable_text_input(app, 1);

    printf("Running. Left = control, Right = display. Close window to exit.\n");

    DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());

    /* 8. Main loop */
    while (dong_app_is_running(app)) {
        if (!dong_app_poll_events(app)) break;

        /* --- offscreen render: main view --- */
        dong_gpu_begin_frame_offscreen(driver, (DongGPUTexture)tex_main, VIEW_W, VIEW_H);
        dong_engine_tick(g_eng_main);
        dong_gpu_end_frame_offscreen(driver);

        /* --- offscreen render: display view --- */
        dong_gpu_begin_frame_offscreen(driver, (DongGPUTexture)tex_display, VIEW_W, VIEW_H);
        dong_engine_tick(g_eng_display);
        dong_gpu_end_frame_offscreen(driver);

        /* --- composite to swapchain --- */
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(dev);
        if (!cmd) continue;

        SDL_GPUTexture* swapchain = NULL;
        uint32_t sw = 0, sh = 0;
        if (!SDL_AcquireGPUSwapchainTexture(cmd, win, &swapchain, &sw, &sh) || !swapchain) {
            SDL_CancelGPUCommandBuffer(cmd);
            continue;
        }

        /* Clear swapchain */
        {
            SDL_GPUColorTargetInfo cti;
            SDL_zero(cti);
            cti.texture     = swapchain;
            cti.load_op     = SDL_GPU_LOADOP_CLEAR;
            cti.store_op    = SDL_GPU_STOREOP_STORE;
            cti.clear_color = {0.08f, 0.08f, 0.12f, 1.0f};
            SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &cti, 1, NULL);
            if (pass) SDL_EndGPURenderPass(pass);
        }

        /* Blit left half: main */
        {
            SDL_GPUBlitInfo blit;
            SDL_zero(blit);
            blit.source.texture = tex_main;
            blit.source.w       = VIEW_W;
            blit.source.h       = VIEW_H;
            blit.destination.texture = swapchain;
            blit.destination.x  = 0;
            blit.destination.y  = 0;
            blit.destination.w  = VIEW_W;
            blit.destination.h  = VIEW_H;
            blit.load_op        = SDL_GPU_LOADOP_LOAD;
            blit.filter         = SDL_GPU_FILTER_LINEAR;
            SDL_BlitGPUTexture(cmd, &blit);
        }

        /* Blit right half: display */
        {
            SDL_GPUBlitInfo blit;
            SDL_zero(blit);
            blit.source.texture = tex_display;
            blit.source.w       = VIEW_W;
            blit.source.h       = VIEW_H;
            blit.destination.texture = swapchain;
            blit.destination.x  = VIEW_W;
            blit.destination.y  = 0;
            blit.destination.w  = VIEW_W;
            blit.destination.h  = VIEW_H;
            blit.load_op        = SDL_GPU_LOADOP_LOAD;
            blit.filter         = SDL_GPU_FILTER_LINEAR;
            SDL_BlitGPUTexture(cmd, &blit);
        }

        SDL_SubmitGPUCommandBuffer(cmd);
    }

    /* 9. Cleanup */
    if (tex_display) SDL_ReleaseGPUTexture(dev, tex_display);
    if (tex_main)    SDL_ReleaseGPUTexture(dev, tex_main);
    dong_engine_destroy(g_eng_display);
    dong_engine_destroy(g_eng_main);
    /* Note: dong_app_destroy() calls dong_sdl_platform_shutdown() internally */
    dong_app_destroy(app);

    printf("=== Demo Complete ===\n");
    return 0;
}
