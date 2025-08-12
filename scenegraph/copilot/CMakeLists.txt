cmake_minimum_required(VERSION 3.10)
project(scenegraph_demo)

set(CMAKE_CXX_STANDARD 23)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

# Add executables
add_executable(oop_demo 
    oop/main.cpp 
    oop/scenegraph.hpp
    oop/scenegraph.cpp
    oop/camera.hpp
    oop/camera.cpp
    oop/light.hpp
    oop/light.cpp
    oop/cone.hpp
    oop/cone.cpp
    oop/sphere.hpp
    oop/sphere.cpp
    oop/cylinder.hpp
    oop/cylinder.cpp
    oop/cube.hpp
    oop/cube.cpp
    oop/plane.hpp
    oop/plane.cpp
    oop/transform.hpp
    oop/transform.cpp
    oop/separator.hpp
    oop/separator.cpp
)

target_link_libraries(oop_demo 
    SDL3::SDL3
    OpenGL::OpenGL
)

add_executable(modern_demo
    modern/modern_demo.cpp
    modern/modern_scenegraph.hpp
    modern/modern_scenegraph.cpp
)

target_link_libraries(modern_demo
    SDL3::SDL3
    OpenGL::OpenGL
    GL
    GLU
)
