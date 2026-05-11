cmake_minimum_required(VERSION 3.10)
project(plot_tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(OpenGL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick QuickWidgets Qml Graphs OpenGL PrintSupport REQUIRED)

add_subdirectory(glfw)
add_subdirectory(qcp)
add_subdirectory(qt6)
add_subdirectory(qwt)
add_subdirectory(sdl3)
