/// https://learnopengl.com/Lighting/Colors
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

    constexpr auto OBJECT_VERTEX_SHADER_SRC = R"(
    #version 330 core
    layout (location = 0) in vec3 in_position;
    layout (location = 1) in vec3 in_normal;

    out vec3 fragment_position;
    out vec3 normal;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
      fragment_position = vec3(model * vec4(in_position, 1.0));
      normal = mat3(transpose(inverse(model))) * in_normal;  

      gl_Position = projection * view * vec4(fragment_position, 1.0);
    }
    )";

    constexpr auto OBJECT_FRAGMENT_SHADER_SRC = R"(
    #version 330 core
    out vec4 fragment_color;

    struct Material {
      vec3 ambient;
      vec3 diffuse;
      vec3 specular;
      float shininess;
    };

    struct Light {
      vec3 position;

      vec3 ambient;
      vec3 diffuse;
      vec3 specular;
    };

    in vec3 normal;  
    in vec3 fragment_position;  

    uniform vec3 view_position; 
    uniform Material material;
    uniform Light light;
    
    void main() {
      // ambient
      vec3 ambient = light.ambient * material.ambient;
  	
      // diffuse 
      vec3 norm = normalize(normal);
      vec3 light_dir = normalize(light.position - fragment_position);
      float diff = max(dot(norm, light_dir), 0.0);
      vec3 diffuse = light.diffuse * (diff * material.diffuse);

      // specular
      vec3 view_dir = normalize(view_position - fragment_position);
      vec3 reflect_dir = reflect(-light_dir, norm);
      float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
      vec3 specular = light.specular * spec * material.specular;  

      vec3 result = ambient + diffuse + specular;
      fragment_color = vec4(result, 1.0);
    } 
    )";

    constexpr auto LIGHT_CUBE_VERTEX_SHADER_SRC = R"(
    #version 330 core
    layout (location = 0) in vec3 in_position;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
      gl_Position = projection * view * model * vec4(in_position, 1.0);
    }
    )";

    constexpr auto LIGHT_CUBE_FRAGMENT_SHADER_SRC = R"(
    #version 330 core
    out vec4 fragment_color;
    
    void main() {
      fragment_color = vec4(1.0); 
    }
    )";

    auto object_shader = ShaderCompiler(OBJECT_VERTEX_SHADER_SRC, OBJECT_FRAGMENT_SHADER_SRC);
    auto light_cube_shader =
        ShaderCompiler(LIGHT_CUBE_VERTEX_SHADER_SRC, LIGHT_CUBE_FRAGMENT_SHADER_SRC);

    // set up vertex data for cube
    // position[3], normal[3]
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,hicpp-uppercase-literal-suffix)
    static constexpr auto VERTICES = std::array<float, 216>{
      //
      -0.5F, -0.5F, -0.5F, 0.0F, 0.0F, -1.0F,  //
      0.5F, -0.5F, -0.5F, 0.0F, 0.0F, -1.0F,   //
      0.5F, 0.5F, -0.5F, 0.0F, 0.0F, -1.0F,    //
      0.5F, 0.5F, -0.5F, 0.0F, 0.0F, -1.0F,    //
      -0.5F, 0.5F, -0.5F, 0.0F, 0.0F, -1.0F,   //
      -0.5F, -0.5F, -0.5F, 0.0F, 0.0F, -1.0F,  //
                                               //
      -0.5F, -0.5F, 0.5F, 0.0F, 0.0F, 1.0F,    //
      0.5F, -0.5F, 0.5F, 0.0F, 0.0F, 1.0F,     //
      0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 1.0F,      //
      0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 1.0F,      //
      -0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 1.0F,     //
      -0.5F, -0.5F, 0.5F, 0.0F, 0.0F, 1.0F,    //
                                               //
      -0.5F, 0.5F, 0.5F, -1.0F, 0.0F, 0.0F,    //
      -0.5F, 0.5F, -0.5F, -1.0F, 0.0F, 0.0F,   //
      -0.5F, -0.5F, -0.5F, -1.0F, 0.0F, 0.0F,  //
      -0.5F, -0.5F, -0.5F, -1.0F, 0.0F, 0.0F,  //
      -0.5F, -0.5F, 0.5F, -1.0F, 0.0F, 0.0F,   //
      -0.5F, 0.5F, 0.5F, -1.0F, 0.0F, 0.0F,    //
                                               //
      0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 0.0F,      //
      0.5F, 0.5F, -0.5F, 1.0F, 0.0F, 0.0F,     //
      0.5F, -0.5F, -0.5F, 1.0F, 0.0F, 0.0F,    //
      0.5F, -0.5F, -0.5F, 1.0F, 0.0F, 0.0F,    //
      0.5F, -0.5F, 0.5F, 1.0F, 0.0F, 0.0F,     //
      0.5F, 0.5F, 0.5F, 1.0F, 0.0F, 0.0F,      //
                                               //
      -0.5F, -0.5F, -0.5F, 0.0F, -1.0F, 0.0F,  //
      0.5F, -0.5F, -0.5F, 0.0F, -1.0F, 0.0F,   //
      0.5F, -0.5F, 0.5F, 0.0F, -1.0F, 0.0F,    //
      0.5F, -0.5F, 0.5F, 0.0F, -1.0F, 0.0F,    //
      -0.5F, -0.5F, 0.5F, 0.0F, -1.0F, 0.0F,   //
      -0.5F, -0.5F, -0.5F, 0.0F, -1.0F, 0.0F,  //
                                               //
      -0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F,    //
      0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F,     //
      0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,      //
      0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,      //
      -0.5F, 0.5F, 0.5F, 0.0F, 1.0F, 0.0F,     //
      -0.5F, 0.5F, -0.5F, 0.0F, 1.0F, 0.0F     //
    };
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,hicpp-uppercase-literal-suffix)

    glEnable(GL_DEPTH_TEST);

    unsigned int object_vao = 0;
    glGenVertexArrays(1, &object_vao);

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES.data(), GL_STATIC_DRAW);

    const auto stride = 6 * sizeof(float);
    glBindVertexArray(object_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);  // position attribute
    glEnableVertexAttribArray(0);
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(3 * sizeof(float)));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)

    glEnableVertexAttribArray(1);

    unsigned int light_cube_vao = 0;
    glGenVertexArrays(1, &light_cube_vao);
    glBindVertexArray(light_cube_vao);
    // reuse the same vbo - it's already bound
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

      static constexpr auto LIGHT_CUBE_POSITION = glm::vec3(1.2F, 1.0F, 2.0F);
      static constexpr auto LIGHT_CUBE_SCALE = glm::vec3(0.2F);

      object_shader.use();
      object_shader.set("light.position", LIGHT_CUBE_POSITION);
      object_shader.set("view_position", g_camera.position);

      // light properties
      const auto light_color = glm::vec3(static_cast<float>(sin(glfwGetTime() * 2.0)),
                                         static_cast<float>(sin(glfwGetTime() * 0.7)),
                                         static_cast<float>(sin(glfwGetTime() * 1.3)));
      const auto light_diffuse_color = light_color * glm::vec3(0.5F);
      const auto light_ambient_colot = light_diffuse_color * glm::vec3(0.2F);
      static constexpr auto LIGHT_SPECULAR_COLOR = glm::vec3(1.0F, 1.0F, 1.0F);
      object_shader.set("light.ambient", light_ambient_colot);
      object_shader.set("light.diffuse", light_diffuse_color);
      object_shader.set("light.specular", LIGHT_SPECULAR_COLOR);

      // material properties
      static constexpr auto OBJECT_COLOR = glm::vec3(1.0F, 0.5F, 0.31F);
      static constexpr auto OBJECT_SPECULAR_COLOR = glm::vec3(0.5F, 0.5F, 0.5F);
      static constexpr auto OBJECT_SHININESS = 32.0F;
      object_shader.set("material.ambient", OBJECT_COLOR);
      object_shader.set("material.diffuse", OBJECT_COLOR);
      object_shader.set("material.specular", OBJECT_SPECULAR_COLOR);
      object_shader.set("material.shininess", OBJECT_SHININESS);

      int win_width{ 0 };
      int win_height{ 0 };
      glfwGetWindowSize(window, &win_width, &win_height);

      // view/projection transformations
      const auto win_aspect = static_cast<float>(win_width) / static_cast<float>(win_height);
      static constexpr auto NEAR = 0.1F;
      static constexpr auto FAR = 100.0F;
      const auto projection = glm::perspective(glm::radians(g_camera.fov), win_aspect, NEAR, FAR);
      const auto view =
          glm::lookAt(g_camera.position, g_camera.position + g_camera.front, g_camera.up);
      object_shader.set("projection", projection);
      object_shader.set("view", view);

      // world transformation
      auto model = glm::mat4(1.0F);
      object_shader.set("model", model);

      // render the object
      glBindVertexArray(object_vao);
      glDrawArrays(GL_TRIANGLES, 0, 36);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      glBindVertexArray(0);

      // also draw the lamp cube
      light_cube_shader.use();
      light_cube_shader.set("projection", projection);
      light_cube_shader.set("view", view);
      model = glm::mat4(1.0F);
      model = glm::translate(model, LIGHT_CUBE_POSITION);
      model = glm::scale(model, LIGHT_CUBE_SCALE);
      light_cube_shader.set("model", model);

      glBindVertexArray(light_cube_vao);
      glDrawArrays(GL_TRIANGLES, 0, 36);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
      glBindVertexArray(0);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteVertexArrays(1, &object_vao);
    glDeleteVertexArrays(1, &light_cube_vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = fputs(ex.what(), stderr);
    glfwTerminate();
    return EXIT_FAILURE;
  }
}
