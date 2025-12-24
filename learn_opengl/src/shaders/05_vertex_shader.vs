#version 330 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_texture_coord;

out vec3 our_color;
out vec2 texture_coord;

void main() {
    gl_Position = vec4(in_position, 1.0);
    our_color = in_color;
    texture_coord = in_texture_coord;
}