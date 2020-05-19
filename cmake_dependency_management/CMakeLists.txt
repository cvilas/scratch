cmake_minimum_required(VERSION 3.16)
project(cmake_dep_test LANGUAGES CXX)

macro(find_external_package)
  set(prefix ARG)
  set(noValues REQUIRED)
  set(singleValues PACKAGE VERSION)
  set(multiValues COMPONENTS)

  cmake_parse_arguments(${prefix}
    "${noValues}"
    "${singleValues}"
    "${multiValues}"
    ${ARGN})

    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Keywords missing values: ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()

    if(${ARG_REQUIRED})
        set(is_required REQUIRED)
    endif()

    if(ARG_COMPONENTS)
        find_package(${ARG_PACKAGE} ${ARG_VERSION} COMPONENTS ${ARG_COMPONENTS} ${is_required})
    else()
        find_package(${ARG_PACKAGE} ${ARG_VERSION} ${is_required})
    endif()
    message(STATUS "Added external dependency: ${ARG_PACKAGE} ${ARG_VERSION} ${ARG_COMPONENTS}")

endmacro()

find_external_package(
  PACKAGE Eigen3
  VERSION 3.3.7
  REQUIRED)

find_external_package(
  PACKAGE Qt5
  VERSION 5.9
  COMPONENTS Widgets OpenGL PrintSupport
  REQUIRED)

add_executable(cmake_dep_test main.cpp)
target_link_libraries(cmake_dep_test Eigen3::Eigen)
