cmake_minimum_required(VERSION 3.22)

project(dyanmic_polymorphism)
set(CMAKE_CXX_STANDARD 23) 
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(type_erasure_01 type_erasure_01.cpp)
add_executable(type_erasure_02 type_erasure_02.cpp)
add_executable(crtp crtp.cpp)
