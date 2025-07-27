cmake_minimum_required(VERSION 3.10)
project(scenegraph_demo)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

# Add executable
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

# Link libraries
target_link_libraries(scenegraph_demo 
    SDL3::SDL3
    ${OPENGL_LIBRARIES}
    GL
    GLU
)

# Include directories
target_include_directories(scenegraph_demo PRIVATE 
    ${OPENGL_INCLUDE_DIRS}
)