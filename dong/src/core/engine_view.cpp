#include "engine_view.hpp"

#include "context.hpp"
#include "log.h"
#include "profiler.h"

#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_file_system.h"

#include "../dom/dom_manager.hpp"
#include "../dom/style_engine.hpp"
#include "../dom/event_system.hpp"
#include "../dom/focus_manager.hpp"
#include "../dom/input_element.hpp"
#include "../dom/select_element.hpp"
#include "../layout/layout_engine.hpp"

#include "../render/render_surface.hpp"
#include "../render/painter.hpp"
#include "../render/gpu_ir.hpp"
#include "../script/script_engine.hpp"
#include "../script/js_bindings.hpp"

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
#include <unordered_map>
#include <unordered_set>






namespace dong {

namespace {

using dong::dom::DOMNodePtr;

DOMNodePtr hitTestRecursive(const DOMNodePtr& node, dong::layout::Engine* layout_engine,
                            int32_t x, int32_t y) {
    if (!node || !layout_engine) return nullptr;

    const auto* layout = layout_engine->getLayout(node);
    if (!layout) return nullptr;

    float lx = layout->x;
    float ly = layout->y;
    float w = layout->width;
    float h = layout->height;

    bool in_bounds = (x >= lx && x <= lx + w && y >= ly && y <= ly + h);

    if (!in_bounds) {
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
    return hitTestRecursive(root, layout_engine, x, y);
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


    std::unique_ptr<dong::dom::Manager> dom_manager;
    std::unique_ptr<dong::layout::Engine> layout_engine;
    std::unique_ptr<dong::render::RenderSurface> render_surface;
    std::unique_ptr<dong::render::Painter> painter;

    std::unique_ptr<dong::dom::EventDispatcher> event_dispatcher;
    std::unique_ptr<dong::dom::FocusManager> focus_manager;
    std::unique_ptr<dong::script::ScriptEngine> script_engine;
    std::unique_ptr<dong::script::JSBindings> js_bindings;

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
    bool commands_dirty = true;
    bool js_bindings_initialized = false;

    bool video_dom_scanned = false;
    bool video_dom_has_any = false;
    double last_wall_time_sec = 0.0;


    int32_t last_mouse_x = 0;
    int32_t last_mouse_y = 0;

    DOMNodePtr hovered_element;
    DOMNodePtr active_element;

    // 当前打开的 <select>（用于把下拉框区域的点击正确路由到该 select）
    DOMNodePtr open_select_element;

    Impl(uint32_t w, uint32_t h)

        : width(w), height(h),
          dom_manager(std::make_unique<dong::dom::Manager>()),
          layout_engine(std::make_unique<dong::layout::Engine>()),
          render_surface(std::make_unique<dong::render::CPUBufferSurface>(w, h)),
          painter(std::make_unique<dong::render::Painter>(render_surface.get())),
          event_dispatcher(std::make_unique<dong::dom::EventDispatcher>()),
          focus_manager(std::make_unique<dong::dom::FocusManager>()),
          script_engine(std::make_unique<dong::script::ScriptEngine>()),
          js_bindings(std::make_unique<dong::script::JSBindings>(
              script_engine.get(),
              dom_manager.get(),
              event_dispatcher.get(),
              focus_manager.get())) {
        focus_manager->setEventDispatcher(event_dispatcher.get());
        dong::dom::DOMNode::setFocusManager(focus_manager.get());
        dong::dom::DOMNode::setEventDispatcher(event_dispatcher.get());
        ctx.initialize();
    }

    ~Impl() {
        ctx.shutdown();
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

    void markNeedsRepaint() {
        commands_dirty = true;
        if (render_surface) {
            render_surface->markDirty();
        }
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
            markNeedsRepaint();
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
                    markNeedsRepaint();
                }
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
                markNeedsRepaint();
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

    void updateHoverState(int32_t x, int32_t y) {
        auto hit = hitElementAt(x, y);
        if (hit == hovered_element) return;

        // Clear old chain
        setHoverChain(hovered_element, false);
        hovered_element = hit;
        // Set new chain
        setHoverChain(hovered_element, true);
        markNeedsRepaint();
    }

    void setActiveElement(const DOMNodePtr& hit) {
        if (active_element && active_element != hit) {
            setActiveChain(active_element, false);
        }
        active_element = hit;
        if (active_element) {
            setActiveChain(active_element, true);
        }
        markNeedsRepaint();
    }

    void clearActiveElement() {
        if (!active_element) return;
        setActiveChain(active_element, false);
        active_element.reset();
        markNeedsRepaint();
    }

    bool loadHTML(const char* html_content) {

        if (!html_content || !dom_manager) {
            return false;
        }

        if (!dom_manager->loadHTML(html_content)) {
            return false;
        }

        // DOM 重载后，必须清空全局 select 状态（避免悬空 key 被复用）。
        dong::dom::clearAllSelectStates();
        open_select_element.reset();

        if (js_bindings) {
            js_bindings->resetForNewDOM();
        }

        markNeedsRepaint();

        auto root = dom_manager->getRoot();
        if (root) {
            root->markLayoutDirty();
        }

        // Prime layout + scroll metrics once so <script> can reliably use scrollHeight/clientHeight.
        // (In browsers, layout is flushed on demand; without this, early scripts see scrollHeight==0.)
        if (layout_engine && painter) {
            if (auto root2 = dom_manager->getRoot()) {
                if (root2->isLayoutDirty()) {
                    layout_engine->calculateLayout(root2, static_cast<float>(width), static_cast<float>(height));
                    root2->clearLayoutDirtyRecursive();
                }
                painter->buildDisplayList(root2, layout_engine.get());
            }
        }

        // Execute <script> tags (inline or src)
        DONG_LOG_INFO("[EngineView] Script execution starting, script_engine=%p, dom_manager=%p",
                      (void*)script_engine.get(), (void*)dom_manager.get());
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
            for (const auto& script : scripts) {
                if (!script) continue;

                std::string code;
                std::string src = script->getAttribute("src");
                DONG_LOG_INFO("[EngineView] Processing script tag, src='%s'", src.c_str());

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
                    DONG_LOG_INFO("[EngineView] Executing script...");
                    script_engine->eval(code);
                    DONG_LOG_INFO("[EngineView] Script execution completed");
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
                    }
                }
            }
            }
        } else {
            DONG_LOG_WARN("[EngineView] Cannot execute scripts: script_engine or dom_manager is null");
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

        markNeedsRepaint();

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

    double tickUpdateWallTimeSec() {
        static auto start_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        const double current_time = std::chrono::duration<double>(now - start_time).count();
        last_wall_time_sec = current_time;
        return current_time;
    }

    void tickAdvanceSmoothScroll(double current_time) {
        if (!dom_manager) return;
        auto root_for_scroll = dom_manager->getRoot();
        if (!root_for_scroll) return;

        bool any_active = false;
        bool any_changed = false;
        updateSmoothScrollRecursive(root_for_scroll, current_time, any_active, any_changed);
        if (any_active || any_changed) {
            markNeedsRepaint();
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
    }

    void tickComputeStylesIfNeeded() {
        if (!dom_manager) return;
        auto root = dom_manager->getRoot();
        if (!root) return;
        if (!(root->isStyleDirty() || root->isStyleSubtreeDirty())) return;

        if (auto* se = dom_manager->getStyleEngine()) {
            DONG_PROFILE_SCOPE_CAT("Style::compute", "style");
            se->computeStylesIncremental(root);
        }
        root->clearStyleDirtyRecursive();
    }

    void tickComputeLayoutIfNeeded() {
        if (!layout_engine || !dom_manager) return;
        auto root = dom_manager->getRoot();
        if (!root || !root->isLayoutDirty()) return;

        DONG_PROFILE_SCOPE_CAT("Layout::calculate", "layout");
        layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
        root->clearLayoutDirtyRecursive();
        markNeedsRepaint();
    }

    void tickGenerateCommandsIfNeeded() {
        const bool gpu_ready = true;
        if (!(use_gpu && commands_dirty && gpu_ready && dom_manager && layout_engine && painter)) return;

        DONG_PROFILE_SCOPE_CAT("Render::generateCommands", "render");

        auto root = dom_manager->getRoot();
        if (!root) return;

        painter->buildDisplayList(root, layout_engine.get());
        const auto& dl = painter->getDisplayList();
        DONG_LOG_DEBUG("[tick] DisplayList items: %zu", dl.items.size());

        int text_count = 0;
        for (const auto& item : dl.items) {
            if (item.type == dong::render::DisplayItemType::DrawGlyphRun) {
                text_count++;
                DONG_LOG_DEBUG("[tick]   DrawGlyphRun: glyphs=%zu", item.glyph_run.glyphs.size());
            }
        }
        DONG_LOG_DEBUG("[tick] Total DrawGlyphRun items: %d", text_count);

        if (!cached_cmd_list) {
            cached_cmd_list = std::make_unique<dong::render::GPUCommandList>();
        }
        dong::render::GPUCompiler compiler;
        compiler.compile(painter->getDisplayList(), *cached_cmd_list, &painter->getLayerTree());
        DONG_LOG_DEBUG("[tick] GPU commands: %zu", cached_cmd_list->commands.size());
        commands_dirty = false;
    }

    void tickUploadVideoFramesIfNeeded() {
        if (!use_gpu) return;
        (void)uploadPendingVideoFrames();
    }

    void tickExecuteGPUCommandsIfReady() {
        const bool gpu_ready = true;
        if (!(use_gpu && cached_cmd_list && !commands_dirty && gpu_ready)) return;

        DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
        if (!driver) {
            DONG_LOG_WARN("[tick] GPU driver not available");
            return;
        }

        DONG_LOG_DEBUG("[tick] Executing %zu GPU commands", cached_cmd_list->commands.size());
        DONG_PROFILE_SCOPE_CAT("GPU::execute", "render");
        (void)dong_gpu_execute(driver, cached_cmd_list.get());
    }

    bool tick() {
        static int frame_count = 0;

        DONG_LOG_DEBUG("[tick] Frame %d starting", frame_count++);
        DONG_PROFILE_SCOPE_CAT("EngineView::tick", "frame");

        const double current_time = tickUpdateWallTimeSec();

        DONG_LOG_DEBUG("[tick] step: smooth_scroll");
        tickAdvanceSmoothScroll(current_time);

        DONG_LOG_DEBUG("[tick] step: videos");
        tickSyncVideos(current_time);

        DONG_LOG_DEBUG("[tick] step: script_tasks");
        tickProcessScriptTasks();

        DONG_LOG_DEBUG("[tick] step: styles");
        tickComputeStylesIfNeeded();

        DONG_LOG_DEBUG("[tick] step: layout");
        tickComputeLayoutIfNeeded();

        DONG_LOG_DEBUG("[tick] step: render_build");
        tickGenerateCommandsIfNeeded();

        DONG_LOG_DEBUG("[tick] step: video_upload");
        tickUploadVideoFramesIfNeeded();

        DONG_LOG_DEBUG("[tick] step: gpu_execute");
        tickExecuteGPUCommandsIfReady();

        return true;
    }


    void dispatchMouseEvent(const char* type, int32_t x, int32_t y, int32_t button) {
        if (!script_engine || !js_bindings || !event_dispatcher) return;

        auto target = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
        if (!target) return;

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
        if (!script_engine || !js_bindings) return;
        if (!dom_manager) return;

        auto bodies = dom_manager->getElementsByTagName("body");
        dong::dom::DOMNodePtr target;
        if (!bodies.empty()) {
            target = bodies[0];
        } else {
            target = dom_manager->getRoot();
        }
        if (!target) return;

        JSContext* qjs = script_engine->getContext();
        if (!qjs) return;

        uint64_t node_id = js_bindings->getNodeIdFor(target);
        if (!node_id) {
            JSValue tmp = js_bindings->createJSElement(qjs, target);
            JS_FreeValue(qjs, tmp);
            node_id = js_bindings->getNodeIdFor(target);
            if (!node_id) {
                return;
            }
        }

        js_bindings->dispatchKeyEvent(node_id, type, key_code);
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
        last_mouse_x = x;
        last_mouse_y = y;
        updateHoverState(x, y);
        updateOpenSelectHover(x, y);
        dispatchMouseEvent("mousemove", x, y, 0);
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
            markNeedsRepaint();
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
                markNeedsRepaint();
                return true;
            }
        }


        if (in_select) {
            state->close();
            open_select_element.reset();
            markNeedsRepaint();
            return true;
        }

        // Click outside: close dropdown but do not consume the event.
        state->close();
        open_select_element.reset();
        markNeedsRepaint();
        return false;
    }


    void sendMouseButton(int32_t button, bool pressed) {
        updateHoverState(last_mouse_x, last_mouse_y);

        if (pressed) {
            // Select 下拉框不在 DOM/layout hit-test 中：如果当前有打开的 select，优先把点击路由过去。
            if (handleOpenSelectMouseDown(last_mouse_x, last_mouse_y, button)) {
                return;
            }

            auto hit = hitElementAt(last_mouse_x, last_mouse_y);

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
                    markNeedsRepaint();
                    return;
                }
            }

            setActiveElement(hit);
            dispatchMouseEvent("mousedown", last_mouse_x, last_mouse_y, button);

            // Dispatch contextmenu event on right-click (button 2)
            if (button == 2) {
                dispatchMouseEvent("contextmenu", last_mouse_x, last_mouse_y, button);
            }

            return;
        }

        dispatchMouseEvent("mouseup", last_mouse_x, last_mouse_y, button);

        if (focus_manager && dom_manager && layout_engine) {
            auto clicked = hitTestElementAt(dom_manager.get(), layout_engine.get(), last_mouse_x, last_mouse_y);
            if (clicked) {
                focus_manager->focusOnClick(clicked);
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
                            markNeedsRepaint();
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
                    markNeedsRepaint();
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
                        markNeedsRepaint();
                        dispatchSimpleEventForNode(clicked, "change");
                    }
                }
            }
        }
    }



    void sendMouseWheel(float delta_x, float delta_y) {
        constexpr float kScrollSpeed = 20.0f;
        float scroll_dx = delta_x * kScrollSpeed;
        float scroll_dy = delta_y * kScrollSpeed;

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
                            markNeedsRepaint();
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
        bool allow_chain_x = (style.overscroll_behavior_x == "auto");
        bool allow_chain_y = (style.overscroll_behavior_y == "auto");

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

        markNeedsRepaint();

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
        constexpr uint32_t SDLK_TAB = 9;
        constexpr uint32_t SDLK_RETURN = 13;
        constexpr uint32_t SDLK_BACKSPACE = 8;
        constexpr uint32_t SDLK_DELETE = 127;
        constexpr uint32_t SDLK_LEFT = 0x40000050;
        constexpr uint32_t SDLK_RIGHT = 0x4000004F;
        constexpr uint32_t SDLK_UP = 0x40000052;
        constexpr uint32_t SDLK_DOWN = 0x40000051;

        if (pressed) {
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
                                markNeedsRepaint();
                                handled = true;
                            } else {
                                const size_t prev = state->getSelectedIndex();
                                const size_t next = nextEnabled(prev, dir);
                                if (next != prev) {
                                    state->selectOption(next);
                                    state->applySelectionToDOM(focused);
                                    markNeedsRepaint();
                                    dispatchSimpleEventForNode(focused, "change");
                                }
                                handled = true;
                            }
                        } else if (key_code == SDLK_RETURN) {
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
                                markNeedsRepaint();
                                handled = true;
                            } else {
                                state->open();
                                open_select_element = focused;
                                markNeedsRepaint();
                                handled = true;
                            }
                        } else if (key_code == SDLK_ESCAPE) {
                            if (state->isOpen()) {
                                state->close();
                                if (open_select_element.get() == focused.get()) {
                                    open_select_element.reset();
                                }
                                markNeedsRepaint();
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
            if (key_code == SDLK_RETURN && focus_manager && dom_manager) {
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
                            state->deleteBackward();
                            handled = true;
                            input_type = "deleteContentBackward";
                        } else if (key_code == SDLK_DELETE) {
                            state->deleteForward();
                            handled = true;
                            input_type = "deleteContentForward";
                        } else if (key_code == SDLK_LEFT) {
                            state->moveCursor(-1);
                            handled = true;
                        } else if (key_code == SDLK_RIGHT) {
                            state->moveCursor(1);
                            handled = true;
                        }

                        if (handled) {
                            focused->setAttribute("value", state->getValue());
                            markNeedsRepaint();

                            // Dispatch input event for delete operations
                            if (input_type && js_bindings && script_engine) {
                                ensureJSBindingsInitialized();
                                uint64_t nid = js_bindings->getNodeIdFor(focused);
                                if (nid) {
                                    js_bindings->dispatchInputEvent(nid, input_type, nullptr);
                                }
                            }
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

                state->insertText(text, focused);
                focused->setAttribute("value", state->getValue());
                markNeedsRepaint();

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
    }

    void sendTextEditing(const char* text, int32_t cursor, int32_t selection_length) {
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

private:
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

    void beginComposition(const dong::dom::DOMNodePtr& focused,
                          dong::dom::InputElementState* state,
                          const char* text) {
        state->startComposition(text);
        focused->setAttribute("value", state->getValue());
        markNeedsRepaint();

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
        markNeedsRepaint();

        dispatchCompositionJS(focused, "compositionupdate", text);
        dispatchInputEventForNode(focused, "insertCompositionText", text);
    }

    void finishComposition(const dong::dom::DOMNodePtr& focused,
                           dong::dom::InputElementState* state) {
        std::string final_data = state->getCompositionText();
        state->endComposition();
        focused->setAttribute("value", state->getValue());
        markNeedsRepaint();

        dispatchCompositionJS(focused, "compositionend", final_data.c_str());
    }

public:

    bool evalScript(const char* code) {
        if (!code || !script_engine) {
            return false;
        }
        ensureJSBindingsInitialized();
        return script_engine->eval(std::string(code));
    }
};

EngineView::EngineView(uint32_t width, uint32_t height)
    : impl_(std::make_unique<Impl>(width, height)) {}

EngineView::~EngineView() = default;

bool EngineView::isInitialized() const {
    return impl_ ? impl_->ctx.isInitialized() : false;
}

void EngineView::setResourceRoot(const char* root) {
    if (!impl_) return;
    impl_->resource_root = root ? root : "";

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
        const auto& cursor = current->getComputedStyle().cursor;
        if (!cursor.empty() && cursor != "auto") {
            impl_->cached_cursor = cursor;
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

void EngineView::sendTextEditing(const char* text, int32_t cursor, int32_t selection_length) {
    if (impl_) impl_->sendTextEditing(text, cursor, selection_length);
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

} // namespace dong
