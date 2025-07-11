cmake_minimum_required(VERSION 2.8)
project(fsm)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(fsm fsm.h main.cpp)
target_link_libraries(fsm pthread)

add_executable(fsm2 fsm2.h fsm2.cpp main2.cpp)
target_link_libraries(fsm2 pthread)

add_executable(modern_fsm modern_fsm.cpp)

