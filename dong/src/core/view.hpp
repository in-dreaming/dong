#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "dong_plugin_api.h"
#include "dong_gpu_driver.h"
#include "dong_surface.h"

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

    // Optional: debugging label (used for profiling/logging; no functional impact)
    void setDebugName(const std::string& name) { debug_name_ = name; }


    // Optional: inject platform plugin vtable (enables optional subsystems like video).
    void setPlugin(const dong_plugin_vtable_t* plugin, void* plugin_user);

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
    // Inject external GPU driver (replaces setExternalGPUDevice with SDL types)
    void setExternalGPUDriver(DongGPUDriver* driver, DongSurface* surface);
    
    // Offscreen rendering API
    // 底层接口：渲染到 GPU 纹理。
    // B: 纹理由 View 内部缓存并复用（避免每帧 Create/Release 造成的驱动开销）。
    // 返回的纹理指针在 view 生命周期内有效；调用方不要释放（也不应长期持有跨设备切换）。
    // Returns opaque DongGPUTexture handle. Use dong_gpu_get_native_texture_handle() to get native handle if needed.
    DongGPUTexture renderToGPUTexture(DongGPUDriver* driver, uint32_t width, uint32_t height);

    // 上层接口：渲染到GPU纹理并回读到内存（基于 renderToGPUTexture 实现）
    bool renderOffscreen(DongGPUDriver* driver, uint32_t width, uint32_t height,
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
    std::string debug_name_;
    std::unique_ptr<script::ScriptEngine> script_engine;

    std::unique_ptr<script::JSBindings> js_bindings;

    // Optional: platform plugin vtable (non-owning)
    const dong_plugin_vtable_t* plugin_ = nullptr;
    void* plugin_user_ = nullptr;

    // =============================================================================
    // Video (optional, provided by plugin)
    // =============================================================================
    struct VideoState {
        dong::dom::DOMNodePtr node;
        dong_video_player_t* player = nullptr;

        std::string src;
        std::string frame_key; // e.g. "video://<node_ptr>"

        dong_video_metadata_t meta{};
        bool meta_ready = false;
        bool loadeddata_sent = false;


        bool preload = true;   // preload != none
        bool autoplay = false;
        bool loop = false;
        bool controls = false;

        bool playing = false;
        bool ended = false;
        bool errored = false;

        double wall_clock_start = 0.0;
        double video_time_start = 0.0;
        double current_pts = 0.0;

        // Upload staging
        // NOTE: For the SDL+FFmpeg plugin, plane pointers stay valid until the next `video_read_frame()` call.
        dong_video_pixel_format_t frame_format = DONG_VIDEO_PIXEL_FORMAT_RGBA8;
        const uint8_t* plane_data[3] = {nullptr, nullptr, nullptr};
        uint32_t plane_stride[3] = {0, 0, 0};

        std::vector<uint8_t> owned_frame; // optional owned copy (fallback/debug)

        uint32_t frame_w = 0;
        uint32_t frame_h = 0;
        uint32_t frame_stride = 0; // convenience (RGBA stride or Y plane stride)
        bool has_frame = false;
        bool needs_upload = false;


        // Throttle `timeupdate` based on media time (pts), not wall-clock time.
        // Using wall-clock can create a feedback loop under low FPS: slow frames -> timeupdate every tick -> DOM dirtied every tick -> rebuild every frame.
        double last_timeupdate_pts = 0.0;


        // Playback resync (avoid decode spiral when render thread can't keep up)
        double last_resync_wall = 0.0;
    };

    std::unordered_map<void*, VideoState> video_states_;

    // Avoid scanning the whole DOM every frame when there are no <video> elements.
    bool video_dom_scanned_ = false;
    bool video_dom_has_any_ = false;

    void syncVideoElements(double wall_time_sec, bool dom_tree_dirty);
    void updateVideoPlayback(VideoState& vs, double wall_time_sec);

    void closeVideo(VideoState& vs);
    void dispatchMediaEvent(VideoState& vs, const char* type_name);
    void dispatchMediaError(VideoState& vs, const char* message);
    void toggleVideoPlayPause(VideoState& vs, double wall_time_sec);
    bool uploadPendingVideoFrames();


    // GPU 渲染相关（可选）- 通过注入的 DongGPUDriver 访问，不直接依赖 SDL
    DongGPUDriver* gpu_driver_ = nullptr;  // Non-owning, injected via setExternalGPUDriver
    DongSurface* gpu_surface_ = nullptr;   // Non-owning, injected via setExternalGPUDriver

    // 缓存的 GPU 命令列表（用于 swapchain 渲染时避免每帧重新编译）
    std::unique_ptr<render::GPUCommandList> cached_cmd_list_;
    
    // 离屏渲染缓存（优化策略1：避免每帧重建 DisplayList/GPUCommandList）
    std::unique_ptr<render::GPUCommandList> offscreen_cmd_list_cache_;
    uint32_t offscreen_cache_width_ = 0;
    uint32_t offscreen_cache_height_ = 0;
    bool offscreen_commands_dirty_ = true;
    bool offscreen_resources_dirty_ = true;


    // B：离屏纹理缓存（避免每次 renderToGPUTexture 都 Create/Release）
    DongGPUDriver* offscreen_texture_cache_driver_ = nullptr; // non-owning
    DongGPUTexture offscreen_texture_cache_ = nullptr;       // owned by View (released when possible)
    uint32_t offscreen_texture_cache_width_ = 0;
    uint32_t offscreen_texture_cache_height_ = 0;

    // 3D demo 友好：只有在内容变更（invalidate / update / 输入事件等）后才会重新执行离屏渲染。
    // 否则 renderToGPUTexture() 直接返回上一帧的纹理，避免每帧重复渲染。
    bool offscreen_texture_dirty_ = true;

    // 优化策略2：GPU idle 等待仅首帧执行
    bool offscreen_first_frame_done_ = false;



    bool use_gpu_;
    // 标记当前 DisplayList / GPUCommandList 是否需要重建
    bool commands_dirty_ = true;
    bool js_bindings_initialized_ = false;
    int32_t last_mouse_x_ = 0;
    int32_t last_mouse_y_ = 0;
    double last_wall_time_sec_ = 0.0;

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
