cmake_minimum_required(VERSION 3.16)

project(dyanmic_polymorphism)
set(CMAKE_CXX_STANDARD 17) # Turn on C++17 compile flags
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(type_erasure type_erasure.cpp)
