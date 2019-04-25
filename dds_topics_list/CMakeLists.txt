cmake_minimum_required(VERSION 3.5)
project(dd_topics_list CXX)

find_package(fastcdr REQUIRED)
include_directories(${fastcdr_INCLUDE_DIR})
set(EXTERNAL_LIBS fastcdr ${EXTERNAL_LIBS})

find_package(fastrtps REQUIRED)
include_directories(${fastrtps_INCLUDE_DIR})
set(EXTERNAL_LIBS fastrtps ${EXTERNAL_LIBS})

add_executable(dd_topics_list main.cpp)
target_link_libraries(dd_topics_list ${EXTERNAL_LIBS})
