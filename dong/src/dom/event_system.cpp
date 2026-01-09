#include "event_system.hpp"
#include <algorithm>

namespace dong::dom {

EventDispatcher::EventDispatcher() = default;

EventDispatcher::~EventDispatcher() = default;

uint64_t EventDispatcher::addEventListener(DOMNodePtr target, EventType type, const EventListener& listener) {
    if (!target) return 0;

    uint64_t id = next_listener_id++;
    uintptr_t key = reinterpret_cast<uintptr_t>(target.get());
    auto& target_listeners = listeners[key];
    target_listeners[static_cast<int>(type)].push_back({id, listener});
    
    return id;
}

uint64_t EventDispatcher::addEventListener(DOMNodePtr target, const std::string& type_name, const EventListener& listener) {
    auto it = event_type_map.find(type_name);
    if (it != event_type_map.end()) {
        return addEventListener(target, it->second, listener);
    }
    return 0;
}

void EventDispatcher::removeEventListener(DOMNodePtr target, EventType type, uint64_t listener_id) {
    if (!target) return;

    uintptr_t key = reinterpret_cast<uintptr_t>(target.get());
    auto target_it = listeners.find(key);
    if (target_it == listeners.end()) return;

    auto& type_listeners = target_it->second;
    auto it = type_listeners.find(static_cast<int>(type));
    if (it == type_listeners.end()) return;

    auto& entries = it->second;
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
            [listener_id](const ListenerEntry& entry) { return entry.id == listener_id; }),
        entries.end()
    );
}

void EventDispatcher::removeEventListener(DOMNodePtr target, const std::string& type_name, uint64_t listener_id) {
    auto it = event_type_map.find(type_name);
    if (it != event_type_map.end()) {
        removeEventListener(target, it->second, listener_id);
    }
}

void EventDispatcher::dispatch(const Event& event) {
    if (!event.target) return;

    // Walk up the DOM tree and dispatch to all parents
    DOMNodePtr current = event.target;
    
    while (current && !event.stopped) {
        uintptr_t key = reinterpret_cast<uintptr_t>(current.get());
        auto it = listeners.find(key);
        if (it != listeners.end()) {
            auto& type_listeners = it->second;
            auto type_it = type_listeners.find(static_cast<int>(event.type));
            
            if (type_it != type_listeners.end()) {
                for (const auto& entry : type_it->second) {
                    if (!event.stopped) {
                        entry.callback(event);
                    }
                }
            }
        }

        current = current->getParent();
    }
}

Event EventDispatcher::createEvent(EventType type) {
    Event event;
    event.type = type;
    
    // Map type to name
    switch (type) {
        case EventType::CLICK: event.type_name = "click"; break;
        case EventType::DOUBLE_CLICK: event.type_name = "dblclick"; break;
        case EventType::MOUSE_MOVE: event.type_name = "mousemove"; break;
        case EventType::MOUSE_DOWN: event.type_name = "mousedown"; break;
        case EventType::MOUSE_UP: event.type_name = "mouseup"; break;
        case EventType::MOUSE_ENTER: event.type_name = "mouseenter"; break;
        case EventType::MOUSE_LEAVE: event.type_name = "mouseleave"; break;
        case EventType::KEY_DOWN: event.type_name = "keydown"; break;
        case EventType::KEY_UP: event.type_name = "keyup"; break;
        case EventType::KEY_PRESS: event.type_name = "keypress"; break;
        case EventType::FOCUS: event.type_name = "focus"; break;
        case EventType::BLUR: event.type_name = "blur"; break;
        case EventType::CHANGE: event.type_name = "change"; break;
        case EventType::INPUT: event.type_name = "input"; break;
        case EventType::SUBMIT: event.type_name = "submit"; break;
        case EventType::CUSTOM: event.type_name = "custom"; break;
    }
    
    return event;
}

Event EventDispatcher::createMouseEvent(EventType type, int32_t x, int32_t y, int32_t button) {
    Event event = createEvent(type);
    event.mouse_x = x;
    event.mouse_y = y;
    event.mouse_button = button;
    return event;
}

Event EventDispatcher::createKeyEvent(EventType type, uint32_t key_code) {
    Event event = createEvent(type);
    event.key_code = key_code;
    return event;
}

} // namespace dong::dom
