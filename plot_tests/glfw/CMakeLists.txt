find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(glfw_plot glfw_plot.cpp)
target_link_libraries(glfw_plot PRIVATE glfw OpenGL::GL)

target_compile_options(glfw_plot PRIVATE -Wall -Wextra -Wno-unused-parameter)
