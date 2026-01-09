add_executable(dong_app
    src/main.cpp
)

target_include_directories(dong_app PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL3_PREFIX}/include
)

# Link dong_legacy for GPU rendering support (View API)
target_link_libraries(dong_app PRIVATE dong_legacy)

# SDL3
if(WIN32)
    target_link_directories(dong_app PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_app PRIVATE SDL3)
elseif(APPLE)
    target_link_directories(dong_app PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_app PRIVATE SDL3)
else()
    target_link_directories(dong_app PRIVATE "${SDL3_PREFIX}/lib")
    target_link_libraries(dong_app PRIVATE SDL3)
endif()

# Vulkan SDK (for SPIRV-Cross)
if(VULKAN_SDK_PATH)
    if(WIN32)
        target_link_directories(dong_app PRIVATE "${VULKAN_SDK_PATH}/Lib")
    else()
        target_link_directories(dong_app PRIVATE "${VULKAN_SDK_PATH}/lib")
    endif()
endif()
target_link_libraries(dong_app PRIVATE spirv-cross-c-shared)

# DXC
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
