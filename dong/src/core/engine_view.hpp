#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "dong_plugin_api.h"
#include "../render/text_renderer_mode.hpp"

namespace dong::script {
class ScriptEngine;
}

namespace dong {

// Platform-agnostic "View" used by dong_engine_*.
// This is the single source of truth for DOM/Layout/Script + command generation.
class EngineView {
public:
    EngineView(uint32_t width, uint32_t height);

    // Create a view that shares another view's ScriptEngine (multi-view-1-JS).
    // The shared_script_engine must outlive this view.
    EngineView(uint32_t width, uint32_t height,
               dong::script::ScriptEngine* shared_script_engine);

    ~EngineView();

    EngineView(const EngineView&) = delete;
    EngineView& operator=(const EngineView&) = delete;

    bool loadHTML(const char* html);
    void resize(uint32_t width, uint32_t height);

    // Enable GPU path for command generation + (optionally) backend execution.
    bool setGPU(void* gpu_device, void* window);

    // Resource root for resolving relative assets/scripts.
    void setResourceRoot(const char* root);

    // Optional: inject platform plugin vtable (video, etc.).
    void setPlugin(const dong_plugin_vtable_t* plugin, void* plugin_user);

    // Drive one frame: update script/layout and (if needed) rebuild GPUCommandList.
    bool tick();


    bool isInitialized() const;

    // Cursor query (CSS cursor name; valid until next tick).
    const char* getCursorAt(int32_t x, int32_t y);

    // Input
    void sendMouseMove(int32_t x, int32_t y);

    void sendMouseButton(int32_t button, bool pressed);
    void sendMouseWheel(float delta_x, float delta_y);
    void sendKey(uint32_t key_code, bool pressed);
    void sendText(const char* text);
    void sendTextEditing(const char* text, int32_t cursor, int32_t selection_length);

    // URL fragment management for :target pseudo-class
    void setCurrentFragment(const std::string& fragment);

    // Script
    bool evalScript(const char* code);

    // Set a name for this view (used by dong.getView(name) in JS).
    void setViewName(const char* name);

    // Get the raw ScriptEngine pointer (for sharing with other views).
    dong::script::ScriptEngine* getScriptEngine() const;

    // Rendering
    const void* getCommandList() const;
    void invalidateCommands();

    // Text renderer mode configuration
    void setTextRendererMode(render::TextRendererMode mode);
    render::TextRendererMode getTextRendererMode() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace dong
