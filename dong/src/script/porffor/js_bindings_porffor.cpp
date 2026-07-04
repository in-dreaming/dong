#include "js_bindings_porffor.hpp"

#include "dong_porf_host.hpp"
#include "../porffor/script_engine_porffor.hpp"
#include "../porffor/porffor_script_registry.hpp"
#include "../../core/log.h"

#include <algorithm>
#include <cctype>

namespace dong::script {
namespace {

thread_local JSBindings* g_active_bindings = nullptr;

} // namespace

JSBindings::JSBindings(ScriptEngine* engine,
                     dom::Manager* dom_manager,
                     dong::layout::Engine* layout_engine,
                     dom::EventDispatcher* event_dispatcher,
                     dom::FocusManager* focus_manager)
    : engine_(engine),
      dom_manager_(dom_manager),
      layout_engine_(layout_engine),
      event_dispatcher_(event_dispatcher),
      focus_manager_(focus_manager) {
    script_start_time_ = std::chrono::steady_clock::now();
}

JSBindings::~JSBindings() = default;

void JSBindings::setActiveBindings(JSBindings* bindings) {
    g_active_bindings = bindings;
}

void JSBindings::initialize() {
    DONG_LOG_INFO("[JSBindings/Porffor] initialize");
}

void JSBindings::scanAndRegisterInlineEventHandlers() {
    DONG_LOG_WARN("[JSBindings/Porffor] inline event handlers not supported (AOT only)");
}

void JSBindings::resetForNewDOM() {
    node_by_id_.clear();
    named_listeners_.clear();
    event_bridge_ids_.clear();
    next_node_id_ = 1;
}

void JSBindings::registerAsNamedView() {
    // Multi-view registry not implemented for Porffor MVP.
}

uint64_t JSBindings::getNodeIdFor(const dom::DOMNodePtr& node) {
    if (!node) {
        return 0;
    }
    for (const auto& pair : node_by_id_) {
        if (pair.second == node) {
            return pair.first;
        }
    }
    const uint64_t id = next_node_id_++;
    node_by_id_[id] = node;
    return id;
}

bool JSBindings::hasEventListeners(uint64_t node_id, const char* type_name) {
    if (!type_name) {
        return false;
    }
    const std::string type = type_name;
    for (const auto& l : named_listeners_) {
        if (l.node_id == node_id && l.type == type) {
            return true;
        }
    }
    return false;
}

void JSBindings::dispatchMediaEvent(uint64_t node_id, const char* type_name, double /*current_pts*/,
                                    double /*duration*/, const char* /*message*/) {
    if (!type_name) {
        return;
    }
    dispatchPorfforEvent(node_id, type_name);
}

void JSBindings::tickTimers(double /*current_time_sec*/) {
    if (!engine_ || !engine_->host()) {
        return;
    }
    const double now = engine_->host()->timeNow();
    engine_->host()->processTimers(now);
}

void JSBindings::tickAnimationFrames(double /*timestamp_ms*/) {
    // No rAF queue in Porffor MVP.
}

void JSBindings::registerExportHandler(uint64_t node_id, const std::string& type,
                                       const std::string& export_name) {
    NamedListener l;
    l.node_id = node_id;
    l.type = type;
    l.export_name = export_name;
    named_listeners_.push_back(std::move(l));
    DONG_LOG_INFO("[JSBindings/Porffor] listener node=%llu type=%s export=%s",
                  static_cast<unsigned long long>(node_id), type.c_str(), export_name.c_str());

    const auto node = findNodeById(node_id);
    if (node) {
        ensureEventBridgeForNode(node, type, node_id);
    }
}

void JSBindings::ensureEventBridgeForNode(const dom::DOMNodePtr& node, const std::string& type,
                                          uint64_t node_id) {
    if (!event_dispatcher_ || !node || node_id == 0) {
        return;
    }

    void* key = node.get();
    auto& per_node = event_bridge_ids_[key];
    if (per_node.find(type) != per_node.end()) {
        return;
    }

    dom::EventListener callback = [this, node_id, type](const dom::Event&) {
        dispatchPorfforEvent(node_id, type);
    };

    const uint64_t bridge_id = event_dispatcher_->addEventListener(node, type, callback);
    per_node[type] = bridge_id;
}

void JSBindings::dispatchSimpleEvent(uint64_t node_id, const char* type_name) {
    if (!type_name) {
        return;
    }
    dispatchPorfforEvent(node_id, type_name);
}

void JSBindings::dispatchCompositionEvent(uint64_t /*node_id*/, const char* /*type*/,
                                          const char* /*data*/) {}

void JSBindings::dispatchInputEvent(uint64_t /*node_id*/, const char* /*input_type*/,
                                    const char* /*data*/) {}

bool JSBindings::dispatchBeforeInputEvent(uint64_t /*node_id*/, const char* /*input_type*/,
                                          const char* /*data*/) {
    return false;
}

void JSBindings::dispatchClipboardEvent(uint64_t /*node_id*/, const char* /*type*/,
                                        const char* /*data*/) {}

void JSBindings::dispatchPorfforEvent(uint64_t node_id, const std::string& type) {
    if (!engine_ || !engine_->registry()) {
        return;
    }
  std::string norm = type;
  std::transform(norm.begin(), norm.end(), norm.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (const auto& l : named_listeners_) {
        if (l.node_id == node_id && l.type == norm) {
            engine_->registry()->callExport(engine_->registry()->activeModule(), l.export_name);
        }
    }
}

void JSBindings::setNodeTextContent(uint64_t node_id, const std::string& text) {
    const auto node = findNodeById(node_id);
    if (node) {
        node->setTextContent(text);
    }
}

std::string JSBindings::getNodeTextContent(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    return node ? node->getTextContent() : std::string{};
}

dom::DOMNodePtr JSBindings::findNodeById(uint64_t node_id) const {
    const auto it = node_by_id_.find(node_id);
    if (it != node_by_id_.end()) {
        return it->second;
    }
    return nullptr;
}

void resetFetchState(void* /*ctx*/) {}

} // namespace dong::script
