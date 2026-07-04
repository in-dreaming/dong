#include "dong_porf_host.hpp"

#include "registry.h"
#include "dong_porf_runtime.h"
#include "../../core/log.h"
#include "porffor_script_registry.hpp"
#include "js_bindings_porffor.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <array>

namespace dong::script {
namespace {

PorfforHost* g_host = nullptr;
PorfforScriptRegistry* g_registry = nullptr;
thread_local const struct dong_porf_module* g_active_module = nullptr;
std::array<double, 256> g_state_nums{};

JSBindings* activeBindings() {
    return g_host ? g_host->bindings() : nullptr;
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
    bindings->registerExportHandler(static_cast<uint64_t>(node_id), type, export_name);
}

double PorfforHost::timerSetTimeout(double handler_ptr, double delay_ms) {
    TimerTask task;
    task.export_name = readByteString(handler_ptr);
    task.fire_at_ms = timeNow() + delay_ms;
    if (g_registry) {
        task.module_name = g_registry->activeModule();
    }
    const double id = static_cast<double>(timers_.size() + 1);
    timers_.push_back(std::move(task));
    return id;
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

void PorfforHost::processTimers(double now_ms) {
    if (timers_.empty() || !g_registry) {
        return;
    }
    std::vector<TimerTask> remaining;
    remaining.reserve(timers_.size());
    for (auto& t : timers_) {
        if (t.fire_at_ms <= now_ms) {
            g_registry->callExport(t.module_name, t.export_name);
        } else {
            remaining.push_back(std::move(t));
        }
    }
    timers_ = std::move(remaining);
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
using dong::script::g_host;
using dong::script::g_registry;

extern "C" {

void __porf_import_dong_print(f64 arg) {
    if (g_host) {
        g_host->printString(arg);
    }
}

void __porf_import_dong_bench_log(f64 arg) {
    if (g_host) {
        g_host->benchLog(arg);
    }
}

f64 __porf_import_dong_time_now(void) {
    return g_host ? g_host->timeNow() : 0.0;
}

f64 __porf_import_dong_dom_getElementById(f64 id_ptr) {
    return g_host ? g_host->domGetElementById(id_ptr) : 0.0;
}

void __porf_import_dong_dom_set_textContent(f64 node_id, f64 text_ptr) {
    if (g_host) {
        g_host->domSetTextContent(node_id, text_ptr);
    }
}

void __porf_import_dong_dom_get_textContent(f64 node_id) {
    if (g_host) {
        g_host->domPrepareTextContent(node_id);
    }
}

f64 __porf_import_dong_str_len(void) {
    return g_host ? g_host->strLen() : 0.0;
}

f64 __porf_import_dong_str_read(f64 dest_ptr, f64 max_len) {
    return g_host ? g_host->strRead(dest_ptr, max_len) : 0.0;
}

f64 __porf_import_dong_str_byte_at(f64 index) {
    return g_host ? g_host->strByteAt(index) : -1.0;
}

void __porf_import_dong_dom_addEventListener(f64 node_id, f64 type_ptr, f64 handler_ptr) {
    if (g_host) {
        g_host->domAddEventListener(node_id, type_ptr, handler_ptr);
    }
}

f64 __porf_import_dong_timer_setTimeout(f64 handler_ptr, f64 delay_ms) {
    return g_host ? g_host->timerSetTimeout(handler_ptr, delay_ms) : 0.0;
}

void __porf_import_dong_stage_0(f64 v) {
    if (g_host) {
        g_host->stage0(v);
    }
}

void __porf_import_dong_stage_1(f64 v) {
    if (g_host) {
        g_host->stage1(v);
    }
}

void __porf_import_dong_stage_2(f64 v) {
    if (g_host) {
        g_host->stage2(v);
    }
}

void __porf_import_dong_commit_set_textContent(void) {
    if (g_host) {
        g_host->commitSetTextContent();
    }
}

void __porf_import_dong_commit_addEventListener(void) {
    if (g_host) {
        g_host->commitAddEventListener();
    }
}

f64 __porf_import_dong_commit_setTimeout(void) {
    return g_host ? g_host->commitSetTimeout() : 0.0;
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
    if (g_host) {
        g_host->getValue(node_id);
    }
}

void __porf_import_dong_set_value(f64 node_id, f64 value_ptr) {
    if (g_host) {
        g_host->setValue(node_id, value_ptr);
    }
}

f64 __porf_import_dong_get_checked(f64 node_id) {
    return g_host ? g_host->getChecked(node_id) : 0.0;
}

void __porf_import_dong_set_checked(f64 node_id, f64 checked) {
    if (g_host) {
        g_host->setChecked(node_id, checked);
    }
}

f64 __porf_import_dong_get_disabled(f64 node_id) {
    return g_host ? g_host->getDisabled(node_id) : 0.0;
}

void __porf_import_dong_set_disabled(f64 node_id, f64 disabled) {
    if (g_host) {
        g_host->setDisabled(node_id, disabled);
    }
}

void __porf_import_dong_get_attribute(f64 node_id, f64 name_ptr) {
    if (g_host) {
        g_host->getAttribute(node_id, name_ptr);
    }
}

void __porf_import_dong_set_attribute(f64 node_id, f64 name_ptr, f64 value_ptr) {
    if (g_host) {
        g_host->setAttribute(node_id, name_ptr, value_ptr);
    }
}

void __porf_import_dong_remove_attribute(f64 node_id, f64 name_ptr) {
    if (g_host) {
        g_host->removeAttribute(node_id, name_ptr);
    }
}

void __porf_import_dong_set_inner_html(f64 node_id, f64 html_ptr) {
    if (g_host) {
        g_host->setInnerHTML(node_id, html_ptr);
    }
}

f64 __porf_import_dong_query_selector(f64 root_id, f64 selector_ptr) {
    return g_host ? g_host->querySelector(root_id, selector_ptr) : 0.0;
}

void __porf_import_dong_query_selector_all(f64 root_id, f64 selector_ptr) {
    if (g_host) {
        g_host->querySelectorAll(root_id, selector_ptr);
    }
}

void __porf_import_dong_get_elements_by_tag_name(f64 root_id, f64 tag_ptr) {
    if (g_host) {
        g_host->getElementsByTagName(root_id, tag_ptr);
    }
}

void __porf_import_dong_class_add(f64 node_id, f64 cls_ptr) {
    if (g_host) {
        g_host->classAdd(node_id, cls_ptr);
    }
}

void __porf_import_dong_class_remove(f64 node_id, f64 cls_ptr) {
    if (g_host) {
        g_host->classRemove(node_id, cls_ptr);
    }
}

f64 __porf_import_dong_class_toggle(f64 node_id, f64 cls_ptr) {
    return g_host ? g_host->classToggle(node_id, cls_ptr) : 0.0;
}

f64 __porf_import_dong_class_contains(f64 node_id, f64 cls_ptr) {
    return g_host ? g_host->classContains(node_id, cls_ptr) : 0.0;
}

void __porf_import_dong_style_set(f64 node_id, f64 prop_ptr, f64 value_ptr) {
    if (g_host) {
        g_host->styleSet(node_id, prop_ptr, value_ptr);
    }
}

void __porf_import_dong_style_get(f64 node_id, f64 prop_ptr) {
    if (g_host) {
        g_host->styleGet(node_id, prop_ptr);
    }
}

void __porf_import_dong_computed_style_get(f64 node_id, f64 prop_ptr) {
    if (g_host) {
        g_host->computedStyleGet(node_id, prop_ptr);
    }
}

void __porf_import_dong_get_rect(f64 node_id) {
    if (g_host) {
        g_host->getRect(node_id);
    }
}

f64 __porf_import_dong_get_metric(f64 node_id, f64 metric_id) {
    return g_host ? g_host->getMetric(node_id, metric_id) : 0.0;
}

f64 __porf_import_dong_get_scroll_top(f64 node_id) {
    return g_host ? g_host->getScrollTop(node_id) : 0.0;
}

void __porf_import_dong_set_scroll_top(f64 node_id, f64 value) {
    if (g_host) {
        g_host->setScrollTop(node_id, value);
    }
}

f64 __porf_import_dong_get_scroll_left(f64 node_id) {
    return g_host ? g_host->getScrollLeft(node_id) : 0.0;
}

void __porf_import_dong_set_scroll_left(f64 node_id, f64 value) {
    if (g_host) {
        g_host->setScrollLeft(node_id, value);
    }
}

void __porf_import_dong_focus(f64 node_id) {
    if (g_host) {
        g_host->focusNode(node_id);
    }
}

void __porf_import_dong_blur(f64 node_id) {
    if (g_host) {
        g_host->blurNode(node_id);
    }
}

void __porf_import_dong_click(f64 node_id) {
    if (g_host) {
        g_host->clickNode(node_id);
    }
}

f64 __porf_import_dong_matches(f64 node_id, f64 selector_ptr) {
    return g_host ? g_host->matchesSelector(node_id, selector_ptr) : 0.0;
}

f64 __porf_import_dong_closest(f64 node_id, f64 selector_ptr) {
    return g_host ? g_host->closestSelector(node_id, selector_ptr) : 0.0;
}

} // extern "C"

namespace dong::script {

void PorfforHost_setGlobals(PorfforHost* host, PorfforScriptRegistry* registry) {
    g_host = host;
    g_registry = registry;
}

void PorfforHost_setActiveModule(const struct dong_porf_module* mod) {
    g_active_module = mod;
    if (g_host && mod) {
        g_host->setActiveMemory(*mod->memory, mod->memory_pages);
    }
}

const struct dong_porf_module* PorfforHost_activeModule() {
    return g_active_module;
}

} // namespace dong::script
