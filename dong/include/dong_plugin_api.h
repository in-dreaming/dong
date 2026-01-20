#ifndef DONG_PLUGIN_API_H
#define DONG_PLUGIN_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Dong plugin interface (C ABI)
// - Implemented by platform backends (e.g. SDL) as a separate DLL.
// - Loaded by dong_app, then injected into dong.dll.
// =============================================================================

#define DONG_PLUGIN_API_VERSION 4u

// Capability bitmask for optional subsystems.
typedef uint64_t dong_plugin_caps_t;

enum {
    DONG_PLUGIN_CAP_LOG = 1ull << 0,
    DONG_PLUGIN_CAP_TIME = 1ull << 1,
    DONG_PLUGIN_CAP_FS = 1ull << 2,
    DONG_PLUGIN_CAP_WINDOW = 1ull << 3,
    DONG_PLUGIN_CAP_INPUT = 1ull << 4,
    DONG_PLUGIN_CAP_RENDERER = 1ull << 5,
    DONG_PLUGIN_CAP_VIDEO = 1ull << 6,
};


typedef enum dong_log_level_t {
    DONG_LOG_TRACE = 0,
    DONG_LOG_DEBUG = 1,
    DONG_LOG_INFO = 2,
    DONG_LOG_WARN = 3,
    DONG_LOG_ERROR = 4,
} dong_log_level_t;

typedef struct dong_plugin_info_t {
    uint32_t plugin_api_version; // must be DONG_PLUGIN_API_VERSION
    dong_plugin_caps_t capabilities;
} dong_plugin_info_t;

// Minimal input event type (extend later as needed).
typedef enum dong_input_event_type_t {
    DONG_INPUT_EVENT_NONE = 0,
    DONG_INPUT_EVENT_QUIT = 1,
    DONG_INPUT_EVENT_MOUSE_MOVE = 2,
    DONG_INPUT_EVENT_MOUSE_BUTTON = 3,
    DONG_INPUT_EVENT_MOUSE_WHEEL = 4,
    DONG_INPUT_EVENT_KEY = 5,
    DONG_INPUT_EVENT_TEXT = 6,
    DONG_INPUT_EVENT_WINDOW_RESIZE = 7,
} dong_input_event_type_t;

typedef struct dong_input_event_t {
    dong_input_event_type_t type;
    uint32_t a;
    uint32_t b;
    float x;
    float y;
    float dx;
    float dy;
    const char* text; // only valid for TEXT events, UTF-8, lifetime defined by plugin.
} dong_input_event_t;

// Window creation descriptor
typedef struct dong_window_desc_t {
    const char* title;
    uint32_t width;
    uint32_t height;
    uint32_t flags; // reserved
} dong_window_desc_t;

// Opaque window handle
typedef struct dong_window_t dong_window_t;

// GPU device handle (opaque, platform-specific)
typedef struct dong_gpu_device_t dong_gpu_device_t;

// A backend-specific renderer command stream (opaque to core for now).
typedef struct dong_renderer_cmd_stream_t {
    const void* data;
    size_t size;
} dong_renderer_cmd_stream_t;

// =============================================================================
// Video (optional)
// =============================================================================

typedef struct dong_video_player_t dong_video_player_t;

typedef enum dong_video_pixel_format_t {
    DONG_VIDEO_PIXEL_FORMAT_RGBA8 = 1,
    DONG_VIDEO_PIXEL_FORMAT_YUV420P = 2,
} dong_video_pixel_format_t;

typedef struct dong_video_metadata_t {
    uint32_t video_width;
    uint32_t video_height;
    double duration_seconds;
    uint32_t has_video;
    uint32_t has_audio;
} dong_video_metadata_t;

typedef struct dong_video_frame_t {
    dong_video_pixel_format_t format;
    uint32_t width;
    uint32_t height;

    // Convenience/compat for single-plane formats:
    // - RGBA8: stride_bytes = bytes per row, data = RGBA pointer
    // - YUV420P: stride_bytes = Y plane bytes per row, data = Y plane pointer
    uint32_t stride_bytes;
    const uint8_t* data;

    // Multi-plane data for YUV formats.
    // - YUV420P: plane_data[0]=Y, [1]=U, [2]=V
    // - RGBA8: plane_data[0]=data, others null
    const uint8_t* plane_data[3];
    uint32_t plane_stride_bytes[3];

    double pts_seconds;
} dong_video_frame_t;

typedef struct dong_plugin_vtable_t {
    dong_plugin_info_t info;


    // log
    void (*log)(void* user, dong_log_level_t level, const char* msg);

    // time
    uint64_t (*now_ns)(void* user);

    // window (optional, requires DONG_PLUGIN_CAP_WINDOW)
    dong_window_t* (*window_create)(void* user, const dong_window_desc_t* desc);
    void (*window_destroy)(void* user, dong_window_t* window);
    void (*window_get_size)(void* user, dong_window_t* window, uint32_t* out_w, uint32_t* out_h);

    // input
    // Returns 1 if an event is written to out_event, 0 if no event.
    int (*poll_event)(void* user, dong_input_event_t* out_event);

    // renderer (optional, requires DONG_PLUGIN_CAP_RENDERER)
    dong_gpu_device_t* (*renderer_init)(void* user, dong_window_t* window);
    void (*renderer_shutdown)(void* user, dong_gpu_device_t* device);
    int (*renderer_begin_frame)(void* user, dong_gpu_device_t* device);
    int (*renderer_submit)(void* user, dong_gpu_device_t* device, const dong_renderer_cmd_stream_t* stream);
    int (*renderer_end_frame)(void* user, dong_gpu_device_t* device);

    // Get raw platform handles (for advanced integration)
    void* (*get_native_window_handle)(void* user, dong_window_t* window);
    void* (*get_native_gpu_device)(void* user, dong_gpu_device_t* device);

    // video (optional, requires DONG_PLUGIN_CAP_VIDEO)
    dong_video_player_t* (*video_open)(void* user, const char* url);
    void (*video_close)(void* user, dong_video_player_t* player);
    int (*video_get_metadata)(void* user, dong_video_player_t* player, dong_video_metadata_t* out);
    int (*video_read_frame)(void* user, dong_video_player_t* player, dong_video_frame_t* out_frame);
    int (*video_seek)(void* user, dong_video_player_t* player, double time_seconds);
} dong_plugin_vtable_t;


// The exported symbol type from plugin DLL.
// dong_app will LoadLibrary/dlsym this to obtain the vtable.
typedef const dong_plugin_vtable_t* (*dong_plugin_get_api_fn)(void);

#ifdef __cplusplus
}
#endif

#endif // DONG_PLUGIN_API_H
