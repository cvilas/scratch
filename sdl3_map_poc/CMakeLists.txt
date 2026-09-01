cmake_minimum_required(VERSION 3.20)
project(sdl3_map_poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# SDL3
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.2.18
)
FetchContent_MakeAvailable(SDL3)

# cpp-httplib (HTTP client)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.17.3
)
FetchContent_MakeAvailable(httplib)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(sdl3_map_poc main.cpp)
target_compile_definitions(sdl3_map_poc PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(sdl3_map_poc PRIVATE ${httplib_SOURCE_DIR})
target_link_libraries(sdl3_map_poc PRIVATE SDL3::SDL3 OpenSSL::SSL OpenSSL::Crypto Threads::Threads)


add_executable(sdl3_map_poc_v2 main_v2.cpp)
target_compile_definitions(sdl3_map_poc_v2 PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(sdl3_map_poc_v2 PRIVATE ${httplib_SOURCE_DIR})
target_link_libraries(sdl3_map_poc_v2 PRIVATE SDL3::SDL3 Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
