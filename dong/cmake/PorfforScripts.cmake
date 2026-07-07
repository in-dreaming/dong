# Porffor AOT script compilation for Dong
# Requires: Node.js + git submodule dong/third_party/porffor (branch enjin_porffor)
#           + npm install in third_party/porffor (acorn runtime dep)

set(DONG_PORFFOR_MANIFEST "${CMAKE_SOURCE_DIR}/scripts/porffor_manifest.json")
set(DONG_PORFFOR_OUT_DIR "${CMAKE_SOURCE_DIR}/generated/porffor")
set(DONG_PORFFOR_COMPILE_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/porffor_compile.mjs")

add_custom_command(
    OUTPUT
        "${DONG_PORFFOR_OUT_DIR}/registry.c"
        "${DONG_PORFFOR_OUT_DIR}/registry.h"
        "${DONG_PORFFOR_OUT_DIR}/sources.cmake"
    COMMAND ${CMAKE_COMMAND} -E env "NODE_NO_WARNINGS=1"
            node "${DONG_PORFFOR_COMPILE_SCRIPT}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    DEPENDS
        "${DONG_PORFFOR_COMPILE_SCRIPT}"
        "${CMAKE_SOURCE_DIR}/scripts/porffor_inline_handlers.mjs"
        "${CMAKE_SOURCE_DIR}/scripts/porffor_paths.mjs"
        "${DONG_PORFFOR_MANIFEST}"
        "${CMAKE_SOURCE_DIR}/third_party/porffor/compiler/index.js"
        "${CMAKE_SOURCE_DIR}/third_party/porffor/compiler/builtins.js"
        "${CMAKE_SOURCE_DIR}/src/script/porffor/dong_porffor_prelude.js"
    COMMENT "Compiling Porffor manifest scripts to C"
    VERBATIM
)

add_custom_target(dong_porffor_scripts
    DEPENDS "${DONG_PORFFOR_OUT_DIR}/registry.c"
)

function(dong_attach_porffor_scripts target_name)
    add_dependencies(${target_name} dong_porffor_scripts)
    if(EXISTS "${DONG_PORFFOR_OUT_DIR}/sources.cmake")
        include("${DONG_PORFFOR_OUT_DIR}/sources.cmake")
    else()
        set(DONG_PORFFOR_MODULE_SOURCES)
    endif()
    target_sources(${target_name} PRIVATE
        "${DONG_PORFFOR_OUT_DIR}/registry.c"
        ${DONG_PORFFOR_MODULE_SOURCES}
        src/script/porffor/dong_porf_host.cpp
        src/script/porffor/porffor_script_registry.cpp
        src/script/porffor/script_engine_porffor.cpp
        src/script/porffor/js_bindings_porffor.cpp
        src/script/porffor/js_scene_porffor.cpp
        src/script/porffor/js_text_layout_porffor.cpp
    )
    target_include_directories(${target_name} PRIVATE
        "${DONG_PORFFOR_OUT_DIR}"
        "${CMAKE_SOURCE_DIR}/src/script/porffor"
    )
endfunction()
