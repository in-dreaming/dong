#pragma once

#include <cstdint>
#include <memory>

namespace dong::dom {
class Manager;
}

namespace dong::render {
class Painter;
class RenderSurface;
}

namespace dong::script {
class ScriptEngine;
class JSBindings;
}

namespace dong::layout {
class Engine;
}

namespace dong {

class View {
public:
    View(uint32_t width, uint32_t height);
    ~View();

    void load_html(const char* html);
    void resize(uint32_t width, uint32_t height);
    void update();
    void* get_pixel_buffer();

    // DOM access
    dom::Manager* getDOMManager() const { return dom_manager.get(); }

    // Script API
    bool eval_script(const char* code);

    // Rendering modes
    void setRenderMode(bool use_gpu);

private:
    uint32_t width_;
    uint32_t height_;
    std::unique_ptr<dom::Manager> dom_manager;
    std::unique_ptr<layout::Engine> layout_engine;
    std::unique_ptr<render::RenderSurface> render_surface;
    std::unique_ptr<render::Painter> painter;
    std::unique_ptr<script::ScriptEngine> script_engine;
    std::unique_ptr<script::JSBindings> js_bindings;
    bool use_gpu_;
};

} // namespace dong
