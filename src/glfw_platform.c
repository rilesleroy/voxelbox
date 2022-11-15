// TODO(BadBanana): sound mixing with miniaudio?
// TODO(BadBanana): start learning about better lighting models
// TODO(BadBanana): model loading? look at raylibs: models.c file for inspiration
// TODO(BadBanana): swap render api to vulkan?
// TODO(BadBanana): multitheading? possibly use a job system (our machinery has blog posts on the topic, and there is a full chapter in the game engine arch book)
// TODO(BadBnnana): C as the build script for cross platform?

// NOTE(BadBanana): Open GL Stuff 

//https://github.com/ryanfleury/dungeoneer look 
#include <windows.h>
#include <glad/glad.h>
#include "glad.c"

// NOTE(BadBanana): Platform code
// TODO(BadBanana): These two combined might be heavier than 
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

// NOTE(BadBanana): ginger bill's math lib. I'll be using it as inspiration to write my own
#define GB_MATH_IMPLEMENTATION
#include "gb_math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define global static

typedef struct {
  size_t offset;
  size_t total;
  uint8_t* buffer;
} Arena;

Arena arena_init(size_t total_size) 
{
  Arena arena;
  arena.offset = 0;
  arena.total = total_size;
  arena.buffer = malloc(total_size);
  memset(arena.buffer, arena.total, 0);
  return arena;
}

#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
uintptr_t align_ptr(uintptr_t current_ptr, uintptr_t alignment)
{
  uintptr_t r_ptr = current_ptr;
  uintptr_t gap = current_ptr % alignment;
  if(gap != 0)
  {
    r_ptr += alignment - gap;
    return r_ptr;
  }
  return r_ptr;
}

void* arena_alloc(Arena* arena, size_t allocation)
{
  uintptr_t current_ptr = (uintptr_t)(arena->buffer) + (uintptr_t)(arena->offset);
  uintptr_t aligned_ptr = align_ptr(current_ptr, DEFAULT_ALIGNMENT);
  
  uintptr_t ptr_offset = aligned_ptr - current_ptr;
  arena->offset += (size_t)ptr_offset;
  
  if (arena->offset + allocation < arena->total)
  {
    void* r_ptr = (arena->buffer) + (arena->offset + allocation);
    arena->offset += allocation;
    return r_ptr;
  }
  return NULL;
}

void arena_zero(Arena* arena)
{
  memset(arena->buffer, arena->total, 0);
  arena->offset = 0;
}

void arena_destroy(Arena* arena) 
{
  free(arena->buffer);
}

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "renderer.c"
#include "tests.c"

#define RES_WIDTH 800
#define RES_HEIGHT 600



typedef struct {
  bool left;
  bool right;
  bool foreward;
  bool backward;
  bool up;
  bool down;
  // Mouse
  bool first_mouse;
  float last_x;
  float last_y;
  float current_x;
  float current_y;
}ActionMap;
global ActionMap active_action_map;


static void update_camera_position(RCamera* camera, ActionMap* actions, float delta_time) 
{
  
  gbVec3 wish_dir = {0.0f, 0.0f, 0.0f};
  float speed_update = camera->speed * delta_time;
  gbVec3 offset = {0.0f, 0.0f, 0.0f};
  
  // Camera look
  float x_offset = active_action_map.current_x - active_action_map.last_x;
  float y_offset = active_action_map.current_y - active_action_map.last_y;
  
  active_action_map.last_x = active_action_map.current_x;
  active_action_map.last_y = active_action_map.current_y;
  
  x_offset *= camera->look_sensitivity;
  y_offset *= camera->look_sensitivity;
  
  camera->yaw += x_offset;
  camera->pitch -= y_offset;
  
  if(camera->pitch > 89.0f)
    camera->pitch = 89.0f;
  if(camera->pitch < -89.0f)
    camera->pitch = -89.0f;
  
  gbVec3 direction;
  direction.x = cosf(gb_to_radians(camera->yaw)) * cosf(gb_to_radians(camera->pitch));
  direction.y = sinf(gb_to_radians(camera->pitch));
  direction.z = sinf(gb_to_radians(camera->yaw)) * cosf(gb_to_radians(camera->pitch));
  gb_vec3_norm(&(camera->front), direction);
  
  if (actions->foreward) {
    gbVec3 dir;
    gb_vec3_norm(&dir, camera->front);
    gb_vec3_addeq(&wish_dir, dir);
  }
  
  if (actions->backward) 
  {
    gbVec3 dir;
    gb_vec3_norm(&dir, camera->front);
    gb_vec3_subeq(&wish_dir, dir);
  }
  
  if (actions->left) 
  {
    gbVec3 dir;
    gb_vec3_cross(&dir, camera->front, camera->up);
    gb_vec3_norm(&dir, dir);
    gb_vec3_subeq(&wish_dir, dir);
  }
  
  if (actions->right) 
  {
    gbVec3 dir;
    gb_vec3_cross(&dir, camera->front, camera->up);
    gb_vec3_norm(&dir, dir);
    gb_vec3_addeq(&wish_dir, dir);
  }
  
  if (actions->down) 
  {
    gbVec3 dir;
    gb_vec3_norm(&dir, camera->up);
    gb_vec3_subeq(&wish_dir, dir);
  }
  
  if (actions->up) 
  {
    gbVec3 dir;
    gb_vec3_norm(&dir, camera->up);
    gb_vec3_addeq(&wish_dir, dir);
  }
  
  // update
  if (gb_vec3_mag(wish_dir) != 0.0f)
  {
    gb_vec3_norm(&wish_dir, wish_dir);
    gb_vec3_mul(&offset, wish_dir, speed_update);
    gb_vec3_addeq(&(camera->pos), offset);
  }
}

void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height)
{
  glViewport(0, 0, RES_WIDTH, RES_HEIGHT);
}

void process_input (GLFWwindow* window) 
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  active_action_map.foreward = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
  active_action_map.backward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
  active_action_map.left = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
  active_action_map.right = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
  active_action_map.down = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
  active_action_map.up = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  if (active_action_map.first_mouse)
  {
    active_action_map.last_x = xpos;
    active_action_map.last_y = ypos;
    active_action_map.first_mouse = false;
  }
  active_action_map.last_x = active_action_map.current_x;
  active_action_map.last_y = active_action_map.current_y;
  active_action_map.current_x = xpos;
  active_action_map.current_y = ypos;
}

int main(void)
{
  test_run_tests();
  // StartUp Code.
  // TODO: Move this startup up code so some place else
  // in order to clean up the main function at least a little bit wj
  
  if (glfwInit()){
    // Create a windowed mode window and its OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(RES_WIDTH,
                                          RES_HEIGHT,
                                          "SE-poto",
                                          NULL,
                                          NULL);
    if (!window) {
      printf("Couldnt create GLFW window\n");
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    // init GLAD
    if (!gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress)) {
      return -1;
    }
    
    RRenderState render_state;
    r_init_render_state(&render_state, window);
    
    RCamera active_camera = r_make_camera(render_state);
    
    //graphics_arena = arena_init(sizeof(Vertex) * 4096);
    
    
    RDirLight sun =
    {
      .color.r = 1.0f,
      .color.g = 1.0f,
      .color.b = 1.0f,
      .color.a = 1.0f,
      .dir.x = 0.2f,
      .dir.y = 0.3f,
      .dir.z = 0.2f
    };
    
    RColorRGBA cube_color = 
    {
      .r = 0.7f,
      .g = 0.7f, 
      .b = 0.7f,
      .a = 1.0f
    };
    
    RColorRGBA bg = 
    {
      .r = 0.5f,
      .g = 0.5f,
      .b = 0.5f,
      .a = 1.0f
    };
    
    r_compile_cube_shader(&render_state);
    r_mesh_chunk(&render_state, 0, 0);
    r_bind_gl_buffers(&render_state);
    
    
    float delta_time = 0.0f;
    float last_frame = 0.0f;
    
    // capture cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Render/input loop
    while (!glfwWindowShouldClose(window)) {
      // NOTE(BadBanana): need to fix my time loop
      float current_frame = glfwGetTime();
      delta_time = current_frame - last_frame;
      last_frame = current_frame;
      
      glfwPollEvents();
      process_input(window);
      
      // simulation
      update_camera_position(&active_camera, &active_action_map, delta_time);
      active_camera = r_update_camera_projection(render_state, active_camera);
      
      
      // rendering 
      // clear screen
      glEnable(GL_DEPTH_TEST);
      glClearColor(bg.r, bg.g, bg.b, bg.a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      r_draw_buffers(&render_state, active_camera, cube_color, sun);
      
      
      // Window resizing
      glfwSwapBuffers(window);
      r_resize_window_viewport(&render_state, window);
    }
    
    // Cleanup VAO/VBOs
    //glDeleteBuffers(1, &EBO);
    glfwTerminate();
  }
  
  else
  {
    printf("Couldnt init glfw\n");
  }
  return 0;
}