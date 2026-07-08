#include "js_bindings_porffor.hpp"

#include "dong_porf_host.hpp"
#include "js_scene_porffor.hpp"
#include "js_text_layout_porffor.hpp"
#include "../porffor/script_engine_porffor.hpp"
#include "../porffor/porffor_script_registry.hpp"
#include "../../core/log.h"

#include "../../dom/input_element.hpp"
#include "../../dom/select_element.hpp"
#include "../../dom/dialog_element.hpp"
#include "../../dom/contenteditable.hpp"
#include "../../dom/editing_commands.hpp"
#include "../../dom/focus_manager.hpp"
#include "../../dom/css/style_engine.hpp"
#include "../../dom/selection.hpp"
#include "../../layout/layout_engine.hpp"
#include "dong_platform.h"
#include "dong_clipboard.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace dong::script {
namespace {

thread_local JSBindings* g_active_bindings = nullptr;

std::string camelToCss(const std::string& property) {
    std::string css;
    css.reserve(property.size() * 2);
    for (char c : property) {
        if (std::isupper(static_cast<unsigned char>(c))) {
            css.push_back('-');
            css.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else {
            css.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }
    return css;
}

std::string getComputedStyleValue(const dom::ComputedStyle& style, const std::string& css_prop) {
    if (css_prop == "display") return dong::dom::toString(style.display);
    if (css_prop == "color") return style.color;
    if (css_prop == "background-color") return style.background_color;
    if (css_prop == "font-size") return std::to_string(style.font_size);
    if (css_prop == "font-weight") return dong::dom::toString(style.font_weight);
    if (css_prop == "text-align") return dong::dom::toString(style.text_align);
    if (css_prop == "position") return dong::dom::toString(style.position);
    if (css_prop == "opacity") return std::to_string(style.opacity);
    if (css_prop == "border-radius") return std::to_string(style.border_radius);
    if (css_prop == "border-width") return std::to_string(style.border_width);
    if (css_prop == "border-color") return style.border_color;
    return "";
}

std::string formatCssLengthPx(float value) {
    if (std::fabs(value - std::round(value)) < 0.001f) {
        return std::to_string(static_cast<int>(std::round(value))) + "px";
    }
    std::ostringstream oss;
    oss << value << "px";
    return oss.str();
}

std::string getStyleValueForNode(const dom::DOMNodePtr& node, const std::string& css_prop) {
    if (!node) return "";
    std::string inline_value = node->getInlineStyleProperty(css_prop);
    if (!inline_value.empty()) {
        return inline_value;
    }
    const auto& style = node->getComputedStyle();
    if (css_prop == "padding") {
        return formatCssLengthPx(style.padding_top.resolvePixels(0.0f, 16.0f, 800.0f, 600.0f));
    }
    if (css_prop == "border-width") {
        const float width = style.border_top_width >= 0.0f ? style.border_top_width : style.border_width;
        return formatCssLengthPx(width);
    }
    return getComputedStyleValue(style, css_prop);
}

std::string nodeIdsToJson(const std::vector<uint64_t>& ids) {
  std::string s = "[";
  for (size_t i = 0; i < ids.size(); ++i) {
    if (i > 0) {
      s += ',';
    }
    s += std::to_string(ids[i]);
  }
  s += ']';
  return s;
}

void collectFormControls(const dom::DOMNodePtr& node, std::vector<dom::DOMNodePtr>& out) {
  if (!node) {
    return;
  }
  for (const auto& ch : node->getChildren()) {
    if (!ch) {
      continue;
    }
    if (ch->getType() == dom::DOMNode::NodeType::ELEMENT) {
      const std::string& tag = ch->getTagName();
      if (tag == "input" || tag == "textarea" || tag == "select" || tag == "button") {
        out.push_back(ch);
      }
    }
    collectFormControls(ch, out);
  }
}

bool serializeFormControl(const dom::DOMNodePtr& control,
                          std::vector<std::pair<std::string, std::string>>& entries) {
  if (!control || control->hasAttribute("disabled") || !control->hasAttribute("name")) {
    return false;
  }

  const std::string& name = control->getAttribute("name");
  const std::string& tag = control->getTagName();

  if (tag == "input") {
    std::string input_type = control->hasAttribute("type") ? control->getAttribute("type") : "text";
    if (input_type == "checkbox" || input_type == "radio") {
      if (!control->hasAttribute("checked")) {
        return false;
      }
    }
    std::string value;
    if (input_type == "checkbox" || input_type == "radio") {
      value = control->hasAttribute("value") ? control->getAttribute("value") : "on";
    } else if (auto* st = dom::getInputState(control)) {
      value = st->getValue();
    } else if (control->hasAttribute("value")) {
      value = control->getAttribute("value");
    }
    entries.push_back({name, value});
    return true;
  }

  if (tag == "textarea") {
    std::string value;
    if (auto* st = dom::getInputState(control)) {
      value = st->getValue();
    } else {
      value = control->getTextContent();
    }
    entries.push_back({name, value});
    return true;
  }

  if (tag == "select") {
    if (auto* st = dom::getSelectState(control)) {
      entries.push_back({name, st->getSelectedValue()});
      return true;
    }
    if (control->hasAttribute("value")) {
      entries.push_back({name, control->getAttribute("value")});
      return true;
    }
  }

  return false;
}

std::string jsonEscape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out.push_back(c); break;
    }
  }
  return out;
}

std::string formEntriesToJson(const std::vector<std::pair<std::string, std::string>>& entries) {
  std::string out = "[";
  for (size_t i = 0; i < entries.size(); ++i) {
    if (i > 0) {
      out += ',';
    }
    out += "{\"name\":\"" + jsonEscape(entries[i].first) + "\",\"value\":\"" +
           jsonEscape(entries[i].second) + "\"}";
  }
  out += ']';
  return out;
}

void collectElementsByTagName(const dom::DOMNodePtr& node, const std::string& tag,
                              std::vector<dom::DOMNodePtr>& out) {
    if (!node) {
        return;
    }
    if (node->getTagName() == tag) {
        out.push_back(node);
    }
    for (const auto& child : node->getChildren()) {
        collectElementsByTagName(child, tag, out);
    }
}

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
}

void JSBindings::resetForNewDOM() {
    node_by_id_.clear();
    id_by_node_ptr_.clear();
    named_listeners_.clear();
    event_bridge_ids_.clear();
    next_node_id_ = 1;
    if (engine_ && engine_->host()) {
        engine_->host()->resetFetches();
    }
}

void JSBindings::registerAsNamedView() {
    // Multi-view registry not implemented for Porffor MVP.
}

void JSBindings::registerNodeId(uint64_t id, const dom::DOMNodePtr& node) {
    if (!node || id == 0) {
        return;
    }
    node_by_id_[id] = node;
    id_by_node_ptr_[node.get()] = id;
}

void JSBindings::releaseNodeId(uint64_t node_id) {
    const auto it = node_by_id_.find(node_id);
    if (it == node_by_id_.end()) {
        return;
    }
    if (it->second) {
        id_by_node_ptr_.erase(it->second.get());
    }
    node_by_id_.erase(it);

    named_listeners_.erase(
        std::remove_if(named_listeners_.begin(), named_listeners_.end(),
                       [node_id](const NamedListener& l) { return l.node_id == node_id; }),
        named_listeners_.end());
}

uint64_t JSBindings::getNodeIdFor(const dom::DOMNodePtr& node) {
    if (!node) {
        return 0;
    }
    const auto it = id_by_node_ptr_.find(node.get());
    if (it != id_by_node_ptr_.end()) {
        return it->second;
    }
    const uint64_t id = next_node_id_++;
    registerNodeId(id, node);
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
    engine_->host()->processFetches();
    const double now = engine_->host()->timeNow();
    engine_->host()->processTimers(now);
}

void JSBindings::tickAnimationFrames(double timestamp_ms) {
    if (!engine_ || !engine_->host()) {
        return;
    }
    engine_->host()->processAnimationFrames(timestamp_ms);
}

void JSBindings::registerExportHandler(uint64_t node_id, const std::string& type,
                                       const std::string& export_name,
                                       const std::string& module_name) {
    NamedListener l;
    l.node_id = node_id;
    l.type = type;
    l.export_name = export_name;
    l.module_name = module_name;
    if (l.module_name.empty() && engine_ && engine_->registry()) {
        l.module_name = engine_->registry()->activeModule();
    }
    DONG_LOG_INFO("[JSBindings/Porffor] listener node=%llu type=%s export=%s module=%s",
                  static_cast<unsigned long long>(node_id), type.c_str(), export_name.c_str(),
                  l.module_name.c_str());
    named_listeners_.push_back(std::move(l));

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

    dom::EventListener callback = [this, node_id, type](const dom::Event& ev) {
        dispatchPorfforEvent(node_id, type, &ev);
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

void JSBindings::dispatchPorfforEvent(uint64_t node_id, const std::string& type,
                                        const dom::Event* dom_event) {
    if (!engine_ || !engine_->registry()) {
        return;
    }
    std::string norm = type;
    std::transform(norm.begin(), norm.end(), norm.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    PorfforHost* host = engine_->host();
    if (host) {
        host->pushEventSlot(dom_event, node_id, norm);
    }

    for (const auto& l : named_listeners_) {
        if (l.node_id != node_id || l.type != norm) {
            continue;
        }
        const std::string& mod =
            l.module_name.empty() ? engine_->registry()->activeModule() : l.module_name;
        engine_->registry()->callExport(mod, l.export_name);
        if (host && host->eventStopPropagation()) {
            break;
        }
    }

    if (host) {
        host->popEventSlot();
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

dom::DOMNodePtr JSBindings::resolveNode(uint64_t node_id, const char* op) const {
    if (node_id == 0) {
        DONG_LOG_DEBUG("[JSBindings/Porffor] %s: invalid nodeId=0 (no-op)", op ? op : "op");
        return nullptr;
    }
    const auto node = findNodeById(node_id);
    if (!node) {
        DONG_LOG_DEBUG("[JSBindings/Porffor] %s: stale nodeId=%llu (no-op)",
                       op ? op : "op", static_cast<unsigned long long>(node_id));
    }
    return node;
}

void JSBindings::markNodeTreeDirty(const dom::DOMNodePtr& node) {
    if (!node) {
        return;
    }
    node->markStyleDirty();
    node->markLayoutDirty();
    if (auto parent = node->getParent()) {
        parent->markStyleDirty();
        parent->markLayoutDirty();
    }
}

void JSBindings::markNodeTreeDirtyRecursive(const dom::DOMNodePtr& node) {
    if (!node) {
        return;
    }
    node->markStyleDirty();
    node->markLayoutDirty();
    for (const auto& child : node->getChildren()) {
        markNodeTreeDirtyRecursive(child);
    }
    if (auto parent = node->getParent()) {
        parent->markStyleDirty();
        parent->markLayoutDirty();
    }
}

void JSBindings::ensureLayoutFresh() const {
    if (!layout_engine_ || !dom_manager_) {
        return;
    }
    auto root = dom_manager_->getRoot();
    const float vw = layout_engine_->getViewportWidth();
    const float vh = layout_engine_->getViewportHeight();
    if (root && vw > 0.0f && vh > 0.0f) {
        layout_engine_->calculateLayout(root, vw, vh);
        root->clearLayoutDirtyRecursive();
    }
}

std::string JSBindings::getNodeValue(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return {};
    }
    if (node->getNodeName() == "select") {
        if (auto* state = dom::getSelectState(node)) {
            return state->getSelectedValue();
        }
    }
    if (auto* state = dom::getInputState(node)) {
        return state->getValue();
    }
    return node->getAttribute("value");
}

void JSBindings::setNodeValue(uint64_t node_id, const std::string& value) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    if (node->getTagName() == "select") {
        if (auto* sel_state = dom::getSelectState(node)) {
            sel_state->syncFromDOM(node);
            const size_t prev = sel_state->getSelectedIndex();
            sel_state->selectByValue(value);
            if (sel_state->getSelectedIndex() != prev) {
                sel_state->applySelectionToDOM(node);
            } else {
                node->setAttribute("value", value);
            }
        } else {
            node->setAttribute("value", value);
        }
        node->markLayoutDirty();
        return;
    }
    if (auto* state = dom::getInputState(node)) {
        state->setValue(value);
    }
    node->setAttribute("value", value);
    node->markLayoutDirty();
}

bool JSBindings::getNodeChecked(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    return node && node->hasAttribute("checked");
}

void JSBindings::setNodeChecked(uint64_t node_id, bool checked) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    if (checked) {
        node->setAttribute("checked", "");
    } else {
        node->removeAttribute("checked");
    }
}

bool JSBindings::getNodeDisabled(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    return node && node->hasAttribute("disabled");
}

void JSBindings::setNodeDisabled(uint64_t node_id, bool disabled) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    if (disabled) {
        node->setAttribute("disabled", "");
    } else {
        node->removeAttribute("disabled");
    }
}

std::string JSBindings::getNodeAttribute(uint64_t node_id, const std::string& name) const {
    const auto node = findNodeById(node_id);
    if (!node || !node->hasAttribute(name)) {
        return {};
    }
    return node->getAttribute(name);
}

void JSBindings::setNodeAttribute(uint64_t node_id, const std::string& name,
                                  const std::string& value) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    node->setAttribute(name, value);
    node->markStyleDirty();
    node->markLayoutDirty();
}

void JSBindings::removeNodeAttribute(uint64_t node_id, const std::string& name) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    node->removeAttribute(name);
    node->markStyleDirty();
    node->markLayoutDirty();
}

void JSBindings::setNodeInnerHTML(uint64_t node_id, const std::string& html) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    std::string clean_html = html;
    if (clean_html.find('\0') != std::string::npos) {
        clean_html.erase(std::remove(clean_html.begin(), clean_html.end(), '\0'), clean_html.end());
    }
    node->setInnerHTML(clean_html);
    registerNodeTree(node);
    if (dom_manager_) {
        std::function<void(const dom::DOMNodePtr&)> recompute_styles;
        recompute_styles = [&](const dom::DOMNodePtr& n) {
            if (!n) {
                return;
            }
            dom_manager_->recomputeNodeStyle(n);
            for (const auto& child : n->getChildren()) {
                recompute_styles(child);
            }
        };
        recompute_styles(node);
    }
    markNodeTreeDirtyRecursive(node);
    ensureLayoutFresh();
}

uint64_t JSBindings::querySelector(uint64_t root_id, const std::string& selector) const {
    dom::DOMNodePtr root = findNodeById(root_id);
    if (!root && dom_manager_) {
        root = dom_manager_->getRoot();
    }
    if (!root) {
        return 0;
    }
    auto found = root->querySelector(selector);
    if (!found) {
        return 0;
    }
    return const_cast<JSBindings*>(this)->getNodeIdFor(found);
}

std::string JSBindings::querySelectorAllJson(uint64_t root_id, const std::string& selector) const {
    dom::DOMNodePtr root = findNodeById(root_id);
    if (!root && dom_manager_) {
        root = dom_manager_->getRoot();
    }
    if (!root) {
        return "[]";
    }
    auto results = root->querySelectorAll(selector);
    std::vector<uint64_t> ids;
    ids.reserve(results.size());
    for (const auto& n : results) {
        ids.push_back(const_cast<JSBindings*>(this)->getNodeIdFor(n));
    }
    return nodeIdsToJson(ids);
}

std::string JSBindings::getElementsByTagNameJson(uint64_t root_id, const std::string& tag) const {
    dom::DOMNodePtr root = findNodeById(root_id);
    if (!root && dom_manager_) {
        root = dom_manager_->getRoot();
    }
    if (!root) {
        return "[]";
    }
    std::vector<dom::DOMNodePtr> nodes;
    collectElementsByTagName(root, tag, nodes);
    std::vector<uint64_t> ids;
    ids.reserve(nodes.size());
    for (const auto& n : nodes) {
        ids.push_back(const_cast<JSBindings*>(this)->getNodeIdFor(n));
    }
    return nodeIdsToJson(ids);
}

void JSBindings::classAdd(uint64_t node_id, const std::string& cls) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    node->getClassList().add(cls);
    if (dom_manager_) {
        dom_manager_->recomputeNodeStyle(node);
    }
    node->markStyleDirty();
    node->markLayoutDirty();
}

void JSBindings::classRemove(uint64_t node_id, const std::string& cls) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    node->getClassList().remove(cls);
    if (dom_manager_) {
        dom_manager_->recomputeNodeStyle(node);
    }
    node->markStyleDirty();
    node->markLayoutDirty();
}

bool JSBindings::classToggle(uint64_t node_id, const std::string& cls) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return false;
    }
    const bool had = node->getClassList().contains(cls);
    node->getClassList().toggle(cls);
    if (dom_manager_) {
        dom_manager_->recomputeNodeStyle(node);
    }
    node->markStyleDirty();
    node->markLayoutDirty();
    return !had;
}

bool JSBindings::classContains(uint64_t node_id, const std::string& cls) const {
    const auto node = findNodeById(node_id);
    return node && node->getClassList().contains(cls);
}

void JSBindings::styleSet(uint64_t node_id, const std::string& prop, const std::string& value) {
    const auto node = findNodeById(node_id);
    if (!node) {
        return;
    }
    std::string css_prop = prop;
    if (prop.find('-') == std::string::npos) {
        css_prop = camelToCss(prop);
    }
    node->setInlineStyleProperty(css_prop, value);
    node->markStyleDirty();
    node->markLayoutDirty();
}

std::string JSBindings::styleGet(uint64_t node_id, const std::string& prop) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return {};
    }
    std::string css_prop = prop;
    if (prop.find('-') == std::string::npos) {
        css_prop = camelToCss(prop);
    }
    return getStyleValueForNode(node, css_prop);
}

std::string JSBindings::computedStyleGet(uint64_t node_id, const std::string& prop) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return {};
    }
    ensureLayoutFresh();
    std::string css_prop = prop;
    if (prop.find('-') == std::string::npos) {
        css_prop = camelToCss(prop);
    }
    return getComputedStyleValue(node->getComputedStyle(), css_prop);
}

std::string JSBindings::getRectJson(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return R"({"x":0,"y":0,"w":0,"h":0})";
    }
    ensureLayoutFresh();
    const auto rect = node->getBoundingClientRect();
    std::ostringstream oss;
    oss << "{\"x\":" << rect.x << ",\"y\":" << rect.y << ",\"w\":" << rect.width
        << ",\"h\":" << rect.height << "}";
    return oss.str();
}

double JSBindings::getMetric(uint64_t node_id, int metric_id) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return 0.0;
    }
    ensureLayoutFresh();
    switch (metric_id) {
    case 0: return static_cast<double>(node->getOffsetWidth());
    case 1: return static_cast<double>(node->getOffsetHeight());
    case 2: return static_cast<double>(node->getOffsetTop());
    case 3: return static_cast<double>(node->getOffsetLeft());
    case 4: return static_cast<double>(node->getClientWidth());
    case 5: return static_cast<double>(node->getClientHeight());
    case 6: return static_cast<double>(node->getScrollWidth());
    case 7: return static_cast<double>(node->getScrollHeight());
    default: return 0.0;
    }
}

double JSBindings::getScrollTop(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    return node ? static_cast<double>(node->getScrollTop()) : 0.0;
}

void JSBindings::setScrollTop(uint64_t node_id, double value) {
    const auto node = findNodeById(node_id);
    if (node) {
        node->setScrollTop(static_cast<float>(value));
    }
}

double JSBindings::getScrollLeft(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    return node ? static_cast<double>(node->getScrollLeft()) : 0.0;
}

void JSBindings::setScrollLeft(uint64_t node_id, double value) {
    const auto node = findNodeById(node_id);
    if (node) {
        node->setScrollLeft(static_cast<float>(value));
    }
}

void JSBindings::focusNode(uint64_t node_id) {
    const auto node = findNodeById(node_id);
    if (node) {
        node->focus();
    }
}

void JSBindings::blurNode(uint64_t node_id) {
    const auto node = findNodeById(node_id);
    if (node) {
        node->blur();
    }
}

void JSBindings::clickNode(uint64_t node_id) {
    const auto node = findNodeById(node_id);
    if (node) {
        node->click();
    }
}

bool JSBindings::matchesSelector(uint64_t node_id, const std::string& selector) const {
    const auto node = findNodeById(node_id);
    return node && node->matches(selector);
}

uint64_t JSBindings::closestSelector(uint64_t node_id, const std::string& selector) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return 0;
    }
    auto found = node->closest(selector);
    if (!found) {
        return 0;
    }
    return const_cast<JSBindings*>(this)->getNodeIdFor(found);
}

uint64_t JSBindings::createElement(const std::string& tag) {
    auto node = std::make_shared<dom::DOMNode>(dom::DOMNode::NodeType::ELEMENT, tag);
    dom::StyleEngine::applyDefaultStyleForNode(node);
    return getNodeIdFor(node);
}

uint64_t JSBindings::createTextNode(const std::string& text) {
    auto node = std::make_shared<dom::DOMNode>(dom::DOMNode::NodeType::TEXT, "");
    node->setTextContent(text);
    return getNodeIdFor(node);
}

void JSBindings::appendChild(uint64_t parent_id, uint64_t child_id) {
    const auto parent = resolveNode(parent_id, "appendChild(parent)");
    const auto child = resolveNode(child_id, "appendChild(child)");
    if (!parent || !child) {
        return;
    }
    parent->appendChild(child);
    registerNodeTree(child);
    markNodeTreeDirty(parent);
    ensureLayoutFresh();
}

void JSBindings::insertBefore(uint64_t parent_id, uint64_t new_id, uint64_t ref_id) {
    const auto parent = resolveNode(parent_id, "insertBefore(parent)");
    const auto new_child = resolveNode(new_id, "insertBefore(new)");
    if (!parent || !new_child) {
        return;
    }
    dom::DOMNodePtr ref_child;
    if (ref_id != 0) {
        ref_child = resolveNode(ref_id, "insertBefore(ref)");
        if (!ref_child) {
            return;
        }
    }
    parent->insertBefore(new_child, ref_child);
    markNodeTreeDirty(parent);
}

void JSBindings::removeNode(uint64_t node_id) {
    const auto node = resolveNode(node_id, "remove");
    if (!node) {
        return;
    }
    if (auto parent = node->getParent()) {
        parent->markStyleDirty();
        parent->markLayoutDirty();
    }
    node->remove();
    releaseNodeId(node_id);
}

void JSBindings::replaceChild(uint64_t parent_id, uint64_t new_id, uint64_t old_id) {
    const auto parent = resolveNode(parent_id, "replaceChild(parent)");
    const auto new_child = resolveNode(new_id, "replaceChild(new)");
    const auto old_child = resolveNode(old_id, "replaceChild(old)");
    if (!parent || !new_child || !old_child) {
        return;
    }
    parent->replaceChild(new_child, old_child);
    markNodeTreeDirty(parent);
    releaseNodeId(old_id);
}

uint64_t JSBindings::getParentId(uint64_t node_id) const {
    const auto node = resolveNode(node_id, "parent");
    if (!node) {
        return 0;
    }
    const auto parent = node->getParent();
    if (!parent) {
        return 0;
    }
    return const_cast<JSBindings*>(this)->getNodeIdFor(parent);
}

uint64_t JSBindings::getFirstChildId(uint64_t node_id) const {
    const auto node = resolveNode(node_id, "firstChild");
    if (!node) {
        return 0;
    }
    const auto child = node->getFirstChild();
    if (!child) {
        return 0;
    }
    return const_cast<JSBindings*>(this)->getNodeIdFor(child);
}

uint64_t JSBindings::getNextSiblingId(uint64_t node_id) const {
    const auto node = resolveNode(node_id, "nextSibling");
    if (!node) {
        return 0;
    }
    const auto sibling = node->getNextSibling();
    if (!sibling) {
        return 0;
    }
    return const_cast<JSBindings*>(this)->getNodeIdFor(sibling);
}

uint64_t JSBindings::cloneNodeId(uint64_t node_id, bool deep) const {
    const auto node = resolveNode(node_id, "cloneNode");
    if (!node) {
        return 0;
    }
    const auto clone = node->cloneNode(deep);
    if (!clone) {
        return 0;
    }
    return const_cast<JSBindings*>(this)->getNodeIdFor(clone);
}

void JSBindings::registerNodeTree(const dom::DOMNodePtr& node) {
    if (!node) {
        return;
    }
    getNodeIdFor(node);
    for (const auto& child : node->getChildren()) {
        registerNodeTree(child);
    }
}

uint64_t JSBindings::parseHtmlDetached(const std::string& html) {
    dom::HTMLParser parser;
    const auto root = parser.parse(html);
    if (!root) {
        return 0;
    }
    registerNodeTree(root);
    return getNodeIdFor(root);
}

std::string JSBindings::formSerializeJson(uint64_t form_id) const {
    const auto form = findNodeById(form_id);
    if (!form || form->getTagName() != "form") {
        return "[]";
    }
    std::vector<dom::DOMNodePtr> controls;
    collectFormControls(form, controls);
    std::vector<std::pair<std::string, std::string>> entries;
    for (const auto& control : controls) {
        serializeFormControl(control, entries);
    }
    return formEntriesToJson(entries);
}

std::string JSBindings::getSelectionText() const {
    if (!selection_) {
        return {};
    }
    return selection_->toString();
}

bool JSBindings::selectAllEditable() {
    if (!selection_) {
        return false;
    }
    dom::DOMNodePtr editable_root;
    if (focus_manager_) {
        auto focused = focus_manager_->getFocusedElement();
        if (focused && focused->isContentEditable()) {
            editable_root = dom::ContentEditableState::findEditableRoot(focused);
        }
    }
    if (!editable_root && last_editable_root_) {
        editable_root = last_editable_root_;
    }
    if (!editable_root) {
        return false;
    }
    selection_->selectAllChildren(editable_root);
    return true;
}

bool JSBindings::execCommand(const std::string& command, const std::string& value) {
    if (!selection_) {
        return false;
    }
    dom::DOMNodePtr editable_root;
    if (focus_manager_) {
        auto focused = focus_manager_->getFocusedElement();
        if (focused && focused->isContentEditable()) {
            editable_root = dom::ContentEditableState::findEditableRoot(focused);
        }
    }
    if (!editable_root && last_editable_root_) {
        editable_root = last_editable_root_;
    }
    if (!editable_root) {
        return false;
    }
    return dom::execCommand(editable_root, *selection_, command, value);
}

bool JSBindings::queryCommandSupported(const std::string& command) const {
    return dom::queryCommandSupported(command);
}

std::string JSBindings::clipboardRead() const {
    DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
    if (!cb) {
        return {};
    }
    char* text = dong_clipboard_get_text(cb);
    std::string result = text ? text : "";
    free(text);
    return result;
}

void JSBindings::clipboardWrite(const std::string& text) {
    DongClipboard* cb = dong_platform_get_clipboard(dong_platform_get());
    if (cb) {
        dong_clipboard_set_text(cb, text.c_str());
    }
}

bool JSBindings::matchMedia(const std::string& query) const {
    if (!dom_manager_) {
        return false;
    }
    auto* se = dom_manager_->getStyleEngine();
    if (!se) {
        return false;
    }
    float vw = layout_engine_ ? layout_engine_->getViewportWidth() : 800.0f;
    float vh = layout_engine_ ? layout_engine_->getViewportHeight() : 600.0f;
    const_cast<dom::StyleEngine*>(se)->setViewportSize(vw, vh);
    return se->evaluateMediaQuery(query);
}

bool JSBindings::cssSupports(const std::string& property, const std::string& value) const {
    std::string prop = property;
    std::transform(prop.begin(), prop.end(), prop.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    static const std::unordered_set<std::string> supported = {
        "display", "position", "width", "height", "margin", "padding", "border", "background",
        "color", "font-size", "font-weight", "flex", "flex-direction", "justify-content",
        "align-items", "overflow", "opacity", "z-index", "text-align", "line-height",
        "border-radius", "cursor", "visibility",
    };
    if (!supported.count(prop)) {
        return false;
    }
    if (value.empty()) {
        return true;
    }
    std::string val = value;
    std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return val.find("grid") == std::string::npos;
}

void JSBindings::dialogShow(uint64_t node_id) {
    const auto node = resolveNode(node_id, "dialog.show");
    if (!node) {
        return;
    }
    auto* state = dom::getDialogState(node);
    if (state && !state->isOpen()) {
        state->show();
        node->setAttribute("open", "");
        node->markStyleDirty();
        node->markLayoutDirty();
    }
}

void JSBindings::dialogShowModal(uint64_t node_id) {
    const auto node = resolveNode(node_id, "dialog.showModal");
    if (!node) {
        return;
    }
    auto* state = dom::getDialogState(node);
    if (state && !state->isOpen()) {
        state->showModal();
        node->setAttribute("open", "");
        node->markStyleDirty();
        node->markLayoutDirty();
    }
}

void JSBindings::dialogClose(uint64_t node_id, const std::string& return_value) {
    const auto node = resolveNode(node_id, "dialog.close");
    if (!node) {
        return;
    }
    auto* state = dom::getDialogState(node);
    if (state && state->isOpen()) {
        state->close(return_value);
        node->removeAttribute("open");
        node->markStyleDirty();
        node->markLayoutDirty();
    }
}

std::string JSBindings::dialogReturnValue(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return {};
    }
    auto* state = dom::getDialogState(node);
    return state ? state->getReturnValue() : std::string{};
}

bool JSBindings::dialogOpen(uint64_t node_id) const {
    const auto node = findNodeById(node_id);
    if (!node) {
        return false;
    }
    auto* state = dom::getDialogState(node);
    return state && state->isOpen();
}

uint32_t JSBindings::sceneAddNode(const std::string& config_json) {
    return porfforSceneAddNode(config_json);
}

void JSBindings::sceneRemove(uint32_t id) {
    porfforSceneRemove(id);
}

void JSBindings::sceneSet(uint32_t id, const std::string& prop, const std::string& value) {
    porfforSceneSet(id, prop, value);
}

int32_t JSBindings::sceneFind(const std::string& name) {
    return porfforSceneFind(name);
}

void JSBindings::sceneOn(uint32_t id, const std::string& type, const std::string& export_name,
                         const std::string& module_name) {
    PorfforScriptRegistry* registry = nullptr;
    if (engine_) {
        registry = engine_->registry();
    }
    porfforSceneOn(id, type, export_name, module_name, registry);
}

void JSBindings::sceneClear() {
    porfforSceneClear();
}

uint32_t JSBindings::sceneCount() const {
    return porfforSceneCount();
}

std::string JSBindings::textLayout(const std::string& config_json) {
    return porfforTextLayout(config_json);
}

void JSBindings::clearOverlay() {
    porfforClearOverlay(overlay_draw_);
}

void JSBindings::renderText(const std::string& config_json) {
    porfforRenderText(config_json, overlay_draw_);
}

void JSBindings::drawRect(const std::string& config_json) {
    porfforDrawRect(config_json, overlay_draw_);
}

void JSBindings::drawCircle(const std::string& config_json) {
    porfforDrawCircle(config_json, overlay_draw_);
}

void resetFetchState(void* /*ctx*/) {}

} // namespace dong::script
