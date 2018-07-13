cmake_minimum_required(VERSION 3.5)
project(has_method)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(has_method has_method.cpp)
#target_link_libraries(has_method fastrtps fastcdr tinyxml2)


