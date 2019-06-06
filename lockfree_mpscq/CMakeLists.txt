cmake_minimum_required(VERSION 3.10)
project(mpscq LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # Turn on C++17 compile flags
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Yes we really need it
set(CMAKE_CXX_EXTENSIONS OFF) # Turn off non-standard extensions to ISO C++
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Required for utils::demangle

add_library(mpscq mpscq.cpp mpscq.h)
add_executable(mpscq_example mpscq_example.cpp)
target_link_libraries(mpscq_example mpscq)
 
