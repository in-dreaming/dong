add_library(dong_plugin_sdl SHARED
    src/dong_plugin_sdl.cpp
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

# Export all symbols on Windows is handled by __declspec(dllexport) in code.

# Install
install(TARGETS dong_plugin_sdl RUNTIME DESTINATION bin LIBRARY DESTINATION bin)
