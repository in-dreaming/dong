cmake_minimum_required(VERSION 3.24)
project(dong_gpu_backend LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

add_library(dong_gpu_backend SHARED
    dong_gpu_platform.c
)

target_include_directories(dong_gpu_backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(dong_gpu_backend PRIVATE DONG_GPU_BUILDING_DLL)

target_link_libraries(dong_gpu_backend PUBLIC dong dong_render)

if(DONG_OPTIMIZE_SIZE)
    if(WIN32 AND NOT MINGW)
        target_link_options(dong_gpu_backend PRIVATE /OPT:REF /OPT:ICF=3)
    else()
        target_compile_options(dong_gpu_backend PRIVATE -ffunction-sections -fdata-sections)
        target_compile_options(dong_gpu_backend PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
        target_link_options(dong_gpu_backend PRIVATE -Wl,--gc-sections -Wl,--icf=safe)
    endif()
endif()

install(TARGETS dong_gpu_backend
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    dong_gpu_platform.h
    DESTINATION include/dong/backends/gpu
)
