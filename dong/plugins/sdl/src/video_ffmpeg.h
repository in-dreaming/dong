#pragma once

#include "dong_plugin_api.h"

// Video implementation for dong_plugin_sdl.
// FFmpeg is used when DONG_PLUGIN_SDL_HAS_FFMPEG is enabled; Windows builds can
// fall back to Media Foundation via DONG_PLUGIN_SDL_HAS_NATIVE_VIDEO.

#ifdef __cplusplus
extern "C" {
#endif

dong_video_player_t* sdl_video_open(void* user, const char* url);
void sdl_video_close(void* user, dong_video_player_t* player);
int sdl_video_get_metadata(void* user, dong_video_player_t* player, dong_video_metadata_t* out);
int sdl_video_read_frame(void* user, dong_video_player_t* player, dong_video_frame_t* out_frame);
int sdl_video_seek(void* user, dong_video_player_t* player, double time_seconds);

#ifdef __cplusplus
}
#endif
