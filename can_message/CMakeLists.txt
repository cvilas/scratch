cmake_minimum_required(VERSION 3.5)
project(can_message)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../../iota/third_party/gsl/include/)
add_executable(can_message can_message.cpp)
#target_link_libraries(has_method fastrtps fastcdr tinyxml2)
