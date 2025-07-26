cmake_minimum_required(VERSION 3.23)
project(SceneGraphDemo)

set(CMAKE_CXX_STANDARD 23)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(scenegraph_demo 
    main.cpp 
    scenegraph.cpp
    camera.cpp
    light.cpp
    transform.cpp
    separator.cpp
    cone.cpp
    sphere.cpp
    cylinder.cpp
    cube.cpp
    plane.cpp
)
target_include_directories(scenegraph_demo PRIVATE ${GLM_INCLUDE_DIRS})
target_link_libraries(scenegraph_demo PRIVATE OpenGL::GL glfw)