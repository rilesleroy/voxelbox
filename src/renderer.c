/* date = May 19th 2021 10:37 pm */
typedef uint32_t RShaderProgram;
typedef struct{
  gbVec3 up;
  gbVec3 pos;
  gbVec3 front;
  float yaw;	
  float pitch;
  float fov;
  float look_sensitivity;
  float near_plane;
  float far_plane;
  float speed;
  gbMat4 proj_view;
} RCamera;

typedef union {
  struct {
    float r;
    float g;
    float b;
    float a;
  };
  float e[4];
} RColorRGBA;

typedef union {
  gbVec3 dir;
  RColorRGBA color;
} RDirLight;

typedef struct 
{
  gbVec3 position;
  gbVec3 normal;
} Vertex;

typedef struct {
  gbVec3 position;
  Vertex* vertex_buffer;
} Chunk;

typedef struct {
  int win_width;
  int win_height;
  uint32_t vao;
  uint32_t vbo;
  uint32_t cube_shader;
  Vertex* vertex_buffer;
} RRenderState;

#define VOXEL_PER_CHUNK 8

///////////////////////////////////
// Viewport and Windowing
static void r_resize_window_viewport(RRenderState* render_state, GLFWwindow* window)
{
  glfwGetWindowSize(window, &(render_state->win_width), &(render_state->win_height));
  glViewport(0, 0, render_state->win_width, render_state->win_height);
}

static void r_init_render_state(RRenderState* render_state, GLFWwindow* window)
{
  // more stuff will go here
  render_state->vao = 0;
  render_state->vbo = 0;
  r_resize_window_viewport(render_state, window);
  render_state->vertex_buffer = NULL;
}

///////////////////////////////////
// SHADERS
static RShaderProgram r_compile_shader_program(char* vertex_source, char* fragment_source)
{
  // getShader Source From file
  int32_t success;
  RShaderProgram vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  RShaderProgram fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  
  char info_log[512]; // possible overflow but who cares
  
  // vert
  // reset comp error
  success = GL_FALSE;
  glShaderSource(vertex_shader, 1, &vertex_source, NULL);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
    printf("SHADER HANDLE: %d\nERROR:%s\naborting..\n", vertex_shader, info_log);
    abort();
  } else {
    printf("SHADER HANDLE: %d\nSTATUS: COMPILED!\n", vertex_shader);
  }
  
  // frag
  // reset comp error
  success = GL_FALSE;
  glShaderSource(fragment_shader, 1, &fragment_source, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    glGetShaderInfoLog(fragment_shader, 1024, NULL, info_log);
    printf("SHADER HANDLE: %d\nERROR: \n%s\naborting\n", fragment_shader, info_log);
    abort();
  } else {
    printf("SHADER HANDLE: %d\nSTATUS: COMPILED!\n", fragment_shader);
  }
  
  // ShaderProgram the "linking Stage" of graphics pipline
  RShaderProgram shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  
  // Delete Shaders
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  return shader_program;
}

//static void r_set_active_shader(RShaderProgram shader)
//{
//glUseProgram(shader);
//}

static void r_set_uniform_bool(RShaderProgram shader, char* name, bool value) 
{
  glUniform1i(glGetUniformLocation(shader, name), (int32_t)value);
}

static void r_set_uniform_i32(RShaderProgram shader, char* name, int32_t value)
{
  glUniform1i(glGetUniformLocation(shader, name), value);
}

static void r_set_uniform_f32(RShaderProgram shader, char* name, float value)
{
  glUniform1f(glGetUniformLocation(shader, name), value);
}

static void r_set_uniform_rbg(RShaderProgram shader, char* name, RColorRGBA color)
{
  glUniform3f(glGetUniformLocation(shader, name), color.r, color.g, color.b);
}

static void r_set_uniform_v3(RShaderProgram shader, char* name, gbVec3 vector)
{
  glUniform3f(glGetUniformLocation(shader, name), vector.x, vector.y, vector.z);
}

static void r_set_uniform_mat4(RShaderProgram shader, char* name, gbMat4 value)
{
  glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, false, &(value.e[0]));
}
// END SHADERS
///////////////////////////////////

///////////////////////////////////
// CAMERA
static RCamera r_update_camera_projection(RRenderState render_state, RCamera base_camera)
{
  RCamera r_camera = base_camera;
  gb_mat4_identity(&(r_camera.proj_view));
  
  gbVec3 eye;
  gb_vec3_add(&eye, r_camera.pos, r_camera.front);
  
  gbMat4 view;
  gb_mat4_look_at(&view, r_camera.pos, eye, r_camera.up);
  
  gbMat4 proj;
  gb_mat4_perspective(&proj,
                      gb_to_radians(r_camera.fov),
                      (float)render_state.win_width/(float)render_state.win_height,
                      r_camera.near_plane,
                      r_camera.far_plane);
  
  gb_mat4_mul(&(r_camera.proj_view), &proj, &view);
  
  return r_camera;
}


static RCamera r_make_camera(RRenderState render_state)
{
  RCamera base_camera = 
  {
    .up = {0.0f, 1.0f, 0.0f},
    .pos = {0.0f, 0.0f, 10.0f},
    .front = {0.0f, 0.0f, -1.0f},
    .yaw   = -90.0f,
    .pitch = 0.0f,
    .fov   = 90.0f,
    .speed = 5.0f,
    .look_sensitivity = 0.1f,
    .near_plane = 0.1f,
    .far_plane = 100.0f
  };
  
  return r_update_camera_projection(render_state, base_camera);
}


static int SEED = 0;

static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
  185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
  9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
  70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
  203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
  164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
  228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
  232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
  193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
  101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
  135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
  114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int noise2(int x, int y)
{
  int tmp = hash[(y + SEED) % 256];
  return hash[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s)
{
  return x + s * (y-x);
}

float smooth_inter(float x, float y, float s)
{
  return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y)
{
  int x_int = x;
  int y_int = y;
  float x_frac = x - x_int;
  float y_frac = y - y_int;
  int s = noise2(x_int, y_int);
  int t = noise2(x_int+1, y_int);
  int u = noise2(x_int, y_int+1);
  int v = noise2(x_int+1, y_int+1);
  float low = smooth_inter(s, t, x_frac);
  float high = smooth_inter(u, v, x_frac);
  return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, int depth)
{
  float xa = x*freq;
  float ya = y*freq;
  float amp = 1.0;
  float fin = 0;
  float div = 0.0;
  
  int i;
  for(i=0; i<depth; i++)
  {
    div += 256 * amp;
    fin += noise2d(xa, ya) * amp;
    amp /= 2;
    xa *= 2;
    ya *= 2;
  }
  
  return fin/div;
}


static bool has_voxel(int x, int y, int z)
{
  float base_perlin = perlin2d(x, z, 0.1, VOXEL_PER_CHUNK);
  float height = base_perlin * VOXEL_PER_CHUNK;
  return (y < height);
}


static void r_mesh_chunk(RRenderState* render_state, uint32_t chunk_x, uint32_t chunk_z)
{
  for (uint32_t x=0; x < VOXEL_PER_CHUNK; x++)
  {
    for (uint32_t y=0; y < VOXEL_PER_CHUNK; y++)
    {
      for (uint32_t z=0; z < VOXEL_PER_CHUNK; z++)
      {
        if (has_voxel(x, y, z))
        {
          // top +y 
          if (!has_voxel(x+chunk_x, y+1, z+chunk_z))
          {
            // add top to vertex buffer
            Vertex top[6] = {
              {{-0.5f + x + chunk_x, 0.5f + y, -0.5f + z + chunk_z},  {0.0f,  1.0f,  0.0f}},
              {{0.5f + x + chunk_x,  0.5f + y, -0.5f + z + chunk_z},  {0.0f,  1.0f,  0.0f}},
              {{0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {0.0f,  1.0f,  0.0f}},
              {{0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {0.0f,  1.0f,  0.0f}},
              {{-0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {0.0f,  1.0f,  0.0f}},
              {{-0.5f + x + chunk_x,  0.5f + y, -0.5f + z + chunk_z},  {0.0f,  1.0f,  0.0f}}
            };
            for (uint32_t i = 0; i < 6; i++)
            {
              arrpush(render_state->vertex_buffer, top[i]);
            }
          }
          
          // bottom -y
          if (!has_voxel(x+chunk_x, y-1, z+chunk_z))
          {
            // add bottom
            Vertex bottom[6] = {
              {{-0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {0.0f, -1.0f,  0.0f}},
              {{0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {0.0f, -1.0f,  0.0f}},
              {{0.5f + x + chunk_x, -0.5f + y,  0.5f + z + chunk_z},  {0.0f, -1.0f,  0.0f}},
              {{0.5f + x + chunk_x, -0.5f + y,  0.5f + z + chunk_z},  {0.0f, -1.0f,  0.0f}},
              {{-0.5f + x + chunk_x, -0.5f + y,  0.5f + z + chunk_z},  {0.0f, -1.0f,  0.0f}},
              {{-0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {0.0f, -1.0f,  0.0f}}
            };
            for (uint32_t i = 0; i < 6; i++)
            {
              arrpush(render_state->vertex_buffer, bottom[i]);
            }
          }
          
          // front -1 z
          if (!has_voxel(x+chunk_x, y, z+1+chunk_z))
          {
            // add front
            Vertex front[6] = {
              {{-0.5f + x + chunk_x, -0.5f + y,  0.5f + z + chunk_z},  {0.0f,  0.0f, 1.0f}},
              {{0.5f + x + chunk_x, -0.5f + y,  0.5f + z + chunk_z},  {0.0f,  0.0f, 1.0f}},
              {{0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {0.0f,  0.0f, 1.0f}},
              {{0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {0.0f,  0.0f, 1.0f}},
              {{-0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {0.0f,  0.0f, 1.0f}},
              {{-0.5f + x + chunk_x, -0.5f + y ,  0.5f + z + chunk_z},  {0.0f,  0.0f, 1.0f}}
            };
            for (uint32_t i = 0; i < 6; i++)
            {
              arrpush(render_state->vertex_buffer, front[i]);
            }
          }
          
          // back 
          if (!has_voxel(x+chunk_x, y, z-1+chunk_z))
          {
            // add back
            Vertex back[6] = {
              {{-0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {0.0f,  0.0f, -1.0f}},
              {{0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {0.0f,  0.0f, -1.0f}},
              {{0.5f + x + chunk_x,  0.5f + y, -0.5f + z + chunk_z},  {0.0f,  0.0f, -1.0f}}, 
              {{0.5f + x + chunk_x,  0.5f + y, -0.5f + z + chunk_z}, {0.0f,  0.0f, -1.0f}}, 
              {{-0.5f + x + chunk_x,  0.5f + y, -0.5f + z + chunk_z},  {0.0f,  0.0f, -1.0f}}, 
              {{-0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {0.0f,  0.0f, -1.0f}}
            };
            for (uint32_t i = 0; i < 6; i++)
            {
              arrpush(render_state->vertex_buffer, back[i]);
            }
          }
          
          if (!has_voxel(x-1+chunk_x, y, z+chunk_z))
          {
            // add left
            Vertex left[6] = {
              {{-0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z}, {-1.0f,  0.0f,  0.0f}},
              {{-0.5f + x + chunk_x,  0.5f + y, -0.5f + z + chunk_z}, {-1.0f,  0.0f,  0.0f}},
              {{-0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z}, {-1.0f,  0.0f,  0.0f}},
              {{-0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z}, {-1.0f,  0.0f,  0.0f}},
              {{-0.5f + x + chunk_x, -0.5f + y,  0.5f + z + chunk_z}, {-1.0f,  0.0f,  0.0f}},
              {{-0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z}, {-1.0f,  0.0f,  0.0f}}
            };
            for (uint32_t i = 0; i < 6; i++)
            {
              arrpush(render_state->vertex_buffer, left[i]);
            }
          }
          
          if (!has_voxel(x+1+chunk_x, y, z+chunk_z))
          {
            // right
            Vertex right[6] = {
              {{0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {1.0f,  0.0f,  0.0f}},
              {{0.5f + x + chunk_x,  0.5f + y, -0.5f + z + chunk_z},  {1.0f,  0.0f,  0.0f}},
              {{0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {1.0f,  0.0f,  0.0f}},
              {{0.5f + x + chunk_x, -0.5f + y, -0.5f + z + chunk_z},  {1.0f,  0.0f,  0.0f}},
              {{0.5f + x + chunk_x, -0.5f + y,  0.5f + z + chunk_z},  {1.0f,  0.0f,  0.0f}},
              {{0.5f + x + chunk_x,  0.5f + y,  0.5f + z + chunk_z},  {1.0f,  0.0f,  0.0f}}
            };
            for (uint32_t i = 0; i < 6; i++)
            {
              arrpush(render_state->vertex_buffer, right[i]);
            }
            
          }
        }
      }
    }
  }
}


static void r_compile_cube_shader(RRenderState* render_state)
{
  char* vertex_source = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "layout (location = 1) in vec3 a_normal;\n"
    "uniform mat4 model\n;"
    "uniform mat4 proj_view;\n"
    
    "out vec3 normal;\n"
    "out vec3 frag_pos;\n"
    
    "void main() {\n"
    "  frag_pos = vec3(model * vec4(a_pos, 1.0));\n"
    "  normal = a_normal;\n"
    "  gl_Position = proj_view * model * vec4(a_pos, 1.0);\n"
    "}\n";
  
  char* frag_source =
    "#version 330 core\n"
    "uniform vec3 object_color;\n"
    "uniform vec3 light_cast;\n"
    "uniform vec3 light_color;\n"
    
    "in vec3 normal;\n"
    "in vec3 frag_pos;\n"
    
    "out vec4 FragColor;\n"
    
    "void main()\n"
    "{\n"
    "  vec3 norm = normalize(normal);\n"
    "  vec3 light_dir = normalize(light_cast);\n"
    "  float ambient_strength = 0.1;\n"
    "  vec3 ambient = ambient_strength * light_color;\n"
    "  float diff = max(dot(norm, light_dir), 0.0);\n"
    "  vec3 diffuse = diff * light_color;\n"
    "  vec3 result = (diffuse + ambient) * object_color;\n"
    "  FragColor = vec4(result, 1.0);\n"
    "}\n";
  
  render_state->cube_shader = r_compile_shader_program(vertex_source, frag_source);
}

static void r_bind_gl_buffers(RRenderState* render_state)
{
  glGenVertexArrays(1, &(render_state->vao));
  glGenBuffers(1, &(render_state->vbo));
  glBindVertexArray(render_state->vao);
  glBindBuffer(GL_ARRAY_BUFFER, render_state->vbo);
  
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Vertex) * arrlen(render_state->vertex_buffer),
               render_state->vertex_buffer,
               GL_STATIC_DRAW);
  
  
  // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
}

static void r_draw_buffers(RRenderState* render_state,
                           RCamera camera,
                           RColorRGBA cube_color,
                           RDirLight light)
{
  glBindVertexArray(render_state->vao);
  glUseProgram(render_state->cube_shader);
  gbMat4 model;
  gbMat4 translation;
  gbMat4 scale;
  
  gbVec3 scale_v = {1.0f, 1.0f, 1.0f};
  gbVec3 pos_v = {0.0f, 0.0f, 0.0f};
  
  gb_mat4_scale(&scale, scale_v);
  gb_mat4_translate(&translation, pos_v);
  gb_mat4_mul(&model, &translation, &scale);
  
  r_set_uniform_mat4(render_state->cube_shader, "model", model);
  r_set_uniform_mat4(render_state->cube_shader, "proj_view", camera.proj_view);
  r_set_uniform_rbg(render_state->cube_shader, "object_color", cube_color);
  r_set_uniform_v3(render_state->cube_shader, "light_cast", light.dir);
  r_set_uniform_rbg(render_state->cube_shader, "light_color", light.color);
  
  glDrawArrays(GL_TRIANGLES, 0, arrlen(render_state->vertex_buffer) * sizeof(Vertex));
}

// END CAMERA
///////////////////////////////////

#if 0
static RCube r_make_cube(RColorRGBA color, gbVec3 position, float size)
{
  Vertexvertices[] = {
    // position          normal
    
    // back
    
    {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
    {{0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
    {{0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}}, 
    {{0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}}, 
    {-0.5f,  0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}}, 
  {{-0.5f, -0.5f, -0.5f},  {0.0f,  0.0f, -1.0f}},
  
  
  // front
  {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
  {{0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
  {{0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
  {{-0.5f,  0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
  {{-0.5f, -0.5f,  0.5f},  {0.0f,  0.0f, 1.0f}},
  
  // left
  {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
  {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
  {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
  {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}},
  {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
  {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}},
  
  // right
  {{0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},
  {{0.5f,  0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
  {{0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
  {{0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
  {{0.5f, -0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},
  {{0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},
  
  // bottom
  {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}},
  {{0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}},
  {{0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}},
  {{0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}},
  {{-0.5f, -0.5f,  0.5f},  {0.0f, -1.0f,  0.0f}},
  {{-0.5f, -0.5f, -0.5f},  {0.0f, -1.0f,  0.0f}},
  
  // top
  {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}},
  {{0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}},
  {{0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}},
  {{0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}},
  {{-0.5f,  0.5f,  0.5f},  {0.0f,  1.0f,  0.0f}},
  {{-0.5f,  0.5f, -0.5f},  {0.0f,  1.0f,  0.0f}}
};

char* vertex_source = 
"#version 330 core\n"
"layout (location = 0) in vec3 a_pos;\n"
"layout (location = 1) in vec3 a_normal;\n"
"uniform mat4 model\n;"
"uniform mat4 proj_view;\n"

"out vec3 normal;\n"
"out vec3 frag_pos;\n"

"void main() {\n"
"  frag_pos = vec3(model * vec4(a_pos, 1.0));\n"
"  normal = a_normal;\n"
"  gl_Position = proj_view * model * vec4(a_pos, 1.0);\n"
"}\n";

char* frag_source =
"#version 330 core\n"
"uniform vec3 object_color;\n"
"uniform vec3 light_cast;\n"
"uniform vec3 light_color;\n"

"in vec3 normal;\n"
"in vec3 frag_pos;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"  vec3 norm = normalize(normal);\n"
"  vec3 light_dir = normalize(light_cast);\n"
"  float ambient_strength = 0.1;\n"
"  vec3 ambient = ambient_strength * light_color;\n"
"  float diff = max(dot(norm, light_dir), 0.0);\n"
"  vec3 diffuse = diff * light_color;\n"
"  vec3 result = (diffuse + ambient) * object_color;\n"
"  FragColor = vec4(result, 1.0);\n"
"}\n";

// model configuration

RCube return_cube;
glGenVertexArrays(1, &(return_cube.vao));
glGenBuffers(1, &(return_cube.vbo));
glBindVertexArray(return_cube.vao);
glBindBuffer(GL_ARRAY_BUFFER, return_cube.vbo);

glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

return_cube.color = color;
return_cube.pos = position;
return_cube.size = size;
return_cube.shader = r_compile_shader_program(vertex_source, frag_source);
return return_cube;
}// end of func

static void r_draw_cube(RDirLight light, RCamera camera)
{
  glBindVertexArray(cube.vao);
  r_set_active_shader(cube.shader);
  
  gbMat4 model;
  gbMat4 translation;
  gbMat4 scale;
  
  gbVec3 scale_v;
  
  scale_v.x = cube.size;
  scale_v.y = cube.size;
  scale_v.z = cube.size;
  
  gb_mat4_scale(&scale, scale_v);
  gb_mat4_translate(&translation, cube.pos);
  
  gb_mat4_mul(&model, &translation, &scale);
  
  r_set_uniform_mat4(cube.shader, "model", model);
  r_set_uniform_mat4(cube.shader, "proj_view", camera.proj_view);
  r_set_uniform_rbg(cube.shader, "object_color", cube.color);
  r_set_uniform_v3(cube.shader, "light_cast", light.dir);
  r_set_uniform_rbg(cube.shader, "light_color", light.color);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void r_clean_up_cube_buffers(RCube* cube)
{
  glDeleteVertexArrays(1, &(cube->vao));
  glDeleteBuffers(1, &(cube->vbo));
}


static void r_draw_chunk(Chunk chunk, RCube base_cube, RDirLight sun, RCamera camera)
{
  RCube cube = base_cube;
  
}
#endif