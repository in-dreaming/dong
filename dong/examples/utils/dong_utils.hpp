#pragma once

#include <dong.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>


namespace dong::utils {

// ============================================================================
// Dong 引擎辅助函数
// ============================================================================

// 读取文件内容
inline std::string readFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        SDL_Log("Failed to open file: %s", path);
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 获取可执行文件所在目录
inline std::string getExeDir() {
    const char* basePath = SDL_GetBasePath();
    std::string result = basePath ? basePath : "";
    return result;
}

// 构建数据文件路径
inline std::string getDataPath(const char* filename) {
    return getExeDir() + "data/" + filename;
}

// ============================================================================
// 3D HTML 屏幕结构
// ============================================================================

inline const dong_plugin_vtable_t* tryLoadSDLPluginVTable() {
    static const dong_plugin_vtable_t* s_vtable = nullptr;
    static SDL_SharedObject* s_module = nullptr;
    static bool s_tried = false;

    if (s_tried) {
        return s_vtable;
    }
    s_tried = true;

    const char* filename =
#if defined(_WIN32)
        "dong_plugin_sdl.dll";
#elif defined(__APPLE__)
        "libdong_plugin_sdl.dylib";
#else
        "libdong_plugin_sdl.so";
#endif

    const std::string path = getExeDir() + filename;
    s_module = SDL_LoadObject(path.c_str());
    if (!s_module) {
        SDL_Log("[HtmlScreen3D] Plugin not loaded (%s): %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    auto fp = SDL_LoadFunction(s_module, "dong_plugin_get_api");
    auto* fn = reinterpret_cast<dong_plugin_get_api_fn>(fp);
    if (!fn) {
        SDL_Log("[HtmlScreen3D] Plugin missing symbol dong_plugin_get_api (%s): %s", path.c_str(), SDL_GetError());
        return nullptr;
    }

    const dong_plugin_vtable_t* api = fn();
    if (!api) {
        SDL_Log("[HtmlScreen3D] Plugin returned null API (%s)", path.c_str());
        return nullptr;
    }
    if (api->info.plugin_api_version != DONG_PLUGIN_API_VERSION) {
        SDL_Log("[HtmlScreen3D] Plugin API version mismatch: got=%u want=%u (%s)",
                (unsigned)api->info.plugin_api_version,
                (unsigned)DONG_PLUGIN_API_VERSION,
                path.c_str());
        return nullptr;
    }

    s_vtable = api;
    SDL_Log("[HtmlScreen3D] Loaded plugin: %s (caps=0x%llx)", path.c_str(), (unsigned long long)api->info.capabilities);
    return s_vtable;
}

// 用于在 3D 空间中显示 HTML 内容的屏幕
struct HtmlScreen3D {
    // Dong 引擎资源
    dong_view_t* view = nullptr;
    SDL_GPUTexture* renderTexture = nullptr;
    
    // 3D 空间属性
    SDL_GPUBuffer* quadVB = nullptr;
    float posX = 0, posY = 0, posZ = 0;
    float yaw = 0;
    float width = 2.5f;      // 3D 空间中的宽度
    float height = 1.875f;   // 3D 空间中的高度
    uint32_t rtWidth = 800;  // RT 像素宽度
    uint32_t rtHeight = 600; // RT 像素高度
    
    // 交互状态
    bool hovered = false;
    bool focused = false;
    
    // 鼠标在屏幕上的位置（0-1 UV 坐标）
    float mouseU = 0, mouseV = 0;
    
    // 初始化 HTML 屏幕
    bool init(dong_context_t* ctx, SDL_GPUDevice* device, SDL_Window* window,
              const char* htmlContent, uint32_t w, uint32_t h,
              const char* resourceRoot = nullptr) {
        rtWidth = w;
        rtHeight = h;
        
        // 创建 dong view
        view = dong_view_create(ctx, rtWidth, rtHeight);
        if (!view) {
            SDL_Log("Failed to create dong view");
            return false;
        }

        // Inject platform plugin vtable (enables optional subsystems like video).
        if (const dong_plugin_vtable_t* plugin = tryLoadSDLPluginVTable()) {
            dong_view_set_plugin_api(view, plugin, nullptr);
        }
        
        // 设置 GPU 渲染模式
        dong_view_set_external_gpu_device(view, device, window);

        // 资源解析规则：相对 URL 以 resourceRoot（通常是 HTML 文件所在目录）为基准。
        // 这能让 HTML 内的 "../images/bg.png"、"screen1.js" 等相对路径行为接近浏览器。
        if (resourceRoot && resourceRoot[0]) {
            dong_view_set_resource_root(view, resourceRoot);
        } else {
            // 兜底：示例程序默认把资源根设为 <exe>/data/
            // 这样在只提供 HTML 字符串时，也能加载 data 下的资源。
            const std::string defaultRoot = getExeDir() + "data/";
            dong_view_set_resource_root(view, defaultRoot.c_str());
        }
        
        // 加载 HTML
        dong_view_load_html(view, htmlContent);

        // 注意：不调用 dong_view_update()，因为它会渲染到 swapchain
        // dong_view_render_to_gpu_texture() 内部会处理布局计算
        
        // 创建顶点缓冲区
        SDL_GPUBufferCreateInfo vbInfo{};
        vbInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        vbInfo.size = sizeof(float) * 5 * 6; // 6 vertices, 5 floats each (pos + uv)
        quadVB = SDL_CreateGPUBuffer(device, &vbInfo);
        
        if (!quadVB) {
            SDL_Log("Failed to create quad vertex buffer");
            return false;
        }
        
        return true;
    }
    
    // 更新并渲染到纹理
    void update(SDL_GPUDevice* device) {
        if (!view) return;
        
        // 注意：不调用 dong_view_update()，因为它会渲染到 swapchain
        // dong_view_render_to_gpu_texture() 内部会处理布局计算和渲染
        
        // 渲染到纹理（由 View 内部缓存并复用；这里仅持有指针用于采样）
        SDL_GPUTexture* newTexture = (SDL_GPUTexture*)dong_view_render_to_gpu_texture(
            view, device, rtWidth, rtHeight);

        if (newTexture) {
            renderTexture = newTexture;
        }

    }
    
    // 发送鼠标移动事件
    void sendMouseMove(int32_t x, int32_t y) {
        if (view) dong_view_send_mouse_move(view, x, y);
    }
    
    // 发送鼠标按下事件
    void sendMouseDown(int32_t button) {
        if (view) dong_view_send_mouse_down(view, button);
    }
    
    // 发送鼠标释放事件
    void sendMouseUp(int32_t button) {
        if (view) dong_view_send_mouse_up(view, button);
    }
    
    // 发送滚轮事件
    void sendMouseWheel(float dx, float dy) {
        if (view) dong_view_send_mouse_wheel(view, dx, dy);
    }
    
    // 发送按键事件
    void sendKeyDown(uint32_t keyCode) {
        if (view) dong_view_send_key_down(view, keyCode);
    }
    
    void sendKeyUp(uint32_t keyCode) {
        if (view) dong_view_send_key_up(view, keyCode);
    }
    
    // 发送文本输入
    void sendTextInput(const char* text) {
        if (view) dong_view_send_text_input(view, text);
    }
    
    // 执行 JavaScript 代码
    bool eval(const char* js_code) {
        if (view) return dong_view_eval(view, js_code);
        return false;
    }
    
    // 清理资源
    void cleanup(SDL_GPUDevice* device) {
        // renderTexture 由 View 内部管理，这里只清空引用
        renderTexture = nullptr;

        if (quadVB) {
            SDL_ReleaseGPUBuffer(device, quadVB);
            quadVB = nullptr;
        }
        if (view) {
            dong_view_free(view);
            view = nullptr;
        }
    }
};

// ============================================================================
// Cursor helpers (host-side)
//
// Dong 内部只计算 CSS cursor；真正设置“系统鼠标形状”需要宿主程序来做。
// 这些 helper 提供一个轻量的 CSS cursor -> SDL system cursor 映射，并做静态缓存避免泄漏。
// ============================================================================

inline SDL_SystemCursor mapCSSCursorToSDLSystemCursor(const std::string& cursor_name) {
    SDL_SystemCursor sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;

    if (cursor_name == "pointer" || cursor_name == "hand") {
        sdl_cursor = SDL_SYSTEM_CURSOR_POINTER;
    } else if (cursor_name == "text" || cursor_name == "ibeam") {
        sdl_cursor = SDL_SYSTEM_CURSOR_TEXT;
    } else if (cursor_name == "move" || cursor_name == "all-scroll") {
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;
    } else if (cursor_name == "wait") {
        sdl_cursor = SDL_SYSTEM_CURSOR_WAIT;
    } else if (cursor_name == "progress") {
        sdl_cursor = SDL_SYSTEM_CURSOR_PROGRESS;
    } else if (cursor_name == "crosshair") {
        sdl_cursor = SDL_SYSTEM_CURSOR_CROSSHAIR;
    } else if (cursor_name == "not-allowed" || cursor_name == "no-drop") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    } else if (cursor_name == "n-resize" || cursor_name == "s-resize" || cursor_name == "ns-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NS_RESIZE;
    } else if (cursor_name == "e-resize" || cursor_name == "w-resize" || cursor_name == "ew-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_EW_RESIZE;
    } else if (cursor_name == "ne-resize" || cursor_name == "sw-resize" || cursor_name == "nesw-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NESW_RESIZE;
    } else if (cursor_name == "nw-resize" || cursor_name == "se-resize" || cursor_name == "nwse-resize") {
        sdl_cursor = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    } else if (cursor_name == "grab" || cursor_name == "grabbing") {
        // SDL3 没有 grab/grabbing 光标，使用 move 代替
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;
    } else {
        // auto/default 或其他未知值
        sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;
    }

    return sdl_cursor;
}

inline void applyCSSCursor(const char* cursor_name_cstr) {
    const std::string cursor_name = (cursor_name_cstr && cursor_name_cstr[0]) ? cursor_name_cstr : "auto";

    // Avoid churn if unchanged
    static std::string s_last;
    if (cursor_name == s_last) {
        return;
    }
    s_last = cursor_name;

    if (cursor_name == "none") {
        SDL_HideCursor();
        return;
    }

    SDL_ShowCursor();

    const SDL_SystemCursor sys = mapCSSCursorToSDLSystemCursor(cursor_name);

    // Cache system cursors to avoid leaks; SDL_CreateSystemCursor returns a new cursor.
    static SDL_Cursor* s_cache[SDL_SYSTEM_CURSOR_COUNT] = {};
    if (!s_cache[sys]) {
        s_cache[sys] = SDL_CreateSystemCursor(sys);
    }
    if (s_cache[sys]) {
        SDL_SetCursor(s_cache[sys]);
    }
}

} // namespace dong::utils

