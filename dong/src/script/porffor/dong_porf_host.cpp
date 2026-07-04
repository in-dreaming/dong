#include "dong_porf_host.hpp"

#include "registry.h"
#include "dong_porf_runtime.h"
#include "../../core/log.h"
#include "porffor_script_registry.hpp"
#include "js_bindings_porffor.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace dong::script {
namespace {

PorfforHost* g_host = nullptr;
PorfforScriptRegistry* g_registry = nullptr;
thread_local const struct dong_porf_module* g_active_module = nullptr;

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

std::string PorfforHost::readByteString(double ptr) const {
    if (!memory_ || ptr < 0) {
        return {};
    }
    const auto offset = static_cast<size_t>(ptr);
    const u32 len = *reinterpret_cast<u32*>(memory_ + offset);
    std::string out;
    out.reserve(len);
    for (u32 i = 0; i < len; ++i) {
        out.push_back(static_cast<char>(memory_[offset + 4 + i]));
    }
    return out;
}

double PorfforHost::makeByteString(const std::string& s) {
    if (!memory_) {
        return 0;
    }
    // Simple bump allocator in module memory (after page 0 guard).
    static size_t bump = 4096;
    const size_t offset = bump;
    const u32 len = static_cast<u32>(s.size());
    if (memory_pages_) {
        const size_t need = offset + 4 + s.size();
        const size_t cap = static_cast<size_t>(*memory_pages_) * 65536;
        if (need > cap && memory_pages_) {
            *memory_pages_ = static_cast<unsigned int>((need + 65535) / 65536);
            memory_ = static_cast<char*>(std::realloc(memory_, static_cast<size_t>(*memory_pages_) * 65536));
        }
    }
    *reinterpret_cast<u32*>(memory_ + offset) = len;
    for (u32 i = 0; i < len; ++i) {
        memory_[offset + 4 + i] = s[i];
    }
    bump = offset + 4 + s.size();
    return static_cast<double>(offset);
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

double PorfforHost::domGetTextContent(double node_id) {
    auto* bindings = activeBindings();
    if (!bindings) {
        return 0;
    }
    return makeByteString(bindings->getNodeTextContent(static_cast<uint64_t>(node_id)));
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

// ---------------------------------------------------------------------------
// C ABI: Porffor import stubs (linked from generated module code).
// ---------------------------------------------------------------------------
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

f64 __porf_import_dong_dom_get_textContent(f64 node_id) {
    return g_host ? g_host->domGetTextContent(node_id) : 0.0;
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
