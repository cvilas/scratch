cmake_minimum_required(VERSION 3.6)
project(libcamera-test DESCRIPTION "Example application for libcamera" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hello hello.cpp)
target_link_libraries(hello -lcamera -lcamera-base)
target_include_directories(hello PRIVATE /usr/include/libcamera)
