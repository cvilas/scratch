#pragma once

#include <string>

#include <glm/glm.hpp>

class ShaderCompiler {
public:
  ShaderCompiler(const std::string& vertex_shader_source,
                 const std::string& fragment_shader_source);
  void use() const;
  auto id() const -> unsigned int;
  void set(const std::string& name, bool value) const;
  void set(const std::string& name, int value) const;
  void set(const std::string& name, float value) const;
  void set(const std::string& name, const glm::vec2& value) const;
  void set(const std::string& name, const glm::vec3& value) const;
  void set(const std::string& name, const glm::vec4& value) const;
  void set(const std::string& name, const glm::mat2& value) const;
  void set(const std::string& name, const glm::mat3& value) const;
  void set(const std::string& name, const glm::mat4& value) const;

  ~ShaderCompiler();
  ShaderCompiler(const ShaderCompiler&) = delete;
  auto operator=(const ShaderCompiler&) -> ShaderCompiler& = delete;
  ShaderCompiler(ShaderCompiler&&) = delete;
  auto operator=(ShaderCompiler&&) -> ShaderCompiler& = delete;

private:
  auto check(const std::string& name) const -> int;
  unsigned int id_{ 0 };
};