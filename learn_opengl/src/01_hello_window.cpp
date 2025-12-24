#include <array>
#include <cstdlib>
#include <print>

#include <GLFW/glfw3.h>

namespace {
void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
}
}  // namespace

auto main() -> int {
  try {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    static constexpr auto WINDOW_WIDTH = 800;
    static constexpr auto WINDOW_HEIGHT = 600;
    auto* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
      std::println("Failed to create GLFW window");
      glfwTerminate();
      return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    while (glfwWindowShouldClose(window) == 0) {
      processInput(window);

      static constexpr auto CLEAR_COLOR = std::array{ 0.2F, 0.3F, 0.3F, 1.0F };
      glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);
      glClear(GL_COLOR_BUFFER_BIT);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }
    glfwTerminate();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}