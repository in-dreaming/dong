#pragma once

#include <cstdint>
#include <memory>
#include <string>

// Forward declare SDL types
struct SDL_GPUDevice;
struct SDL_GPUTexture;
struct SDL_Window;

namespace dong::dom {
class Manager;
class EventDispatcher;
}

namespace dong::render {
class Painter;
class RenderSurface;
class GPUDevice;
class GPUTextureSurfaceImpl;
class GPUPainter;
class ShaderManager;
class GPUDriver;
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
    render::RenderSurface* getRenderSurface() const { return render_surface.get(); }

    // Input events (forwarded from C API)
    void handle_mouse_move(int32_t x, int32_t y);
    void handle_mouse_down(int32_t button);
    void handle_mouse_up(int32_t button);
    void handle_key_down(uint32_t key_code);
    void handle_key_up(uint32_t key_code);

    // DOM access
    dom::Manager* getDOMManager() const { return dom_manager.get(); }

    // Script API
    bool eval_script(const char* code);
    // 【缺口3】返回 JS 代码的执行结果
    std::string eval_script_with_return(const char* code);

    // Rendering modes
    void setRenderMode(bool use_gpu);
    void setExternalGPUDevice(SDL_GPUDevice* device, SDL_Window* window);
    
    // Offscreen rendering API
    // 底层接口：渲染到GPU纹理（调用者负责释放返回的纹理）
    SDL_GPUTexture* renderToGPUTexture(SDL_GPUDevice* device, uint32_t width, uint32_t height);
    
    // 上层接口：渲染到GPU纹理并回读到内存（基于 renderToGPUTexture 实现）
    bool renderOffscreen(SDL_GPUDevice* device, uint32_t width, uint32_t height, 
                        uint8_t* out_pixels);

private:
    uint32_t width_;
    uint32_t height_;
    std::unique_ptr<dom::Manager> dom_manager;
    std::unique_ptr<layout::Engine> layout_engine;
    std::unique_ptr<render::RenderSurface> render_surface;
    std::unique_ptr<dom::EventDispatcher> event_dispatcher;
    std::unique_ptr<render::Painter> painter;
    std::unique_ptr<script::ScriptEngine> script_engine;
    std::unique_ptr<script::JSBindings> js_bindings;

    // GPU 渲染相关（可选）
    std::unique_ptr<render::GPUDevice> gpu_device_;
    std::unique_ptr<render::GPUTextureSurfaceImpl> gpu_surface_;
    std::unique_ptr<render::ShaderManager> shader_manager_;
    std::unique_ptr<render::GPUPainter> gpu_painter_;
    std::unique_ptr<render::GPUDriver> gpu_driver_;

    bool use_gpu_;
    bool js_bindings_initialized_ = false;
    int32_t last_mouse_x_ = 0;
    int32_t last_mouse_y_ = 0;
    // 【缺口3】缓存最后的 eval 返回值
    std::string last_eval_return_value_;

    void ensureJSBindingsInitialized();
    void dispatchMouseEventToJS(const char* type, int32_t x, int32_t y, int32_t button);
    void dispatchKeyEventToJS(const char* type, uint32_t key_code);
};

} // namespace dong
