cd /d d:\mix\agents\game\indr\dong\dong\zig-out\bin
rem set DONG_DEBUG_MARK_NEEDS_REPAINT=1
rem set DONG_DEBUG_OFFSCREEN_REBUILD_REASON=1
rem dong_app.exe --html .\data\video\video_play_test.html --width 960 --height 540 --frames 120 --frame-ms 33 --click 520,120 --click-frame 10

@REM 3d_screen_script.exe --profiler trace.json

rem set DONG_HUD_KEY_WHITE_BG=1
@REM set DONG_DEBUG_VIDEO_UPLOAD=1
@REM set DONG_DEBUG_VIDEO_PAINT=1
set DONG_TEXT_RENDERER=slug
dong_app.exe --html .\data\tests\chinese_font_small_test.html
@rem 3d_screens_simple.exe