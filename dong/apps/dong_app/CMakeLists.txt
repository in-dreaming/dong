# dong_app - Simple HTML Viewer using AppCore
#
# This app uses the new AppCore architecture instead of dong_legacy

add_executable(dong_app
    src/main.c
)

target_include_directories(dong_app PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/appcore/include
)

# SDL3 link directory (AppCore depends on SDL3)
set(SDL3_PREFIX "${CMAKE_SOURCE_DIR}/zig-out/sdl3" CACHE PATH "SDL3 install prefix")
target_link_directories(dong_app PRIVATE "${SDL3_PREFIX}/lib")

# Link to AppCore (which links to dong_legacy and SDL3)
target_link_libraries(dong_app PRIVATE dong_appcore)

# Vulkan SDK for SPIRV cross (needed by dong_legacy via SDL_shadercross)
set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}" CACHE PATH "Vulkan SDK path")
if(VULKAN_SDK_PATH)
    if(WIN32)
        target_link_directories(dong_app PRIVATE "${VULKAN_SDK_PATH}/Lib")
    else()
        target_link_directories(dong_app PRIVATE "${VULKAN_SDK_PATH}/lib")
    endif()
    target_link_libraries(dong_app PRIVATE spirv-cross-c-shared)
endif()

# DXC library (needed by dong_legacy via SDL_shadercross)
set(DXC_LIB_PATH "${CMAKE_SOURCE_DIR}/third_party/dxc_2025_07_14/lib/x64" CACHE PATH "DXC library path")
target_link_directories(dong_app PRIVATE ${DXC_LIB_PATH})
target_link_libraries(dong_app PRIVATE dxcompiler)

# Platform libraries
if(WIN32)
    target_link_libraries(dong_app PRIVATE user32 gdi32 ole32 shell32 advapi32)
elseif(APPLE)
    target_link_libraries(dong_app PRIVATE
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreText"
    )
endif()

# Install
install(TARGETS dong_app RUNTIME DESTINATION bin)
