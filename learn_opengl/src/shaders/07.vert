#version 330 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texture_coord;

out vec2 texture_coord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(in_position, 1.0F);
    texture_coord = in_texture_coord;
}