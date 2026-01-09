add_executable(dong_runner
    src/main.cpp
)

target_compile_features(dong_runner PRIVATE cxx_std_20)

# Include tests directory for cases.hpp
target_include_directories(dong_runner PRIVATE ${CMAKE_SOURCE_DIR}/tests)

# Platform libraries
if(WIN32)
    target_link_libraries(dong_runner PRIVATE shell32)
endif()
