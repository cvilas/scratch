cmake_minimum_required(VERSION 3.10)

project(plot_tests)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets OpenGL PrintSupport REQUIRED)

#add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

add_executable(qcp_example 
    qcp/window.cpp 
    qcp/window.h 
    qcp/qcustomplot.cpp
    qcp/qcustomplot.h
    qcp/main.cpp)
    
add_executable(qwt_example 
    qwt/window.cpp 
    qwt/window.h 
    qwt/main.cpp)

target_link_libraries(qcp_example Qt5::Widgets Qt5::OpenGL Qt5::PrintSupport GL)
target_link_libraries(qwt_example qwt-qt5 Qt5::Widgets Qt5::OpenGL)
