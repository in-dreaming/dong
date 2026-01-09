#define DONG_DISABLE_LEGACY_VIEW_API
#include "dong.h"

#include "core/context.hpp"
#include "core/log.h"
#include "dom/dom_manager.hpp"
#include "dom/event_system.hpp"
#include "dom/focus_manager.hpp"
#include "dom/input_element.hpp"
#include "layout/layout_engine.hpp"
#include "render/render_surface.hpp"
#include "render/painter.hpp"
#include "render/gpu_ir.hpp"
#include "script/script_engine.hpp"
#include "script/js_bindings.hpp"

#include <cstring>
#include <string>
#include <memory>
#include <functional>
#include <fstream>
#include <sstream>

namespace {

// Forward log calls to plugin if available
static const dong_plugin_vtable_t* g_plugin = nullptr;
static void* g_plugin_user = nullptr;

void plugin_log_callback(dong_log_level_t level, const char* msg) {
    if (g_plugin && g_plugin->log) {
        g_plugin->log(g_plugin_user, level, msg);
    }
}

// Hit test helper
dong::dom::DOMNodePtr hitTestRecursive(const dong::dom::DOMNodePtr& node, 
                                        dong::layout::Engine* layout_engine,
                                        int32_t x, int32_t y) {
    if (!node || !layout_engine) return nullptr;

    const auto* layout = layout_engine->getLayout(node);
    if (!layout) return nullptr;
    
    float lx = layout->x;
    float ly = layout->y;
    float w = layout->width;
    float h = layout->height;
    
    bool in_bounds = (x >= lx && x <= lx + w && y >= ly && y <= ly + h);
    
    if (!in_bounds) {
        return nullptr;
    }
    
    const auto& children = node->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto child_hit = hitTestRecursive(*it, layout_engine, x, y);
        if (child_hit) {
            return child_hit;
        }
    }
    
    return node;
}

dong::dom::DOMNodePtr hitTestElementAt(dong::dom::Manager* dom_mgr, 
                                        dong::layout::Engine* layout_engine,
                                        int32_t x, int32_t y) {
    if (!dom_mgr || !layout_engine) return nullptr;
    auto root = dom_mgr->getRoot();
    if (!root) return nullptr;
    return hitTestRecursive(root, layout_engine, x, y);
}

dong::dom::DOMNodePtr findScrollContainerAt(dong::dom::Manager* dom_mgr,
                                             dong::layout::Engine* layout_engine,
                                             int32_t x, int32_t y) {
    if (!dom_mgr || !layout_engine) return nullptr;
    
    auto element = hitTestElementAt(dom_mgr, layout_engine, x, y);
    if (!element) return nullptr;
    
    auto current = element;
    while (current) {
        if (current->isScrollContainer()) {
            return current;
        }
        current = current->getParent();
    }
    
    return nullptr;
}

struct EngineImpl {
    dong::Context ctx;
    const dong_plugin_vtable_t* plugin = nullptr;
    void* plugin_user = nullptr;
    std::string html;
    uint32_t width = 960;
    uint32_t height = 540;
    
    // DOM/Layout/Render subsystems
    std::unique_ptr<dong::dom::Manager> dom_manager;
    std::unique_ptr<dong::layout::Engine> layout_engine;
    std::unique_ptr<dong::render::RenderSurface> render_surface;
    std::unique_ptr<dong::render::Painter> painter;
    std::unique_ptr<dong::dom::EventDispatcher> event_dispatcher;
    std::unique_ptr<dong::dom::FocusManager> focus_manager;
    std::unique_ptr<dong::script::ScriptEngine> script_engine;
    std::unique_ptr<dong::script::JSBindings> js_bindings;
    
    // GPU command list (platform-agnostic)
    std::unique_ptr<dong::render::GPUCommandList> cached_cmd_list;
    
    // External GPU device/window handles (opaque, owned by plugin)
    void* external_gpu_device = nullptr;
    void* external_window = nullptr;
    
    bool use_gpu = false;
    bool commands_dirty = true;
    bool js_bindings_initialized = false;
    int32_t last_mouse_x = 0;
    int32_t last_mouse_y = 0;
    
    EngineImpl() 
        : dom_manager(std::make_unique<dong::dom::Manager>()),
          layout_engine(std::make_unique<dong::layout::Engine>()),
          render_surface(std::make_unique<dong::render::CPUBufferSurface>(960, 540)),
          event_dispatcher(std::make_unique<dong::dom::EventDispatcher>()),
          focus_manager(std::make_unique<dong::dom::FocusManager>()),
          script_engine(std::make_unique<dong::script::ScriptEngine>()) {
        
        focus_manager->setEventDispatcher(event_dispatcher.get());
        painter = std::make_unique<dong::render::Painter>(render_surface.get());
        
        js_bindings = std::make_unique<dong::script::JSBindings>(
            script_engine.get(),
            dom_manager.get(),
            event_dispatcher.get()
        );
    }
    
    ~EngineImpl() = default;
    
    void log(dong_log_level_t level, const char* msg) {
        if (plugin && plugin->log) {
            plugin->log(plugin_user, level, msg);
        }
    }
    
    void ensureJSBindingsInitialized() {
        if (js_bindings_initialized || !js_bindings || !script_engine) {
            return;
        }
        JSContext* ctx = script_engine->getContext();
        if (!ctx) {
            return;
        }
        js_bindings->initialize();
        js_bindings_initialized = true;
    }
    
    void markNeedsRepaint() {
        commands_dirty = true;
        if (render_surface) {
            render_surface->markDirty();
        }
    }
    
    bool loadHTML(const char* html_content) {
        if (!html_content || !dom_manager) {
            return false;
        }
        
        if (dom_manager->loadHTML(html_content)) {
            if (js_bindings) {
                js_bindings->resetForNewDOM();
            }
            markNeedsRepaint();
            
            auto root = dom_manager->getRoot();
            if (root) {
                root->markLayoutDirty();
            }
            
            // Execute <script> tags
            if (script_engine && dom_manager) {
                ensureJSBindingsInitialized();
                
                auto scripts = dom_manager->getElementsByTagName("script");
                for (const auto& script : scripts) {
                    if (!script) continue;
                    
                    std::string code;
                    std::string src = script->getAttribute("src");
                    
                    if (!src.empty()) {
                        std::ifstream file(src);
                        if (file.is_open()) {
                            std::stringstream buffer;
                            buffer << file.rdbuf();
                            code = buffer.str();
                        }
                    } else {
                        for (const auto& child : script->getChildren()) {
                            if (child && child->getType() == dong::dom::DOMNode::NodeType::TEXT) {
                                code += child->getTextContent();
                            }
                        }
                    }
                    
                    if (!code.empty()) {
                        script_engine->eval(code);
                    }
                }
            }
            
            return true;
        }
        return false;
    }
    
    void resize(uint32_t new_width, uint32_t new_height) {
        if (new_width == width && new_height == height) return;
        
        width = new_width;
        height = new_height;
        
        if (render_surface && render_surface->getType() == dong::render::RenderSurface::Type::CPU_BUFFER) {
            static_cast<dong::render::CPUBufferSurface*>(render_surface.get())->resize(width, height);
        }
        
        markNeedsRepaint();
        
        if (dom_manager) {
            auto root = dom_manager->getRoot();
            if (root) {
                root->markLayoutDirty();
            }
        }
    }
    
    bool setGPU(void* device, void* window) {
        if (!device || !window) {
            return false;
        }
        
        external_gpu_device = device;
        external_window = window;
        use_gpu = true;
        markNeedsRepaint();
        
        return true;
    }
    
    dong_result_t tick() {
        // Process pending JS tasks
        if (script_engine) {
            script_engine->processPendingTasks();
        }
        
        // Update layout if needed
        if (layout_engine && dom_manager) {
            auto root = dom_manager->getRoot();
            if (root && root->isLayoutDirty()) {
                layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
                markNeedsRepaint();
            }
        }
        
        // For now, tick just updates DOM/layout
        // GPU rendering is handled by dong_legacy/View or external code
        // TODO: In future, generate GPUCommandList here and pass to plugin
        
        return DONG_OK;
    }
    
    void handleMouseMove(int32_t x, int32_t y) {
        last_mouse_x = x;
        last_mouse_y = y;
        dispatchMouseEvent("mousemove", x, y, 0);
    }
    
    void handleMouseButton(int32_t button, bool pressed) {
        if (pressed) {
            dispatchMouseEvent("mousedown", last_mouse_x, last_mouse_y, button);
        } else {
            dispatchMouseEvent("mouseup", last_mouse_x, last_mouse_y, button);
            dispatchMouseEvent("click", last_mouse_x, last_mouse_y, button);
            
            // Handle focus on click
            if (focus_manager && dom_manager && layout_engine) {
                auto clicked = hitTestElementAt(dom_manager.get(), layout_engine.get(),
                                                last_mouse_x, last_mouse_y);
                if (clicked) {
                    focus_manager->focusOnClick(clicked);
                }
            }
        }
    }
    
    void handleMouseWheel(float delta_x, float delta_y) {
        auto scroll_container = findScrollContainerAt(dom_manager.get(), layout_engine.get(),
                                                      last_mouse_x, last_mouse_y);
        if (scroll_container) {
            constexpr float kScrollSpeed = 20.0f;
            // dong 约定：delta_y 正值=向下滚动（内容向上移动，scroll_y 增加）
            scroll_container->scrollBy(delta_x * kScrollSpeed, delta_y * kScrollSpeed);
            markNeedsRepaint();
        }
    }
    
    void handleKey(uint32_t key_code, bool pressed) {
        constexpr uint32_t SDLK_TAB = 9;
        constexpr uint32_t SDLK_BACKSPACE = 8;
        constexpr uint32_t SDLK_DELETE = 127;
        constexpr uint32_t SDLK_LEFT = 0x40000050;
        constexpr uint32_t SDLK_RIGHT = 0x4000004F;
        
        if (pressed) {
            if (key_code == SDLK_TAB && focus_manager && dom_manager) {
                focus_manager->moveFocus(dom_manager->getRoot(), false);
                return;
            }
            
            if (focus_manager) {
                auto focused = focus_manager->getFocusedElement();
                if (focused && dong::dom::isEditableElement(focused)) {
                    auto* state = dong::dom::getInputState(focused);
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
                            focused->setAttribute("value", state->getValue());
                            markNeedsRepaint();
                        }
                    }
                }
            }
            
            dispatchKeyEvent("keydown", key_code);
        } else {
            dispatchKeyEvent("keyup", key_code);
        }
    }
    
    void handleTextInput(const char* text) {
        if (!text || !text[0]) return;
        
        dong::dom::DOMNodePtr focused;
        if (focus_manager) {
            focused = focus_manager->getFocusedElement();
        }
        
        if (focused && dong::dom::isEditableElement(focused)) {
            auto* state = dong::dom::getInputState(focused);
            if (state) {
                state->insertText(text);
                focused->setAttribute("value", state->getValue());
                markNeedsRepaint();
            }
        }
    }
    
    bool evalScript(const char* code) {
        if (!code || !script_engine) {
            return false;
        }
        ensureJSBindingsInitialized();
        return script_engine->eval(std::string(code));
    }
    
private:
    void dispatchMouseEvent(const char* type, int32_t x, int32_t y, int32_t button) {
        if (!script_engine || !js_bindings || !event_dispatcher) return;
        
        auto target = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
        if (!target && dom_manager) {
            auto buttons = dom_manager->getElementsByTagName("button");
            if (!buttons.empty()) {
                target = buttons[0];
            }
        }
        if (!target) return;
        
        dong::dom::EventType ev_type;
        std::string type_str = type ? std::string(type) : std::string();
        if (type_str == "click") {
            ev_type = dong::dom::EventType::CLICK;
        } else if (type_str == "mousedown") {
            ev_type = dong::dom::EventType::MOUSE_DOWN;
        } else if (type_str == "mouseup") {
            ev_type = dong::dom::EventType::MOUSE_UP;
        } else if (type_str == "mousemove") {
            ev_type = dong::dom::EventType::MOUSE_MOVE;
        } else {
            return;
        }
        
        dong::dom::Event event = event_dispatcher->createMouseEvent(ev_type, x, y, button);
        event.target = target;
        event.current_target = target;
        event_dispatcher->dispatch(event);
    }
    
    void dispatchKeyEvent(const char* type, uint32_t key_code) {
        if (!script_engine || !js_bindings) return;
        if (!dom_manager) return;
        
        auto bodies = dom_manager->getElementsByTagName("body");
        dong::dom::DOMNodePtr target;
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
};

} // namespace

extern "C" {

dong_result_t dong_engine_create(const dong_engine_desc_t* desc, dong_engine_t** out_engine) {
    if (!desc || !out_engine) {
        return DONG_ERR_INVALID_ARG;
    }
    if (desc->api_version != DONG_API_VERSION) {
        *out_engine = nullptr;
        return DONG_ERR_VERSION_MISMATCH;
    }

    if (desc->plugin) {
        if (desc->plugin_api_version != DONG_PLUGIN_API_VERSION) {
            *out_engine = nullptr;
            return DONG_ERR_VERSION_MISMATCH;
        }
        if (desc->plugin->info.plugin_api_version != DONG_PLUGIN_API_VERSION) {
            *out_engine = nullptr;
            return DONG_ERR_VERSION_MISMATCH;
        }
    }

    try {
        auto* impl = new EngineImpl();
        impl->plugin = desc->plugin;
        impl->plugin_user = desc->plugin_user;
        impl->width = desc->width > 0 ? desc->width : 960;
        impl->height = desc->height > 0 ? desc->height : 540;

        // Resize to match requested dimensions
        impl->resize(impl->width, impl->height);

        if (desc->html) {
            impl->html = desc->html;
        }

        // Set up global plugin for logging
        g_plugin = desc->plugin;
        g_plugin_user = desc->plugin_user;

        // Initialize core resources
        if (!impl->ctx.initialize()) {
            delete impl;
            *out_engine = nullptr;
            return DONG_ERR_INTERNAL;
        }

        // Load initial HTML if provided
        if (!impl->html.empty()) {
            impl->loadHTML(impl->html.c_str());
        }

        // Log engine creation
        if (impl->plugin && impl->plugin->log) {
            char msg[256];
            std::snprintf(msg, sizeof(msg), "Engine created (%ux%u, html=%s)",
                         impl->width, impl->height, impl->html.empty() ? "none" : "loaded");
            impl->plugin->log(impl->plugin_user, DONG_LOG_INFO, msg);
        }

        *out_engine = reinterpret_cast<dong_engine_t*>(impl);
        return DONG_OK;
    } catch (...) {
        *out_engine = nullptr;
        return DONG_ERR_INTERNAL;
    }
}

void dong_engine_destroy(dong_engine_t* engine) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return;
    }

    if (impl->plugin && impl->plugin->log) {
        impl->plugin->log(impl->plugin_user, DONG_LOG_INFO, "Engine destroyed");
    }

    impl->ctx.shutdown();

    // Clear global plugin if it matches
    if (g_plugin == impl->plugin) {
        g_plugin = nullptr;
        g_plugin_user = nullptr;
    }

    delete impl;
}

dong_result_t dong_engine_tick(dong_engine_t* engine) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    return impl->tick();
}

dong_result_t dong_engine_load_html(dong_engine_t* engine, const char* html) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    if (!html) {
        return DONG_ERR_INVALID_ARG;
    }
    
    if (impl->loadHTML(html)) {
        if (impl->plugin && impl->plugin->log) {
            impl->plugin->log(impl->plugin_user, DONG_LOG_INFO, "HTML loaded");
        }
        return DONG_OK;
    }
    return DONG_ERR_INTERNAL;
}

dong_result_t dong_engine_resize(dong_engine_t* engine, uint32_t width, uint32_t height) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    impl->resize(width, height);
    return DONG_OK;
}

dong_result_t dong_engine_set_gpu(dong_engine_t* engine, void* gpu_device, void* window) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    
    if (impl->setGPU(gpu_device, window)) {
        if (impl->plugin && impl->plugin->log) {
            impl->plugin->log(impl->plugin_user, DONG_LOG_INFO, "GPU device set");
        }
        return DONG_OK;
    }
    return DONG_ERR_INTERNAL;
}

dong_result_t dong_engine_send_mouse_move(dong_engine_t* engine, int32_t x, int32_t y) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    impl->handleMouseMove(x, y);
    return DONG_OK;
}

dong_result_t dong_engine_send_mouse_button(dong_engine_t* engine, int32_t button, int pressed) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    impl->handleMouseButton(button, pressed != 0);
    return DONG_OK;
}

dong_result_t dong_engine_send_mouse_wheel(dong_engine_t* engine, float delta_x, float delta_y) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    impl->handleMouseWheel(delta_x, delta_y);
    return DONG_OK;
}

dong_result_t dong_engine_send_key(dong_engine_t* engine, uint32_t key_code, int pressed) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    impl->handleKey(key_code, pressed != 0);
    return DONG_OK;
}

dong_result_t dong_engine_send_text(dong_engine_t* engine, const char* text) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    impl->handleTextInput(text);
    return DONG_OK;
}

dong_result_t dong_engine_eval_script(dong_engine_t* engine, const char* code) {
    auto* impl = reinterpret_cast<EngineImpl*>(engine);
    if (!impl) {
        return DONG_ERR_INVALID_ARG;
    }
    if (impl->evalScript(code)) {
        return DONG_OK;
    }
    return DONG_ERR_INTERNAL;
}

uint32_t dong_get_api_version(void) {
    return DONG_API_VERSION;
}

} // extern "C"
