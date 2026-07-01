# dong_gtc: GPU texture compression via gpu_texture_compress SDK (SDL GPU path)

set(DONG_GTC_ROOT "${CMAKE_SOURCE_DIR}/third_party/gpu_texture_compress")

set(DONG_GTC_SOURCES
    src/render/gtc/dong_gtc.cpp
    src/render/gtc/dong_format_policy.cpp
    src/render/gtc/dong_image_format_convert.cpp
    src/render/gtc/dong_texture_bake.cpp
    ${DONG_GTC_ROOT}/src/shader_compiler.cpp
    ${DONG_GTC_ROOT}/src/compute_dispatch.cpp
    ${DONG_GTC_ROOT}/src/texture_loader.cpp
)

function(dong_apply_gtc_target target_name)
    target_sources(${target_name} PRIVATE ${DONG_GTC_SOURCES})
    target_include_directories(${target_name} PRIVATE
        ${DONG_GTC_ROOT}/include
        ${DONG_GTC_ROOT}/sdk/include
        ${DONG_GTC_ROOT}/src
        ${DONG_GTC_ROOT}/external/stb
    )
    target_compile_definitions(${target_name} PRIVATE
        DONG_GTC_SHADER_DIR=\"${DONG_GTC_ROOT}/sdk/shaders\"
    )
endfunction()
