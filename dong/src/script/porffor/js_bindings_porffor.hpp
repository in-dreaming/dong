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
#include "../../dom/html/html_parser.hpp"

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
                               const std::string& export_name,
                               const std::string& module_name = {});
    void dispatchPorfforEvent(uint64_t node_id, const std::string& type,
                              const dom::Event* dom_event = nullptr);
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

    uint64_t createElement(const std::string& tag);
    uint64_t createTextNode(const std::string& text);
    void appendChild(uint64_t parent_id, uint64_t child_id);
    void insertBefore(uint64_t parent_id, uint64_t new_id, uint64_t ref_id);
    void removeNode(uint64_t node_id);
    void replaceChild(uint64_t parent_id, uint64_t new_id, uint64_t old_id);
    uint64_t getParentId(uint64_t node_id) const;
    uint64_t getFirstChildId(uint64_t node_id) const;
    uint64_t getNextSiblingId(uint64_t node_id) const;
    uint64_t cloneNodeId(uint64_t node_id, bool deep) const;
    size_t liveNodeCount() const { return node_by_id_.size(); }

    uint64_t parseHtmlDetached(const std::string& html);
    std::string formSerializeJson(uint64_t form_id) const;
    std::string getSelectionText() const;
    bool selectAllEditable();
    bool execCommand(const std::string& command, const std::string& value = "");
    bool queryCommandSupported(const std::string& command) const;

    std::string clipboardRead() const;
    void clipboardWrite(const std::string& text);
    bool matchMedia(const std::string& query) const;
    bool cssSupports(const std::string& property, const std::string& value) const;

    void dialogShow(uint64_t node_id);
    void dialogShowModal(uint64_t node_id);
    void dialogClose(uint64_t node_id, const std::string& return_value);
    std::string dialogReturnValue(uint64_t node_id) const;
    bool dialogOpen(uint64_t node_id) const;

    uint32_t sceneAddNode(const std::string& config_json);
    void sceneRemove(uint32_t id);
    void sceneSet(uint32_t id, const std::string& prop, const std::string& value);
    int32_t sceneFind(const std::string& name);
    void sceneOn(uint32_t id, const std::string& type, const std::string& export_name,
                 const std::string& module_name);
    void sceneClear();
    uint32_t sceneCount() const;

    std::string textLayout(const std::string& config_json);
    void clearOverlay();
    void renderText(const std::string& config_json);
    void drawRect(const std::string& config_json);
    void drawCircle(const std::string& config_json);

private:
    void registerNodeTree(const dom::DOMNodePtr& node);
    void ensureLayoutFresh() const;
    std::string view_name_;
    std::unordered_map<uint64_t, dom::DOMNodePtr> node_by_id_;
    std::unordered_map<void*, uint64_t> id_by_node_ptr_;
    uint64_t next_node_id_ = 1;

    struct NamedListener {
        uint64_t node_id = 0;
        std::string type;
        std::string export_name;
        std::string module_name;
    };
    std::vector<NamedListener> named_listeners_;
    std::unordered_map<void*, std::unordered_map<std::string, uint64_t>> event_bridge_ids_;

    void ensureEventBridgeForNode(const dom::DOMNodePtr& node, const std::string& type,
                                  uint64_t node_id);

    dom::DOMNodePtr findNodeById(uint64_t node_id) const;
    dom::DOMNodePtr resolveNode(uint64_t node_id, const char* op) const;
    void registerNodeId(uint64_t id, const dom::DOMNodePtr& node);
    void releaseNodeId(uint64_t node_id);
    void markNodeTreeDirty(const dom::DOMNodePtr& node);
};

void resetFetchState(void* /*ctx*/);

} // namespace dong::script
