#include "view.hpp"
#include <cstdio>
#include <string>
#include "../dom/dom_manager.hpp"
#include "../dom/event_system.hpp"
#include "../render/render_surface.hpp"
#include "../render/painter.hpp"
#include "../render/gpu_device.hpp"
#include "../render/gpu_surface.hpp"
#include "../render/gpu_painter.hpp"
#include "../render/shader_manager.hpp"
#include "../script/script_engine.hpp"
#include "../script/js_bindings.hpp"
#include "../layout/layout_engine.hpp"

namespace dong {

namespace {

using dong::dom::DOMNodePtr;

DOMNodePtr hitTestRecursive(const DOMNodePtr& node, dong::layout::Engine* layout_engine,
                            int32_t x, int32_t y) {
    if (!node || !layout_engine) return nullptr;

    DOMNodePtr best;
    const auto* layout = layout_engine->getLayout(node);
    if (layout) {
        float lx = layout->x;
        float ly = layout->y;
        float w = layout->width;
        float h = layout->height;
        if (x >= lx && x <= lx + w && y >= ly && y <= ly + h) {
            best = node;
        }
    }

    for (const auto& child : node->getChildren()) {
        auto child_hit = hitTestRecursive(child, layout_engine, x, y);
        if (child_hit) {
            best = child_hit;
        }
    }
    return best;
}

DOMNodePtr hitTestElementAt(dong::dom::Manager* dom_mgr, dong::layout::Engine* layout_engine,
                            int32_t x, int32_t y) {
    if (!dom_mgr || !layout_engine) return nullptr;
    auto root = dom_mgr->getRoot();
    if (!root) return nullptr;
    return hitTestRecursive(root, layout_engine, x, y);
}

} // anonymous namespace

View::View(uint32_t width, uint32_t height)
    : width_(width), height_(height), use_gpu_(false),
      dom_manager(std::make_unique<dom::Manager>()),
      layout_engine(std::make_unique<layout::Engine>()),
      render_surface(std::make_unique<render::CPUBufferSurface>(width, height)),
      painter(nullptr),
      script_engine(std::make_unique<script::ScriptEngine>()),
      event_dispatcher(std::make_unique<dom::EventDispatcher>()),
      js_bindings(std::make_unique<script::JSBindings>(
          script_engine.get(),
          dom_manager.get(),
          event_dispatcher.get()
      )) {
    // Defer JS bindings initialization until after first HTML load / script eval
    painter = std::make_unique<render::Painter>(render_surface.get());
}

View::~View() {
    // All resources are cleaned up via unique_ptr destructors
}

void View::load_html(const char* html) {
    if (!html || !dom_manager) return;
    
    if (dom_manager->loadHTML(html)) {
        // DOM 树被整体替换，清理 JS 侧的节点映射和事件监听
        if (js_bindings) {
            js_bindings->resetForNewDOM();
        }
        // 标记需要重新渲染
        if (render_surface) {
            render_surface->markDirty();
        }
        // 新的 DOM 树需要重新布局
        auto root = dom_manager->getRoot();
        if (root) {
            root->markLayoutDirty();
        }
    }
}

void View::resize(uint32_t width, uint32_t height) {
    if (width == width_ && height == height_) return;

    width_ = width;
    height_ = height;

    // 重新分配渲染表面
    if (render_surface && render_surface->getType() == render::RenderSurface::Type::CPU_BUFFER) {
        static_cast<render::CPUBufferSurface*>(render_surface.get())->resize(width, height);
    }

    render_surface->markDirty();

    // 尺寸变化会影响整个布局
    if (dom_manager) {
        auto root = dom_manager->getRoot();
        if (root) {
            root->markLayoutDirty();
        }
    }
}

void View::update() {
    if (script_engine) {
        script_engine->processPendingTasks();
    }

    if (layout_engine && dom_manager) {
        auto root = dom_manager->getRoot();
        if (root && root->isLayoutDirty()) {
            layout_engine->calculateLayout(root, static_cast<float>(width_), static_cast<float>(height_));
            root->clearLayoutDirtyRecursive();
            if (render_surface) {
                render_surface->markDirty();
            }
        }
    }

    if (!render_surface || !render_surface->isDirty() || !dom_manager) {
        return;
    }

    auto root = dom_manager->getRoot();
    if (!root) {
        return;
    }

    // 无论 GPU 还是 CPU 模式，都先用 CPU Painter 渲染 DOM
    if (painter) {
        painter->renderDOM(root, layout_engine.get());
    }

    // GPU 模式下通过 GPU Painter 进行最终显示（包括像素上传和渲染）
    if (use_gpu_ && gpu_painter_) {
        // 获取 CPU 缓冲的像素数据
        const void* cpu_buffer = nullptr;
        if (render_surface && render_surface->getType() == render::RenderSurface::Type::CPU_BUFFER) {
            cpu_buffer = render_surface->getCPUBuffer();
        }

        // GPU Painter 内部管理命令缓冲的生命周期
        gpu_painter_->beginFrame();

        // 上传 CPU 像素到纹理
        if (cpu_buffer) {
            gpu_painter_->uploadCPUPixelsToGPU(cpu_buffer, width_, height_);
        }

        // 渲染内容纹理到屏幕
        gpu_painter_->render(dom_manager.get(), layout_engine.get());

        gpu_painter_->endFrame();
    }
}

void* View::get_pixel_buffer() {
    if (render_surface && render_surface->getType() == render::RenderSurface::Type::CPU_BUFFER) {
        return render_surface->getCPUBuffer();
    }
    return nullptr;
}

bool View::eval_script(const char* code) {
    if (!code || !script_engine) return false;
    ensureJSBindingsInitialized();
    return script_engine->eval(std::string(code));
}

// 【缺口3】执行脚本并返回结果
std::string View::eval_script_with_return(const char* code) {
    if (!code || !script_engine) return "";
    ensureJSBindingsInitialized();
    last_eval_return_value_ = script_engine->evalWithReturn(std::string(code));
    return last_eval_return_value_;
}

void View::ensureJSBindingsInitialized() {
    if (js_bindings_initialized_ || !js_bindings || !script_engine) return;
    JSContext* ctx = script_engine->getContext();
    if (!ctx) return;
    js_bindings->initialize();
    js_bindings_initialized_ = true;
}

void View::handle_mouse_move(int32_t x, int32_t y) {
    last_mouse_x_ = x;
    last_mouse_y_ = y;
    dispatchMouseEventToJS("mousemove", last_mouse_x_, last_mouse_y_, 0);
}

void View::handle_mouse_down(int32_t button) {
    dispatchMouseEventToJS("mousedown", last_mouse_x_, last_mouse_y_, button);
}

void View::handle_mouse_up(int32_t button) {
    dispatchMouseEventToJS("mouseup", last_mouse_x_, last_mouse_y_, button);
    dispatchMouseEventToJS("click", last_mouse_x_, last_mouse_y_, button);
}

void View::handle_key_down(uint32_t key_code) {
    dispatchKeyEventToJS("keydown", key_code);
}

void View::handle_key_up(uint32_t key_code) {
    dispatchKeyEventToJS("keyup", key_code);
}

void View::dispatchMouseEventToJS(const char* type, int32_t x, int32_t y, int32_t button) {
    if (!script_engine || !js_bindings || !event_dispatcher) return;

    // Determine the DOM event target via layout hit-testing
    auto target = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
    // For simple demos, fall back to the first <button> element if hit-test fails
    if (!target && dom_manager) {
        auto buttons = dom_manager->getElementsByTagName("button");
        if (!buttons.empty()) {
            target = buttons[0];
        }
    }
    if (!target) return;

    // Map DOM event type string to internal EventType
    dom::EventType ev_type;
    std::string type_str = type ? std::string(type) : std::string();
    if (type_str == "click") {
        ev_type = dom::EventType::CLICK;
    } else if (type_str == "mousedown") {
        ev_type = dom::EventType::MOUSE_DOWN;
    } else if (type_str == "mouseup") {
        ev_type = dom::EventType::MOUSE_UP;
    } else if (type_str == "mousemove") {
        ev_type = dom::EventType::MOUSE_MOVE;
    } else {
        // Unsupported mouse event type for native dispatch
        return;
    }

    // Create a native DOM event and dispatch through EventDispatcher, which
    // will walk up the DOM tree and invoke any bridged JS listeners.
    dom::Event event = event_dispatcher->createMouseEvent(ev_type, x, y, button);
    event.target = target;
    event.current_target = target;

    event_dispatcher->dispatch(event);
}

void View::dispatchKeyEventToJS(const char* type, uint32_t key_code) {
    if (!script_engine || !js_bindings) return;

    if (!dom_manager) return;
    auto bodies = dom_manager->getElementsByTagName("body");
    dom::DOMNodePtr target;
    if (!bodies.empty()) {
        target = bodies[0];
    } else {
        target = dom_manager->getRoot();
    }
    if (!target) return;

    JSContext* ctx = script_engine->getContext();
    if (!ctx) return;

    uint64_t node_id = js_bindings->getNodeIdFor(target);
    if (!node_id) {
        JSValue tmp = js_bindings->createJSElement(ctx, target);
        JS_FreeValue(ctx, tmp);
        node_id = js_bindings->getNodeIdFor(target);
        if (!node_id) {
            return;
        }
    }

    js_bindings->dispatchKeyEvent(node_id, type, key_code);
}

void View::setRenderMode(bool use_gpu) {
    if (use_gpu == use_gpu_) return;

    use_gpu_ = use_gpu;

    if (use_gpu_) {
        // 创建 GPU 设备（离屏渲染，暂不绑定窗口）
#ifdef __APPLE__
        render::GPUDevice::CreateInfo ci{ SDL_GPU_SHADERFORMAT_MSL, false };
#else
        render::GPUDevice::CreateInfo ci{ SDL_GPU_SHADERFORMAT_SPIRV, false };
#endif
        gpu_device_ = std::make_unique<render::GPUDevice>();
        if (!gpu_device_->initialize(ci)) {
            gpu_device_.reset();
            use_gpu_ = false;
        }

        if (use_gpu_) {
            // 用 GPU 纹理替换渲染表面
            auto gpu_surface = std::make_unique<render::GPUTextureSurfaceImpl>(
                gpu_device_->getHandle(),
                nullptr,
                width_,
                height_
            );
            render_surface = std::move(gpu_surface);

            shader_manager_ = std::make_unique<render::ShaderManager>(gpu_device_.get());
            gpu_painter_ = std::make_unique<render::GPUPainter>(
                static_cast<render::GPUTextureSurfaceImpl*>(render_surface.get()),
                gpu_device_.get(),
                shader_manager_.get()
            );
            gpu_painter_->initialize();
        }
    } else {
        // 回退到 CPU 缓冲路径
        gpu_painter_.reset();
        shader_manager_.reset();
        gpu_device_.reset();
        render_surface = std::make_unique<render::CPUBufferSurface>(width_, height_);
        painter = std::make_unique<render::Painter>(render_surface.get());
    }

    if (!use_gpu_ && !painter) {
        // 如果 GPU 初始化失败，回退到 CPU
        render_surface = std::make_unique<render::CPUBufferSurface>(width_, height_);
        painter = std::make_unique<render::Painter>(render_surface.get());
    }

    if (render_surface) {
        render_surface->markDirty();
    }
}

void View::setExternalGPUDevice(SDL_GPUDevice* device, SDL_Window* window) {
    if (!device || !window) {
        return;
    }

#ifdef __APPLE__
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_MSL;
#else
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
#endif

    if (!gpu_device_) {
        gpu_device_ = std::make_unique<render::GPUDevice>();
    }
    gpu_device_->adoptExternal(device, format);

    // 创建 GPU 表面用于窗口绑定
    gpu_surface_ = std::make_unique<render::GPUTextureSurfaceImpl>(
        device,
        window,
        width_,
        height_
    );

    // 始终创建 CPU 缓冲作为主渲染表面（由 CPU Painter 绘制）
    auto cpu_surface = std::make_unique<render::CPUBufferSurface>(width_, height_);
    render_surface = std::move(cpu_surface);

    // 创建 CPU Painter 用于绘制 DOM
    painter = std::make_unique<render::Painter>(render_surface.get());

    shader_manager_ = std::make_unique<render::ShaderManager>(gpu_device_.get());
    gpu_painter_ = std::make_unique<render::GPUPainter>(
        gpu_surface_.get(),
        gpu_device_.get(),
        shader_manager_.get()
    );

    if (!gpu_painter_->initialize()) {
        gpu_painter_.reset();
        shader_manager_.reset();
        gpu_surface_.reset();
        use_gpu_ = false;
        return;
    }

    use_gpu_ = true;
    if (render_surface) {
        render_surface->markDirty();
    }
}

} // namespace dong
