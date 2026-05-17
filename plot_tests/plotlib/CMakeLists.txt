cmake_minimum_required(VERSION 3.20)

# Allow building standalone or as a subdirectory
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(plotlib CXX)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

find_package(SDL3     REQUIRED)
find_package(SDL3_ttf REQUIRED)

# ── Library ──────────────────────────────────────────────────────────────────
add_library(plotlib STATIC
    src/PlotWidget.cpp
    src/Trace.cpp
    src/embedded_font.cpp   # C++23 #embed bundles the OTF into the binary
)

target_include_directories(plotlib
    PUBLIC  include
)

target_link_libraries(plotlib
    PUBLIC SDL3::SDL3 SDL3_ttf::SDL3_ttf
)

target_compile_features(plotlib PUBLIC cxx_std_23)
target_compile_options(plotlib  PRIVATE -Wall -Wextra -Wno-unused-parameter)

# ── Example executable ────────────────────────────────────────────────────────
add_executable(plotlib_example example/main.cpp)
target_link_libraries(plotlib_example PRIVATE plotlib)
