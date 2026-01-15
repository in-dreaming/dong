#pragma once

#include "dong_plugin_api.h"

// FFmpeg-backed video implementation for dong_plugin_sdl.
// NOTE: This module is optional; build is controlled by DONG_PLUGIN_SDL_HAS_FFMPEG.

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
