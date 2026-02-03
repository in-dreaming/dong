#include "engine_view.hpp"

#include "context.hpp"
#include "log.h"
#include "profiler.h"

#include "dong_platform.h"
#include "dong_gpu_driver.h"
#include "dong_file_system.h"

#include "../dom/dom_manager.hpp"
#include "../dom/style_engine.hpp"
#include "../dom/event_system.hpp"
#include "../dom/focus_manager.hpp"
#include "../dom/input_element.hpp"
#include "../layout/layout_engine.hpp"

#include "../render/render_surface.hpp"
#include "../render/painter.hpp"
#include "../render/gpu_ir.hpp"
#include "../script/script_engine.hpp"
#include "../script/js_bindings.hpp"

#include <cstring>
#include <string>
#include <sstream>
#include <functional>
#include <filesystem>
#include <cctype>



namespace dong {

namespace {

using dong::dom::DOMNodePtr;

DOMNodePtr hitTestRecursive(const DOMNodePtr& node, dong::layout::Engine* layout_engine,
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

DOMNodePtr hitTestElementAt(dong::dom::Manager* dom_mgr, dong::layout::Engine* layout_engine,
                            int32_t x, int32_t y) {
    if (!dom_mgr || !layout_engine) return nullptr;
    auto root = dom_mgr->getRoot();
    if (!root) return nullptr;
    return hitTestRecursive(root, layout_engine, x, y);
}

DOMNodePtr findScrollContainerAt(dong::dom::Manager* dom_mgr, dong::layout::Engine* layout_engine,
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

bool isAbsolutePath(const std::string& p) {
    if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':') {
        return true;
    }
    if (!p.empty() && (p[0] == '/' || p[0] == '\\')) {
        return true;
    }
    return false;
}

bool readTextFileFromPlatformFS(const std::string& path,
                                const std::string& resource_root,
                                std::string& out) {
    out.clear();
    if (path.empty()) {
        return false;
    }

    std::string resolved = path;
    if (!resource_root.empty() && !isAbsolutePath(path)) {
        try {
            namespace fs = std::filesystem;
            resolved = (fs::path(resource_root) / fs::path(path)).lexically_normal().string();
        } catch (...) {
            resolved = path;
        }
    }

    DongPlatform* platform = dong_platform_get();
    DongFileSystem* fs = platform ? dong_platform_get_file_system(platform) : nullptr;
    if (!fs) {
        return false;
    }

    DongFileData data{};
    DongFileSystemResult r = dong_fs_read_all(fs, resolved.c_str(), &data);
    if (r != DONG_FS_OK || !data.data || data.size == 0) {
        dong_fs_free_data(fs, &data);
        return false;
    }

    out.assign(static_cast<const char*>(data.data), data.size);
    dong_fs_free_data(fs, &data);
    return true;
}


} // namespace

struct EngineView::Impl {
    dong::Context ctx;

    uint32_t width = 960;
    uint32_t height = 540;
    std::string html;
    std::string resource_root;
    std::string cached_cursor;


    std::unique_ptr<dong::dom::Manager> dom_manager;
    std::unique_ptr<dong::layout::Engine> layout_engine;
    std::unique_ptr<dong::render::RenderSurface> render_surface;
    std::unique_ptr<dong::render::Painter> painter;

    std::unique_ptr<dong::dom::EventDispatcher> event_dispatcher;
    std::unique_ptr<dong::dom::FocusManager> focus_manager;
    std::unique_ptr<dong::script::ScriptEngine> script_engine;
    std::unique_ptr<dong::script::JSBindings> js_bindings;

    std::unique_ptr<dong::render::GPUCommandList> cached_cmd_list;

    void* external_gpu_device = nullptr;
    void* external_window = nullptr;

    bool use_gpu = false;
    bool commands_dirty = true;
    bool js_bindings_initialized = false;

    int32_t last_mouse_x = 0;
    int32_t last_mouse_y = 0;

    DOMNodePtr hovered_element;
    DOMNodePtr active_element;

    Impl(uint32_t w, uint32_t h)

        : width(w), height(h),
          dom_manager(std::make_unique<dong::dom::Manager>()),
          layout_engine(std::make_unique<dong::layout::Engine>()),
          render_surface(std::make_unique<dong::render::CPUBufferSurface>(w, h)),
          painter(std::make_unique<dong::render::Painter>(render_surface.get())),
          event_dispatcher(std::make_unique<dong::dom::EventDispatcher>()),
          focus_manager(std::make_unique<dong::dom::FocusManager>()),
          script_engine(std::make_unique<dong::script::ScriptEngine>()),
          js_bindings(std::make_unique<dong::script::JSBindings>(
              script_engine.get(),
              dom_manager.get(),
              event_dispatcher.get())) {
        focus_manager->setEventDispatcher(event_dispatcher.get());
        ctx.initialize();
    }

    ~Impl() {
        ctx.shutdown();
    }

    void ensureJSBindingsInitialized() {
        if (js_bindings_initialized || !js_bindings || !script_engine) {
            return;
        }
        JSContext* qjs = script_engine->getContext();
        if (!qjs) {
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

    DOMNodePtr hitElementAt(int32_t x, int32_t y) {
        if (!dom_manager || !layout_engine) return nullptr;
        auto hit = hitTestElementAt(dom_manager.get(), layout_engine.get(), x, y);
        while (hit && hit->getType() != dong::dom::DOMNode::NodeType::ELEMENT) {
            hit = hit->getParent();
        }
        return hit;
    }

    void updateHoverState(int32_t x, int32_t y) {
        auto hit = hitElementAt(x, y);
        if (hit == hovered_element) return;

        if (hovered_element) {
            hovered_element->setHovered(false);
        }
        hovered_element = hit;
        if (hovered_element) {
            hovered_element->setHovered(true);
        }
        markNeedsRepaint();
    }

    void setActiveElement(const DOMNodePtr& hit) {
        if (active_element && active_element != hit) {
            active_element->setActive(false);
        }
        active_element = hit;
        if (active_element) {
            active_element->setActive(true);
        }
        markNeedsRepaint();
    }

    void clearActiveElement() {
        if (!active_element) return;
        active_element->setActive(false);
        active_element.reset();
        markNeedsRepaint();
    }

    bool loadHTML(const char* html_content) {

        if (!html_content || !dom_manager) {
            return false;
        }

        if (!dom_manager->loadHTML(html_content)) {
            return false;
        }

        if (js_bindings) {
            js_bindings->resetForNewDOM();
        }
        markNeedsRepaint();

        auto root = dom_manager->getRoot();
        if (root) {
            root->markLayoutDirty();
        }

        // Execute <script> tags (inline or src)
        if (script_engine && dom_manager) {
            ensureJSBindingsInitialized();

            auto scripts = dom_manager->getElementsByTagName("script");
            for (const auto& script : scripts) {
                if (!script) continue;

                std::string code;
                std::string src = script->getAttribute("src");

                if (!src.empty()) {
                    (void)readTextFileFromPlatformFS(src, resource_root, code);
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

    void resize(uint32_t new_width, uint32_t new_height) {
        if (new_width == width && new_height == height) return;

        width = new_width;
        height = new_height;

        if (render_surface && render_surface->getType() == dong::render::RenderSurface::Type::CPU_BUFFER) {
            static_cast<dong::render::CPUBufferSurface*>(render_surface.get())->resize(width, height);
        }

        markNeedsRepaint();

        auto root = dom_manager ? dom_manager->getRoot() : nullptr;
        if (root) {
            root->markLayoutDirty();
        }
    }

    bool setGPU(void* device, void* window) {
        if (!device || !window) {
            return false;
        }

        external_gpu_device = device;
        external_window = window;
        use_gpu = true;

        if (!resource_root.empty()) {
            DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
            if (driver) {
                dong_gpu_set_resource_root(driver, resource_root.c_str());
            }
        }

        markNeedsRepaint();
        return true;

    }

    bool tick() {
        DONG_PROFILE_SCOPE_CAT("EngineView::tick", "frame");

        if (script_engine) {
            DONG_PROFILE_SCOPE_CAT("Script::processTasks", "script");
            script_engine->processPendingTasks();
        }

        if (dom_manager) {
            auto root = dom_manager->getRoot();
            if (root && root->isStyleDirty()) {
                if (auto* se = dom_manager->getStyleEngine()) {
                    DONG_PROFILE_SCOPE_CAT("Style::compute", "style");
                    se->computeStylesIncremental(root);
                }
                root->clearStyleDirtyRecursive();
            }
        }

        if (layout_engine && dom_manager) {
            auto root = dom_manager->getRoot();
            if (root && root->isLayoutDirty()) {
                DONG_PROFILE_SCOPE_CAT("Layout::calculate", "layout");
                layout_engine->calculateLayout(root, static_cast<float>(width), static_cast<float>(height));
                markNeedsRepaint();
            }
        }


        if (use_gpu && commands_dirty && dom_manager && layout_engine && painter) {
            DONG_PROFILE_SCOPE_CAT("Render::generateCommands", "render");

            auto root = dom_manager->getRoot();
            if (root) {
                painter->buildDisplayList(root, layout_engine.get());

                if (!cached_cmd_list) {
                    cached_cmd_list = std::make_unique<dong::render::GPUCommandList>();
                }
                dong::render::GPUCompiler compiler;
                compiler.compile(painter->getDisplayList(), *cached_cmd_list, &painter->getLayerTree());
                commands_dirty = false;
            }
        }

        if (use_gpu && cached_cmd_list && !commands_dirty) {
            DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
            if (driver) {
                DONG_PROFILE_SCOPE_CAT("GPU::execute", "render");
                (void)dong_gpu_execute(driver, cached_cmd_list.get());
            }
        }

        return true;
    }

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

        JSContext* qjs = script_engine->getContext();
        if (!qjs) return;

        uint64_t node_id = js_bindings->getNodeIdFor(target);
        if (!node_id) {
            JSValue tmp = js_bindings->createJSElement(qjs, target);
            JS_FreeValue(qjs, tmp);
            node_id = js_bindings->getNodeIdFor(target);
            if (!node_id) {
                return;
            }
        }

        js_bindings->dispatchKeyEvent(node_id, type, key_code);
    }

    void sendMouseMove(int32_t x, int32_t y) {
        last_mouse_x = x;
        last_mouse_y = y;
        updateHoverState(x, y);
        dispatchMouseEvent("mousemove", x, y, 0);
    }


    void sendMouseButton(int32_t button, bool pressed) {
        updateHoverState(last_mouse_x, last_mouse_y);

        if (pressed) {
            auto hit = hitElementAt(last_mouse_x, last_mouse_y);
            setActiveElement(hit);
            dispatchMouseEvent("mousedown", last_mouse_x, last_mouse_y, button);
            return;
        }

        dispatchMouseEvent("mouseup", last_mouse_x, last_mouse_y, button);
        clearActiveElement();
        dispatchMouseEvent("click", last_mouse_x, last_mouse_y, button);

        if (focus_manager && dom_manager && layout_engine) {
            auto clicked = hitTestElementAt(dom_manager.get(), layout_engine.get(), last_mouse_x, last_mouse_y);
            if (clicked) {
                focus_manager->focusOnClick(clicked);
            }
        }
    }


    void sendMouseWheel(float delta_x, float delta_y) {
        auto scroll_container = findScrollContainerAt(dom_manager.get(), layout_engine.get(), last_mouse_x, last_mouse_y);
        if (!scroll_container) {
            return;
        }

        constexpr float kScrollSpeed = 20.0f;
        scroll_container->scrollBy(delta_x * kScrollSpeed, delta_y * kScrollSpeed);
        markNeedsRepaint();
    }

    void sendKey(uint32_t key_code, bool pressed) {
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

    void sendText(const char* text) {
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
};

EngineView::EngineView(uint32_t width, uint32_t height)
    : impl_(std::make_unique<Impl>(width, height)) {}

EngineView::~EngineView() = default;

bool EngineView::isInitialized() const {
    return impl_ ? impl_->ctx.isInitialized() : false;
}

void EngineView::setResourceRoot(const char* root) {
    if (!impl_) return;
    impl_->resource_root = root ? root : "";

    DongPlatform* platform = dong_platform_get();
    DongGPUDriver* driver = platform ? dong_platform_get_gpu_driver(platform) : nullptr;
    if (driver) {
        dong_gpu_set_resource_root(driver, impl_->resource_root.c_str());
    }
}

const char* EngineView::getCursorAt(int32_t x, int32_t y) {
    if (!impl_ || !impl_->dom_manager || !impl_->layout_engine) {
        impl_->cached_cursor = "auto";
        return impl_->cached_cursor.c_str();
    }

    auto element = hitTestElementAt(impl_->dom_manager.get(), impl_->layout_engine.get(), x, y);
    if (!element) {
        impl_->cached_cursor = "auto";
        return impl_->cached_cursor.c_str();
    }

    auto current = element;
    while (current) {
        const auto& cursor = current->getComputedStyle().cursor;
        if (!cursor.empty() && cursor != "auto") {
            impl_->cached_cursor = cursor;
            return impl_->cached_cursor.c_str();
        }
        current = current->getParent();
    }

    impl_->cached_cursor = "auto";
    return impl_->cached_cursor.c_str();
}

bool EngineView::loadHTML(const char* html) {
    return impl_ ? impl_->loadHTML(html) : false;
}



void EngineView::resize(uint32_t width, uint32_t height) {
    if (impl_) impl_->resize(width, height);
}

bool EngineView::setGPU(void* gpu_device, void* window) {
    return impl_ ? impl_->setGPU(gpu_device, window) : false;
}

bool EngineView::tick() {
    return impl_ ? impl_->tick() : false;
}

void EngineView::sendMouseMove(int32_t x, int32_t y) {
    if (impl_) impl_->sendMouseMove(x, y);
}

void EngineView::sendMouseButton(int32_t button, bool pressed) {
    if (impl_) impl_->sendMouseButton(button, pressed);
}

void EngineView::sendMouseWheel(float delta_x, float delta_y) {
    if (impl_) impl_->sendMouseWheel(delta_x, delta_y);
}

void EngineView::sendKey(uint32_t key_code, bool pressed) {
    if (impl_) impl_->sendKey(key_code, pressed);
}

void EngineView::sendText(const char* text) {
    if (impl_) impl_->sendText(text);
}

bool EngineView::evalScript(const char* code) {
    return impl_ ? impl_->evalScript(code) : false;
}

const void* EngineView::getCommandList() const {
    return (impl_ && impl_->cached_cmd_list) ? impl_->cached_cmd_list.get() : nullptr;
}

void EngineView::invalidateCommands() {
    if (impl_) impl_->markNeedsRepaint();
}

} // namespace dong
