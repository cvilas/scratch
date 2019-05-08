cmake_minimum_required(VERSION 2.8)
project(rest_serve CXX)

find_package(Pistache REQUIRED)
include_directories(${pistache_INCLUDE_DIR})
set(EXTERNAL_LIBS pistache ${EXTERNAL_LIBS})

add_executable(server server.cpp)
target_link_libraries(server ${EXTERNAL_LIBS})

add_executable(client client.cpp)
target_link_libraries(client ${EXTERNAL_LIBS} -lboost_system -lpthread)
