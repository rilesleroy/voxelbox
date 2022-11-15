#version 330 core
uniform vec3 object_color;
uniform vec3 light_cast;
uniform vec3 light_color;
in vec3 normal;
in vec3 frag_pos;
out vec4 FragColor;
void main()
{
  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(light_cast);

  float ambient_strength = 0.1;
  vec3 ambient = ambient_strength * light_color;

  // angle between lightdir and fragment normal ditermines
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = diff * light_color;

  vec3 result = (diffuse + ambient) * object_color;
  FragColor = vec4(result, 1.0);
}