#include "view.hpp"
#include <cstdio>
#include <string>
#include <cstring>
#include <functional>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <chrono>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_filesystem.h>
#include "log.h"
#include "../dom/dom_manager.hpp"
#include "../dom/event_system.hpp"
#include "../dom/focus_manager.hpp"
#include "../dom/input_element.hpp"
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
#include "../animation/animation_controller.hpp"

namespace dong {

namespace {

using dong::dom::DOMNodePtr;

DOMNodePtr hitTestRecursive(const DOMNodePtr& node, dong::layout::Engine* layout_engine,
                            int32_t x, int32_t y, int depth = 0) {
    if (!node || !layout_engine) return nullptr;

    const auto* layout = layout_engine->getLayout(node);
    if (!layout) return nullptr;
    
    float lx = layout->x;
    float ly = layout->y;
    float w = layout->width;
    float h = layout->height;
    
    // 调试：打印 input 元素的布局信息
    if (node->getTagName() == "input") {
        DONG_LOG_INFO("[hitTest] input id=%s bounds=(%.1f,%.1f,%.1f,%.1f) test=(%d,%d)",
                      node->getAttribute("id").c_str(), lx, ly, w, h, x, y);
    }
    
    // 检查点是否在当前节点范围内
    bool in_bounds = (x >= lx && x <= lx + w && y >= ly && y <= ly + h);
    
    
    if (!in_bounds) {
        return nullptr;  // 不在范围内，直接返回
    }
    
    // 在范围内，先检查子节点（深度优先，返回最深的命中元素）
    // 按 z-index 逆序遍历，优先返回 z-index 高的元素
    const auto& children = node->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto child_hit = hitTestRecursive(*it, layout_engine, x, y, depth + 1);
        if (child_hit) {
            return child_hit;  // 找到子节点命中，立即返回
        }
    }
    
    // 没有子节点命中，返回当前节点
    return node;
}

DOMNodePtr hitTestElementAt(dong::dom::Manager* dom_mgr, dong::layout::Engine* layout_engine,
                            int32_t x, int32_t y) {
    if (!dom_mgr || !layout_engine) return nullptr;
    auto root = dom_mgr->getRoot();
    if (!root) return nullptr;
    return hitTestRecursive(root, layout_engine, x, y);
}

void debugLogLayerTreeIfEnabled(const dong::render::LayerTree& tree) {
    const char* env = std::getenv("DONG_DEBUG_LAYER_TREE");
    if (!env || env[0] != '1') {
        return;
    }

    SDL_Log("[LayerTree] nodes=%zu, root=%d",
            tree.nodes.size(),
            tree.root_index);

    for (std::size_t i = 0; i < tree.nodes.size(); ++i) {
        const auto& node = tree.nodes[i];
        SDL_Log("[LayerTree] node[%zu]: id=%llu parent=%d type=%d bounds=(%.1f,%.1f,%.1f,%.1f) opacity=%.3f surface=%d dirty(content=%d,transform=%d,opacity=%d,scroll=%d)",
                i,
                static_cast<unsigned long long>(node.id),
                node.parent,
                static_cast<int>(node.type),
                node.bounds.x,
                node.bounds.y,
                node.bounds.width,
                node.bounds.height,
                node.opacity,
                node.is_surface ? 1 : 0,
                node.content_dirty ? 1 : 0,
                node.transform_dirty ? 1 : 0,
                node.opacity_dirty ? 1 : 0,
                node.scroll_dirty ? 1 : 0);
    }
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
      focus_manager(std::make_unique<dom::FocusManager>()),
      js_bindings(std::make_unique<script::JSBindings>(
          script_engine.get(),
          dom_manager.get(),
          event_dispatcher.get()
      )) {
    // 设置焦点管理器的事件分发器
    focus_manager->setEventDispatcher(event_dispatcher.get());
    // Defer JS bindings initialization until after first HTML load / script eval
    painter = std::make_unique<render::Painter>(render_surface.get());
}

View::~View() {
    // 显式清理 GPU 资源，确保在外部 window/device 被销毁前释放引用
    // 这很重要，因为 gpu_driver_ 和 shader_manager_ 可能持有外部传入的 window/device 指针
    
    // 首先清理 GPU 驱动，它可能持有 window/device 的引用
    if (gpu_driver_) {
        gpu_driver_.reset();
    }
    
    // 在设备仍然有效时清理 ShaderManager（它需要设备来释放 shader）
    // 必须在 gpu_device_ 被重置前清理
    if (shader_manager_ && gpu_device_ && gpu_device_->isInitialized()) {
        shader_manager_->releaseAll();
    }
    shader_manager_.reset();
    
    // 清理其他 GPU 相关资源
    gpu_painter_.reset();
    gpu_surface_.reset();
    
    // gpu_device_ 如果是 adoptExternal 的，由外部管理生命周期
    // 我们只需要重置指针，避免在析构时访问已销毁的设备
    // unique_ptr 会自动处理，但我们需要确保顺序正确
    gpu_device_.reset();
    
    // 其他资源通过 unique_ptr 自动清理
}

void View::load_html(const char* html) {
    SDL_Log("[View::load_html] Entry");
    if (!html || !dom_manager) {
        SDL_Log("[View::load_html] Early return: html=%p, dom_manager=%p", html, dom_manager.get());
        return;
    }
    
    SDL_Log("[View::load_html] HTML length=%zu", strlen(html));
    SDL_Log("[View::load_html] Calling dom_manager->loadHTML...");
    if (dom_manager->loadHTML(html)) {
        SDL_Log("[View::load_html] loadHTML succeeded");
        // DOM 树被整体替换，清理 JS 侧的节点映射和事件监听
        if (js_bindings) {
            SDL_Log("[View::load_html] Calling js_bindings->resetForNewDOM...");
            js_bindings->resetForNewDOM();
            SDL_Log("[View::load_html] resetForNewDOM done");
        }
        // 标记需要重新渲染
        markNeedsRepaint();
        // 新的 DOM 树需要重新布局
        auto root = dom_manager->getRoot();
        if (root) {
            root->markLayoutDirty();
        }
        
        // 执行 <script> 标签中的 JavaScript 代码
        if (script_engine && dom_manager) {
            // 确保 JS bindings 已初始化（包括 console 等全局对象）
            ensureJSBindingsInitialized();
            
            auto scripts = dom_manager->getElementsByTagName("script");
            SDL_Log("[View::load_html] Found %zu script tags", scripts.size());
            
            // 调试：打印所有标签
            auto root = dom_manager->getRoot();
            if (root) {
                std::function<void(const dom::DOMNodePtr&, int)> printTree = [&](const dom::DOMNodePtr& node, int depth) {
                    if (!node) return;
                    std::string indent(depth * 2, ' ');
                    if (node->getType() == dom::DOMNode::NodeType::ELEMENT) {
                        SDL_Log("[View::load_html] %s<%s>", indent.c_str(), node->getTagName().c_str());
                    }
                    for (const auto& child : node->getChildren()) {
                        printTree(child, depth + 1);
                    }
                };
                SDL_Log("[View::load_html] DOM Tree:");
                printTree(root, 0);
            }
            
            for (const auto& script : scripts) {
                if (script) {
                    std::string code;
                    
                    // 检查是否有 src 属性（外部脚本）
                    std::string src = script->getAttribute("src");
                    if (!src.empty()) {
                        SDL_Log("[View::load_html] Loading external script: %s", src.c_str());
                        
                        // 尝试直接打开文件
                        std::ifstream file(src);
                        if (!file.is_open()) {
                            // 如果失败，尝试使用 SDL 基础路径
                            const char* basePath = SDL_GetBasePath();
                            if (basePath) {
                                std::string fullPath = std::string(basePath) + src;
                                SDL_Log("[View::load_html] Trying with base path: %s", fullPath.c_str());
                                file.open(fullPath);
                            }
                        }
                        
                        if (file.is_open()) {
                            std::stringstream buffer;
                            buffer << file.rdbuf();
                            code = buffer.str();
                            SDL_Log("[View::load_html] Loaded external script (%zu chars)", code.length());
                        } else {
                            SDL_Log("[View::load_html] Failed to load external script: %s", src.c_str());
                        }
                    } else {
                        // 获取 script 标签的文本内容（inline 脚本）
                        SDL_Log("[View::load_html] Script tag has %zu children", script->getChildren().size());
                        for (const auto& child : script->getChildren()) {
                            if (child) {
                                SDL_Log("[View::load_html] Script child type=%d", (int)child->getType());
                                if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                                    code += child->getTextContent();
                                }
                            }
                        }
                    }
                    
                    if (!code.empty()) {
                        SDL_Log("[View::load_html] Executing script (%zu chars)", code.length());
                        script_engine->eval(code);
                    } else {
                        SDL_Log("[View::load_html] Script tag is empty");
                    }
                }
            }
        }
        
        SDL_Log("[View::load_html] Done");
    } else {
        SDL_Log("[View::load_html] loadHTML failed");
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

    markNeedsRepaint();

    // 尺寸变化会影响整个布局
    if (dom_manager) {
        auto root = dom_manager->getRoot();
        if (root) {
            root->markLayoutDirty();
        }
    }
}

void View::update() {
    // Update animations
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double current_time = std::chrono::duration<double>(now - start_time).count();
    
    auto& anim_controller = animation::getAnimationController();
    if (anim_controller.hasActiveAnimations()) {
        anim_controller.update(current_time);
        markNeedsRepaint();
    }
    
    if (script_engine) {
        script_engine->processPendingTasks();
    }

    if (layout_engine && dom_manager) {
        auto root = dom_manager->getRoot();
        if (root && root->isLayoutDirty()) {
            layout_engine->calculateLayout(root, static_cast<float>(width_), static_cast<float>(height_));
            markNeedsRepaint();
            
            // 输出布局调试信息
            auto htmls = dom_manager->getElementsByTagName("html");
            if (!htmls.empty()) {
                const auto* layout = layout_engine->getLayout(htmls[0]);
                if (layout) {
                    DONG_LOG_DEBUG("[View::update Layout] html: at (%.1f, %.1f) size %.1fx%.1f",
                           layout->x, layout->y, layout->width, layout->height);
                }
            }
            
            auto bodies = dom_manager->getElementsByTagName("body");
            if (!bodies.empty()) {
                const auto* layout = layout_engine->getLayout(bodies[0]);
                if (layout) {
                    DONG_LOG_DEBUG("[View::update Layout] body: at (%.1f, %.1f) size %.1fx%.1f",
                           layout->x, layout->y, layout->width, layout->height);
                }
            }
            
            auto h1s = dom_manager->getElementsByTagName("h1");
            if (!h1s.empty()) {
                const auto* layout = layout_engine->getLayout(h1s[0]);
                if (layout) {
                    DONG_LOG_DEBUG("[View::update Layout] h1: at (%.1f, %.1f) size %.1fx%.1f",
                           layout->x, layout->y, layout->width, layout->height);
                }
            }
            
            auto divs = dom_manager->getElementsByTagName("div");
            for (size_t i = 0; i < divs.size() && i < 3; i++) {
                const auto* layout = layout_engine->getLayout(divs[i]);
                if (layout) {
                    DONG_LOG_DEBUG("[View::update Layout] div[%zu]: at (%.1f, %.1f) size %.1fx%.1f",
                           i, layout->x, layout->y, layout->width, layout->height);
                }
            }
        }
    }

    // GPU 路径：DisplayList → GPU 渲染
    // 注意：对于 swapchain 渲染，每帧都必须获取 swapchain 纹理并提交命令
    // 否则会导致闪烁（因为 swapchain 是双缓冲/三缓冲的）
    if (use_gpu_ && gpu_driver_ && painter && dom_manager) {
        auto root = dom_manager->getRoot();
        if (!root) {
            return;
        }
        
        // 确保缓存命令列表已初始化
        if (!cached_cmd_list_) {
            cached_cmd_list_ = std::make_unique<render::GPUCommandList>();
            commands_dirty_ = true;
        }
        
        // 只在内容变化时重新构建 DisplayList 和 GPUCommandList
        bool need_rebuild = commands_dirty_;
        DONG_LOG_VERBOSE("[View::update] need_rebuild=%d commands_dirty_=%d", need_rebuild ? 1 : 0, commands_dirty_ ? 1 : 0);
        
        if (need_rebuild) {
            DONG_LOG_DEBUG("[View::update] Building DisplayList...");
            const render::DisplayList& dl = painter->buildDisplayList(root, layout_engine.get());
            DONG_LOG_DEBUG("[View::update] DisplayList built with %zu items", dl.items.size());
            
            // 缓存编译后的命令列表
            cached_cmd_list_->commands.clear();
            cached_cmd_list_->sorted_draw_indices.clear();
            cached_cmd_list_->draw_batches.clear();
            
            render::GPUCompiler compiler;
            DONG_LOG_DEBUG("[View::update] Compiling DisplayList to GPUCommandList...");
            const render::LayerTree& layer_tree = painter->getLayerTree();
            debugLogLayerTreeIfEnabled(layer_tree);
            compiler.compile(dl, *cached_cmd_list_, &layer_tree);
            DONG_LOG_DEBUG("[View::update] GPUCommandList compiled with %zu commands", cached_cmd_list_->commands.size());
            
            // 清除命令脏标记（下次只有在 markNeedsRepaint 时才会重建）
            commands_dirty_ = false;

            // 清除 layout dirty 标记
            root->clearLayoutDirtyRecursive();
        }

        // 每帧都执行渲染（即使使用缓存的命令列表）
        // 关键：在 beginFrame() 之前预处理资源（如 glyph 纹理上传）
        // 这样可以避免在 render pass 中进行纹理上传导致的 GPU 状态冲突
        gpu_driver_->prepareResources(*cached_cmd_list_);
        gpu_driver_->beginFrame();
        gpu_driver_->execute(*cached_cmd_list_);
        gpu_driver_->endFrame();
        DONG_LOG_VERBOSE("[View::update] GPU frame complete");
        
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
    DONG_LOG_DEBUG("[View::renderToGPUTexture] Created offscreen texture %p size=%ux%u", (void*)offscreen_texture, width, height);
    
    // 2. 确保布局已计算
    auto root = dom_manager ? dom_manager->getRoot() : nullptr;
    if (!root) {
        SDL_Log("[View::renderToGPUTexture] No DOM root");
        SDL_ReleaseGPUTexture(device, offscreen_texture);
        return nullptr;
    }
    
    DONG_LOG_DEBUG("[View::renderToGPUTexture] root->isLayoutDirty() = %s", root->isLayoutDirty() ? "TRUE" : "FALSE");
    
    // 为离屏渲染强制执行一次完整布局，以确保 absolute / inline 等最新逻辑
    // 始终生效，而不是依赖增量 dirty 标记（截图对齐场景更看重正确性而非微观性能）。
    if (layout_engine) {
        DONG_LOG_DEBUG("[View::renderToGPUTexture] Calculating layout (offscreen pass, forced)...");
        root->markLayoutDirty();
        layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
        root->clearLayoutDirtyRecursive();
        DONG_LOG_DEBUG("[View::renderToGPUTexture] Layout calculated");
    }
    
    // 3. 构建 DisplayList 并编译为 GPUCommandList
    DONG_LOG_DEBUG("[View::renderToGPUTexture] Building DisplayList...");
    const render::DisplayList& dl = painter->buildDisplayList(root, layout_engine.get());
    DONG_LOG_DEBUG("[View::renderToGPUTexture] DisplayList has %zu items", dl.items.size());
    
    render::GPUCompiler compiler;
    render::GPUCommandList cmd_list;
    const render::LayerTree& layer_tree = painter->getLayerTree();
    debugLogLayerTreeIfEnabled(layer_tree);
    compiler.compile(dl, cmd_list, &layer_tree);
    DONG_LOG_DEBUG("[View::renderToGPUTexture] GPUCommandList has %zu commands", cmd_list.commands.size());
    
    // 4. 执行离屏渲染
    // 关键：在 beginFrame() 之前预处理资源（如 glyph 纹理上传）
    // 这样可以避免在 render pass 中进行纹理上传导致的 GPU 状态冲突
    // 
    // 首帧修复：在 prepareResources 之前等待 GPU 空闲，确保 atlas 纹理初始化完成。
    // GlyphAtlas::initialize() 会创建初始页面并填充全黑，这个操作使用独立的 command buffer。
    // 虽然内部使用了 fence 等待，但在某些驱动下可能存在缓存一致性问题。
    if (gpu_device_ && gpu_device_->isInitialized()) {
        SDL_WaitForGPUIdle(gpu_device_->getHandle());
    }
    gpu_driver_->prepareResources(cmd_list);
    gpu_driver_->beginFrameOffscreen(offscreen_texture, width, height);
    gpu_driver_->execute(cmd_list);
    gpu_driver_->endFrameOffscreen();
    
    DONG_LOG_DEBUG("[View::renderToGPUTexture] Successfully rendered to GPU texture %u x %u", width, height);
    
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
    DONG_LOG_DEBUG("[View::renderOffscreen] Got texture %p, now downloading...", (void*)offscreen_texture);
    
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
    
    // 关键：读回（Download）路径要用 fence 来精确等待 command buffer 完成。
    // SDL 文档明确建议：提交 download 的 command buffer 后用
    // SDL_SubmitGPUCommandBufferAndAcquireFence + SDL_WaitForGPUFences。
    // 否则在部分后端/驱动下，首帧可能 map 到尚未填充完的 download buffer，表现为截图缺字/缺内容。
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
    if (fence) {
        SDL_GPUFence* fences[] = { fence };
        if (!SDL_WaitForGPUFences(device, true, fences, 1)) {
            SDL_Log("[View::renderOffscreen] SDL_WaitForGPUFences failed: %s", SDL_GetError());
            SDL_WaitForGPUIdle(device);
        }
        SDL_ReleaseGPUFence(device, fence);
    } else {
        SDL_Log("[View::renderOffscreen] SDL_SubmitGPUCommandBufferAndAcquireFence failed: %s", SDL_GetError());
        SDL_SubmitGPUCommandBuffer(cmd);
        SDL_WaitForGPUIdle(device);
    }

    
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
    
    return true;
}

bool View::eval_script(const char* code) {
    DONG_LOG_DEBUG("[View::eval_script] Entry, code=%p, script_engine=%p", (void*)code, (void*)script_engine.get());
    if (!code || !script_engine) {
        SDL_Log("[View::eval_script] Early return: null code or script_engine");
        return false;
    }
    DONG_LOG_DEBUG("[View::eval_script] Calling ensureJSBindingsInitialized...");
    ensureJSBindingsInitialized();
    DONG_LOG_DEBUG("[View::eval_script] ensureJSBindingsInitialized done, calling script_engine->eval...");
    bool result = script_engine->eval(std::string(code));
    DONG_LOG_DEBUG("[View::eval_script] eval returned %d", result ? 1 : 0);
    return result;
}

// 【缺口3】执行脚本并返回结果
std::string View::eval_script_with_return(const char* code) {
    if (!code || !script_engine) return "";
    ensureJSBindingsInitialized();
    last_eval_return_value_ = script_engine->evalWithReturn(std::string(code));
    return last_eval_return_value_;
}

void View::markNeedsRepaint() {
    commands_dirty_ = true;
    if (render_surface) {
        render_surface->markDirty();
    }
}

void View::ensureJSBindingsInitialized() {
    SDL_Log("[View::ensureJSBindingsInitialized] this=%p, initialized=%d, js_bindings=%p, script_engine=%p",
            (void*)this, js_bindings_initialized_ ? 1 : 0, (void*)js_bindings.get(), (void*)script_engine.get());
    if (js_bindings_initialized_ || !js_bindings || !script_engine) {
        SDL_Log("[View::ensureJSBindingsInitialized] this=%p Early return (already initialized)", (void*)this);
        return;
    }
    JSContext* ctx = script_engine->getContext();
    SDL_Log("[View::ensureJSBindingsInitialized] this=%p Got JSContext=%p", (void*)this, (void*)ctx);
    if (!ctx) {
        SDL_Log("[View::ensureJSBindingsInitialized] this=%p ctx is null, returning", (void*)this);
        return;
    }
    SDL_Log("[View::ensureJSBindingsInitialized] this=%p Calling js_bindings->initialize()...", (void*)this);
    js_bindings->initialize();
    SDL_Log("[View::ensureJSBindingsInitialized] this=%p initialize() done", (void*)this);
    js_bindings_initialized_ = true;
}

void View::handle_mouse_move(int32_t x, int32_t y) {
    last_mouse_x_ = x;
    last_mouse_y_ = y;
    dispatchMouseEventToJS("mousemove", last_mouse_x_, last_mouse_y_, 0);
}

const std::string& View::getCursorAt(int32_t x, int32_t y) {
    static const std::string default_cursor = "auto";
    
    if (!dom_manager || !layout_engine) {
        cached_cursor_style_ = default_cursor;
        return cached_cursor_style_;
    }
    
    auto element = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
    if (!element) {
        cached_cursor_style_ = default_cursor;
        return cached_cursor_style_;
    }
    
    // Walk up the tree to find the first element with a non-auto cursor
    auto current = element;
    while (current) {
        const auto& cursor = current->getComputedStyle().cursor;
        if (!cursor.empty() && cursor != "auto") {
            cached_cursor_style_ = cursor;
            return cached_cursor_style_;
        }
        current = current->getParent();
    }
    
    cached_cursor_style_ = default_cursor;
    return cached_cursor_style_;
}

void View::handle_mouse_down(int32_t button) {
    dispatchMouseEventToJS("mousedown", last_mouse_x_, last_mouse_y_, button);
}

void View::handle_mouse_up(int32_t button) {
    dispatchMouseEventToJS("mouseup", last_mouse_x_, last_mouse_y_, button);
    dispatchMouseEventToJS("click", last_mouse_x_, last_mouse_y_, button);
    
    // 处理焦点：点击时尝试聚焦被点击的元素
    if (focus_manager && dom_manager && layout_engine) {
        auto clicked = hitTestElementAt(dom_manager.get(), layout_engine.get(), 
                                        last_mouse_x_, last_mouse_y_);
        DONG_LOG_INFO("[View::handle_mouse_up] clicked at (%d,%d), element=%p tag=%s",
                       last_mouse_x_, last_mouse_y_,
                       clicked.get(),
                       clicked ? clicked->getTagName().c_str() : "null");
        if (clicked) {
            bool changed = focus_manager->focusOnClick(clicked);
            DONG_LOG_INFO("[View::handle_mouse_up] focusOnClick returned %d, focused=%p",
                           changed ? 1 : 0,
                           focus_manager->getFocusedElement().get());
        }
    }
}

void View::handle_key_down(uint32_t key_code) {
    // SDL3 键码定义
    constexpr uint32_t SDLK_TAB = 9;
    constexpr uint32_t SDLK_BACKSPACE = 8;
    constexpr uint32_t SDLK_DELETE = 127;
    constexpr uint32_t SDLK_LEFT = 0x40000050;
    constexpr uint32_t SDLK_RIGHT = 0x4000004F;
    
    // Tab 键焦点切换
    if (key_code == SDLK_TAB && focus_manager && dom_manager) {
        focus_manager->moveFocus(dom_manager->getRoot(), false);
        return;
    }
    
    // 处理可编辑元素的键盘输入
    if (focus_manager) {
        auto focused = focus_manager->getFocusedElement();
        if (focused && dom::isEditableElement(focused)) {
            auto* state = dom::getInputState(focused);
            if (state) {
                bool handled = false;
                
                if (key_code == SDLK_BACKSPACE) {
                    state->deleteBackward();
                    handled = true;
                } else if (key_code == SDLK_DELETE) {
                    state->deleteForward();
                    handled = true;
                } else if (key_code == SDLK_LEFT) {
                    state->moveCursor(-1);
                    handled = true;
                } else if (key_code == SDLK_RIGHT) {
                    state->moveCursor(1);
                    handled = true;
                }
                
                if (handled) {
                    // 同步到 DOM 属性
                    focused->setAttribute("value", state->getValue());
                    // 标记需要重新渲染
                    markNeedsRepaint();
                }
            }
        }
    }
    
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

void View::handle_mouse_wheel(float delta_x, float delta_y) {
    DONG_LOG_INFO("[View::handle_mouse_wheel] delta=(%.2f, %.2f) at (%d,%d)", 
                  delta_x, delta_y, last_mouse_x_, last_mouse_y_);
    
    // 先检查 hitTest 是否能找到元素
    auto hit_element = hitTestElementAt(dom_manager.get(), layout_engine.get(), 
                                        last_mouse_x_, last_mouse_y_);
    DONG_LOG_INFO("[View::handle_mouse_wheel] hitTestElementAt returned %p tag=%s",
                  hit_element.get(),
                  hit_element ? hit_element->getTagName().c_str() : "null");
    
    // 查找鼠标位置下的滚动容器
    auto scroll_container = findScrollContainerAt(last_mouse_x_, last_mouse_y_);
    DONG_LOG_INFO("[View::handle_mouse_wheel] scroll_container=%p tag=%s",
                  scroll_container.get(),
                  scroll_container ? scroll_container->getTagName().c_str() : "null");
    
    if (scroll_container) {
        // 滚动速度系数
        constexpr float kScrollSpeed = 20.0f;
        float old_scroll_y = scroll_container->getScrollY();
        
        // 调试：检查 isScrollContainer 状态
        DONG_LOG_INFO("[View::handle_mouse_wheel] container id=%s overflow=%s isScrollContainer=%d",
                      scroll_container->getAttribute("id").c_str(),
                      scroll_container->getComputedStyle().overflow.c_str(),
                      scroll_container->isScrollContainer() ? 1 : 0);
        
        // dong 约定：delta_y 正值=向下滚动（内容向上移动，scroll_y 增加）
        // 调用方（如 SDL3InputAdapter）负责将平台值转换为此语义
        scroll_container->scrollBy(delta_x * kScrollSpeed, delta_y * kScrollSpeed);
        DONG_LOG_INFO("[View::handle_mouse_wheel] scrollBy: old_y=%.1f new_y=%.1f",
                      old_scroll_y, scroll_container->getScrollY());
        
        // 标记需要重新渲染
        markNeedsRepaint();
        
        // 触发 wheel 事件到 JS
        dispatchWheelEventToJS(delta_x, delta_y);
    }
}

void View::handle_text_input(const char* text) {
    if (!text || !text[0]) return;
    
    DONG_LOG_INFO("[View::handle_text_input] text='%s'", text);
    
    // 获取当前焦点元素
    dom::DOMNodePtr focused;
    if (focus_manager) {
        focused = focus_manager->getFocusedElement();
    }
    
    DONG_LOG_INFO("[View::handle_text_input] focused=%p, tag=%s", 
                   focused.get(), 
                   focused ? focused->getTagName().c_str() : "null");
    
    // 如果焦点元素是可编辑的，更新其内容
    if (focused && dom::isEditableElement(focused)) {
        auto* state = dom::getInputState(focused);
        DONG_LOG_INFO("[View::handle_text_input] isEditable=true, state=%p", state);
        if (state) {
            state->insertText(text);
            // 同步到 DOM 属性
            focused->setAttribute("value", state->getValue());
            DONG_LOG_INFO("[View::handle_text_input] new value='%s'", state->getValue().c_str());
            // 标记需要重新渲染
            markNeedsRepaint();
        }
    } else {
        DONG_LOG_INFO("[View::handle_text_input] not editable or no focus");
    }
    
    // 触发 input 事件到 JS
    dispatchTextInputEventToJS(text);
}

void View::dispatchWheelEventToJS(float delta_x, float delta_y) {
    if (!script_engine || !js_bindings || !event_dispatcher) return;
    
    // 查找事件目标
    auto target = hitTestElementAt(dom_manager.get(), layout_engine.get(), 
                                   last_mouse_x_, last_mouse_y_);
    if (!target && dom_manager) {
        target = dom_manager->getRoot();
    }
    if (!target) return;
    
    // 创建 wheel 事件
    dom::Event event = event_dispatcher->createMouseEvent(
        dom::EventType::MOUSE_MOVE,  // TODO: 添加 WHEEL 事件类型
        last_mouse_x_, last_mouse_y_, 0);
    event.type_name = "wheel";
    event.target = target;
    event.current_target = target;
    // TODO: 将 delta_x/delta_y 存入事件数据
    
    event_dispatcher->dispatch(event);
}

void View::dispatchTextInputEventToJS(const char* text) {
    if (!script_engine || !js_bindings || !event_dispatcher) return;
    if (!text) return;
    
    // TODO: 获取当前焦点元素作为目标
    // 暂时使用 body 或第一个 input 元素
    dom::DOMNodePtr target;
    if (dom_manager) {
        auto inputs = dom_manager->getElementsByTagName("input");
        if (!inputs.empty()) {
            target = inputs[0];
        } else {
            auto bodies = dom_manager->getElementsByTagName("body");
            if (!bodies.empty()) {
                target = bodies[0];
            }
        }
    }
    if (!target) return;
    
    // 创建 input 事件
    dom::Event event = event_dispatcher->createEvent(dom::EventType::INPUT);
    event.target = target;
    event.current_target = target;
    event.data["inputText"] = text;
    
    event_dispatcher->dispatch(event);
}

dom::DOMNodePtr View::findScrollContainerAt(int32_t x, int32_t y) {
    if (!dom_manager || !layout_engine) return nullptr;
    
    // 首先找到点击位置的元素
    auto element = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
    if (!element) return nullptr;
    
    // 向上查找滚动容器
    auto current = element;
    while (current) {
        if (current->isScrollContainer()) {
            return current;
        }
        current = current->getParent();
    }
    
    return nullptr;
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

    markNeedsRepaint();
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
    markNeedsRepaint();
}

} // namespace dong
