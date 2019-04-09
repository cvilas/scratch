cmake_minimum_required(VERSION 2.8)
project(serdes_eigen CXX)

find_package(fastcdr REQUIRED)
include_directories(${fastcdr_INCLUDE_DIR})
set(EXTERNAL_LIBS fastcdr ${EXTERNAL_LIBS})

find_package(Eigen3 REQUIRED NO_MODULE)
include_directories(${EIGEN3_INCLUDE_DIR})
set(EXTERNAL_LIBS Eigen3::Eigen ${EXTERNAL_LIBS})


add_executable(serdes_eigen main.cpp)
target_link_libraries(serdes_eigen ${EXTERNAL_LIBS})
