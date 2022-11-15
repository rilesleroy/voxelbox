#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform mat4 model;
uniform mat4 view_proj;

out vec3 normal;
out vec3 frag_pos;

void main() {
  frag_pos = vec3(model * vec4(a_pos, 1.0));
  normal = a_normal;
  gl_Position = view_proj * model * vec4(a_pos, 1.0);
}