#pragma once

#include "dom_node.hpp"
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
    KEY_DOWN,
    KEY_UP,
    KEY_PRESS,
    FOCUS,
    BLUR,
    CHANGE,
    INPUT,
    SUBMIT,
    CUSTOM
};

// Event object
struct Event {
    EventType type;
    std::string type_name;
    DOMNodePtr target;
    DOMNodePtr current_target;
    
    // Mouse event data
    int32_t mouse_x = 0;
    int32_t mouse_y = 0;
    int32_t mouse_button = 0;
    
    // Keyboard event data
    uint32_t key_code = 0;
    std::string key_name;
    
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
        {"keydown", EventType::KEY_DOWN},
        {"keyup", EventType::KEY_UP},
        {"keypress", EventType::KEY_PRESS},
        {"focus", EventType::FOCUS},
        {"blur", EventType::BLUR},
        {"change", EventType::CHANGE},
        {"input", EventType::INPUT},
        {"submit", EventType::SUBMIT},
    };
};

} // namespace dong::dom
