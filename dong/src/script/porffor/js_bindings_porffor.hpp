#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../dom/dom_manager.hpp"
#include "../../dom/event_system.hpp"
#include "../../dom/dom/dom_node.hpp"

namespace dong::dom {
class Selection;
}

namespace dong::layout {
class Engine;
}

namespace dong::render {
class OverlayDraw;
}

namespace dong::script {

class ScriptEngine;
class PorfforScriptRegistry;

// Minimal bindings for Porffor host (no QuickJS).
class JSBindings {
public:
    JSBindings(ScriptEngine* engine,
               dom::Manager* dom_manager,
               dong::layout::Engine* layout_engine,
               dom::EventDispatcher* event_dispatcher,
               dom::FocusManager* focus_manager = nullptr);
    ~JSBindings();

    void initialize();
    void scanAndRegisterInlineEventHandlers();
    void resetForNewDOM();
    void registerAsNamedView();

    void setViewName(const std::string& name) { view_name_ = name; }
    const std::string& getViewName() const { return view_name_; }

    static void setActiveBindings(JSBindings* bindings);

    uint64_t getNodeIdFor(const dom::DOMNodePtr& node);
    bool hasEventListeners(uint64_t node_id, const char* type_name);

    void dispatchMediaEvent(uint64_t node_id, const char* type_name, double current_pts,
                            double duration, const char* message);

    void tickTimers(double current_time_sec);
    void tickAnimationFrames(double timestamp_ms);

    ScriptEngine* engine_;
    dom::Manager* dom_manager_;
    dong::layout::Engine* layout_engine_;
    dom::EventDispatcher* event_dispatcher_;
    dom::FocusManager* focus_manager_;
    dom::Selection* selection_ = nullptr;
    dom::DOMNodePtr last_editable_root_;
    dong::render::OverlayDraw* overlay_draw_ = nullptr;

    std::chrono::steady_clock::time_point script_start_time_;

    void registerExportHandler(uint64_t node_id, const std::string& type,
                               const std::string& export_name);
    void dispatchPorfforEvent(uint64_t node_id, const std::string& type);
    void dispatchSimpleEvent(uint64_t node_id, const char* type_name);

    void dispatchCompositionEvent(uint64_t node_id, const char* type, const char* data);
    void dispatchInputEvent(uint64_t node_id, const char* input_type, const char* data = nullptr);
    bool dispatchBeforeInputEvent(uint64_t node_id, const char* input_type, const char* data = nullptr);
    void dispatchClipboardEvent(uint64_t node_id, const char* type, const char* data);

    void setNodeTextContent(uint64_t node_id, const std::string& text);
    std::string getNodeTextContent(uint64_t node_id) const;

    std::string getNodeValue(uint64_t node_id) const;
    void setNodeValue(uint64_t node_id, const std::string& value);
    bool getNodeChecked(uint64_t node_id) const;
    void setNodeChecked(uint64_t node_id, bool checked);
    bool getNodeDisabled(uint64_t node_id) const;
    void setNodeDisabled(uint64_t node_id, bool disabled);
    std::string getNodeAttribute(uint64_t node_id, const std::string& name) const;
    void setNodeAttribute(uint64_t node_id, const std::string& name, const std::string& value);
    void removeNodeAttribute(uint64_t node_id, const std::string& name);
    void setNodeInnerHTML(uint64_t node_id, const std::string& html);

    uint64_t querySelector(uint64_t root_id, const std::string& selector) const;
    std::string querySelectorAllJson(uint64_t root_id, const std::string& selector) const;
    std::string getElementsByTagNameJson(uint64_t root_id, const std::string& tag) const;

    void classAdd(uint64_t node_id, const std::string& cls);
    void classRemove(uint64_t node_id, const std::string& cls);
    bool classToggle(uint64_t node_id, const std::string& cls);
    bool classContains(uint64_t node_id, const std::string& cls) const;

    void styleSet(uint64_t node_id, const std::string& prop, const std::string& value);
    std::string styleGet(uint64_t node_id, const std::string& prop) const;
    std::string computedStyleGet(uint64_t node_id, const std::string& prop) const;

    std::string getRectJson(uint64_t node_id) const;
    double getMetric(uint64_t node_id, int metric_id) const;
    double getScrollTop(uint64_t node_id) const;
    void setScrollTop(uint64_t node_id, double value);
    double getScrollLeft(uint64_t node_id) const;
    void setScrollLeft(uint64_t node_id, double value);

    void focusNode(uint64_t node_id);
    void blurNode(uint64_t node_id);
    void clickNode(uint64_t node_id);
    bool matchesSelector(uint64_t node_id, const std::string& selector) const;
    uint64_t closestSelector(uint64_t node_id, const std::string& selector) const;

private:
    void ensureLayoutFresh() const;
    std::string view_name_;
    std::unordered_map<uint64_t, dom::DOMNodePtr> node_by_id_;
    uint64_t next_node_id_ = 1;

    struct NamedListener {
        uint64_t node_id = 0;
        std::string type;
        std::string export_name;
    };
    std::vector<NamedListener> named_listeners_;
    std::unordered_map<void*, std::unordered_map<std::string, uint64_t>> event_bridge_ids_;

    void ensureEventBridgeForNode(const dom::DOMNodePtr& node, const std::string& type,
                                  uint64_t node_id);

    dom::DOMNodePtr findNodeById(uint64_t node_id) const;
};

void resetFetchState(void* /*ctx*/);

} // namespace dong::script
