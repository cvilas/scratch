cmake_minimum_required(VERSION 3.6)
project(libcamera-test DESCRIPTION "Example application for libcamera" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)
message(STATUS "libcamera: Using version: ${LIBCAMERA_VERSION} (${LIBCAMERA_LINK_LIBRARIES})")

add_executable(hello hello.cpp)
target_link_libraries(hello PRIVATE PkgConfig::LIBCAMERA)
