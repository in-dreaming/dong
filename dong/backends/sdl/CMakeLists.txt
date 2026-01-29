cmake_minimum_required(VERSION 3.16)
project(dong_sdl_backend LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# =============================================================================
# SDL Backend Library
# =============================================================================
# This library provides SDL-based implementations of the Platform abstraction.
# It can be built as a shared or static library.

add_library(dong_sdl_backend SHARED
    dong_sdl_platform.c
)

target_include_directories(dong_sdl_backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Define build macro for DLL exports
target_compile_definitions(dong_sdl_backend PRIVATE DONG_SDL_BUILDING_DLL)

# SDL3 dependency
set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3" CACHE PATH "SDL3 install prefix")
target_include_directories(dong_sdl_backend PRIVATE "${SDL3_PREFIX}/include")
if(WIN32)
    target_link_directories(dong_sdl_backend PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_sdl_backend PRIVATE SDL3)
else()
    target_link_directories(dong_sdl_backend PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_sdl_backend PRIVATE SDL3)
endif()

# Link to core dong library for platform.h types
target_link_libraries(dong_sdl_backend PRIVATE dong)

# Install
install(TARGETS dong_sdl_backend
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    dong_sdl_platform.h
    DESTINATION include/dong/backends/sdl
)
