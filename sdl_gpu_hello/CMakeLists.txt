cmake_minimum_required(VERSION 3.10)
project(sdl_gpu_hello)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_SOURCE_DIR}/cmake/formatter.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/linter.cmake)


find_package(SDL3 REQUIRED)

add_custom_command(
    OUTPUT 
        ${CMAKE_CURRENT_BINARY_DIR}/vertex.spv
        ${CMAKE_CURRENT_BINARY_DIR}/fragment.spv
    COMMAND glslc -fshader-stage=vertex ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex.glsl -o ${CMAKE_CURRENT_BINARY_DIR}/vertex.spv
    COMMAND glslc -fshader-stage=fragment ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment.glsl -o ${CMAKE_CURRENT_BINARY_DIR}/fragment.spv
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex.glsl
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment.glsl
    COMMENT "Compiling shaders"
    VERBATIM
)

add_custom_target(compile_shaders ALL
    DEPENDS 
        ${CMAKE_CURRENT_BINARY_DIR}/vertex.spv
        ${CMAKE_CURRENT_BINARY_DIR}/fragment.spv
)

add_executable(sdl_gpu_hello main.cpp)
add_clang_format(sdl_gpu_hello)
target_link_libraries(sdl_gpu_hello SDL3::SDL3)
add_dependencies(sdl_gpu_hello compile_shaders)