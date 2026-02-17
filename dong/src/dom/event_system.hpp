#pragma once

#include "dom/dom_node.hpp"
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

namespace dong::dom {

// Event types
enum class EventType {
    CLICK,
    DOUBLE_CLICK,
    MOUSE_MOVE,
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_ENTER,
    MOUSE_LEAVE,
    // Pointer events (unified mouse/touch/pen)
    POINTER_DOWN,
    POINTER_UP,
    POINTER_MOVE,
    POINTER_ENTER,
    POINTER_LEAVE,
    POINTER_OVER,
    POINTER_OUT,
    POINTER_CANCEL,
    // Keyboard events
    KEY_DOWN,
    KEY_UP,
    KEY_PRESS,
    FOCUS,
    BLUR,
    CHANGE,
    INPUT,
    SUBMIT,
    SCROLL,
    RESIZE,
    WHEEL,
    DOM_CONTENT_LOADED,
    CUSTOM
};

// Pointer type for unified pointer events
enum class PointerType {
    MOUSE,
    TOUCH,
    PEN
};

// Event object
struct Event {
    EventType type;
    std::string type_name;
    DOMNodePtr target;
    DOMNodePtr current_target;
    
    // Mouse/Pointer event data
    int32_t mouse_x = 0;
    int32_t mouse_y = 0;
    int32_t mouse_button = 0;
    
    // Pointer-specific data
    int32_t pointer_id = 0;          // Unique pointer identifier
    PointerType pointer_type = PointerType::MOUSE;
    float pressure = 0.0f;           // Pressure (0.0-1.0)
    float tilt_x = 0.0f;             // Tilt angle X
    float tilt_y = 0.0f;             // Tilt angle Y
    int32_t width = 1;               // Contact width
    int32_t height = 1;              // Contact height
    bool is_primary = true;          // Is primary pointer
    
    // Keyboard event data
    uint32_t key_code = 0;
    std::string key_name;
    std::string key_string;      // Standard 'key' property (e.g. "Enter", "a")
    std::string code_string;     // Standard 'code' property (e.g. "Enter", "KeyA")

    // Modifier key state
    bool alt_key = false;
    bool ctrl_key = false;
    bool shift_key = false;
    bool meta_key = false;

    // Wheel event data
    float delta_x = 0.0f;
    float delta_y = 0.0f;
    float delta_z = 0.0f;

    // Input event data
    std::string input_data;      // The inserted text for 'input' events
    
    // Custom data
    std::unordered_map<std::string, std::string> data;
    
    bool stopped = false;
    bool prevented = false;

    void stopPropagation() { stopped = true; }
    void preventDefault() { prevented = true; }
};

// Event listener callback
using EventListener = std::function<void(const Event&)>;

// Event dispatcher
class EventDispatcher {
public:
    EventDispatcher();
    ~EventDispatcher();

    // Register event listener
    uint64_t addEventListener(DOMNodePtr target, EventType type, const EventListener& listener);
    uint64_t addEventListener(DOMNodePtr target, const std::string& type_name, const EventListener& listener);

    // Unregister event listener
    void removeEventListener(DOMNodePtr target, EventType type, uint64_t listener_id);
    void removeEventListener(DOMNodePtr target, const std::string& type_name, uint64_t listener_id);

    // Dispatch event
    void dispatch(const Event& event);

    // Create and dispatch event
    Event createEvent(EventType type);
    Event createMouseEvent(EventType type, int32_t x, int32_t y, int32_t button = 0);
    Event createKeyEvent(EventType type, uint32_t key_code);
    Event createPointerEvent(EventType type, int32_t x, int32_t y, 
                             int32_t pointer_id = 0, PointerType pointer_type = PointerType::MOUSE,
                             int32_t button = 0, float pressure = 0.5f);

private:
    struct ListenerEntry {
        uint64_t id;
        EventListener callback;
    };

    // Use raw pointers as keys since DOMNodePtr is a shared_ptr
    std::unordered_map<uintptr_t, std::unordered_map<int, std::vector<ListenerEntry>>> listeners;
    uint64_t next_listener_id = 1;

    // Event handler registry
    std::unordered_map<std::string, EventType> event_type_map = {
        {"click", EventType::CLICK},
        {"dblclick", EventType::DOUBLE_CLICK},
        {"mousemove", EventType::MOUSE_MOVE},
        {"mousedown", EventType::MOUSE_DOWN},
        {"mouseup", EventType::MOUSE_UP},
        {"mouseenter", EventType::MOUSE_ENTER},
        {"mouseleave", EventType::MOUSE_LEAVE},
        // Pointer events
        {"pointerdown", EventType::POINTER_DOWN},
        {"pointerup", EventType::POINTER_UP},
        {"pointermove", EventType::POINTER_MOVE},
        {"pointerenter", EventType::POINTER_ENTER},
        {"pointerleave", EventType::POINTER_LEAVE},
        {"pointerover", EventType::POINTER_OVER},
        {"pointerout", EventType::POINTER_OUT},
        {"pointercancel", EventType::POINTER_CANCEL},
        // Keyboard events
        {"keydown", EventType::KEY_DOWN},
        {"keyup", EventType::KEY_UP},
        {"keypress", EventType::KEY_PRESS},
        {"focus", EventType::FOCUS},
        {"blur", EventType::BLUR},
        {"change", EventType::CHANGE},
        {"input", EventType::INPUT},
        {"submit", EventType::SUBMIT},
        {"scroll", EventType::SCROLL},
        {"resize", EventType::RESIZE},
        {"wheel", EventType::WHEEL},
        {"DOMContentLoaded", EventType::DOM_CONTENT_LOADED},
    };
};

} // namespace dong::dom
