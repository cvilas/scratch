cmake_minimum_required(VERSION 2.8)
project(fsm)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(fsm fsm.h main.cpp)
target_link_libraries(fsm pthread)


