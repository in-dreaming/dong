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
