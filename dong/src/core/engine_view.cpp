#include "engine_view.hpp"

#include "context.hpp"
#include "log.h"
#include "profiler.h"

#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_file_system.h"
#include "dong_clipboard.h"

#include "../dom/dom_manager.hpp"
#include "../dom/style_engine.hpp"
#include "../dom/event_system.hpp"
#include "../dom/focus_manager.hpp"
#include "../dom/input_element.hpp"
#include "../dom/select_element.hpp"
#include "../dom/details_element.hpp"
#include "../dom/dialog_element.hpp"
#include "../dom/contenteditable.hpp"
#include "../dom/editing_commands.hpp"
#include "../dom/selection.hpp"
#include "../dom/text_hit_testing.hpp"
#include "../dom/drag_manager.hpp"
#include "../layout/layout_engine.hpp"
#include "../input/spatial_nav.hpp"

#include "../render/render_surface.hpp"
#include "../render/painter.hpp"
#include "../render/gpu_ir.hpp"
#include "../render/text_shaper.hpp"
#include "../render/overlay_draw.hpp"
#include "../render/scene_graph.hpp"
#include "../dom/scene_compiler.hpp"
#include "../script/script_engine.hpp"
#include "../script/js_bindings.hpp"
#include "../script/js_fetch_bindings.hpp"
#include "../devtools/devtools_overlay.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <vector>

#include <filesystem>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <chrono>






namespace dong::script {
    extern render::SceneGraph& getGlobalSceneGraph();
}

namespace dong {

static render::SceneGraph& getSceneGraph() {
    return script::getGlobalSceneGraph();
}

namespace {

using dong::dom::DOMNodePtr;

// --- P0-6 S1: Invalidation tracking (behavioral equivalence) ---
// All invalidations still trigger full repaint. This tracks WHAT caused it.
enum class InvalidationKind : uint8_t {
    Style,      // color, opacity, visibility change - no layout needed
    Layout,     // width, height, margin, display - needs layout
    Paint,      // background, border-color, box-shadow, scroll, caret - only repaint
    Geometry,   // hit-test rect change
    Full,       // entire view (e.g. resize, initial load)
};

struct Invalidation {
    InvalidationKind kind;
    // Source node - nullptr means entire view
    // Not stored as DOMNodePtr to avoid reference cycle; just for tracking
    const void* source_node = nullptr;
    const char* reason = nullptr;  // debug string literal (not owned)
};
// --- end P0-6 S1 types ---

/**
 * Check if a script element is an ES module script.
 * @param script_node: The <script> DOM element to check
 * @return true if type="module", false otherwise
 */
static bool isModuleScript(const DOMNodePtr& script_node) {
    if (!script_node) return false;
    std::string type = script_node->getAttribute("type");
    // ES module scripts have type="module"
    return (type == "module");
}

static void mapPointThroughInverseTransform(const dong::dom::ComputedStyle& style,
                                            float lx, float ly, float w, float h,
                                            float& x, float& y) {
    const bool has_transform =
        (style.transform_translate_x != 0.0f || style.transform_translate_y != 0.0f ||
         style.transform_scale_x != 1.0f || style.transform_scale_y != 1.0f ||
         style.transform_rotate != 0.0f || style.transform_skew_x != 0.0f ||
         style.transform_skew_y != 0.0f);
    if (!has_transform) return;

    const float sx = style.transform_scale_x;
    const float sy = style.transform_scale_y;
    const float tx = style.transform_translate_x_is_percent
                         ? w * style.transform_translate_x / 100.0f
                         : style.transform_translate_x;
    const float ty = style.transform_translate_y_is_percent
                         ? h * style.transform_translate_y / 100.0f
                         : style.transform_translate_y;
    const float angle_rad = style.transform_rotate * 3.14159265358979f / 180.0f;
    const float skew_x_rad = style.transform_skew_x * 3.14159265358979f / 180.0f;
    const float skew_y_rad = style.transform_skew_y * 3.14159265358979f / 180.0f;

    const float origin_x = lx + w * style.transform_origin_x / 100.0f;
    const float origin_y = ly + h * style.transform_origin_y / 100.0f;

    const float cos_r = cosf(angle_rad);
    const float sin_r = sinf(angle_rad);
    const float tan_kx = tanf(skew_x_rad);
    const float tan_ky = tanf(skew_y_rad);

    const float a00 = sx * (cos_r - sin_r * tan_ky);
    const float a01 = sy * (cos_r * tan_kx - sin_r);
    const float a10 = sx * (sin_r + cos_r * tan_ky);
    const float a11 = sy * (sin_r * tan_kx + cos_r);

    const float m02 = origin_x + tx - (a00 * origin_x + a01 * origin_y);
    const float m12 = origin_y + ty - (a10 * origin_x + a11 * origin_y);

    const float det = a00 * a11 - a01 * a10;
    if (fabsf(det) < 1e-6f) return;

    const float dx = x - m02;
    const float dy = y - m12;
    const float inv_x = (a11 * dx - a01 * dy) / det;
    const float inv_y = (-a10 * dx + a00 * dy) / det;
    x = inv_x;
    y = inv_y;
}

DOMNodePtr hitTestRecursive(const DOMNodePtr& node, dong::layout::Engine* layout_engine,
                            float x, float y) {
    if (!node || !layout_engine) return nullptr;

    // Skip inert subtrees
    if (node->hasAttribute("inert")) return nullptr;

    // P1-9: pointer-events: none — skip this element (and its subtree) from hit testing
    const auto& style = node->getComputedStyle();
    if (style.pointer_events == dom::CSSPointerEvents::None) return nullptr;

    const auto* layout = layout_engine->getLayout(node);
    if (!layout) return nullptr;

    if (node->getAttribute("data-dong-native-tooltip") == "1") {
        return nullptr;
    }

    float lx = layout->x;
    float ly = layout->y;
    float w = layout->width;
    float h = layout->height;

    // Render path applies CSS transforms in painter/GPU; hit testing must invert
    // the same transform so pointer coordinates match visual positions.
    mapPointThroughInverseTransform(style, lx, ly, w, h, x, y);

    bool in_bounds = (x >= lx && x <= lx + w && y >= ly && y <= ly + h);

    if (!in_bounds) {
        // Even if this node is out of bounds, check children with position:absolute/fixed
        // since they can be positioned outside their parent's layout bounds.
        const auto& children = node->getChildren();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if (!*it) continue;
            const auto& cs = (*it)->getComputedStyle();
            if (cs.position == dom::CSSPosition::Absolute || cs.position == dom::CSSPosition::Fixed) {
                auto child_hit = hitTestRecursive(*it, layout_engine, x, y);
                if (child_hit) {
                    return child_hit;
                }
            }
        }
        return nullptr;
    }

    const auto& children = node->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto child_hit = hitTestRecursive(*it, layout_engine, x, y);
        if (child_hit) {
            return child_hit;
        }
    }

    return node;
}

DOMNodePtr hitTestElementAt(dong::dom::Manager* dom_mgr, dong::layout::Engine* layout_engine,
                            int32_t x, int32_t y) {
    if (!dom_mgr || !layout_engine) return nullptr;
    auto root = dom_mgr->getRoot();
    if (!root) return nullptr;
    return hitTestRecursive(root, layout_engine, static_cast<float>(x), static_cast<float>(y));
}

DOMNodePtr findScrollContainerAt(dong::dom::Manager* dom_mgr, dong::layout::Engine* layout_engine,
                                 int32_t x, int32_t y) {
    if (!dom_mgr || !layout_engine) return nullptr;

    auto element = hitTestElementAt(dom_mgr, layout_engine, x, y);
    if (!element) return nullptr;

    auto current = element;
    while (current) {
        if (current->isScrollContainer()) {
            return current;
        }
        current = current->getParent();
    }

    return nullptr;
}

std::string toLowerTrim(std::string s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

float effectiveBorderWidth(const dong::dom::ComputedStyle& style, float side_width, dong::dom::CSSBorderStyle side_style) {
    float w = side_width >= 0.0f ? side_width : style.border_width;
    if (w < 0.0f) w = 0.0f;
    auto st = (side_style == dong::dom::CSSBorderStyleUnset) ? style.border_style : side_style;
    if (st == dong::dom::CSSBorderStyle::None || st == dong::dom::CSSBorderStyle::Hidden) {
        return 0.0f;
    }
    return w;
}

bool computeTextareaResizeHandleRect(dong::layout::Engine* layout_engine,
                                     const DOMNodePtr& textarea,
                                     float& out_x, float& out_y,
                                     float& out_w, float& out_h) {
    if (!layout_engine || !textarea || textarea->getTagName() != "textarea") return false;
    const auto* layout = layout_engine->getLayout(textarea);
    if (!layout) return false;

    const auto& style = textarea->getComputedStyle();
    const auto mode = style.resize;
    if (mode == dom::CSSResize::None) return false;

    out_x = layout->x;
    out_y = layout->y;
    out_w = layout->width;
    out_h = layout->height;
    if (out_w <= 0.0f || out_h <= 0.0f) return false;

    const float bw_l = effectiveBorderWidth(style, style.border_left_width, style.border_left_style);
    const float bw_r = effectiveBorderWidth(style, style.border_right_width, style.border_right_style);
    const float bw_t = effectiveBorderWidth(style, style.border_top_width, style.border_top_style);
    const float bw_b = effectiveBorderWidth(style, style.border_bottom_width, style.border_bottom_style);

    const float inner_x = out_x + bw_l;
    const float inner_y = out_y + bw_t;
    const float inner_w = std::max(0.0f, out_w - bw_l - bw_r);
    const float inner_h = std::max(0.0f, out_h - bw_t - bw_b);
    if (inner_w <= 0.0f || inner_h <= 0.0f) return false;

    const float size = std::min(16.0f, std::min(inner_w, inner_h));
    if (size < 8.0f) return false;

    out_x = inner_x + inner_w - size;
    out_y = inner_y + inner_h - size;
    out_w = size;
    out_h = size;
    return true;
}

bool isPointInside(float px, float py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

std::string toPx(float v) {
    int iv = static_cast<int>(std::round(v));
    if (iv < 1) iv = 1;
    return std::to_string(iv) + "px";
}

bool isAbsolutePath(const std::string& p) {
    if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':') {
        return true;
    }
    if (!p.empty() && (p[0] == '/' || p[0] == '\\')) {
        return true;
    }
    return false;
}

bool readTextFileFromPlatformFS(const std::string& path,
                                const std::string& resource_root,
                                std::string& out) {
    out.clear();
    if (path.empty()) {
        return false;
    }

    std::string resolved = path;
    if (!resource_root.empty() && !isAbsolutePath(path)) {
        try {
            namespace fs = std::filesystem;
            resolved = (fs::path(resource_root) / fs::path(path)).lexically_normal().string();
        } catch (...) {
            resolved = path;
        }
    }

    DongPlatform* platform = dong_platform_get();
    DongFileSystem* fs = platform ? dong_platform_get_file_system(platform) : nullptr;
    if (!fs) {
        return false;
    }

    DongFileData data{};
    DongFileSystemResult r = dong_fs_read_all(fs, resolved.c_str(), &data);
    if (r != DONG_FS_OK || !data.data || data.size == 0) {
        dong_fs_free_data(fs, &data);
        return false;
    }

    out.assign(static_cast<const char*>(data.data), data.size);
    dong_fs_free_data(fs, &data);
    return true;
}


static bool isAbsolutePathLike(const std::string& p) {
    if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':') {
        return true;
    }
    if (!p.empty() && (p[0] == '/' || p[0] == '\\')) {
        return true;
    }
    return false;
}

static std::string resolveVideoUrlForPlugin(const std::string& src, const std::string& resource_root) {
    if (src.empty()) return {};
    if (src.rfind("http://", 0) == 0 || src.rfind("https://", 0) == 0 || src.rfind("data:", 0) == 0) {
        return {};
    }
    std::string path = src;
    if (path.rfind("file://", 0) == 0) {
        path = path.substr(std::string("file://").size());
        if (path.size() >= 3 && path[0] == '/' && std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == ':') {
            path.erase(path.begin());
        }
    }
    if (isAbsolutePathLike(path)) {
        return path;
    }
    if (!resource_root.empty()) {
        try {
            namespace fs = std::filesystem;
            fs::path resolved = fs::path(resource_root) / fs::path(path);
            return resolved.lexically_normal().string();
        } catch (...) {
        }
    }
    return path;
}

static bool debug_video_enabled() {
    static int s_cached = -1;
    if (s_cached != -1) return s_cached != 0;
    const char* v = std::getenv("DONG_DEBUG_VIDEO");
    s_cached = (v && v[0] && v[0] != '0') ? 1 : 0;
    return s_cached != 0;
}

// P0-6 S6: Force full repaint on every invalidation (disables paint-only optimization).
// Set DONG_FORCE_FULL_REPAINT=1 to force all invalidations to Full kind.
static bool force_full_repaint_enabled() {
    static int s_cached = -1;
    if (s_cached != -1) return s_cached != 0;
    const char* v = std::getenv("DONG_FORCE_FULL_REPAINT");
    s_cached = (v && v[0] && v[0] != '0') ? 1 : 0;
    if (s_cached) {
        DONG_LOG_INFO("[P0-6] DONG_FORCE_FULL_REPAINT=1: all invalidations treated as Full");
    }
    return s_cached != 0;
}

static void markIsolatedLayerDirtyFlags(dong::render::GPUCommandList& list, uint64_t layer_id) {
    if (layer_id == 0) return;
    for (auto& cmd : list.commands) {
        if ((cmd.type == dong::render::GPUCommandType::BeginIsolatedLayer ||
             cmd.type == dong::render::GPUCommandType::EndIsolatedLayer) &&
            cmd.layer_id == layer_id) {
            cmd.layer_dirty = true;
        }
    }
}

static void updateSmoothScrollRecursive(const DOMNodePtr& node,
                                       double now_sec,
                                       bool& any_active,
                                       bool& any_changed) {
    if (!node) return;

    if (node->updateSmoothScroll(now_sec)) {
        any_changed = true;
    }

    if (node->hasActiveSmoothScroll()) {
        any_active = true;
    }

    for (const auto& ch : node->getChildren()) {
        if (!ch) continue;
        updateSmoothScrollRecursive(ch, now_sec, any_active, any_changed);
    }
}

} // namespace

struct EngineView::Impl {

    dong::Context ctx;

    uint32_t width = 960;
    uint32_t height = 540;
    std::string html;
    std::string resource_root;
    std::string cached_cursor;
    std::string view_name;


    std::unique_ptr<dong::dom::Manager> dom_manager;
    std::unique_ptr<dong::layout::Engine> layout_engine;
    std::unique_ptr<dong::render::RenderSurface> render_surface;
    std::unique_ptr<dong::render::Painter> painter;

    std::unique_ptr<dong::dom::EventDispatcher> event_dispatcher;
    std::unique_ptr<dong::dom::FocusManager> focus_manager;
    std::unique_ptr<dong::drag::DragManager> drag_manager;
    std::unique_ptr<dong::script::ScriptEngine> owned_script_engine;  // null when sharing
    dong::script::ScriptEngine* script_engine = nullptr;              // always valid
    std::unique_ptr<dong::script::JSBindings> js_bindings;
    dong::render::OverlayDraw overlay_draw;

    // P1-3: DevTools overlay
    dong::devtools::DevToolsOverlay devtools_overlay;

    const dong_plugin_vtable_t* plugin = nullptr;
    void* plugin_user = nullptr;

    struct VideoState {
        dong::dom::DOMNodePtr node;
        dong_video_player_t* player = nullptr;

        std::string src;
        std::string frame_key;

        dong_video_metadata_t meta{};
        bool meta_ready = false;
        bool loadeddata_sent = false;

        bool preload = true;
        bool autoplay = false;
        bool loop = false;
        bool controls = false;

        bool playing = false;
        bool ended = false;
        bool errored = false;

        double wall_clock_start = 0.0;
        double video_time_start = 0.0;
        double current_pts = 0.0;

        dong_video_pixel_format_t frame_format = DONG_VIDEO_PIXEL_FORMAT_RGBA8;
        const uint8_t* plane_data[3] = {nullptr, nullptr, nullptr};
        uint32_t plane_stride[3] = {0, 0, 0};
        std::vector<uint8_t> owned_frame;

        uint32_t frame_w = 0;
        uint32_t frame_h = 0;
        uint32_t frame_stride = 0;
        bool has_frame = false;
        bool needs_upload = false;

        double last_timeupdate_pts = 0.0;
        double last_resync_wall = 0.0;
        double last_debug_log_pts = 0.0;
        double last_debug_pull_wall = 0.0;
        double last_debug_state_wall = 0.0;
        int debug_frames_logged = 0;
    };

    std::unordered_map<void*, VideoState> video_states;

    std::unique_ptr<dong::render::GPUCommandList> cached_cmd_list;


    void* external_gpu_device = nullptr;
    void* external_window = nullptr;

    bool use_gpu = false;
    bool commands_dirty_ = true;  // P0-6 S1: renamed (underscore), still controls actual behavior
    std::vector<Invalidation> pending_invalidations_;
    // P0-6: Set of node pointers that have been explicitly invalidated this tick.
    // Empty set means "full invalidation" (all nodes dirty). Non-empty means only
    // those specific nodes need re-painting. Foundation for future partial repaint.
    std::unordered_set<const void*> dirty_node_ptrs_;
    uint32_t invalidation_count_this_tick_ = 0;
    uint32_t full_repaint_count_ = 0;  // for P0-7 perf metric
    uint32_t paint_only_tick_count_ = 0;  // ticks where only Paint invalidations were pending
    uint32_t layout_skip_count_ = 0;     // ticks where layout was skipped due to Paint-only
    bool js_bindings_initialized = false;

    // Top-layer: modal dialogs rendered above everything
    std::vector<dong::dom::DOMNodePtr> top_layer;

    // Selection for contenteditable
    std::unique_ptr<dong::dom::Selection> selection;

    // ContentEditable drag selection state
    bool ce_drag_active_ = false;
    DOMNodePtr ce_drag_editable_root_;

    // Input/textarea drag selection state
    bool input_drag_active_ = false;
    DOMNodePtr input_drag_node_;
    size_t input_drag_anchor_ = 0;  // char index at mousedown

    // Caret blink state
    double caret_blink_time_ = 0.0;
    bool caret_visible_ = true;
    static constexpr double CARET_BLINK_INTERVAL = 0.5;

    // Double-click detection
    int click_count_ = 0;
    double last_click_time_ = 0.0;
    int32_t last_click_x_ = 0;
    int32_t last_click_y_ = 0;
    static constexpr double DOUBLE_CLICK_TIME = 0.4;
    static constexpr int32_t DOUBLE_CLICK_DIST = 5;

    // Text shaper for hit testing (independent of painter's shaper)
    dong::render::TextShaper text_shaper_hit_;

    bool video_dom_scanned = false;
    bool video_dom_has_any = false;
    double last_wall_time_sec = 0.0;

    // Text renderer mode (Auto / MSDF / Slug)
    dong::render::TextRendererMode text_renderer_mode = dong::render::TextRendererMode::Auto;


    int32_t last_mouse_x = 0;
    int32_t last_mouse_y = 0;

    // Modifier keys
    bool mod_ctrl = false;
    bool mod_shift = false;
    bool mod_alt = false;
    bool mod_meta = false;

    // Track pressed keys for KeyboardEvent.repeat detection
    std::unordered_set<uint32_t> pressed_keys;

    DOMNodePtr hovered_element;

    DOMNodePtr active_element;

    // 当前拖放目标元素
    DOMNodePtr drop_target;

    // 当前打开的 <select>（用于把下拉框区域的点击正确路由到该 select）
    DOMNodePtr open_select_element;

    struct TextareaResizeState {
        bool active = false;
        DOMNodePtr node;
        int32_t start_mouse_x = 0;
        int32_t start_mouse_y = 0;
        float start_width = 0.0f;
        float start_height = 0.0f;
        dom::CSSResize mode = dom::CSSResize::None;
    };
    TextareaResizeState textarea_resize;

    DOMNodePtr native_tooltip_node;
    DOMNodePtr native_tooltip_target;

    // 当前URL fragment（用于:target伪类匹配）
    std::string current_fragment;

    Impl(uint32_t w, uint32_t h)

        : width(w), height(h),
          dom_manager(std::make_unique<dong::dom::Manager>()),
          layout_engine(std::make_unique<dong::layout::Engine>()),
          render_surface(std::make_unique<dong::render::CPUBufferSurface>(w, h)),
          painter(std::make_unique<dong::render::Painter>(render_surface.get())),
          event_dispatcher(std::make_unique<dong::dom::EventDispatcher>()),
          focus_manager(std::make_unique<dong::dom::FocusManager>()),
          drag_manager(std::make_unique<dong::drag::DragManager>()),
          owned_script_engine(std::make_unique<dong::script::ScriptEngine>()),
          script_engine(owned_script_engine.get()),
          js_bindings(std::make_unique<dong::script::JSBindings>(
              script_engine,
              dom_manager.get(),
              layout_engine.get(),
              event_dispatcher.get(),
              focus_manager.get())) {
        selection = std::make_unique<dong::dom::Selection>();
        js_bindings->selection_ = selection.get();
        js_bindings->overlay_draw_ = &overlay_draw;
        focus_manager->setEventDispatcher(event_dispatcher.get());
        activateViewContext();
        ctx.initialize();
    }

    // Shared-JS constructor: uses an external ScriptEngine (not owned by this Impl).
    Impl(uint32_t w, uint32_t h, dong::script::ScriptEngine* shared_se)

        : width(w), height(h),
          dom_manager(std::make_unique<dong::dom::Manager>()),
          layout_engine(std::make_unique<dong::layout::Engine>()),
          render_surface(std::make_unique<dong::render::CPUBufferSurface>(w, h)),
          painter(std::make_unique<dong::render::Painter>(render_surface.get())),
          event_dispatcher(std::make_unique<dong::dom::EventDispatcher>()),
          focus_manager(std::make_unique<dong::dom::FocusManager>()),
          drag_manager(std::make_unique<dong::drag::DragManager>()),
          owned_script_engine(nullptr),
          script_engine(shared_se),
          js_bindings(std::make_unique<dong::script::JSBindings>(
              script_engine,
              dom_manager.get(),
              layout_engine.get(),
              event_dispatcher.get(),
              focus_manager.get())) {
        selection = std::make_unique<dong::dom::Selection>();
        js_bindings->selection_ = selection.get();
        js_bindings->overlay_draw_ = &overlay_draw;
        focus_manager->setEventDispatcher(event_dispatcher.get());
        activateViewContext();
        ctx.initialize();
    }

    ~Impl() {
        ctx.shutdown();
    }

    // Set the global DOMNode statics to this view's managers.
    // Must be called before any operation that triggers DOM events/focus.
    // Safe because everything is single-threaded.
    void activateViewContext() {
        dong::dom::DOMNode::setFocusManager(focus_manager.get());
        dong::dom::DOMNode::setEventDispatcher(event_dispatcher.get());
        if (js_bindings) {
            dong::script::JSBindings::setActiveBindings(js_bindings.get());
        }
    }

    void ensureJSBindingsInitialized() {
        if (js_bindings_initialized || !js_bindings || !script_engine) {
            return;
        }
        JSContext* qjs = script_engine->getContext();
        if (!qjs) {
            return;
        }
        js_bindings->initialize();
        js_bindings_initialized = true;
    }

    // --- P0-6 S1: Invalidation tracking ---
    void invalidate(InvalidationKind kind, const void* source = nullptr, const char* reason = nullptr) {
        pending_invalidations_.push_back({kind, source, reason});
        commands_dirty_ = true;
        dom_display_list_valid_ = false;
        // P0-6: Track which specific nodes are dirty for future partial repaint.
        // A null source or Full/Layout kind means all nodes are potentially dirty,
        // so we clear the set (empty = full invalidation).
        if (source && kind != InvalidationKind::Full && kind != InvalidationKind::Layout) {
            // Only track targeted invalidations (Style/Paint/Geometry with a source)
            if (!dirty_node_ptrs_.empty() || pending_invalidations_.size() == 1) {
                dirty_node_ptrs_.insert(source);
            }
            // else: already in "all dirty" mode (empty set after a Full/Layout/null)
        } else {
            // Null source or Full/Layout → all nodes dirty: clear set to signal full invalidation
            dirty_node_ptrs_.clear();
        }
        if (render_surface) {
            render_surface->markDirty();
        }
    }

    // Backward-compat shortcut: Full invalidation (used by 60+ callsites)
    void markNeedsRepaint() {
        invalidate(InvalidationKind::Full, nullptr, "markNeedsRepaint");
    }

    // P0-6: Check if all pending invalidations are paint-only (no layout needed)
    // Returns false if DONG_FORCE_FULL_REPAINT is set (env var fallback).
    bool isPaintOnlyInvalidation() const {
        if (force_full_repaint_enabled()) return false;
        if (pending_invalidations_.empty()) return false;
        for (const auto& inv : pending_invalidations_) {
            if (inv.kind == InvalidationKind::Layout || inv.kind == InvalidationKind::Full) {
                return false;
            }
        }
        return true;  // Only Style/Paint/Geometry — no layout recalc needed
    }

    // P0-6: Query whether a specific node needs re-painting.
    // Returns true if:
    //   - dirty_node_ptrs_ is empty (full invalidation — all nodes dirty), OR
    //   - the given node_ptr is in the dirty set.
    // This is the foundation for skipping buildDisplayListNode on clean nodes
    // once per-node display list caching is implemented.
    bool isNodeDirty(const void* node_ptr) const {
        return dirty_node_ptrs_.empty() || dirty_node_ptrs_.count(node_ptr) > 0;
    }
    // --- end P0-6 ---

    void resetCaretBlink() {
        caret_blink_time_ = 0.0;
        caret_visible_ = true;
    }

    // Measure the pixel width of a string using the hit-test shaper.
    float measureTextWidthHit(const std::string& text, const dom::ComputedStyle& style, float font_size) {
        if (text.empty()) return 0.0f;
        dong::render::TextShapeRequest req;
        req.text = text;
        req.font_family = style.font_family;
        req.font_size = font_size;
        req.font_weight = dom::toString(style.font_weight);
        req.font_style = dom::toString(style.font_style);
        dong::render::ShapedText shaped;
        if (text_shaper_hit_.shape(req, shaped) && shaped.units_per_em > 0) {
            return shaped.width_units * (font_size / static_cast<float>(shaped.units_per_em));
        }
        return text.size() * font_size * 0.6f;
    }

    // Get the line height used for rendering text in an input/textarea.
    float getInputLineHeight(const dom::ComputedStyle& style, float font_size) {
        dong::render::TextShapeRequest mreq{"X", style.font_family, dom::toString(style.font_weight), dom::toString(style.font_style), font_size};
        dong::render::ShapedText mshaped;
        if (text_shaper_hit_.shape(mreq, mshaped) && mshaped.scale_to_pixels > 0.0f) {
            float scale = mshaped.scale_to_pixels;
            float lh_units = mshaped.line_height_units;
            if (style.line_height > 0.0f) {
                if (style.line_height_is_unitless)
                    lh_units = (style.line_height * font_size) / std::max(scale, 1e-3f);
                else
                    lh_units = style.line_height / std::max(scale, 1e-3f);
            }
            if (lh_units <= 0.0f) lh_units = font_size / std::max(scale, 1e-3f);
            return lh_units * scale;
        }
        return font_size * 1.2f;
    }

    // Binary search for the character offset in a single line string closest to click_x.
    // Returns char index within the line (0-based).
    size_t binarySearchCharInLine(const std::string& line, float click_x,
                                   const dom::ComputedStyle& style, float font_size) {
        if (line.empty() || click_x <= 0.0f) return 0;

        // Count UTF-8 chars in line
        size_t char_count = 0;
        for (size_t i = 0; i < line.size(); ) {
            unsigned char c = line[i];
            if ((c & 0x80) == 0) i += 1;
            else if ((c & 0xE0) == 0xC0) i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else i += 4;
            char_count++;
        }

        auto byteOffInLine = [&](size_t ci) -> size_t {
            size_t bo = 0, cc = 0;
            while (bo < line.size() && cc < ci) {
                unsigned char c = line[bo];
                if ((c & 0x80) == 0) bo += 1;
                else if ((c & 0xE0) == 0xC0) bo += 2;
                else if ((c & 0xF0) == 0xE0) bo += 3;
                else bo += 4;
                cc++;
            }
            return bo;
        };

        size_t lo = 0, hi = char_count;
        while (lo < hi) {
            size_t mid = (lo + hi) / 2;
            std::string prefix = line.substr(0, byteOffInLine(mid));
            float mid_width = measureTextWidthHit(prefix, style, font_size);
            if (mid_width < click_x)
                lo = mid + 1;
            else
                hi = mid;
        }

        // Snap to nearest boundary
        if (lo > 0) {
            float w_lo = measureTextWidthHit(line.substr(0, byteOffInLine(lo)), style, font_size);
            float w_prev = measureTextWidthHit(line.substr(0, byteOffInLine(lo - 1)), style, font_size);
            if (click_x < (w_prev + w_lo) * 0.5f)
                return lo - 1;
        }
        return lo;
    }

    // Hit-test mouse coordinates to find the character index in an input/textarea value.
    // For single-line input: only X matters.
    // For textarea: Y selects the line, X selects the column.
    size_t hitTestInputCharOffset(const DOMNodePtr& node, int32_t mouse_x, int32_t mouse_y) {
        if (!node || !layout_engine) return 0;

        auto* state = dong::dom::getInputState(node);
        if (!state) return 0;

        const std::string& value = state->getValue();
        if (value.empty()) return 0;

        auto* layout_node = layout_engine->getLayout(node);
        if (!layout_node) return 0;

        const auto& style = node->getComputedStyle();
        float bl = style.border_left_width >= 0.0f ? style.border_left_width : 0.0f;
        float bt = style.border_top_width >= 0.0f ? style.border_top_width : 0.0f;
        float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
        float pad_t = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
        float text_origin_x = layout_node->layout.position[0] + bl + pad_l;
        float text_origin_y = layout_node->layout.position[1] + bt + pad_t;
        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

        bool is_textarea = (node->getTagName() == "textarea");

        if (is_textarea && value.find('\n') != std::string::npos) {
            // Multiline textarea: determine line from Y, then char from X
            float line_height = getInputLineHeight(style, font_size);
            float click_y = static_cast<float>(mouse_y) - text_origin_y;
            float click_x = static_cast<float>(mouse_x) - text_origin_x;

            // Split value by newlines
            std::vector<std::string> lines;
            size_t start = 0;
            while (start <= value.size()) {
                size_t pos = value.find('\n', start);
                if (pos == std::string::npos) {
                    lines.push_back(value.substr(start));
                    break;
                }
                lines.push_back(value.substr(start, pos - start));
                start = pos + 1;
            }

            // Determine which line was clicked
            int line_idx = static_cast<int>(click_y / line_height);
            if (line_idx < 0) line_idx = 0;
            if (line_idx >= static_cast<int>(lines.size())) line_idx = static_cast<int>(lines.size()) - 1;

            // Count chars before this line (including \n separators)
            size_t chars_before = 0;
            for (int i = 0; i < line_idx; i++) {
                // Count UTF-8 chars in lines[i]
                for (size_t j = 0; j < lines[i].size(); ) {
                    unsigned char c = lines[i][j];
                    if ((c & 0x80) == 0) j += 1;
                    else if ((c & 0xE0) == 0xC0) j += 2;
                    else if ((c & 0xF0) == 0xE0) j += 3;
                    else j += 4;
                    chars_before++;
                }
                chars_before++; // for the '\n'
            }

            // Binary search within the line
            size_t col = binarySearchCharInLine(lines[line_idx], click_x, style, font_size);
            return chars_before + col;
        }

        // Single-line path (input or textarea without newlines)
        float click_x = static_cast<float>(mouse_x) - text_origin_x;

        // For single-line input, center text vertically — use content_h for vertical centering
        // but only X matters for char offset
        size_t char_count = state->charCount();
        return binarySearchCharInLine(value, click_x, style, font_size);
    }

    void setPlugin(const dong_plugin_vtable_t* new_plugin, void* new_plugin_user) {
        plugin = new_plugin;
        plugin_user = new_plugin_user;
        if (!videoCapable()) {
            clearVideoStates();
        }
    }

    bool videoCapable() const {
        return plugin &&
            (plugin->info.capabilities & DONG_PLUGIN_CAP_VIDEO) &&
            plugin->video_open && plugin->video_close && plugin->video_read_frame;
    }

    void clearVideoStates() {
        for (auto& kv : video_states) {
            closeVideo(kv.second);
        }
        video_states.clear();
        video_dom_scanned = false;
        video_dom_has_any = false;
    }

    void closeVideo(VideoState& vs) {
        if (vs.player && plugin && plugin->video_close) {
            plugin->video_close(plugin_user, vs.player);
        }
        vs.player = nullptr;
        vs.meta_ready = false;
        vs.loadeddata_sent = false;
        vs.playing = false;
        vs.ended = false;
        vs.errored = false;
        vs.current_pts = 0.0;
        vs.last_timeupdate_pts = 0.0;
        vs.last_resync_wall = 0.0;
        vs.last_debug_log_pts = 0.0;
        vs.last_debug_pull_wall = 0.0;
        vs.last_debug_state_wall = 0.0;
        vs.debug_frames_logged = 0;
        vs.has_frame = false;
        vs.needs_upload = false;
        vs.frame_format = DONG_VIDEO_PIXEL_FORMAT_RGBA8;
        vs.plane_data[0] = nullptr;
        vs.plane_data[1] = nullptr;
        vs.plane_data[2] = nullptr;
        vs.plane_stride[0] = 0;
        vs.plane_stride[1] = 0;
        vs.plane_stride[2] = 0;
        vs.owned_frame.clear();
        vs.frame_w = 0;
        vs.frame_h = 0;
        vs.frame_stride = 0;
        if (vs.node && !vs.node->getAttribute("__dong_video_frame").empty()) {
            vs.node->setAttribute("__dong_video_frame", "");
            invalidate(InvalidationKind::Paint, nullptr, "video_frame");
        }
    }

    void dispatchMediaEvent(VideoState& vs, const char* type_name) {
        if (!type_name || !type_name[0] || !js_bindings || !script_engine || !vs.node) return;
        ensureJSBindingsInitialized();
        uint64_t node_id = js_bindings->getNodeIdFor(vs.node);
        if (!node_id) return;
        if (!js_bindings->hasEventListeners(node_id, type_name)) return;
        const double duration = vs.meta_ready ? vs.meta.duration_seconds : 0.0;
        js_bindings->dispatchMediaEvent(node_id, type_name, vs.current_pts, duration, nullptr);
    }

    void dispatchMediaError(VideoState& vs, const char* message) {
        if (!js_bindings || !script_engine || !vs.node) return;
        ensureJSBindingsInitialized();
        uint64_t node_id = js_bindings->getNodeIdFor(vs.node);
        if (!node_id) return;
        if (!js_bindings->hasEventListeners(node_id, "error")) return;
        const double duration = vs.meta_ready ? vs.meta.duration_seconds : 0.0;
        js_bindings->dispatchMediaEvent(node_id, "error", vs.current_pts, duration, message ? message : "");
    }

    void toggleVideoPlayPause(VideoState& vs, double wall_time_sec) {
        if (!vs.player) return;
        if (!vs.playing) {
            if (vs.ended && plugin && plugin->video_seek) {
                plugin->video_seek(plugin_user, vs.player, 0.0);
                vs.current_pts = 0.0;
            }
            vs.playing = true;
            vs.ended = false;
            vs.wall_clock_start = wall_time_sec;
            vs.video_time_start = vs.current_pts;
            if (vs.node) {
                vs.node->setAttribute("__dong_video_playing", "1");
                vs.node->setAttribute("__dong_video_ended", "0");
                vs.node->setAttribute("__dong_video_seeking", "0");
                vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
            }
            dispatchMediaEvent(vs, "play");
            dispatchMediaEvent(vs, "playing");
        } else {
            vs.playing = false;
            if (vs.node) {
                vs.node->setAttribute("__dong_video_playing", "0");
                vs.node->setAttribute("__dong_video_seeking", "0");
                vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
            }
            dispatchMediaEvent(vs, "pause");
        }
    }

    void updateVideoFrameFromDecoded(VideoState& vs, const dong_video_frame_t& f, bool copy_frame) {
        vs.frame_format = f.format;
        vs.frame_w = f.width;
        vs.frame_h = f.height;
        vs.frame_stride = f.stride_bytes;
        vs.plane_data[0] = f.plane_data[0];
        vs.plane_data[1] = f.plane_data[1];
        vs.plane_data[2] = f.plane_data[2];
        vs.plane_stride[0] = f.plane_stride_bytes[0];
        vs.plane_stride[1] = f.plane_stride_bytes[1];
        vs.plane_stride[2] = f.plane_stride_bytes[2];

        if (!copy_frame) {
            vs.owned_frame.clear();
            return;
        }

        vs.owned_frame.clear();
        if (f.format == DONG_VIDEO_PIXEL_FORMAT_RGBA8) {
            const size_t sz = static_cast<size_t>(f.stride_bytes) * static_cast<size_t>(f.height);
            if (sz > 0 && f.data) {
                vs.owned_frame.resize(sz);
                std::memcpy(vs.owned_frame.data(), f.data, sz);
                vs.plane_data[0] = vs.owned_frame.data();
                vs.plane_stride[0] = f.stride_bytes;
                vs.plane_data[1] = nullptr;
                vs.plane_data[2] = nullptr;
                vs.plane_stride[1] = 0;
                vs.plane_stride[2] = 0;
            }
        } else if (f.format == DONG_VIDEO_PIXEL_FORMAT_YUV420P) {
            const uint32_t w = f.width;
            const uint32_t h = f.height;
            const uint32_t cw = (w + 1u) / 2u;
            const uint32_t ch = (h + 1u) / 2u;
            const size_t y_size = static_cast<size_t>(w) * static_cast<size_t>(h);
            const size_t u_size = static_cast<size_t>(cw) * static_cast<size_t>(ch);
            const size_t v_size = u_size;
            vs.owned_frame.resize(y_size + u_size + v_size);

            uint8_t* dst_y = vs.owned_frame.data();
            uint8_t* dst_u = dst_y + y_size;
            uint8_t* dst_v = dst_u + u_size;

            for (uint32_t y = 0; y < h; ++y) {
                std::memcpy(dst_y + static_cast<size_t>(y) * w,
                            f.plane_data[0] + static_cast<size_t>(y) * f.plane_stride_bytes[0],
                            w);
            }
            for (uint32_t y = 0; y < ch; ++y) {
                std::memcpy(dst_u + static_cast<size_t>(y) * cw,
                            f.plane_data[1] + static_cast<size_t>(y) * f.plane_stride_bytes[1],
                            cw);
                std::memcpy(dst_v + static_cast<size_t>(y) * cw,
                            f.plane_data[2] + static_cast<size_t>(y) * f.plane_stride_bytes[2],
                            cw);
            }

            vs.plane_data[0] = dst_y;
            vs.plane_data[1] = dst_u;
            vs.plane_data[2] = dst_v;
            vs.plane_stride[0] = w;
            vs.plane_stride[1] = cw;
            vs.plane_stride[2] = cw;
            vs.frame_stride = w;
        }
    }

    void updateVideoPlayback(VideoState& vs, double wall_time_sec) {
        if (!vs.player || !plugin || !plugin->video_read_frame) {
            return;
        }

        const double target_time = vs.video_time_start + (wall_time_sec - vs.wall_clock_start);

        if (debug_video_enabled() && (wall_time_sec - vs.last_debug_state_wall) >= 1.0) {
            vs.last_debug_state_wall = wall_time_sec;
            DONG_LOG_INFO("[Video] state playing=%d ended=%d ct=%.3f target=%.3f",
                         vs.playing ? 1 : 0,
                         vs.ended ? 1 : 0,
                         vs.current_pts,
                         target_time);
        }

        static const double kResyncLagSec = ([]() {
            const char* v = std::getenv("DONG_VIDEO_RESYNC_LAG_SEC");
            if (!v || !*v) return 0.50;
            return std::atof(v);
        })();
        static const double kResyncCooldownSec = ([]() {
            const char* v = std::getenv("DONG_VIDEO_RESYNC_COOLDOWN_SEC");
            if (!v || !*v) return 0.75;
            return std::atof(v);
        })();

        const double lag = target_time - vs.current_pts;
        if (plugin->video_seek && lag > kResyncLagSec && (wall_time_sec - vs.last_resync_wall) >= kResyncCooldownSec) {
            plugin->video_seek(plugin_user, vs.player, target_time);
            vs.wall_clock_start = wall_time_sec;
            vs.video_time_start = target_time;
            vs.last_resync_wall = wall_time_sec;
        }

        static const int kMaxFramePullPerTick = ([]() {
            const char* v = std::getenv("DONG_VIDEO_MAX_PULL_PER_TICK");
            if (!v || !*v) return 8;
            return std::atoi(v);
        })();

        static const bool kCopyVideoFrames = ([]() {
            const char* v = std::getenv("DONG_VIDEO_FRAME_COPY");
            return v && (std::strcmp(v, "1") == 0 || std::strcmp(v, "true") == 0);
        })();

        int pulled = 0;
        while (pulled < kMaxFramePullPerTick) {
            dong_video_frame_t frame{};
            int r = plugin->video_read_frame(plugin_user, vs.player, &frame);
            if (r == 0) {
                if (vs.loop && plugin->video_seek) {
                    plugin->video_seek(plugin_user, vs.player, 0.0);
                    vs.wall_clock_start = wall_time_sec;
                    vs.video_time_start = 0.0;
                    vs.current_pts = 0.0;
                    vs.ended = false;
                } else {
                    vs.playing = false;
                    vs.ended = true;
                    if (vs.node) {
                        vs.node->setAttribute("__dong_video_playing", "0");
                        vs.node->setAttribute("__dong_video_ended", "1");
                        vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
                    }
                    dispatchMediaEvent(vs, "ended");
                }
                return;
            }
            if (r < 0) {
                // No new frame available yet. Keep current state and try again next tick.
                if (debug_video_enabled() && (wall_time_sec - vs.last_debug_pull_wall) >= 0.5) {
                    vs.last_debug_pull_wall = wall_time_sec;
                    DONG_LOG_INFO("[Video] no_new_frame ct=%.3f target=%.3f", vs.current_pts, target_time);
                }
                break;
            }

            if (frame.plane_data[0] && frame.width > 0 && frame.height > 0) {
                updateVideoFrameFromDecoded(vs, frame, kCopyVideoFrames);
                vs.current_pts = frame.pts_seconds;
                vs.has_frame = true;
                vs.needs_upload = true;
                if (vs.node) {
                    vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
                }
                if (debug_video_enabled() && vs.debug_frames_logged < 5) {
                    ++vs.debug_frames_logged;
                    DONG_LOG_INFO("[Video] decoded frame #%d pts=%.3f %ux%u fmt=%d",
                                 vs.debug_frames_logged,
                                 frame.pts_seconds,
                                 frame.width,
                                 frame.height,
                                 (int)frame.format);
                }
            }

            if (frame.pts_seconds + 1e-6 >= target_time) {
                break;
            }
            ++pulled;
        }

        if (vs.current_pts - vs.last_timeupdate_pts >= 0.25) {
            vs.last_timeupdate_pts = vs.current_pts;
            dispatchMediaEvent(vs, "timeupdate");
            if (debug_video_enabled() && (vs.current_pts - vs.last_debug_log_pts >= 0.25)) {
                vs.last_debug_log_pts = vs.current_pts;
                DONG_LOG_INFO("[Video] timeupdate ct=%.3f has_frame=%d needs_upload=%d", vs.current_pts, vs.has_frame ? 1 : 0, vs.needs_upload ? 1 : 0);
            }
        }
    }

    bool uploadPendingVideoFrames() {
        if (!use_gpu) {
            return false;
        }
        DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
        if (!driver) {
            return false;
        }

        bool did_upload = false;
        for (auto& kv : video_states) {
            VideoState& vs = kv.second;
            if (!vs.needs_upload || !vs.has_frame || vs.frame_key.empty()) {
                continue;
            }
            if (vs.frame_w == 0 || vs.frame_h == 0) {
                vs.needs_upload = false;
                continue;
            }

            bool ok = false;
            if (vs.frame_format == DONG_VIDEO_PIXEL_FORMAT_RGBA8) {
                const uint8_t* src = vs.plane_data[0];
                if (!src && !vs.owned_frame.empty()) {
                    src = vs.owned_frame.data();
                }
                if (!src) {
                    vs.needs_upload = false;
                    continue;
                }
                ok = dong_gpu_update_external_image_rgba(driver, vs.frame_key.c_str(), src, vs.frame_w, vs.frame_h, vs.frame_stride) != 0;
                did_upload = did_upload || ok;
            } else if (vs.frame_format == DONG_VIDEO_PIXEL_FORMAT_YUV420P) {
                const uint8_t* y = vs.plane_data[0];
                const uint8_t* u = vs.plane_data[1];
                const uint8_t* v = vs.plane_data[2];
                if (!y || !u || !v) {
                    vs.needs_upload = false;
                    continue;
                }
                ok = dong_gpu_update_external_image_yuv420p(
                    driver,
                    vs.frame_key.c_str(),
                    y,
                    vs.plane_stride[0],
                    u,
                    vs.plane_stride[1],
                    v,
                    vs.plane_stride[2],
                    vs.frame_w,
                    vs.frame_h
                ) != 0;
                did_upload = did_upload || ok;
            } else {
                vs.needs_upload = false;
                continue;
            }

            if (ok) {
                vs.needs_upload = false;
                if (vs.node && vs.node->getAttribute("__dong_video_frame").empty()) {
                    vs.node->setAttribute("__dong_video_frame", vs.frame_key);
                }
                // Video frames are time-varying content: force a paint invalidation on each
                // successful upload so single-view dong_app keeps presenting without input.
                invalidate(InvalidationKind::Paint, nullptr, "video_frame");
                if (vs.node && cached_cmd_list) {
                    const uint64_t layer_id = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(vs.node.get()));
                    markIsolatedLayerDirtyFlags(*cached_cmd_list, layer_id);
                }
            }
        }
        return did_upload;
    }

    void syncVideoElements(double wall_time_sec, bool dom_tree_dirty) {
        if (!videoCapable() || !dom_manager) {
            clearVideoStates();
            return;
        }

        std::vector<DOMNodePtr> nodes;
        bool need_dom_scan = dom_tree_dirty || !video_dom_scanned;
        if (need_dom_scan) {
            nodes = dom_manager->getElementsByTagName("video");
            video_dom_scanned = true;
            video_dom_has_any = !nodes.empty();
        } else if (!video_dom_has_any && video_states.empty()) {
            return;
        } else {
            nodes.reserve(video_states.size());
            for (auto& kv : video_states) {
                if (kv.second.node) {
                    nodes.push_back(kv.second.node);
                }
            }
        }

        std::unordered_set<void*> live;
        for (auto& n : nodes) {
            if (!n) continue;
            live.insert(n.get());

            VideoState& vs = video_states[n.get()];
            vs.node = n;
            if (vs.frame_key.empty()) {
                vs.frame_key = "video://" + std::to_string(reinterpret_cast<uintptr_t>(n.get()));
            }

            const std::string src = n->getAttribute("src");
            const bool autoplay = n->hasAttribute("autoplay");
            const bool loop = n->hasAttribute("loop");
            const bool controls = n->hasAttribute("controls");
            bool preload = true;
            {
                std::string p = n->getAttribute("preload");
                std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c) { return (char)std::tolower(c); });
                if (p == "none") preload = false;
            }

            if (src.empty()) {
                if (vs.player) {
                    closeVideo(vs);
                }
                vs.src.clear();
                continue;
            }

            const bool need_reopen = (!vs.player || vs.src != src);
            vs.autoplay = autoplay;
            vs.loop = loop;
            vs.controls = controls;
            vs.preload = preload;

            if (need_reopen) {
                if (vs.player) {
                    closeVideo(vs);
                }
                vs.src = src;
                vs.errored = false;
                vs.ended = false;
                vs.current_pts = 0.0;
                vs.loadeddata_sent = false;
                vs.last_timeupdate_pts = 0.0;
                vs.last_debug_log_pts = 0.0;
                vs.last_debug_pull_wall = 0.0;
                vs.last_debug_state_wall = 0.0;
                vs.debug_frames_logged = 0;

                const std::string resolved = resolveVideoUrlForPlugin(src, resource_root);
                if (resolved.empty()) {
                    vs.errored = true;
                    dispatchMediaError(vs, "unsupported/invalid video src");
                    continue;
                }

                if (debug_video_enabled()) {
                    DONG_LOG_INFO("[Video] open src='%s' resolved='%s' autoplay=%d loop=%d preload=%d",
                                 src.c_str(),
                                 resolved.c_str(),
                                 vs.autoplay ? 1 : 0,
                                 vs.loop ? 1 : 0,
                                 vs.preload ? 1 : 0);
                }

                vs.player = plugin->video_open(plugin_user, resolved.c_str());
                if (!vs.player) {
                    vs.errored = true;
                    dispatchMediaError(vs, "video_open failed");
                    continue;
                }

                if (plugin->video_get_metadata) {
                    dong_video_metadata_t md{};
                    if (plugin->video_get_metadata(plugin_user, vs.player, &md) != 0) {
                        vs.meta = md;
                        vs.meta_ready = true;
                        if (vs.node) {
                            vs.node->setAttribute("__dong_video_duration", std::to_string(vs.meta.duration_seconds));
                        }
                        dispatchMediaEvent(vs, "loadedmetadata");
                    }
                }

                if (vs.preload) {
                    dong_video_frame_t f{};
                    int rr = plugin->video_read_frame(plugin_user, vs.player, &f);
                    if (rr == 1 && f.plane_data[0] && f.width > 0 && f.height > 0) {
                        static const bool kCopyVideoFrames = ([]() {
                            const char* v = std::getenv("DONG_VIDEO_FRAME_COPY");
                            return v && (std::strcmp(v, "1") == 0 || std::strcmp(v, "true") == 0);
                        })();
                        updateVideoFrameFromDecoded(vs, f, kCopyVideoFrames);
                        vs.current_pts = f.pts_seconds;
                        vs.has_frame = true;
                        vs.needs_upload = true;
                        if (vs.node) {
                            vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
                            vs.node->setAttribute("__dong_video_seeking", "0");
                            vs.node->setAttribute("__dong_video_ended", "0");
                        }
                        if (!vs.loadeddata_sent) {
                            vs.loadeddata_sent = true;
                            dispatchMediaEvent(vs, "loadeddata");
                            dispatchMediaEvent(vs, "canplay");
                        }
                    }
                }

                if (vs.autoplay) {
                    vs.playing = true;
                    vs.wall_clock_start = wall_time_sec;
                    vs.video_time_start = vs.current_pts;
                    dispatchMediaEvent(vs, "play");
                    dispatchMediaEvent(vs, "playing");
                } else {
                    vs.playing = false;
                }
            }

            const std::string seek_req = n->getAttribute("__dong_video_seek");
            if (!seek_req.empty() && plugin->video_seek) {
                char* end = nullptr;
                double t = std::strtod(seek_req.c_str(), &end);
                if (end && end != seek_req.c_str()) {
                    if (t < 0.0) t = 0.0;
                    if (vs.meta_ready && vs.meta.duration_seconds > 0.0) {
                        t = std::min(t, vs.meta.duration_seconds);
                    }
                    if (vs.node) {
                        vs.node->setAttribute("__dong_video_seeking", "1");
                        vs.node->setAttribute("__dong_video_ended", "0");
                        vs.node->setAttribute("__dong_video_currentTime", std::to_string(t));
                    }
                    plugin->video_seek(plugin_user, vs.player, t);
                    vs.current_pts = t;
                    vs.video_time_start = vs.current_pts;
                    vs.wall_clock_start = wall_time_sec;
                    vs.ended = false;
                    dispatchMediaEvent(vs, "seeking");
                }
            }

            const std::string desired_play = n->getAttribute("__dong_video_playing");
            if (!desired_play.empty()) {
                const bool want_playing = (desired_play == "1" || desired_play == "true");
                if (want_playing != vs.playing) {
                    toggleVideoPlayPause(vs, wall_time_sec);
                }
            } else {
                n->setAttribute("__dong_video_playing", vs.playing ? "1" : "0");
            }

            if (vs.playing && !vs.ended) {
                updateVideoPlayback(vs, wall_time_sec);
            }

            if (!vs.has_frame && vs.node && !vs.node->getAttribute("__dong_video_frame").empty()) {
                vs.node->setAttribute("__dong_video_frame", "");
                invalidate(InvalidationKind::Paint, nullptr, "video_frame");
            }
        }

        if (need_dom_scan) {
            for (auto it = video_states.begin(); it != video_states.end();) {
                if (live.find(it->first) == live.end()) {
                    closeVideo(it->second);
                    it = video_states.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    DOMNodePtr hitElementAt(int32_t x, int32_t y) {

        if (!dom_manager || !layout_engine) return nullptr;
        auto hit = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
        while (hit && hit->getType() != dong::dom::DOMNode::NodeType::ELEMENT) {
            hit = hit->getParent();
        }
        return hit;
    }

    // Set hover on a node and all its ancestors (browser behavior).
    static void setHoverChain(const DOMNodePtr& node, bool v) {
        for (auto n = node; n; n = n->getParent()) {
            n->setHovered(v);
        }
    }

    // Set active on a node and all its ancestors (browser behavior).
    static void setActiveChain(const DOMNodePtr& node, bool v) {
        for (auto n = node; n; n = n->getParent()) {
            n->setActive(v);
        }
    }

    void hideNativeTooltip() {
        if (!native_tooltip_node) return;
        native_tooltip_node->setInlineStyleProperty("display", "none");
        native_tooltip_target.reset();
        invalidate(InvalidationKind::Layout, nullptr, "dom_mutation");
    }

    DOMNodePtr ensureNativeTooltipNode() {
        if (native_tooltip_node) return native_tooltip_node;
        auto tooltip = std::make_shared<dong::dom::DOMNode>(dong::dom::DOMNode::NodeType::ELEMENT, "div");
        tooltip->setAttribute("data-dong-native-tooltip", "1");
        tooltip->setInlineStyleProperty("position", "fixed");
        tooltip->setInlineStyleProperty("left", "0px");
        tooltip->setInlineStyleProperty("top", "0px");
        tooltip->setInlineStyleProperty("max-width", "320px");
        tooltip->setInlineStyleProperty("padding", "6px 8px");
        tooltip->setInlineStyleProperty("border-radius", "4px");
        tooltip->setInlineStyleProperty("background", "rgba(30,30,30,0.92)");
        tooltip->setInlineStyleProperty("color", "#ffffff");
        tooltip->setInlineStyleProperty("font-size", "12px");
        tooltip->setInlineStyleProperty("line-height", "1.3");
        tooltip->setInlineStyleProperty("white-space", "pre-wrap");
        tooltip->setInlineStyleProperty("pointer-events", "none");
        tooltip->setInlineStyleProperty("z-index", "2147483647");
        tooltip->setInlineStyleProperty("display", "none");
        auto bodies = dom_manager ? dom_manager->getElementsByTagName("body") : std::vector<DOMNodePtr>{};
        auto parent = !bodies.empty() ? bodies[0] : (dom_manager ? dom_manager->getRoot() : nullptr);
        if (parent) {
            parent->appendChild(tooltip);
            native_tooltip_node = tooltip;
        }
        return native_tooltip_node;
    }

    void updateNativeTooltip(const DOMNodePtr& hit, int32_t x, int32_t y) {
        if (!hit) {
            hideNativeTooltip();
            return;
        }
        if (native_tooltip_node && hit.get() == native_tooltip_node.get()) {
            return;
        }
        const std::string title = hit->getAttribute("title");
        if (title.empty()) {
            hideNativeTooltip();
            return;
        }

        auto tooltip = ensureNativeTooltipNode();
        if (!tooltip) return;
        if (!native_tooltip_target || native_tooltip_target.get() != hit.get() || tooltip->getTextContent() != title) {
            tooltip->setTextContent(title);
            native_tooltip_target = hit;
        }

        const float px = std::clamp(static_cast<float>(x + 12), 0.0f, std::max(0.0f, static_cast<float>(width - 160)));
        const float py = std::clamp(static_cast<float>(y + 18), 0.0f, std::max(0.0f, static_cast<float>(height - 40)));
        tooltip->setInlineStyleProperty("left", toPx(px));
        tooltip->setInlineStyleProperty("top", toPx(py));
        tooltip->setInlineStyleProperty("display", "block");
        invalidate(InvalidationKind::Layout, nullptr, "dom_mutation");
    }

    bool updateTextareaResizeDrag(int32_t x, int32_t y) {
        if (!textarea_resize.active || !textarea_resize.node) return false;
        const float dx = static_cast<float>(x - textarea_resize.start_mouse_x);
        const float dy = static_cast<float>(y - textarea_resize.start_mouse_y);
        float nw = textarea_resize.start_width;
        float nh = textarea_resize.start_height;
        if (textarea_resize.mode == dom::CSSResize::Both || textarea_resize.mode == dom::CSSResize::Horizontal) nw = std::max(40.0f, nw + dx);
        if (textarea_resize.mode == dom::CSSResize::Both || textarea_resize.mode == dom::CSSResize::Vertical) nh = std::max(30.0f, nh + dy);
        if (textarea_resize.mode == dom::CSSResize::Both || textarea_resize.mode == dom::CSSResize::Horizontal) textarea_resize.node->setInlineStyleProperty("width", toPx(nw));
        if (textarea_resize.mode == dom::CSSResize::Both || textarea_resize.mode == dom::CSSResize::Vertical) textarea_resize.node->setInlineStyleProperty("height", toPx(nh));
        invalidate(InvalidationKind::Layout, nullptr, "dom_mutation");
        return true;
    }

    bool beginTextareaResize(int32_t button) {
        if (button != 1 || !layout_engine) return false;
        auto hit = hitElementAt(last_mouse_x, last_mouse_y);
        if (!hit || hit->getTagName() != "textarea" || hit->hasAttribute("disabled")) return false;
        const auto mode = hit->getComputedStyle().resize;
        if (mode != dom::CSSResize::Both && mode != dom::CSSResize::Horizontal && mode != dom::CSSResize::Vertical) return false;

        float hx = 0, hy = 0, hw = 0, hh = 0;
        if (!computeTextareaResizeHandleRect(layout_engine.get(), hit, hx, hy, hw, hh)) return false;
        if (!isPointInside(static_cast<float>(last_mouse_x), static_cast<float>(last_mouse_y), hx, hy, hw, hh)) return false;

        const auto* layout = layout_engine->getLayout(hit);
        if (!layout) return false;

        textarea_resize.active = true;
        textarea_resize.node = hit;
        textarea_resize.start_mouse_x = last_mouse_x;
        textarea_resize.start_mouse_y = last_mouse_y;
        textarea_resize.start_width = layout->width;
        textarea_resize.start_height = layout->height;
        textarea_resize.mode = mode;
        setActiveElement(hit);
        hideNativeTooltip();
        return true;
    }

    bool endTextareaResize(int32_t button, bool pressed) {
        if (!textarea_resize.active || button != 1 || pressed) return false;
        textarea_resize.active = false;
        textarea_resize.node.reset();
        textarea_resize.mode = dom::CSSResize::None;
        clearActiveElement();
        invalidate(InvalidationKind::Paint, nullptr, "input_text");
        return true;
    }

    void updateHoverState(int32_t x, int32_t y) {
        if (scene_mode_active_) return;
        auto hit = hitElementAt(x, y);
        if (hit != hovered_element) {
            setHoverChain(hovered_element, false);
            hovered_element = hit;
            setHoverChain(hovered_element, true);
            invalidate(InvalidationKind::Style, hit.get(), "hover");
        }
        updateNativeTooltip(hovered_element, x, y);
    }

    void setActiveElement(const DOMNodePtr& hit) {
        if (active_element && active_element != hit) {
            setActiveChain(active_element, false);
        }
        active_element = hit;
        if (active_element) {
            setActiveChain(active_element, true);
        }
        invalidate(InvalidationKind::Style, nullptr, "className");
    }

    void clearActiveElement() {
        if (!active_element) return;
        setActiveChain(active_element, false);
        active_element.reset();
        invalidate(InvalidationKind::Style, nullptr, "className");
    }

    bool loadHTML(const char* html_content) {
        activateViewContext();
        DONG_PROFILE_SCOPE_CAT("EngineView::loadHTML", "init");

        if (!html_content || !dom_manager) {
            return false;
        }

        dom_manager->setResourceRoot(resource_root);

        {
            DONG_PROFILE_SCOPE_CAT("HTML::parse", "init");
            if (!dom_manager->loadHTML(html_content)) {
                return false;
            }
        }

        // DOM 重载后，必须清空全局 select 状态（避免悬空 key 被复用）。
        dong::dom::clearAllSelectStates();
        open_select_element.reset();
        hovered_element.reset();
        active_element.reset();
        native_tooltip_node.reset();
        native_tooltip_target.reset();
        textarea_resize = TextareaResizeState{};

        if (js_bindings) {
            js_bindings->resetForNewDOM();
        }
        if (script_engine && script_engine->getContext()) {
            dong::script::resetFetchState(script_engine->getContext());
        }

        invalidate(InvalidationKind::Full, nullptr, "loadHTML");

        auto root = dom_manager->getRoot();
        if (root) {
            root->markLayoutDirty();
        }

        // Prime layout + scroll metrics once so <script> can reliably use scrollHeight/clientHeight.
        // (In browsers, layout is flushed on demand; without this, early scripts see scrollHeight==0.)
        if (layout_engine && painter) {
            if (auto root2 = dom_manager->getRoot()) {
                if (root2->isLayoutDirty()) {
                    DONG_PROFILE_SCOPE_CAT("Layout::primeInitial", "init");
                    layout_engine->calculateLayout(root2, static_cast<float>(width), static_cast<float>(height));
                    root2->clearLayoutDirtyRecursive();
                }
                {
                    auto fe = focus_manager ? focus_manager->getFocusedElement() : nullptr;
                    if (fe && fe->isContentEditable() && !dong::dom::isInputElement(fe))
                        painter->setEditingState(dong::dom::ContentEditableState::findEditableRoot(fe), selection.get(), caret_visible_);
                    else
                        painter->setEditingState(nullptr, nullptr);
                    if (fe && dong::dom::isInputElement(fe))
                        painter->setInputEditingState(fe, caret_visible_);
                    else
                        painter->setInputEditingState(nullptr, false);
                }
                painter->setTextRendererMode(text_renderer_mode);
                painter->buildDisplayList(root2, layout_engine.get());
            }
        }

        // Try scene graph compilation for absolute-positioned game UIs
        if (auto root3 = dom_manager->getRoot()) {
            if (dong::dom::SceneCompiler::canCompile(root3)) {
                auto& sg = dong::getSceneGraph();
                if (dong::dom::SceneCompiler::compile(root3, sg)) {
                    scene_mode_active_ = true;
                    DONG_LOG_INFO("[EngineView] Scene mode activated (%zu nodes)", sg.nodeCount());
                }
            }
        }

        // Execute <script> tags (inline or src)
        DONG_LOG_INFO("[EngineView] Script execution starting, script_engine=%p, dom_manager=%p",
                      (void*)script_engine, (void*)dom_manager.get());
        if (script_engine && dom_manager) {
            const char* disable_scripts_env = std::getenv("DONG_DISABLE_SCRIPTS");
            const bool scripts_disabled = (disable_scripts_env && disable_scripts_env[0] != '\0' && std::strcmp(disable_scripts_env, "0") != 0);
            if (scripts_disabled) {
                DONG_LOG_WARN("[EngineView] Script execution disabled by DONG_DISABLE_SCRIPTS");
            } else {
                ensureJSBindingsInitialized();

            // Scan DOM tree for inline event handlers (onclick, onchange, etc.) and register them
            if (js_bindings) {
                DONG_LOG_INFO("[EngineView] Scanning for inline event handlers...");
                js_bindings->scanAndRegisterInlineEventHandlers();
                DONG_LOG_INFO("[EngineView] Inline event handler scan completed");
            } else {
                DONG_LOG_WARN("[EngineView] js_bindings is null, skipping inline handler registration");
            }

            auto scripts = dom_manager->getElementsByTagName("script");
            DONG_LOG_INFO("[EngineView] Found %zu script tag(s)", scripts.size());

            // Phase 1: Separate module scripts from regular scripts
            std::vector<DOMNodePtr> module_scripts;
            std::vector<DOMNodePtr> regular_scripts;

            for (const auto& script : scripts) {
                if (!script) continue;
                if (isModuleScript(script)) {
                    module_scripts.push_back(script);
                } else {
                    regular_scripts.push_back(script);
                }
            }

            DONG_LOG_INFO("[EngineView] Found %zu regular script(s) and %zu ES module script(s)",
                          regular_scripts.size(), module_scripts.size());

            // Phase 2: Execute regular scripts first
            for (const auto& script : regular_scripts) {
                if (!script) continue;

                std::string code;
                std::string src = script->getAttribute("src");
                DONG_LOG_INFO("[EngineView] Processing regular script tag, src='%s'", src.c_str());

                if (!src.empty()) {
                    (void)readTextFileFromPlatformFS(src, resource_root, code);
                } else {
                    for (const auto& child : script->getChildren()) {
                        if (child && child->getType() == dong::dom::DOMNode::NodeType::TEXT) {
                            code += child->getTextContent();
                        }
                    }
                }

                DONG_LOG_INFO("[EngineView] Script code length: %zu bytes", code.length());
                if (!code.empty()) {
                    DONG_LOG_INFO("[EngineView] Executing regular script...");
                    DONG_PROFILE_SCOPE_CAT("Script::eval", "init");
                    if (js_bindings) {
                        js_bindings->setCurrentExecutingScript(script);
                    }
                    script_engine->eval(code);
                    if (js_bindings) {
                        js_bindings->clearCurrentExecutingScript();
                    }
                    DONG_LOG_INFO("[EngineView] Regular script execution completed");
                }
            }

            // Phase 3: Execute ES module scripts
            for (const auto& script : module_scripts) {
                if (!script) continue;

                std::string src = script->getAttribute("src");
                DONG_LOG_INFO("[EngineView] Processing ES module script, src='%s'", src.c_str());

                if (src.empty()) {
                    DONG_LOG_WARN("[EngineView] Module script without src attribute (inline modules not supported)");
                    continue;
                }

                std::string code;
                bool loaded = readTextFileFromPlatformFS(src, resource_root, code);
                if (!loaded) {
                    DONG_LOG_ERROR("[EngineView] Failed to load module script: %s", src.c_str());
                    continue;
                }

                DONG_LOG_INFO("[EngineView] Loaded ES module: %zu bytes", code.length());

                if (!code.empty()) {
                    DONG_LOG_INFO("[EngineView] Executing ES module script...");
                    DONG_PROFILE_SCOPE_CAT("Script::eval_module", "init");
                    if (js_bindings) {
                        js_bindings->setCurrentExecutingScript(script);
                    }

                    // Execute module using ES module evaluation
                    bool module_ok = script_engine->evalModule(src, code);

                    if (js_bindings) {
                        js_bindings->clearCurrentExecutingScript();
                    }

                    if (module_ok) {
                        DONG_LOG_INFO("[EngineView] ES module execution completed: %s", src.c_str());
                    } else {
                        DONG_LOG_ERROR("[EngineView] ES module execution failed: %s", src.c_str());
                    }
                } else {
                    DONG_LOG_WARN("[EngineView] Empty module script: %s", src.c_str());
                }
            }

            // Dispatch DOMContentLoaded event on document (body)
            if (js_bindings) {
                auto bodies = dom_manager->getElementsByTagName("body");
                dong::dom::DOMNodePtr doc_target;
                if (!bodies.empty()) {
                    doc_target = bodies[0];
                } else {
                    doc_target = dom_manager->getRoot();
                }
                if (doc_target) {
                    uint64_t nid = js_bindings->getNodeIdFor(doc_target);
                    if (!nid) {
                        JSContext* qctx = script_engine->getContext();
                        if (qctx) {
                            JSValue tmp = js_bindings->createJSElement(qctx, doc_target);
                            JS_FreeValue(qctx, tmp);
                            nid = js_bindings->getNodeIdFor(doc_target);
                        }
                    }
                    if (nid) {
                        js_bindings->dispatchSimpleEvent(nid, "DOMContentLoaded");
                        js_bindings->dispatchSimpleEvent(nid, "load");
                    }
                }
            }
            }
        } else {
            DONG_LOG_WARN("[EngineView] Cannot execute scripts: script_engine or dom_manager is null");
        }

        // In shared-JS mode, register this view's window/document on dong.views.
        if (!view_name.empty() && js_bindings) {
            js_bindings->registerAsNamedView();
        }

        // Ensure the first rendered frame reflects DOM/style mutations from scripts.
        // This is especially important for document.write and attribute-driven selectors.
        flushStyleLayoutAfterScripts();

        // P1-9: autofocus — find the first element with the autofocus attribute and focus it.
        if (focus_manager && dom_manager) {
            auto root_af = dom_manager->getRoot();
            if (root_af) {
                dong::dom::DOMNodePtr autofocus_target;
                std::function<void(const dong::dom::DOMNodePtr&)> find_autofocus =
                    [&](const dong::dom::DOMNodePtr& n) {
                        if (!n || autofocus_target) return;
                        if (n->hasAttribute("autofocus")) {
                            autofocus_target = n;
                            return;
                        }
                        for (const auto& c : n->getChildren()) {
                            find_autofocus(c);
                        }
                    };
                find_autofocus(root_af);
                if (autofocus_target) {
                    focus_manager->setFocus(autofocus_target);
                    invalidate(InvalidationKind::Paint, autofocus_target.get(), "autofocus");
                }
            }
        }

        return true;
    }

    void resize(uint32_t new_width, uint32_t new_height) {
        if (new_width == width && new_height == height) return;

        width = new_width;
        height = new_height;

        if (render_surface && render_surface->getType() == dong::render::RenderSurface::Type::CPU_BUFFER) {
            static_cast<dong::render::CPUBufferSurface*>(render_surface.get())->resize(width, height);
        }

        invalidate(InvalidationKind::Full, nullptr, "resize");

        auto root = dom_manager ? dom_manager->getRoot() : nullptr;
        if (root) {
            root->markLayoutDirty();
        }

        // Dispatch resize event on window (body element)
        if (js_bindings && script_engine && dom_manager) {
            auto bodies = dom_manager->getElementsByTagName("body");
            dong::dom::DOMNodePtr target;
            if (!bodies.empty()) {
                target = bodies[0];
            } else {
                target = root;
            }
            if (target) {
                ensureJSBindingsInitialized();
                uint64_t nid = js_bindings->getNodeIdFor(target);
                if (nid) {
                    js_bindings->dispatchSimpleEvent(nid, "resize");
                }
            }
        }
    }

    bool setGPU(void* device, void* window) {
        if (!device || !window) {
            return false;
        }

        external_gpu_device = device;
        external_window = window;
        use_gpu = true;

        if (!resource_root.empty()) {
            DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
            if (driver) {
                dong_gpu_set_resource_root(driver, resource_root.c_str());
            }
        }

        markNeedsRepaint();
        return true;
    }

    void setCurrentFragment(const std::string& fragment) {
        current_fragment = fragment;
        // 更新全局的fragment状态，以便:target伪类可以匹配新的目标元素
        dong::dom::DOMNode::setCurrentFragment(fragment);
        // 标记样式需要重新计算
        if (dom_manager) {
            // 重新计算所有节点的样式，以便:target伪类可以匹配新的目标元素
            dom_manager->recomputeNodeStyle(dom_manager->getRoot());
        }
    }

    double tickUpdateWallTimeSec() {
        static auto start_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        const double current_time = std::chrono::duration<double>(now - start_time).count();
        last_wall_time_sec = current_time;
        return current_time;
    }

    double prev_tick_time_ = 0.0;

    void tickUpdateCaretBlink(double current_time) {
        double delta_time = (prev_tick_time_ > 0.0) ? (current_time - prev_tick_time_) : 0.016;
        prev_tick_time_ = current_time;

        // Check if we need caret blinking (contenteditable collapsed selection OR focused input)
        bool needs_blink = false;
        if (selection && selection->getRangeCount() > 0 && selection->isCollapsed()) {
            needs_blink = true;
        }
        if (focus_manager) {
            auto fe = focus_manager->getFocusedElement();
            if (fe && dong::dom::isInputElement(fe)) {
                auto* state = dong::dom::getInputState(fe);
                if (state && !state->hasSelection()) {
                    needs_blink = true;
                }
            }
        }

        if (needs_blink) {
            caret_blink_time_ += delta_time;
            if (caret_blink_time_ >= CARET_BLINK_INTERVAL) {
                caret_blink_time_ -= CARET_BLINK_INTERVAL;
                caret_visible_ = !caret_visible_;
                invalidate(InvalidationKind::Paint, nullptr, "caret_blink");
            }
        } else {
            caret_visible_ = true;
            caret_blink_time_ = 0.0;
        }
    }

    void tickAdvanceSmoothScroll(double current_time) {
        if (!dom_manager) return;
        auto root_for_scroll = dom_manager->getRoot();
        if (!root_for_scroll) return;

        bool any_active = false;
        bool any_changed = false;
        updateSmoothScrollRecursive(root_for_scroll, current_time, any_active, any_changed);
        if (any_active || any_changed) {
            invalidate(InvalidationKind::Paint, nullptr, "smooth_scroll");
        }
    }

    void tickSyncVideos(double current_time) {
        bool dom_tree_dirty = false;
        if (dom_manager) {
            auto root_for_media = dom_manager->getRoot();
            dom_tree_dirty = (root_for_media && root_for_media->isLayoutDirty());
        }
        syncVideoElements(current_time, dom_tree_dirty);
    }

    void tickProcessScriptTasks() {
        if (!script_engine) return;
        DONG_PROFILE_SCOPE_CAT("Script::processTasks", "script");
        script_engine->processPendingTasks();

        // Drain completed async fetch requests
        JSContext* ctx = script_engine->getContext();
        if (ctx) {
            dong::script::tickPendingFetches(ctx);
            script_engine->processPendingTasks();
        }

        if (js_bindings) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                now - js_bindings->script_start_time_).count() / 1e6;
            js_bindings->tickTimers(elapsed);
            js_bindings->tickAnimationFrames(elapsed * 1000.0);
            flushStyleLayoutAfterScripts();
        }
    }

    void flushStyleLayoutAfterScripts() {
        if (scene_mode_active_) return;
        if (!dom_manager) return;
        auto root = dom_manager->getRoot();
        if (!root) return;

        bool style_work = false;
        bool layout_work = false;

        if (root->isStyleDirty() || root->isStyleSubtreeDirty()) {
            if (auto* se = dom_manager->getStyleEngine()) {
                DONG_PROFILE_SCOPE_CAT("Style::compute", "style");
                se->computeStylesIncremental(root);
                style_work = true;
            }
            root->clearStyleDirtyRecursive();
        }

        if (layout_engine && root->isLayoutDirty()) {
            DONG_PROFILE_SCOPE_CAT("Layout::calculate", "layout");
            layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
            root->clearLayoutDirtyRecursive();
            layout_work = true;
        }

        // P0-6 S4: Distinguish layout vs paint-only invalidation
        if (layout_work) {
            invalidate(InvalidationKind::Layout, nullptr, "dom_mutation");
        } else if (style_work) {
            // Style-only change (no layout affected): paint invalidation.
            // Note: we don't have specific dirty node info here, so use nullptr.
            invalidate(InvalidationKind::Paint, nullptr, "style_flush");
        }
    }

    void tickComputeStylesIfNeeded() {
        if (scene_mode_active_) return;
        if (!dom_manager) return;
        auto root = dom_manager->getRoot();
        if (!root) return;
        bool style_dirty = root->isStyleDirty() || root->isStyleSubtreeDirty();
        if (!style_dirty) return;

        // P0-6 S4: Check if layout is also dirty BEFORE style computation.
        bool layout_also_dirty = root->isLayoutDirty();

        if (auto* se = dom_manager->getStyleEngine()) {
            DONG_PROFILE_SCOPE_CAT("Style::compute", "style");
            se->computeStylesIncremental(root);
        }
        root->clearStyleDirtyRecursive();

        // P0-6 S4: If style changed but layout did NOT, generate targeted paint
        // invalidation using paint_dirty_ flags on DOM nodes.
        if (!layout_also_dirty && !root->isLayoutDirty()) {
            // Collect nodes with paint_dirty_ flag set
            std::vector<const void*> paint_dirty_nodes;
            std::function<void(const dong::dom::DOMNodePtr&)> collect_paint =
                [&](const dong::dom::DOMNodePtr& n) {
                    if (!n) return;
                    if (n->isPaintDirty()) {
                        paint_dirty_nodes.push_back(n.get());
                        n->clearPaintDirty();
                    }
                    for (const auto& c : n->getChildren()) {
                        collect_paint(c);
                    }
                };
            collect_paint(root);

            if (!paint_dirty_nodes.empty()) {
                for (const void* ptr : paint_dirty_nodes) {
                    invalidate(InvalidationKind::Paint, ptr, "style_paint_only");
                }
                DONG_LOG_DEBUG("[P0-6 S4] paint-only style change: %zu dirty nodes",
                              paint_dirty_nodes.size());
            } else {
                // Style changed but no paint_dirty_ nodes found: full paint
                invalidate(InvalidationKind::Paint, nullptr, "style_no_paint_nodes");
            }
        }
    }

    void tickComputeLayoutIfNeeded() {
        if (scene_mode_active_) return;
        if (!layout_engine || !dom_manager) return;
        auto root = dom_manager->getRoot();
        if (!root || !root->isLayoutDirty()) return;

        DONG_PROFILE_SCOPE_CAT("Layout::calculate", "layout");
        layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
        root->clearLayoutDirtyRecursive();
        invalidate(InvalidationKind::Layout, nullptr, "dom_mutation");
    }

    void tickSyncDialogTopLayer() {
        if (!dom_manager) return;
        auto root = dom_manager->getRoot();
        if (!root) return;

        // Rebuild top_layer from DOM: find all <dialog> with open modal state
        top_layer.clear();
        std::function<void(const dong::dom::DOMNodePtr&)> scan = [&](const dong::dom::DOMNodePtr& node) {
            if (!node) return;
            if (node->getTagName() == "dialog") {
                auto* state = dong::dom::getDialogState(node);
                if (state && state->isOpen() && state->isModal()) {
                    top_layer.push_back(node);
                }
            }
            for (const auto& child : node->getChildren()) {
                scan(child);
            }
        };
        scan(root);

        // Update focus trap to topmost modal
        if (focus_manager) {
            focus_manager->setModalDialogRoot(
                top_layer.empty() ? nullptr : top_layer.back());
        }
        // Update DOMNode static for isInert() modal awareness
        dong::dom::DOMNode::setModalDialogRoot(
            top_layer.empty() ? nullptr : top_layer.back());
    }

    // Cached DOM display list: reused when only overlay changes (skip-if-clean).
    std::vector<dong::render::DisplayItem> cached_dom_display_items_;
    bool dom_display_list_valid_ = false;
    bool scene_mode_active_ = false; // true when scene compiler took over rendering
    bool commands_regenerated_this_tick_ = false;

    void appendSceneAndOverlay(dong::render::OverlayDraw& overlay) {
        auto& sg = dong::getSceneGraph();
        if (!sg.empty()) {
            const auto& scene_items = sg.buildDisplayItems();
            if (!scene_items.empty()) {
                painter->appendOverlayItems(scene_items);
            }
        }
        if (!overlay.empty()) {
            painter->appendOverlayItems(overlay.items());
            overlay.clearDirty();
        }
    }

    void tickGenerateCommandsIfNeeded() {
        commands_regenerated_this_tick_ = false;
        const bool gpu_ready = true;
        auto& overlay = overlay_draw;
        auto& sg = dong::getSceneGraph();

        bool overlay_dirty = overlay.isDirty();
        bool scene_dirty = sg.isDirty();
        bool dom_dirty = commands_dirty_;

        // Fast path: only overlay/scene changed, DOM display list still valid
        if ((overlay_dirty || scene_dirty) && !dom_dirty && dom_display_list_valid_) {
            DONG_PROFILE_SCOPE_CAT("Render::overlayOnly", "render");

            painter->restoreDisplayList(cached_dom_display_items_);
            appendSceneAndOverlay(overlay);

            if (!cached_cmd_list) {
                cached_cmd_list = std::make_unique<dong::render::GPUCommandList>();
            }
            dong::render::GPUCompiler compiler;
            compiler.compile(painter->getDisplayList(), *cached_cmd_list, &painter->getLayerTree());
            commands_regenerated_this_tick_ = true;
            return;
        }

        if (overlay_dirty || scene_dirty)
            dom_dirty = true;
        if (!(use_gpu && dom_dirty && gpu_ready && dom_manager && layout_engine && painter)) return;

        DONG_PROFILE_SCOPE_CAT("Render::generateCommands", "render");

        auto root = dom_manager->getRoot();
        if (!root) return;

        {
            auto fe = focus_manager ? focus_manager->getFocusedElement() : nullptr;
            if (fe && fe->isContentEditable() && !dong::dom::isInputElement(fe))
                painter->setEditingState(dong::dom::ContentEditableState::findEditableRoot(fe), selection.get());
            else
                painter->setEditingState(nullptr, nullptr);
            if (fe && dong::dom::isInputElement(fe))
                painter->setInputEditingState(fe, caret_visible_);
            else
                painter->setInputEditingState(nullptr, false);
        }
        painter->setTextRendererMode(text_renderer_mode);

        // P0-6 S4: When we have paint-only invalidation with targeted dirty nodes,
        // tell the painter so it can skip unchanged nodes and reuse cached display items.
        // Also compute ancestors-of-dirty set so container nodes are NOT cached.
        bool using_partial_repaint = false;
        std::unordered_set<const void*> dirty_ancestors;
        if (isPaintOnlyInvalidation() && !dirty_node_ptrs_.empty()) {
            // Build ancestors set: for each dirty node, walk up to root
            for (const void* dirty_ptr : dirty_node_ptrs_) {
                // Walk up using DOM tree. We need to find the node from the pointer.
                // The dirty_node_ptrs contain raw DOMNode pointers.
                const auto* dirty_node = static_cast<const dong::dom::DOMNode*>(dirty_ptr);
                auto parent = dirty_node->getParent();
                while (parent) {
                    dirty_ancestors.insert(parent.get());
                    parent = parent->getParent();
                }
            }
            painter->setDirtyNodes(dirty_node_ptrs_, dirty_ancestors);
            using_partial_repaint = true;
        }

        if (scene_mode_active_) {
            painter->clearDisplayList();
        } else {
            painter->buildDisplayList(root, layout_engine.get());
        }

        if (using_partial_repaint) {
            DONG_LOG_DEBUG("[P0-6 partial] skipped=%u repainted=%u",
                          painter->getLastPartialSkipCount(),
                          painter->getLastPartialRepaintCount());
        }

        cached_dom_display_items_ = painter->getDisplayList().items;
        dom_display_list_valid_ = true;

        appendSceneAndOverlay(overlay);

        // P1-3: Render DevTools overlay on top of all content
        if (devtools_overlay.isVisible()) {
            devtools_overlay.setRoot(dom_manager->getRoot());
            devtools_overlay.setViewport(static_cast<float>(width), static_cast<float>(height));
            devtools_overlay.setMetrics(
                0,
                static_cast<uint32_t>(painter->getDisplayList().items.size()),
                0.0f);
            dong::render::DisplayListBuilder devtools_builder;
            devtools_overlay.buildOverlay(devtools_builder);
            painter->appendOverlayItems(devtools_builder.get().items);
        }

        const auto& dl = painter->getDisplayList();
        DONG_LOG_DEBUG("[tick] DisplayList items: %zu", dl.items.size());

        if (!cached_cmd_list) {
            cached_cmd_list = std::make_unique<dong::render::GPUCommandList>();
        }
        dong::render::GPUCompiler compiler;
        compiler.compile(painter->getDisplayList(), *cached_cmd_list, &painter->getLayerTree());
        DONG_LOG_DEBUG("[tick] GPU commands: %zu", cached_cmd_list->commands.size());
        commands_dirty_ = false;

        // --- P0-6: log invalidation metrics ---
        if (!pending_invalidations_.empty()) {
            invalidation_count_this_tick_ = static_cast<uint32_t>(pending_invalidations_.size());
            if (isPaintOnlyInvalidation()) {
                paint_only_tick_count_++;
                layout_skip_count_++;
                DONG_LOG_DEBUG("[P0-6 invalidation] %u Paint-only invalidations (layout skip #%u, "
                               "paint_only_ticks=%u, full_repaints=%u)",
                               invalidation_count_this_tick_, layout_skip_count_,
                               paint_only_tick_count_, full_repaint_count_);
            } else {
                full_repaint_count_++;
                DONG_LOG_DEBUG("[P0-6 invalidation] %u invalidations -> full repaint #%u "
                               "(paint_only_ticks=%u, full_repaints=%u)",
                               invalidation_count_this_tick_, full_repaint_count_,
                               paint_only_tick_count_, full_repaint_count_);
            }
            pending_invalidations_.clear();
            dirty_node_ptrs_.clear();
        }
        // --- end P0-6 ---

        commands_regenerated_this_tick_ = true;
    }

    bool video_uploaded_this_tick_ = false;

    void tickUploadVideoFramesIfNeeded() {
        if (!use_gpu) return;
        video_uploaded_this_tick_ = uploadPendingVideoFrames();
    }

    uint32_t idle_frame_count_ = 0;

    void tickExecuteGPUCommandsIfReady() {
        const bool gpu_ready = true;
        if (!(use_gpu && cached_cmd_list && !commands_dirty_ && gpu_ready)) return;

        DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
        if (!driver) {
            DONG_LOG_WARN("[tick] GPU driver not available");
            return;
        }

        if (commands_regenerated_this_tick_ || video_uploaded_this_tick_) {
            DONG_LOG_DEBUG("[tick] Executing %zu GPU commands", cached_cmd_list->commands.size());
            DONG_PROFILE_SCOPE_CAT("GPU::execute", "render");
            (void)dong_gpu_execute(driver, cached_cmd_list.get());
            idle_frame_count_ = 0;
        } else if (idle_frame_count_ < 2) {
            // Present-only for the first couple of idle frames to ensure
            // the swapchain has valid content, then stop submitting.
            DONG_PROFILE_SCOPE_CAT("GPU::presentOnly", "render");
            static dong::render::GPUCommandList empty_list;
            empty_list.commands.clear();
            empty_list.sorted_draw_indices.clear();
            (void)dong_gpu_execute(driver, &empty_list);
            ++idle_frame_count_;
        }
        // else: content unchanged, swapchain already has valid image — skip GPU entirely
    }

    bool tick() {
        activateViewContext();
        static int frame_count = 0;

        DONG_LOG_DEBUG("[tick] Frame %d starting", frame_count++);
        DONG_PROFILE_SCOPE_CAT("EngineView::tick", "frame");

        const double current_time = tickUpdateWallTimeSec();

        tickUpdateCaretBlink(current_time);

        DONG_LOG_DEBUG("[tick] step: smooth_scroll");
        {
            DONG_PROFILE_SCOPE_CAT("SmoothScroll", "tick");
            tickAdvanceSmoothScroll(current_time);
        }

        DONG_LOG_DEBUG("[tick] step: videos");
        {
            DONG_PROFILE_SCOPE_CAT("SyncVideos", "tick");
            tickSyncVideos(current_time);
        }

        // Upload any decoded video frames to GPU textures BEFORE command
        // generation. uploadPendingVideoFrames() calls invalidate(Paint),
        // which sets commands_dirty_ = true; this must happen before
        // tickGenerateCommandsIfNeeded() runs (and clears the dirty flag)
        // so that the same tick's GPU execute step actually sees the new
        // frame. If done after command generation (as before), the dirty
        // flag set by the upload would block tickExecuteGPUCommandsIfReady()
        // (which requires !commands_dirty_) every single tick, silently
        // starving GPU submission for as long as video keeps playing.
        DONG_LOG_DEBUG("[tick] step: video_upload");
        {
            DONG_PROFILE_SCOPE_CAT("UploadVideoFrames", "tick");
            tickUploadVideoFramesIfNeeded();
        }

        DONG_LOG_DEBUG("[tick] step: script_tasks");
        {
            DONG_PROFILE_SCOPE_CAT("ScriptTasks", "tick");
            tickProcessScriptTasks();
        }

        DONG_LOG_DEBUG("[tick] step: styles");
        double style_ms = 0.0;
        {
            DONG_PROFILE_SCOPE_CAT("ComputeStyles", "tick");
            auto t0 = std::chrono::steady_clock::now();
            tickComputeStylesIfNeeded();
            auto t1 = std::chrono::steady_clock::now();
            style_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        }

        DONG_LOG_DEBUG("[tick] step: layout");
        double layout_ms = 0.0;
        {
            DONG_PROFILE_SCOPE_CAT("ComputeLayout", "tick");
            auto t0 = std::chrono::steady_clock::now();
            tickComputeLayoutIfNeeded();
            auto t1 = std::chrono::steady_clock::now();
            layout_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        }

        {
            DONG_PROFILE_SCOPE_CAT("SyncDialogTopLayer", "tick");
            tickSyncDialogTopLayer();
        }

        // P1-8: Pre-warm text shape cache before paint phase.
        // Collect visible text nodes and batch-shape them for better cache locality.
        if (commands_dirty_ && dom_manager && layout_engine) {
            DONG_PROFILE_SCOPE_CAT("TextPrewarm", "tick");
            auto root_pw = dom_manager->getRoot();
            if (root_pw) {
                std::vector<dong::render::TextShapeRequest> prewarm_requests;
                std::function<void(const dong::dom::DOMNodePtr&)> collect_text =
                    [&](const dong::dom::DOMNodePtr& node) {
                        if (!node) return;
                        const auto& cs = node->getComputedStyle();
                        if (cs.display == dong::dom::CSSDisplay::None) return;

                        if (node->getType() == dong::dom::DOMNode::NodeType::TEXT) {
                            auto parent = node->getParent();
                            if (parent) {
                                const auto& ps = parent->getComputedStyle();
                                std::string text = node->getRawTextContent();
                                if (!text.empty() && text.size() < 2000) {
                                    dong::render::TextShapeRequest req;
                                    req.text = text;
                                    req.font_family = ps.font_family;
                                    req.font_weight = dong::dom::toString(ps.font_weight);
                                    req.font_style = dong::dom::toString(ps.font_style);
                                    req.font_size = ps.font_size > 0.0f ? ps.font_size : 16.0f;
                                    prewarm_requests.push_back(std::move(req));
                                }
                            }
                        }

                        for (const auto& child : node->getChildren()) {
                            collect_text(child);
                        }
                    };
                collect_text(root_pw);

                if (!prewarm_requests.empty()) {
                    dong::render::TextShaper prewarm_shaper;
                    size_t misses = prewarm_shaper.prewarmCache(prewarm_requests);
                    if (misses > 0) {
                        DONG_LOG_DEBUG("[P1-8] pre-warmed %zu text shape entries (%zu requests)",
                                      misses, prewarm_requests.size());
                    }
                }
            }
        }

        DONG_LOG_DEBUG("[tick] step: render_build");
        double paint_ms = 0.0;
        {
            DONG_PROFILE_SCOPE_CAT("GenerateCommands", "tick");
            auto t0 = std::chrono::steady_clock::now();
            tickGenerateCommandsIfNeeded();
            auto t1 = std::chrono::steady_clock::now();
            paint_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        }

        // P0-6 S6: Log tick breakdown metrics when work was done
        if (commands_regenerated_this_tick_ && (style_ms > 0.01 || layout_ms > 0.01 || paint_ms > 0.01)) {
            DONG_LOG_DEBUG("[P0-6 metrics] style=%.2fms layout=%.2fms paint+compile=%.2fms total=%.2fms",
                          style_ms, layout_ms, paint_ms,
                          style_ms + layout_ms + paint_ms);
        }

        DONG_LOG_DEBUG("[tick] step: gpu_execute");
        {
            DONG_PROFILE_SCOPE_CAT("ExecuteGPU", "tick");
            tickExecuteGPUCommandsIfReady();
        }

        return true;
    }


    void dispatchMouseEvent(const char* type, int32_t x, int32_t y, int32_t button) {
        if (!script_engine || !js_bindings || !event_dispatcher) return;

        // Scene graph gets first chance at events
        {
            auto& sg = dong::getSceneGraph();
            if (!sg.empty()) {
                std::string t = type ? type : "";
                if (sg.dispatchEvent(t, (float)x, (float)y)) {
                    return;
                }
            }
        }

        auto target = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
        if (!target) return;

        // P1-9: Block events on disabled elements and their descendants.
        // In HTML, disabled form elements (<input>, <button>, <select>, <textarea>)
        // and children of disabled <fieldset> should not receive click/mousedown events.
        {
            auto check = target;
            while (check) {
                if (check->hasAttribute("disabled")) {
                    const std::string& t = check->getTagName();
                    if (t == "input" || t == "button" || t == "select" || t == "textarea" || t == "fieldset") {
                        return;  // Swallow the event
                    }
                }
                check = check->getParent();
            }
        }

        // Pointer capture (mouse-only): route events to captured element if present.
        if (auto captured = dong::dom::DOMNode::getPointerCapture()) {
            target = captured;
        }


        dong::dom::EventType ev_type;
        std::string type_str = type ? std::string(type) : std::string();
        if (type_str == "click") {
            ev_type = dong::dom::EventType::CLICK;
        } else if (type_str == "mousedown") {
            ev_type = dong::dom::EventType::MOUSE_DOWN;
        } else if (type_str == "mouseup") {
            ev_type = dong::dom::EventType::MOUSE_UP;
        } else if (type_str == "mousemove") {
            ev_type = dong::dom::EventType::MOUSE_MOVE;
        } else {
            return;
        }

        dong::dom::Event event = event_dispatcher->createMouseEvent(ev_type, x, y, button);
        event.target = target;
        event.current_target = target;
        event.is_trusted = true;


        // Calculate offsetX/offsetY relative to target element
        if (layout_engine) {
            auto layout_node = layout_engine->getLayout(target);
            if (layout_node) {
                float elem_x = layout_node->x;
                float elem_y = layout_node->y;
                event.offset_x = static_cast<int32_t>(x - elem_x);
                event.offset_y = static_cast<int32_t>(y - elem_y);
            } else {
                // Fallback: offset == global position
                event.offset_x = x;
                event.offset_y = y;
            }
        } else {
            event.offset_x = x;
            event.offset_y = y;
        }

        event_dispatcher->dispatch(event);
    }

    void dispatchDragStartEvent(const dong::dom::DOMNodePtr& source, int32_t x, int32_t y) {
        if (!script_engine || !js_bindings || !event_dispatcher) return;
        if (!source) return;

        dong::dom::Event event = event_dispatcher->createEvent(dong::dom::EventType::DRAG_START);
        event.target = source;
        event.current_target = source;
        event.mouse_x = x;
        event.mouse_y = y;
        event.is_trusted = true;

        // Set drag data from DragManager
        if (drag_manager) {
            event.data_transfer = drag_manager->getDragData();
        }

        // Calculate offsetX/offsetY relative to source element
        if (layout_engine) {
            auto layout_node = layout_engine->getLayout(source);
            if (layout_node) {
                float elem_x = layout_node->x;
                float elem_y = layout_node->y;
                event.offset_x = static_cast<int32_t>(x - elem_x);
                event.offset_y = static_cast<int32_t>(y - elem_y);
            } else {
                // Fallback: offset == global position
                event.offset_x = x;
                event.offset_y = y;
            }
        } else {
            event.offset_x = x;
            event.offset_y = y;
        }

        event_dispatcher->dispatch(event);
    }

    void dispatchDragEvent(const dong::dom::DOMNodePtr& source, int32_t x, int32_t y) {
        if (!script_engine || !js_bindings || !event_dispatcher) return;
        if (!source) return;

        dong::dom::Event event = event_dispatcher->createEvent(dong::dom::EventType::DRAG);
        event.target = source;
        event.current_target = source;
        event.mouse_x = x;
        event.mouse_y = y;
        event.is_trusted = true;

        // Set drag data from DragManager
        if (drag_manager) {
            event.data_transfer = drag_manager->getDragData();
        }

        // Calculate offsetX/offsetY relative to source element
        if (layout_engine) {
            auto layout_node = layout_engine->getLayout(source);
            if (layout_node) {
                float elem_x = layout_node->x;
                float elem_y = layout_node->y;
                event.offset_x = static_cast<int32_t>(x - elem_x);
                event.offset_y = static_cast<int32_t>(y - elem_y);
            } else {
                // Fallback: offset == global position
                event.offset_x = x;
                event.offset_y = y;
            }
        } else {
            event.offset_x = x;
            event.offset_y = y;
        }

        event_dispatcher->dispatch(event);
    }

    void dispatchDragEndEvent(const dong::dom::DOMNodePtr& source, int32_t x, int32_t y) {
        if (!script_engine || !js_bindings || !event_dispatcher) return;
        if (!source) return;

        dong::dom::Event event = event_dispatcher->createEvent(dong::dom::EventType::DRAG_END);
        event.target = source;
        event.current_target = source;
        event.mouse_x = x;
        event.mouse_y = y;
        event.is_trusted = true;

        // Set drag data from DragManager
        if (drag_manager) {
            event.data_transfer = drag_manager->getDragData();
        }

        // Calculate offsetX/offsetY relative to source element
        if (layout_engine) {
            auto layout_node = layout_engine->getLayout(source);
            if (layout_node) {
                float elem_x = layout_node->x;
                float elem_y = layout_node->y;
                event.offset_x = static_cast<int32_t>(x - elem_x);
                event.offset_y = static_cast<int32_t>(y - elem_y);
            } else {
                // Fallback: offset == global position
                event.offset_x = x;
                event.offset_y = y;
            }
        } else {
            event.offset_x = x;
            event.offset_y = y;
        }

        event_dispatcher->dispatch(event);
    }

    void dispatchDropEvent(const dong::dom::DOMNodePtr& target, int32_t x, int32_t y) {
        if (!script_engine || !js_bindings || !event_dispatcher) return;
        if (!target) return;

        dong::dom::Event event = event_dispatcher->createEvent(dong::dom::EventType::DROP);
        event.target = target;
        event.current_target = target;
        event.mouse_x = x;
        event.mouse_y = y;
        event.is_trusted = true;

        // Set drag data from DragManager
        if (drag_manager) {
            event.data_transfer = drag_manager->getDragData();
        }

        // Calculate offsetX/offsetY relative to target element
        if (layout_engine) {
            auto layout_node = layout_engine->getLayout(target);
            if (layout_node) {
                float elem_x = layout_node->x;
                float elem_y = layout_node->y;
                event.offset_x = static_cast<int32_t>(x - elem_x);
                event.offset_y = static_cast<int32_t>(y - elem_y);
            } else {
                // Fallback: offset == global position
                event.offset_x = x;
                event.offset_y = y;
            }
        } else {
            event.offset_x = x;
            event.offset_y = y;
        }

        event_dispatcher->dispatch(event);
    }

    void dispatchKeyEvent(const char* type, uint32_t key_code) {
        if (!event_dispatcher || !dom_manager) return;
        if (!type || !type[0]) return;

        dong::dom::DOMNodePtr target;
        if (focus_manager) {
            target = focus_manager->getFocusedElement();
        }
        if (!target) {
            auto bodies = dom_manager->getElementsByTagName("body");
            target = !bodies.empty() ? bodies[0] : dom_manager->getRoot();
        }
        if (!target) return;

        dong::dom::EventType ev_type;
        const std::string type_str(type);
        if (type_str == "keydown") {
            ev_type = dong::dom::EventType::KEY_DOWN;
        } else if (type_str == "keyup") {
            ev_type = dong::dom::EventType::KEY_UP;
        } else if (type_str == "keypress") {
            ev_type = dong::dom::EventType::KEY_PRESS;
        } else {
            return;
        }

        // Check if this is a repeat (key was already pressed)
        bool is_repeat = (pressed_keys.count(key_code) > 0) && (type_str == "keydown");

        dong::dom::Event event = event_dispatcher->createKeyEvent(ev_type, key_code);
        event.target = target;
        event.current_target = target;
        event.is_trusted = true;
        event.repeat = is_repeat;
        event.alt_key = mod_alt;
        event.ctrl_key = mod_ctrl;
        event.shift_key = mod_shift;
        event.meta_key = mod_meta;
        event_dispatcher->dispatch(event);

    }

    void updateModifierState(uint32_t key_code, bool pressed) {
        constexpr uint32_t SDLK_LCTRL  = 0x400000E0;
        constexpr uint32_t SDLK_LSHIFT = 0x400000E1;
        constexpr uint32_t SDLK_LALT   = 0x400000E2;
        constexpr uint32_t SDLK_LGUI   = 0x400000E3;
        constexpr uint32_t SDLK_RCTRL  = 0x400000E4;
        constexpr uint32_t SDLK_RSHIFT = 0x400000E5;
        constexpr uint32_t SDLK_RALT   = 0x400000E6;
        constexpr uint32_t SDLK_RGUI   = 0x400000E7;

        // Track pressed keys for repeat detection
        if (pressed) {
            pressed_keys.insert(key_code);
        } else {
            pressed_keys.erase(key_code);
        }

        switch (key_code) {
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            mod_ctrl = pressed;
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            mod_shift = pressed;
            break;
        case SDLK_LALT:
        case SDLK_RALT:
            mod_alt = pressed;
            break;
        case SDLK_LGUI:
        case SDLK_RGUI:
            mod_meta = pressed;
            break;
        default:
            break;
        }
    }

    void dispatchClipboardEventIfNeeded(uint32_t key_code) {
        if (!event_dispatcher || !dom_manager) return;
        if (!mod_ctrl) return;

        std::string event_type;
        if (key_code == 'c' || key_code == 'C') {
            event_type = "copy";
        } else if (key_code == 'x' || key_code == 'X') {
            event_type = "cut";
        } else if (key_code == 'v' || key_code == 'V') {
            event_type = "paste";
        } else {
            return;
        }

        dong::dom::DOMNodePtr target;
        if (focus_manager) {
            target = focus_manager->getFocusedElement();
        }
        if (!target) {
            auto bodies = dom_manager->getElementsByTagName("body");
            target = !bodies.empty() ? bodies[0] : dom_manager->getRoot();
        }
        if (!target) return;

        // Get selected text for copy/cut events
        std::string clipboard_data;
        if ((event_type == "copy" || event_type == "cut") && dong::dom::isEditableElement(target)) {
            auto* state = dong::dom::getInputState(target);
            if (state) {
                clipboard_data = state->getSelectedText();
            }
        }

        // Dispatch C++ event for internal handling
        dong::dom::EventType ev_type;
        if (event_type == "copy") {
            ev_type = dong::dom::EventType::COPY;
        } else if (event_type == "cut") {
            ev_type = dong::dom::EventType::CUT;
        } else {
            ev_type = dong::dom::EventType::PASTE;
        }

        dong::dom::Event ev = event_dispatcher->createEvent(ev_type);
        ev.target = target;
        ev.current_target = target;
        ev.is_trusted = true;
        ev.alt_key = mod_alt;
        ev.ctrl_key = mod_ctrl;
        ev.shift_key = mod_shift;
        ev.meta_key = mod_meta;
        event_dispatcher->dispatch(ev);

        // Dispatch JS event with clipboardData property
        if (js_bindings && script_engine) {
            ensureJSBindingsInitialized();
            uint64_t nid = ensureNodeIdForJS(target);
            if (nid) {
                js_bindings->dispatchClipboardEvent(nid, event_type.c_str(),
                    clipboard_data.empty() ? nullptr : clipboard_data.c_str());
            }
        }

        // Contenteditable clipboard operations
        if (target && target->isContentEditable() && !dong::dom::isInputElement(target)) {
            if (event_type == "copy" || event_type == "cut") {
                if (selection && !selection->isCollapsed()) {
                    std::string selected_text = selection->toString();
                    DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
                    if (cb && !selected_text.empty()) {
                        dong_clipboard_set_text(cb, selected_text.c_str());
                    }
                    if (event_type == "cut") {
                        auto* range = selection->getRangeAt(0);
                        if (range) {
                            range->deleteContents();
                            selection->collapse(range->getStartContainer(), range->getStartOffset());
                        }
                        target->markLayoutDirty();
                        resetCaretBlink();
                        invalidate(InvalidationKind::Paint, nullptr, "input_text");
                    }
                }
            } else if (event_type == "paste") {
                DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
                if (cb) {
                    char* text = dong_clipboard_get_text(cb);
                    if (text) {
                        auto editable_root = dong::dom::ContentEditableState::findEditableRoot(target);
                        if (editable_root && selection) {
                            dong::dom::ContentEditableState::insertText(editable_root, *selection, text);
                            target->markLayoutDirty();
                            resetCaretBlink();
                            invalidate(InvalidationKind::Paint, nullptr, "input_text");
                        }
                        free(text);
                    }
                }
            }
        }

        // Input/textarea clipboard operations
        if (target && dong::dom::isInputElement(target)) {
            auto* state = dong::dom::getInputState(target);
            if (state) {
                if (event_type == "copy" || event_type == "cut") {
                    if (state->hasSelection()) {
                        std::string selected = state->getSelectedText();
                        DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
                        if (cb && !selected.empty()) {
                            dong_clipboard_set_text(cb, selected.c_str());
                        }
                        if (event_type == "cut" && !target->hasAttribute("readonly")) {
                            state->deleteSelection();
                            target->setAttribute("value", state->getValue());
                            if (target->getTagName() == "textarea") {
                                target->setTextContent(state->getValue());
                                target->markLayoutDirty();
                            }
                            resetCaretBlink();
                            invalidate(InvalidationKind::Paint, nullptr, "input_text");
                        }
                    }
                } else if (event_type == "paste" && !target->hasAttribute("readonly")) {
                    DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
                    if (cb) {
                        char* text = dong_clipboard_get_text(cb);
                        if (text && text[0]) {
                            state->insertText(text, target);
                            target->setAttribute("value", state->getValue());
                            if (target->getTagName() == "textarea") {
                                target->setTextContent(state->getValue());
                                target->markLayoutDirty();
                            }
                            resetCaretBlink();
                            invalidate(InvalidationKind::Paint, nullptr, "input_text");

                            if (js_bindings && script_engine) {
                                ensureJSBindingsInitialized();
                                uint64_t nid = js_bindings->getNodeIdFor(target);
                                if (nid) {
                                    js_bindings->dispatchInputEvent(nid, "insertFromPaste", text);
                                }
                            }
                            free(text);
                        }
                    }
                }
            }
        }
    }

    uint64_t ensureNodeIdForJS(const DOMNodePtr& node) {
        if (!node || !script_engine || !js_bindings) return 0;
        ensureJSBindingsInitialized();

        uint64_t node_id = js_bindings->getNodeIdFor(node);
        if (node_id) return node_id;

        JSContext* qjs = script_engine->getContext();
        if (!qjs) return 0;

        JSValue tmp = js_bindings->createJSElement(qjs, node);
        JS_FreeValue(qjs, tmp);
        return js_bindings->getNodeIdFor(node);
    }

    void dispatchSimpleEventForNode(const DOMNodePtr& node, const char* type) {
        if (!type || !type[0] || !node) return;
        if (!script_engine || !js_bindings) return;

        uint64_t node_id = ensureNodeIdForJS(node);
        if (!node_id) return;

        js_bindings->dispatchSimpleEvent(node_id, type);
    }

    void sendMouseMove(int32_t x, int32_t y) {
        activateViewContext();
        last_mouse_x = x;
        last_mouse_y = y;

        if (devtools_overlay.isVisible() &&
            devtools_overlay.handleMouseMove(static_cast<float>(x), static_cast<float>(y))) {
            invalidate(InvalidationKind::Paint, nullptr, "devtools_hover");
            return;
        }

        if (updateTextareaResizeDrag(x, y)) {
            dispatchMouseEvent("mousemove", x, y, 0);
            return;
        }

        updateHoverState(x, y);
        updateOpenSelectHover(x, y);

        if (drag_manager) {
            if (drag_manager->hasPotentialDrag()) {
                drag_manager->updateDrag(x, y);
                if (drag_manager->isDragging()) {
                    auto source = drag_manager->getDragSource();
                    if (source) {
                        dispatchDragStartEvent(source, x, y);
                    }
                }
            } else if (drag_manager->isDragging()) {
                drag_manager->updateDrag(x, y);
                auto source = drag_manager->getDragSource();
                if (source) {
                    dispatchDragEvent(source, x, y);
                }
            }
        }

        dispatchMouseEvent("mousemove", x, y, 0);

        // Contenteditable drag-to-select
        if (ce_drag_active_ && ce_drag_editable_root_ && selection && layout_engine) {
            auto hit = dong::dom::TextHitTester::hitTestAt(ce_drag_editable_root_, layout_engine.get(), x, y, &text_shaper_hit_);
            if (hit.found) {
                selection->extend(hit.text_node, hit.char_offset);
                invalidate(InvalidationKind::Paint, nullptr, "input_text");
            }
        }

        // Input/textarea drag-to-select
        if (input_drag_active_ && input_drag_node_ && layout_engine) {
            auto* state = dong::dom::getInputState(input_drag_node_);
            if (state) {
                size_t current_idx = hitTestInputCharOffset(input_drag_node_, x, y);
                if (current_idx != input_drag_anchor_) {
                    state->setSelection(input_drag_anchor_, current_idx);
                } else {
                    state->setCursorPosition(current_idx);
                }
                invalidate(InvalidationKind::Paint, nullptr, "input_text");
            }
        }
    }


    static constexpr float kSelectOptionHeight = dong::dom::kSelectOptionHeight;


    bool computeSelectGeometry(const DOMNodePtr& select,
                              float& select_x, float& select_y,
                              float& select_w, float& select_h,
                              float& dropdown_x, float& dropdown_y,
                              float& dropdown_w, float& dropdown_h) const {
        if (!layout_engine || !select) return false;
        auto layout_node = layout_engine->getLayout(select);
        if (!layout_node) return false;

        select_x = layout_node->x;
        select_y = layout_node->y;
        select_w = layout_node->width;
        select_h = layout_node->height;

        auto* state = dong::dom::getSelectState(select);
        size_t option_count = state ? state->getOptionCount() : 0;

        dropdown_x = select_x;
        dropdown_y = select_y + select_h;
        dropdown_w = select_w;
        dropdown_h = std::min(static_cast<float>(option_count) * kSelectOptionHeight, dong::dom::kSelectDropdownMaxHeight);
        return true;

    }

    DOMNodePtr normalizeSelectHit(const DOMNodePtr& hit) const {
        if (!hit) return nullptr;
        if (dong::dom::isSelectElement(hit)) return hit;
        if (hit->getTagName() == "option") {
            auto parent = hit->getParent();
            if (parent && dong::dom::isSelectElement(parent)) {
                return parent;
            }
        }
        return nullptr;
    }

    void updateOpenSelectHover(int32_t x, int32_t y) {
        if (!open_select_element) return;

        auto* state = dong::dom::getSelectState(open_select_element);
        if (!state || !state->isOpen()) return;

        float sx = 0, sy = 0, sw = 0, sh = 0;
        float dx = 0, dy = 0, dw = 0, dh = 0;
        if (!computeSelectGeometry(open_select_element, sx, sy, sw, sh, dx, dy, dw, dh)) {
            return;
        }

        const float fx = static_cast<float>(x);
        const float fy = static_cast<float>(y);

        auto inRect = [](float px, float py, float rx, float ry, float rw, float rh) {
            return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
        };

        const bool in_dropdown = (dh > 0.0f) && inRect(fx, fy, dx, dy, dw, dh);

        int new_hover = -1;
        if (in_dropdown) {
            const float rel_y = fy - dy;
            const float y_in_content = rel_y + std::max(0.0f, state->getScrollOffset());
            const size_t idx = static_cast<size_t>(y_in_content / kSelectOptionHeight);
            if (idx < state->getOptionCount() && !state->isOptionDisabled(idx)) {
                new_hover = static_cast<int>(idx);
            }
        }

        if (new_hover != state->getHoverIndex()) {
            state->setHoverIndex(new_hover);
            invalidate(InvalidationKind::Paint, nullptr, "select_state");
        }
    }

    bool handleOpenSelectMouseDown(int32_t x, int32_t y, int32_t button) {

        if (button != 1) return false;
        if (!open_select_element) return false;

        auto* state = dong::dom::getSelectState(open_select_element);
        if (!state || !state->isOpen()) {
            open_select_element.reset();
            return false;
        }

        float sx = 0, sy = 0, sw = 0, sh = 0;
        float dx = 0, dy = 0, dw = 0, dh = 0;
        if (!computeSelectGeometry(open_select_element, sx, sy, sw, sh, dx, dy, dw, dh)) {
            return false;
        }

        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);

        auto inRect = [](float px, float py, float rx, float ry, float rw, float rh) {
            return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
        };

        const bool in_select = inRect(fx, fy, sx, sy, sw, sh);
        const bool in_dropdown = (dh > 0.0f) && inRect(fx, fy, dx, dy, dw, dh);

        if (in_dropdown) {
            const float rel_y = fy - dy;
            const float y_in_content = rel_y + std::max(0.0f, state->getScrollOffset());
            size_t option_index = static_cast<size_t>(y_in_content / kSelectOptionHeight);
            if (option_index < state->getOptionCount()) {
                if (state->isOptionDisabled(option_index)) {
                    // Disabled option: consume the click, keep dropdown open.
                    return true;
                }

                const size_t prev_index = state->getSelectedIndex();
                state->selectOption(option_index);
                state->applySelectionToDOM(open_select_element);
                state->close();

                if (state->getSelectedIndex() != prev_index) {
                    dispatchSimpleEventForNode(open_select_element, "change");
                }

                open_select_element.reset();
                invalidate(InvalidationKind::Paint, nullptr, "select_state");
                return true;
            }
        }


        if (in_select) {
            state->close();
            open_select_element.reset();
            invalidate(InvalidationKind::Paint, nullptr, "select_state");
            return true;
        }

        // Click outside: close dropdown but do not consume the event.
        state->close();
        open_select_element.reset();
        invalidate(InvalidationKind::Paint, nullptr, "select_state");
        return false;
    }


    void sendMouseButton(int32_t button, bool pressed) {
        activateViewContext();

        if (devtools_overlay.isVisible() &&
            devtools_overlay.handleMouseButton(static_cast<float>(last_mouse_x),
                                               static_cast<float>(last_mouse_y),
                                               pressed, button)) {
            invalidate(InvalidationKind::Paint, nullptr, "devtools_click");
            return;
        }

        if (endTextareaResize(button, pressed)) {
            return;
        }

        if (pressed && beginTextareaResize(button)) {
            return;
        }

        updateHoverState(last_mouse_x, last_mouse_y);

        if (!pressed && drag_manager && drag_manager->isDragging()) {
            auto source = drag_manager->getDragSource();
            drop_target = hitElementAt(last_mouse_x, last_mouse_y);

            if (source) {
                if (drop_target && drop_target != source) {
                    dispatchDropEvent(drop_target, last_mouse_x, last_mouse_y);
                }
                dispatchDragEndEvent(source, last_mouse_x, last_mouse_y);
            }

            drag_manager->endDrag(last_mouse_x, last_mouse_y);
            drop_target.reset();
            invalidate(InvalidationKind::Paint, nullptr, "input_text");
        } else if (!pressed && drag_manager && drag_manager->hasPotentialDrag()) {
            drag_manager->endDrag(last_mouse_x, last_mouse_y);
        }

        if (pressed) {
            hideNativeTooltip();
            // Select 下拉框不在 DOM/layout hit-test 中：如果当前有打开的 select，优先把点击路由过去。
            if (handleOpenSelectMouseDown(last_mouse_x, last_mouse_y, button)) {
                return;
            }

            auto hit = hitElementAt(last_mouse_x, last_mouse_y);

            // Handle drag start for draggable elements (left button only)
            if (drag_manager && button == 1 && hit && hit->hasAttribute("draggable") &&
                hit->getAttribute("draggable") == "true") {
                drag_manager->beginDrag(hit, last_mouse_x, last_mouse_y);
            }

            // Select element click handling (BEFORE general click handling)
            // 兼容：如果 hit 到的是 <option>，将其归一化到父 <select>。
            auto select_hit = normalizeSelectHit(hit);
            if (select_hit && button == 1 && !select_hit->hasAttribute("disabled")) {

                auto* state = dong::dom::getSelectState(select_hit);
                if (state) {
                    state->syncFromDOM(select_hit);

                    // 只允许一个 select 下拉同时打开
                    if (open_select_element && open_select_element.get() != select_hit.get()) {
                        auto* open_state = dong::dom::getSelectState(open_select_element);
                        if (open_state && open_state->isOpen()) {
                            open_state->close();
                        }
                        open_select_element.reset();
                    }

                    if (state->isOpen()) {
                        state->close();
                        open_select_element.reset();
                    } else {
                        state->open();
                        open_select_element = select_hit;
                    }

                    setActiveElement(select_hit);
                    invalidate(InvalidationKind::Paint, nullptr, "select_state");
                    return;
                }
            }

            setActiveElement(hit);
            dispatchMouseEvent("mousedown", last_mouse_x, last_mouse_y, button);

            if (hit) {
                DONG_LOG_DEBUG("[CE-Mouse] mousedown: hit=%s ce=%d sel_ranges=%u sel_collapsed=%d",
                              hit->getTagName().c_str(), (int)hit->isContentEditable(),
                              selection ? selection->getRangeCount() : 0,
                              selection ? (int)selection->isCollapsed() : -1);
            }

            // Start drag selection for contenteditable on mousedown
            // Check both currently-focused element AND the click target itself.
            // On the first click, focused might not be the editable yet (focus moves in mouseup).
            if (focus_manager && layout_engine && selection && hit) {
                dong::dom::DOMNodePtr editable_root;

                // Try 1: currently focused element
                auto focused = focus_manager->getFocusedElement();
                if (focused && focused->isContentEditable() && !dong::dom::isInputElement(focused)) {
                    editable_root = dong::dom::ContentEditableState::findEditableRoot(focused);
                    // Verify click is inside this editable root
                    if (editable_root && !(hit == editable_root || editable_root->contains(hit))) {
                        editable_root.reset(); // click was outside, not for this editable
                    }
                }

                // Try 2: hit target itself or its ancestors might be contenteditable
                if (!editable_root && hit->isContentEditable() && !dong::dom::isInputElement(hit)) {
                    editable_root = dong::dom::ContentEditableState::findEditableRoot(hit);
                }

                if (editable_root) {
                    auto text_hit = dong::dom::TextHitTester::hitTestAt(editable_root, layout_engine.get(), last_mouse_x, last_mouse_y, &text_shaper_hit_);
                    if (text_hit.found) {
                        const std::string& txt = text_hit.text_node->getRawTextContent();
                        std::string before = txt.substr(0, std::min((uint32_t)txt.size(), text_hit.char_offset));
                        std::string after = text_hit.char_offset < txt.size() ? txt.substr(text_hit.char_offset) : "";
                        DONG_LOG_WARN("[CE-Mouse]   collapse off=%u text_size=%zu before=\"%s\" | after=\"%s\"",
                                       text_hit.char_offset, txt.size(), before.c_str(), after.c_str());
                        selection->collapse(text_hit.text_node, text_hit.char_offset);
                        if (auto* r = selection->getRangeAt(0)) {
                            auto c = r->getStartContainer();
                            DONG_LOG_WARN("[CE-Mouse]   collapsed(range) off=%u type=%s text=\"%s\"",
                                          r->getStartOffset(),
                                          c ? (c->getType() == dong::dom::DOMNode::NodeType::TEXT ? "text" : "elem") : "null",
                                          (c && c->getType() == dong::dom::DOMNode::NodeType::TEXT) ? c->getRawTextContent().c_str() : "");
                        }
                    } else {
                        // Fallback: when text hit fails (often around inline wrappers after execCommand),
                        // place caret at the end of editable content instead of hard-resetting to root start.
                        DOMNodePtr last_text;
                        std::function<void(const DOMNodePtr&)> find_last_text;
                        find_last_text = [&](const DOMNodePtr& n) {
                            if (!n) return;
                            if (n->getType() == dong::dom::DOMNode::NodeType::TEXT) {
                                last_text = n;
                                return;
                            }
                            for (const auto& c : n->getChildren()) {
                                find_last_text(c);
                            }
                        };
                        find_last_text(editable_root);
                        if (last_text) {
                            const auto end_off = static_cast<uint32_t>(last_text->getRawTextContent().size());
                            DONG_LOG_WARN("[CE-Mouse]   fallback collapse to last text end off=%u", end_off);
                            selection->collapse(last_text, end_off);
                        } else {
                            // Ensure caret is placeable even when editable subtree is temporarily empty.
                            auto new_text = std::make_shared<dong::dom::DOMNode>(dong::dom::DOMNode::NodeType::TEXT);
                            new_text->setTextContent("");
                            editable_root->appendChild(new_text);
                            editable_root->markLayoutDirty();
                            DONG_LOG_WARN("[CE-Mouse]   fallback create empty text node and collapse");
                            selection->collapse(new_text, 0);
                        }
                    }
                    ce_drag_active_ = true;
                    ce_drag_editable_root_ = editable_root;
                    // Track last editable root so execCommand works after focus loss
                    if (js_bindings) {
                        js_bindings->last_editable_root_ = editable_root;
                    }
                    resetCaretBlink();
                    invalidate(InvalidationKind::Paint, nullptr, "input_text");
                }
            }

            // Start input/textarea cursor positioning and drag selection on mousedown
            if (focus_manager && layout_engine && hit && dong::dom::isInputElement(hit) && button == 1) {
                auto* state = dong::dom::getInputState(hit);
                if (state && !hit->hasAttribute("readonly") && !hit->hasAttribute("disabled")) {
                    size_t char_idx = hitTestInputCharOffset(hit, last_mouse_x, last_mouse_y);
                    state->setCursorPosition(char_idx);
                    hit->setAttribute("value", state->getValue());
                    input_drag_active_ = true;
                    input_drag_node_ = hit;
                    input_drag_anchor_ = char_idx;
                    resetCaretBlink();
                    invalidate(InvalidationKind::Paint, nullptr, "input_text");
                }
            }

            // Dispatch contextmenu event on right-click (button 2)
            if (button == 2) {
                dispatchMouseEvent("contextmenu", last_mouse_x, last_mouse_y, button);
            }

            return;
        }

        dispatchMouseEvent("mouseup", last_mouse_x, last_mouse_y, button);

        // End contenteditable drag selection
        ce_drag_active_ = false;
        ce_drag_editable_root_.reset();

        // End input/textarea drag selection
        input_drag_active_ = false;
        input_drag_node_.reset();

        if (focus_manager && dom_manager && layout_engine) {
            auto clicked = hitTestElementAt(dom_manager.get(), layout_engine.get(), last_mouse_x, last_mouse_y);
            if (clicked) {
                focus_manager->focusOnClick(clicked);

                // Double/triple-click handling for contenteditable
                auto focused = focus_manager->getFocusedElement();
                if (focused && focused->isContentEditable() && !dong::dom::isInputElement(focused)) {
                    auto editable_root = dong::dom::ContentEditableState::findEditableRoot(focused);
                    if (editable_root && selection) {
                        // Double-click / triple-click detection
                        double now = last_wall_time_sec;
                        if (now - last_click_time_ < DOUBLE_CLICK_TIME &&
                            std::abs(last_mouse_x - last_click_x_) < DOUBLE_CLICK_DIST &&
                            std::abs(last_mouse_y - last_click_y_) < DOUBLE_CLICK_DIST) {
                            click_count_++;
                        } else {
                            click_count_ = 1;
                        }
                        last_click_time_ = now;
                        last_click_x_ = last_mouse_x;
                        last_click_y_ = last_mouse_y;

                        if (click_count_ == 2) {
                            // Double-click: select word
                            dong::dom::ContentEditableState::selectWordAtCaret(editable_root, *selection);
                            resetCaretBlink();
                            invalidate(InvalidationKind::Paint, nullptr, "input_text");
                        } else if (click_count_ >= 3) {
                            // Triple-click: select all
                            selection->selectAllChildren(editable_root);
                            resetCaretBlink();
                            invalidate(InvalidationKind::Paint, nullptr, "input_text");
                        }
                    }
                }
            }
        }

        clearActiveElement();
        dispatchMouseEvent("click", last_mouse_x, last_mouse_y, button);

        // Handle <label> for attribute - clicking label focuses associated element
        if (dom_manager && layout_engine && focus_manager) {
            auto clicked = hitTestElementAt(dom_manager.get(), layout_engine.get(), last_mouse_x, last_mouse_y);
            if (clicked && clicked->getTagName() == "label" && clicked->hasAttribute("for")) {
                std::string target_id = clicked->getAttribute("for");
                if (!target_id.empty()) {
                    auto target_element = dom_manager->getElementById(target_id);
                    if (target_element) {
                        // Check if target is a focusable element
                        std::string tag = target_element->getTagName();
                        if (tag == "input" || tag == "textarea" || tag == "select" || tag == "button") {
                            focus_manager->setFocus(target_element);
                            invalidate(InvalidationKind::Paint, nullptr, "focus");
                        }
                    }
                }
            }
        }

        // Checkbox/radio toggle on click
        if (dom_manager && layout_engine) {
            auto clicked = hitTestElementAt(dom_manager.get(), layout_engine.get(), last_mouse_x, last_mouse_y);
            if (clicked && clicked->getTagName() == "input" && !clicked->hasAttribute("disabled")) {
                std::string type = clicked->getAttribute("type");
                if (type == "checkbox") {
                    if (clicked->hasAttribute("checked")) {
                        clicked->removeAttribute("checked");
                    } else {
                        clicked->setAttribute("checked", "");
                    }
                    invalidate(InvalidationKind::Paint, nullptr, "checkbox_toggle");
                    // Dispatch change event
                    dispatchSimpleEventForNode(clicked, "change");
                } else if (type == "radio") {
                    if (!clicked->hasAttribute("checked")) {
                        // Uncheck other radios in same name group
                        std::string name = clicked->getAttribute("name");
                        if (!name.empty()) {
                            auto inputs = dom_manager->getElementsByTagName("input");
                            for (auto& inp : inputs) {
                                if (inp.get() != clicked.get() &&
                                    inp->getAttribute("type") == "radio" &&
                                    inp->getAttribute("name") == name) {
                                    inp->removeAttribute("checked");
                                }
                            }
                        }
                        clicked->setAttribute("checked", "");
                        invalidate(InvalidationKind::Paint, nullptr, "checkbox_toggle");
                        dispatchSimpleEventForNode(clicked, "change");
                    }
                }
            }

            // Handle <summary> click - toggle parent <details>
            if (clicked && dong::dom::isSummaryElement(clicked) && button == 1) {
                // Find parent details element
                auto parent = clicked->getParent();
                auto details_element = DOMNodePtr();
                while (parent) {
                    if (dong::dom::isDetailsElement(parent)) {
                        details_element = parent;
                        break;
                    }
                    parent = parent->getParent();
                }

                if (details_element) {
                    auto* state = dong::dom::getDetailsState(details_element);
                    if (state) {
                        state->toggle();
                        state->applyOpenStateToDOM(details_element);
                        // Dispatch toggle event
                        dispatchSimpleEventForNode(details_element, "toggle");
                        invalidate(InvalidationKind::Layout, nullptr, "dom_mutation");
                        return;  // Don't let this click continue to general handling
                    }
                }
            }

            // Handle <a> anchor click - scroll to target element
            if (clicked && clicked->getTagName() == "a" && clicked->hasAttribute("href") && button == 1) {
                std::string href = clicked->getAttribute("href");
                if (!href.empty() && href[0] == '#') {
                    std::string target_id = href.substr(1);
                    if (!target_id.empty()) {
                        auto target_element = dom_manager->getElementById(target_id);
                        if (target_element) {
                            // Scroll to target element
                            target_element->scrollIntoView(true);
                            // Update URL fragment for :target pseudo-class
                            setCurrentFragment(target_id);
                            invalidate(InvalidationKind::Style, nullptr, "setAttribute");
                        }
                    }
                }
            }
        }
    }



    void sendMouseWheel(float delta_x, float delta_y) {
        activateViewContext();
        constexpr float kScrollSpeed = 20.0f;
        float scroll_dx = delta_x * kScrollSpeed;
        float scroll_dy = delta_y * kScrollSpeed;

        if (devtools_overlay.isVisible() &&
            devtools_overlay.handleMouseWheel(static_cast<float>(last_mouse_x),
                                              static_cast<float>(last_mouse_y),
                                              scroll_dy)) {
            invalidate(InvalidationKind::Paint, nullptr, "devtools_scroll");
            return;
        }

        // Select 下拉框滚动：下拉不在 DOM 树中（不属于 scroll container），因此需要在这里优先处理。
        if (open_select_element) {
            auto* state = dong::dom::getSelectState(open_select_element);
            if (state && state->isOpen()) {
                float sx = 0, sy = 0, sw = 0, sh = 0;
                float dx = 0, dy = 0, dw = 0, dh = 0;
                if (computeSelectGeometry(open_select_element, sx, sy, sw, sh, dx, dy, dw, dh)) {
                    const float fx = static_cast<float>(last_mouse_x);
                    const float fy = static_cast<float>(last_mouse_y);
                    auto inRect = [](float px, float py, float rx, float ry, float rw, float rh) {
                        return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
                    };

                    const bool in_dropdown = (dh > 0.0f) && inRect(fx, fy, dx, dy, dw, dh);
                    if (in_dropdown) {
                        const float prev = state->getScrollOffset();
                        state->scrollBy(scroll_dy, dh);
                        if (state->getScrollOffset() != prev) {
                            updateOpenSelectHover(last_mouse_x, last_mouse_y);
                            invalidate(InvalidationKind::Paint, open_select_element.get(), "scroll");
                        }
                        return;
                    }
                }
            }
        }

        auto scroll_container = findScrollContainerAt(dom_manager.get(), layout_engine.get(), last_mouse_x, last_mouse_y);
        if (!scroll_container) {
            return;
        }


        // Try to scroll the current container
        auto result = scroll_container->scrollBy(scroll_dx, scroll_dy);

        // Check overscroll-behavior for scroll chaining
        const auto& style = scroll_container->getComputedStyle();
        bool allow_chain_x = (style.overscroll_behavior_x == dom::CSSOverscrollBehavior::Auto);
        bool allow_chain_y = (style.overscroll_behavior_y == dom::CSSOverscrollBehavior::Auto);

        // If scroll was not fully consumed and chaining is allowed, propagate to parent
        bool should_chain_x = !result.consumed_x && allow_chain_x && (scroll_dx != 0.0f);
        bool should_chain_y = !result.consumed_y && allow_chain_y && (scroll_dy != 0.0f);

        if (should_chain_x || should_chain_y) {
            // Find parent scroll container
            auto parent = scroll_container->getParent();
            while (parent) {
                if (parent->isScrollContainer()) {
                    float remaining_dx = should_chain_x ? scroll_dx : 0.0f;
                    float remaining_dy = should_chain_y ? scroll_dy : 0.0f;
                    parent->scrollBy(remaining_dx, remaining_dy);
                    scroll_container = parent;  // Update for event dispatch
                    break;
                }
                parent = parent->getParent();
            }
        }

        invalidate(InvalidationKind::Paint, scroll_container.get(), "scroll");

        // Dispatch scroll event on the scroll container
        if (js_bindings && script_engine) {
            ensureJSBindingsInitialized();
            uint64_t nid = js_bindings->getNodeIdFor(scroll_container);
            if (nid) {
                js_bindings->dispatchSimpleEvent(nid, "scroll");
            }
        }
    }

    void sendKey(uint32_t key_code, bool pressed) {
        activateViewContext();
        constexpr uint32_t SDLK_TAB = 9;
        constexpr uint32_t SDLK_RETURN = 13;
        constexpr uint32_t SDLK_KP_ENTER = 0x40000058;
        constexpr uint32_t SDLK_BACKSPACE = 8;
        constexpr uint32_t SDLK_DELETE = 127;
        constexpr uint32_t SDLK_LEFT = 0x40000050;
        constexpr uint32_t SDLK_RIGHT = 0x4000004F;
        constexpr uint32_t SDLK_UP = 0x40000052;
        constexpr uint32_t SDLK_DOWN = 0x40000051;
        constexpr uint32_t SDLK_F12 = 0x40000045;

        updateModifierState(key_code, pressed);

        // P1-3: F12 toggles DevTools overlay
        if (pressed && key_code == SDLK_F12) {
            devtools_overlay.toggle();
            if (devtools_overlay.isVisible() && dom_manager) {
                devtools_overlay.setRoot(dom_manager->getRoot());
                devtools_overlay.setViewport(static_cast<float>(width), static_cast<float>(height));
            }
            invalidate(InvalidationKind::Paint, nullptr, "devtools_toggle");
            return;
        }

        if (pressed) {
            dispatchClipboardEventIfNeeded(key_code);

            // Escape closes topmost modal dialog
            constexpr uint32_t SDLK_ESCAPE_DIALOG = 27;
            if (key_code == SDLK_ESCAPE_DIALOG && !top_layer.empty()) {
                auto dialog = top_layer.back();
                auto* state = dong::dom::getDialogState(dialog);
                if (state && state->isOpen() && state->isModal()) {
                    // Dispatch cancel event (cancelable)
                    if (event_dispatcher) {
                        dong::dom::Event cancel_evt = event_dispatcher->createEvent(dong::dom::EventType::CANCEL);
                        cancel_evt.target = dialog;
                        cancel_evt.current_target = dialog;
                        event_dispatcher->dispatch(cancel_evt);
                        if (!cancel_evt.prevented) {
                            state->close("");
                            dialog->removeAttribute("open");
                            dialog->markStyleDirty();
                            dialog->markLayoutDirty();
                            top_layer.pop_back();
                            focus_manager->setModalDialogRoot(
                                top_layer.empty() ? nullptr : top_layer.back());

                            // Dispatch close event
                            dong::dom::Event close_evt = event_dispatcher->createEvent(dong::dom::EventType::CLOSE);
                            close_evt.target = dialog;
                            close_evt.current_target = dialog;
                            event_dispatcher->dispatch(close_evt);
                        }
                    }
                    invalidate(InvalidationKind::Layout, nullptr, "dialog");
                    return;
                }
            }

            if (key_code == SDLK_TAB && focus_manager && dom_manager) {
                focus_manager->moveFocus(dom_manager->getRoot(), false);
                return;
            }

            // Handle select element keyboard navigation
            if (focus_manager && dom_manager) {
                constexpr uint32_t SDLK_ESCAPE = 27;

                auto focused = focus_manager->getFocusedElement();
                if (focused && focused->getTagName() == "select" && !focused->hasAttribute("disabled")) {
                    auto* state = dong::dom::getSelectState(focused);
                    if (state) {
                        state->syncFromDOM(focused);

                        auto nextEnabled = [&](size_t start, int dir) {
                            if (state->getOptionCount() == 0) return start;
                            size_t idx = start;
                            for (size_t step = 0; step < state->getOptionCount(); ++step) {
                                int ni = static_cast<int>(idx) + dir;
                                if (ni < 0 || ni >= static_cast<int>(state->getOptionCount())) break;
                                idx = static_cast<size_t>(ni);
                                if (!state->isOptionDisabled(idx)) return idx;
                            }
                            return start;
                        };

                        auto ensureVisible = [&](size_t idx) {
                            float sx = 0, sy = 0, sw = 0, sh = 0;
                            float dx = 0, dy = 0, dw = 0, dh = 0;
                            if (!computeSelectGeometry(focused, sx, sy, sw, sh, dx, dy, dw, dh)) return;
                            if (dh <= 0.0f) return;

                            const float top = static_cast<float>(idx) * kSelectOptionHeight;
                            const float bottom = top + kSelectOptionHeight;
                            const float scroll = std::max(0.0f, state->getScrollOffset());

                            if (top < scroll) {
                                state->scrollBy(top - scroll, dh);
                            } else if (bottom > scroll + dh) {
                                state->scrollBy(bottom - (scroll + dh), dh);
                            }
                        };

                        bool handled = false;

                        if (key_code == SDLK_UP || key_code == SDLK_DOWN) {
                            const int dir = (key_code == SDLK_DOWN) ? 1 : -1;

                            if (state->isOpen()) {
                                const int h = state->getHoverIndex();
                                const size_t base = (h >= 0) ? static_cast<size_t>(h) : state->getSelectedIndex();
                                const size_t next = nextEnabled(base, dir);
                                state->setHoverIndex(static_cast<int>(next));
                                ensureVisible(next);
                                invalidate(InvalidationKind::Paint, nullptr, "select_state");
                                handled = true;
                            } else {
                                const size_t prev = state->getSelectedIndex();
                                const size_t next = nextEnabled(prev, dir);
                                if (next != prev) {
                                    state->selectOption(next);
                                    state->applySelectionToDOM(focused);
                                    invalidate(InvalidationKind::Paint, nullptr, "select_state");
                                    dispatchSimpleEventForNode(focused, "change");
                                }
                                handled = true;
                            }
                        } else if (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) {
                            if (state->isOpen()) {
                                const int h = state->getHoverIndex();
                                if (h >= 0) {
                                    const size_t prev = state->getSelectedIndex();
                                    state->selectOption(static_cast<size_t>(h));
                                    state->applySelectionToDOM(focused);
                                    if (state->getSelectedIndex() != prev) {
                                        dispatchSimpleEventForNode(focused, "change");
                                    }
                                }

                                state->close();
                                if (open_select_element.get() == focused.get()) {
                                    open_select_element.reset();
                                }
                                invalidate(InvalidationKind::Paint, nullptr, "select_state");
                                handled = true;
                            } else {
                                state->open();
                                open_select_element = focused;
                                invalidate(InvalidationKind::Paint, nullptr, "select_state");
                                handled = true;
                            }
                        } else if (key_code == SDLK_ESCAPE) {
                            if (state->isOpen()) {
                                state->close();
                                if (open_select_element.get() == focused.get()) {
                                    open_select_element.reset();
                                }
                                invalidate(InvalidationKind::Paint, nullptr, "select_state");
                                handled = true;
                            }
                        }

                        if (handled) {
                            dispatchKeyEvent("keydown", key_code);
                            return;
                        }
                    }
                }
            }


            // Handle Enter key in form inputs
            if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && focus_manager && dom_manager) {
                auto focused = focus_manager->getFocusedElement();
                if (focused && focused->getTagName() == "input") {
                    // Find parent form element
                    auto current = focused->getParent();
                    while (current) {
                        if (current->getTagName() == "form") {
                            // Trigger submit event on the form
                            if (js_bindings && script_engine) {
                                ensureJSBindingsInitialized();
                                uint64_t form_id = js_bindings->getNodeIdFor(current);
                                if (form_id) {
                                    js_bindings->dispatchSimpleEvent(form_id, "submit");
                                }
                            }
                            // Prevent default text insertion
                            dispatchKeyEvent("keydown", key_code);
                            return;
                        }
                        current = current->getParent();
                    }
                }
            }

            if (focus_manager) {
                auto focused = focus_manager->getFocusedElement();
                if (focused && dong::dom::isEditableElement(focused) && !focused->hasAttribute("readonly")) {
                    auto* state = dong::dom::getInputState(focused);
                    if (state) {
                        bool handled = false;
                        const char* input_type = nullptr;

                        if (key_code == SDLK_BACKSPACE) {
                            input_type = "deleteContentBackward";
                            if (dispatchBeforeInputForNode(focused, input_type, nullptr)) {
                                dispatchKeyEvent("keydown", key_code);
                                return;
                            }
                            state->deleteBackward();
                            handled = true;
                        } else if (key_code == SDLK_DELETE) {
                            input_type = "deleteContentForward";
                            if (dispatchBeforeInputForNode(focused, input_type, nullptr)) {
                                dispatchKeyEvent("keydown", key_code);
                                return;
                            }
                            state->deleteForward();
                            handled = true;
                        } else if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && focused->getTagName() == "textarea") {
                            // Textarea Enter: insert newline
                            input_type = "insertLineBreak";
                            if (dispatchBeforeInputForNode(focused, input_type, "\n")) {
                                dispatchKeyEvent("keydown", key_code);
                                return;
                            }
                            state->insertText("\n", focused);
                            handled = true;
                        } else if (key_code == SDLK_LEFT || key_code == SDLK_RIGHT) {
                            if (mod_shift) {
                                // Shift+Arrow: extend selection
                                size_t cursor = state->getCursorPosition();
                                size_t new_pos = (key_code == SDLK_RIGHT)
                                    ? std::min(cursor + 1, state->charCount())
                                    : (cursor > 0 ? cursor - 1 : 0);
                                if (state->hasSelection()) {
                                    // Extend existing selection
                                    size_t sel_start = state->getSelectionStart();
                                    size_t sel_end = state->getSelectionEnd();
                                    // Determine which end to move
                                    if (cursor == sel_end) {
                                        state->setSelection(sel_start, new_pos);
                                    } else {
                                        state->setSelection(new_pos, sel_end);
                                    }
                                } else {
                                    // Start new selection
                                    state->setSelection(cursor, new_pos);
                                }
                                handled = true;
                            } else {
                                // Arrow without shift: if selection exists, move to edge
                                if (state->hasSelection()) {
                                    size_t pos = (key_code == SDLK_LEFT) ? state->getSelectionStart() : state->getSelectionEnd();
                                    state->setCursorPosition(pos);
                                } else {
                                    state->moveCursor(key_code == SDLK_RIGHT ? 1 : -1);
                                }
                                handled = true;
                            }
                        } else if (key_code == 0x4000004A /* SDLK_HOME */) {
                            state->setCursorPosition(0);
                            handled = true;
                        } else if (key_code == 0x4000004D /* SDLK_END */) {
                            state->setCursorPosition(state->charCount());
                            handled = true;
                        } else if ((key_code == 0x40000052 /* SDLK_UP */ || key_code == 0x40000051 /* SDLK_DOWN */) && focused->getTagName() == "textarea") {
                            // Up/Down arrow for textarea multiline navigation
                            const std::string& val = state->getValue();
                            size_t cursor = state->getCursorPosition();
                            // Split value into lines and find current line/col
                            std::vector<std::string> lines;
                            {
                                std::string::size_type start = 0;
                                for (;;) {
                                    auto pos = val.find('\n', start);
                                    if (pos == std::string::npos) {
                                        lines.push_back(val.substr(start));
                                        break;
                                    }
                                    lines.push_back(val.substr(start, pos - start));
                                    start = pos + 1;
                                }
                            }
                            // Convert char index to (line, col)
                            int cur_line = 0;
                            size_t chars_before = 0;
                            for (size_t li = 0; li < lines.size(); li++) {
                                // Count UTF-8 chars in this line
                                size_t line_chars = 0;
                                for (size_t bi = 0; bi < lines[li].size(); ) {
                                    unsigned char c = (unsigned char)lines[li][bi];
                                    if (c < 0x80) bi += 1;
                                    else if (c < 0xE0) bi += 2;
                                    else if (c < 0xF0) bi += 3;
                                    else bi += 4;
                                    line_chars++;
                                }
                                if (cursor <= chars_before + line_chars) {
                                    cur_line = (int)li;
                                    break;
                                }
                                chars_before += line_chars + 1; // +1 for '\n'
                                cur_line = (int)li;
                            }
                            size_t cur_col = cursor - chars_before;

                            int target_line = (key_code == 0x40000052) ? cur_line - 1 : cur_line + 1;
                            if (target_line >= 0 && target_line < (int)lines.size()) {
                                // Count chars in target line
                                size_t target_line_chars = 0;
                                for (size_t bi = 0; bi < lines[target_line].size(); ) {
                                    unsigned char c = (unsigned char)lines[target_line][bi];
                                    if (c < 0x80) bi += 1;
                                    else if (c < 0xE0) bi += 2;
                                    else if (c < 0xF0) bi += 3;
                                    else bi += 4;
                                    target_line_chars++;
                                }
                                size_t target_col = std::min(cur_col, target_line_chars);
                                // Convert (target_line, target_col) back to global char index
                                size_t global_idx = 0;
                                for (int li = 0; li < target_line; li++) {
                                    size_t lc = 0;
                                    for (size_t bi = 0; bi < lines[li].size(); ) {
                                        unsigned char c = (unsigned char)lines[li][bi];
                                        if (c < 0x80) bi += 1;
                                        else if (c < 0xE0) bi += 2;
                                        else if (c < 0xF0) bi += 3;
                                        else bi += 4;
                                        lc++;
                                    }
                                    global_idx += lc + 1; // +1 for '\n'
                                }
                                global_idx += target_col;
                                state->setCursorPosition(global_idx);
                                handled = true;
                            }
                        } else if (mod_ctrl && (key_code == 'a' || key_code == 'A')) {
                            state->selectAll();
                            handled = true;
                        }

                        if (handled) {
                            focused->setAttribute("value", state->getValue());
                            // For textarea, also sync child text node for rendering
                            if (focused->getTagName() == "textarea") {
                                focused->setTextContent(state->getValue());
                                focused->markLayoutDirty();
                            }
                            resetCaretBlink();
                            invalidate(InvalidationKind::Paint, nullptr, "input_text");

                            // Dispatch input event for delete/insert operations
                            if (input_type && js_bindings && script_engine) {
                                ensureJSBindingsInitialized();
                                uint64_t nid = js_bindings->getNodeIdFor(focused);
                                if (nid) {
                                    js_bindings->dispatchInputEvent(nid, input_type, nullptr);
                                }
                            }
                            dispatchKeyEvent("keydown", key_code);
                            return;
                        }
                    }
                }
            }

            // Handle contenteditable keyboard (Backspace, Delete, Enter, Arrow keys, Home/End, Ctrl+A)
            if (focus_manager && dom_manager) {
                auto focused = focus_manager->getFocusedElement();
                if (focused && focused->isContentEditable() && !dong::dom::isInputElement(focused)) {
                    auto editable_root = dong::dom::ContentEditableState::findEditableRoot(focused);
                    if (editable_root && selection) {
                        // Stabilize CE keyboard behavior: ensure we always have a valid collapsed range.
                        if (selection->getRangeCount() == 0) {
                            dong::dom::DOMNodePtr last_text;
                            std::function<void(const dong::dom::DOMNodePtr&)> find_last_text;
                            find_last_text = [&](const dong::dom::DOMNodePtr& n) {
                                if (!n) return;
                                if (n->getType() == dong::dom::DOMNode::NodeType::TEXT) {
                                    last_text = n;
                                    return;
                                }
                                for (const auto& c : n->getChildren()) {
                                    find_last_text(c);
                                }
                            };
                            find_last_text(editable_root);
                            if (!last_text) {
                                last_text = std::make_shared<dong::dom::DOMNode>(dong::dom::DOMNode::NodeType::TEXT);
                                last_text->setTextContent("");
                                editable_root->appendChild(last_text);
                                editable_root->markLayoutDirty();
                            }
                            selection->collapse(last_text, static_cast<uint32_t>(last_text->getRawTextContent().size()));
                        }

                        constexpr uint32_t SDLK_HOME = 0x4000004A;
                        constexpr uint32_t SDLK_END  = 0x4000004D;

                        bool ce_handled = false;
                        if (key_code == SDLK_BACKSPACE) {
                            ce_handled = dong::dom::ContentEditableState::deleteBackward(editable_root, *selection);
                        } else if (key_code == SDLK_DELETE) {
                            ce_handled = dong::dom::ContentEditableState::deleteForward(editable_root, *selection);
                        } else if (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) {
                            if (auto* r = selection->getRangeAt(0)) {
                                auto c = r->getStartContainer();
                                DONG_LOG_WARN("[CE-Key] Enter pre-insert: collapsed=%d off=%u type=%s text=\"%s\"",
                                              (int)selection->isCollapsed(),
                                              r->getStartOffset(),
                                              c ? (c->getType() == dong::dom::DOMNode::NodeType::TEXT ? "text" : "elem") : "null",
                                              (c && c->getType() == dong::dom::DOMNode::NodeType::TEXT) ? c->getRawTextContent().c_str() : "");
                            }
                            ce_handled = dong::dom::ContentEditableState::insertParagraph(editable_root, *selection);
                        } else if (key_code == SDLK_LEFT || key_code == SDLK_RIGHT) {
                            int dir = (key_code == SDLK_RIGHT) ? 1 : -1;
                            if (mod_shift) {
                                ce_handled = dong::dom::ContentEditableState::extendSelection(editable_root, *selection, dir);
                            } else {
                                ce_handled = dong::dom::ContentEditableState::moveCaret(editable_root, *selection, dir);
                            }
                        } else if (key_code == SDLK_UP || key_code == SDLK_DOWN) {
                            int dir = (key_code == SDLK_DOWN) ? 1 : -1;
                            ce_handled = dong::dom::ContentEditableState::moveCaretVertical(editable_root, *selection, dir);
                        } else if (key_code == SDLK_HOME || key_code == SDLK_END) {
                            ce_handled = dong::dom::ContentEditableState::moveCaretToLineEdge(editable_root, *selection, key_code == SDLK_END);
                        } else if (mod_ctrl && (key_code == 'a' || key_code == 'A')) {
                            // Ctrl+A: select all content
                            selection->selectAllChildren(editable_root);
                            ce_handled = true;
                        }
                        if (ce_handled) {
                            resetCaretBlink();
                            invalidate(InvalidationKind::Paint, nullptr, "input_text");
                            dispatchKeyEvent("keydown", key_code);
                            return;
                        }
                    }
                }
            }

            dispatchKeyEvent("keydown", key_code);
        } else {
            dispatchKeyEvent("keyup", key_code);
        }
    }

    void sendText(const char* text) {
        activateViewContext();
        if (!text || !text[0]) return;

        dong::dom::DOMNodePtr focused;
        if (focus_manager) {
            focused = focus_manager->getFocusedElement();
        }

        if (focused && dong::dom::isEditableElement(focused)) {
            // Check readonly
            if (focused->hasAttribute("readonly")) {
                return;
            }

            auto* state = dong::dom::getInputState(focused);
            if (state) {
                // 如果正在组合中，先结束组合（endComposition 会撤回预编辑文本）
                if (state->isComposing()) {
                    finishComposition(focused, state);
                }

                if (dispatchBeforeInputForNode(focused, "insertText", text)) {
                    return;
                }

                state->insertText(text, focused);
                focused->setAttribute("value", state->getValue());
                // For textarea, also sync child text node for rendering
                if (focused->getTagName() == "textarea") {
                    focused->setTextContent(state->getValue());
                    focused->markLayoutDirty();
                }
                resetCaretBlink();
                invalidate(InvalidationKind::Paint, nullptr, "input_text");


                // Dispatch input event with inputType
                if (js_bindings && script_engine) {
                    ensureJSBindingsInitialized();
                    uint64_t nid = js_bindings->getNodeIdFor(focused);
                    if (nid) {
                        js_bindings->dispatchInputEvent(nid, "insertText", text);
                    }
                }
            }
        }

        // Handle contenteditable text input
        if (focused && focused->isContentEditable() && !dong::dom::isInputElement(focused)) {
            auto editable_root = dong::dom::ContentEditableState::findEditableRoot(focused);
            if (editable_root && selection) {
                dong::dom::ContentEditableState::insertText(editable_root, *selection, text);
                resetCaretBlink();
                invalidate(InvalidationKind::Paint, nullptr, "input_text");
            }
        }
    }

    void sendTextEditing(const char* text, int32_t cursor, int32_t selection_length) {
        activateViewContext();
        dong::dom::DOMNodePtr focused;
        if (focus_manager) {
            focused = focus_manager->getFocusedElement();
        }
        if (!focused || !dong::dom::isEditableElement(focused)) return;
        if (focused->hasAttribute("readonly")) return;

        auto* state = dong::dom::getInputState(focused);
        if (!state) return;

        bool is_empty = (!text || !text[0]);

        if (is_empty && state->isComposing()) {
            // 空文本 = 组合结束
            finishComposition(focused, state);
        } else if (!is_empty && !state->isComposing()) {
            // 首次非空 = 组合开始
            beginComposition(focused, state, text);
        } else if (!is_empty && state->isComposing()) {
            // 后续非空 = 组合更新
            continueComposition(focused, state, text);
        }
        // is_empty && !isComposing → 无事发生
    }

    // =========================================================================
    // Spatial Navigation / Gamepad
    // =========================================================================

    bool focusNav(int dir) {
        activateViewContext();
        if (!focus_manager || !dom_manager || !layout_engine) {
            return false;
        }

        // DONG_NAV_NEXT / DONG_NAV_PREV → delegate to tab-order navigation
        if (dir == 4 /* DONG_NAV_NEXT */) {
            focus_manager->moveFocus(dom_manager->getRoot(), false);
            invalidate(InvalidationKind::Paint, nullptr, "focus");
            return true;
        }
        if (dir == 5 /* DONG_NAV_PREV */) {
            focus_manager->moveFocus(dom_manager->getRoot(), true);
            invalidate(InvalidationKind::Paint, nullptr, "focus");
            return true;
        }

        // Spatial: Up/Down/Left/Right
        dong::input::NavDirection nav_dir;
        switch (dir) {
            case 0: nav_dir = dong::input::NavDirection::Up; break;
            case 1: nav_dir = dong::input::NavDirection::Down; break;
            case 2: nav_dir = dong::input::NavDirection::Left; break;
            case 3: nav_dir = dong::input::NavDirection::Right; break;
            default: return false;
        }

        auto current = focus_manager->getFocusedElement();

        // Collect all focusable elements
        std::vector<dong::dom::DOMNodePtr> candidates;
        collectFocusableCandidates(dom_manager->getRoot(), candidates);

        if (candidates.empty()) {
            return false;
        }

        // If no current focus, pick the first candidate
        if (!current) {
            focus_manager->setKeyboardFocus(true);
            focus_manager->setFocus(candidates.front());
            invalidate(InvalidationKind::Paint, nullptr, "focus");
            return true;
        }

        auto target = dong::input::findSpatialNavTarget(current, nav_dir, candidates, layout_engine.get());
        if (!target) {
            return false;
        }

        focus_manager->setKeyboardFocus(true);
        focus_manager->setFocus(target);
        invalidate(InvalidationKind::Paint, nullptr, "focus");
        return true;
    }

    bool sendGamepadButton(int32_t gamepad_id, int button, bool pressed) {
        activateViewContext();

        // Only handle press events for navigation/actions
        if (!pressed) {
            return true;
        }

        // Dispatch JS 'gamepadbutton' event on focused element (or document)
        bool prevented = dispatchGamepadButtonEvent(gamepad_id, button, pressed);
        if (prevented) {
            return true;
        }

        // Default behaviors if not preventDefault'd
        switch (button) {
            case 0: // DONG_GAMEPAD_DPAD_UP
                return focusNav(0);
            case 1: // DONG_GAMEPAD_DPAD_DOWN
                return focusNav(1);
            case 2: // DONG_GAMEPAD_DPAD_LEFT
                return focusNav(2);
            case 3: // DONG_GAMEPAD_DPAD_RIGHT
                return focusNav(3);
            case 4: // DONG_GAMEPAD_BUTTON_A → click focused element
                return clickFocusedElement();
            case 5: // DONG_GAMEPAD_BUTTON_B → cancel / close dialog
                return handleGamepadCancel();
            default:
                break;
        }
        return true;
    }

private:
    // Collect all focusable elements from the DOM tree (for spatial navigation)
    void collectFocusableCandidates(const dong::dom::DOMNodePtr& node,
                                     std::vector<dong::dom::DOMNodePtr>& out) {
        if (!node) return;
        if (node->hasAttribute("inert")) return;

        if (dong::dom::FocusManager::isFocusable(node)) {
            // Skip elements with no layout (hidden/not rendered)
            const auto* layout = layout_engine->getLayout(node);
            if (layout && layout->width > 0 && layout->height > 0) {
                out.push_back(node);
            }
        }

        for (const auto& child : node->getChildren()) {
            collectFocusableCandidates(child, out);
        }
    }

    // Dispatch 'gamepadbutton' JS event on focused element. Returns true if preventDefault'd.
    bool dispatchGamepadButtonEvent(int32_t gamepad_id, int button, bool pressed) {
        if (!js_bindings || !script_engine) return false;
        ensureJSBindingsInitialized();

        JSContext* ctx = script_engine->getContext();
        if (!ctx) return false;

        // Target: focused element or document body
        dong::dom::DOMNodePtr target;
        if (focus_manager) {
            target = focus_manager->getFocusedElement();
        }
        if (!target && dom_manager) {
            target = dom_manager->getRoot();
        }
        if (!target) return false;

        uint64_t node_id = ensureNodeIdForJS(target);
        if (!node_id) return false;

        // Build event object
        JSValue ev = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, "gamepadbutton"));
        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "isTrusted", JS_TRUE);
        JS_SetPropertyStr(ctx, ev, "gamepadId", JS_NewInt32(ctx, gamepad_id));
        JS_SetPropertyStr(ctx, ev, "button", JS_NewInt32(ctx, button));
        JS_SetPropertyStr(ctx, ev, "pressed", pressed ? JS_TRUE : JS_FALSE);

        // Map button to name string
        const char* button_name = "unknown";
        switch (button) {
            case 0: button_name = "dpad_up"; break;
            case 1: button_name = "dpad_down"; break;
            case 2: button_name = "dpad_left"; break;
            case 3: button_name = "dpad_right"; break;
            case 4: button_name = "a"; break;
            case 5: button_name = "b"; break;
            case 6: button_name = "x"; break;
            case 7: button_name = "y"; break;
            case 8: button_name = "lb"; break;
            case 9: button_name = "rb"; break;
            case 10: button_name = "start"; break;
            case 11: button_name = "back"; break;
        }
        JS_SetPropertyStr(ctx, ev, "buttonName", JS_NewString(ctx, button_name));

        // dispatchEventObject returns true if NOT prevented
        bool not_prevented = js_bindings->dispatchEventObject(node_id, ev);
        JS_FreeValue(ctx, ev);
        return !not_prevented; // return true if prevented
    }

    // Simulate a click on the currently focused element
    bool clickFocusedElement() {
        if (!focus_manager) return false;
        auto focused = focus_manager->getFocusedElement();
        if (!focused) return false;

        // Get the center of the element for the click coordinates
        if (layout_engine) {
            const auto* layout = layout_engine->getLayout(focused);
            if (layout) {
                int32_t cx = static_cast<int32_t>(layout->x + layout->width * 0.5f);
                int32_t cy = static_cast<int32_t>(layout->y + layout->height * 0.5f);
                // Simulate mouse move to element center, then click
                sendMouseMove(cx, cy);
                sendMouseButton(1, true);  // button 1 = SDL left
                sendMouseButton(1, false);
                return true;
            }
        }
        return false;
    }

    // Handle gamepad B button: dispatch 'gamepadcancel', default close topmost dialog
    bool handleGamepadCancel() {
        // Dispatch 'gamepadcancel' event on focused element
        if (js_bindings && script_engine) {
            ensureJSBindingsInitialized();
            JSContext* ctx = script_engine->getContext();
            if (ctx) {
                dong::dom::DOMNodePtr target;
                if (focus_manager) {
                    target = focus_manager->getFocusedElement();
                }
                if (!target && dom_manager) {
                    target = dom_manager->getRoot();
                }
                if (target) {
                    uint64_t node_id = ensureNodeIdForJS(target);
                    if (node_id) {
                        JSValue ev = JS_NewObject(ctx);
                        JS_SetPropertyStr(ctx, ev, "type", JS_NewString(ctx, "gamepadcancel"));
                        JS_SetPropertyStr(ctx, ev, "bubbles", JS_TRUE);
                        JS_SetPropertyStr(ctx, ev, "cancelable", JS_TRUE);
                        JS_SetPropertyStr(ctx, ev, "isTrusted", JS_TRUE);

                        // dispatchEventObject returns true if NOT prevented
                        bool not_prevented = js_bindings->dispatchEventObject(node_id, ev);
                        JS_FreeValue(ctx, ev);
                        if (!not_prevented) return true; // prevented
                    }
                }
            }
        }

        // Default: close topmost dialog
        if (!top_layer.empty()) {
            auto dialog = top_layer.back();
            auto* state = dong::dom::getDialogState(dialog);
            if (state && state->isOpen()) {
                state->close("");
                dialog->removeAttribute("open");
                dialog->markStyleDirty();
                dialog->markLayoutDirty();
                top_layer.pop_back();
                if (focus_manager) {
                    focus_manager->setModalDialogRoot(
                        top_layer.empty() ? nullptr : top_layer.back());
                }
                invalidate(InvalidationKind::Layout, nullptr, "dialog");
                return true;
            }
        }
        return false;
    }

    void dispatchCompositionJS(const dong::dom::DOMNodePtr& node,
                               const char* type, const char* data) {
        if (!js_bindings || !script_engine) return;
        ensureJSBindingsInitialized();
        uint64_t nid = ensureNodeIdForJS(node);
        if (!nid) return;
        js_bindings->dispatchCompositionEvent(nid, type, data);
    }

    void dispatchInputEventForNode(const dong::dom::DOMNodePtr& node,
                                    const char* input_type, const char* data) {
        if (!js_bindings || !script_engine) return;
        ensureJSBindingsInitialized();
        uint64_t nid = ensureNodeIdForJS(node);
        if (!nid) return;
        js_bindings->dispatchInputEvent(nid, input_type, data);
    }

    bool dispatchBeforeInputForNode(const dong::dom::DOMNodePtr& node,
                                   const char* input_type,
                                   const char* data) {
        if (!js_bindings || !script_engine) return false;
        ensureJSBindingsInitialized();
        uint64_t nid = ensureNodeIdForJS(node);
        if (!nid) return false;
        if (!js_bindings->hasEventListeners(nid, "beforeinput")) return false;
        return js_bindings->dispatchBeforeInputEvent(nid, input_type, data);
    }


    void beginComposition(const dong::dom::DOMNodePtr& focused,
                          dong::dom::InputElementState* state,
                          const char* text) {
        state->startComposition(text);
        focused->setAttribute("value", state->getValue());
        invalidate(InvalidationKind::Paint, nullptr, "input_text");

        // W3C: compositionstart.data is "" (or selected text, which is empty here)
        dispatchCompositionJS(focused, "compositionstart", "");
        dispatchCompositionJS(focused, "compositionupdate", text);
        dispatchInputEventForNode(focused, "insertCompositionText", text);
    }

    void continueComposition(const dong::dom::DOMNodePtr& focused,
                             dong::dom::InputElementState* state,
                             const char* text) {
        state->updateComposition(text);
        focused->setAttribute("value", state->getValue());
        invalidate(InvalidationKind::Paint, nullptr, "input_text");

        dispatchCompositionJS(focused, "compositionupdate", text);
        dispatchInputEventForNode(focused, "insertCompositionText", text);
    }

    void finishComposition(const dong::dom::DOMNodePtr& focused,
                           dong::dom::InputElementState* state) {
        std::string final_data = state->getCompositionText();
        state->endComposition();
        focused->setAttribute("value", state->getValue());
        invalidate(InvalidationKind::Paint, nullptr, "input_text");

        dispatchCompositionJS(focused, "compositionend", final_data.c_str());
    }

public:

    bool evalScript(const char* code) {
        activateViewContext();
        if (!code || !script_engine) {
            return false;
        }
        DONG_PROFILE_SCOPE_CAT("Script::evalScript", "script");
        ensureJSBindingsInitialized();
        return script_engine->eval(std::string(code));
    }
};

EngineView::EngineView(uint32_t width, uint32_t height)
    : impl_(std::make_unique<Impl>(width, height)) {}

EngineView::EngineView(uint32_t width, uint32_t height,
                       dong::script::ScriptEngine* shared_script_engine)
    : impl_(std::make_unique<Impl>(width, height, shared_script_engine)) {}

EngineView::~EngineView() = default;

bool EngineView::isInitialized() const {
    return impl_ ? impl_->ctx.isInitialized() : false;
}

void EngineView::setResourceRoot(const char* root) {
    if (!impl_) return;
    impl_->resource_root = root ? root : "";

    // Propagate to the Painter so it can resolve image paths for broken-image detection.
    if (impl_->painter) {
        impl_->painter->setResourceRoot(impl_->resource_root);
    }

    DongPlatform* platform = dong_platform_get();
    DongGPUDriver* driver = platform ? dong_platform_get_gpu_driver(platform) : nullptr;
    if (driver) {
        dong_gpu_set_resource_root(driver, impl_->resource_root.c_str());
    }
}

void EngineView::setPlugin(const dong_plugin_vtable_t* plugin, void* plugin_user) {
    if (impl_) {
        impl_->setPlugin(plugin, plugin_user);
    }
}

const char* EngineView::getCursorAt(int32_t x, int32_t y) {

    if (!impl_ || !impl_->dom_manager || !impl_->layout_engine) {
        impl_->cached_cursor = "auto";
        return impl_->cached_cursor.c_str();
    }

    auto element = hitTestElementAt(impl_->dom_manager.get(), impl_->layout_engine.get(), x, y);
    if (!element) {
        impl_->cached_cursor = "auto";
        return impl_->cached_cursor.c_str();
    }

    auto current = element;
    while (current) {
        const auto& cs = current->getComputedStyle();
        if (cs.cursor != dom::CSSCursor::Auto) {
            impl_->cached_cursor = dom::toString(cs.cursor);
            return impl_->cached_cursor.c_str();
        }
        // P1-9: user-select: none — show default cursor instead of text cursor
        if (cs.user_select == dom::CSSUserSelect::None) {
            impl_->cached_cursor = "default";
            return impl_->cached_cursor.c_str();
        }
        current = current->getParent();
    }

    impl_->cached_cursor = "auto";
    return impl_->cached_cursor.c_str();
}

bool EngineView::loadHTML(const char* html) {
    return impl_ ? impl_->loadHTML(html) : false;
}



void EngineView::resize(uint32_t width, uint32_t height) {
    if (impl_) impl_->resize(width, height);
}

bool EngineView::setGPU(void* gpu_device, void* window) {
    return impl_ ? impl_->setGPU(gpu_device, window) : false;
}

bool EngineView::tick() {
    return impl_ ? impl_->tick() : false;
}

void EngineView::sendMouseMove(int32_t x, int32_t y) {
    if (impl_) impl_->sendMouseMove(x, y);
}

void EngineView::sendMouseButton(int32_t button, bool pressed) {
    if (impl_) impl_->sendMouseButton(button, pressed);
}

void EngineView::sendMouseWheel(float delta_x, float delta_y) {
    if (impl_) impl_->sendMouseWheel(delta_x, delta_y);
}

void EngineView::sendKey(uint32_t key_code, bool pressed) {
    if (impl_) impl_->sendKey(key_code, pressed);
}

void EngineView::sendText(const char* text) {
    if (impl_) impl_->sendText(text);
}

void EngineView::setCurrentFragment(const std::string& fragment) {
    if (impl_) impl_->setCurrentFragment(fragment);
}

void EngineView::sendTextEditing(const char* text, int32_t cursor, int32_t selection_length) {
    if (impl_) impl_->sendTextEditing(text, cursor, selection_length);
}

bool EngineView::focusNav(int dir) {
    return impl_ ? impl_->focusNav(dir) : false;
}

bool EngineView::sendGamepadButton(int32_t gamepad_id, int button, bool pressed) {
    return impl_ ? impl_->sendGamepadButton(gamepad_id, button, pressed) : false;
}

bool EngineView::evalScript(const char* code) {
    return impl_ ? impl_->evalScript(code) : false;
}

const void* EngineView::getCommandList() const {
    return (impl_ && impl_->cached_cmd_list) ? impl_->cached_cmd_list.get() : nullptr;
}

void EngineView::invalidateCommands() {
    if (impl_) impl_->markNeedsRepaint();
}

void EngineView::setViewName(const char* name) {
    if (impl_ && name) {
        impl_->view_name = name;
        if (impl_->js_bindings) {
            impl_->js_bindings->setViewName(name);
        }
    }
}

dong::script::ScriptEngine* EngineView::getScriptEngine() const {
    return impl_ ? impl_->script_engine : nullptr;
}

void EngineView::setTextRendererMode(render::TextRendererMode mode) {
    if (impl_) {
        impl_->text_renderer_mode = mode;
    }
}

render::TextRendererMode EngineView::getTextRendererMode() const {
    return impl_ ? impl_->text_renderer_mode : render::TextRendererMode::Auto;
}

} // namespace dong
