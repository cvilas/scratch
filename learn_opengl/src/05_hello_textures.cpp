/// Implements https://learnopengl.com/Getting-started/Textures
///
#include <array>
#include <cmath>
#include <cstdlib>
#include <print>
#include <stdexcept>
#include <string>

#include <glad/gl.h>
//  include glfw headers after glad headers
#include <GLFW/glfw3.h>

#include "shader_compiler.h"
#include "stb_image.h"

namespace {
void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }

  static auto enable_wireframe = false;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    enable_wireframe = not enable_wireframe;
    if (enable_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }
}

}  // namespace

auto main() -> int {
  try {
    // glfw: initialise and configure for OpenGL v3.3 core profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw: create window
    static constexpr auto WIN_WIDTH = 800;
    static constexpr auto WIN_HEIGHT = 600;
    auto* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Hello Triangle", nullptr, nullptr);
    if (window == nullptr) {
      throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // glad: load openGL function pointers
    const auto version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
      throw std::runtime_error("Failed to initialize OpenGL context");
    }

    // setup vertex data for triangle
    static constexpr auto VERTICES = std::array<float, 24>{
      // positions[3], colors[3], texture_coord[2]
      0.5F,  -0.5F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F,  // bottom right
      -0.5F, -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F,  // bottom left
      0.0F,  0.5F,  0.0F, 0.0F, 0.0F, 1.0F, 0.5F, 1.0F   // top
    };

    // load texture file
    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    auto tex_wh = 0;
    auto tex_ht = 0;
    auto tex_num_channels = 0;
    const auto* const texture_file = "../textures/container.jpg";
    auto* data = stbi_load(texture_file, &tex_wh, &tex_ht, &tex_num_channels, 0);
    if (data != nullptr) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_wh, tex_ht, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      throw std::runtime_error("Failed to load texture");
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES.data(), GL_STATIC_DRAW);

    // position attribute
    const auto stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    // color attribute
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(3 * sizeof(float)));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    glEnableVertexAttribArray(1);

    // texture attribute
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr,cppcoreguidelines-avoid-magic-numbers)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(6 * sizeof(float)));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr,cppcoreguidelines-avoid-magic-numbers)
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    const auto vertex_shader_src = std::string{
#embed "./shaders/05_vertex_shader.vs"
    };

    const auto fragment_shader_src = std::string{
#embed "./shaders/05_fragment_shader.fs"
    };

    auto shader = ShaderCompiler(vertex_shader_src, fragment_shader_src);
    shader.use();

    while (glfwWindowShouldClose(window) == GL_FALSE) {
      processInput(window);

      static constexpr auto BG_COLOR = std::array{ 0.2F, 0.3F, 0.3F, 1.0F };
      glClearColor(BG_COLOR[0], BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);
      glClear(GL_COLOR_BUFFER_BIT);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glBindVertexArray(0);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = fputs(ex.what(), stderr);
    glfwTerminate();
    return EXIT_FAILURE;
  }
}