#define test(name, func) \
printf("TEST-%s: %s\n", name, (func ? "PASSED" : "FAILED"))


static bool test_test_macro() {
  return true;
}

static bool test_add_vertex_to_strechy_buffer()
{
  Vertex* vertex_buffer = {NULL};
  Vertex vert = {{-0.5f, -0.5f , -0.5f},  {0.0f, -1.0f,  0.0f}};
  arrpush(vertex_buffer, vert);
  if (arrlen(vertex_buffer) == 1) return true;
  return false;
}

static bool test_add_multiple_verts_to_strechy_buffer()
{
  Vertex verts[6] = {
    {{0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},
    {{0.5f,  0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
    {{0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
    {{0.5f, -0.5f, -0.5f},  {1.0f,  0.0f,  0.0f}},
    {{0.5f, -0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}},
    {{0.5f,  0.5f,  0.5f},  {1.0f,  0.0f,  0.0f}}
  };
  
  Vertex* vertex_buffer = {NULL};
  
  for (uint32_t i = 0; i < 6; i++)
  {
    arrpush(vertex_buffer, verts[i]);
  }
  
  if (arrlen(vertex_buffer) == 6) return true;
  return false;
}


static void test_run_tests()
{
  printf("START OF TESTS\n");
  test("test_macro", test_test_macro());
  test("add_vertex_to_strechy_buffer", test_add_vertex_to_strechy_buffer());
  test("add_multiple_verts_to_strechy_buffer", test_add_multiple_verts_to_strechy_buffer());
  printf("END OF TESTS\n");
}