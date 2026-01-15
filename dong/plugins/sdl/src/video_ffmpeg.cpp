#include "video_ffmpeg.h"

#if defined(DONG_PLUGIN_SDL_HAS_FFMPEG) && DONG_PLUGIN_SDL_HAS_FFMPEG

#include <SDL3/SDL_log.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>
#include <filesystem>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// This type is part of the plugin C ABI (forward-declared in dong_plugin_api.h).
// Keep it in the global namespace so `dong_video_player_t*` matches across TUs.
struct dong_video_player_t {
    AVFormatContext* fmt = nullptr;
    AVCodecContext* vdec = nullptr;
    int video_stream = -1;
    AVRational video_time_base = {1, 1};

    AVPacket* pkt = nullptr;
    AVFrame* frame = nullptr;

    SwsContext* sws = nullptr;

    std::vector<uint8_t> rgba;
    uint8_t* rgba_data[4] = {nullptr, nullptr, nullptr, nullptr};
    int rgba_linesize[4] = {0, 0, 0, 0};

    dong_video_metadata_t meta = {};
};

namespace {



// Small helper: load FFmpeg DLLs next to dong_plugin_sdl.dll and resolve only the symbols we use.
struct FF {
#if defined(_WIN32)
    HMODULE avutil = nullptr;
    HMODULE swscale = nullptr;
    HMODULE avcodec = nullptr;
    HMODULE avformat = nullptr;
#endif

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

static std::string win32_last_error() {
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
    return "";
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
    return std::filesystem::current_path();
#endif
}

static std::filesystem::path find_first_dll(const std::filesystem::path& dir, const char* prefix) {
    if (!std::filesystem::exists(dir)) return {};

    std::filesystem::path best;
    for (const auto& it : std::filesystem::directory_iterator(dir)) {
        if (!it.is_regular_file()) continue;
        auto p = it.path();
        auto name = p.filename().string();
        if (name.size() < 4) continue;
        if (!std::equal(name.begin(), name.begin() + std::strlen(prefix), prefix)) continue;
        if (p.extension().string() != ".dll") continue;

        // Choose the lexicographically largest match (usually highest version).
        if (best.empty() || name > best.filename().string()) {
            best = p;
        }
    }

    return best;
}

static void* load_sym(HMODULE mod, const char* name) {
#if defined(_WIN32)
    if (!mod) return nullptr;
    return reinterpret_cast<void*>(GetProcAddress(mod, name));
#else
    (void)mod; (void)name;
    return nullptr;
#endif
}

static void ff_init_once() {
    std::call_once(g_ff_once, []() {
#if !defined(_WIN32)
        SDL_Log("[dong_plugin_sdl][video] FFmpeg loader not implemented for this platform");
        g_ff.ok = false;
        return;
#else
        const auto dir = get_self_dir();
        const auto avutil = find_first_dll(dir, "avutil-");
        const auto swscale = find_first_dll(dir, "swscale-");
        const auto avcodec = find_first_dll(dir, "avcodec-");
        const auto avformat = find_first_dll(dir, "avformat-");

        if (avutil.empty() || swscale.empty() || avcodec.empty() || avformat.empty()) {
            SDL_Log("[dong_plugin_sdl][video] FFmpeg DLLs not found next to plugin. dir=%s", dir.string().c_str());
            g_ff.ok = false;
            return;
        }

        g_ff.avutil = LoadLibraryA(avutil.string().c_str());
        g_ff.swscale = LoadLibraryA(swscale.string().c_str());
        g_ff.avcodec = LoadLibraryA(avcodec.string().c_str());
        g_ff.avformat = LoadLibraryA(avformat.string().c_str());

        if (!g_ff.avutil || !g_ff.swscale || !g_ff.avcodec || !g_ff.avformat) {
            SDL_Log("[dong_plugin_sdl][video] LoadLibrary failed: %s", win32_last_error().c_str());
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
            SDL_Log("[dong_plugin_sdl][video] FFmpeg loaded from %s", dir.string().c_str());
        }
#endif
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



static void player_close(dong_video_player_t* p) {
    ff_init_once();
    if (!p) return;

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
    p->sws = g_ff.p_sws_getContext(
        w, h, p->vdec->pix_fmt,
        w, h, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!p->sws) {
        SDL_Log("[dong_plugin_sdl][video] sws_getContext failed");
        player_close(p);
        delete p;
        return nullptr;
    }

    // Allocate an RGBA buffer without calling into FFmpeg (we don't link FFmpeg; we load DLLs at runtime).
    if (w <= 0 || h <= 0) {
        SDL_Log("[dong_plugin_sdl][video] invalid video size: %dx%d", w, h);
        player_close(p);
        delete p;
        return nullptr;
    }

    p->rgba_linesize[0] = w * 4;
    p->rgba.resize((size_t)p->rgba_linesize[0] * (size_t)h);

    p->rgba_data[0] = p->rgba.data();
    p->rgba_data[1] = nullptr;
    p->rgba_data[2] = nullptr;
    p->rgba_data[3] = nullptr;
    p->rgba_linesize[1] = 0;
    p->rgba_linesize[2] = 0;
    p->rgba_linesize[3] = 0;


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
    out_frame->format = DONG_VIDEO_PIXEL_FORMAT_RGBA8;

    while (true) {
        int ret = g_ff.p_av_read_frame(player->fmt, player->pkt);
        if (ret < 0) {
            // EOF or error.
            return 0;
        }

        if (player->pkt->stream_index != player->video_stream) {
            g_ff.p_av_packet_unref(player->pkt);
            continue;
        }

        ret = g_ff.p_avcodec_send_packet(player->vdec, player->pkt);
        g_ff.p_av_packet_unref(player->pkt);
        if (ret < 0) {
            SDL_Log("[dong_plugin_sdl][video] avcodec_send_packet failed: %s", ff_err(ret).c_str());
            return -1;
        }

        ret = g_ff.p_avcodec_receive_frame(player->vdec, player->frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        }
        if (ret < 0) {
            SDL_Log("[dong_plugin_sdl][video] avcodec_receive_frame failed: %s", ff_err(ret).c_str());
            return -1;
        }

        // Convert to RGBA.
        const int h = player->frame->height;
        g_ff.p_sws_scale(player->sws,
                         player->frame->data,
                         player->frame->linesize,
                         0,
                         h,
                         player->rgba_data,
                         player->rgba_linesize);

        // PTS.
        double pts_sec = 0.0;
        int64_t pts = player->frame->best_effort_timestamp;
        if (pts != AV_NOPTS_VALUE) {
            pts_sec = (double)pts * av_q2d(player->video_time_base);
        }

        out_frame->width = (uint32_t)player->vdec->width;
        out_frame->height = (uint32_t)player->vdec->height;
        out_frame->stride_bytes = (uint32_t)player->rgba_linesize[0];
        out_frame->data = player->rgba_data[0];
        out_frame->pts_seconds = pts_sec;

        g_ff.p_av_frame_unref(player->frame);
        return 1;
    }
}

int sdl_video_seek(void* /*user*/, dong_video_player_t* player, double time_seconds) {
    ff_init_once();
    if (!g_ff.ok) return -1;
    if (!player) return -1;

    AVStream* st = player->fmt->streams[player->video_stream];
    if (!st) return -1;

    const int64_t target_ts = (int64_t)(time_seconds / av_q2d(st->time_base));
    int ret = g_ff.p_av_seek_frame(player->fmt, player->video_stream, target_ts, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        SDL_Log("[dong_plugin_sdl][video] av_seek_frame failed: %s", ff_err(ret).c_str());
        return -1;
    }

    // Flush decoder after seek.
    // NOTE: Do NOT use avcodec_send_packet(NULL) here; that puts the decoder into draining state.
    if (g_ff.p_avcodec_flush_buffers) {
        g_ff.p_avcodec_flush_buffers(player->vdec);
    }
    if (player->pkt) {
        g_ff.p_av_packet_unref(player->pkt);
    }
    if (player->frame) {
        g_ff.p_av_frame_unref(player->frame);
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
