find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)

add_executable(sdlplot sdlplot.cpp)
target_link_libraries(sdlplot PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf)

target_compile_options(sdlplot PRIVATE -Wall -Wextra -Wno-unused-parameter)
