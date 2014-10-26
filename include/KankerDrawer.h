/*
  
  KankerDrawer
  -----------

  Draws glyphsx in a more interesint way.

 */
#ifndef KANKER_DRAWER_H
#define KANKER_DRAWER_H

#include <KankerGlyph.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <glad/glad.h>
#include <tinylib.h>

#define KankerVertex VertexP

static const char* KANKER_VS = ""
  "#version 330\n"
  ""
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "uniform mat4 u_vm;"
  ""
  "layout ( location = 0 ) in vec4 a_pos; "
  ""
  "void main() { "
  "  gl_Position = u_pm * u_vm * u_mm * a_pos; "
  "}"
  "";

static const char* KANKER_FS = ""
  "#version 330\n"
  "layout ( location = 0 ) out vec4 fragcolor;"
  ""
  "void main() {"
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
  "}"
  "";

class KankerDrawer {

 public:
  KankerDrawer();
  ~KankerDrawer();
  int init();
  int updateVertices(KankerGlyph* glyph);
  void draw();

 public:
  GLuint vbo;
  GLuint vao;
  GLuint prog;
  GLuint vert;
  GLuint frag;
  GLint u_pm;
  GLint u_mm;
  GLint u_vm;
  size_t capacity;        /* number of bytes that can be stored in the VBO. */
  mat4 mm;                /* model matrix */
  mat4 pm;                /* projection matrix. */
  mat4 vm;                /* view matrix. */

  std::vector<GLint> offsets;
  std::vector<GLsizei> counts;
  std::vector<KankerVertex> vertices;
};

#endif
