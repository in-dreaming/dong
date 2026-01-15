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
class FocusManager;
class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;
}

namespace dong::render {
class Painter;
class RenderSurface;
class ResourceManager;
class GPUDevice;
class GPUTextureSurfaceImpl;
class GPUPainter;
class ShaderManager;
class GPUDriver;
struct GPUCommandList;
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

    // Optional: configure a base directory for resolving relative resource URLs.
    void setResourceRoot(const std::string& root);

    void* get_pixel_buffer();
    render::RenderSurface* getRenderSurface() const { return render_surface.get(); }

    // Input events (forwarded from C API)
    void handle_mouse_move(int32_t x, int32_t y);
    void handle_mouse_down(int32_t button);
    void handle_mouse_up(int32_t button);
    void handle_mouse_wheel(float delta_x, float delta_y);
    void handle_key_down(uint32_t key_code);
    void handle_key_up(uint32_t key_code);
    void handle_text_input(const char* text);

    // Get the CSS cursor style at the given position
    const std::string& getCursorAt(int32_t x, int32_t y);

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
    // 底层接口：渲染到 GPU 纹理。
    // B: 纹理由 View 内部缓存并复用（避免每帧 Create/Release 造成的驱动开销）。
    // 返回的纹理指针在 view 生命周期内有效；调用方不要释放（也不应长期持有跨设备切换）。
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
    std::unique_ptr<dom::FocusManager> focus_manager;
    std::unique_ptr<render::Painter> painter;
    std::unique_ptr<render::ResourceManager> resource_manager_;
    std::unique_ptr<script::ScriptEngine> script_engine;
    std::unique_ptr<script::JSBindings> js_bindings;


    // GPU 渲染相关（可选）
    std::unique_ptr<render::GPUDevice> gpu_device_;
    std::unique_ptr<render::GPUTextureSurfaceImpl> gpu_surface_;
    std::unique_ptr<render::ShaderManager> shader_manager_;
    std::unique_ptr<render::GPUPainter> gpu_painter_;
    std::unique_ptr<render::GPUDriver> gpu_driver_;

    // 缓存的 GPU 命令列表（用于 swapchain 渲染时避免每帧重新编译）
    std::unique_ptr<render::GPUCommandList> cached_cmd_list_;
    
    // 离屏渲染缓存（优化策略1：避免每帧重建 DisplayList/GPUCommandList）
    std::unique_ptr<render::GPUCommandList> offscreen_cmd_list_cache_;
    uint32_t offscreen_cache_width_ = 0;
    uint32_t offscreen_cache_height_ = 0;
    bool offscreen_commands_dirty_ = true;
    bool offscreen_resources_dirty_ = true;


    // B：离屏纹理缓存（避免每次 renderToGPUTexture 都 Create/Release）
    SDL_GPUDevice* offscreen_texture_cache_device_ = nullptr; // non-owning
    SDL_GPUTexture* offscreen_texture_cache_ = nullptr;       // owned by View (released when possible)
    uint32_t offscreen_texture_cache_width_ = 0;
    uint32_t offscreen_texture_cache_height_ = 0;

    // 3D demo 友好：只有在内容变更（invalidate / update / 输入事件等）后才会重新执行离屏渲染。
    // 否则 renderToGPUTexture() 直接返回上一帧的纹理，避免每帧重复渲染。
    bool offscreen_texture_dirty_ = true;

    // 优化策略2：SDL_WaitForGPUIdle 仅首帧执行
    bool offscreen_first_frame_done_ = false;



    bool use_gpu_;
    // 标记当前 DisplayList / GPUCommandList 是否需要重建
    bool commands_dirty_ = true;
    bool js_bindings_initialized_ = false;
    int32_t last_mouse_x_ = 0;
    int32_t last_mouse_y_ = 0;

    // Runtime interaction states for pseudo-classes
    dom::DOMNodePtr hovered_element_;
    dom::DOMNodePtr active_element_;
    bool left_mouse_down_ = false;


    // Scrollbar dragging state (for overflow: auto/scroll)
    bool scroll_dragging_ = false;
    dom::DOMNodePtr scroll_drag_container_;
    float scroll_drag_offset_y_ = 0.0f;     // mouse_y - thumb_y
    float scroll_drag_track_y_ = 0.0f;
    float scroll_drag_track_h_ = 0.0f;
    float scroll_drag_thumb_h_ = 0.0f;
    float scroll_drag_max_scroll_ = 0.0f;

    // Cached cursor style for getCursorAt

    std::string cached_cursor_style_ = "auto";
    // 【缺口3】缓存最后的 eval 返回值
    std::string last_eval_return_value_;

    // 标记当前视图需要重新构建 DisplayList / GPUCommandList
    void markNeedsRepaint();
    // 标记离屏渲染缓存需要重建
    void markOffscreenDirty() { offscreen_commands_dirty_ = true; }
    void ensureJSBindingsInitialized();
    void dispatchMouseEventToJS(const char* type, int32_t x, int32_t y, int32_t button);
    void dispatchKeyEventToJS(const char* type, uint32_t key_code);
    void dispatchWheelEventToJS(float delta_x, float delta_y);
    void dispatchTextInputEventToJS(const char* text);
    
    // 查找滚动容器（overflow: scroll/auto）
    dom::DOMNodePtr findScrollContainerAt(int32_t x, int32_t y);
};

} // namespace dong
