#pragma once

#include "input_types.hpp"
#include <functional>
#include <memory>
#include <string>

namespace dong::input {

// 输入事件回调类型
using InputEventCallback = std::function<void(InputEvent&&)>;

/**
 * 输入适配器抽象接口
 * 
 * 每个平台 SDK（SDL3、GLFW、Win32 等）实现此接口，
 * 将平台特定的输入事件转换为 dong 引擎的统一格式。
 * 
 * 使用示例：
 *   auto adapter = createSDL3InputAdapter(window);
 *   adapter->setEventCallback([&view](InputEvent&& e) {
 *       view.handleInputEvent(std::move(e));
 *   });
 *   
 *   // 在事件循环中调用
 *   while (running) {
 *       adapter->pollEvents();
 *   }
 */
class InputAdapter {
public:
    virtual ~InputAdapter() = default;

    /**
     * 设置事件回调
     * 当平台事件被转换后，通过此回调传递给引擎
     */
    virtual void setEventCallback(InputEventCallback callback) = 0;

    /**
     * 轮询并处理平台事件
     * 应在主循环中调用，会触发 setEventCallback 设置的回调
     * @return false 如果收到退出请求
     */
    virtual bool pollEvents() = 0;

    /**
     * 启用文本输入模式（激活 IME）
     * 在需要文本输入时调用（如聚焦到输入框）
     */
    virtual void startTextInput() = 0;

    /**
     * 停止文本输入模式（关闭 IME）
     * 在不需要文本输入时调用（如输入框失焦）
     */
    virtual void stopTextInput() = 0;

    /**
     * 检查文本输入模式是否激活
     */
    virtual bool isTextInputActive() const = 0;

    /**
     * 设置文本输入区域（用于 IME 候选框定位）
     * @param x, y 输入区域左上角坐标
     * @param w, h 输入区域尺寸
     */
    virtual void setTextInputRect(int32_t x, int32_t y, int32_t w, int32_t h) = 0;

    /**
     * 获取当前鼠标位置
     */
    virtual void getMousePosition(int32_t* x, int32_t* y) const = 0;

    /**
     * 获取修饰键状态
     */
    virtual KeyModifiers getKeyModifiers() const = 0;

    /**
     * 设置鼠标光标样式
     * @param cursor_name CSS 光标名称：auto, default, pointer, text, move, wait, help, crosshair, not-allowed, grab, grabbing
     */
    virtual void setCursor(const std::string& cursor_name) = 0;
};

using InputAdapterPtr = std::unique_ptr<InputAdapter>;

} // namespace dong::input
