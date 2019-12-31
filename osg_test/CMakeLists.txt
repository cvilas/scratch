cmake_minimum_required(VERSION 3.5)
project(osg_test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(osg)
find_package(osgViewer)
find_package(osgDB)
find_package(osgUtil)

add_executable(osg_test main.cpp README.md)
target_link_libraries(osg_test osg osgUtil osgViewer osgDB)

