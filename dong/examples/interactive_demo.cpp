/**
 * Dong Engine Interactive Demo
 * 
 * 演示按钮点击、ScrollView 滚轮滑动、文字输入等交互功能。
 * 使用 SDL3 输入适配器将平台事件转换为引擎统一格式。
 */

#include <cstdio>
#include <cstdint>

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "platform/sdl3_window.hpp"
#include "input/input_adapter_sdl3.hpp"

using dong::platform::SDL3Window;
using dong::input::SDL3InputAdapter;
using dong::input::InputEvent;
using dong::input::InputEventType;

int main() {
    SDL_Log("=== Dong Engine Interactive Demo ===");
    SDL_Log("Features: Button clicks, ScrollView, Text input");

    // 1. 创建带 GPU 的 SDL 窗口
    SDL3Window::CreateInfo ci{};
    ci.title = "Dong Interactive Demo";
    ci.width = 800;
    ci.height = 600;
    ci.use_gpu = true;
    ci.debug_mode = false;

    SDL3Window window;
    if (!window.initialize(ci)) {
        SDL_Log("ERROR: Failed to initialize SDL3Window: %s", SDL_GetError());
        return 1;
    }

    SDL_GPUDevice* device = window.getGPUDevice();
    if (!device) {
        SDL_Log("ERROR: GPU device is null after initialization.");
        return 1;
    }

    // 2. 创建 Dong 上下文和视图
    SDL_Log("[interactive_demo] Creating dong context...");
    dong_context_t* ctx = dong_create_context();
    if (!ctx) {
        SDL_Log("ERROR: Failed to create dong context");
        return 1;
    }

    dong_view_t* view = dong_view_create(ctx, ci.width, ci.height);
    if (!view) {
        SDL_Log("ERROR: Failed to create dong view");
        dong_destroy_context(ctx);
        return 1;
    }

    // 设置 GPU 渲染模式
    dong_view_set_external_gpu_device(view, static_cast<void*>(device), 
                                      static_cast<void*>(window.getHandle()));

    // 3. 加载包含交互元素的 HTML
    const char* html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                * {
                    box-sizing: border-box;
                }
                body {
                    margin: 0;
                    padding: 20px;
                    background-color: #1a1a2e;
                    color: #eaeaea;
                    font-family: Arial, sans-serif;
                }
                
                h1 {
                    font-size: 28px;
                    color: #00d4ff;
                    margin-bottom: 20px;
                }
                
                .section {
                    background-color: #16213e;
                    border-radius: 8px;
                    padding: 16px;
                    margin-bottom: 16px;
                }
                
                .section-title {
                    font-size: 18px;
                    color: #e94560;
                    margin-bottom: 12px;
                }
                
                /* Button Section */
                .button-row {
                    display: flex;
                    gap: 12px;
                }
                
                button {
                    background-color: #0f3460;
                    color: #ffffff;
                    padding: 12px 24px;
                    border-radius: 6px;
                    font-size: 16px;
                }
                
                #click-count {
                    margin-top: 12px;
                    font-size: 14px;
                    color: #94a3b8;
                }
                
                /* ScrollView Section */
                .scroll-container {
                    overflow: scroll;
                    height: 150px;
                    background-color: #0f3460;
                    border-radius: 6px;
                    padding: 8px;
                }
                
                .scroll-item {
                    padding: 10px;
                    margin-bottom: 8px;
                    background-color: #1a1a2e;
                    border-radius: 4px;
                }
                
                /* Input Section */
                .input-group {
                    margin-bottom: 12px;
                }
                
                .input-label {
                    font-size: 14px;
                    color: #94a3b8;
                    margin-bottom: 6px;
                }
                
                input {
                    width: 100%;
                    padding: 12px;
                    background-color: #0f3460;
                    color: #ffffff;
                    border-radius: 6px;
                    font-size: 16px;
                }
                
                #input-preview {
                    margin-top: 12px;
                    padding: 12px;
                    background-color: #0f3460;
                    border-radius: 6px;
                    font-size: 14px;
                    color: #94a3b8;
                }
                
                /* Status Bar */
                .status-bar {
                    position: fixed;
                    bottom: 0;
                    left: 0;
                    right: 0;
                    padding: 8px 20px;
                    background-color: #0f3460;
                    font-size: 12px;
                    color: #64748b;
                }
            </style>
        </head>
        <body>
            <h1>Dong Interactive Demo</h1>
            
            <!-- Button Section -->
            <div class="section">
                <div class="section-title">Button Clicks</div>
                <div class="button-row">
                    <button id="btn-primary">Primary Button</button>
                    <button id="btn-secondary">Secondary</button>
                    <button id="btn-reset">Reset</button>
                </div>
                <div id="click-count">Clicks: 0</div>
            </div>
            
            <!-- ScrollView Section -->
            <div class="section">
                <div class="section-title">ScrollView (use mouse wheel)</div>
                <div class="scroll-container" id="scroll-area">
                    <div class="scroll-item">Item 1 - Scroll down to see more</div>
                    <div class="scroll-item">Item 2 - Lorem ipsum dolor sit amet</div>
                    <div class="scroll-item">Item 3 - Consectetur adipiscing elit</div>
                    <div class="scroll-item">Item 4 - Sed do eiusmod tempor</div>
                    <div class="scroll-item">Item 5 - Incididunt ut labore</div>
                    <div class="scroll-item">Item 6 - Et dolore magna aliqua</div>
                    <div class="scroll-item">Item 7 - Ut enim ad minim veniam</div>
                    <div class="scroll-item">Item 8 - Quis nostrud exercitation</div>
                </div>
            </div>
            
            <!-- Input Section -->
            <div class="section">
                <div class="section-title">Text Input</div>
                <div class="input-group">
                    <div class="input-label">Enter your name:</div>
                    <input type="text" id="name-input" placeholder="Type here..." />
                </div>
                <div class="input-group">
                    <div class="input-label">Enter your message:</div>
                    <input type="text" id="message-input" placeholder="Your message..." />
                </div>
                <div id="input-preview">Preview: (type something above)</div>
            </div>
            
            <div class="status-bar" id="status">
                Ready. Press Tab to navigate, click buttons, scroll the list, or type in inputs.
            </div>
        </body>
        </html>
    )";

    SDL_Log("[interactive_demo] Loading HTML...");
    dong_view_load_html(view, html);
    dong_view_update(view);
    SDL_Log("[interactive_demo] First update complete, installing JS handlers...");

    // 4. 安装 JavaScript 事件处理
    const char* js_code = R"JS(
        console.log("[JS] Interactive demo initialized");
        
        var state = {
            clicks: 0,
            name: "",
            message: ""
        };
        
        // Button click handlers
        var btnPrimary = document.getElementById("btn-primary");
        var btnSecondary = document.getElementById("btn-secondary");
        var btnReset = document.getElementById("btn-reset");
        var clickCount = document.getElementById("click-count");
        var status = document.getElementById("status");
        
        function updateClickCount() {
            if (clickCount) {
                clickCount.textContent = "Clicks: " + state.clicks;
            }
        }
        
        function setStatus(msg) {
            if (status) {
                status.textContent = msg;
            }
        }
        
        if (btnPrimary) {
            btnPrimary.addEventListener("click", function() {
                state.clicks++;
                updateClickCount();
                setStatus("Primary button clicked! Total: " + state.clicks);
                console.log("[JS] Primary clicked, count:", state.clicks);
            });
        }
        
        if (btnSecondary) {
            btnSecondary.addEventListener("click", function() {
                state.clicks++;
                updateClickCount();
                setStatus("Secondary button clicked! Total: " + state.clicks);
                console.log("[JS] Secondary clicked, count:", state.clicks);
            });
        }
        
        if (btnReset) {
            btnReset.addEventListener("click", function() {
                state.clicks = 0;
                updateClickCount();
                setStatus("Counter reset!");
                console.log("[JS] Reset clicked");
            });
        }
        
        // Input handlers
        var nameInput = document.getElementById("name-input");
        var messageInput = document.getElementById("message-input");
        var preview = document.getElementById("input-preview");
        
        function updatePreview() {
            if (preview) {
                var text = "Preview: ";
                if (state.name) {
                    text += "Hello, " + state.name + "!";
                }
                if (state.message) {
                    text += " Message: " + state.message;
                }
                if (!state.name && !state.message) {
                    text += "(type something above)";
                }
                preview.textContent = text;
            }
        }
        
        if (nameInput) {
            nameInput.addEventListener("input", function(e) {
                state.name = nameInput.value || "";
                updatePreview();
                setStatus("Typing name: " + state.name);
            });
            
            nameInput.addEventListener("focus", function() {
                setStatus("Name input focused - start typing");
            });
        }
        
        if (messageInput) {
            messageInput.addEventListener("input", function(e) {
                state.message = messageInput.value || "";
                updatePreview();
                setStatus("Typing message: " + state.message);
            });
            
            messageInput.addEventListener("focus", function() {
                setStatus("Message input focused - start typing");
            });
        }
        
        // Scroll handler
        var scrollArea = document.getElementById("scroll-area");
        if (scrollArea) {
            scrollArea.addEventListener("wheel", function(e) {
                setStatus("Scrolling... delta: " + (e.deltaY || 0));
            });
        }
        
        console.log("[JS] All event handlers installed");
    )JS";

    SDL_Log("[interactive_demo] About to call dong_view_eval...");
    if (!dong_view_eval(view, js_code)) {
        SDL_Log("WARNING: JS eval failed");
    } else {
        SDL_Log("[interactive_demo] JavaScript handlers installed");
    }
    SDL_Log("[interactive_demo] JS eval returned");

    // 5. 创建输入适配器
    auto input_adapter = dong::input::createSDL3InputAdapter(window.getHandle());
    
    // 设置事件回调
    bool running = true;
    input_adapter->setEventCallback([&](InputEvent&& event) {
        switch (event.type) {
            case InputEventType::Window:
                if (event.window.type == dong::input::WindowEvent::Type::CloseRequested) {
                    running = false;
                } else if (event.window.type == dong::input::WindowEvent::Type::Resized) {
                    dong_view_resize(view, event.window.data1, event.window.data2);
                }
                break;
                
            case InputEventType::MouseMove:
                dong_view_send_mouse_move(view, event.mouse_move.x, event.mouse_move.y);
                break;
                
            case InputEventType::MouseButton:
                if (event.mouse_button.pressed) {
                    dong_view_send_mouse_move(view, event.mouse_button.x, event.mouse_button.y);
                    dong_view_send_mouse_down(view, static_cast<int32_t>(event.mouse_button.button));
                } else {
                    dong_view_send_mouse_move(view, event.mouse_button.x, event.mouse_button.y);
                    dong_view_send_mouse_up(view, static_cast<int32_t>(event.mouse_button.button));
                }
                break;
                
            case InputEventType::MouseWheel:
                dong_view_send_mouse_wheel(view, event.mouse_wheel.delta_x, event.mouse_wheel.delta_y);
                break;
                
            case InputEventType::Key:
                if (event.key.pressed) {
                    dong_view_send_key_down(view, event.key.key_code);
                } else {
                    dong_view_send_key_up(view, event.key.key_code);
                }
                break;
                
            case InputEventType::TextInput:
                dong_view_send_text_input(view, event.text_input.text.c_str());
                break;
                
            default:
                break;
        }
    });

    // 启用文本输入（激活 IME）
    input_adapter->startTextInput();

    SDL_Log("[interactive_demo] Entering main loop...");
    SDL_Log("Instructions:");
    SDL_Log("  - Click buttons to increment counter");
    SDL_Log("  - Scroll mouse wheel over the list");
    SDL_Log("  - Click input fields and type text");
    SDL_Log("  - Press Tab to navigate between focusable elements");
    SDL_Log("  - Close window to exit");

    // 自动退出帧数（0 表示不自动退出）
    constexpr int AUTO_EXIT_FRAMES = 0;
    int frame_count = 0;

    // 6. 主循环
    while (running) {
        // 处理输入事件
        if (!input_adapter->pollEvents()) {
            running = false;
            break;
        }

        // 更新和渲染
        dong_view_update(view);
        ++frame_count;

        // 自动退出检查
        if (AUTO_EXIT_FRAMES > 0 && frame_count >= AUTO_EXIT_FRAMES) {
            SDL_Log("[interactive_demo] Auto-exit after %d frames", frame_count);
            break;
        }

        // 简单限帧
        SDL_Delay(16);
    }

    SDL_Log("[interactive_demo] Exiting...");

    // 7. 清理
    input_adapter->stopTextInput();
    dong_view_free(view);
    dong_destroy_context(ctx);

    SDL_Log("=== Interactive Demo Complete ===");
    return 0;
}
