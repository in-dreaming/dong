#pragma once

#include <memory>
#include <unordered_map>
#include "script_engine.hpp"
#include "../dom/dom_manager.hpp"
#include "../dom/event_system.hpp"

// Forward declarations
extern "C" {
    struct JSContext;
    struct JSRuntime;
}

// JSValue is a scalar type in QuickJS (int64_t), not a struct
using JSValue = int64_t;

namespace dong::script {

// JavaScript 绑定层 - 提供 JavaScript 访问 DOM 和事件系统的接口
class JSBindings {
public:
    JSBindings(ScriptEngine* engine, dom::Manager* dom_manager, dom::EventDispatcher* event_dispatcher);
    ~JSBindings();

    // 初始化所有内置 API
    void initialize();

    // 注册 DOM 节点到 JS 环境
    JSValue createJSElement(JSContext* ctx, const dom::DOMNodePtr& node);

    // 获取 ScriptEngine
    ScriptEngine* getEngine() const { return engine_; }

    // Public access to managers for callbacks
    ScriptEngine* engine_;
    dom::Manager* dom_manager_;
    dom::EventDispatcher* event_dispatcher_;

private:
    
    // 映射：DOMNode 指针 -> JS 对象 ID
    std::unordered_map<uintptr_t, uint64_t> node_to_js_id_;
    uint64_t next_js_id_ = 1;

    // 初始化各个 API 模块
    void initializeDocumentAPI();
    void initializeElementAPI();
    void initializeEventAPI();
    void initializeConsoleAPI();

public:
    // Helper methods (public for use by callback functions)
    static dom::DOMNodePtr getNodeOpaque(JSContext* ctx, JSValue val);
    
private:
    // Static helper for storing node pointers
    static void setNodeOpaque(JSContext* ctx, JSValue val, dom::DOMNodePtr node);
};

using JSBindingsPtr = std::unique_ptr<JSBindings>;

} // namespace dong::script
