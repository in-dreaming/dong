add_executable(dong_runner
    src/main.cpp
)

target_compile_features(dong_runner PRIVATE cxx_std_20)

# Platform libraries
if(WIN32)
    target_link_libraries(dong_runner PRIVATE shell32)
endif()
