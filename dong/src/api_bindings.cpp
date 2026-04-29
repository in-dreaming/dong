#define DONG_DISABLE_VIEW_API
#include "dong.h"

#include "core/engine_view.hpp"
#include "core/profiler.h"
#include "render/text_renderer_mode.hpp"

#include <cstdio>
#include <cstdlib>
#include <memory>

namespace {

static inline bool pluginCanLog(const dong_plugin_vtable_t* plugin) {
    return plugin && plugin->log;
}

static inline void pluginLog(const dong_plugin_vtable_t* plugin, void* plugin_user,
                             dong_log_level_t level, const char* msg) {
    if (!pluginCanLog(plugin) || !msg) {
        return;
    }
    plugin->log(plugin_user, level, msg);
}

} // namespace

extern "C" {

struct dong_engine_t {
    const dong_plugin_vtable_t* plugin = nullptr;
    void* plugin_user = nullptr;

    uint32_t width = 960;
    uint32_t height = 540;

    std::unique_ptr<dong::EngineView> view;
};

static inline dong_engine_t* asEngine(dong_engine_t* engine) {
    return engine;
}

static inline const dong_engine_t* asEngineConst(const dong_engine_t* engine) {
    return engine;
}

dong_result_t dong_engine_create(const dong_engine_desc_t* desc, dong_engine_t** out_engine) {
    if (!desc || !out_engine) {
        return DONG_ERR_INVALID_ARG;
    }
    *out_engine = nullptr;

    if (desc->api_version != DONG_API_VERSION) {
        return DONG_ERR_VERSION_MISMATCH;
    }

    if (desc->plugin) {
        if (desc->plugin_api_version != DONG_PLUGIN_API_VERSION) {
            return DONG_ERR_VERSION_MISMATCH;
        }
        if (desc->plugin->info.plugin_api_version != DONG_PLUGIN_API_VERSION) {
            return DONG_ERR_VERSION_MISMATCH;
        }
    }

    const uint32_t w = desc->width > 0 ? desc->width : 960;
    const uint32_t h = desc->height > 0 ? desc->height : 540;

    try {
        auto* engine = new dong_engine_t();
        engine->plugin = desc->plugin;
        engine->plugin_user = desc->plugin_user;
        engine->width = w;
        engine->height = h;

        engine->view = std::make_unique<dong::EngineView>(w, h);
        if (!engine->view || !engine->view->isInitialized()) {
            delete engine;
            return DONG_ERR_INTERNAL;
        }

        if (engine->plugin) {
            engine->view->setPlugin(engine->plugin, engine->plugin_user);
        }

        if (desc->html) {
            (void)engine->view->loadHTML(desc->html);
        }


        if (pluginCanLog(engine->plugin)) {
            char msg[256];
            std::snprintf(msg, sizeof(msg), "Engine created (%ux%u, html=%s)",
                          engine->width, engine->height, desc->html ? "loaded" : "none");
            pluginLog(engine->plugin, engine->plugin_user, DONG_LOG_INFO, msg);
        }

        *out_engine = engine;
        return DONG_OK;
    } catch (...) {
        *out_engine = nullptr;
        return DONG_ERR_INTERNAL;
    }
}

void dong_engine_destroy(dong_engine_t* engine) {
    auto* e = asEngine(engine);
    if (!e) {
        return;
    }

    if (dong_profiler_is_enabled()) {
        const char* path = std::getenv("DONG_PROFILE_OUTPUT");
        const char* out = (path && path[0]) ? path : "dong_profile.json";
        if (dong_profiler_dump(out) == 0) {
            fprintf(stderr, "[Profiler] Trace saved to %s\n", out);
        }
    }

    if (pluginCanLog(e->plugin)) {
        pluginLog(e->plugin, e->plugin_user, DONG_LOG_INFO, "Engine destroyed");
    }

    delete e;
}

dong_result_t dong_engine_tick(dong_engine_t* engine) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    return e->view->tick() ? DONG_OK : DONG_ERR_INTERNAL;
}

dong_result_t dong_engine_load_html(dong_engine_t* engine, const char* html) {
    auto* e = asEngine(engine);
    if (!e || !e->view || !html) {
        return DONG_ERR_INVALID_ARG;
    }

    if (!e->view->loadHTML(html)) {
        return DONG_ERR_INTERNAL;
    }

    if (pluginCanLog(e->plugin)) {
        pluginLog(e->plugin, e->plugin_user, DONG_LOG_INFO, "HTML loaded");
    }

    return DONG_OK;
}

dong_result_t dong_engine_resize(dong_engine_t* engine, uint32_t width, uint32_t height) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->width = width;
    e->height = height;
    e->view->resize(width, height);
    return DONG_OK;
}

dong_result_t dong_engine_set_gpu(dong_engine_t* engine, void* gpu_device, void* window) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    if (!e->view->setGPU(gpu_device, window)) {
        return DONG_ERR_INTERNAL;
    }

    if (pluginCanLog(e->plugin)) {
        pluginLog(e->plugin, e->plugin_user, DONG_LOG_INFO, "GPU device set");
    }

    return DONG_OK;
}

dong_result_t dong_engine_set_resource_root(dong_engine_t* engine, const char* root) {
    auto* e = asEngine(engine);
    if (!e || !e->view || !root) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->setResourceRoot(root);
    return DONG_OK;
}

const char* dong_engine_get_cursor_at(dong_engine_t* engine, int32_t x, int32_t y) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return "auto";
    }

    return e->view->getCursorAt(x, y);
}

dong_result_t dong_engine_send_mouse_move(dong_engine_t* engine, int32_t x, int32_t y) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->sendMouseMove(x, y);
    return DONG_OK;
}

dong_result_t dong_engine_send_mouse_button(dong_engine_t* engine, int32_t button, int pressed) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->sendMouseButton(button, pressed != 0);
    return DONG_OK;
}

dong_result_t dong_engine_send_mouse_wheel(dong_engine_t* engine, float delta_x, float delta_y) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->sendMouseWheel(delta_x, delta_y);
    return DONG_OK;
}

dong_result_t dong_engine_send_key(dong_engine_t* engine, uint32_t key_code, int pressed) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->sendKey(key_code, pressed != 0);
    return DONG_OK;
}

dong_result_t dong_engine_send_text(dong_engine_t* engine, const char* text) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->sendText(text);
    return DONG_OK;
}

dong_result_t dong_engine_send_text_editing(dong_engine_t* engine, const char* text, int32_t cursor, int32_t selection_length) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->sendTextEditing(text, cursor, selection_length);
    return DONG_OK;
}

dong_result_t dong_engine_eval_script(dong_engine_t* engine, const char* code) {
    auto* e = asEngine(engine);
    if (!e || !e->view || !code) {
        return DONG_ERR_INVALID_ARG;
    }

    return e->view->evalScript(code) ? DONG_OK : DONG_ERR_INTERNAL;
}

const void* dong_engine_get_command_list(dong_engine_t* engine) {
    const auto* e = asEngineConst(engine);
    if (!e || !e->view) {
        return nullptr;
    }

    return e->view->getCommandList();
}

void dong_engine_invalidate_commands(dong_engine_t* engine) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return;
    }

    e->view->invalidateCommands();
}

uint32_t dong_get_api_version(void) {
    return DONG_API_VERSION;
}

dong_result_t dong_engine_set_text_renderer_mode(dong_engine_t* engine,
                                                  dong_text_renderer_mode_t mode) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->setTextRendererMode(static_cast<dong::render::TextRendererMode>(mode));
    return DONG_OK;
}

dong_text_renderer_mode_t dong_engine_get_text_renderer_mode(dong_engine_t* engine) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_TEXT_RENDERER_AUTO;
    }

    return static_cast<dong_text_renderer_mode_t>(e->view->getTextRendererMode());
}

dong_result_t dong_engine_focus_nav(dong_engine_t* engine, dong_nav_dir_t dir) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    bool moved = e->view->focusNav(static_cast<int>(dir));
    return moved ? DONG_OK : DONG_ERR_INVALID_ARG;
}

dong_result_t dong_engine_send_gamepad_button(dong_engine_t* engine,
                                              int32_t gamepad_id,
                                              dong_gamepad_button_t button,
                                              int pressed) {
    auto* e = asEngine(engine);
    if (!e || !e->view) {
        return DONG_ERR_INVALID_ARG;
    }

    e->view->sendGamepadButton(gamepad_id, static_cast<int>(button), pressed != 0);
    return DONG_OK;
}

dong_result_t dong_engine_create_shared_js(
    const dong_engine_desc_t* desc,
    dong_engine_t* script_source,
    const char* view_name,
    dong_engine_t** out_engine) {

    if (!desc || !script_source || !view_name || !out_engine) {
        return DONG_ERR_INVALID_ARG;
    }
    *out_engine = nullptr;

    if (desc->api_version != DONG_API_VERSION) {
        return DONG_ERR_VERSION_MISMATCH;
    }

    if (!script_source->view) {
        return DONG_ERR_INVALID_ARG;
    }

    auto* shared_se = script_source->view->getScriptEngine();
    if (!shared_se) {
        return DONG_ERR_INTERNAL;
    }

    const uint32_t w = desc->width > 0 ? desc->width : 960;
    const uint32_t h = desc->height > 0 ? desc->height : 540;

    try {
        auto* engine = new dong_engine_t();
        engine->plugin = desc->plugin ? desc->plugin : script_source->plugin;
        engine->plugin_user = desc->plugin ? desc->plugin_user : script_source->plugin_user;
        engine->width = w;
        engine->height = h;

        engine->view = std::make_unique<dong::EngineView>(w, h, shared_se);
        if (!engine->view || !engine->view->isInitialized()) {
            delete engine;
            return DONG_ERR_INTERNAL;
        }

        engine->view->setViewName(view_name);

        if (engine->plugin) {
            engine->view->setPlugin(engine->plugin, engine->plugin_user);
        }

        if (desc->html) {
            (void)engine->view->loadHTML(desc->html);
        }

        if (pluginCanLog(engine->plugin)) {
            char msg[256];
            std::snprintf(msg, sizeof(msg),
                          "Engine created (shared-js, %ux%u, view='%s')",
                          engine->width, engine->height, view_name);
            pluginLog(engine->plugin, engine->plugin_user, DONG_LOG_INFO, msg);
        }

        *out_engine = engine;
        return DONG_OK;
    } catch (...) {
        *out_engine = nullptr;
        return DONG_ERR_INTERNAL;
    }
}

} // extern "C"
