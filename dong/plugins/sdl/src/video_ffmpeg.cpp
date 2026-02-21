#include "video_ffmpeg.h"

#if defined(DONG_PLUGIN_SDL_HAS_FFMPEG) && DONG_PLUGIN_SDL_HAS_FFMPEG

#include <SDL3/SDL_log.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <chrono>
#include <string>
#include <vector>
#include <filesystem>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace {

constexpr std::size_t kFrameQueueCapacity = 6; // ~200ms buffering at 30fps

struct DecodedFrame {
    dong_video_pixel_format_t format = DONG_VIDEO_PIXEL_FORMAT_RGBA8;
    std::vector<uint8_t> storage;

    uint32_t width = 0;
    uint32_t height = 0;

    // For RGBA8: stride0 = width*4, plane_stride_bytes[0]=stride0, plane_offset[0]=0
    // For YUV420P: stride0 = width, plane_stride_bytes[0]=width, [1]=cw, [2]=cw,
    //              plane_offset points into packed storage.
    uint32_t stride0 = 0;
    uint32_t plane_stride_bytes[3] = {0, 0, 0};
    size_t plane_offset[3] = {0, 0, 0};

    double pts_seconds = 0.0;

    const uint8_t* planePtr(int plane) const {
        if (plane < 0 || plane > 2) return nullptr;
        if (plane_stride_bytes[plane] == 0) return nullptr;
        if (storage.empty()) return nullptr;
        if (plane_offset[plane] >= storage.size()) return nullptr;
        return storage.data() + plane_offset[plane];
    }
};

} // namespace

// This type is part of the plugin C ABI (forward-declared in dong_plugin_api.h).
// Keep it in the global namespace so `dong_video_player_t*` matches across TUs.
struct dong_video_player_t {
    // FFmpeg contexts (owned by player; accessed by decode thread)
    AVFormatContext* fmt = nullptr;
    AVCodecContext* vdec = nullptr;
    int video_stream = -1;
    AVRational video_time_base = {1, 1};

    // Timestamp tracking (some streams may omit per-frame PTS; use a best-effort fallback)
    double last_pts_seconds = 0.0;
    double frame_duration_seconds = 1.0 / 30.0;
    bool have_last_pts = false;

    AVPacket* pkt = nullptr;
    AVFrame* frame = nullptr;

    SwsContext* sws = nullptr;
    AVPixelFormat src_pix_fmt = AV_PIX_FMT_NONE;
    int sws_flags = SWS_FAST_BILINEAR; // used when sws conversion is needed
    dong_video_pixel_format_t out_format = DONG_VIDEO_PIXEL_FORMAT_YUV420P;

    // Async decode state
    std::thread decode_thread;
    std::atomic<bool> stop_decode{false};
    std::atomic<bool> eof{false};

    // Frame queue implemented as slot pools:
    // - decode thread writes into a free slot, then pushes the index to ready_slots.
    // - main thread pops a ready slot and holds it until the next read_frame() call.
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::vector<DecodedFrame> slots;
    std::deque<std::size_t> free_slots;
    std::deque<std::size_t> ready_slots;
    std::size_t held_slot = (std::size_t)-1; // slot index currently exposed via out_frame

    // Seek coordination
    std::mutex seek_mutex;
    std::condition_variable seek_cv;
    uint64_t seek_gen = 0;
    uint64_t seek_done_gen = 0;
    double seek_target_seconds = 0.0;
    bool seek_pending = false;

    dong_video_metadata_t meta = {};

    // Debug (used when DONG_DEBUG_VIDEO is enabled)
    int debug_publish_logged = 0;
    int debug_pts_logged = 0;
};

namespace {



// Small helper: load FFmpeg shared libraries next to dong_plugin_sdl and resolve only the symbols we use.
#if defined(_WIN32)
using ff_mod_t = HMODULE;
#else
using ff_mod_t = void*;
#endif

struct FF {
    ff_mod_t avutil = nullptr;
    ff_mod_t swscale = nullptr;
    ff_mod_t avcodec = nullptr;
    ff_mod_t avformat = nullptr;

    // libavformat
    decltype(&avformat_network_init) p_avformat_network_init = nullptr;
    decltype(&avformat_open_input) p_avformat_open_input = nullptr;
    decltype(&avformat_find_stream_info) p_avformat_find_stream_info = nullptr;
    decltype(&av_read_frame) p_av_read_frame = nullptr;
    decltype(&avformat_close_input) p_avformat_close_input = nullptr;
    decltype(&av_seek_frame) p_av_seek_frame = nullptr;

    // libavcodec
    decltype(&avcodec_find_decoder) p_avcodec_find_decoder = nullptr;
    decltype(&avcodec_alloc_context3) p_avcodec_alloc_context3 = nullptr;
    decltype(&avcodec_parameters_to_context) p_avcodec_parameters_to_context = nullptr;
    decltype(&avcodec_open2) p_avcodec_open2 = nullptr;
    decltype(&avcodec_send_packet) p_avcodec_send_packet = nullptr;
    decltype(&avcodec_receive_frame) p_avcodec_receive_frame = nullptr;
    decltype(&avcodec_flush_buffers) p_avcodec_flush_buffers = nullptr;
    decltype(&avcodec_free_context) p_avcodec_free_context = nullptr;

    // libavutil
    decltype(&av_frame_alloc) p_av_frame_alloc = nullptr;
    decltype(&av_frame_free) p_av_frame_free = nullptr;
    decltype(&av_frame_unref) p_av_frame_unref = nullptr;
    decltype(&av_packet_alloc) p_av_packet_alloc = nullptr;
    decltype(&av_packet_free) p_av_packet_free = nullptr;
    decltype(&av_packet_unref) p_av_packet_unref = nullptr;
    decltype(&av_strerror) p_av_strerror = nullptr;

    // libswscale
    decltype(&sws_getContext) p_sws_getContext = nullptr;
    decltype(&sws_scale) p_sws_scale = nullptr;
    decltype(&sws_freeContext) p_sws_freeContext = nullptr;

    bool ok = false;
};

static std::once_flag g_ff_once;
static FF g_ff;

static std::string platform_last_error() {
#if defined(_WIN32)
    DWORD err = GetLastError();
    if (err == 0) return "";
    LPSTR buf = nullptr;
    DWORD len = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buf,
        0,
        nullptr
    );
    std::string out;
    if (buf && len) {
        out.assign(buf, buf + len);
        LocalFree(buf);
    }
    return out;
#else
    const char* e = dlerror();
    return e ? std::string(e) : std::string();
#endif
}

static std::filesystem::path get_self_dir() {
#if defined(_WIN32)
    HMODULE hm = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&get_self_dir),
            &hm)) {
        return std::filesystem::current_path();
    }

    char path[MAX_PATH] = {0};
    DWORD n = GetModuleFileNameA(hm, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(path).parent_path();
#else
    Dl_info info;
    std::memset(&info, 0, sizeof(info));
    if (dladdr((void*)&get_self_dir, &info) && info.dli_fname) {
        try {
            return std::filesystem::path(info.dli_fname).parent_path();
        } catch (...) {
        }
    }
    return std::filesystem::current_path();
#endif
}

static bool is_shared_lib_name(const std::string& name) {
#if defined(_WIN32)
    return name.size() >= 4 && name.rfind(".dll") == name.size() - 4;
#elif defined(__APPLE__)
    return name.size() >= 6 && name.rfind(".dylib") == name.size() - 6;
#else
    // Linux/Unix: libfoo.so or libfoo.so.X
    return name.find(".so") != std::string::npos;
#endif
}

static std::filesystem::path find_best_shared_lib(const std::filesystem::path& dir, const char* prefix) {
    if (!std::filesystem::exists(dir)) return {};

    std::filesystem::path best;
    for (const auto& it : std::filesystem::directory_iterator(dir)) {
        if (!it.is_regular_file()) continue;
        auto p = it.path();
        auto name = p.filename().string();
        if (name.size() < 4) continue;
        if (!prefix || !prefix[0]) continue;
        if (name.size() < std::strlen(prefix)) continue;
        if (!std::equal(name.begin(), name.begin() + std::strlen(prefix), prefix)) continue;
        if (!is_shared_lib_name(name)) continue;

        // Choose the lexicographically largest match (usually highest version).
        if (best.empty() || name > best.filename().string()) {
            best = p;
        }
    }

    return best;
}

static void* load_sym(ff_mod_t mod, const char* name) {
    if (!mod || !name || !name[0]) return nullptr;
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(mod, name));
#else
    dlerror();
    return dlsym(mod, name);
#endif
}

static ff_mod_t load_library(const std::filesystem::path& p) {
#if defined(_WIN32)
    return LoadLibraryA(p.string().c_str());
#else
    dlerror();
    return dlopen(p.string().c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
}

static void ff_init_once() {
    std::call_once(g_ff_once, []() {
        const auto dir = get_self_dir();

#if defined(_WIN32)
        const auto avutil = find_best_shared_lib(dir, "avutil-");
        const auto swscale = find_best_shared_lib(dir, "swscale-");
        const auto avcodec = find_best_shared_lib(dir, "avcodec-");
        const auto avformat = find_best_shared_lib(dir, "avformat-");
#else
        const auto avutil = find_best_shared_lib(dir, "libavutil");
        const auto swscale = find_best_shared_lib(dir, "libswscale");
        const auto avcodec = find_best_shared_lib(dir, "libavcodec");
        const auto avformat = find_best_shared_lib(dir, "libavformat");
#endif

        if (avutil.empty() || swscale.empty() || avcodec.empty() || avformat.empty()) {
            SDL_Log("[dong_plugin_sdl][video] FFmpeg shared libs not found next to plugin. dir=%s", dir.string().c_str());
            g_ff.ok = false;
            return;
        }

        // Note: order matters on some platforms due to inter-lib dependencies.
        g_ff.avutil = load_library(avutil);
        g_ff.swscale = load_library(swscale);
        g_ff.avcodec = load_library(avcodec);
        g_ff.avformat = load_library(avformat);

        if (!g_ff.avutil || !g_ff.swscale || !g_ff.avcodec || !g_ff.avformat) {
            SDL_Log("[dong_plugin_sdl][video] Failed to load FFmpeg shared libs: %s", platform_last_error().c_str());
            g_ff.ok = false;
            return;
        }

        // Resolve.
        g_ff.p_avformat_network_init = reinterpret_cast<decltype(g_ff.p_avformat_network_init)>(load_sym(g_ff.avformat, "avformat_network_init"));
        g_ff.p_avformat_open_input = reinterpret_cast<decltype(g_ff.p_avformat_open_input)>(load_sym(g_ff.avformat, "avformat_open_input"));
        g_ff.p_avformat_find_stream_info = reinterpret_cast<decltype(g_ff.p_avformat_find_stream_info)>(load_sym(g_ff.avformat, "avformat_find_stream_info"));
        g_ff.p_av_read_frame = reinterpret_cast<decltype(g_ff.p_av_read_frame)>(load_sym(g_ff.avformat, "av_read_frame"));
        g_ff.p_avformat_close_input = reinterpret_cast<decltype(g_ff.p_avformat_close_input)>(load_sym(g_ff.avformat, "avformat_close_input"));
        g_ff.p_av_seek_frame = reinterpret_cast<decltype(g_ff.p_av_seek_frame)>(load_sym(g_ff.avformat, "av_seek_frame"));

        g_ff.p_avcodec_find_decoder = reinterpret_cast<decltype(g_ff.p_avcodec_find_decoder)>(load_sym(g_ff.avcodec, "avcodec_find_decoder"));
        g_ff.p_avcodec_alloc_context3 = reinterpret_cast<decltype(g_ff.p_avcodec_alloc_context3)>(load_sym(g_ff.avcodec, "avcodec_alloc_context3"));
        g_ff.p_avcodec_parameters_to_context = reinterpret_cast<decltype(g_ff.p_avcodec_parameters_to_context)>(load_sym(g_ff.avcodec, "avcodec_parameters_to_context"));
        g_ff.p_avcodec_open2 = reinterpret_cast<decltype(g_ff.p_avcodec_open2)>(load_sym(g_ff.avcodec, "avcodec_open2"));
        g_ff.p_avcodec_send_packet = reinterpret_cast<decltype(g_ff.p_avcodec_send_packet)>(load_sym(g_ff.avcodec, "avcodec_send_packet"));
        g_ff.p_avcodec_receive_frame = reinterpret_cast<decltype(g_ff.p_avcodec_receive_frame)>(load_sym(g_ff.avcodec, "avcodec_receive_frame"));
        g_ff.p_avcodec_flush_buffers = reinterpret_cast<decltype(g_ff.p_avcodec_flush_buffers)>(load_sym(g_ff.avcodec, "avcodec_flush_buffers"));
        g_ff.p_avcodec_free_context = reinterpret_cast<decltype(g_ff.p_avcodec_free_context)>(load_sym(g_ff.avcodec, "avcodec_free_context"));

        g_ff.p_av_frame_alloc = reinterpret_cast<decltype(g_ff.p_av_frame_alloc)>(load_sym(g_ff.avutil, "av_frame_alloc"));
        g_ff.p_av_frame_free = reinterpret_cast<decltype(g_ff.p_av_frame_free)>(load_sym(g_ff.avutil, "av_frame_free"));
        g_ff.p_av_frame_unref = reinterpret_cast<decltype(g_ff.p_av_frame_unref)>(load_sym(g_ff.avutil, "av_frame_unref"));
        g_ff.p_av_packet_alloc = reinterpret_cast<decltype(g_ff.p_av_packet_alloc)>(load_sym(g_ff.avcodec, "av_packet_alloc"));
        g_ff.p_av_packet_free = reinterpret_cast<decltype(g_ff.p_av_packet_free)>(load_sym(g_ff.avcodec, "av_packet_free"));
        g_ff.p_av_packet_unref = reinterpret_cast<decltype(g_ff.p_av_packet_unref)>(load_sym(g_ff.avcodec, "av_packet_unref"));
        g_ff.p_av_strerror = reinterpret_cast<decltype(g_ff.p_av_strerror)>(load_sym(g_ff.avutil, "av_strerror"));

        g_ff.p_sws_getContext = reinterpret_cast<decltype(g_ff.p_sws_getContext)>(load_sym(g_ff.swscale, "sws_getContext"));
        g_ff.p_sws_scale = reinterpret_cast<decltype(g_ff.p_sws_scale)>(load_sym(g_ff.swscale, "sws_scale"));
        g_ff.p_sws_freeContext = reinterpret_cast<decltype(g_ff.p_sws_freeContext)>(load_sym(g_ff.swscale, "sws_freeContext"));

        g_ff.ok = g_ff.p_avformat_open_input && g_ff.p_avformat_find_stream_info && g_ff.p_av_read_frame &&
                  g_ff.p_avcodec_find_decoder && g_ff.p_avcodec_alloc_context3 && g_ff.p_avcodec_parameters_to_context &&
                  g_ff.p_avcodec_open2 && g_ff.p_avcodec_send_packet && g_ff.p_avcodec_receive_frame && g_ff.p_avcodec_flush_buffers &&
                  g_ff.p_av_frame_alloc && g_ff.p_av_frame_free && g_ff.p_av_packet_alloc && g_ff.p_av_packet_free &&
                  g_ff.p_sws_getContext && g_ff.p_sws_scale && g_ff.p_sws_freeContext;

        if (!g_ff.ok) {
            SDL_Log("[dong_plugin_sdl][video] Failed to resolve required FFmpeg symbols");
        } else {
            if (g_ff.p_avformat_network_init) {
                g_ff.p_avformat_network_init();
            }
            SDL_Log("[dong_plugin_sdl][video] FFmpeg loaded (dir=%s)", dir.string().c_str());
        }
    });
}

static std::string ff_err(int err) {
    ff_init_once();
    if (!g_ff.ok || !g_ff.p_av_strerror) {
        return std::to_string(err);
    }
    char buf[256] = {0};
    g_ff.p_av_strerror(err, buf, sizeof(buf));
    return std::string(buf);
}


static void clear_ready_frames_locked(dong_video_player_t* p) {
    if (!p) return;

    // Return ready frames to free list.
    while (!p->ready_slots.empty()) {
        p->free_slots.push_back(p->ready_slots.front());
        p->ready_slots.pop_front();
    }
}

static bool decode_one_video_frame(dong_video_player_t* p, DecodedFrame& out) {
    // Decode until we get a video frame or hit EOF/error.
    while (!p->stop_decode.load(std::memory_order_acquire)) {
        int ret = g_ff.p_av_read_frame(p->fmt, p->pkt);
        if (ret < 0) {
            p->eof.store(true, std::memory_order_release);
            return false;
        }

        if (p->pkt->stream_index != p->video_stream) {
            g_ff.p_av_packet_unref(p->pkt);
            continue;
        }

        ret = g_ff.p_avcodec_send_packet(p->vdec, p->pkt);
        g_ff.p_av_packet_unref(p->pkt);
        if (ret < 0) {
            SDL_Log("[dong_plugin_sdl][video] avcodec_send_packet failed: %s", ff_err(ret).c_str());
            // Treat as fatal for this decode iteration; try to continue.
            continue;
        }

        // Drain all available frames produced from this packet.
        while (!p->stop_decode.load(std::memory_order_acquire)) {
            ret = g_ff.p_avcodec_receive_frame(p->vdec, p->frame);
            if (ret == AVERROR(EAGAIN)) {
                break;
            }
            if (ret < 0) {
                SDL_Log("[dong_plugin_sdl][video] avcodec_receive_frame failed: %s", ff_err(ret).c_str());
                return false;
            }

            const int w = p->frame->width;
            const int h = p->frame->height;
            if (w <= 0 || h <= 0) {
                g_ff.p_av_frame_unref(p->frame);
                continue;
            }

            const AVPixelFormat src_fmt = (AVPixelFormat)p->frame->format;
            const AVPixelFormat dst_fmt = (p->out_format == DONG_VIDEO_PIXEL_FORMAT_RGBA8) ? AV_PIX_FMT_RGBA : AV_PIX_FMT_YUV420P;
            const bool need_sws = (src_fmt != dst_fmt);
            if (need_sws) {
                if (!p->sws || p->src_pix_fmt != src_fmt) {
                    if (p->sws && g_ff.p_sws_freeContext) {
                        g_ff.p_sws_freeContext(p->sws);
                        p->sws = nullptr;
                    }

                    p->sws = g_ff.p_sws_getContext(
                        w, h, src_fmt,
                        w, h, dst_fmt,
                        p->sws_flags, nullptr, nullptr, nullptr
                    );
                    p->src_pix_fmt = src_fmt;

                    if (!p->sws) {
                        SDL_Log("[dong_plugin_sdl][video] sws_getContext failed (src=%d dst=%d)", (int)src_fmt, (int)dst_fmt);
                        p->eof.store(true, std::memory_order_release);
                        g_ff.p_av_frame_unref(p->frame);
                        return false;
                    }
                }
            } else {
                // If we previously needed sws but now don't, drop the cached context to avoid mismatches.
                if (p->sws && g_ff.p_sws_freeContext) {
                    g_ff.p_sws_freeContext(p->sws);
                    p->sws = nullptr;
                }
                p->src_pix_fmt = src_fmt;
            }

            // PTS.
            double pts_sec = 0.0;
            int64_t pts = p->frame->best_effort_timestamp;
            if (pts != AV_NOPTS_VALUE) {
                pts_sec = (double)pts * av_q2d(p->video_time_base);
            } else if (p->have_last_pts) {
                pts_sec = p->last_pts_seconds + p->frame_duration_seconds;
            }

            if (const char* dv = std::getenv("DONG_DEBUG_VIDEO")) {
                if (dv[0] && dv[0] != '0' && p->debug_pts_logged < 12) {
                    ++p->debug_pts_logged;
                    SDL_Log("[dong_plugin_sdl][video] ptsdbg#%d raw=%lld last=%.3f have_last=%d fdur=%.4f pre=%.3f",
                            p->debug_pts_logged,
                            (long long)pts,
                            p->last_pts_seconds,
                            p->have_last_pts ? 1 : 0,
                            p->frame_duration_seconds,
                            pts_sec);
                }
            }

            // Make timestamps monotonic for smoother playback/timeupdate semantics.
            if (p->have_last_pts && pts_sec <= p->last_pts_seconds) {
                pts_sec = p->last_pts_seconds + p->frame_duration_seconds;
            }
            p->last_pts_seconds = pts_sec;
            p->have_last_pts = true;

            out.width = static_cast<uint32_t>(w);
            out.height = static_cast<uint32_t>(h);
            out.pts_seconds = pts_sec;

            if (p->out_format == DONG_VIDEO_PIXEL_FORMAT_RGBA8) {
                out.format = DONG_VIDEO_PIXEL_FORMAT_RGBA8;
                out.stride0 = static_cast<uint32_t>(w * 4);
                out.plane_stride_bytes[0] = out.stride0;
                out.plane_stride_bytes[1] = 0;
                out.plane_stride_bytes[2] = 0;
                out.plane_offset[0] = 0;
                out.plane_offset[1] = 0;
                out.plane_offset[2] = 0;

                const size_t need = static_cast<size_t>(out.stride0) * static_cast<size_t>(h);
                if (out.storage.size() != need) {
                    out.storage.resize(need);
                }

                uint8_t* dst_data[4] = { out.storage.data(), nullptr, nullptr, nullptr };
                int dst_linesize[4] = { static_cast<int>(out.stride0), 0, 0, 0 };

                if (p->sws) {
                    g_ff.p_sws_scale(p->sws,
                                     p->frame->data,
                                     p->frame->linesize,
                                     0,
                                     h,
                                     dst_data,
                                     dst_linesize);
                } else {
                    // No conversion needed; copy from an RGBA frame (handle linesize).
                    for (int y = 0; y < h; ++y) {
                        std::memcpy(out.storage.data() + static_cast<size_t>(y) * out.stride0,
                                    p->frame->data[0] + static_cast<size_t>(y) * static_cast<size_t>(p->frame->linesize[0]),
                                    out.stride0);
                    }
                }
            } else {
                // YUV420P (preferred path)
                out.format = DONG_VIDEO_PIXEL_FORMAT_YUV420P;

                const uint32_t cw = (static_cast<uint32_t>(w) + 1u) / 2u;
                const uint32_t ch = (static_cast<uint32_t>(h) + 1u) / 2u;

                const size_t y_size = static_cast<size_t>(w) * static_cast<size_t>(h);
                const size_t u_size = static_cast<size_t>(cw) * static_cast<size_t>(ch);
                const size_t v_size = u_size;
                const size_t need = y_size + u_size + v_size;

                out.stride0 = static_cast<uint32_t>(w);
                out.plane_stride_bytes[0] = static_cast<uint32_t>(w);
                out.plane_stride_bytes[1] = cw;
                out.plane_stride_bytes[2] = cw;
                out.plane_offset[0] = 0;
                out.plane_offset[1] = y_size;
                out.plane_offset[2] = y_size + u_size;

                if (out.storage.size() != need) {
                    out.storage.resize(need);
                }

                uint8_t* dst_y = out.storage.data() + out.plane_offset[0];
                uint8_t* dst_u = out.storage.data() + out.plane_offset[1];
                uint8_t* dst_v = out.storage.data() + out.plane_offset[2];

                const AVPixelFormat src_fmt = (AVPixelFormat)p->frame->format;
                if (src_fmt == AV_PIX_FMT_YUV420P && !p->sws) {
                    // Copy planes with stride handling.
                    for (int y = 0; y < h; ++y) {
                        std::memcpy(dst_y + static_cast<size_t>(y) * static_cast<size_t>(w),
                                    p->frame->data[0] + static_cast<size_t>(y) * static_cast<size_t>(p->frame->linesize[0]),
                                    static_cast<size_t>(w));
                    }
                    for (uint32_t y = 0; y < ch; ++y) {
                        std::memcpy(dst_u + static_cast<size_t>(y) * static_cast<size_t>(cw),
                                    p->frame->data[1] + static_cast<size_t>(y) * static_cast<size_t>(p->frame->linesize[1]),
                                    static_cast<size_t>(cw));
                        std::memcpy(dst_v + static_cast<size_t>(y) * static_cast<size_t>(cw),
                                    p->frame->data[2] + static_cast<size_t>(y) * static_cast<size_t>(p->frame->linesize[2]),
                                    static_cast<size_t>(cw));
                    }
                } else {
                    // Convert to YUV420P using swscale.
                    if (!p->sws) {
                        SDL_Log("[dong_plugin_sdl][video] YUV420P requested but no sws context available (src=%d)", (int)src_fmt);
                        p->eof.store(true, std::memory_order_release);
                        g_ff.p_av_frame_unref(p->frame);
                        return false;
                    }
                    uint8_t* dst_data[4] = { dst_y, dst_u, dst_v, nullptr };
                    int dst_linesize[4] = { (int)out.plane_stride_bytes[0], (int)out.plane_stride_bytes[1], (int)out.plane_stride_bytes[2], 0 };
                    g_ff.p_sws_scale(p->sws,
                                     p->frame->data,
                                     p->frame->linesize,
                                     0,
                                     h,
                                     dst_data,
                                     dst_linesize);
                }
            }

            g_ff.p_av_frame_unref(p->frame);
            return true;
        }
    }

    return false;
}

static void decode_thread_main(dong_video_player_t* p) {
    if (!p) return;
    ff_init_once();
    if (!g_ff.ok) return;

    while (!p->stop_decode.load(std::memory_order_acquire)) {
        // Handle pending seek.
        uint64_t do_seek_gen = 0;
        double do_seek_target = 0.0;
        {
            std::lock_guard<std::mutex> lk(p->seek_mutex);
            if (p->seek_pending) {
                do_seek_gen = p->seek_gen;
                do_seek_target = p->seek_target_seconds;
                p->seek_pending = false;
            }
        }

        if (do_seek_gen != 0) {
            AVStream* st = p->fmt ? p->fmt->streams[p->video_stream] : nullptr;
            if (st && g_ff.p_av_seek_frame) {
                const int64_t target_ts = (int64_t)(do_seek_target / av_q2d(st->time_base));
                const int ret = g_ff.p_av_seek_frame(p->fmt, p->video_stream, target_ts, AVSEEK_FLAG_BACKWARD);
                if (ret < 0) {
                    SDL_Log("[dong_plugin_sdl][video] av_seek_frame failed (async): %s", ff_err(ret).c_str());
                }
            }

            // Flush decoder after seek.
            if (g_ff.p_avcodec_flush_buffers) {
                g_ff.p_avcodec_flush_buffers(p->vdec);
            }
            if (p->pkt) {
                g_ff.p_av_packet_unref(p->pkt);
            }
            if (p->frame) {
                g_ff.p_av_frame_unref(p->frame);
            }

            p->eof.store(false, std::memory_order_release);

            // Reset timestamp tracking around seek.
            p->last_pts_seconds = do_seek_target;
            p->have_last_pts = true;

            // Clear queued frames.
            {
                std::lock_guard<std::mutex> qlk(p->queue_mutex);
                clear_ready_frames_locked(p);
            }
            p->queue_cv.notify_all();

            {
                std::lock_guard<std::mutex> lk(p->seek_mutex);
                p->seek_done_gen = do_seek_gen;
            }
            p->seek_cv.notify_all();
        }

        // Acquire a free slot to decode into.
        std::size_t slot_index = (std::size_t)-1;
        {
            std::unique_lock<std::mutex> lk(p->queue_mutex);
            p->queue_cv.wait(lk, [&]() {
                return p->stop_decode.load(std::memory_order_acquire) || !p->free_slots.empty();
            });
            if (p->stop_decode.load(std::memory_order_acquire)) {
                return;
            }
            slot_index = p->free_slots.front();
            p->free_slots.pop_front();
        }

        if (slot_index == (std::size_t)-1 || slot_index >= p->slots.size()) {
            continue;
        }

        DecodedFrame& slot = p->slots[slot_index];
        const bool ok = decode_one_video_frame(p, slot);
        if (!ok) {
            // Return slot to free list.
            {
                std::lock_guard<std::mutex> lk(p->queue_mutex);
                p->free_slots.push_back(slot_index);
            }
            p->queue_cv.notify_all();

            // If EOF, avoid busy-loop.
            if (p->eof.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            continue;
        }

        // Publish slot to consumers.
        {
            std::lock_guard<std::mutex> lk(p->queue_mutex);
            p->ready_slots.push_back(slot_index);
        }

        if (const char* dv = std::getenv("DONG_DEBUG_VIDEO")) {
            if (dv[0] && dv[0] != '0' && p->debug_publish_logged < 20) {
                ++p->debug_publish_logged;
                SDL_Log("[dong_plugin_sdl][video] queued frame #%d pts=%.3f", p->debug_publish_logged, slot.pts_seconds);
            }
        }

        p->queue_cv.notify_all();
    }
}


static void player_close(dong_video_player_t* p) {
    ff_init_once();
    if (!p) return;

    // Stop decode thread first.
    if (p->decode_thread.joinable()) {
        p->stop_decode.store(true, std::memory_order_release);
        p->queue_cv.notify_all();
        p->seek_cv.notify_all();
        p->decode_thread.join();
    }

    if (p->sws && g_ff.p_sws_freeContext) {
        g_ff.p_sws_freeContext(p->sws);
        p->sws = nullptr;
    }

    if (p->frame && g_ff.p_av_frame_free) {
        g_ff.p_av_frame_free(&p->frame);
    }

    if (p->pkt && g_ff.p_av_packet_free) {
        g_ff.p_av_packet_free(&p->pkt);
    }

    if (p->vdec && g_ff.p_avcodec_free_context) {
        g_ff.p_avcodec_free_context(&p->vdec);
    }

    if (p->fmt && g_ff.p_avformat_close_input) {
        g_ff.p_avformat_close_input(&p->fmt);
    }
}

} // namespace

extern "C" {

dong_video_player_t* sdl_video_open(void* /*user*/, const char* url) {
    ff_init_once();
    if (!g_ff.ok) {
        return nullptr;
    }
    if (!url || !url[0]) {
        return nullptr;
    }

    auto* p = new dong_video_player_t();

    int ret = g_ff.p_avformat_open_input(&p->fmt, url, nullptr, nullptr);
    if (ret < 0) {
        SDL_Log("[dong_plugin_sdl][video] avformat_open_input failed: %s (%s)", ff_err(ret).c_str(), url);
        player_close(p);
        delete p;
        return nullptr;
    }

    ret = g_ff.p_avformat_find_stream_info(p->fmt, nullptr);
    if (ret < 0) {
        SDL_Log("[dong_plugin_sdl][video] avformat_find_stream_info failed: %s", ff_err(ret).c_str());
        player_close(p);
        delete p;
        return nullptr;
    }

    // Find first video stream.
    for (unsigned i = 0; i < p->fmt->nb_streams; ++i) {
        AVStream* st = p->fmt->streams[i];
        if (st && st->codecpar && st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            p->video_stream = (int)i;
            p->video_time_base = st->time_base;
            break;
        }
    }

    if (p->video_stream < 0) {
        SDL_Log("[dong_plugin_sdl][video] No video stream found");
        player_close(p);
        delete p;
        return nullptr;
    }

    AVStream* vst = p->fmt->streams[p->video_stream];

    // Derive a fallback frame duration from stream frame rate (used when PTS is missing).
    {
        double fps = 0.0;
        if (vst->avg_frame_rate.num > 0 && vst->avg_frame_rate.den > 0) {
            fps = av_q2d(vst->avg_frame_rate);
        } else if (vst->r_frame_rate.num > 0 && vst->r_frame_rate.den > 0) {
            fps = av_q2d(vst->r_frame_rate);
        }
        if (fps > 1e-3) {
            p->frame_duration_seconds = 1.0 / fps;
        }
    }

    const AVCodec* dec = g_ff.p_avcodec_find_decoder(vst->codecpar->codec_id);
    if (!dec) {
        SDL_Log("[dong_plugin_sdl][video] avcodec_find_decoder failed");
        player_close(p);
        delete p;
        return nullptr;
    }

    p->vdec = g_ff.p_avcodec_alloc_context3(dec);
    if (!p->vdec) {
        SDL_Log("[dong_plugin_sdl][video] avcodec_alloc_context3 failed");
        player_close(p);
        delete p;
        return nullptr;
    }

    ret = g_ff.p_avcodec_parameters_to_context(p->vdec, vst->codecpar);
    if (ret < 0) {
        SDL_Log("[dong_plugin_sdl][video] avcodec_parameters_to_context failed: %s", ff_err(ret).c_str());
        player_close(p);
        delete p;
        return nullptr;
    }

    ret = g_ff.p_avcodec_open2(p->vdec, dec, nullptr);
    if (ret < 0) {
        SDL_Log("[dong_plugin_sdl][video] avcodec_open2 failed: %s", ff_err(ret).c_str());
        player_close(p);
        delete p;
        return nullptr;
    }

    p->pkt = g_ff.p_av_packet_alloc();
    p->frame = g_ff.p_av_frame_alloc();
    if (!p->pkt || !p->frame) {
        SDL_Log("[dong_plugin_sdl][video] packet/frame alloc failed");
        player_close(p);
        delete p;
        return nullptr;
    }

    // Metadata
    p->meta.video_width = (uint32_t)p->vdec->width;
    p->meta.video_height = (uint32_t)p->vdec->height;
    p->meta.has_video = 1;
    p->meta.has_audio = 0;
    if (p->fmt->duration != AV_NOPTS_VALUE) {
        p->meta.duration_seconds = (double)p->fmt->duration / (double)AV_TIME_BASE;
    } else {
        p->meta.duration_seconds = 0.0;
    }

    const int w = p->vdec->width;
    const int h = p->vdec->height;
    if (w <= 0 || h <= 0) {
        SDL_Log("[dong_plugin_sdl][video] invalid video size: %dx%d", w, h);
        player_close(p);
        delete p;
        return nullptr;
    }

    // Initialize conversion state. The actual decoded frame format may not be known until the first frame.
    // We therefore create/refresh the swscale context lazily on the decode thread based on AVFrame::format.
    p->src_pix_fmt = AV_PIX_FMT_NONE;
    p->sws = nullptr;

    // Prefer GPU YUV path by default; allow opt-out for compatibility/debug.
    const bool force_rgba = ([]() {
        const char* v = std::getenv("DONG_VIDEO_FORCE_RGBA");
        return v && (v[0] == '1' || v[0] == 't' || v[0] == 'T' || v[0] == 'y' || v[0] == 'Y');
    })();

    p->out_format = force_rgba ? DONG_VIDEO_PIXEL_FORMAT_RGBA8 : DONG_VIDEO_PIXEL_FORMAT_YUV420P;

    // Video scale/convert is a hot path. Prefer a fast scaler by default.
    // Set DONG_VIDEO_SWS_QUALITY=1 to force higher quality (slower) scaling.
    p->sws_flags = ([]() {
        const char* v = std::getenv("DONG_VIDEO_SWS_QUALITY");
        if (v && (v[0] == '1' || v[0] == 't' || v[0] == 'T' || v[0] == 'y' || v[0] == 'Y')) {
            return SWS_BILINEAR;
        }
        return SWS_FAST_BILINEAR;
    })();

    // Initialize async frame slots.
    p->slots.resize(kFrameQueueCapacity);
    p->free_slots.clear();
    p->ready_slots.clear();
    for (std::size_t i = 0; i < kFrameQueueCapacity; ++i) {
        p->free_slots.push_back(i);
    }
    p->held_slot = (std::size_t)-1;

    // Start decode thread.
    p->stop_decode.store(false, std::memory_order_release);
    p->eof.store(false, std::memory_order_release);
    p->decode_thread = std::thread(decode_thread_main, p);

    // Best-effort: wait a short time for first frame so callers don't treat temporary "no frame" as an error.
    {
        std::unique_lock<std::mutex> lk(p->queue_mutex);
        p->queue_cv.wait_for(lk, std::chrono::milliseconds(250), [&]() {
            return !p->ready_slots.empty() || p->eof.load(std::memory_order_acquire);
        });
    }

    return p;
}

void sdl_video_close(void* /*user*/, dong_video_player_t* player) {
    if (!player) return;
    player_close(player);
    delete player;
}

int sdl_video_get_metadata(void* /*user*/, dong_video_player_t* player, dong_video_metadata_t* out) {
    if (!player || !out) return 0;
    *out = player->meta;
    return 1;
}

int sdl_video_read_frame(void* /*user*/, dong_video_player_t* player, dong_video_frame_t* out_frame) {
    ff_init_once();
    if (!g_ff.ok) return -1;
    if (!player || !out_frame) return -1;

    std::memset(out_frame, 0, sizeof(*out_frame));

    std::size_t new_slot = (std::size_t)-1;

    {
        std::unique_lock<std::mutex> lk(player->queue_mutex);

        if (!player->ready_slots.empty()) {
            // Swap in the newest queued frame.
            // Drop older queued frames to reduce latency and to avoid presenting stale content.
            while (player->ready_slots.size() > 1) {
                player->free_slots.push_back(player->ready_slots.front());
                player->ready_slots.pop_front();
            }

            if (player->held_slot != (std::size_t)-1) {
                player->free_slots.push_back(player->held_slot);
                player->held_slot = (std::size_t)-1;
            }

            new_slot = player->ready_slots.front();
            player->ready_slots.pop_front();
            player->held_slot = new_slot;
        }

        // If we hit EOF and there's no *new* frame available, signal EOF.
        // We still want to allow consumers to present the final decoded frame first.
        if (new_slot == (std::size_t)-1 && player->eof.load(std::memory_order_acquire) && player->ready_slots.empty()) {
            return 0;
        }

        // If we still don't have any frame to show, wait briefly for the very first frame.
        if (player->held_slot == (std::size_t)-1 && !player->eof.load(std::memory_order_acquire)) {
            player->queue_cv.wait_for(lk, std::chrono::milliseconds(200), [&]() {
                return !player->ready_slots.empty() || player->eof.load(std::memory_order_acquire);
            });

            if (!player->ready_slots.empty()) {
                new_slot = player->ready_slots.front();
                player->ready_slots.pop_front();
                player->held_slot = new_slot;
            }

            if (player->eof.load(std::memory_order_acquire) && player->held_slot == (std::size_t)-1) {
                return 0;
            }
        }
    }

    if (new_slot != (std::size_t)-1) {
        // Wake decode thread if it was waiting for a free slot.
        player->queue_cv.notify_all();
    }

    // If there is no newly queued frame, tell the caller to try again later.
    // The caller is expected to keep presenting the last uploaded frame.
    if (new_slot == (std::size_t)-1) {
        return -1;
    }

    if (player->held_slot == (std::size_t)-1) {
        // No frame available yet.
        return -1;
    }

    const DecodedFrame& f = player->slots[player->held_slot];
    if (f.storage.empty() || f.width == 0 || f.height == 0 || f.stride0 == 0) {
        return -1;
    }

    out_frame->format = f.format;
    out_frame->width = f.width;
    out_frame->height = f.height;
    out_frame->pts_seconds = f.pts_seconds;

    out_frame->data = f.planePtr(0);
    out_frame->stride_bytes = f.plane_stride_bytes[0];

    out_frame->plane_data[0] = f.planePtr(0);
    out_frame->plane_stride_bytes[0] = f.plane_stride_bytes[0];
    out_frame->plane_data[1] = f.planePtr(1);
    out_frame->plane_stride_bytes[1] = f.plane_stride_bytes[1];
    out_frame->plane_data[2] = f.planePtr(2);
    out_frame->plane_stride_bytes[2] = f.plane_stride_bytes[2];

    // For RGBA8, keep plane0 consistent with data.
    if (f.format == DONG_VIDEO_PIXEL_FORMAT_RGBA8) {
        out_frame->plane_data[0] = out_frame->data;
    }

    return 1;
}

int sdl_video_seek(void* /*user*/, dong_video_player_t* player, double time_seconds) {
    ff_init_once();
    if (!g_ff.ok) return -1;
    if (!player) return -1;

    // Clear any queued/held frames so the next read_frame returns post-seek content.
    {
        std::lock_guard<std::mutex> lk(player->queue_mutex);
        clear_ready_frames_locked(player);
        if (player->held_slot != (std::size_t)-1) {
            player->free_slots.push_back(player->held_slot);
            player->held_slot = (std::size_t)-1;
        }
    }
    player->queue_cv.notify_all();

    // Signal decode thread to perform seek.
    uint64_t gen = 0;
    {
        std::lock_guard<std::mutex> lk(player->seek_mutex);
        gen = ++player->seek_gen;
        player->seek_target_seconds = time_seconds;
        player->seek_pending = true;
    }
    player->seek_cv.notify_all();

    // Best-effort: wait briefly for seek to apply.
    {
        std::unique_lock<std::mutex> lk(player->seek_mutex);
        player->seek_cv.wait_for(lk, std::chrono::milliseconds(200), [&]() {
            return player->seek_done_gen >= gen || player->stop_decode.load(std::memory_order_acquire);
        });
    }

    // Best-effort: wait briefly for at least one decoded frame after seek.
    {
        std::unique_lock<std::mutex> lk(player->queue_mutex);
        player->queue_cv.wait_for(lk, std::chrono::milliseconds(200), [&]() {
            return !player->ready_slots.empty() || player->eof.load(std::memory_order_acquire);
        });
    }

    return 1;
}

} // extern "C"

#else

extern "C" {

dong_video_player_t* sdl_video_open(void* /*user*/, const char* /*url*/) { return nullptr; }
void sdl_video_close(void* /*user*/, dong_video_player_t* /*player*/) {}
int sdl_video_get_metadata(void* /*user*/, dong_video_player_t* /*player*/, dong_video_metadata_t* /*out*/) { return 0; }
int sdl_video_read_frame(void* /*user*/, dong_video_player_t* /*player*/, dong_video_frame_t* /*out_frame*/) { return -1; }
int sdl_video_seek(void* /*user*/, dong_video_player_t* /*player*/, double /*time_seconds*/) { return -1; }

}

#endif
