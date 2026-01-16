#include "view.hpp"
#include <cstdio>
#include <string>
#include <cstring>
#include <functional>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cctype>

#include <algorithm>
#include <unordered_set>
#include <filesystem>
#include <SDL3/SDL_log.h>


#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_filesystem.h>
#include "log.h"
#include "profiler.h"
#include "../dom/dom_manager.hpp"
#include "../dom/style_engine.hpp"
#include "../dom/event_system.hpp"
#include "../dom/focus_manager.hpp"
#include "../dom/input_element.hpp"

#include "../render/render_surface.hpp"
#include "../render/painter.hpp"
#include "../render/resource_manager.hpp"
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
                            float x, float y, int depth = 0) {
    if (!node || !layout_engine) return nullptr;

    const auto* layout = layout_engine->getLayout(node);
    if (!layout) return nullptr;

    float lx = layout->x;
    float ly = layout->y;
    float w = layout->width;
    float h = layout->height;

    // 调试：打印 input 元素的布局信息
    if (node->getTagName() == "input") {
        DONG_LOG_DEBUG("[hitTest] input id=%s bounds=(%.1f,%.1f,%.1f,%.1f) test=(%.1f,%.1f)",
                      node->getAttribute("id").c_str(), lx, ly, w, h, x, y);
    }

    // 检查点是否在当前节点范围内
    bool in_bounds = (x >= lx && x <= lx + w && y >= ly && y <= ly + h);

    if (!in_bounds) {
        return nullptr;  // 不在范围内，直接返回
    }

    // 滚动容器的子树 hit-test 需要在“内容坐标系”里：screen_point + scroll_offset
    float child_x = x;
    float child_y = y;
    if (node->isScrollContainer()) {
        child_x += node->getScrollX();
        child_y += node->getScrollY();
    }

    // 在范围内，先检查子节点（深度优先，返回最深的命中元素）
    // 按 z-index 逆序遍历，优先返回 z-index 高的元素
    const auto& children = node->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto child_hit = hitTestRecursive(*it, layout_engine, child_x, child_y, depth + 1);
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
    return hitTestRecursive(root, layout_engine, static_cast<float>(x), static_cast<float>(y));
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
      resource_manager_(std::make_unique<render::ResourceManager>()),
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

    // B: 释放离屏纹理缓存（如果设备仍然有效）
    if (offscreen_texture_cache_) {
        SDL_GPUDevice* dev = offscreen_texture_cache_device_;
        if (!dev && gpu_device_ && gpu_device_->isInitialized()) {
            dev = gpu_device_->getHandle();
        }
        if (dev) {
            SDL_ReleaseGPUTexture(dev, offscreen_texture_cache_);
        }
        offscreen_texture_cache_ = nullptr;
        offscreen_texture_cache_device_ = nullptr;
        offscreen_texture_cache_width_ = 0;
        offscreen_texture_cache_height_ = 0;
    }

    // gpu_device_ 如果是 adoptExternal 的，由外部管理生命周期
    // 我们只需要重置指针，避免在析构时访问已销毁的设备
    // unique_ptr 会自动处理，但我们需要确保顺序正确
    gpu_device_.reset();

    
    // 其他资源通过 unique_ptr 自动清理
}

void View::setResourceRoot(const std::string& root) {
    if (resource_manager_) {
        resource_manager_->setResourceRoot(root);
    }
}

void View::setPlugin(const dong_plugin_vtable_t* plugin, void* plugin_user) {
    plugin_ = plugin;
    plugin_user_ = plugin_user;

    // If video capability is missing, close any existing players (best effort).
    const bool video_ok = (plugin_ && (plugin_->info.capabilities & DONG_PLUGIN_CAP_VIDEO) && plugin_->video_open && plugin_->video_close);
    if (!video_ok) {
        for (auto& kv : video_states_) {
            closeVideo(kv.second);
        }
        video_states_.clear();
    }
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
                        
                        auto isAbsolutePath = [](const std::string& p) -> bool {
                            if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':') {
                                return true; // Windows drive path
                            }
                            if (!p.empty() && (p[0] == '/' || p[0] == '\\')) {
                                return true; // Unix absolute or UNC-like
                            }

                            return false;
                        };

                        // Basic URL handling: support file://, reject http(s)/data.
                        std::string script_path = src;
                        if (script_path.rfind("http://", 0) == 0 || script_path.rfind("https://", 0) == 0 || script_path.rfind("data:", 0) == 0) {
                            SDL_Log("[View::load_html] Unsupported external script URL: %s", src.c_str());
                        }
                        if (script_path.rfind("file://", 0) == 0) {
                            script_path = script_path.substr(std::string("file://").size());
                            // Windows file URL often looks like "/d:/xxx"; strip the leading '/' if it precedes a drive letter.
                            if (script_path.size() >= 3 && script_path[0] == '/' && std::isalpha(static_cast<unsigned char>(script_path[1])) && script_path[2] == ':') {
                                script_path.erase(script_path.begin());
                            }
                        }

                        // 解析顺序：
                        // 1) 直接当作路径打开
                        // 2) 相对路径：以 View 的 resource_root（通常是 HTML 文件所在目录）为基准拼接
                        // 3) 回退：SDL_GetBasePath()
                        std::ifstream file(script_path, std::ios::binary);
                        if (!file.is_open() && resource_manager_ && !isAbsolutePath(script_path)) {
                            const std::string& root = resource_manager_->getResourceRoot();
                            if (!root.empty()) {
                                try {
                                    namespace fs = std::filesystem;
                                    fs::path resolved = fs::path(root) / fs::path(script_path);
                                    const std::string fullPath = resolved.lexically_normal().string();
                                    SDL_Log("[View::load_html] Trying with resource root: %s", fullPath.c_str());
                                    file.open(fullPath, std::ios::binary);
                                } catch (...) {
                                    // ignore
                                }
                            }
                        }

                        if (!file.is_open()) {
                            // 如果失败，尝试使用 SDL 基础路径
                            const char* basePath = SDL_GetBasePath();
                            if (basePath) {
                                std::string fullPath = std::string(basePath) + script_path;
                                SDL_Log("[View::load_html] Trying with base path: %s", fullPath.c_str());
                                file.open(fullPath, std::ios::binary);
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
    DONG_PROFILE_SCOPE_CAT("View::update", "frame");
    
    // Update animations
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double current_time = std::chrono::duration<double>(now - start_time).count();
    last_wall_time_sec_ = current_time;

    // Tick media elements (e.g. <video>) before building command lists.
    syncVideoElements(current_time);
    
    auto& anim_controller = animation::getAnimationController();
    if (anim_controller.hasActiveAnimations()) {
        DONG_PROFILE_SCOPE_CAT("Animation::update", "animation");
        anim_controller.update(current_time);
        markNeedsRepaint();
    }
    
    if (script_engine) {
        DONG_PROFILE_SCOPE_CAT("Script::processTasks", "script");
        script_engine->processPendingTasks();
    }

    if (layout_engine && dom_manager) {
        auto root = dom_manager->getRoot();
        if (root && commands_dirty_) {
            // Styles may depend on runtime state (:hover/:active) or JS-driven attribute changes.
            // 优化策略3：使用索引加速的增量样式计算
            if (auto* se = dom_manager->getStyleEngine()) {
                DONG_PROFILE_SCOPE_CAT("Style::compute", "style");
                se->computeStylesIncremental(root);
            }
        }
        if (root && root->isLayoutDirty()) {
            {
                DONG_PROFILE_SCOPE_CAT("Layout::calculate", "layout");
                layout_engine->calculateLayout(root, static_cast<float>(width_), static_cast<float>(height_));
            }
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
        DONG_LOG_DEBUG("[View::update] need_rebuild=%d commands_dirty_=%d", need_rebuild ? 1 : 0, commands_dirty_ ? 1 : 0);
        
        if (need_rebuild) {
            DONG_LOG_DEBUG("[View::update] Building DisplayList...");
            const render::DisplayList& dl = [&]() {
                DONG_PROFILE_SCOPE_CAT("Painter::buildDisplayList", "render");
                return painter->buildDisplayList(root, layout_engine.get());
            }();
            DONG_LOG_DEBUG("[View::update] DisplayList built with %zu items", dl.items.size());
            
            // 缓存编译后的命令列表
            cached_cmd_list_->commands.clear();
            cached_cmd_list_->sorted_draw_indices.clear();
            cached_cmd_list_->draw_batches.clear();
            
            render::GPUCompiler compiler;
            DONG_LOG_DEBUG("[View::update] Compiling DisplayList to GPUCommandList...");
            const render::LayerTree& layer_tree = painter->getLayerTree();
            
            // DEBUG: 强制打印LayerTree信息
            DONG_LOG_DEBUG("[LayerTree] nodes=%zu, root=%d", layer_tree.nodes.size(), layer_tree.root_index);
            for (std::size_t i = 0; i < layer_tree.nodes.size(); ++i) {
                const auto& node = layer_tree.nodes[i];
                DONG_LOG_DEBUG("[LayerTree] node[%zu]: id=%llu bounds=(%.1f,%.1f,%.1f,%.1f) opacity=%.3f",
                    i, static_cast<unsigned long long>(node.id),
                    node.bounds.x, node.bounds.y, node.bounds.width, node.bounds.height, node.opacity);
            }
            
            debugLogLayerTreeIfEnabled(layer_tree);
            {
                DONG_PROFILE_SCOPE_CAT("GPUCompiler::compile", "render");
                compiler.compile(dl, *cached_cmd_list_, &layer_tree);
            }
            DONG_LOG_DEBUG("[View::update] GPUCommandList compiled with %zu commands", cached_cmd_list_->commands.size());
            
            // 清除命令脏标记（下次只有在 markNeedsRepaint 时才会重建）
            DONG_LOG_DEBUG("[View::update] Setting commands_dirty_ = false");
            commands_dirty_ = false;

            // 清除 layout dirty 标记
            root->clearLayoutDirtyRecursive();
        }

        // 每帧都执行渲染（即使使用缓存的命令列表）
        // 关键：在 beginFrame() 之前预处理资源（如 glyph 纹理上传）
        // 这样可以避免在 render pass 中进行纹理上传导致的 GPU 状态冲突
        {
            DONG_PROFILE_SCOPE_CAT("GPU::prepareResources", "gpu");
            gpu_driver_->prepareResources(*cached_cmd_list_);
        }
        
        {
            DONG_PROFILE_SCOPE_CAT("GPU::frame", "gpu");
            gpu_driver_->beginFrame();
            uploadPendingVideoFrames();
            gpu_driver_->execute(*cached_cmd_list_);
            gpu_driver_->endFrame();
        }
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
    DONG_PROFILE_SCOPE_CAT("View::renderToGPUTexture", "render");

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

    // Tick media elements (e.g. <video>) for offscreen rendering too.
    // Otherwise autoplay videos won't advance when the embedder only calls renderToGPUTexture().
    {
        static auto start_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        double current_time = std::chrono::duration<double>(now - start_time).count();
        last_wall_time_sec_ = current_time;
        syncVideoElements(current_time);
    }

    // 1. 获取/创建离屏渲染目标纹理（使用UNORM格式，shader手动处理gamma）

    // B: 复用同一张纹理，避免每帧 Create/Release。
    SDL_GPUTexture* offscreen_texture = offscreen_texture_cache_;

    const bool need_new_texture = (!offscreen_texture_cache_ ||
                                  offscreen_texture_cache_device_ != device ||
                                  offscreen_texture_cache_width_ != width ||
                                  offscreen_texture_cache_height_ != height);

    if (need_new_texture) {
        if (offscreen_texture_cache_ && offscreen_texture_cache_device_) {
            SDL_ReleaseGPUTexture(offscreen_texture_cache_device_, offscreen_texture_cache_);
        }

        SDL_GPUTextureCreateInfo tex_info{};
        tex_info.type = SDL_GPU_TEXTURETYPE_2D;
        tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;  // UNORM格式，不自动gamma校正
        tex_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        tex_info.width = width;
        tex_info.height = height;
        tex_info.layer_count_or_depth = 1;
        tex_info.num_levels = 1;
        tex_info.sample_count = SDL_GPU_SAMPLECOUNT_1;

        offscreen_texture_cache_device_ = device;
        offscreen_texture_cache_width_ = width;
        offscreen_texture_cache_height_ = height;

        offscreen_texture_cache_ = SDL_CreateGPUTexture(device, &tex_info);
        if (!offscreen_texture_cache_) {
            SDL_Log("[View::renderToGPUTexture] Failed to create offscreen texture: %s", SDL_GetError());
            offscreen_texture_cache_device_ = nullptr;
            offscreen_texture_cache_width_ = 0;
            offscreen_texture_cache_height_ = 0;
            return nullptr;
        }
        DONG_LOG_DEBUG("[View::renderToGPUTexture] Created offscreen texture %p size=%ux%u", (void*)offscreen_texture_cache_, width, height);

        offscreen_texture = offscreen_texture_cache_;
        offscreen_texture_dirty_ = true;
    }

    // 2. 确保布局已计算
    auto root = dom_manager ? dom_manager->getRoot() : nullptr;
    if (!root) {
        SDL_Log("[View::renderToGPUTexture] No DOM root");
        // 不要在这里释放缓存纹理：它由 View 生命周期管理。
        // 若之前已有有效内容，继续返回旧纹理以避免 3D demo 采样闪烁。
        return offscreen_texture_cache_;
    }

    // 检查尺寸是否变化，如果变化则标记 dirty
    if (width != offscreen_cache_width_ || height != offscreen_cache_height_) {
        offscreen_commands_dirty_ = true;
        offscreen_texture_dirty_ = true;
        offscreen_cache_width_ = width;
        offscreen_cache_height_ = height;
    }

    // 优化策略1：只在 dirty 时重新计算样式/布局/构建命令列表
    // Offscreen 路径只看 offscreen_commands_dirty_ / layout dirty。
    // 注意：3D demo 只走 renderToGPUTexture()，不会走 View::update() 去清除 commands_dirty_；
    // 若这里也依赖 commands_dirty_，会导致每帧都 rebuild（DisplayList/GPUCommandList）并成为主要热点。
    bool needs_rebuild = offscreen_commands_dirty_ || root->isLayoutDirty() || !offscreen_cmd_list_cache_;

    // 环境变量强制全量重建（用于调试/正确性验证）
    static bool force_full_layout = (std::getenv("DONG_OFFSCREEN_FORCE_FULL_LAYOUT") != nullptr);
    if (force_full_layout) {
        needs_rebuild = true;
    }

    // 3D demo 友好：内容未变时直接复用上一帧纹理，避免每帧重复离屏渲染。
    if (!needs_rebuild && !offscreen_texture_dirty_) {
        return offscreen_texture_cache_;
    }

    if (needs_rebuild) {
        DONG_PROFILE_SCOPE_CAT("offscreen_rebuild", "render");

        // 计算样式（使用索引加速的增量计算）
        if (dom_manager) {
            if (auto* se = dom_manager->getStyleEngine()) {
                DONG_PROFILE_SCOPE_CAT("offscreen_computeStyles", "style");
                // 优化策略3：使用 computeStylesIncremental 替代 computeStyles
                // 内部使用规则索引加速匹配，减少每节点的规则扫描量
                se->computeStylesIncremental(root);
            }
        }

        DONG_LOG_DEBUG("[View::renderToGPUTexture] root->isLayoutDirty() = %s", root->isLayoutDirty() ? "TRUE" : "FALSE");

        // 计算布局（只在 dirty 时）
        if (layout_engine && (root->isLayoutDirty() || force_full_layout)) {
            DONG_PROFILE_SCOPE_CAT("offscreen_calculateLayout", "layout");
            DONG_LOG_DEBUG("[View::renderToGPUTexture] Calculating layout (offscreen pass)...");
            if (force_full_layout) {
                root->markLayoutDirty();
            }
            layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
            // 注意：不要在这里清 layout dirty。
            // LayerTree/layer_dirty 等增量策略可能依赖 isLayoutDirty() 判断本帧哪些 layer 需要重栅格。
            DONG_LOG_DEBUG("[View::renderToGPUTexture] Layout calculated");

        }

        // 3. 构建 DisplayList 并编译为 GPUCommandList
        const render::DisplayList* dl_ptr = nullptr;
        {
            DONG_PROFILE_SCOPE_CAT("offscreen_buildDisplayList", "render");
            DONG_LOG_DEBUG("[View::renderToGPUTexture] Building DisplayList...");
            dl_ptr = &painter->buildDisplayList(root, layout_engine.get());
            DONG_LOG_DEBUG("[View::renderToGPUTexture] DisplayList has %zu items", dl_ptr->items.size());
        }

        // 创建或重用缓存的命令列表
        if (!offscreen_cmd_list_cache_) {
            offscreen_cmd_list_cache_ = std::make_unique<render::GPUCommandList>();
        }
        offscreen_cmd_list_cache_->commands.clear();

        {
            DONG_PROFILE_SCOPE_CAT("offscreen_compile", "render");
            render::GPUCompiler compiler;
            const render::LayerTree& layer_tree = painter->getLayerTree();
            debugLogLayerTreeIfEnabled(layer_tree);
            compiler.compile(*dl_ptr, *offscreen_cmd_list_cache_, &layer_tree);
        }

        // build+compile 完成后再清 layout dirty，避免影响 LayerTree/layer_dirty 的增量判断。
        root->clearLayoutDirtyRecursive();

        DONG_LOG_DEBUG("[View::renderToGPUTexture] GPUCommandList has %zu commands", offscreen_cmd_list_cache_->commands.size());

        offscreen_commands_dirty_ = false;
        offscreen_resources_dirty_ = true;
        offscreen_texture_dirty_ = true;
        // 不清除 commands_dirty_，让主渲染路径自己管理

    }

    if (!offscreen_cmd_list_cache_) {
        SDL_Log("[View::renderToGPUTexture] No cached command list");
        return offscreen_texture_cache_;
    }

    // 4. 执行离屏渲染
    // 关键：在 beginFrame() 之前预处理资源（如 glyph 纹理上传）
    //
    // 优化策略2：SDL_WaitForGPUIdle 仅首帧执行
    // 首帧修复：在 prepareResources 之前等待 GPU 空闲，确保 atlas 纹理初始化完成。
    if (!offscreen_first_frame_done_ && gpu_device_ && gpu_device_->isInitialized()) {
        // 性能：每个 View 首帧都全局 WaitForGPUIdle 会造成严重卡顿（多屏场景会被放大）。
        // 默认关闭；如需验证同步相关问题，可设置 DONG_DEBUG_OFFSCREEN_WAIT_GPU_IDLE_FIRST_FRAME=1。
        static const bool kWaitGpuIdleFirstFrame = (std::getenv("DONG_DEBUG_OFFSCREEN_WAIT_GPU_IDLE_FIRST_FRAME") != nullptr);
        if (kWaitGpuIdleFirstFrame) {
            DONG_PROFILE_SCOPE_CAT("offscreen_WaitForGPUIdle", "gpu");
            SDL_WaitForGPUIdle(gpu_device_->getHandle());
        }
        offscreen_first_frame_done_ = true;
    }


    if (offscreen_resources_dirty_) {
        DONG_PROFILE_SCOPE_CAT("offscreen_prepareResources", "gpu");
        gpu_driver_->prepareResources(*offscreen_cmd_list_cache_);
        offscreen_resources_dirty_ = false;
    }


    {
        DONG_PROFILE_SCOPE_CAT("offscreen_execute", "gpu");
        gpu_driver_->beginFrameOffscreen(offscreen_texture, width, height);
        uploadPendingVideoFrames();
        gpu_driver_->execute(*offscreen_cmd_list_cache_);
        gpu_driver_->endFrameOffscreen();
    }


    offscreen_texture_dirty_ = false;

    DONG_LOG_DEBUG("[View::renderToGPUTexture] Successfully rendered to GPU texture %u x %u", width, height);

    // 返回纹理（View 内部缓存并复用；调用方不要释放）
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
        return false;
    }

    
    // 3. 执行纹理到传输缓冲区的拷贝
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        SDL_Log("[View::renderOffscreen] Failed to acquire command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, download_buffer);
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
        return false;
    }


    std::memcpy(out_pixels, mapped, width * height * 4);
    
    
    SDL_UnmapGPUTransferBuffer(device, download_buffer);
    
    // 5. 清理资源
    SDL_ReleaseGPUTransferBuffer(device, download_buffer);

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
    DONG_LOG_DEBUG("[View::markNeedsRepaint] Setting commands_dirty_ = true");
    commands_dirty_ = true;

    // Offscreen 渲染路径（renderToGPUTexture）有独立的命令缓存，需要同步置脏。
    // 否则 offscreen 只能依赖 commands_dirty_，但它在 offscreen 路径并不会被清除，
    // 容易造成每帧 rebuild。
    offscreen_commands_dirty_ = true;
    offscreen_texture_dirty_ = true;

    if (render_surface) {
        render_surface->markDirty();
    }
}


void View::ensureJSBindingsInitialized() {
    DONG_LOG_DEBUG("[View::ensureJSBindingsInitialized] this=%p, initialized=%d, js_bindings=%p, script_engine=%p",
            (void*)this, js_bindings_initialized_ ? 1 : 0, (void*)js_bindings.get(), (void*)script_engine.get());
    if (js_bindings_initialized_ || !js_bindings || !script_engine) {
        DONG_LOG_DEBUG("[View::ensureJSBindingsInitialized] this=%p Early return (already initialized)", (void*)this);
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

    // If user is dragging a scrollbar thumb, update scroll position.
    if (scroll_dragging_ && scroll_drag_container_) {
        const float my = static_cast<float>(last_mouse_y_);
        const float track_range = scroll_drag_track_h_ - scroll_drag_thumb_h_;
        if (track_range > 1.0f && scroll_drag_max_scroll_ > 0.0f) {
            float thumb_y = my - scroll_drag_offset_y_;
            thumb_y = std::clamp(thumb_y, scroll_drag_track_y_, scroll_drag_track_y_ + track_range);
            float ratio = (thumb_y - scroll_drag_track_y_) / track_range;
            scroll_drag_container_->scrollTo(scroll_drag_container_->getScrollX(), ratio * scroll_drag_max_scroll_);
            markNeedsRepaint();
        }
    }

    // Hover state for :hover
    if (dom_manager && layout_engine) {
        auto hit = hitTestElementAt(dom_manager.get(), layout_engine.get(), last_mouse_x_, last_mouse_y_);
        while (hit && hit->getType() != dom::DOMNode::NodeType::ELEMENT) {
            hit = hit->getParent();
        }

        if (hit != hovered_element_) {
            if (hovered_element_) {
                hovered_element_->setHovered(false);
            }
            hovered_element_ = hit;
            if (hovered_element_) {
                hovered_element_->setHovered(true);
            }
            markNeedsRepaint();
        }
    }

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
    // Start scrollbar thumb drag (left button only).
    // Note: dong_app uses SDL button codes; SDL_BUTTON_LEFT is 1.
    if (button == 1 && dom_manager && layout_engine) {
        auto container = findScrollContainerAt(last_mouse_x_, last_mouse_y_);
        if (container) {
            const auto* layout = layout_engine->getLayout(container);
            if (layout && layout->width > 0.0f && layout->height > 0.0f) {
                // Compute content height (max bottom of children)
                float content_bottom = layout->y;
                for (const auto& child : container->getChildren()) {
                    if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                        continue;
                    }
                    const auto* cl = layout_engine->getLayout(child);
                    if (!cl) continue;
                    float bottom = cl->y + cl->height;
                    if (bottom > content_bottom) content_bottom = bottom;
                }
                const float content_height = content_bottom - layout->y;
                const float visible_height = layout->height;

                if (content_height > visible_height + 1.0f) {
                    constexpr float kScrollbarWidth = 8.0f;
                    constexpr float kScrollbarMinThumbHeight = 20.0f;
                    constexpr float kScrollbarPadding = 2.0f;

                    const float track_x = layout->x + layout->width - kScrollbarWidth - kScrollbarPadding;
                    const float track_y = layout->y + kScrollbarPadding;
                    const float track_w = kScrollbarWidth;
                    const float track_h = layout->height - kScrollbarPadding * 2.0f;

                    const float max_scroll = content_height - visible_height;
                    const float thumb_height_ratio = visible_height / content_height;
                    const float thumb_h = std::max(kScrollbarMinThumbHeight, track_h * thumb_height_ratio);

                    float scroll_ratio = (max_scroll > 0.0f) ? (container->getScrollY() / max_scroll) : 0.0f;
                    scroll_ratio = std::clamp(scroll_ratio, 0.0f, 1.0f);

                    const float thumb_y = track_y + (track_h - thumb_h) * scroll_ratio;

                    const float mx = static_cast<float>(last_mouse_x_);
                    const float my = static_cast<float>(last_mouse_y_);
                    const bool in_track = (mx >= track_x && mx <= track_x + track_w && my >= track_y && my <= track_y + track_h);
                    const bool in_thumb = (mx >= track_x && mx <= track_x + track_w && my >= thumb_y && my <= thumb_y + thumb_h);
                    if (in_track && in_thumb) {
                        scroll_dragging_ = true;
                        scroll_drag_container_ = container;
                        scroll_drag_offset_y_ = my - thumb_y;
                        scroll_drag_track_y_ = track_y;
                        scroll_drag_track_h_ = track_h;
                        scroll_drag_thumb_h_ = thumb_h;
                        scroll_drag_max_scroll_ = max_scroll;
                    }
                }
            }
        }
    }

    // Active state for :active (left button only)
    if (button == 1 && dom_manager && layout_engine) {
        left_mouse_down_ = true;
        auto hit = hitTestElementAt(dom_manager.get(), layout_engine.get(), last_mouse_x_, last_mouse_y_);
        while (hit && hit->getType() != dom::DOMNode::NodeType::ELEMENT) {
            hit = hit->getParent();
        }
        if (active_element_ && active_element_ != hit) {
            active_element_->setActive(false);
            active_element_.reset();
        }
        active_element_ = hit;
        if (active_element_) {
            active_element_->setActive(true);
            markNeedsRepaint();
        }
    }

    dispatchMouseEventToJS("mousedown", last_mouse_x_, last_mouse_y_, button);
}



void View::handle_mouse_up(int32_t button) {
    // Stop scrollbar drag (left button).
    if (button == 1 && scroll_dragging_) {
        scroll_dragging_ = false;
        scroll_drag_container_.reset();
        scroll_drag_offset_y_ = 0.0f;
        scroll_drag_track_y_ = 0.0f;
        scroll_drag_track_h_ = 0.0f;
        scroll_drag_thumb_h_ = 0.0f;
        scroll_drag_max_scroll_ = 0.0f;
    }

    // Clear :active state
    if (button == 1) {
        left_mouse_down_ = false;
        if (active_element_) {
            active_element_->setActive(false);
            active_element_.reset();
            markNeedsRepaint();
        }
    }

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

        // Minimal <video controls> interaction: clicking the bottom controls bar toggles play/pause.
        if (clicked && clicked->getTagName() == "video" && clicked->hasAttribute("controls")) {
            auto it = video_states_.find(clicked.get());
            if (it != video_states_.end()) {
                const auto* layout = layout_engine->getLayout(clicked);
                if (layout && layout->height > 0.0f) {
                    const float bar_h = std::min(28.0f, layout->height);
                    const float my = static_cast<float>(last_mouse_y_);
                    const float bar_top = layout->y + layout->height - bar_h;
                    if (my >= bar_top && my <= layout->y + layout->height) {
                        toggleVideoPlayPause(it->second, last_wall_time_sec_);
                    }
                }
            }
        }

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
    if (!dom_manager || !layout_engine) {
        return;
    }

    const bool debug_wheel = (std::getenv("DONG_DEBUG_WHEEL") != nullptr);

    // 查找鼠标位置下的滚动容器
    auto scroll_container = findScrollContainerAt(last_mouse_x_, last_mouse_y_);
    if (debug_wheel) {
        DONG_LOG_INFO("[View::handle_mouse_wheel] delta=(%.2f, %.2f) at (%d,%d) container=%p tag=%s",
                      delta_x, delta_y, last_mouse_x_, last_mouse_y_,
                      scroll_container.get(),
                      scroll_container ? scroll_container->getTagName().c_str() : "null");
    }

    if (scroll_container) {
        constexpr float kScrollSpeed = 20.0f;

        // 计算可滚动的最大范围（和 Painter 的滚动条逻辑保持一致）
        float max_scroll_y = 0.0f;
        if (const auto* layout = layout_engine->getLayout(scroll_container)) {
            if (layout->height > 0.0f) {
                float content_bottom = layout->y;
                for (const auto& child : scroll_container->getChildren()) {
                    if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                        continue;
                    }
                    const auto* cl = layout_engine->getLayout(child);
                    if (!cl) continue;
                    float bottom = cl->y + cl->height;
                    if (bottom > content_bottom) content_bottom = bottom;
                }
                float content_height = content_bottom - layout->y;
                max_scroll_y = std::max(0.0f, content_height - layout->height);
            }
        }

        float old_x = scroll_container->getScrollX();
        float old_y = scroll_container->getScrollY();

        float new_x = std::max(0.0f, old_x + delta_x * kScrollSpeed);
        float new_y = std::clamp(old_y + delta_y * kScrollSpeed, 0.0f, max_scroll_y);

        if (debug_wheel) {
            DONG_LOG_INFO("[View::handle_mouse_wheel] scroll: old=(%.1f,%.1f) new=(%.1f,%.1f) max_y=%.1f",
                          old_x, old_y, new_x, new_y, max_scroll_y);
        }

        if (new_x != old_x || new_y != old_y) {
            scroll_container->scrollTo(new_x, new_y);
            markNeedsRepaint();
        }
    }

    // wheel 事件仍然派发到 JS（即使没有找到滚动容器）
    dispatchWheelEventToJS(delta_x, delta_y);
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

    // B: 切换外部 device 时清理离屏纹理缓存（避免在新 device 上误用旧纹理）
    if (offscreen_texture_cache_) {
        SDL_GPUDevice* old_dev = offscreen_texture_cache_device_;
        if (old_dev) {
            SDL_ReleaseGPUTexture(old_dev, offscreen_texture_cache_);
        }
        offscreen_texture_cache_ = nullptr;
        offscreen_texture_cache_device_ = nullptr;
        offscreen_texture_cache_width_ = 0;
        offscreen_texture_cache_height_ = 0;
    }

    // Offscreen 的 GPU idle 首帧等待只对“同一 device 生命周期”成立；切换 device 后重置
    offscreen_first_frame_done_ = false;
    offscreen_commands_dirty_ = true;

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

    // 注入图片资源管理器：用于 <img> / background-image 的像素解码与 GPU atlas 构建
    if (gpu_driver_) {
        gpu_driver_->setImageResourceManager(resource_manager_.get());
    }


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

// =============================================================================
// Video support (plugin-provided)
// =============================================================================

namespace {

static bool isAbsolutePathLike(const std::string& p) {
    if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':') {
        return true; // Windows drive path
    }
    if (!p.empty() && (p[0] == '/' || p[0] == '\\')) {
        return true; // Unix abs / UNC-like
    }
    return false;
}

static std::string resolveVideoUrlForPlugin(const std::string& src, const std::string& resource_root) {
    if (src.empty()) return {};

    // Basic URL handling: support file://, reject http(s)/data for now.
    if (src.rfind("http://", 0) == 0 || src.rfind("https://", 0) == 0 || src.rfind("data:", 0) == 0) {
        return {};
    }

    std::string path = src;
    if (path.rfind("file://", 0) == 0) {
        path = path.substr(std::string("file://").size());
        // Windows file URL often looks like "/d:/xxx"; strip the leading '/' if it precedes a drive letter.
        if (path.size() >= 3 && path[0] == '/' && std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == ':') {
            path.erase(path.begin());
        }
    }

    if (isAbsolutePathLike(path)) {
        return path;
    }

    if (!resource_root.empty()) {
        try {
            namespace fs = std::filesystem;
            fs::path resolved = fs::path(resource_root) / fs::path(path);
            return resolved.lexically_normal().string();
        } catch (...) {
            // ignore
        }
    }

    // Fallback: relative to current working directory.
    return path;
}

} // anonymous namespace

void View::closeVideo(VideoState& vs) {
    if (vs.player && plugin_ && plugin_->video_close) {
        plugin_->video_close(plugin_user_, vs.player);
    }
    vs.player = nullptr;

    vs.meta_ready = false;
    vs.loadeddata_sent = false;
    vs.playing = false;

    vs.ended = false;
    vs.errored = false;
    vs.current_pts = 0.0;

    vs.has_frame = false;
    vs.needs_upload = false;
    vs.rgba.clear();
    vs.frame_w = 0;
    vs.frame_h = 0;
    vs.frame_stride = 0;

    if (vs.node) {
        // Internal attribute (won't affect layout). Clearing requires a rebuild to remove DrawImage from cached command lists.
        vs.node->setAttribute("__dong_video_frame", "");
        markNeedsRepaint();
    }
}

void View::dispatchMediaEvent(VideoState& vs, const char* type_name) {
    if (!type_name || !type_name[0]) return;
    if (!js_bindings || !script_engine || !vs.node) return;

    ensureJSBindingsInitialized();

    uint64_t node_id = js_bindings->getNodeIdFor(vs.node);
    if (!node_id) {
        // If JS never touched this node, it can't have listeners.
        return;
    }
    if (!js_bindings->hasEventListeners(node_id, type_name)) {
        return;
    }

    const double duration = vs.meta_ready ? vs.meta.duration_seconds : 0.0;
    js_bindings->dispatchMediaEvent(node_id, type_name, vs.current_pts, duration, nullptr);
}

void View::dispatchMediaError(VideoState& vs, const char* message) {
    if (!js_bindings || !script_engine || !vs.node) return;

    ensureJSBindingsInitialized();

    uint64_t node_id = js_bindings->getNodeIdFor(vs.node);
    if (!node_id) {
        return;
    }
    if (!js_bindings->hasEventListeners(node_id, "error")) {
        return;
    }

    const double duration = vs.meta_ready ? vs.meta.duration_seconds : 0.0;
    js_bindings->dispatchMediaEvent(node_id, "error", vs.current_pts, duration, message ? message : "");
}

void View::toggleVideoPlayPause(VideoState& vs, double wall_time_sec) {
    if (!vs.player) return;

    if (!vs.playing) {
        // Start / resume.
        if (vs.ended && plugin_ && plugin_->video_seek) {
            plugin_->video_seek(plugin_user_, vs.player, 0.0);
            vs.current_pts = 0.0;
        }

        vs.playing = true;
        vs.ended = false;
        vs.wall_clock_start = wall_time_sec;
        vs.video_time_start = vs.current_pts;

        if (vs.node) {
            vs.node->setAttribute("__dong_video_playing", "1");
            vs.node->setAttribute("__dong_video_ended", "0");
            vs.node->setAttribute("__dong_video_seeking", "0");
            vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
        }


        dispatchMediaEvent(vs, "play");
        dispatchMediaEvent(vs, "playing");
    } else {
        // Pause.
        vs.playing = false;
        if (vs.node) {
            vs.node->setAttribute("__dong_video_playing", "0");
            vs.node->setAttribute("__dong_video_seeking", "0");
            vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
        }

        dispatchMediaEvent(vs, "pause");
    }
}



void View::updateVideoPlayback(VideoState& vs, double wall_time_sec) {
    if (!vs.player || !plugin_ || !plugin_->video_read_frame) {
        return;
    }
    if (!vs.playing) {
        return;
    }

    // Target playback time in seconds.
    const double target_time = vs.video_time_start + (wall_time_sec - vs.wall_clock_start);

    // Pull a few frames at most per tick to avoid long stalls on slow videos.
    constexpr int kMaxFramePullPerTick = 8;
    int pulled = 0;

    while (pulled < kMaxFramePullPerTick) {
        dong_video_frame_t frame{};
        int r = plugin_->video_read_frame(plugin_user_, vs.player, &frame);
        if (r == 0) {
            // EOF
            if (vs.loop && plugin_->video_seek) {
                plugin_->video_seek(plugin_user_, vs.player, 0.0);
                vs.wall_clock_start = wall_time_sec;
                vs.video_time_start = 0.0;
                vs.current_pts = 0.0;
                if (vs.node) {
                    vs.node->setAttribute("__dong_video_currentTime", "0");
                    vs.node->setAttribute("__dong_video_ended", "0");
                    vs.node->setAttribute("__dong_video_seeking", "0");
                }
                continue;
            }

            if (!vs.ended) {
                vs.ended = true;
                vs.playing = false;
                if (vs.node) {
                    vs.node->setAttribute("__dong_video_playing", "0");
                    vs.node->setAttribute("__dong_video_ended", "1");
                    vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
                }
                // Keep event order close to browser behavior: pause first, then ended.
                dispatchMediaEvent(vs, "pause");
                dispatchMediaEvent(vs, "ended");
            }
            return;

        }

        if (r < 0) {
            vs.errored = true;
            vs.playing = false;
            dispatchMediaError(vs, "video_read_frame failed");
            return;
        }

        // We got a frame. If it's still behind target_time, keep pulling.
        const double pts = frame.pts_seconds;

        const size_t sz = static_cast<size_t>(frame.stride_bytes) * static_cast<size_t>(frame.height);
        if (sz > 0 && frame.data) {
            vs.rgba.resize(sz);
            std::memcpy(vs.rgba.data(), frame.data, sz);
            vs.frame_w = frame.width;
            vs.frame_h = frame.height;
            vs.frame_stride = frame.stride_bytes;
            vs.current_pts = pts;
            vs.has_frame = true;
            vs.needs_upload = true;
            offscreen_texture_dirty_ = true;

            // Keep JS-visible currentTime reasonably up-to-date even between timeupdate events.
            if (vs.node) {
                vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
            }
        }


        ++pulled;

        // Ensure the renderer starts drawing video once we have the first frame.
        if (vs.node && vs.node->getAttribute("__dong_video_frame").empty()) {
            vs.node->setAttribute("__dong_video_frame", vs.frame_key);
            markNeedsRepaint();
        }

        // Fire ready events once we have decoded first frame (best-effort semantics).
        if (!vs.loadeddata_sent) {
            vs.loadeddata_sent = true;
            dispatchMediaEvent(vs, "loadeddata");
            dispatchMediaEvent(vs, "canplay");
        }


        // Dispatch timeupdate at ~4Hz (best effort).
        if (wall_time_sec - vs.last_timeupdate_wall >= 0.25) {
            vs.last_timeupdate_wall = wall_time_sec;
            if (vs.node) {
                vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
            }
            dispatchMediaEvent(vs, "timeupdate");
        }


        if (pts + 1e-6 >= target_time) {
            break;
        }
    }
}

void View::uploadPendingVideoFrames() {
    if (!use_gpu_ || !gpu_driver_) {
        return;
    }

    for (auto& kv : video_states_) {
        VideoState& vs = kv.second;
        if (!vs.needs_upload || !vs.has_frame || vs.frame_key.empty()) {
            continue;
        }
        if (vs.rgba.empty() || vs.frame_w == 0 || vs.frame_h == 0) {
            vs.needs_upload = false;
            continue;
        }

        const bool ok = gpu_driver_->updateExternalImageRGBA(
            vs.frame_key,
            vs.rgba.data(),
            vs.frame_w,
            vs.frame_h,
            vs.frame_stride
        );

        if (ok) {
            vs.needs_upload = false;
        }
    }
}

void View::syncVideoElements(double wall_time_sec) {
    const bool video_ok = (plugin_ && (plugin_->info.capabilities & DONG_PLUGIN_CAP_VIDEO) &&
                           plugin_->video_open && plugin_->video_close && plugin_->video_read_frame);
    if (!video_ok || !dom_manager) {
        if (!video_states_.empty()) {
            for (auto& kv : video_states_) {
                closeVideo(kv.second);
            }
            video_states_.clear();
        }
        return;
    }

    auto nodes = dom_manager->getElementsByTagName("video");
    std::unordered_set<void*> live;
    live.reserve(nodes.size());

    const std::string resource_root = resource_manager_ ? resource_manager_->getResourceRoot() : std::string();

    for (auto& n : nodes) {
        if (!n) continue;
        live.insert(n.get());

        VideoState& vs = video_states_[n.get()];
        vs.node = n;
        if (vs.frame_key.empty()) {
            vs.frame_key = "video://" + std::to_string((uintptr_t)n.get());
        }

        const std::string src = n->getAttribute("src");
        const bool autoplay = n->hasAttribute("autoplay");
        const bool loop = n->hasAttribute("loop");
        const bool controls = n->hasAttribute("controls");
        bool preload = true;
        {
            std::string p = n->getAttribute("preload");
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            if (p == "none") preload = false;
        }

        // If src removed, close.
        if (src.empty()) {
            if (vs.player) {
                closeVideo(vs);
            }
            vs.src.clear();
            continue;
        }

        const bool need_reopen = (!vs.player || vs.src != src);
        vs.autoplay = autoplay;
        vs.loop = loop;
        vs.controls = controls;
        vs.preload = preload;

        if (need_reopen) {
            if (vs.player) {
                closeVideo(vs);
            }

            vs.src = src;
            vs.errored = false;
            vs.ended = false;
            vs.current_pts = 0.0;
            vs.loadeddata_sent = false;
            vs.last_timeupdate_wall = 0.0;


            const std::string resolved = resolveVideoUrlForPlugin(src, resource_root);
            if (resolved.empty()) {
                vs.errored = true;
                dispatchMediaError(vs, "unsupported/invalid video src");
                continue;
            }

            vs.player = plugin_->video_open(plugin_user_, resolved.c_str());
            if (!vs.player) {
                vs.errored = true;
                dispatchMediaError(vs, "video_open failed");
                continue;
            }

            // Metadata.
            if (plugin_->video_get_metadata) {
                dong_video_metadata_t md{};
                if (plugin_->video_get_metadata(plugin_user_, vs.player, &md) != 0) {
                    vs.meta = md;
                    vs.meta_ready = true;
                    if (vs.node) {
                        vs.node->setAttribute("__dong_video_duration", std::to_string(vs.meta.duration_seconds));
                    }
                    dispatchMediaEvent(vs, "loadedmetadata");
                }
            }


            // Preload first frame (best effort) so poster-less videos show something.
            if (vs.preload && plugin_->video_seek) {
                plugin_->video_seek(plugin_user_, vs.player, 0.0);
            }
            if (vs.preload) {
                dong_video_frame_t f{};
                int rr = plugin_->video_read_frame(plugin_user_, vs.player, &f);
                if (rr == 1 && f.data && f.width > 0 && f.height > 0) {
                    const size_t sz = static_cast<size_t>(f.stride_bytes) * static_cast<size_t>(f.height);
                    vs.rgba.resize(sz);
                    std::memcpy(vs.rgba.data(), f.data, sz);
                    vs.frame_w = f.width;
                    vs.frame_h = f.height;
                    vs.frame_stride = f.stride_bytes;
                    vs.current_pts = f.pts_seconds;
                    vs.has_frame = true;
                    vs.needs_upload = true;
                    offscreen_texture_dirty_ = true;

                    if (vs.node) {
                        vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
                        vs.node->setAttribute("__dong_video_seeking", "0");
                        vs.node->setAttribute("__dong_video_ended", "0");
                    }


                    // Enable video rendering in Painter.
                    if (vs.node) {
                        vs.node->setAttribute("__dong_video_frame", vs.frame_key);
                        markNeedsRepaint();
                    }

                    // Preload counts as ready-to-play for our minimal model.
                    if (!vs.loadeddata_sent) {
                        vs.loadeddata_sent = true;
                        dispatchMediaEvent(vs, "loadeddata");
                        dispatchMediaEvent(vs, "canplay");
                    }

                }
            }

            // Autoplay.
            if (vs.autoplay) {
                vs.playing = true;
                vs.wall_clock_start = wall_time_sec;
                vs.video_time_start = vs.current_pts;
                dispatchMediaEvent(vs, "play");
                dispatchMediaEvent(vs, "playing");
            } else {
                vs.playing = false;
            }

        }

        // JS-driven seek/play/pause (via internal __dong_video_* attributes).
        // These attributes are "internal" ("__"-prefixed) and won't dirty layout.
        {
            const std::string seek_req = n->getAttribute("__dong_video_seek");
            if (!seek_req.empty() && plugin_ && plugin_->video_seek) {
                char* end = nullptr;
                double t = std::strtod(seek_req.c_str(), &end);
                if (end && end != seek_req.c_str()) {
                    if (t < 0.0) t = 0.0;
                    if (vs.meta_ready && vs.meta.duration_seconds > 0.0) {
                        t = std::min(t, vs.meta.duration_seconds);
                    }

                    if (vs.node) {
                        vs.node->setAttribute("__dong_video_seeking", "1");
                        vs.node->setAttribute("__dong_video_ended", "0");
                        vs.node->setAttribute("__dong_video_currentTime", std::to_string(t));
                    }

                    plugin_->video_seek(plugin_user_, vs.player, t);
                    vs.current_pts = t;
                    vs.video_time_start = vs.current_pts;
                    vs.wall_clock_start = wall_time_sec;
                    vs.ended = false;

                    dispatchMediaEvent(vs, "seeking");

                    // Best effort: decode one frame after seek so paused videos update.
                    dong_video_frame_t f{};
                    int rr = plugin_->video_read_frame(plugin_user_, vs.player, &f);
                    if (rr == 1 && f.data && f.width > 0 && f.height > 0) {
                        const size_t sz = static_cast<size_t>(f.stride_bytes) * static_cast<size_t>(f.height);
                        vs.rgba.resize(sz);
                        std::memcpy(vs.rgba.data(), f.data, sz);
                        vs.frame_w = f.width;
                        vs.frame_h = f.height;
                        vs.frame_stride = f.stride_bytes;
                        vs.current_pts = f.pts_seconds;
                        vs.has_frame = true;
                        vs.needs_upload = true;
                        offscreen_texture_dirty_ = true;

                        if (vs.node) {
                            vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
                        }

                        if (vs.node && vs.node->getAttribute("__dong_video_frame").empty()) {
                            vs.node->setAttribute("__dong_video_frame", vs.frame_key);
                            markNeedsRepaint();
                        }
                    }

                    if (vs.node) {
                        vs.node->setAttribute("__dong_video_seeking", "0");
                        vs.node->setAttribute("__dong_video_ended", "0");
                        vs.node->setAttribute("__dong_video_currentTime", std::to_string(vs.current_pts));
                    }
                    vs.last_timeupdate_wall = wall_time_sec;
                    dispatchMediaEvent(vs, "seeked");
                    dispatchMediaEvent(vs, "timeupdate");

                }

                // Clear request (avoid removeAttribute() which would dirty layout).
                n->setAttribute("__dong_video_seek", "");
            }

            // Desired playing state; default to current vs.playing.
            const std::string desired_play = n->getAttribute("__dong_video_playing");
            if (!desired_play.empty()) {
                const bool want_playing = (desired_play == "1" || desired_play == "true");
                if (want_playing != vs.playing) {
                    toggleVideoPlayPause(vs, wall_time_sec);
                }
            } else {
                // Ensure JS-visible state is initialized.
                n->setAttribute("__dong_video_playing", vs.playing ? "1" : "0");
            }

            // Publish duration once metadata is known.
            if (vs.meta_ready && n->getAttribute("__dong_video_duration").empty()) {
                n->setAttribute("__dong_video_duration", std::to_string(vs.meta.duration_seconds));
            }
        }

        // Drive playback.
        updateVideoPlayback(vs, wall_time_sec);
    }


    // Cleanup removed nodes.
    for (auto it = video_states_.begin(); it != video_states_.end();) {
        if (live.find(it->first) == live.end()) {
            closeVideo(it->second);
            it = video_states_.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace dong
