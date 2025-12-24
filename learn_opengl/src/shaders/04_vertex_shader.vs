#version 330 core
layout (location = 0) in vec3 position;  // the position variable has attribute position 0
layout (location = 1) in vec3 color;     // the color variable has attribute position 1
  
out vec3 our_color; // output a color to the fragment shader

void main() {
    gl_Position = vec4(position, 1.0);
    our_color = color; // set color to the input color we got from the vertex data
}   