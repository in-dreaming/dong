cmake_minimum_required(VERSION 3.16)
project(minimal_dong_demo LANGUAGES C)

add_executable(minimal_dong_demo main.c)

# Link to AppCore (which links to dong)
target_link_libraries(minimal_dong_demo PRIVATE dong_appcore)

# Installation
install(TARGETS minimal_dong_demo RUNTIME DESTINATION bin)
