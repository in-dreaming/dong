#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct dong_porf_module;

namespace dong::script {

class JSBindings;
class PorfforScriptRegistry;

class PorfforHost {
public:
    PorfforHost();

    void setBindings(JSBindings* bindings);
    JSBindings* bindings() const;

    void setActiveMemory(char* memory, unsigned int* memory_pages);
    char* activeMemory() const { return memory_; }
    unsigned int* activeMemoryPages() const { return memory_pages_; }

    void processTimers(double now_ms);

    void printString(double str_ptr);
    void benchLog(double str_ptr);
    double timeNow() const;
    double domGetElementById(double id_ptr);
    void domSetTextContent(double node_id, double text_ptr);
    double domGetTextContent(double node_id);
    void domAddEventListener(double node_id, double type_ptr, double handler_ptr);
    double timerSetTimeout(double handler_ptr, double delay_ms);

    void stage0(double v);
    void stage1(double v);
    void stage2(double v);
    void commitSetTextContent();
    void commitAddEventListener();
    double commitSetTimeout();

private:
    JSBindings* bindings_ = nullptr;
    char* memory_ = nullptr;
    unsigned int* memory_pages_ = nullptr;

    double stage_0_ = 0;
    double stage_1_ = 0;
    double stage_2_ = 0;

    std::string readByteString(double ptr) const;
    double makeByteString(const std::string& s);

    struct TimerTask {
        double fire_at_ms = 0;
        std::string module_name;
        std::string export_name;
    };
    std::vector<TimerTask> timers_;
    double start_ms_ = 0;
};

void PorfforHost_setGlobals(PorfforHost* host, PorfforScriptRegistry* registry);
void PorfforHost_setActiveModule(const struct dong_porf_module* mod);
const struct dong_porf_module* PorfforHost_activeModule();

} // namespace dong::script
