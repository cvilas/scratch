cmake_minimum_required(VERSION 3.10)
project("rtps_keys_exp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
#find_package(fastcdr REQUIRED)
#find_package(fastrtps REQUIRED)

add_subdirectory(third_party)

message(STATUS "Configuring ${CMAKE_PROJECT_NAME}...")
file(GLOB ${CMAKE_PROJECT_NAME}_sources "*.cpp")
file(GLOB ${CMAKE_PROJECT_NAME}_headers "*.h")

add_executable(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}_sources} ${${CMAKE_PROJECT_NAME}_headers})
target_link_libraries(${CMAKE_PROJECT_NAME} fastrtps)
add_dependencies(${CMAKE_PROJECT_NAME} fastrtps)
