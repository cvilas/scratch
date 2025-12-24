cmake_minimum_required(VERSION 4.0.0)
project(logl LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_SOURCE_DIR}/cmake/formatter.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/linter.cmake)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

FetchContent_Declare(
    glfw
    URL ${CMAKE_SOURCE_DIR}/third_party/glfw-3.4.zip
    URL_HASH MD5=770b7cc3751f71a4fa0a95386a6ea415
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(glfw)
set_target_properties(glfw PROPERTIES C_CLANG_TIDY "")

FetchContent_Declare(
    glad 
    URL ${CMAKE_SOURCE_DIR}/third_party/glad-2.0.8.zip
    URL_HASH MD5=189a93732f3d86ceed3939603f1dd696
    SOURCE_SUBDIR cmake
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    message("Fetching glad")
    FetchContent_MakeAvailable(glad)
    add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
    glad_add_library(glad REPRODUCIBLE EXCLUDE_FROM_ALL LOADER API gl:core=4.6 EXTENSIONS GL_ARB_bindless_texture GL_EXT_texture_compression_s3tc)
endif()
set_target_properties(glad PROPERTIES C_CLANG_TIDY "")

FetchContent_Declare(
    glm
    URL ${CMAKE_SOURCE_DIR}/third_party/glm-1.0.2.zip
    URL_HASH MD5=6fcf2c6e3f65f5406aad0f2901591897
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(glm)
set_target_properties(glm PROPERTIES C_CLANG_TIDY "")

add_executable(${PROJECT_NAME}_hello_window src/01_hello_window.cpp)
add_clang_format(${PROJECT_NAME}_hello_window)
target_link_libraries(${PROJECT_NAME}_hello_window PRIVATE glfw OpenGL::OpenGL)

add_executable(${PROJECT_NAME}_hello_triangle src/02_hello_triangle.cpp)
add_clang_format(${PROJECT_NAME}_hello_triangle)
target_link_libraries(${PROJECT_NAME}_hello_triangle PRIVATE glad glfw OpenGL::OpenGL Threads::Threads)
