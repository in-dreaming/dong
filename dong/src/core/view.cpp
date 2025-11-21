#include "view.hpp"
#include <cstdio>
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
      painter(nullptr),
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
    std::fprintf(stderr, "[dong::View] scripts\n");
    if (script_engine) {
        script_engine->processPendingTasks();
    }

    std::fprintf(stderr, "[dong::View] layout\n");
    if (layout_engine && dom_manager) {
        auto root = dom_manager->getRoot();
        if (root) {
            layout_engine->calculateLayout(root, static_cast<float>(width_), static_cast<float>(height_));
            if (render_surface) {
                render_surface->markDirty();
            } else {
                std::fprintf(stderr, "[dong::View] layout warning: no render surface\n");
            }
        } else {
            std::fprintf(stderr, "[dong::View] layout skipped: no DOM root\n");
        }
    } else {
        std::fprintf(stderr, "[dong::View] layout skipped: missing engine or DOM\n");
    }

    std::fprintf(stderr, "[dong::View] render\n");
    if (render_surface && render_surface->isDirty() && painter && dom_manager) {
        auto root = dom_manager->getRoot();
        if (root) {
            std::fprintf(stderr, "[dong::View] render invoking painter\n");
            painter->renderDOM(root, layout_engine.get());
            std::fprintf(stderr, "[dong::View] render finished\n");
        } else {
            std::fprintf(stderr, "[dong::View] render skipped: no DOM root\n");
        }
    } else {
        std::fprintf(stderr, "[dong::View] render skipped: prerequisites not met\n");
    }

    std::fprintf(stderr, "[dong::View] done\n");
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
