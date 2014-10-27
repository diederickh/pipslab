/*
  
  KankerDrawer
  -----------

  Draws glyphsx in a more interesint way.

 */
#ifndef KANKER_DRAWER_H
#define KANKER_DRAWER_H

#include <KankerGlyph.h>
#include <BlurFBO.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#define ROXLU_USE_OPENGL
#include <glad/glad.h>
#include <tinylib.h>

typedef VertexPT KankerVertex;

static const char* KANKER_VS = ""
  "#version 330\n"
  ""
  "uniform float u_time;"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "uniform mat4 u_vm;"
  ""
  "layout ( location = 0 ) in vec4 a_pos; "
  "layout ( location = 1 ) in vec2 a_tex; "
  ""
  "out vec2 v_tex;"
  ""
  "void main() { "
  "  vec4 pos = a_pos;"
  //  "  pos.x = a_pos.x + (0.5 * sin(15.0 * pos.y));"
  "  gl_Position = u_pm * u_vm * u_mm * pos; "
  "  v_tex = a_tex;"
  "}"
  "";

static const char* KANKER_FS = ""
  "#version 330\n"
  ""
  "uniform float u_time;"
  "uniform sampler2D u_tex;"
  ""
  "layout ( location = 0 ) out vec4 fragcolor;"
  ""
  "in vec2 v_tex;"
  ""
  "void main() {"
  "  vec4 tc = texture(u_tex, vec2(v_tex.s, 0.5 + sin(v_tex.t + u_time * 0.5) * 0.5));"
  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
  "  fragcolor = tc * vec4(1.0 * v_tex.t, 0.5 + sin(u_time) * 0.5, 1.0 - v_tex.t, 1.0);"
  "}"
  "";

static const char* KANKER_MIX_FS = ""
  "#version 330\n"
  ""
  "uniform sampler2D u_bg_tex;"
  "uniform sampler2D u_blur_tex;"
  "uniform sampler2D u_scene_tex;"
  ""
  "layout ( location = 0 ) out vec4 fragcolor; " 
  ""
  "in vec2 v_texcoord;"
  ""
  "void main() {"
  "  vec4 blur_col = texture(u_blur_tex, v_texcoord);"
  "  vec4 scene_col = texture(u_scene_tex, v_texcoord);"
  //  "  vec4 bg_col = texture(u_bg_tex, v_texcoord);"

  "  fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
  "  fragcolor = 0.6 * vec4(blur_col.r) + (3.4 * scene_col);"
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
  GLuint geom_vbo;
  GLuint geom_vao;
  GLuint geom_prog;
  GLuint geom_vert;
  GLuint geom_frag;
  GLuint geom_tex;
  GLuint mix_vao;        /* mix, mixes blurred + front + ?? */
  GLuint mix_vert;
  GLuint mix_frag;
  GLuint mix_prog;
  GLuint bg_tex;
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

  FBO fbo;
  BlurFBO blur;
};

#endif
