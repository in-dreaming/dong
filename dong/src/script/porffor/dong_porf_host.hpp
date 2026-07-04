#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../dom/event_system.hpp"

struct dong_porf_module;

namespace dong::script {

class JSBindings;
class PorfforScriptRegistry;

class PorfforHost {
public:
    static constexpr size_t kMaxImportStringBytes = 4 * 1024 * 1024;

    PorfforHost();

    void setBindings(JSBindings* bindings);
    JSBindings* bindings() const;

    void setActiveMemory(char* memory, unsigned int* memory_pages);
    char* activeMemory() const { return memory_; }
    unsigned int* activeMemoryPages() const { return memory_pages_; }

    void processTimers(double now_ms);
    void processAnimationFrames(double timestamp_ms);

    void printString(double str_ptr);
    void benchLog(double str_ptr);
    double timeNow() const;
    double domGetElementById(double id_ptr);
    void domSetTextContent(double node_id, double text_ptr);
    void domPrepareTextContent(double node_id);
    void domAddEventListener(double node_id, double type_ptr, double handler_ptr);
    double timerSetTimeout(double handler_ptr, double delay_ms);
    double timerSetInterval(double handler_ptr, double interval_ms);
    void timerClear(double timer_id);
    double requestAnimationFrame(double handler_ptr);
    void cancelAnimationFrame(double raf_id);
    double rafTimestamp() const;

    double strLen() const;
    double strRead(double dest_ptr, double max_len);
    double strByteAt(double index) const;

    void stage0(double v);
    void stage1(double v);
    void stage2(double v);
    void commitSetTextContent();
    void commitAddEventListener();
    double commitSetTimeout();
    double commitSetInterval();
    double commitRequestAnimationFrame();

    void getValue(double node_id);
    void setValue(double node_id, double value_ptr);
    double getChecked(double node_id) const;
    void setChecked(double node_id, double checked);
    double getDisabled(double node_id) const;
    void setDisabled(double node_id, double disabled);
    void getAttribute(double node_id, double name_ptr);
    void setAttribute(double node_id, double name_ptr, double value_ptr);
    void removeAttribute(double node_id, double name_ptr);
    void setInnerHTML(double node_id, double html_ptr);

    double querySelector(double root_id, double selector_ptr) const;
    void querySelectorAll(double root_id, double selector_ptr);
    void getElementsByTagName(double root_id, double tag_ptr);

    void classAdd(double node_id, double cls_ptr);
    void classRemove(double node_id, double cls_ptr);
    double classToggle(double node_id, double cls_ptr) const;
    double classContains(double node_id, double cls_ptr) const;

    void styleSet(double node_id, double prop_ptr, double value_ptr);
    void styleGet(double node_id, double prop_ptr);
    void computedStyleGet(double node_id, double prop_ptr);

    void getRect(double node_id);
    double getMetric(double node_id, double metric_id) const;
    double getScrollTop(double node_id) const;
    void setScrollTop(double node_id, double value);
    double getScrollLeft(double node_id) const;
    void setScrollLeft(double node_id, double value);

    void focusNode(double node_id);
    void blurNode(double node_id);
    void clickNode(double node_id);
    double matchesSelector(double node_id, double selector_ptr) const;
    double closestSelector(double node_id, double selector_ptr) const;

    double createElement(double tag_ptr);
    double createTextNode(double text_ptr);
    void appendChild(double parent_id, double child_id);
    void insertBefore(double parent_id, double new_id, double ref_id);
    void removeNode(double node_id);
    void replaceChild(double parent_id, double new_id, double old_id);
    double getParentId(double node_id) const;
    double getFirstChildId(double node_id) const;
    double getNextSiblingId(double node_id) const;
    double cloneNodeId(double node_id, double deep) const;

    void pushEventSlot(const dom::Event* dom_event, uint64_t target_node_id, const std::string& type);
    void popEventSlot();
    bool eventPreventDefault() const;
    bool eventStopPropagation() const;

    void eventPrepareType();
    double eventTarget() const;
    void eventPrepareKey();
    double eventKeyCode() const;
    double eventX() const;
    double eventY() const;
    double eventButton() const;
    double eventModifiers() const;
    void eventPrepareValue();
    void eventPreventDefaultFlag();
    void eventStopPropagationFlag();

    void pushResultSlot();
    void popResultSlot();

    void setRegistry(PorfforScriptRegistry* registry) { registry_ = registry; }
    PorfforScriptRegistry* registry() const { return registry_; }

private:
    JSBindings* bindings_ = nullptr;
    PorfforScriptRegistry* registry_ = nullptr;
    char* memory_ = nullptr;
    unsigned int* memory_pages_ = nullptr;

    double stage_0_ = 0;
    double stage_1_ = 0;
    double stage_2_ = 0;

    std::string result_slot_;
    std::vector<std::string> result_slot_stack_;

    struct EventSlot {
        std::string type;
        uint64_t target_node_id = 0;
        std::string key;
        uint32_t key_code = 0;
        double x = 0;
        double y = 0;
        int32_t button = 0;
        int32_t modifiers = 0;
        std::string value;
        bool prevent_default = false;
        bool stop_propagation = false;
        const dom::Event* dom_event = nullptr;
    };
    EventSlot event_slot_;
    std::vector<EventSlot> event_slot_stack_;

    size_t memoryCapacityBytes() const;
    std::string readByteString(double ptr) const;
    void setResultString(std::string s);

    struct TimerTask {
        double fire_at_ms = 0;
        double interval_ms = -1.0;
        std::string module_name;
        std::string export_name;
    };
    struct RafTask {
        uint64_t id = 0;
        std::string module_name;
        std::string export_name;
    };

    std::unordered_map<uint64_t, TimerTask> timers_;
    uint64_t next_timer_id_ = 1;
    std::vector<RafTask> raf_queue_;
    uint64_t next_raf_id_ = 1;
    double raf_timestamp_ = 0.0;
    double start_ms_ = 0;

    uint64_t scheduleTimer(double handler_ptr, double delay_ms, double interval_ms);
    void invokeExport(const std::string& module_name, const std::string& export_name,
                      double timestamp_ms, bool has_timestamp);
};

void PorfforHost_setActiveModule(const struct dong_porf_module* mod);
const struct dong_porf_module* PorfforHost_activeModule();
void PorfforHost_setActiveHost(PorfforHost* host);
PorfforHost* PorfforHost_active();

} // namespace dong::script
