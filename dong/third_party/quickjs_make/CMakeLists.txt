cmake_minimum_required(VERSION 3.16)
project(quickjs C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Always build static library for ABI compatibility
# This ensures QuickJS links correctly with the main project regardless of compiler
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# QuickJS source directory (git submodule)
set(QUICKJS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../quickjs")

# Source files - only core files needed (not quickjs-libc.c which has more POSIX deps)
set(QUICKJS_SOURCES
    ${QUICKJS_SRC_DIR}/quickjs.c
    ${QUICKJS_SRC_DIR}/libregexp.c
    ${QUICKJS_SRC_DIR}/libunicode.c
    ${QUICKJS_SRC_DIR}/cutils.c
    ${QUICKJS_SRC_DIR}/dtoa.c
)

# Create static library
add_library(quickjs STATIC ${QUICKJS_SOURCES})

target_include_directories(quickjs PUBLIC 
    $<BUILD_INTERFACE:${QUICKJS_SRC_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Add compat directory for Windows builds (provides sys/time.h)
if(WIN32 AND NOT MINGW)
    target_include_directories(quickjs PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}/compat"
    )
endif()

target_compile_definitions(quickjs PRIVATE 
    _GNU_SOURCE 
    CONFIG_VERSION="2024-02-14"
)

if(WIN32)
    target_compile_definitions(quickjs PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(MINGW)
        # MinGW specific settings
        target_compile_definitions(quickjs PRIVATE __USE_MINGW_ANSI_STDIO=1)
    else()
        # MSVC/clang-cl specific settings
        # Disable CONFIG_ATOMICS by pretending to be EMSCRIPTEN for this check
        # This avoids the pthread_cond dependency
        target_compile_definitions(quickjs PRIVATE 
            _CRT_NONSTDC_NO_DEPRECATE
            EMSCRIPTEN=1
        )
        # Suppress specific warnings and force include alloca.h
        if(CMAKE_C_COMPILER_ID MATCHES "Clang")
            target_compile_options(quickjs PRIVATE 
                -Wno-implicit-const-int-float-conversion
                -Wno-deprecated-declarations
                /FIalloca.h
            )
        endif()
    endif()
endif()

# Install
install(TARGETS quickjs
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES 
    ${QUICKJS_SRC_DIR}/quickjs.h
    ${QUICKJS_SRC_DIR}/quickjs-libc.h
    DESTINATION include
)
