// isolation_demo.cpp - 验证 CSS isolation 属性的离屏图层合成
// 
// 此示例展示 isolation: isolate 如何创建独立的混合上下文：
// 1. 使用 isolation: auto (默认) - 内容与父级混合
// 2. 使用 isolation: isolate - 创建独立图层，先在图层内混合，再整体合成到父级
//
// 预期结果：
// - auto 模式下，半透明元素会与背景叠加产生视觉混合
// - isolate 模式下，内部元素先在离屏纹理合成，再以整体透明度混合到父级

#include "core/view.hpp"
#include "dom/dom_node.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <memory>

using namespace dong;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Dong - CSS Isolation Demo",
        1200, 800,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int window_w = 0, window_h = 0;
    SDL_GetWindowSize(window, &window_w, &window_h);
    
    auto view = std::make_unique<View>(static_cast<uint32_t>(window_w), static_cast<uint32_t>(window_h));
    
    // 获取 SDL GPU 设备并传递给 View
    SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        true,
        nullptr
    );
    
    if (!gpu_device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
        SDL_Log("Failed to claim window for GPU: %s", SDL_GetError());
        SDL_DestroyGPUDevice(gpu_device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    view->setExternalGPUDevice(gpu_device, window);
    view->setRenderMode(true);

    // 构建测试 HTML：左侧 auto，右侧 isolate
    const char* html = R"(
<!DOCTYPE html>
<html>
<head>
<style>
body {
    margin: 0;
    padding: 20px;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    font-family: Arial, sans-serif;
}

.container {
    display: flex;
    gap: 40px;
    padding: 20px;
}

.column {
    flex: 1;
    background: white;
    border-radius: 12px;
    padding: 20px;
    box-shadow: 0 10px 40px rgba(0,0,0,0.15);
}

.title {
    font-size: 24px;
    font-weight: bold;
    margin-bottom: 20px;
    text-align: center;
}

.auto-mode .title {
    color: #e74c3c;
}

.isolate-mode .title {
    color: #27ae60;
}

.test-area {
    position: relative;
    height: 300px;
    background: linear-gradient(45deg, #f39c12 0%, #e74c3c 100%);
    border-radius: 8px;
    margin-bottom: 20px;
}

/* auto 模式：默认行为，元素与父级混合 */
.auto-box {
    position: absolute;
    top: 50px;
    left: 50px;
    width: 200px;
    height: 200px;
    background: rgba(52, 152, 219, 0.7);
    border-radius: 8px;
    isolation: auto;
}

.auto-box .inner {
    position: absolute;
    top: 30px;
    left: 30px;
    width: 140px;
    height: 140px;
    background: rgba(46, 204, 113, 0.7);
    border-radius: 8px;
}

/* isolate 模式：创建独立混合上下文 */
.isolate-box {
    position: absolute;
    top: 50px;
    left: 50px;
    width: 200px;
    height: 200px;
    background: rgba(52, 152, 219, 0.7);
    border-radius: 8px;
    isolation: isolate;
}

.isolate-box .inner {
    position: absolute;
    top: 30px;
    left: 30px;
    width: 140px;
    height: 140px;
    background: rgba(46, 204, 113, 0.7);
    border-radius: 8px;
}

.description {
    font-size: 14px;
    line-height: 1.6;
    color: #555;
}

.code {
    background: #f8f9fa;
    border-left: 4px solid #667eea;
    padding: 10px;
    margin-top: 10px;
    font-family: monospace;
    font-size: 13px;
    border-radius: 4px;
}

.note {
    background: #fff3cd;
    border: 1px solid #ffc107;
    border-radius: 4px;
    padding: 10px;
    margin-top: 15px;
    font-size: 13px;
    color: #856404;
}
</style>
</head>
<body>
    <h1 style="color: white; text-align: center; margin-bottom: 30px;">
        CSS Isolation 属性演示
    </h1>
    
    <div class="container">
        <!-- 左列：isolation: auto -->
        <div class="column auto-mode">
            <div class="title">isolation: auto (默认)</div>
            
            <div class="test-area">
                <div class="auto-box">
                    <div class="inner"></div>
                </div>
            </div>
            
            <div class="description">
                <strong>默认行为：</strong>
                半透明元素直接与背景混合，所有层级的颜色叠加产生复合视觉效果。
                
                <div class="code">
.auto-box {
    isolation: auto;
    background: rgba(52, 152, 219, 0.7);
}
                </div>
            </div>
        </div>
        
        <!-- 右列：isolation: isolate -->
        <div class="column isolate-mode">
            <div class="title">isolation: isolate</div>
            
            <div class="test-area">
                <div class="isolate-box">
                    <div class="inner"></div>
                </div>
            </div>
            
            <div class="description">
                <strong>隔离模式：</strong>
                创建独立的离屏图层，内部元素先在图层内合成，然后整体以指定透明度混合到父级。
                
                <div class="code">
.isolate-box {
    isolation: isolate;
    background: rgba(52, 152, 219, 0.7);
}
                </div>
                
                <div class="note">
                    <strong>GPU 实现：</strong>
                    此模式会触发离屏渲染目标创建，
                    通过 BeginIsolatedLayer / EndIsolatedLayer 
                    在 GPU 上完成图层合成。
                </div>
            </div>
        </div>
    </div>
</body>
</html>
)";

    view->load_html(html);
    view->update();

    SDL_Log("Isolation demo running...");
    SDL_Log("Left panel: isolation: auto (default blending)");
    SDL_Log("Right panel: isolation: isolate (offscreen layer composition)");

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
                    running = false;
                }
            }
        }

        view->update();
        SDL_Delay(16);
    }

    view.reset();
    SDL_DestroyGPUDevice(gpu_device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
