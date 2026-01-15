add_library(dong_plugin_sdl SHARED

    src/dong_plugin_sdl.cpp
    src/video_ffmpeg.cpp
)


target_include_directories(dong_plugin_sdl PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL3_PREFIX}/include
)

if(WIN32)
    target_link_directories(dong_plugin_sdl PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_plugin_sdl PRIVATE SDL3)
elseif(APPLE)
    target_link_directories(dong_plugin_sdl PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_plugin_sdl PRIVATE SDL3)
else()
    target_link_directories(dong_plugin_sdl PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_plugin_sdl PRIVATE SDL3)
endif()

# =============================================================================
# Optional: FFmpeg (build from source under third_party/ffmpeg)
# - Built as shared DLLs, but dong_plugin_sdl does NOT link against them.
# - dong_plugin_sdl will dynamically load them at runtime.
# =============================================================================
option(DONG_PLUGIN_SDL_ENABLE_FFMPEG "Build FFmpeg for video support" ON)

if(DONG_PLUGIN_SDL_ENABLE_FFMPEG)
    include(ExternalProject)

    set(FFMPEG_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/ffmpeg")
    # NOTE: FFmpeg's build system does not support a clean out-of-tree build here.
    # If you run configure from a separate dir, it generates a tiny Makefile that
    # includes "${FFMPEG_SOURCE_DIR}/Makefile" (which does not exist until you
    # configure in-source). So we build in-source and install into the build tree.
    set(FFMPEG_BUILD_DIR "${FFMPEG_SOURCE_DIR}")
    set(FFMPEG_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_install")


    if(NOT EXISTS "${FFMPEG_SOURCE_DIR}/configure")
        message(FATAL_ERROR "FFmpeg source not found at: ${FFMPEG_SOURCE_DIR}")
    endif()

    if(WIN32)
        # Prefer MSYS2/Git bash (NOT WSL bash.exe) because configure expects Windows paths.
        if(DEFINED BASH_EXECUTABLE AND BASH_EXECUTABLE MATCHES "System32[/\\]bash\\.exe$")
            unset(BASH_EXECUTABLE CACHE)
        endif()

        set(_bash_hints
            "C:/msys64/usr/bin"
            "C:/msys32/usr/bin"
            "C:/Program Files/Git/bin"
            "C:/Program Files/Git/usr/bin"
        )
        find_program(BASH_EXECUTABLE NAMES bash.exe bash HINTS ${_bash_hints})
        if(NOT BASH_EXECUTABLE)
            find_program(BASH_EXECUTABLE NAMES bash.exe bash)
        endif()
    else()
        find_program(BASH_EXECUTABLE NAMES bash)
    endif()

    find_program(MAKE_EXECUTABLE NAMES make mingw32-make nmake)

    if(NOT BASH_EXECUTABLE)
        message(FATAL_ERROR "FFmpeg build requires 'bash' in PATH (e.g. MSYS2 bash / Git bash).")
    endif()
    if(WIN32 AND BASH_EXECUTABLE MATCHES "System32[/\\]bash\\.exe$")
        message(FATAL_ERROR "Detected WSL bash at ${BASH_EXECUTABLE}. Please install MSYS2 or Git Bash and make sure its bash.exe is found first in PATH.")
    endif()

    if(NOT MAKE_EXECUTABLE)
        message(FATAL_ERROR "FFmpeg build requires 'make' in PATH (e.g. MSYS2 make / mingw32-make).")
    endif()

    set(_ffmpeg_common_args
        --prefix="${FFMPEG_INSTALL_DIR}"
        --disable-programs
        --disable-doc
        --disable-debug
        --enable-pic
        --enable-shared
        --disable-static

        # Keep the build small to avoid hitting Windows command-line length limits
        # during .def generation (makedef gets a huge list of objects otherwise).
        --disable-everything
        --disable-network

        # Libraries we need for basic video decode.
        --enable-avcodec
        --enable-avformat
        --enable-avutil
        --enable-swscale

        # Minimal demux/decoder stack (file:// + MP4/H.264).
        --enable-protocol=file
        --enable-demuxer=mov
        --enable-parser=h264
        --enable-decoder=h264
    )


    if(WIN32)
        # Prefer MinGW/MSYS toolchain in our Windows CI/dev flow.
        list(APPEND _ffmpeg_common_args --target-os=mingw32)
    endif()

    # IMPORTANT: join args into a single bash string (avoid CMake list splitting).
    string(JOIN " " _ffmpeg_common_args_str ${_ffmpeg_common_args})

    # Build-in-source: use ./configure, and distclean if we previously configured.
    # IMPORTANT: avoid ';' here (CMake treats it as a list separator and splits the command).
    set(_ffmpeg_configure_cmd "(test -f Makefile && ${MAKE_EXECUTABLE} distclean || true) && ./configure ${_ffmpeg_common_args_str}")



    set(_ffmpeg_build_cmd ${MAKE_EXECUTABLE})
    set(_ffmpeg_install_cmd ${MAKE_EXECUTABLE} install)

    # Parallel build where supported.
    if(NOT "${MAKE_EXECUTABLE}" MATCHES "nmake(\\.exe)?$")
        if(DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL})
            set(_ffmpeg_build_cmd ${MAKE_EXECUTABLE} -j $ENV{CMAKE_BUILD_PARALLEL_LEVEL})
        else()
            set(_ffmpeg_build_cmd ${MAKE_EXECUTABLE} -j)
        endif()
    endif()

    # Some Windows bash.exe distributions (notably Git's usr/bin/bash.exe when invoked directly)
    # do NOT populate PATH with coreutils. Inject PATH inside bash to make configure usable.
    set(_ffmpeg_bash_env "")
    if(WIN32)
        get_filename_component(_bash_dir "${BASH_EXECUTABLE}" DIRECTORY)
        if(_bash_dir MATCHES ".*/Git/usr/bin$" OR _bash_dir MATCHES ".*\\\\Git\\\\usr\\\\bin$")
            get_filename_component(_git_root "${_bash_dir}/../.." ABSOLUTE)
            file(TO_CMAKE_PATH "${_git_root}" _git_root_posix)
            # Avoid nested quoting problems with spaces in "Program Files".
            string(REPLACE " " "\\ " _git_root_posix_escaped "${_git_root_posix}")
            set(_ffmpeg_bash_env "export PATH=${_git_root_posix_escaped}/usr/bin:${_git_root_posix_escaped}/bin:$PATH && ")



        elseif(_bash_dir MATCHES ".*/Git/bin$" OR _bash_dir MATCHES ".*\\\\Git\\\\bin$")
            get_filename_component(_git_root "${_bash_dir}/.." ABSOLUTE)
            file(TO_CMAKE_PATH "${_git_root}" _git_root_posix)
            # Avoid nested quoting problems with spaces in "Program Files".
            string(REPLACE " " "\\ " _git_root_posix_escaped "${_git_root_posix}")
            set(_ffmpeg_bash_env "export PATH=${_git_root_posix_escaped}/usr/bin:${_git_root_posix_escaped}/bin:$PATH && ")



        endif()
    endif()

    string(JOIN " " _ffmpeg_build_cmd_str ${_ffmpeg_build_cmd})
    string(JOIN " " _ffmpeg_install_cmd_str ${_ffmpeg_install_cmd})

    ExternalProject_Add(ffmpeg_ep
        SOURCE_DIR "${FFMPEG_SOURCE_DIR}"
        CONFIGURE_COMMAND "${BASH_EXECUTABLE}" -lc "${_ffmpeg_bash_env}${_ffmpeg_configure_cmd}"
        BUILD_COMMAND "${BASH_EXECUTABLE}" -lc "${_ffmpeg_bash_env}${_ffmpeg_build_cmd_str}"
        INSTALL_COMMAND "${BASH_EXECUTABLE}" -lc "${_ffmpeg_bash_env}${_ffmpeg_install_cmd_str}"
        BUILD_IN_SOURCE 1
        UPDATE_COMMAND ""
    )



    # Headers are generated during configure; consume the installed headers.
    add_dependencies(dong_plugin_sdl ffmpeg_ep)
    target_include_directories(dong_plugin_sdl PRIVATE "${FFMPEG_INSTALL_DIR}/include")
    target_compile_definitions(dong_plugin_sdl PRIVATE DONG_PLUGIN_SDL_HAS_FFMPEG=1)

    # Copy FFmpeg runtime DLLs next to dong_plugin_sdl so loading works out-of-the-box.
    add_custom_command(TARGET dong_plugin_sdl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${FFMPEG_INSTALL_DIR}/bin"
            "$<TARGET_FILE_DIR:dong_plugin_sdl>"
        COMMENT "Copying FFmpeg DLLs to plugin output directory"
    )

    # Also install FFmpeg runtime DLLs next to the installed dong_plugin_sdl.dll
    # (zig build runs cmake --install into zig-out/bin).
    install(
        DIRECTORY "${FFMPEG_INSTALL_DIR}/bin/"
        DESTINATION bin
        FILES_MATCHING
        PATTERN "*.dll"
    )
endif()

# Export all symbols on Windows is handled by __declspec(dllexport) in code.

# Install
install(TARGETS dong_plugin_sdl RUNTIME DESTINATION bin LIBRARY DESTINATION bin)

