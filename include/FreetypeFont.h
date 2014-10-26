#ifndef FREETYPE_FONT_H
#define FREETYPE_FONT_H

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <glad/glad.h>
#include <tinylib.h>

#include <sstream>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

static const char* FREETYPE_FONT_VS = ""
  "#version 330\n"
  ""
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "uniform float[8] u_verts;"
  "out vec2 v_tex;"
  ""
  "const vec2[] texc = vec2[4] ("
  "  vec2(0.0, 1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)  "
  ");"
  ""
  "void main() {"
  "   gl_Position = u_pm * u_mm * vec4(u_verts[gl_VertexID * 2], u_verts[gl_VertexID * 2 + 1],  0.0, 1.0);"
  "   v_tex = texc[gl_VertexID];"
  "}";

static const char* FREETYPE_FONT_FS = "" 
  "#version 330\n"
  ""
  "uniform sampler2D u_tex;"
  "uniform float u_tex_coord_scale;"
  "uniform vec4 u_color;"
  "in vec2 v_tex;"
  "layout( location = 0 ) out vec4 fragcolor; "
  ""
  "void main() {"
  "  float a = texture(u_tex, vec2(v_tex.x * u_tex_coord_scale, v_tex.y)).r;"
  "  fragcolor = a * u_color;"
  "}"
  "";

class FreetypeFont {

 public:
  FreetypeFont();
  ~FreetypeFont();
  int open(std::string filepath, int pts);                                    /* Open the given font file and set the size to `pts`. Returns 0 on success otherwise < 0. */
  int write(std::string str);                                                 /* Write the text; you don't have to call this every draw(), we generate a texture once. */      
  void draw(int x, int y);                                                    /* Draw the text at x / y. */ 
  void color(float r = 1.0, float g = 1.0, float b = 1.0, float a = 1.0);     /* Set the color of the font. */
  void alignCenter();                                                         /* Align from center. */
  void alignTopLeft();                                                        /* When top left alignemnt is used, the x/y of draw(), point to the top left position where we start drawing. */

  template<class T>
    int write(T str) { 
      std::stringstream ss;
      ss << str;
      return write(ss.str());
    }
                                                                              
 private:                                                                     
  int initOpenGl();                                                           /* Only executed once, to setup the shaders. */
  int createTexture();                                                        /* Create the texture and upload the current pixels. */
  int getSizeOfString(std::string str, int& width, int& height);              /* Gets the width and height for the given string. Height is not the exact size of the pixels, but the necessary space we use. */
                                                                              
 public:                                                                      
  int align;                                                                  /* 0 = left, 1 = center. */
  int img_width;                                                              /* The height of the currently rendered string, can be smaller then tex_height. */ 
  int img_height;                                                             /* The width of the currently rendered string; the tex_width can be bigger when you the current string is smaller then one you drew previously */
  unsigned char* pixels;                                                      /* The pixels of the texture */
  int max_under_baseline;                                                     /* We adjust the y position of the characters with this value so the the descender touches the bottom of the bitmap */
  FT_Library library;                                                         /* The font context. */ 
  FT_Face face;                                                               /* The loaded font. */
  mat4 mm;                                                                    /* Model matrix */
  GLuint tex_id;                                                              /* The texture id that holds the currently drawn string. */
  int tex_width;                                                              /* Width of the texture (can be bigger then img_width). */ 
  int tex_height;                                                             /* Height of the texture (can be bigger then img_height). */
  size_t tex_nbytes;                                                          /* The number of bytes currently uploaded. Not the same as tex_ncapacity. tex_ncapacity is the maximum number of bytes that the texture can hold. */ 
  size_t tex_capacity;                                                        /* The maximum number of bytes we can store in the texture. */
  float tex_scale;                                                            /* When the img_width is smaller then tex_width we need to scale the texture coordinates a bit */
  float col[4];                                                               /* Color of font. */
                                                                              
  static GLuint vao;                                                          /* We use attribute less rendering which needs a VAO. */
  static GLuint vert;                                                         /* The vertex shader. */    
  static GLuint frag;                                                         /* The fragment shader. */
  static GLuint prog;                                                         /* The shader program. */
  static GLint u_pm;                                                          /* The projection matrix uniform. */
  static GLint u_mm;                                                          /* The model matrix uniform */
  static GLint u_verts;                                                       /* The vertices that we use to draw. */
  static GLint u_tex;                                                         /* Uniform to texture. */
  static GLint u_tex_coord_scale;                                             /* Uniform to the texcoord scale. */
  static GLint u_color;                                                       /* Uniform to the color. */
  static mat4 pm;                                                             /* Projection matrix. */
};

#endif
