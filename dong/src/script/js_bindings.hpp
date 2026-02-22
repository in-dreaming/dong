#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include "script_engine.hpp"
#include "../dom/dom_manager.hpp"
#include "../dom/event_system.hpp"

// Forward declarations
namespace dong::dom {
class FocusManager;
}

extern "C" {
#include "quickjs.h"
}

namespace dong::script {

// JavaScript 绑定层 - 提供 JavaScript 访问 DOM 和事件系统的接口
class JSBindings {
public:
    JSBindings(ScriptEngine* engine, dom::Manager* dom_manager, dom::EventDispatcher* event_dispatcher, dom::FocusManager* focus_manager = nullptr);
    ~JSBindings();

    // 初始化所有内置 API
    void initialize();

    // 扫描 DOM 树并注册内联事件处理器（如 onclick, onchange 等）
    void scanAndRegisterInlineEventHandlers();

    // 注册 DOM 节点到 JS 环境
    JSValue createJSElement(JSContext* ctx, const dom::DOMNodePtr& node);

    // 获取 ScriptEngine
    ScriptEngine* getEngine() const { return engine_; }

    // Public access to managers for callbacks
    ScriptEngine* engine_;
    dom::Manager* dom_manager_;
    dom::EventDispatcher* event_dispatcher_;
    dom::FocusManager* focus_manager_;

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
    void dispatchMouseEvent(uint64_t node_id, const char* type, int32_t x, int32_t y, int32_t button);
    void dispatchKeyEvent(uint64_t node_id, const char* type, uint32_t key_code);

    // Generic event dispatch for non-input events (e.g. media events).
    void dispatchSimpleEvent(uint64_t node_id, const char* type);
    void dispatchMediaEvent(uint64_t node_id, const char* type, double current_time, double duration, const char* message = nullptr);

    // Input event dispatch with inputType and data properties
    void dispatchInputEvent(uint64_t node_id, const char* input_type, const char* data = nullptr);

    // Focus event dispatch with relatedTarget property
    void dispatchFocusEvent(uint64_t node_id, const char* type, uint64_t related_node_id);

    bool hasEventListeners(uint64_t node_id, const char* type) const;

    // Bridge between JS listeners and C++ DOM EventDispatcher
    void ensureEventBridgeForNode(const dom::DOMNodePtr& node, const std::string& type, uint64_t node_id);

    // DOM lifecycle
    void resetForNewDOM();
    
private:
    // Track C++ bridge listener IDs to avoid duplicate registration
    std::unordered_map<void*, std::unordered_map<std::string, uint64_t>> event_bridge_ids_;

    // Static helper for storing node pointers
    static void setNodeOpaque(JSContext* ctx, JSValue val, dom::DOMNodePtr node);
};

using JSBindingsPtr = std::unique_ptr<JSBindings>;

} // namespace dong::script
