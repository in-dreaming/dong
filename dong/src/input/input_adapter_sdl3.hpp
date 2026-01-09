#pragma once

#include "input_adapter.hpp"

// Forward declare SDL types to avoid including SDL headers in this header
struct SDL_Window;

namespace dong::input {

/**
 * SDL3 输入适配器
 * 
 * 将 SDL3 事件转换为 dong 引擎的统一输入事件格式
 */
class SDL3InputAdapter : public InputAdapter {
public:
    /**
     * 构造函数
     * @param window SDL 窗口指针，用于文本输入等功能
     */
    explicit SDL3InputAdapter(SDL_Window* window);
    ~SDL3InputAdapter() override;

    // InputAdapter interface
    void setEventCallback(InputEventCallback callback) override;
    bool pollEvents() override;
    void startTextInput() override;
    void stopTextInput() override;
    bool isTextInputActive() const override;
    void setTextInputRect(int32_t x, int32_t y, int32_t w, int32_t h) override;
    void getMousePosition(int32_t* x, int32_t* y) const override;
    KeyModifiers getKeyModifiers() const override;

private:
    SDL_Window* window_ = nullptr;
    InputEventCallback callback_;
    bool text_input_active_ = false;
    
    // 上一次鼠标位置（用于计算 delta）
    int32_t last_mouse_x_ = 0;
    int32_t last_mouse_y_ = 0;

    // 将 SDL 鼠标按钮转换为引擎格式
    static MouseButton convertMouseButton(uint8_t sdl_button);
    
    // 获取当前修饰键状态
    KeyModifiers getCurrentModifiers() const;
};

/**
 * 创建 SDL3 输入适配器的工厂函数
 */
InputAdapterPtr createSDL3InputAdapter(SDL_Window* window);

} // namespace dong::input
