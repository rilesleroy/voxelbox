/* date = May 19th 2021 10:37 pm */

#ifndef RENDERER_H
#define RENDERER_H

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

typedef struct {
  int win_width;
  int win_height;
} RRenderState;

typedef struct {
  uint32_t vao;
  uint32_t vbo;
  RShaderProgram shader;
  gbVec3 pos;
  float size;
  RColorRGBA color;
} RCube;

#define MAX_VOXEL_COUNT 16

typedef struct {
  float density;
} Voxel;

typedef struct
{
  gbVec3 position;
  //Voxel voxels[MAX_VOXEL_COUNT][MAX_VOXEL_COUNT][MAX_VOXEL_COUNT];
} Chunk;


static void r_resize_window_viewport(RRenderState* render_state, GLFWwindow* window);
static void r_init_render_state(RRenderState* render_state, GLFWwindow* window);

static RShaderProgram r_compile_shader_program(char* vertex_source, char* fragment_source);
static void r_set_active_shader(RShaderProgram shader);
static void r_set_uniform_bool(RShaderProgram shader, char* name, bool value);
static void set_uniform_i32(RShaderProgram shader, char* name, int32_t value);
static void r_set_uniform_f32(RShaderProgram shader, char* name, float value);
static void r_set_uniform_rbg(RShaderProgram shader, char* name, RColorRGBA color);
static void r_set_uniform_v3(RShaderProgram shader, char* name, gbVec3 vector);
static void r_set_uniform_mat4(RShaderProgram shader, char* name, gbMat4 value);

static RCamera r_make_camera(RRenderState render_state);
static RCamera r_update_camera_projection(RRenderState render_state, RCamera camera);

static RCube r_make_cube(RColorRGBA, gbVec3 position, float size);
static void r_draw_cube(RCube cube, RDirLight light, RCamera camera);
static void r_clean_up_cube(RCube* cube);
static void r_draw_chunk(Chunk chunk, RCube base_cube, RDirLight sun, RCamera camera);

#endif //RENDERER_H
