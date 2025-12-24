/// Implements https://learnopengl.com/Getting-started/Shaders
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

constexpr auto VERTEX_SHADER_SRC = R"(
#version 330 core
layout (location = 0) in vec3 vertex_position;
void main() {
  gl_Position = vec4(vertex_position, 1.0);
}
)";

constexpr auto FRAGMENT_SHADER_SRC = R"(
#version 330 core
out vec4 fragment_color;
uniform vec4 our_color;
void main() {
  fragment_color = our_color;
} 
)";

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

    // compile vertex shader
    const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &VERTEX_SHADER_SRC, nullptr);
    glCompileShader(vertex_shader);

    auto is_vertex_shader_compiled = GL_FALSE;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_vertex_shader_compiled);
    if (is_vertex_shader_compiled != GL_TRUE) {
      static constexpr auto LOG_LENGTH = 1024;
      auto info_log = std::string(LOG_LENGTH, '\0');
      glGetShaderInfoLog(vertex_shader, LOG_LENGTH, nullptr, info_log.data());
      throw std::runtime_error(std::format("ERROR: Vertex shader compilation: {}", info_log));
    }

    // compile fragment shader
    const auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &FRAGMENT_SHADER_SRC, nullptr);
    glCompileShader(fragment_shader);

    auto is_fragment_shader_compiled = GL_FALSE;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_fragment_shader_compiled);
    if (is_fragment_shader_compiled != GL_TRUE) {
      static constexpr auto LOG_LENGTH = 1024;
      auto info_log = std::string(LOG_LENGTH, '\0');
      glGetShaderInfoLog(fragment_shader, LOG_LENGTH, nullptr, info_log.data());
      throw std::runtime_error(std::format("ERROR: Fragment shader compilation: {}", info_log));
    }

    // link shaders
    const auto shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    auto is_shaders_linked = GL_FALSE;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &is_shaders_linked);
    if (is_shaders_linked != GL_TRUE) {
      static constexpr auto LOG_LENGTH = 1024;
      auto info_log = std::string(LOG_LENGTH, '\0');
      glGetProgramInfoLog(shader_program, LOG_LENGTH, nullptr, info_log.data());
      throw std::runtime_error(std::format("ERROR: Shader program linking: {}", info_log));
    }

    // setup vertex data for triangle
    static constexpr auto VERTICES = std::array<float, 12>{
      //
      0.5F,  0.5F,  0.0F,  // top right
      0.5F,  -0.5F, 0.0F,  // bottom right
      -0.5F, -0.5F, 0.0F,  // bottom left
      -0.5F, 0.5F,  0.0F   // top left
    };
    static constexpr auto INDICES = std::array<unsigned int, 6>{
      0, 1, 3,  // first Triangle
      1, 2, 3   // second Triangle
    };

    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);  // vertex array object

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);  // vertex buffer object

    unsigned int ebo = 0;
    glGenBuffers(1, &ebo);  // element buffer object

    // - bind vertex array object,
    // - bind and set vertices array in vertex buffer
    // - bind and set indices array in element buffer
    // - configure vertex attribute pointers
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    const auto vertex_color_location = glGetUniformLocation(shader_program, "our_color");
    if (vertex_color_location == -1) {
      throw std::runtime_error("Uniform variable 'our_color' not found");
    }

    while (glfwWindowShouldClose(window) == GL_FALSE) {
      processInput(window);

      static constexpr auto BG_COLOR = std::array{ 0.2F, 0.3F, 0.3F, 1.0F };
      glClearColor(BG_COLOR[0], BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);
      glClear(GL_COLOR_BUFFER_BIT);

      const auto time_value = static_cast<float>(glfwGetTime());
      const auto greenness = (std::sinf(time_value) / 2.0F) + 0.5F;

      glUseProgram(shader_program);
      glUniform4f(vertex_color_location, 0.0F, greenness, 0.0F, 1.0F);

      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES, INDICES.size(), GL_UNSIGNED_INT, nullptr);
      glBindVertexArray(0);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }
    glfwTerminate();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = fputs(ex.what(), stderr);
    glfwTerminate();
    return EXIT_FAILURE;
  }
}