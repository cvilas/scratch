cmake_minimum_required(VERSION 3.10)
project(scenegraph_demo)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

# Add executables
add_executable(scenegraph_demo 
    main.cpp 
    scenegraph.cpp
    camera.cpp
    light.cpp
    cone.cpp
    sphere.cpp
    cylinder.cpp
    cube.cpp
    plane.cpp
    transform.cpp
    separator.cpp
)

add_executable(modern_demo
    modern_example.cpp
    modern_scenegraph.cpp
)

add_executable(modern_sdl_demo
    modern_sdl_demo.cpp
    modern_scenegraph.cpp
)

# Link libraries
target_link_libraries(scenegraph_demo 
    SDL3::SDL3
    OpenGL::OpenGL
)

target_link_libraries(modern_demo
    OpenGL::OpenGL
    GL
    GLU
)

target_link_libraries(modern_sdl_demo
    SDL3::SDL3
    OpenGL::OpenGL
    GL
    GLU
)