#include "view.hpp"
#include "../dom/dom_manager.hpp"
#include "../render/render_surface.hpp"
#include "../render/painter.hpp"
#include "../script/script_engine.hpp"
#include "../script/js_bindings.hpp"
#include "../layout/layout_engine.hpp"

namespace dong {

View::View(uint32_t width, uint32_t height)
    : width_(width), height_(height), use_gpu_(false),
      dom_manager(std::make_unique<dom::Manager>()),
      layout_engine(std::make_unique<layout::Engine>()),
      render_surface(std::make_unique<render::CPUBufferSurface>(width, height)),
      painter(std::make_unique<render::Painter>(render_surface.get())),
      script_engine(std::make_unique<script::ScriptEngine>()),
      js_bindings(std::make_unique<script::JSBindings>(
          script_engine.get(), 
          dom_manager.get(), 
          nullptr // TODO: 连接事件分发器
      )) {
    js_bindings->initialize();
}

View::~View() {
    // All resources are cleaned up via unique_ptr destructors
}

void View::load_html(const char* html) {
    if (!html || !dom_manager) return;
    
    if (dom_manager->loadHTML(html)) {
        // 标记需要重新渲染
        if (render_surface) {
            render_surface->markDirty();
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
}

void View::update() {
    // 更新流程：
    // 1. 处理脚本任务
    // 2. 计算布局
    // 3. 渲染 DOM

    if (script_engine) {
        script_engine->processPendingTasks();
    }

    // 计算布局
    if (layout_engine && dom_manager) {
        auto root = dom_manager->getRoot();
        if (root) {
            layout_engine->calculateLayout(root, static_cast<float>(width_), static_cast<float>(height_));
            render_surface->markDirty();
        }
    }

    // 渲染 DOM
    if (render_surface && render_surface->isDirty() && painter && dom_manager) {
        auto root = dom_manager->getRoot();
        if (root) {
            painter->renderDOM(root, layout_engine.get());
        }
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
    return script_engine->eval(std::string(code));
}

void View::setRenderMode(bool use_gpu) {
    if (use_gpu == use_gpu_) return;

    use_gpu_ = use_gpu;

    if (use_gpu) {
        // TODO: 创建 GPU 纹理表面
        // render_surface = std::make_unique<render::GPUTextureSurface>(width_, height_, 0);
    } else {
        // 使用 CPU 缓冲
        render_surface = std::make_unique<render::CPUBufferSurface>(width_, height_);
    }

    painter = std::make_unique<render::Painter>(render_surface.get());
    render_surface->markDirty();
}

} // namespace dong
