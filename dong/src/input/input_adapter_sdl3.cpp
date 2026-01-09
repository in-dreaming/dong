#include "input_adapter_sdl3.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>

namespace dong::input {

SDL3InputAdapter::SDL3InputAdapter(SDL_Window* window)
    : window_(window) {
}

SDL3InputAdapter::~SDL3InputAdapter() {
    if (text_input_active_ && window_) {
        SDL_StopTextInput(window_);
    }
}

void SDL3InputAdapter::setEventCallback(InputEventCallback callback) {
    callback_ = std::move(callback);
}

MouseButton SDL3InputAdapter::convertMouseButton(uint8_t sdl_button) {
    switch (sdl_button) {
        case SDL_BUTTON_LEFT:   return MouseButton::Left;
        case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
        case SDL_BUTTON_RIGHT:  return MouseButton::Right;
        case SDL_BUTTON_X1:     return MouseButton::X1;
        case SDL_BUTTON_X2:     return MouseButton::X2;
        default:                return MouseButton::Left;
    }
}

KeyModifiers SDL3InputAdapter::getCurrentModifiers() const {
    KeyModifiers mods;
    SDL_Keymod sdl_mods = SDL_GetModState();
    mods.shift = (sdl_mods & SDL_KMOD_SHIFT) != 0;
    mods.ctrl = (sdl_mods & SDL_KMOD_CTRL) != 0;
    mods.alt = (sdl_mods & SDL_KMOD_ALT) != 0;
    mods.meta = (sdl_mods & SDL_KMOD_GUI) != 0;
    mods.caps_lock = (sdl_mods & SDL_KMOD_CAPS) != 0;
    mods.num_lock = (sdl_mods & SDL_KMOD_NUM) != 0;
    return mods;
}

bool SDL3InputAdapter::pollEvents() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_QUIT:
                SDL_Log("[SDL3InputAdapter] SDL_EVENT_QUIT received");
                // 发送窗口关闭事件
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::Window;
                    e.window.type = WindowEvent::Type::CloseRequested;
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                return false;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                SDL_Log("[SDL3InputAdapter] SDL_EVENT_WINDOW_CLOSE_REQUESTED received");
                // 发送窗口关闭事件
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::Window;
                    e.window.type = WindowEvent::Type::CloseRequested;
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                return false;

            case SDL_EVENT_WINDOW_RESIZED:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::Window;
                    e.window.type = WindowEvent::Type::Resized;
                    e.window.data1 = ev.window.data1;  // width
                    e.window.data2 = ev.window.data2;  // height
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::Window;
                    e.window.type = WindowEvent::Type::FocusGained;
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::Window;
                    e.window.type = WindowEvent::Type::FocusLost;
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::MouseMove;
                    e.mouse_move.x = static_cast<int32_t>(ev.motion.x);
                    e.mouse_move.y = static_cast<int32_t>(ev.motion.y);
                    e.mouse_move.delta_x = static_cast<int32_t>(ev.motion.x) - last_mouse_x_;
                    e.mouse_move.delta_y = static_cast<int32_t>(ev.motion.y) - last_mouse_y_;
                    e.mouse_move.modifiers = getCurrentModifiers();
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                last_mouse_x_ = static_cast<int32_t>(ev.motion.x);
                last_mouse_y_ = static_cast<int32_t>(ev.motion.y);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::MouseButton;
                    e.mouse_button.x = static_cast<int32_t>(ev.button.x);
                    e.mouse_button.y = static_cast<int32_t>(ev.button.y);
                    e.mouse_button.button = convertMouseButton(ev.button.button);
                    e.mouse_button.pressed = (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                    e.mouse_button.click_count = ev.button.clicks;
                    e.mouse_button.modifiers = getCurrentModifiers();
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::MouseWheel;
                    // SDL3 提供鼠标位置
                    e.mouse_wheel.x = static_cast<int32_t>(ev.wheel.mouse_x);
                    e.mouse_wheel.y = static_cast<int32_t>(ev.wheel.mouse_y);
                    // SDL3 滚轮方向：正值向上/向右
                    // dong 约定：delta_y 正值向下（与滚动内容方向一致）
                    e.mouse_wheel.delta_x = ev.wheel.x;
                    e.mouse_wheel.delta_y = -ev.wheel.y;  // 取反以匹配滚动语义
                    e.mouse_wheel.precise = false;  // SDL3 统一为精确滚动
                    e.mouse_wheel.modifiers = getCurrentModifiers();
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::Key;
                    new (&e.key) KeyEvent();
                    e.key.key_code = ev.key.key;
                    e.key.scan_code = ev.key.scancode;
                    e.key.pressed = (ev.type == SDL_EVENT_KEY_DOWN);
                    e.key.repeat = ev.key.repeat;
                    e.key.modifiers = getCurrentModifiers();
                    // 获取键名
                    const char* name = SDL_GetKeyName(ev.key.key);
                    if (name) {
                        e.key.key_name = name;
                    }
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            case SDL_EVENT_TEXT_INPUT:
                SDL_Log("[SDL3InputAdapter] SDL_EVENT_TEXT_INPUT: text='%s'", ev.text.text);
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::TextInput;
                    new (&e.text_input) TextInputEvent();
                    e.text_input.text = ev.text.text;
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            case SDL_EVENT_TEXT_EDITING:
                if (callback_) {
                    InputEvent e;
                    e.type = InputEventType::TextEditing;
                    new (&e.text_editing) TextEditingEvent();
                    e.text_editing.text = ev.edit.text;
                    e.text_editing.cursor = ev.edit.start;
                    e.text_editing.selection_length = ev.edit.length;
                    e.timestamp = ev.common.timestamp;
                    callback_(std::move(e));
                }
                break;

            default:
                break;
        }
    }
    return true;
}

void SDL3InputAdapter::startTextInput() {
    if (window_ && !text_input_active_) {
        SDL_StartTextInput(window_);
        text_input_active_ = true;
    }
}

void SDL3InputAdapter::stopTextInput() {
    if (window_ && text_input_active_) {
        SDL_StopTextInput(window_);
        text_input_active_ = false;
    }
}

bool SDL3InputAdapter::isTextInputActive() const {
    return text_input_active_;
}

void SDL3InputAdapter::setTextInputRect(int32_t x, int32_t y, int32_t w, int32_t h) {
    if (window_) {
        SDL_Rect rect = { x, y, w, h };
        SDL_SetTextInputArea(window_, &rect, 0);
    }
}

void SDL3InputAdapter::getMousePosition(int32_t* x, int32_t* y) const {
    float fx, fy;
    SDL_GetMouseState(&fx, &fy);
    if (x) *x = static_cast<int32_t>(fx);
    if (y) *y = static_cast<int32_t>(fy);
}

KeyModifiers SDL3InputAdapter::getKeyModifiers() const {
    return getCurrentModifiers();
}

void SDL3InputAdapter::setCursor(const std::string& cursor_name) {
    SDL_SystemCursor sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;
    
    if (cursor_name == "pointer" || cursor_name == "hand") {
        sdl_cursor = SDL_SYSTEM_CURSOR_POINTER;
    } else if (cursor_name == "text" || cursor_name == "ibeam") {
        sdl_cursor = SDL_SYSTEM_CURSOR_TEXT;
    } else if (cursor_name == "move" || cursor_name == "all-scroll") {
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;
    } else if (cursor_name == "wait") {
        sdl_cursor = SDL_SYSTEM_CURSOR_WAIT;
    } else if (cursor_name == "progress") {
        sdl_cursor = SDL_SYSTEM_CURSOR_PROGRESS;
    } else if (cursor_name == "crosshair") {
        sdl_cursor = SDL_SYSTEM_CURSOR_CROSSHAIR;
    } else if (cursor_name == "not-allowed" || cursor_name == "no-drop") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    } else if (cursor_name == "n-resize" || cursor_name == "s-resize" || cursor_name == "ns-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NS_RESIZE;
    } else if (cursor_name == "e-resize" || cursor_name == "w-resize" || cursor_name == "ew-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_EW_RESIZE;
    } else if (cursor_name == "ne-resize" || cursor_name == "sw-resize" || cursor_name == "nesw-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NESW_RESIZE;
    } else if (cursor_name == "nw-resize" || cursor_name == "se-resize" || cursor_name == "nwse-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    } else if (cursor_name == "grab") {
        // SDL3 没有 grab 光标，使用 move 代替
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;
    } else if (cursor_name == "grabbing") {
        // SDL3 没有 grabbing 光标，使用 move 代替
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;
    } else if (cursor_name == "help") {
        // SDL3 没有 help 光标，使用默认
        sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;
    } else if (cursor_name == "none") {
        // 隐藏光标
        SDL_HideCursor();
        return;
    } else {
        // auto, default 或其他未知值
        sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;
    }
    
    SDL_ShowCursor();
    SDL_Cursor* cursor = SDL_CreateSystemCursor(sdl_cursor);
    if (cursor) {
        SDL_SetCursor(cursor);
    }
}

// 工厂函数
InputAdapterPtr createSDL3InputAdapter(SDL_Window* window) {
    return std::make_unique<SDL3InputAdapter>(window);
}

} // namespace dong::input
