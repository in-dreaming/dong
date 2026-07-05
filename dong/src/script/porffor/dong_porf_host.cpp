#include "dong_porf_host.hpp"

#include "registry.h"
#include "dong_porf_runtime.h"
#include "../../core/log.h"
#include "../../core/resource_loader.hpp"
#include "porffor_script_registry.hpp"
#include "js_bindings_porffor.hpp"
#include "dong_clipboard.h"
#include "dong_platform.h"
#include "../../dom/dialog_element.hpp"
#include "../../dom/html/html_parser.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <array>
#include <mutex>
#include <thread>
#include <unordered_set>

namespace dong::script {
namespace {

PorfforHost* g_active_host = nullptr;
thread_local const struct dong_porf_module* g_active_module = nullptr;
std::array<double, 256> g_state_nums{};

JSBindings* activeBindings() {
    return g_active_host ? g_active_host->bindings() : nullptr;
}

std::string normalizeFetchUrl(const std::string& url) {
    if (url.rfind("ui://", 0) == 0) {
        return url.substr(5);
    }
    return url;
}

bool matchMediaQuery(const std::string& query, JSBindings* bindings) {
    std::string q = query;
    std::transform(q.begin(), q.end(), q.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (q.find(',') != std::string::npos || q.find(" and ") != std::string::npos ||
        q.find(" or ") != std::string::npos) {
        return false;
    }
    if (q.find("(prefers-color-scheme: light)") != std::string::npos) {
        return true;
    }
    if (q.find("(prefers-color-scheme: dark)") != std::string::npos) {
        return false;
    }
    if (q.find("(prefers-reduced-motion: reduce)") != std::string::npos) {
        return false;
    }
    const float vw =
        bindings && bindings->layout_engine_ ? bindings->layout_engine_->getViewportWidth() : 800.0f;
    const float vh =
        bindings && bindings->layout_engine_ ? bindings->layout_engine_->getViewportHeight() : 600.0f;
    if (q.find("(orientation: portrait)") != std::string::npos) {
        return vh >= vw;
    }
    if (q.find("(orientation: landscape)") != std::string::npos) {
        return vw > vh;
    }
    if (q.find("(min-width:") != std::string::npos) {
        const size_t width_start = q.find("(min-width:") + 11;
        const size_t width_end = q.find(')', width_start);
        if (width_end != std::string::npos) {
            std::string width_str = q.substr(width_start, width_end - width_start);
            const size_t px_pos = width_str.find("px");
            if (px_pos != std::string::npos) {
                width_str = width_str.substr(0, px_pos);
            }
            try {
                return static_cast<int>(vw) >= std::stoi(width_str);
            } catch (...) {
                return false;
            }
        }
    }
    if (q.find("(max-width:") != std::string::npos) {
        const size_t width_start = q.find("(max-width:") + 11;
        const size_t width_end = q.find(')', width_start);
        if (width_end != std::string::npos) {
            std::string width_str = q.substr(width_start, width_end - width_start);
            const size_t px_pos = width_str.find("px");
            if (px_pos != std::string::npos) {
                width_str = width_str.substr(0, px_pos);
            }
            try {
                return static_cast<int>(vw) <= std::stoi(width_str);
            } catch (...) {
                return false;
            }
        }
    }
    return false;
}

bool cssPropertySupported(const std::string& prop) {
    static const std::unordered_set<std::string> supported = {
        "display",       "position",    "width",         "height",        "margin",
        "padding",       "border",      "background",    "color",         "font-size",
        "font-weight",   "flex",        "flex-direction","justify-content","align-items",
        "overflow",      "opacity",     "transform",     "z-index",       "top",
        "left",          "right",       "bottom",        "text-align",    "line-height",
        "border-radius", "box-shadow",  "cursor",        "visibility",    "white-space",
    };
    std::string key = prop;
    std::transform(key.begin(), key.end(), key.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return supported.count(key) > 0;
}

} // namespace

PorfforHost::PorfforHost() {
    start_ms_ = 0;
}

void PorfforHost::setActiveMemory(char* memory, unsigned int* memory_pages) {
    memory_ = memory;
    memory_pages_ = memory_pages;
}

size_t PorfforHost::memoryCapacityBytes() const {
    if (!memory_pages_) {
        return 0;
    }
    return static_cast<size_t>(*memory_pages_) * 65536u;
}

std::string PorfforHost::readByteString(double ptr) const {
    if (!memory_ || ptr < 0) {
        return {};
    }
    const size_t offset = static_cast<size_t>(ptr);
    const size_t cap = memoryCapacityBytes();
    if (offset + 4 > cap) {
        return {};
    }
    const u32 len = *reinterpret_cast<u32*>(memory_ + offset);
    if (len > kMaxImportStringBytes || offset + 4 + len > cap) {
        return {};
    }
    return std::string(memory_ + offset + 4, memory_ + offset + 4 + len);
}

void PorfforHost::setResultString(std::string s) {
    result_slot_ = std::move(s);
}

void PorfforHost::pushResultSlot() {
    result_slot_stack_.push_back(std::move(result_slot_));
    result_slot_.clear();
}

void PorfforHost::popResultSlot() {
    if (result_slot_stack_.empty()) {
        result_slot_.clear();
        return;
    }
    result_slot_ = std::move(result_slot_stack_.back());
    result_slot_stack_.pop_back();
}

double PorfforHost::strLen() const {
    return static_cast<double>(result_slot_.size());
}

double PorfforHost::strRead(double dest_ptr, double max_len) {
    if (!memory_ || dest_ptr < 0 || max_len <= 0 || result_slot_.empty()) {
        return 0;
    }
    const size_t offset = static_cast<size_t>(dest_ptr);
    const size_t cap = memoryCapacityBytes();
    if (offset >= cap) {
        return 0;
    }
    const size_t avail = cap - offset;
    const size_t want = std::min({
        result_slot_.size(),
        static_cast<size_t>(max_len),
        avail,
    });
    if (want == 0) {
        return 0;
    }
    std::memcpy(memory_ + offset, result_slot_.data(), want);
    return static_cast<double>(want);
}

double PorfforHost::strByteAt(double index) const {
    if (index < 0) {
        return -1;
    }
    const size_t i = static_cast<size_t>(index);
    if (i >= result_slot_.size()) {
        return -1;
    }
    return static_cast<double>(static_cast<unsigned char>(result_slot_[i]));
}

void PorfforHost::printString(double str_ptr) {
    const std::string s = readByteString(str_ptr);
    std::fputs(s.c_str(), stdout);
    std::fputc('\n', stdout);
}

void PorfforHost::benchLog(double str_ptr) {
    printString(str_ptr);
}

double PorfforHost::timeNow() const {
    if (start_ms_ <= 0) {
        return 0;
    }
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    const double ms =
        std::chrono::duration<double, std::milli>(now).count();
    return ms - start_ms_;
}

double PorfforHost::domGetElementById(double id_ptr) {
    auto* bindings = activeBindings();
    if (!bindings || !bindings->dom_manager_) {
        return 0;
    }
    const std::string id = readByteString(id_ptr);
    if (id.empty()) {
        return 0;
    }
    auto node = bindings->dom_manager_->getElementById(id);
    if (!node) {
        return 0;
    }
    return static_cast<double>(bindings->getNodeIdFor(node));
}

void PorfforHost::domSetTextContent(double node_id, double text_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return;
    }
    bindings->setNodeTextContent(static_cast<uint64_t>(node_id), readByteString(text_ptr));
}

void PorfforHost::domPrepareTextContent(double node_id) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString({});
        return;
    }
    setResultString(bindings->getNodeTextContent(static_cast<uint64_t>(node_id)));
}

void PorfforHost::domAddEventListener(double node_id, double type_ptr, double handler_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return;
    }
    const std::string type = readByteString(type_ptr);
    const std::string export_name = readByteString(handler_ptr);
    std::string module_name;
    if (registry_) {
        module_name = registry_->activeModule();
    }
    bindings->registerExportHandler(static_cast<uint64_t>(node_id), type, export_name,
                                    module_name);
}

uint64_t PorfforHost::scheduleTimer(double handler_ptr, double delay_ms, double interval_ms) {
    if (delay_ms < 0.0) {
        delay_ms = 0.0;
    }
    if (interval_ms >= 0.0 && interval_ms < 4.0) {
        interval_ms = 4.0;
    }

    TimerTask task;
    task.export_name = readByteString(handler_ptr);
    task.fire_at_ms = timeNow() + delay_ms;
    task.interval_ms = interval_ms;
    if (registry_) {
        task.module_name = registry_->activeModule();
    }

    const uint64_t id = next_timer_id_++;
    timers_.emplace(id, std::move(task));
    return id;
}

double PorfforHost::timerSetTimeout(double handler_ptr, double delay_ms) {
    return static_cast<double>(scheduleTimer(handler_ptr, delay_ms, -1.0));
}

double PorfforHost::timerSetInterval(double handler_ptr, double interval_ms) {
    const double delay = interval_ms < 0.0 ? 0.0 : interval_ms;
    return static_cast<double>(scheduleTimer(handler_ptr, delay, interval_ms));
}

void PorfforHost::timerClear(double timer_id) {
    if (timer_id <= 0.0) {
        return;
    }
    timers_.erase(static_cast<uint64_t>(timer_id));
}

double PorfforHost::requestAnimationFrame(double handler_ptr) {
    RafTask task;
    task.id = next_raf_id_++;
    task.export_name = readByteString(handler_ptr);
    if (registry_) {
        task.module_name = registry_->activeModule();
    }
    raf_queue_.push_back(std::move(task));
    return static_cast<double>(task.id);
}

void PorfforHost::cancelAnimationFrame(double raf_id) {
    if (raf_id <= 0.0) {
        return;
    }
    const uint64_t id = static_cast<uint64_t>(raf_id);
    raf_queue_.erase(std::remove_if(raf_queue_.begin(), raf_queue_.end(),
                                    [id](const RafTask& t) { return t.id == id; }),
                      raf_queue_.end());
}

double PorfforHost::rafTimestamp() const {
    return raf_timestamp_;
}

void PorfforHost::invokeExport(const std::string& module_name, const std::string& export_name,
                               double timestamp_ms, bool has_timestamp) {
    if (!registry_ || export_name.empty()) {
        return;
    }
    if (has_timestamp) {
        registry_->callExport(module_name, export_name, &timestamp_ms, 1);
    } else {
        registry_->callExport(module_name, export_name);
    }
}

void PorfforHost::stage0(double v) { stage_0_ = v; }
void PorfforHost::stage1(double v) { stage_1_ = v; }
void PorfforHost::stage2(double v) { stage_2_ = v; }

void PorfforHost::commitSetTextContent() {
    domSetTextContent(stage_0_, stage_1_);
}

void PorfforHost::commitAddEventListener() {
    domAddEventListener(stage_0_, stage_1_, stage_2_);
}

double PorfforHost::commitSetTimeout() {
    return timerSetTimeout(stage_1_, stage_0_);
}

double PorfforHost::commitSetInterval() {
    return timerSetInterval(stage_1_, stage_0_);
}

double PorfforHost::commitRequestAnimationFrame() {
    return requestAnimationFrame(stage_0_);
}

void PorfforHost::getValue(double node_id) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString({});
        return;
    }
    setResultString(bindings->getNodeValue(static_cast<uint64_t>(node_id)));
}

void PorfforHost::setValue(double node_id, double value_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return;
    }
    bindings->setNodeValue(static_cast<uint64_t>(node_id), readByteString(value_ptr));
}

double PorfforHost::getChecked(double node_id) const {
    auto* bindings = activeBindings();
    return bindings && bindings->getNodeChecked(static_cast<uint64_t>(node_id)) ? 1.0 : 0.0;
}

void PorfforHost::setChecked(double node_id, double checked) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->setNodeChecked(static_cast<uint64_t>(node_id), checked != 0.0);
    }
}

double PorfforHost::getDisabled(double node_id) const {
    auto* bindings = activeBindings();
    return bindings && bindings->getNodeDisabled(static_cast<uint64_t>(node_id)) ? 1.0 : 0.0;
}

void PorfforHost::setDisabled(double node_id, double disabled) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->setNodeDisabled(static_cast<uint64_t>(node_id), disabled != 0.0);
    }
}

void PorfforHost::getAttribute(double node_id, double name_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString({});
        return;
    }
    setResultString(bindings->getNodeAttribute(static_cast<uint64_t>(node_id),
                                               readByteString(name_ptr)));
}

void PorfforHost::setAttribute(double node_id, double name_ptr, double value_ptr) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->setNodeAttribute(static_cast<uint64_t>(node_id), readByteString(name_ptr),
                                   readByteString(value_ptr));
    }
}

void PorfforHost::removeAttribute(double node_id, double name_ptr) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->removeNodeAttribute(static_cast<uint64_t>(node_id), readByteString(name_ptr));
    }
}

void PorfforHost::setInnerHTML(double node_id, double html_ptr) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->setNodeInnerHTML(static_cast<uint64_t>(node_id), readByteString(html_ptr));
    }
}

double PorfforHost::querySelector(double root_id, double selector_ptr) const {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0.0;
    }
    return static_cast<double>(
        bindings->querySelector(static_cast<uint64_t>(root_id), readByteString(selector_ptr)));
}

void PorfforHost::querySelectorAll(double root_id, double selector_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString("[]");
        return;
    }
    setResultString(bindings->querySelectorAllJson(static_cast<uint64_t>(root_id),
                                                   readByteString(selector_ptr)));
}

void PorfforHost::getElementsByTagName(double root_id, double tag_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString("[]");
        return;
    }
    setResultString(bindings->getElementsByTagNameJson(static_cast<uint64_t>(root_id),
                                                       readByteString(tag_ptr)));
}

void PorfforHost::classAdd(double node_id, double cls_ptr) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->classAdd(static_cast<uint64_t>(node_id), readByteString(cls_ptr));
    }
}

void PorfforHost::classRemove(double node_id, double cls_ptr) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->classRemove(static_cast<uint64_t>(node_id), readByteString(cls_ptr));
    }
}

double PorfforHost::classToggle(double node_id, double cls_ptr) const {
    auto* bindings = activeBindings();
    return bindings && bindings->classToggle(static_cast<uint64_t>(node_id),
                                             readByteString(cls_ptr))
               ? 1.0
               : 0.0;
}

double PorfforHost::classContains(double node_id, double cls_ptr) const {
    auto* bindings = activeBindings();
    return bindings && bindings->classContains(static_cast<uint64_t>(node_id),
                                               readByteString(cls_ptr))
               ? 1.0
               : 0.0;
}

void PorfforHost::styleSet(double node_id, double prop_ptr, double value_ptr) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->styleSet(static_cast<uint64_t>(node_id), readByteString(prop_ptr),
                           readByteString(value_ptr));
    }
}

void PorfforHost::styleGet(double node_id, double prop_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString({});
        return;
    }
    setResultString(bindings->styleGet(static_cast<uint64_t>(node_id), readByteString(prop_ptr)));
}

void PorfforHost::computedStyleGet(double node_id, double prop_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString({});
        return;
    }
    setResultString(
        bindings->computedStyleGet(static_cast<uint64_t>(node_id), readByteString(prop_ptr)));
}

void PorfforHost::getRect(double node_id) {
    auto* bindings = activeBindings();
    if (!bindings) {
        setResultString(R"({"x":0,"y":0,"w":0,"h":0})");
        return;
    }
    setResultString(bindings->getRectJson(static_cast<uint64_t>(node_id)));
}

double PorfforHost::getMetric(double node_id, double metric_id) const {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0.0;
    }
    return bindings->getMetric(static_cast<uint64_t>(node_id), static_cast<int>(metric_id));
}

double PorfforHost::getScrollTop(double node_id) const {
    auto* bindings = activeBindings();
    return bindings ? bindings->getScrollTop(static_cast<uint64_t>(node_id)) : 0.0;
}

void PorfforHost::setScrollTop(double node_id, double value) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->setScrollTop(static_cast<uint64_t>(node_id), value);
    }
}

double PorfforHost::getScrollLeft(double node_id) const {
    auto* bindings = activeBindings();
    return bindings ? bindings->getScrollLeft(static_cast<uint64_t>(node_id)) : 0.0;
}

void PorfforHost::setScrollLeft(double node_id, double value) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->setScrollLeft(static_cast<uint64_t>(node_id), value);
    }
}

void PorfforHost::focusNode(double node_id) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->focusNode(static_cast<uint64_t>(node_id));
    }
}

void PorfforHost::blurNode(double node_id) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->blurNode(static_cast<uint64_t>(node_id));
    }
}

void PorfforHost::clickNode(double node_id) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->clickNode(static_cast<uint64_t>(node_id));
    }
}

double PorfforHost::matchesSelector(double node_id, double selector_ptr) const {
    auto* bindings = activeBindings();
    return bindings && bindings->matchesSelector(static_cast<uint64_t>(node_id),
                                                 readByteString(selector_ptr))
               ? 1.0
               : 0.0;
}

double PorfforHost::closestSelector(double node_id, double selector_ptr) const {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0.0;
    }
    return static_cast<double>(
        bindings->closestSelector(static_cast<uint64_t>(node_id), readByteString(selector_ptr)));
}

double PorfforHost::createElement(double tag_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0.0;
    }
    return static_cast<double>(bindings->createElement(readByteString(tag_ptr)));
}

double PorfforHost::createTextNode(double text_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0.0;
    }
    return static_cast<double>(bindings->createTextNode(readByteString(text_ptr)));
}

void PorfforHost::appendChild(double parent_id, double child_id) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->appendChild(static_cast<uint64_t>(parent_id), static_cast<uint64_t>(child_id));
    }
}

void PorfforHost::insertBefore(double parent_id, double new_id, double ref_id) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->insertBefore(static_cast<uint64_t>(parent_id), static_cast<uint64_t>(new_id),
                               static_cast<uint64_t>(ref_id));
    }
}

void PorfforHost::removeNode(double node_id) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->removeNode(static_cast<uint64_t>(node_id));
    }
}

void PorfforHost::replaceChild(double parent_id, double new_id, double old_id) {
    auto* bindings = activeBindings();
    if (bindings) {
        bindings->replaceChild(static_cast<uint64_t>(parent_id), static_cast<uint64_t>(new_id),
                             static_cast<uint64_t>(old_id));
    }
}

double PorfforHost::getParentId(double node_id) const {
    auto* bindings = activeBindings();
    return bindings ? static_cast<double>(bindings->getParentId(static_cast<uint64_t>(node_id)))
                    : 0.0;
}

double PorfforHost::getFirstChildId(double node_id) const {
    auto* bindings = activeBindings();
    return bindings ? static_cast<double>(bindings->getFirstChildId(static_cast<uint64_t>(node_id)))
                    : 0.0;
}

double PorfforHost::getNextSiblingId(double node_id) const {
    auto* bindings = activeBindings();
    return bindings
               ? static_cast<double>(bindings->getNextSiblingId(static_cast<uint64_t>(node_id)))
               : 0.0;
}

double PorfforHost::cloneNodeId(double node_id, double deep) const {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0.0;
    }
    return static_cast<double>(
        bindings->cloneNodeId(static_cast<uint64_t>(node_id), deep != 0.0));
}

static int32_t eventModifiersFrom(const dom::Event* ev) {
    if (!ev) {
        return 0;
    }
    int32_t m = 0;
    if (ev->shift_key) {
        m |= 1;
    }
    if (ev->ctrl_key) {
        m |= 2;
    }
    if (ev->alt_key) {
        m |= 4;
    }
    if (ev->meta_key) {
        m |= 8;
    }
    return m;
}

void PorfforHost::pushEventSlot(const dom::Event* dom_event, uint64_t target_node_id,
                                const std::string& type) {
    event_slot_stack_.push_back(std::move(event_slot_));
    event_slot_ = EventSlot{};
    event_slot_.dom_event = dom_event;
    event_slot_.target_node_id = target_node_id;
    event_slot_.type = type;
    if (dom_event) {
        if (!dom_event->type_name.empty()) {
            event_slot_.type = dom_event->type_name;
        }
        event_slot_.key = dom_event->key_string.empty() ? dom_event->key_name : dom_event->key_string;
        event_slot_.key_code = dom_event->key_code;
        event_slot_.x = static_cast<double>(dom_event->mouse_x);
        event_slot_.y = static_cast<double>(dom_event->mouse_y);
        event_slot_.button = dom_event->mouse_button;
        event_slot_.modifiers = eventModifiersFrom(dom_event);
        if (!dom_event->input_data.empty()) {
            event_slot_.value = dom_event->input_data;
        } else if (bindings_ && target_node_id != 0) {
            event_slot_.value = bindings_->getNodeValue(target_node_id);
        }
    }
}

void PorfforHost::popEventSlot() {
    if (event_slot_stack_.empty()) {
        event_slot_ = EventSlot{};
        return;
    }
    event_slot_ = std::move(event_slot_stack_.back());
    event_slot_stack_.pop_back();
}

bool PorfforHost::eventPreventDefault() const {
    return event_slot_.prevent_default;
}

bool PorfforHost::eventStopPropagation() const {
    return event_slot_.stop_propagation;
}

void PorfforHost::eventPrepareType() {
    setResultString(event_slot_.type);
}

double PorfforHost::eventTarget() const {
    return static_cast<double>(event_slot_.target_node_id);
}

void PorfforHost::eventPrepareKey() {
    setResultString(event_slot_.key);
}

double PorfforHost::eventKeyCode() const {
    return static_cast<double>(event_slot_.key_code);
}

double PorfforHost::eventX() const {
    return event_slot_.x;
}

double PorfforHost::eventY() const {
    return event_slot_.y;
}

double PorfforHost::eventButton() const {
    return static_cast<double>(event_slot_.button);
}

double PorfforHost::eventModifiers() const {
    return static_cast<double>(event_slot_.modifiers);
}

void PorfforHost::eventPrepareValue() {
    setResultString(event_slot_.value);
}

void PorfforHost::eventPreventDefaultFlag() {
    event_slot_.prevent_default = true;
    if (event_slot_.dom_event) {
        const_cast<dom::Event*>(event_slot_.dom_event)->preventDefault();
    }
}

void PorfforHost::eventStopPropagationFlag() {
    event_slot_.stop_propagation = true;
    if (event_slot_.dom_event) {
        const_cast<dom::Event*>(event_slot_.dom_event)->stopPropagation();
    }
}

void PorfforHost::pushFetchSlot() {
    fetch_slot_stack_.push_back(std::move(fetch_slot_));
    fetch_slot_ = FetchSlot{};
}

void PorfforHost::popFetchSlot() {
    if (fetch_slot_stack_.empty()) {
        fetch_slot_ = FetchSlot{};
        return;
    }
    fetch_slot_ = std::move(fetch_slot_stack_.back());
    fetch_slot_stack_.pop_back();
}

bool PorfforHost::fetchSlotReadable() const {
    return fetch_slot_active_;
}

double PorfforHost::fetchStart(double url_ptr, double export_ptr) {
    const std::string url = readByteString(url_ptr);
    const std::string export_name = readByteString(export_ptr);
    if (url.empty() || export_name.empty()) {
        return 0.0;
    }

    const uint64_t id = next_fetch_id_++;
    PendingFetch pending;
    pending.export_name = export_name;
    if (registry_) {
        pending.module_name = registry_->activeModule();
    }
    {
        std::lock_guard<std::mutex> lock(fetch_mutex_);
        pending_fetches_.emplace(id, std::move(pending));
    }

    std::string resource_root;
    if (auto* bindings = activeBindings()) {
        if (bindings->dom_manager_) {
            resource_root = bindings->dom_manager_->getResourceRoot();
        }
    }
    const std::string norm_url = normalizeFetchUrl(url);

    auto load_and_queue = [this, id, norm_url, resource_root]() {
        dong::ResourceLoadResult result = dong::loadTextResource(norm_url, resource_root);

        FetchCompletion completion;
        completion.id = id;
        completion.result = std::move(result);

        std::lock_guard<std::mutex> lock(fetch_mutex_);
        const auto it = pending_fetches_.find(id);
        if (it != pending_fetches_.end()) {
            completion.module_name = it->second.module_name;
            completion.export_name = it->second.export_name;
        }
        fetch_completions_.push_back(std::move(completion));
    };

    if (dong::isHttpUrl(norm_url)) {
        std::thread(load_and_queue).detach();
    } else {
        load_and_queue();
    }

    return static_cast<double>(id);
}

double PorfforHost::commitFetchStart() {
    return fetchStart(stage_0_, stage_1_);
}

void PorfforHost::fetchAbort(double request_id) {
    if (request_id <= 0.0) {
        return;
    }
    std::lock_guard<std::mutex> lock(fetch_mutex_);
    const auto it = pending_fetches_.find(static_cast<uint64_t>(request_id));
    if (it != pending_fetches_.end()) {
        it->second.aborted = true;
        pending_fetches_.erase(it);
    }
}

void PorfforHost::resetFetches() {
    {
        std::lock_guard<std::mutex> lock(fetch_mutex_);
        pending_fetches_.clear();
        fetch_completions_.clear();
    }
    fetch_slot_ = FetchSlot{};
    fetch_slot_stack_.clear();
    fetch_slot_active_ = false;
}

double PorfforHost::fetchRequestId() const {
    if (!fetchSlotReadable()) {
        DONG_LOG_DEBUG("[PorfforHost] fetchRequestId outside callback");
        return 0.0;
    }
    return static_cast<double>(fetch_slot_.request_id);
}

double PorfforHost::fetchStatus() const {
    if (!fetchSlotReadable()) {
        DONG_LOG_DEBUG("[PorfforHost] fetchStatus outside callback");
        return 0.0;
    }
    return static_cast<double>(fetch_slot_.status);
}

double PorfforHost::fetchOk() const {
    if (!fetchSlotReadable()) {
        DONG_LOG_DEBUG("[PorfforHost] fetchOk outside callback");
        return 0.0;
    }
    return fetch_slot_.ok ? 1.0 : 0.0;
}

void PorfforHost::fetchPrepareBody() {
    if (!fetchSlotReadable()) {
        DONG_LOG_DEBUG("[PorfforHost] fetchBody outside callback");
        setResultString({});
        return;
    }
    setResultString(fetch_slot_.body);
}

void PorfforHost::fetchPrepareError() {
    if (!fetchSlotReadable()) {
        DONG_LOG_DEBUG("[PorfforHost] fetchError outside callback");
        setResultString({});
        return;
    }
    setResultString(fetch_slot_.error);
}

void PorfforHost::fetchPrepareHeader(double /*name_ptr*/) {
    if (!fetchSlotReadable()) {
        DONG_LOG_DEBUG("[PorfforHost] fetchHeader outside callback");
        setResultString({});
        return;
    }
    setResultString(fetch_slot_.header_value);
}

void PorfforHost::processFetches() {
    if (!registry_) {
        return;
    }

    std::vector<FetchCompletion> completions;
    {
        std::lock_guard<std::mutex> lock(fetch_mutex_);
        completions.swap(fetch_completions_);
    }

    for (auto& completion : completions) {
        std::string module_name;
        std::string export_name;
        {
            std::lock_guard<std::mutex> lock(fetch_mutex_);
            const auto it = pending_fetches_.find(completion.id);
            if (it == pending_fetches_.end() || it->second.aborted) {
                pending_fetches_.erase(completion.id);
                continue;
            }
            module_name = it->second.module_name;
            export_name = it->second.export_name;
            pending_fetches_.erase(it);
        }

        if (completion.module_name.empty()) {
            completion.module_name = module_name;
        }
        if (completion.export_name.empty()) {
            completion.export_name = export_name;
        }

        pushFetchSlot();
        fetch_slot_active_ = true;
        fetch_slot_.request_id = static_cast<int32_t>(completion.id);
        if (completion.result.status_code > 0) {
            fetch_slot_.status = completion.result.status_code;
        } else {
            fetch_slot_.status = completion.result.success ? 200 : 0;
        }
        fetch_slot_.ok = completion.result.success;
        fetch_slot_.body = completion.result.content;
        fetch_slot_.error = completion.result.error_msg;

        invokeExport(completion.module_name, completion.export_name, 0.0, false);

        fetch_slot_active_ = false;
        popFetchSlot();
    }
}

void PorfforHost::clipboardWrite(double text_ptr) {
    if (auto* bindings = activeBindings()) {
        bindings->clipboardWrite(readByteString(text_ptr));
    }
}

void PorfforHost::clipboardRead() {
    if (auto* bindings = activeBindings()) {
        setResultString(bindings->clipboardRead());
    } else {
        setResultString({});
    }
}

double PorfforHost::matchMedia(double query_ptr) const {
    if (auto* bindings = activeBindings()) {
        return bindings->matchMedia(readByteString(query_ptr)) ? 1.0 : 0.0;
    }
    return 0.0;
}

double PorfforHost::cssSupportsProp(double prop_ptr, double value_ptr) const {
    if (auto* bindings = activeBindings()) {
        return bindings->cssSupports(readByteString(prop_ptr), readByteString(value_ptr)) ? 1.0 : 0.0;
    }
    return 0.0;
}

void PorfforHost::dialogShow(double node_id) {
    if (auto* bindings = activeBindings()) {
        bindings->dialogShow(static_cast<uint64_t>(node_id));
    }
}

void PorfforHost::dialogShowModal(double node_id) {
    if (auto* bindings = activeBindings()) {
        bindings->dialogShowModal(static_cast<uint64_t>(node_id));
    }
}

void PorfforHost::dialogClose(double node_id, double return_value_ptr) {
    if (auto* bindings = activeBindings()) {
        bindings->dialogClose(static_cast<uint64_t>(node_id), readByteString(return_value_ptr));
    }
}

void PorfforHost::dialogPrepareReturnValue(double node_id) {
    if (auto* bindings = activeBindings()) {
        setResultString(bindings->dialogReturnValue(static_cast<uint64_t>(node_id)));
    } else {
        setResultString({});
    }
}

double PorfforHost::dialogIsOpen(double node_id) const {
    if (auto* bindings = activeBindings()) {
        return bindings->dialogOpen(static_cast<uint64_t>(node_id)) ? 1.0 : 0.0;
    }
    return 0.0;
}

double PorfforHost::sceneAddNode(double config_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0.0;
    }
    return static_cast<double>(bindings->sceneAddNode(readByteString(config_ptr)));
}

void PorfforHost::sceneRemove(double id) {
    if (auto* bindings = activeBindings()) {
        bindings->sceneRemove(static_cast<uint32_t>(id));
    }
}

void PorfforHost::sceneSet(double id, double prop_ptr, double value_ptr) {
    if (auto* bindings = activeBindings()) {
        bindings->sceneSet(static_cast<uint32_t>(id), readByteString(prop_ptr),
                          readByteString(value_ptr));
    }
}

double PorfforHost::sceneFind(double name_ptr) const {
    if (auto* bindings = activeBindings()) {
        return static_cast<double>(bindings->sceneFind(readByteString(name_ptr)));
    }
    return -1.0;
}

void PorfforHost::sceneOn(double id, double type_ptr, double handler_ptr) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return;
    }
    std::string module_name;
    if (registry_) {
        module_name = registry_->activeModule();
    }
    bindings->sceneOn(static_cast<uint32_t>(id), readByteString(type_ptr), readByteString(handler_ptr),
                      module_name);
}

void PorfforHost::sceneClear() {
    if (auto* bindings = activeBindings()) {
        bindings->sceneClear();
    }
}

double PorfforHost::sceneCount() const {
    if (auto* bindings = activeBindings()) {
        return static_cast<double>(bindings->sceneCount());
    }
    return 0.0;
}

void PorfforHost::textLayoutPrepare(double config_ptr) {
    if (auto* bindings = activeBindings()) {
        setResultString(bindings->textLayout(readByteString(config_ptr)));
    } else {
        setResultString("{}");
    }
}

void PorfforHost::clearOverlay() {
    if (auto* bindings = activeBindings()) {
        bindings->clearOverlay();
    }
}

void PorfforHost::renderText(double config_ptr) {
    if (auto* bindings = activeBindings()) {
        bindings->renderText(readByteString(config_ptr));
    }
}

void PorfforHost::drawRect(double config_ptr) {
    if (auto* bindings = activeBindings()) {
        bindings->drawRect(readByteString(config_ptr));
    }
}

void PorfforHost::drawCircle(double config_ptr) {
    if (auto* bindings = activeBindings()) {
        bindings->drawCircle(readByteString(config_ptr));
    }
}

void PorfforHost::parseHtml(double html_ptr) {
    setResultString({});
    auto* bindings = activeBindings();
    if (!bindings) {
        return;
    }
    const std::string html = readByteString(html_ptr);
    const uint64_t id = bindings->parseHtmlDetached(html);
    if (id == 0) {
        return;
    }
    setResultString(std::to_string(id));
}

void PorfforHost::formSerialize(double form_node_id) {
    setResultString("[]");
    auto* bindings = activeBindings();
    if (!bindings) {
        return;
    }
    setResultString(bindings->formSerializeJson(static_cast<uint64_t>(form_node_id)));
}

void PorfforHost::selectionText() {
    setResultString({});
    auto* bindings = activeBindings();
    if (!bindings) {
        return;
    }
    setResultString(bindings->getSelectionText());
}

void PorfforHost::processTimers(double now_ms) {
    if (timers_.empty() || !registry_) {
        return;
    }

    std::vector<uint64_t> due_ids;
    due_ids.reserve(timers_.size());
    for (const auto& [id, task] : timers_) {
        if (task.fire_at_ms <= now_ms) {
            due_ids.push_back(id);
        }
    }

    for (const uint64_t id : due_ids) {
        auto it = timers_.find(id);
        if (it == timers_.end()) {
            continue;
        }

        if (it->second.interval_ms >= 0.0) {
            it->second.fire_at_ms = now_ms + it->second.interval_ms;
            invokeExport(it->second.module_name, it->second.export_name, 0.0, false);
        } else {
            TimerTask task = std::move(it->second);
            timers_.erase(it);
            invokeExport(task.module_name, task.export_name, 0.0, false);
        }
    }
}

void PorfforHost::processAnimationFrames(double timestamp_ms) {
    if (raf_queue_.empty() || !registry_) {
        return;
    }

    raf_timestamp_ = timestamp_ms;
    std::vector<RafTask> callbacks = std::move(raf_queue_);
    raf_queue_.clear();

    for (const auto& task : callbacks) {
        const dong_porf_handler_t* handler =
            dong_porf_find_handler(task.module_name.c_str(), task.export_name.c_str());
        const bool has_timestamp = handler && handler->param_count == 1;
        invokeExport(task.module_name, task.export_name, timestamp_ms, has_timestamp);
    }
}

void PorfforHost::setBindings(JSBindings* bindings) {
    bindings_ = bindings;
    if (start_ms_ <= 0) {
        const auto now = std::chrono::steady_clock::now().time_since_epoch();
        start_ms_ = std::chrono::duration<double, std::milli>(now).count();
    }
}

JSBindings* PorfforHost::bindings() const {
    return bindings_;
}

} // namespace dong::script

using dong::script::g_active_module;
using dong::script::g_active_host;

extern "C" {

void __porf_import_dong_print(f64 arg) {
    if (g_active_host) {
        g_active_host->printString(arg);
    }
}

void __porf_import_dong_bench_log(f64 arg) {
    if (g_active_host) {
        g_active_host->benchLog(arg);
    }
}

f64 __porf_import_dong_time_now(void) {
    return g_active_host ? g_active_host->timeNow() : 0.0;
}

f64 __porf_import_dong_dom_getElementById(f64 id_ptr) {
    return g_active_host ? g_active_host->domGetElementById(id_ptr) : 0.0;
}

void __porf_import_dong_dom_set_textContent(f64 node_id, f64 text_ptr) {
    if (g_active_host) {
        g_active_host->domSetTextContent(node_id, text_ptr);
    }
}

void __porf_import_dong_dom_get_textContent(f64 node_id) {
    if (g_active_host) {
        g_active_host->domPrepareTextContent(node_id);
    }
}

f64 __porf_import_dong_str_len(void) {
    return g_active_host ? g_active_host->strLen() : 0.0;
}

f64 __porf_import_dong_str_read(f64 dest_ptr, f64 max_len) {
    return g_active_host ? g_active_host->strRead(dest_ptr, max_len) : 0.0;
}

f64 __porf_import_dong_str_byte_at(f64 index) {
    return g_active_host ? g_active_host->strByteAt(index) : -1.0;
}

void __porf_import_dong_dom_addEventListener(f64 node_id, f64 type_ptr, f64 handler_ptr) {
    if (g_active_host) {
        g_active_host->domAddEventListener(node_id, type_ptr, handler_ptr);
    }
}

f64 __porf_import_dong_timer_setTimeout(f64 handler_ptr, f64 delay_ms) {
    return g_active_host ? g_active_host->timerSetTimeout(handler_ptr, delay_ms) : 0.0;
}

void __porf_import_dong_stage_0(f64 v) {
    if (g_active_host) {
        g_active_host->stage0(v);
    }
}

void __porf_import_dong_stage_1(f64 v) {
    if (g_active_host) {
        g_active_host->stage1(v);
    }
}

void __porf_import_dong_stage_2(f64 v) {
    if (g_active_host) {
        g_active_host->stage2(v);
    }
}

void __porf_import_dong_commit_set_textContent(void) {
    if (g_active_host) {
        g_active_host->commitSetTextContent();
    }
}

void __porf_import_dong_commit_addEventListener(void) {
    if (g_active_host) {
        g_active_host->commitAddEventListener();
    }
}

f64 __porf_import_dong_commit_setTimeout(void) {
    return g_active_host ? g_active_host->commitSetTimeout() : 0.0;
}

f64 __porf_import_dong_set_interval(f64 handler_ptr, f64 interval_ms) {
    return g_active_host ? g_active_host->timerSetInterval(handler_ptr, interval_ms) : 0.0;
}

void __porf_import_dong_clear_interval(f64 timer_id) {
    if (g_active_host) {
        g_active_host->timerClear(timer_id);
    }
}

void __porf_import_dong_clear_timeout(f64 timer_id) {
    if (g_active_host) {
        g_active_host->timerClear(timer_id);
    }
}

f64 __porf_import_dong_commit_setInterval(void) {
    return g_active_host ? g_active_host->commitSetInterval() : 0.0;
}

f64 __porf_import_dong_request_animation_frame(f64 handler_ptr) {
    return g_active_host ? g_active_host->requestAnimationFrame(handler_ptr) : 0.0;
}

void __porf_import_dong_cancel_animation_frame(f64 raf_id) {
    if (g_active_host) {
        g_active_host->cancelAnimationFrame(raf_id);
    }
}

f64 __porf_import_dong_commit_requestAnimationFrame(void) {
    return g_active_host ? g_active_host->commitRequestAnimationFrame() : 0.0;
}

f64 __porf_import_dong_raf_timestamp(void) {
    return g_active_host ? g_active_host->rafTimestamp() : 0.0;
}

void __porf_import_dong_state_set_num(f64 slot, f64 value) {
    const size_t idx = static_cast<size_t>(slot);
    if (idx < g_state_nums.size()) {
        g_state_nums[idx] = value;
    }
}

f64 __porf_import_dong_state_get_num(f64 slot) {
    const size_t idx = static_cast<size_t>(slot);
    if (idx < g_state_nums.size()) {
        return g_state_nums[idx];
    }
    return 0.0;
}

void __porf_import_dong_get_value(f64 node_id) {
    if (g_active_host) {
        g_active_host->getValue(node_id);
    }
}

void __porf_import_dong_set_value(f64 node_id, f64 value_ptr) {
    if (g_active_host) {
        g_active_host->setValue(node_id, value_ptr);
    }
}

f64 __porf_import_dong_get_checked(f64 node_id) {
    return g_active_host ? g_active_host->getChecked(node_id) : 0.0;
}

void __porf_import_dong_set_checked(f64 node_id, f64 checked) {
    if (g_active_host) {
        g_active_host->setChecked(node_id, checked);
    }
}

f64 __porf_import_dong_get_disabled(f64 node_id) {
    return g_active_host ? g_active_host->getDisabled(node_id) : 0.0;
}

void __porf_import_dong_set_disabled(f64 node_id, f64 disabled) {
    if (g_active_host) {
        g_active_host->setDisabled(node_id, disabled);
    }
}

void __porf_import_dong_get_attribute(f64 node_id, f64 name_ptr) {
    if (g_active_host) {
        g_active_host->getAttribute(node_id, name_ptr);
    }
}

void __porf_import_dong_set_attribute(f64 node_id, f64 name_ptr, f64 value_ptr) {
    if (g_active_host) {
        g_active_host->setAttribute(node_id, name_ptr, value_ptr);
    }
}

void __porf_import_dong_remove_attribute(f64 node_id, f64 name_ptr) {
    if (g_active_host) {
        g_active_host->removeAttribute(node_id, name_ptr);
    }
}

void __porf_import_dong_set_inner_html(f64 node_id, f64 html_ptr) {
    if (g_active_host) {
        g_active_host->setInnerHTML(node_id, html_ptr);
    }
}

f64 __porf_import_dong_query_selector(f64 root_id, f64 selector_ptr) {
    return g_active_host ? g_active_host->querySelector(root_id, selector_ptr) : 0.0;
}

void __porf_import_dong_query_selector_all(f64 root_id, f64 selector_ptr) {
    if (g_active_host) {
        g_active_host->querySelectorAll(root_id, selector_ptr);
    }
}

void __porf_import_dong_get_elements_by_tag_name(f64 root_id, f64 tag_ptr) {
    if (g_active_host) {
        g_active_host->getElementsByTagName(root_id, tag_ptr);
    }
}

void __porf_import_dong_class_add(f64 node_id, f64 cls_ptr) {
    if (g_active_host) {
        g_active_host->classAdd(node_id, cls_ptr);
    }
}

void __porf_import_dong_class_remove(f64 node_id, f64 cls_ptr) {
    if (g_active_host) {
        g_active_host->classRemove(node_id, cls_ptr);
    }
}

f64 __porf_import_dong_class_toggle(f64 node_id, f64 cls_ptr) {
    return g_active_host ? g_active_host->classToggle(node_id, cls_ptr) : 0.0;
}

f64 __porf_import_dong_class_contains(f64 node_id, f64 cls_ptr) {
    return g_active_host ? g_active_host->classContains(node_id, cls_ptr) : 0.0;
}

void __porf_import_dong_style_set(f64 node_id, f64 prop_ptr, f64 value_ptr) {
    if (g_active_host) {
        g_active_host->styleSet(node_id, prop_ptr, value_ptr);
    }
}

void __porf_import_dong_style_get(f64 node_id, f64 prop_ptr) {
    if (g_active_host) {
        g_active_host->styleGet(node_id, prop_ptr);
    }
}

void __porf_import_dong_computed_style_get(f64 node_id, f64 prop_ptr) {
    if (g_active_host) {
        g_active_host->computedStyleGet(node_id, prop_ptr);
    }
}

void __porf_import_dong_get_rect(f64 node_id) {
    if (g_active_host) {
        g_active_host->getRect(node_id);
    }
}

f64 __porf_import_dong_get_metric(f64 node_id, f64 metric_id) {
    return g_active_host ? g_active_host->getMetric(node_id, metric_id) : 0.0;
}

f64 __porf_import_dong_get_scroll_top(f64 node_id) {
    return g_active_host ? g_active_host->getScrollTop(node_id) : 0.0;
}

void __porf_import_dong_set_scroll_top(f64 node_id, f64 value) {
    if (g_active_host) {
        g_active_host->setScrollTop(node_id, value);
    }
}

f64 __porf_import_dong_get_scroll_left(f64 node_id) {
    return g_active_host ? g_active_host->getScrollLeft(node_id) : 0.0;
}

void __porf_import_dong_set_scroll_left(f64 node_id, f64 value) {
    if (g_active_host) {
        g_active_host->setScrollLeft(node_id, value);
    }
}

void __porf_import_dong_focus(f64 node_id) {
    if (g_active_host) {
        g_active_host->focusNode(node_id);
    }
}

void __porf_import_dong_blur(f64 node_id) {
    if (g_active_host) {
        g_active_host->blurNode(node_id);
    }
}

void __porf_import_dong_click(f64 node_id) {
    if (g_active_host) {
        g_active_host->clickNode(node_id);
    }
}

f64 __porf_import_dong_matches(f64 node_id, f64 selector_ptr) {
    return g_active_host ? g_active_host->matchesSelector(node_id, selector_ptr) : 0.0;
}

f64 __porf_import_dong_closest(f64 node_id, f64 selector_ptr) {
    return g_active_host ? g_active_host->closestSelector(node_id, selector_ptr) : 0.0;
}

f64 __porf_import_dong_create_element(f64 tag_ptr) {
    return g_active_host ? g_active_host->createElement(tag_ptr) : 0.0;
}

f64 __porf_import_dong_create_text_node(f64 text_ptr) {
    return g_active_host ? g_active_host->createTextNode(text_ptr) : 0.0;
}

void __porf_import_dong_append_child(f64 parent_id, f64 child_id) {
    if (g_active_host) {
        g_active_host->appendChild(parent_id, child_id);
    }
}

void __porf_import_dong_insert_before(f64 parent_id, f64 new_id, f64 ref_id) {
    if (g_active_host) {
        g_active_host->insertBefore(parent_id, new_id, ref_id);
    }
}

void __porf_import_dong_remove(f64 node_id) {
    if (g_active_host) {
        g_active_host->removeNode(node_id);
    }
}

void __porf_import_dong_replace_child(f64 parent_id, f64 new_id, f64 old_id) {
    if (g_active_host) {
        g_active_host->replaceChild(parent_id, new_id, old_id);
    }
}

f64 __porf_import_dong_parent(f64 node_id) {
    return g_active_host ? g_active_host->getParentId(node_id) : 0.0;
}

f64 __porf_import_dong_first_child(f64 node_id) {
    return g_active_host ? g_active_host->getFirstChildId(node_id) : 0.0;
}

f64 __porf_import_dong_next_sibling(f64 node_id) {
    return g_active_host ? g_active_host->getNextSiblingId(node_id) : 0.0;
}

f64 __porf_import_dong_clone_node(f64 node_id, f64 deep) {
    return g_active_host ? g_active_host->cloneNodeId(node_id, deep) : 0.0;
}

void __porf_import_dong_parse_html(f64 html_ptr) {
    if (g_active_host) {
        g_active_host->parseHtml(html_ptr);
    }
}

void __porf_import_dong_form_serialize(f64 form_id) {
    if (g_active_host) {
        g_active_host->formSerialize(form_id);
    }
}

void __porf_import_dong_selection_text(void) {
    if (g_active_host) {
        g_active_host->selectionText();
    }
}

void __porf_import_dong_event_type(void) {
    if (g_active_host) {
        g_active_host->eventPrepareType();
    }
}

f64 __porf_import_dong_event_target(void) {
    return g_active_host ? g_active_host->eventTarget() : 0.0;
}

void __porf_import_dong_event_key(void) {
    if (g_active_host) {
        g_active_host->eventPrepareKey();
    }
}

f64 __porf_import_dong_event_key_code(void) {
    return g_active_host ? g_active_host->eventKeyCode() : 0.0;
}

f64 __porf_import_dong_event_x(void) {
    return g_active_host ? g_active_host->eventX() : 0.0;
}

f64 __porf_import_dong_event_y(void) {
    return g_active_host ? g_active_host->eventY() : 0.0;
}

f64 __porf_import_dong_event_button(void) {
    return g_active_host ? g_active_host->eventButton() : 0.0;
}

f64 __porf_import_dong_event_modifiers(void) {
    return g_active_host ? g_active_host->eventModifiers() : 0.0;
}

void __porf_import_dong_event_value(void) {
    if (g_active_host) {
        g_active_host->eventPrepareValue();
    }
}

void __porf_import_dong_event_prevent_default(void) {
    if (g_active_host) {
        g_active_host->eventPreventDefaultFlag();
    }
}

void __porf_import_dong_event_stop_propagation(void) {
    if (g_active_host) {
        g_active_host->eventStopPropagationFlag();
    }
}

f64 __porf_import_dong_fetch_start(f64 url_ptr, f64 export_ptr) {
    return g_active_host ? g_active_host->fetchStart(url_ptr, export_ptr) : 0.0;
}

f64 __porf_import_dong_commit_fetch_start(void) {
    return g_active_host ? g_active_host->commitFetchStart() : 0.0;
}

void __porf_import_dong_fetch_abort(f64 request_id) {
    if (g_active_host) {
        g_active_host->fetchAbort(request_id);
    }
}

f64 __porf_import_dong_fetch_request_id(void) {
    return g_active_host ? g_active_host->fetchRequestId() : 0.0;
}

f64 __porf_import_dong_fetch_status(void) {
    return g_active_host ? g_active_host->fetchStatus() : 0.0;
}

f64 __porf_import_dong_fetch_ok(void) {
    return g_active_host ? g_active_host->fetchOk() : 0.0;
}

void __porf_import_dong_fetch_body(void) {
    if (g_active_host) {
        g_active_host->fetchPrepareBody();
    }
}

void __porf_import_dong_fetch_error(void) {
    if (g_active_host) {
        g_active_host->fetchPrepareError();
    }
}

void __porf_import_dong_fetch_header(f64 name_ptr) {
    if (g_active_host) {
        g_active_host->fetchPrepareHeader(name_ptr);
    }
}

void __porf_import_dong_clipboard_write(f64 text_ptr) {
    if (g_active_host) {
        g_active_host->clipboardWrite(text_ptr);
    }
}

void __porf_import_dong_clipboard_read(void) {
    if (g_active_host) {
        g_active_host->clipboardRead();
    }
}

f64 __porf_import_dong_match_media(f64 query_ptr) {
    return g_active_host ? g_active_host->matchMedia(query_ptr) : 0.0;
}

f64 __porf_import_dong_css_supports(f64 prop_ptr, f64 value_ptr) {
    return g_active_host ? g_active_host->cssSupportsProp(prop_ptr, value_ptr) : 0.0;
}

void __porf_import_dong_dialog_show(f64 node_id) {
    if (g_active_host) {
        g_active_host->dialogShow(node_id);
    }
}

void __porf_import_dong_dialog_show_modal(f64 node_id) {
    if (g_active_host) {
        g_active_host->dialogShowModal(node_id);
    }
}

void __porf_import_dong_dialog_close(f64 node_id, f64 return_value_ptr) {
    if (g_active_host) {
        g_active_host->dialogClose(node_id, return_value_ptr);
    }
}

void __porf_import_dong_dialog_return_value(f64 node_id) {
    if (g_active_host) {
        g_active_host->dialogPrepareReturnValue(node_id);
    }
}

f64 __porf_import_dong_dialog_open(f64 node_id) {
    return g_active_host ? g_active_host->dialogIsOpen(node_id) : 0.0;
}

f64 __porf_import_dong_scene_add_node(f64 config_ptr) {
    if (!g_active_host) {
        return 0.0;
    }
    auto* bindings = g_active_host->bindings();
    if (!bindings) {
        return 0.0;
    }
    return static_cast<f64>(bindings->sceneAddNode(g_active_host->readImportString(config_ptr)));
}

void __porf_import_dong_scene_remove(f64 id) {
    if (g_active_host) {
        if (auto* bindings = g_active_host->bindings()) {
            bindings->sceneRemove(static_cast<uint32_t>(id));
        }
    }
}

void __porf_import_dong_scene_set(f64 id, f64 prop_ptr, f64 value_ptr) {
    if (!g_active_host) {
        return;
    }
    if (auto* bindings = g_active_host->bindings()) {
        bindings->sceneSet(static_cast<uint32_t>(id),
                           g_active_host->readImportString(prop_ptr),
                           g_active_host->readImportString(value_ptr));
    }
}

f64 __porf_import_dong_scene_find(f64 name_ptr) {
    if (!g_active_host) {
        return -1.0;
    }
    if (auto* bindings = g_active_host->bindings()) {
        return static_cast<f64>(bindings->sceneFind(g_active_host->readImportString(name_ptr)));
    }
    return -1.0;
}

void __porf_import_dong_scene_on(f64 id, f64 type_ptr, f64 export_ptr) {
    if (!g_active_host || !g_active_host->registry()) {
        return;
    }
    if (auto* bindings = g_active_host->bindings()) {
        bindings->sceneOn(static_cast<uint32_t>(id),
                          g_active_host->readImportString(type_ptr),
                          g_active_host->readImportString(export_ptr),
                          g_active_host->registry()->activeModule());
    }
}

void __porf_import_dong_scene_clear(void) {
    if (g_active_host) {
        if (auto* bindings = g_active_host->bindings()) {
            bindings->sceneClear();
        }
    }
}

f64 __porf_import_dong_scene_count(void) {
    if (!g_active_host) {
        return 0.0;
    }
    if (auto* bindings = g_active_host->bindings()) {
        return static_cast<f64>(bindings->sceneCount());
    }
    return 0.0;
}

void __porf_import_dong_text_layout(f64 config_ptr) {
    if (!g_active_host) {
        return;
    }
    if (auto* bindings = g_active_host->bindings()) {
        g_active_host->setImportResult(bindings->textLayout(g_active_host->readImportString(config_ptr)));
    }
}

void __porf_import_dong_clear_overlay(void) {
    if (g_active_host) {
        g_active_host->clearOverlay();
    }
}

void __porf_import_dong_render_text(f64 config_ptr) {
    if (g_active_host) {
        g_active_host->renderText(config_ptr);
    }
}

void __porf_import_dong_draw_rect(f64 config_ptr) {
    if (g_active_host) {
        g_active_host->drawRect(config_ptr);
    }
}

void __porf_import_dong_draw_circle(f64 config_ptr) {
    if (g_active_host) {
        g_active_host->drawCircle(config_ptr);
    }
}

} // extern "C"

namespace dong::script {

void PorfforHost_setActiveModule(const struct dong_porf_module* mod) {
    g_active_module = mod;
    if (g_active_host && mod) {
        g_active_host->setActiveMemory(*mod->memory, mod->memory_pages);
    }
}

const struct dong_porf_module* PorfforHost_activeModule() {
    return g_active_module;
}

void PorfforHost_setActiveHost(PorfforHost* host) {
    g_active_host = host;
}

PorfforHost* PorfforHost_active() {
    return g_active_host;
}

} // namespace dong::script
