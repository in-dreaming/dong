#pragma once

#include <dong.h>
#include <dong_platform.h>
#include <dong_gpu_driver.h>
#include <dong_plugin_api.h>
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
    dong_engine_t* engine = nullptr;
    SDL_GPUTexture* renderTexture = nullptr;
    
    // 3D 空间属性
    SDL_GPUBuffer* quadVB = nullptr;
    float posX = 0, posY = 0, posZ = 0;
    float yaw = 0;
    float width = 2.5f;      // 3D 空间中的宽度
    float height = 1.875f;   // 3D 空间中的高度
    uint32_t rtWidth = 800;  // RT 像素宽度
    uint32_t rtHeight = 600; // RT 像素高度
    uint32_t texWidth = 0;
    uint32_t texHeight = 0;
    
    // 交互状态
    bool hovered = false;
    bool focused = false;
    
    // 鼠标在屏幕上的位置（0-1 UV 坐标）
    float mouseU = 0, mouseV = 0;
    
    // 初始化 HTML 屏幕
    bool init(SDL_GPUDevice* device, SDL_Window* window,
              const char* htmlContent, uint32_t w, uint32_t h,
              const char* resourceRoot = nullptr) {
        rtWidth = w;
        rtHeight = h;
        texWidth = 0;
        texHeight = 0;

        dong_engine_desc_t desc{};
        desc.api_version = DONG_API_VERSION;
        desc.plugin_api_version = DONG_PLUGIN_API_VERSION;
        desc.plugin = tryLoadSDLPluginVTable();
        desc.plugin_user = nullptr;
        desc.width = rtWidth;
        desc.height = rtHeight;

        if (dong_engine_create(&desc, &engine) != DONG_OK || !engine) {
            SDL_Log("Failed to create dong engine");
            return false;
        }

        if (dong_engine_set_gpu(engine, device, window) != DONG_OK) {
            SDL_Log("Failed to set GPU device for engine");
            dong_engine_destroy(engine);
            engine = nullptr;
            return false;
        }

        // 资源解析规则：相对 URL 以 resourceRoot（通常是 HTML 文件所在目录）为基准。
        // 这能让 HTML 内的 "../images/bg.png"、"screen1.js" 等相对路径行为接近浏览器。
        if (resourceRoot && resourceRoot[0]) {
            dong_engine_set_resource_root(engine, resourceRoot);
        } else {
            // 兜底：示例程序默认把资源根设为 <exe>/data/
            // 这样在只提供 HTML 字符串时，也能加载 data 下的资源。
            const std::string defaultRoot = getExeDir() + "data/";
            dong_engine_set_resource_root(engine, defaultRoot.c_str());
        }

        if (htmlContent && htmlContent[0]) {
            dong_engine_load_html(engine, htmlContent);
        }

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

    SDL_GPUTexture* ensureOffscreenTexture(SDL_GPUDevice* device) {
        if (!device) return nullptr;
        if (renderTexture && texWidth == rtWidth && texHeight == rtHeight) {
            return renderTexture;
        }

        if (renderTexture) {
            SDL_ReleaseGPUTexture(device, renderTexture);
            renderTexture = nullptr;
        }

        SDL_GPUTextureCreateInfo texInfo{};
        texInfo.type = SDL_GPU_TEXTURETYPE_2D;
        texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        texInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        texInfo.width = rtWidth;
        texInfo.height = rtHeight;
        texInfo.layer_count_or_depth = 1;
        texInfo.num_levels = 1;
        texInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

        renderTexture = SDL_CreateGPUTexture(device, &texInfo);
        if (!renderTexture) {
            return nullptr;
        }

        texWidth = rtWidth;
        texHeight = rtHeight;
        return renderTexture;
    }

    // 更新并渲染到纹理
    void update(SDL_GPUDevice* device) {
        if (!engine) return;

        DongGPUDriver* driver = dong_platform_get_gpu_driver(dong_platform_get());
        if (!driver) return;

        SDL_GPUTexture* target = ensureOffscreenTexture(device);
        if (!target) return;

        if (!dong_gpu_begin_frame_offscreen(driver, target, rtWidth, rtHeight)) {
            return;
        }

        if (dong_engine_tick(engine) != DONG_OK) {
            dong_gpu_end_frame_offscreen(driver);
            return;
        }

        dong_gpu_end_frame_offscreen(driver);
    }
    
    // 发送鼠标移动事件
    void sendMouseMove(int32_t x, int32_t y) {
        if (engine) dong_engine_send_mouse_move(engine, x, y);
    }
    
    // 发送鼠标按下事件
    void sendMouseDown(int32_t button) {
        if (engine) dong_engine_send_mouse_button(engine, button, 1);
    }
    
    // 发送鼠标释放事件
    void sendMouseUp(int32_t button) {
        if (engine) dong_engine_send_mouse_button(engine, button, 0);
    }
    
    // 发送滚轮事件
    void sendMouseWheel(float dx, float dy) {
        if (engine) dong_engine_send_mouse_wheel(engine, dx, dy);
    }
    
    // 发送按键事件
    void sendKeyDown(uint32_t keyCode) {
        if (engine) dong_engine_send_key(engine, keyCode, 1);
    }
    
    void sendKeyUp(uint32_t keyCode) {
        if (engine) dong_engine_send_key(engine, keyCode, 0);
    }
    
    // 发送文本输入
    void sendTextInput(const char* text) {
        if (engine) dong_engine_send_text(engine, text);
    }

    // 发送 IME 编辑事件（组合输入）
    void sendTextEditing(const char* text, int32_t cursor, int32_t selection_length) {
        if (engine) dong_engine_send_text_editing(engine, text, cursor, selection_length);
    }
    
    // 执行 JavaScript 代码
    bool eval(const char* js_code) {
        if (engine && js_code) {
            return dong_engine_eval_script(engine, js_code) == DONG_OK;
        }
        return false;
    }
    
    // 执行 JavaScript 代码并返回结果
    std::string evalWithReturn(const char* js_code) {
        if (engine && js_code) {
            (void)dong_engine_eval_script(engine, js_code);
        }
        return "";
    }
    
    // 清理资源
    void cleanup(SDL_GPUDevice* device) {
        if (renderTexture) {
            SDL_ReleaseGPUTexture(device, renderTexture);
            renderTexture = nullptr;
        }

        if (quadVB) {
            SDL_ReleaseGPUBuffer(device, quadVB);
            quadVB = nullptr;
        }
        if (engine) {
            dong_engine_destroy(engine);
            engine = nullptr;
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

