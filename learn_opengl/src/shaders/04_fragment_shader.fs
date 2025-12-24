#version 330 core
out vec4 fragment_color;
in vec3 our_color;
void main() {
  fragment_color = vec4(our_color, 1.0);
}