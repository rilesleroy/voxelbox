#version 450 core
out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 camera_pos;

in vec3 normal;
in vec3 frag_pos;

void main()
{
  // ambient
  float ambient_strength = 0.5;
  vec3 ambient = ambient_strength * light_color;

  // diffuse
  vec3 dif_normal = normalize(normal);
  vec3 dif_light_dir = normalize(light_pos - frag_pos);
  float diffuse_factor = max(dot(normal, dif_light_dir), 0.0);
  vec3 diffuse = diffuse_factor * light_color;

  // specular
  float specular_strength = 0.2f;
  vec3 camera_dir = normalize(camera_pos - frag_pos);
  vec3 reflect_dir = reflect(-dif_light_dir, normal);
  float spec = pow(max(dot(reflect_dir, camera_dir), 0.0), 32);
  vec3 specular = specular_strength * spec * light_color; 
  vec3 result = (ambient + diffuse + specular) * object_color;
  FragColor = vec4(result, 1.0);
}