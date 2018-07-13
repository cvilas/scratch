cmake_minimum_required(VERSION 2.8.12)
project("rtps_rediscovery_test")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)


message(STATUS "Configuring ${CMAKE_PROJECT_NAME}...")
file(GLOB ${CMAKE_PROJECT_NAME}_sources "*.cpp")
file(GLOB ${CMAKE_PROJECT_NAME}_headers "*.h")

add_executable(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}_sources} ${${CMAKE_PROJECT_NAME}_headers})
target_link_libraries(${CMAKE_PROJECT_NAME} fastrtps fastcdr)

