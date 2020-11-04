#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glad_gl.c>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdio.h>
#include <linmath.h>

static unsigned int shader;
static unsigned int texture;
static unsigned int quad;
static mat4x4       proj, view;
// static mat4x4 model;

static char   shader_value_buff[128];
static mat4x4 models[16];

unsigned char* get_file_contents(const char* fp) {
  FILE* f = fopen(fp, "rb+");
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);

  unsigned char* data = malloc(size + 1);
  fread(data, size, sizeof(unsigned char), f);

  data[size] = '\0';
  fclose(f);

  return data;
}

unsigned int get_sub_shader(const char* f, int type) {
  unsigned char* data = get_file_contents(f);
  unsigned int   id   = glCreateShader(type);

  glShaderSource(id, 1, (const char**)&data, NULL);
  glCompileShader(id);

  unsigned int success;
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (success != GL_TRUE) {
    int maxlen = 0;
    int len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxlen);
    char* log = malloc(maxlen);

    if (!log) {
      printf("Unable to malloc space for log.\n");
      return -1;
    }

    glGetShaderInfoLog(id, maxlen, &len, log);
    printf("%s\n", log);
    free(log);
  }

  free(data);
  return id;
}

unsigned int get_shader(const char* v, const char* f) {
  unsigned int _v = get_sub_shader(v, GL_VERTEX_SHADER);
  unsigned int _f = get_sub_shader(f, GL_FRAGMENT_SHADER);

  unsigned int id = glCreateProgram();

  glAttachShader(id, _v);
  glAttachShader(id, _f);

  glLinkProgram(id);

  unsigned int success;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (success != GL_TRUE) {
    int maxlen = 0;
    int len;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxlen);
    char* log = malloc(maxlen);

    if (!log) {
      printf("Unable to malloc space for log.\n");
      return -1;
    }

    glGetProgramInfoLog(id, maxlen, &len, log);
    printf("%s\n", log);
    free(log);
  }

  glDeleteShader(_v);
  glDeleteShader(_f);

  return id;
}

unsigned int get_texture(const char* f) {
  int w, h, c;

  unsigned int   id;
  unsigned char* img = stbi_load(f, &w, &h, &c, 0);

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               img);

  stbi_image_free(img);

  return id;
}

unsigned int get_vao() {
  unsigned int vao, vbo, vboi;
  float        verts[16] = {// pos       //tex
                     -0.5f, -0.5f, 0.f, 0.f, -0.5f, 0.5f,  0.f, 1.f,
                     0.5f,  0.5f,  1.f, 1.f, 0.5f,  -0.5f, 1.f, 0.f};

  unsigned int inds[6] = {0, 1, 2, 2, 3, 0};

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glGenBuffers(1, &vboi);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), &verts[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &inds[0],
               GL_STATIC_DRAW);

  glBindVertexArray(0);

  return vao;
}

void setup_render() {
  shader  = get_shader("res/shader.vert", "res/shader.frag");
  texture = get_texture("res/test.png");
  quad    = get_vao();

  mat4x4_identity(proj);
  mat4x4_identity(view);

  mat4x4_ortho(proj, 0.f, 1280.f, 720.f, 0.f, -10.f, 10.f);

  float x_offset = (1280.f / 2.f) - (2 * 72.f);
  float y_offset = (720.f / 2.f) - (2 * 72.f);

  for (int i = 0; i < 16; ++i) {
    int x = i % 4;
    int y = i / 4;

    mat4x4_identity(models[i]);
    mat4x4_translate(models[i], (72.f * x) + x_offset, (72.f * y) + y_offset,
                     1.f);
    mat4x4_scale_aniso(models[i], models[i], 64.f, 64.f, 1.f);
  }
}

void set_m4(unsigned int shader, const char* uniform, mat4x4 m) {
  glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, m);
}

void set_m4x(unsigned int shader, const char* uniform, mat4x4* arrays,
             unsigned int count) {
  for (int i = 0; i < count; ++i) {
    sprintf(shader_value_buff, "%s[%d]", uniform, i);
    int location = glGetUniformLocation(shader, shader_value_buff);
    glUniformMatrix4fv(location, 1, GL_FALSE, arrays[i]);
    memset(shader_value_buff, 0, sizeof(char) * 128);
  }
}

void render() {
  glUseProgram(shader);

  set_m4(shader, "proj", proj);
  set_m4(shader, "view", view);

  // Standard single render
  // set_m4(shader, "model", model);
  set_m4x(shader, "models", models, 16);

  glBindTexture(GL_TEXTURE_2D, texture);

  glBindVertexArray(quad);
  glEnableVertexAttribArray(0);

  // Standard single render
  // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 16);
}

int main(int argc, char* argv[]) {
  if (!glfwInit()) {
    return 1;
  }
  // Target OpenGL 3.2 with forward compatability!
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Use the core profile, we won't be needing any extensions
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create the window with GLFW
  GLFWwindow* window = glfwCreateWindow(1280, 720, "demo", NULL, NULL);

  if (!window) {
    printf("Unable to make window.\n");
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);

  glViewport(0, 0, 1280, 720);

  glClearColor(0.05f, 0.05f, 0.05f, 1.f);

  setup_render();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render();

    glfwSwapBuffers(window);
  }

  return 0;
}
