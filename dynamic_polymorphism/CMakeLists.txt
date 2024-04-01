cmake_minimum_required(VERSION 3.22)

project(dyanmic_polymorphism)
set(CMAKE_CXX_STANDARD 23) 
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(type_erasure type_erasure.cpp)
add_executable(crtp crtp.cpp)
