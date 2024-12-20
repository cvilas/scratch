cmake_minimum_required(VERSION 3.28.0)
project(grape LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Graphs)

add_executable(graph main.cpp)
target_link_libraries(graph PRIVATE Qt6::Graphs)
