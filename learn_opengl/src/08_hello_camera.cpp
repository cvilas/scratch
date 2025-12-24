/// https://learnopengl.com/Getting-started/Camera
///
#include <array>
#include <cmath>
#include <cstdlib>
#include <print>
#include <stdexcept>
#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//  include glfw headers after glad headers
#include <GLFW/glfw3.h>

#include "shader_compiler.h"
#include "stb_image.h"

namespace {

struct Camera {
  static constexpr auto INIT_POSITION = glm::vec3(0.0F, 0.0F, 3.0F);
  static constexpr auto INIT_FRONT = glm::vec3(0.0F, 0.0F, -1.0F);
  static constexpr auto INIT_UP = glm::vec3(0.0F, 1.0F, 0.0F);
  static constexpr auto MAX_PITCH = 89.0F;
  static constexpr auto MAX_FOV = 45.F;
  static constexpr auto MIN_FOV = 1.F;

  glm::vec3 position{ INIT_POSITION };
  glm::vec3 front{ INIT_FRONT };
  glm::vec3 up{ INIT_UP };
  float fov{ MAX_FOV };

  void reset() {
    position = INIT_POSITION;
    front = INIT_FRONT;
    up = INIT_UP;
    fov = MAX_FOV;
  }
};

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
auto g_camera = Camera{};
auto first_mouse = true;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, float delta_time) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }

  const auto camera_speed = static_cast<float>(2.5 * delta_time);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    g_camera.position += camera_speed * g_camera.front;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    g_camera.position -= camera_speed * g_camera.front;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    g_camera.position -= glm::normalize(glm::cross(g_camera.front, g_camera.up)) * camera_speed;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    g_camera.position += glm::normalize(glm::cross(g_camera.front, g_camera.up)) * camera_speed;
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    g_camera.reset();
    first_mouse = true;
  }
}

void mouseCallback(GLFWwindow* window, double x_pos_in, double y_pos_in) {
  (void)window;

  static auto last_x = 0.0F;
  static auto last_y = 0.0F;
  static auto yaw = -90.0F;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  static auto pitch = 0.0F;

  const auto x_pos = static_cast<float>(x_pos_in);
  const auto y_pos = static_cast<float>(y_pos_in);

  if (first_mouse) {
    last_x = x_pos;
    last_y = y_pos;
    first_mouse = false;
  }

  const auto sensitivity = 0.1F;
  auto x_offset = (x_pos - last_x) * sensitivity;
  auto y_offset = (last_y - y_pos) * sensitivity;
  last_x = x_pos;
  last_y = y_pos;

  yaw += x_offset;
  pitch += y_offset;
  pitch = std::min(Camera::MAX_PITCH, std::max(pitch, -Camera::MAX_PITCH));

  g_camera.front =
      glm::normalize(glm::vec3{ std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),  //
                                std::sin(glm::radians(pitch)),                                //
                                std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)) });
}

void scrollCallback(GLFWwindow* window, double x_offset, double y_offset) {
  (void)window;
  (void)x_offset;
  g_camera.fov -= static_cast<float>(y_offset);
  g_camera.fov = std::min(Camera::MAX_FOV, std::max(Camera::MIN_FOV, g_camera.fov));
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
    // glfw: create window
    static constexpr auto WIN_WIDTH = 800;
    static constexpr auto WIN_HEIGHT = 600;
    auto* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Hello Triangle", nullptr, nullptr);
    if (window == nullptr) {
      throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // glad: load openGL function pointers
    const auto version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
      throw std::runtime_error("Failed to initialize OpenGL context");
    }

    // set up vertex data for cube
    // position[3], tex_coord[2]
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,hicpp-uppercase-literal-suffix)
    static constexpr auto VERTICES = std::array<float, 180>{                                   //
                                                             -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  //
                                                             0.5f, -0.5f, -0.5f, 1.0f, 0.0f,   //
                                                             0.5f, 0.5f, -0.5f, 1.0f, 1.0f,    //
                                                             0.5f, 0.5f, -0.5f, 1.0f, 1.0f,    //
                                                             -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,   //
                                                             -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  //
                                                                                               //
                                                             -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   //
                                                             0.5f, -0.5f, 0.5f, 1.0f, 0.0f,    //
                                                             0.5f, 0.5f, 0.5f, 1.0f, 1.0f,     //
                                                             0.5f, 0.5f, 0.5f, 1.0f, 1.0f,     //
                                                             -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,    //
                                                             -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   //
                                                                                               //
                                                             -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    //
                                                             -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   //
                                                             -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  //
                                                             -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  //
                                                             -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   //
                                                             -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    //
                                                                                               //
                                                             0.5f, 0.5f, 0.5f, 1.0f, 0.0f,     //
                                                             0.5f, 0.5f, -0.5f, 1.0f, 1.0f,    //
                                                             0.5f, -0.5f, -0.5f, 0.0f, 1.0f,   //
                                                             0.5f, -0.5f, -0.5f, 0.0f, 1.0f,   //
                                                             0.5f, -0.5f, 0.5f, 0.0f, 0.0f,    //
                                                             0.5f, 0.5f, 0.5f, 1.0f, 0.0f,     //
                                                                                               //
                                                             -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  //
                                                             0.5f, -0.5f, -0.5f, 1.0f, 1.0f,   //
                                                             0.5f, -0.5f, 0.5f, 1.0f, 0.0f,    //
                                                             0.5f, -0.5f, 0.5f, 1.0f, 0.0f,    //
                                                             -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   //
                                                             -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  //
                                                                                               //
                                                             -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,   //
                                                             0.5f, 0.5f, -0.5f, 1.0f, 1.0f,    //
                                                             0.5f, 0.5f, 0.5f, 1.0f, 0.0f,     //
                                                             0.5f, 0.5f, 0.5f, 1.0f, 0.0f,     //
                                                             -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,    //
                                                             -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,hicpp-uppercase-literal-suffix)

    glEnable(GL_DEPTH_TEST);

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
    const auto stride = 5 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    // texture attribute
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(3 * sizeof(float)));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    const auto vertex_shader_src = std::string{
#embed "./shaders/07.vert"
    };

    const auto fragment_shader_src = std::string{
#embed "./shaders/07.frag"
    };

    auto shader = ShaderCompiler(vertex_shader_src, fragment_shader_src);
    shader.use();

    auto last_frame_time = -1.F;
    while (glfwWindowShouldClose(window) == GL_FALSE) {
      const auto current_time = static_cast<float>(glfwGetTime());
      if (last_frame_time < 0) {
        last_frame_time = current_time;
      }
      const auto delta_time = current_time - last_frame_time;
      last_frame_time = current_time;

      processInput(window, delta_time);

      static constexpr auto BG_COLOR = std::array{ 0.2F, 0.3F, 0.3F, 1.0F };
      glClearColor(BG_COLOR[0], BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // NOLINT(hicpp-signed-bitwise)

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      int win_width{ 0 };
      int win_height{ 0 };
      glfwGetWindowSize(window, &win_width, &win_height);
      const auto win_aspect = static_cast<float>(win_width) / static_cast<float>(win_height);

      // create transformations
      static constexpr auto ROT_AXIS = glm::vec3(0.5F, 1.0F, 0.0F);
      auto model = glm::mat4(1.0F);
      model = glm::rotate(model, current_time, ROT_AXIS);
      auto model_location = glGetUniformLocation(shader.id(), "model");
      glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));

      static constexpr auto NEAR = 0.1F;
      static constexpr auto FAR = 100.0F;
      auto projection = glm::mat4(1.0F);
      projection = glm::perspective(glm::radians(g_camera.fov), win_aspect, NEAR, FAR);
      auto projection_location = glGetUniformLocation(shader.id(), "projection");
      glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

      auto view = glm::lookAt(g_camera.position, g_camera.position + g_camera.front, g_camera.up);
      auto view_location = glGetUniformLocation(shader.id(), "view");
      glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));

      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 36);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
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
