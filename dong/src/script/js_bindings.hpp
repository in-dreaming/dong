#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include "script_engine.hpp"
#include "../dom/dom_manager.hpp"
#include "../dom/event_system.hpp"
#include "../dom/dom/dom_node.hpp"

// Forward declarations
namespace dong::dom {
class FocusManager;
class Selection;
}

namespace dong::layout {
class Engine;
}


#include "quickjs_compat.h"


namespace dong::script {

// JavaScript 绑定层 - 提供 JavaScript 访问 DOM 和事件系统的接口
class JSBindings {
public:
    JSBindings(ScriptEngine* engine,
               dom::Manager* dom_manager,
               dong::layout::Engine* layout_engine,
               dom::EventDispatcher* event_dispatcher,
               dom::FocusManager* focus_manager = nullptr);
    ~JSBindings();

    // 初始化所有内置 API
    void initialize();

    // 扫描 DOM 树并注册内联事件处理器（如 onclick, onchange 等）
    void scanAndRegisterInlineEventHandlers();

    // 注册 DOM 节点到 JS 环境
    JSValue createJSElement(JSContext* ctx, const dom::DOMNodePtr& node);

    // 获取 ScriptEngine
    ScriptEngine* getEngine() const { return engine_; }

    // Set the view name for multi-view JS access (dong.getView(name))
    void setViewName(const std::string& name) { view_name_ = name; }
    const std::string& getViewName() const { return view_name_; }

    // Register this view's window/document on the dong.views JS registry.
    // Call after initialize() and loadHTML() for secondary views in shared-JS mode.
    void registerAsNamedView();

    // Set the thread-local active JSBindings (for shared-JS mode).
    // Must be called before any view operation (tick, input, etc.).
    static void setActiveBindings(JSBindings* bindings);

    // Public access to managers for callbacks
    ScriptEngine* engine_;
    dom::Manager* dom_manager_;
    dong::layout::Engine* layout_engine_;
    dom::EventDispatcher* event_dispatcher_;
    dom::FocusManager* focus_manager_;
    dom::Selection* selection_ = nullptr;
    dom::DOMNodePtr last_editable_root_;  // Last focused contenteditable root (for execCommand after focus loss)


    // Start time for performance.now()
    std::chrono::steady_clock::time_point script_start_time_;

private:
    
    // 映射：JS 对象 ID -> DOM 节点，保证生命周期
    std::unordered_map<uint64_t, dom::DOMNodePtr> id_to_node_;
    // 反向映射：DOM 节点指针 -> JS 对象 ID，便于从原生事件反查
    std::unordered_map<void*, uint64_t> node_to_id_;
    uint64_t next_js_id_ = 1;

    // JS 事件监听器注册表：node_id -> (event_type -> JS 函数列表)
    std::unordered_map<uint64_t, std::unordered_map<std::string, std::vector<JSValue>>> listeners_;

    // 初始化各个 API 模块
    void initializeDocumentAPI();
    void initializeElementAPI();
    void initializeEventAPI();
    void initializeConsoleAPI();
    void initializePerformanceAPI();

public:
    // Helper methods (public for use by callback functions)
    static dom::DOMNodePtr getNodeOpaque(JSContext* ctx, JSValue val);
    uint64_t getNodeIdFor(const dom::DOMNodePtr& node) const;

    // Event bridge helpers
    void registerEventListener(uint64_t node_id, const std::string& type, JSValueConst handler);
    void removeEventListener(uint64_t node_id, const std::string& type, JSValueConst handler);
    void dispatchMouseEvent(uint64_t node_id, const char* type, int32_t x, int32_t y, int32_t button,
                            bool is_trusted, uint64_t related_node_id = 0);
    void dispatchKeyEvent(uint64_t node_id, const char* type, uint32_t key_code,
                          bool is_trusted, bool repeat = false,
                          bool alt_key = false, bool ctrl_key = false, bool shift_key = false, bool meta_key = false);



    // Generic event dispatch for non-input events (e.g. media events).
    void dispatchSimpleEvent(uint64_t node_id, const char* type);
    void dispatchMediaEvent(uint64_t node_id, const char* type, double current_time, double duration, const char* message = nullptr);

    // BeforeInput event dispatch (cancelable; may prevent default)
    bool dispatchBeforeInputEvent(uint64_t node_id, const char* input_type, const char* data = nullptr);

    // Input event dispatch with inputType and data properties
    void dispatchInputEvent(uint64_t node_id, const char* input_type, const char* data = nullptr);


    // Composition event dispatch (compositionstart/update/end) with data property
    void dispatchCompositionEvent(uint64_t node_id, const char* type, const char* data);

    // Clipboard event dispatch (copy/cut/paste) with clipboardData property
    void dispatchClipboardEvent(uint64_t node_id, const char* type, const char* data = nullptr);

    // Focus event dispatch with relatedTarget property
    void dispatchFocusEvent(uint64_t node_id, const char* type, uint64_t related_node_id);

    // Drag & Drop event dispatch with data transfer support
    void dispatchDragDropEvent(uint64_t node_id, const char* type, int32_t x, int32_t y, const std::string& data_transfer, bool is_trusted);

    bool hasEventListeners(uint64_t node_id, const char* type) const;

    // Bridge between JS listeners and C++ DOM EventDispatcher
    void ensureEventBridgeForNode(const dom::DOMNodePtr& node, const std::string& type, uint64_t node_id);

    // Dispatch a JS-provided event object (used by Element.dispatchEvent)
    // Returns true if the event was NOT canceled (i.e. !defaultPrevented).
    bool dispatchEventObject(uint64_t node_id, JSValueConst event_obj);


    // DOM lifecycle
    void resetForNewDOM();

    // Timer API (setTimeout / clearTimeout / setInterval / clearInterval)
    // Called each tick with current wall-clock time in seconds.
    void tickTimers(double current_time_sec);

    struct TimerEntry {
        uint32_t id = 0;
        double fire_at = 0.0;  // absolute time in seconds
        double interval = -1.0; // -1 = one-shot, else repeat period
        JSValue callback = JS_UNDEFINED;
        bool cancelled = false;
    };

    // Timer state (public so JS binding functions can access)
    std::map<uint32_t, TimerEntry> timers_;
    uint32_t next_timer_id_ = 1;

    // requestAnimationFrame / cancelAnimationFrame
    struct RAFEntry {
        uint32_t id = 0;
        JSValue callback = JS_UNDEFINED;
    };
    std::vector<RAFEntry> raf_callbacks_;
    uint32_t next_raf_id_ = 1;
    void tickAnimationFrames(double timestamp_ms);

    // Document.write support: current executing <script>
    void setCurrentExecutingScript(const dom::DOMNodePtr& script_node) { current_executing_script_ = script_node; }
    void clearCurrentExecutingScript() { current_executing_script_.reset(); }
    dom::DOMNodePtr getCurrentExecutingScript() const { return current_executing_script_.lock(); }
    
private:
    void dispatchEventToChain(const dom::DOMNodePtr& target,
                              const std::string& type,
                              JSValue event);

    bool dispatchCancelableEventToChain(const dom::DOMNodePtr& target,
                                       const std::string& type,
                                       JSValue event);

    // Track C++ bridge listener IDs to avoid duplicate registration
    std::unordered_map<void*, std::unordered_map<std::string, uint64_t>> event_bridge_ids_;

    // Current executing script element for document.write insertion point
    dom::DOMNodeWeakPtr current_executing_script_;

    // For MouseEvent.movementX/Y
    bool has_last_mouse_pos_ = false;
    int32_t last_mouse_x_ = 0;
    int32_t last_mouse_y_ = 0;

    // View name for multi-view registry (dong.getView(name))
    std::string view_name_;

    // Static helper for storing node pointers
    static void setNodeOpaque(JSContext* ctx, JSValue val, dom::DOMNodePtr node);
};

using JSBindingsPtr = std::unique_ptr<JSBindings>;

} // namespace dong::script
