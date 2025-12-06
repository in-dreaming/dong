#pragma once

#include <cstdint>
#include <string>

namespace dong::input {

// 平台无关的鼠标按钮定义
enum class MouseButton : int32_t {
    Left = 0,
    Middle = 1,
    Right = 2,
    X1 = 3,
    X2 = 4
};

// 平台无关的修饰键状态
struct KeyModifiers {
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool meta = false;  // Cmd on macOS, Win on Windows
    bool caps_lock = false;
    bool num_lock = false;
};

// 鼠标移动事件
struct MouseMoveEvent {
    int32_t x = 0;          // 相对于 View 的 x 坐标
    int32_t y = 0;          // 相对于 View 的 y 坐标
    int32_t delta_x = 0;    // 相对上次的 x 偏移
    int32_t delta_y = 0;    // 相对上次的 y 偏移
    KeyModifiers modifiers;
};

// 鼠标按钮事件
struct MouseButtonEvent {
    int32_t x = 0;
    int32_t y = 0;
    MouseButton button = MouseButton::Left;
    bool pressed = false;   // true: down, false: up
    int32_t click_count = 1; // 1: single, 2: double, etc.
    KeyModifiers modifiers;
};

// 鼠标滚轮事件
struct MouseWheelEvent {
    int32_t x = 0;          // 鼠标位置
    int32_t y = 0;
    float delta_x = 0.0f;   // 水平滚动量（正值向右）
    float delta_y = 0.0f;   // 垂直滚动量（正值向下，与 SDL 一致）
    bool precise = false;   // 是否为精确滚动（触控板）
    KeyModifiers modifiers;
};

// 键盘事件
struct KeyEvent {
    uint32_t key_code = 0;      // 平台无关的虚拟键码（使用 SDL scancode 作为基准）
    uint32_t scan_code = 0;     // 物理键码
    bool pressed = false;       // true: down, false: up
    bool repeat = false;        // 是否为按键重复
    KeyModifiers modifiers;
    std::string key_name;       // 可读的键名（如 "Enter", "A"）
};

// 文本输入事件（IME 组合完成后的最终文本）
struct TextInputEvent {
    std::string text;           // UTF-8 编码的输入文本
};

// IME 组合事件（输入法编辑中的临时文本）
struct TextEditingEvent {
    std::string text;           // 当前编辑中的文本
    int32_t cursor = 0;         // 光标在 text 中的位置
    int32_t selection_length = 0; // 选中长度
};

// 窗口事件
struct WindowEvent {
    enum class Type {
        Resized,
        Moved,
        FocusGained,
        FocusLost,
        Shown,
        Hidden,
        Minimized,
        Maximized,
        Restored,
        CloseRequested
    };

    Type type = Type::Resized;
    int32_t data1 = 0;  // width for Resized, x for Moved
    int32_t data2 = 0;  // height for Resized, y for Moved
};

// 统一的输入事件类型
enum class InputEventType {
    MouseMove,
    MouseButton,
    MouseWheel,
    Key,
    TextInput,
    TextEditing,
    Window
};

// 统一的输入事件结构
struct InputEvent {
    InputEventType type;
    uint64_t timestamp = 0;  // 事件时间戳（毫秒）

    union {
        MouseMoveEvent mouse_move;
        MouseButtonEvent mouse_button;
        MouseWheelEvent mouse_wheel;
        KeyEvent key;
        TextInputEvent text_input;
        TextEditingEvent text_editing;
        WindowEvent window;
    };

    InputEvent() : type(InputEventType::MouseMove), mouse_move{} {}
    ~InputEvent() {
        // 需要手动析构 string 成员
        switch (type) {
            case InputEventType::Key:
                key.key_name.~basic_string();
                break;
            case InputEventType::TextInput:
                text_input.text.~basic_string();
                break;
            case InputEventType::TextEditing:
                text_editing.text.~basic_string();
                break;
            default:
                break;
        }
    }

    // 禁用拷贝，使用移动
    InputEvent(const InputEvent&) = delete;
    InputEvent& operator=(const InputEvent&) = delete;
    InputEvent(InputEvent&& other) noexcept : type(other.type), timestamp(other.timestamp) {
        switch (type) {
            case InputEventType::MouseMove:
                mouse_move = other.mouse_move;
                break;
            case InputEventType::MouseButton:
                mouse_button = other.mouse_button;
                break;
            case InputEventType::MouseWheel:
                mouse_wheel = other.mouse_wheel;
                break;
            case InputEventType::Key:
                new (&key) KeyEvent(std::move(other.key));
                break;
            case InputEventType::TextInput:
                new (&text_input) TextInputEvent(std::move(other.text_input));
                break;
            case InputEventType::TextEditing:
                new (&text_editing) TextEditingEvent(std::move(other.text_editing));
                break;
            case InputEventType::Window:
                window = other.window;
                break;
        }
    }
    InputEvent& operator=(InputEvent&&) = delete;

    // 工厂方法
    static InputEvent createMouseMove(int32_t x, int32_t y) {
        InputEvent e;
        e.type = InputEventType::MouseMove;
        e.mouse_move.x = x;
        e.mouse_move.y = y;
        return e;
    }

    static InputEvent createMouseButton(int32_t x, int32_t y, MouseButton btn, bool pressed) {
        InputEvent e;
        e.type = InputEventType::MouseButton;
        e.mouse_button.x = x;
        e.mouse_button.y = y;
        e.mouse_button.button = btn;
        e.mouse_button.pressed = pressed;
        return e;
    }

    static InputEvent createMouseWheel(int32_t x, int32_t y, float dx, float dy) {
        InputEvent e;
        e.type = InputEventType::MouseWheel;
        e.mouse_wheel.x = x;
        e.mouse_wheel.y = y;
        e.mouse_wheel.delta_x = dx;
        e.mouse_wheel.delta_y = dy;
        return e;
    }

    static InputEvent createKey(uint32_t key_code, bool pressed) {
        InputEvent e;
        e.type = InputEventType::Key;
        new (&e.key) KeyEvent();
        e.key.key_code = key_code;
        e.key.pressed = pressed;
        return e;
    }

    static InputEvent createTextInput(const std::string& text) {
        InputEvent e;
        e.type = InputEventType::TextInput;
        new (&e.text_input) TextInputEvent();
        e.text_input.text = text;
        return e;
    }
};

} // namespace dong::input
