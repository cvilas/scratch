#pragma once

#include <string>

class ShaderCompiler {
public:
  ShaderCompiler(const std::string& vertex_shader_source,
                 const std::string& fragment_shader_source);
  void use() const;
  auto id() const -> unsigned int;
  void set(const std::string& name, bool value) const;
  void set(const std::string& name, int value) const;
  void set(const std::string& name, float value) const;

  ~ShaderCompiler();
  ShaderCompiler(const ShaderCompiler&) = delete;
  auto operator=(const ShaderCompiler&) -> ShaderCompiler& = delete;
  ShaderCompiler(ShaderCompiler&&) = delete;
  auto operator=(ShaderCompiler&&) -> ShaderCompiler& = delete;

private:
  unsigned int id_{ 0 };
};