#include "shader_compiler.h"

#include <format>
#include <stdexcept>

#include <glad/gl.h>

ShaderCompiler::ShaderCompiler(const std::string& vertex_shader_source,
                               const std::string& fragment_shader_source)
  : id_(glCreateProgram()) {
  const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  const auto* vertex_shader_ptr = vertex_shader_source.c_str();
  glShaderSource(vertex_shader, 1, &vertex_shader_ptr, nullptr);
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
  const auto* fragment_shader_ptr = fragment_shader_source.c_str();
  glShaderSource(fragment_shader, 1, &fragment_shader_ptr, nullptr);
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
  glAttachShader(id_, vertex_shader);
  glAttachShader(id_, fragment_shader);
  glLinkProgram(id_);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  auto is_shaders_linked = GL_FALSE;
  glGetProgramiv(id_, GL_LINK_STATUS, &is_shaders_linked);
  if (is_shaders_linked != GL_TRUE) {
    static constexpr auto LOG_LENGTH = 1024;
    auto info_log = std::string(LOG_LENGTH, '\0');
    glGetProgramInfoLog(id_, LOG_LENGTH, nullptr, info_log.data());
    throw std::runtime_error(std::format("ERROR: Shader program linking: {}", info_log));
  }
}

ShaderCompiler::~ShaderCompiler() {
  glDeleteProgram(id_);
}

void ShaderCompiler::use() const {
  glUseProgram(id_);
}

auto ShaderCompiler::id() const -> unsigned int {
  return id_;
}

void ShaderCompiler::set(const std::string& name, bool value) const {
  const auto var = glGetUniformLocation(id_, name.c_str());
  if (var == -1) {
    throw std::runtime_error(std::format("Uniform variable '{}' not found", name));
  }
  glUniform1i(var, value ? 1 : 0);
}

void ShaderCompiler::set(const std::string& name, int value) const {
  const auto var = glGetUniformLocation(id_, name.c_str());
  if (var == -1) {
    throw std::runtime_error(std::format("Uniform variable '{}' not found", name));
  }
  glUniform1i(var, value);
}

void ShaderCompiler::set(const std::string& name, float value) const {
  const auto var = glGetUniformLocation(id_, name.c_str());
  if (var == -1) {
    throw std::runtime_error(std::format("Uniform variable '{}' not found", name));
  }
  glUniform1f(var, value);
}