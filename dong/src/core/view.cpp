#include "view.hpp"
#include <cstdio>
#include <string>
#include <cstring>
#include <functional>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_gpu.h>
#include "../dom/dom_manager.hpp"
#include "../dom/event_system.hpp"
#include "../render/render_surface.hpp"
#include "../render/painter.hpp"
#include "../render/gpu_device.hpp"
#include "../render/gpu_surface.hpp"
#include "../render/gpu_painter.hpp"
#include "../render/shader_manager.hpp"
#include "../render/gpu_ir.hpp"
#include "../render/gpu_driver.hpp"
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

    // 重新分配渲染表面（CPU 路径）
    if (render_surface && render_surface->getType() == render::RenderSurface::Type::CPU_BUFFER) {
        static_cast<render::CPUBufferSurface*>(render_surface.get())->resize(width, height);
    }

    // GPU 路径下还需要同步调整 GPU 表面和内容纹理尺寸
    if (use_gpu_) {
        // 外部 GPU 设备路径：单独维护一个 gpu_surface_
        if (gpu_surface_) {
            gpu_surface_->resize(width, height);
        }
        // 离屏 GPU 路径：render_surface 本身就是 GPU 纹理表面
        if (render_surface && render_surface->getType() == render::RenderSurface::Type::GPU_TEXTURE) {
            auto* gpu_tex_surface = static_cast<render::GPUTextureSurfaceImpl*>(render_surface.get());
            gpu_tex_surface->resize(width, height);
        }
        // 内容纹理跟随 View 尺寸变化
        if (gpu_painter_) {
            gpu_painter_->resizeContentTexture(width, height);
        }
    }

    if (render_surface) {
        render_surface->markDirty();
    }

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
            
            // 输出布局调试信息
            auto htmls = dom_manager->getElementsByTagName("html");
            if (!htmls.empty()) {
                const auto* layout = layout_engine->getLayout(htmls[0]);
                if (layout) {
                    SDL_Log("[View::update Layout] html: at (%.1f, %.1f) size %.1fx%.1f",
                           layout->x, layout->y, layout->width, layout->height);
                }
            }
            
            auto bodies = dom_manager->getElementsByTagName("body");
            if (!bodies.empty()) {
                const auto* layout = layout_engine->getLayout(bodies[0]);
                if (layout) {
                    SDL_Log("[View::update Layout] body: at (%.1f, %.1f) size %.1fx%.1f",
                           layout->x, layout->y, layout->width, layout->height);
                }
            }
            
            auto h1s = dom_manager->getElementsByTagName("h1");
            if (!h1s.empty()) {
                const auto* layout = layout_engine->getLayout(h1s[0]);
                if (layout) {
                    SDL_Log("[View::update Layout] h1: at (%.1f, %.1f) size %.1fx%.1f",
                           layout->x, layout->y, layout->width, layout->height);
                }
            }
            
            auto divs = dom_manager->getElementsByTagName("div");
            for (size_t i = 0; i < divs.size() && i < 3; i++) {
                const auto* layout = layout_engine->getLayout(divs[i]);
                if (layout) {
                    SDL_Log("[View::update Layout] div[%zu]: at (%.1f, %.1f) size %.1fx%.1f",
                           i, layout->x, layout->y, layout->width, layout->height);
                }
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

    // GPU 路径：DisplayList → GPU 渲染
    if (use_gpu_ && gpu_driver_ && painter) {
        SDL_Log("[View::update] Building DisplayList...");
        const render::DisplayList& dl = painter->buildDisplayList(root, layout_engine.get());
        SDL_Log("[View::update] DisplayList built with %zu items", dl.items.size());
        
        render::GPUCompiler compiler;
        render::GPUCommandList cmd_list;
        SDL_Log("[View::update] Compiling DisplayList to GPUCommandList...");
        compiler.compile(dl, cmd_list);
        SDL_Log("[View::update] GPUCommandList compiled with %zu commands", cmd_list.commands.size());

        SDL_Log("[View::update] Beginning GPU frame...");
        gpu_driver_->beginFrame();
        SDL_Log("[View::update] Executing GPUCommandList...");
        gpu_driver_->execute(cmd_list);
        SDL_Log("[View::update] Ending GPU frame...");
        gpu_driver_->endFrame();
        SDL_Log("[View::update] GPU frame completed");
        return;
    }

    // CPU 路径已经不再支持（原有的 Skia 后端已移除）
    // 如需像素读取，请使用 GPU 渲染模式 + offscreen texture
    SDL_Log("[View::update] Warning: CPU render path is no longer supported");
}

void* View::get_pixel_buffer() {
    if (render_surface && render_surface->getType() == render::RenderSurface::Type::CPU_BUFFER) {
        return render_surface->getCPUBuffer();
    }
    return nullptr;
}

SDL_GPUTexture* View::renderToGPUTexture(SDL_GPUDevice* device, uint32_t width, uint32_t height) {
    if (!device) {
        SDL_Log("[View::renderToGPUTexture] Invalid device");
        return nullptr;
    }
    
    if (!gpu_driver_) {
        SDL_Log("[View::renderToGPUTexture] GPU driver not initialized");
        return nullptr;
    }
    
    if (!painter) {
        SDL_Log("[View::renderToGPUTexture] Painter not initialized");
        return nullptr;
    }
    
    // 1. 创建离屏渲染目标纹理（使用UNORM格式，shader手动处理gamma）
    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;  // UNORM格式，不自动gamma校正
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    tex_info.width = width;
    tex_info.height = height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    
    SDL_GPUTexture* offscreen_texture = SDL_CreateGPUTexture(device, &tex_info);
    if (!offscreen_texture) {
        SDL_Log("[View::renderToGPUTexture] Failed to create offscreen texture: %s", SDL_GetError());
        return nullptr;
    }
    
    // 2. 确保布局已计算
    auto root = dom_manager ? dom_manager->getRoot() : nullptr;
    if (!root) {
        SDL_Log("[View::renderToGPUTexture] No DOM root");
        SDL_ReleaseGPUTexture(device, offscreen_texture);
        return nullptr;
    }
    
    SDL_Log("[View::renderToGPUTexture] root->isLayoutDirty() = %s", root->isLayoutDirty() ? "TRUE" : "FALSE");
    
    // 执行布局计算（如果需要）
    if (layout_engine && root->isLayoutDirty()) {
        SDL_Log("[View::renderToGPUTexture] Calculating layout...");
        layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
        root->clearLayoutDirtyRecursive();
        SDL_Log("[View::renderToGPUTexture] Layout calculated");
    }
    
    std::printf("[DEBUG] After layout check\n");
    std::fflush(stdout);
    
    // 无条件输出所有主要元素的布局
    if (layout_engine && dom_manager) {
        auto htmls = dom_manager->getElementsByTagName("html");
        std::printf("[Layout] Found %zu html elements\n", htmls.size());
        if (!htmls.empty()) {
            const auto* layout = layout_engine->getLayout(htmls[0]);
            if (layout) {
                std::printf("[Layout] html: at (%.1f, %.1f) size %.1fx%.1f\n",
                       layout->x, layout->y, layout->width, layout->height);
            }
        }
        
        auto bodies = dom_manager->getElementsByTagName("body");
        std::printf("[Layout] Found %zu body elements\n", bodies.size());
        if (!bodies.empty()) {
            const auto* layout = layout_engine->getLayout(bodies[0]);
            if (layout) {
                std::printf("[Layout] body: at (%.1f, %.1f) size %.1fx%.1f\n",
                       layout->x, layout->y, layout->width, layout->height);
            }
        }
        
        auto h1s = dom_manager->getElementsByTagName("h1");
        std::printf("[Layout] Found %zu h1 elements\n", h1s.size());
        if (!h1s.empty()) {
            const auto* layout = layout_engine->getLayout(h1s[0]);
            if (layout) {
                std::printf("[Layout] h1: at (%.1f, %.1f) size %.1fx%.1f\n",
                       layout->x, layout->y, layout->width, layout->height);
            }
        }
        
        auto divs = dom_manager->getElementsByTagName("div");
        std::printf("[Layout] Found %zu div elements\n", divs.size());
        for (size_t i = 0; i < divs.size(); i++) {
            const auto* layout = layout_engine->getLayout(divs[i]);
            if (layout) {
                std::printf("[Layout] div[%zu]: at (%.1f, %.1f) size %.1fx%.1f\n",
                       i, layout->x, layout->y, layout->width, layout->height);
            }
        }
        std::fflush(stdout);
    }
    
    // 3. 构建 DisplayList 并编译为 GPUCommandList
    SDL_Log("[View::renderToGPUTexture] Building DisplayList...");
    const render::DisplayList& dl = painter->buildDisplayList(root, layout_engine.get());
    SDL_Log("[View::renderToGPUTexture] DisplayList has %zu items", dl.items.size());
    
    render::GPUCompiler compiler;
    render::GPUCommandList cmd_list;
    compiler.compile(dl, cmd_list);
    SDL_Log("[View::renderToGPUTexture] GPUCommandList has %zu commands", cmd_list.commands.size());
    
    // 4. 执行离屏渲染
    gpu_driver_->beginFrameOffscreen(offscreen_texture, width, height);
    gpu_driver_->execute(cmd_list);
    gpu_driver_->endFrameOffscreen();
    
    SDL_Log("[View::renderToGPUTexture] Successfully rendered to GPU texture %u x %u", width, height);
    
    // 返回纹理，调用者负责释放
    return offscreen_texture;
}

bool View::renderOffscreen(SDL_GPUDevice* device, uint32_t width, uint32_t height, 
                           uint8_t* out_pixels) {
    if (!device || !out_pixels) {
        SDL_Log("[View::renderOffscreen] Invalid parameters");
        return false;
    }
    
    // 1. 调用底层接口渲染到GPU纹理
    SDL_GPUTexture* offscreen_texture = renderToGPUTexture(device, width, height);
    if (!offscreen_texture) {
        SDL_Log("[View::renderOffscreen] Failed to render to GPU texture");
        return false;
    }
    
    // 2. 创建传输缓冲区用于读回像素
    // 2. 创建传输缓冲区用于读回像素
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    transfer_info.size = width * height * 4;
    
    SDL_GPUTransferBuffer* download_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
    if (!download_buffer) {
        SDL_Log("[View::renderOffscreen] Failed to create download buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTexture(device, offscreen_texture);
        return false;
    }
    
    // 3. 执行纹理到传输缓冲区的拷贝
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        SDL_Log("[View::renderOffscreen] Failed to acquire command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
        SDL_ReleaseGPUTexture(device, offscreen_texture);
        return false;
    }
    
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    
    SDL_GPUTextureRegion src_region{};
    src_region.texture = offscreen_texture;
    src_region.mip_level = 0;
    src_region.layer = 0;
    src_region.x = 0;
    src_region.y = 0;
    src_region.z = 0;
    src_region.w = width;
    src_region.h = height;
    src_region.d = 1;
    
    SDL_GPUTextureTransferInfo dst_transfer{};
    dst_transfer.transfer_buffer = download_buffer;
    dst_transfer.offset = 0;
    dst_transfer.pixels_per_row = 0;
    dst_transfer.rows_per_layer = 0;
    
    SDL_DownloadFromGPUTexture(copy_pass, &src_region, &dst_transfer);
    SDL_EndGPUCopyPass(copy_pass);
    
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(device);
    
    // 4. Map 并复制像素数据
    void* mapped = SDL_MapGPUTransferBuffer(device, download_buffer, false);
    if (!mapped) {
        SDL_Log("[View::renderOffscreen] Failed to map download buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
        SDL_ReleaseGPUTexture(device, offscreen_texture);
        return false;
    }
    
    std::memcpy(out_pixels, mapped, width * height * 4);
    SDL_UnmapGPUTransferBuffer(device, download_buffer);
    
    // 5. 清理资源
    SDL_ReleaseGPUTransferBuffer(device, download_buffer);
    SDL_ReleaseGPUTexture(device, offscreen_texture);
    
    SDL_Log("[View::renderOffscreen] Successfully rendered and read back %u x %u pixels", width, height);
    return true;
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

    // 释放旧的 CPU/GPU 表面和 Painter，避免悬空引用
    painter.reset();
    gpu_surface_.reset();
    render_surface.reset();

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

    // 创建 CPU Painter 用于绘制 DOM（DisplayList 生成仍在 CPU 侧完成）
    painter = std::make_unique<render::Painter>(render_surface.get());

    shader_manager_ = std::make_unique<render::ShaderManager>(gpu_device_.get());
    gpu_painter_.reset();

    gpu_driver_ = render::CreateGPUDriver(
        render::GPUBackendType::SDL_GPU,
        gpu_device_.get(),
        window,
        shader_manager_.get()
    );

    // TODO: 重新接入 ResourceManager（用于图片缓存）
    // if (painter && gpu_driver_) {
    //     auto* rm = painter->getResourceManager();
    //     gpu_driver_->setImageResourceManager(rm);
    // }

    if (!gpu_driver_ || !gpu_driver_->initialize()) {
        gpu_driver_.reset();
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
