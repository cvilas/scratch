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
find_package(Qwt REQUIRED)
find_package(SDL3 REQUIRED)
find_package(glfw3 REQUIRED)

# Turn on to enable rendering using OpenGL for both
add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

add_executable(qcp_example 
    qcp/window.cpp 
    qcp/window.h 
    qcp/qcustomplot.cpp
    qcp/qcustomplot.h
    qcp/main.cpp)
target_link_libraries(qcp_example Qt6::Widgets Qt6::OpenGL Qt6::PrintSupport OpenGL::GL)
    
add_executable(qwt_example 
    qwt/window.cpp 
    qwt/window.h 
    qwt/main.cpp)
target_link_libraries(qwt_example qwt-qt6 Qt6::Widgets Qt6::OpenGL)

add_executable(sdlplot sdlplot.cpp)
target_link_libraries(sdlplot SDL3::SDL3)

add_executable(glfw_plot glfw_plot.cpp)
target_link_libraries(glfw_plot glfw OpenGL::GL)

add_executable(qt6plot
    qt6/main.cpp
    qt6/main.qrc
    qt6/main.qml
    qt6/widget.h 
    qt6/widget.cpp
    qt6/line_graph.h 
    qt6/line_graph.cpp
)

target_link_libraries(qt6plot
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickWidgets
        Qt6::Graphs
)
