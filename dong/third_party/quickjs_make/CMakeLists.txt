cmake_minimum_required(VERSION 3.16)
project(quickjs C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_SHARED_LIBS "Build shared library" ON)

# QuickJS source directory (git submodule)
set(QUICKJS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../quickjs")

# Source files
set(QUICKJS_SOURCES
    ${QUICKJS_SRC_DIR}/quickjs.c
    ${QUICKJS_SRC_DIR}/libregexp.c
    ${QUICKJS_SRC_DIR}/libunicode.c
    ${QUICKJS_SRC_DIR}/cutils.c
    ${QUICKJS_SRC_DIR}/dtoa.c
)

# Create library
if(BUILD_SHARED_LIBS)
    add_library(quickjs SHARED ${QUICKJS_SOURCES})
    target_compile_definitions(quickjs PRIVATE JS_SHARED_LIBRARY)
    if(WIN32)
        set_target_properties(quickjs PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            PREFIX ""
            IMPORT_PREFIX ""
            RUNTIME_OUTPUT_NAME "quickjs"
            ARCHIVE_OUTPUT_NAME "quickjs"
        )
    endif()
else()
    add_library(quickjs STATIC ${QUICKJS_SOURCES})
endif()

target_include_directories(quickjs PUBLIC 
    $<BUILD_INTERFACE:${QUICKJS_SRC_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(quickjs PRIVATE _GNU_SOURCE CONFIG_VERSION="2024-02-14")

if(WIN32)
    target_compile_definitions(quickjs PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(MINGW)
        # MinGW specific settings
        target_compile_definitions(quickjs PRIVATE __USE_MINGW_ANSI_STDIO=1)
        target_link_libraries(quickjs PRIVATE pthread)
    endif()
endif()

# Install - use explicit install commands for MinGW compatibility
if(WIN32 AND BUILD_SHARED_LIBS)
    # Install DLL to bin directory
    install(FILES $<TARGET_FILE:quickjs> DESTINATION bin)
    # Install import library to lib directory
    install(FILES $<TARGET_LINKER_FILE:quickjs> DESTINATION lib)
else()
    install(TARGETS quickjs
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

install(FILES 
    ${QUICKJS_SRC_DIR}/quickjs.h
    ${QUICKJS_SRC_DIR}/quickjs-libc.h
    DESTINATION include
)
