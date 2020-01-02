cmake_minimum_required(VERSION 3.5)
project(osg_experiments CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(osg)
find_package(osgViewer)
find_package(osgDB)
find_package(osgUtil)
find_package(GLUT)

set(examples
    00_model_file
    01_simple_shapes
    02_quad
    03_octahedron
    04_tessellate
    05_triangle_face_collector
    06_custom_drawable)

foreach(example_name ${examples})
    add_executable(${example_name} ${example_name}.cpp)
    target_include_directories(${example_name} PUBLIC ${GLUT_INCLUDE_DIRS})
    target_link_libraries(${example_name} osg osgUtil osgViewer osgDB glut GL)
endforeach()
