cmake_minimum_required(VERSION 3.10)
project(GStreamerGLFWExample)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
    gstreamer-app-1.0
)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${GST_INCLUDE_DIRS})
link_directories(${GST_LIBRARY_DIRS})
add_definitions(${GST_CFLAGS_OTHER})

add_executable(main main.cpp)
target_link_libraries(main ${GST_LIBRARIES} glfw OpenGL::GL GLU GL)
