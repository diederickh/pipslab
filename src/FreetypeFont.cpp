#include <stdio.h>
#include <kanker/FreetypeFont.h>

GLuint FreetypeFont::vao = 0;
GLuint FreetypeFont::frag = 0;
GLuint FreetypeFont::vert = 0;
GLuint FreetypeFont::prog = 0;
GLint FreetypeFont::u_pm = -1;
GLint FreetypeFont::u_mm = -1;
GLint FreetypeFont::u_verts = -1;
GLint FreetypeFont::u_tex = -1;
GLint FreetypeFont::u_tex_coord_scale = -1;
GLint FreetypeFont::u_color = -1;
mat4 FreetypeFont::pm;

FreetypeFont::FreetypeFont() 
  :img_width(0)
  ,img_height(0)
  ,pixels(NULL)
  ,library(NULL)
  ,face(NULL)
  ,tex_id(0)
  ,tex_width(0)
  ,tex_height(0)
  ,tex_nbytes(0)
  ,tex_capacity(0)
  ,align(0)
  ,tex_scale(1.0f)
{
  col[0] = 1.0f;
  col[1] = 0.0f;
  col[2] = 1.0f;
  col[3] = 1.0f;
}

FreetypeFont::~FreetypeFont() {
  
  if (NULL != pixels) {
    free(pixels);
    pixels = NULL;
  }

  if (0 != FT_Done_Face(face)) {
    printf("error: error while destroying the font face.\n");
  }

  if(0 != FT_Done_FreeType(library)) {
    printf("error: error fhile trying to free the lib.\n");
  }

  if (tex_id) {
    glDeleteTextures(1, &tex_id);
  }
  
  library = NULL;
  face = NULL;
  img_width = 0;
  img_height = 0;
  max_under_baseline = 0;
  mm.identity();
  tex_width = 0;
  tex_height = 0;
  tex_nbytes = 0;
  tex_capacity = 0;
  tex_id = 0;
  col[0] = 0.0f;
  col[1] = 0.0f;
  col[2] = 0.0f;
  col[3] = 0.0f;
}

int FreetypeFont::open(std::string filepath, int pts) {

  FT_Error err;

  if (0 == vao) {
    if (0 != initOpenGl()) {
      printf("error: cannot initialize OpenGL.\n");
      return -100;
    }
  }

  if (NULL != face) {
    printf("error: we already loaded another file, first close this one.\n");
    return -1;
  }

  if (NULL != library) {
    printf("error: library already initialize, close it first.\n");
    return -2;
  }

  if (0 == filepath.size()) {
    printf("error: invalid filepath; is empty.\n");
    return -3;
  }

  err = FT_Init_FreeType(&library);
  if (0 != err) {
    printf("error: cannot initialize the freetype library.\n");
    return -1;
  }

  err = FT_New_Face(library, filepath.c_str(), 0, &face);

  if (FT_Err_Unknown_File_Format == err) {
    printf("error: unsupported file format.\n");
    /* @todo cleanup. */
    return -4;
  }
  else if (0 != err) {
    printf("error: cannot open: %s\n", filepath.c_str());
    /* @todo cleanup */
    return -5;
  }

  err = FT_Set_Char_Size(face, 0, pts * 64, 72, 72);

  if (0 != err) {
    printf("error: failed to set font size.\n");
    /* @todo cleanup */
    return -6;
  }

  return 0;
}

int FreetypeFont::write(std::string str) {

  int prev_w = img_width;
  int prev_h = img_height;
  int prev_tex_w = tex_width;
  int prev_tex_h = tex_height;

  if (NULL == face) {
    printf("error: cannot render because you didn't open a file yet.\n");
    return -1;
  }

  if (0 != getSizeOfString(str, img_width, img_height)) { 
    printf("error: cannot render because we can't get the size of the the string.\n");
    return -2;
  }

  if (img_width > tex_width) {
    tex_width = img_width;
  }

  if (img_height > tex_height) {
    tex_height = img_height;
  }

  /* align on 32 bits. */
  while ( (tex_width & 0x03) != 0) {
    tex_width++;
  }

  while ( (tex_height & 0x03) != 0) {
    tex_height++;
  }
  
  /* allocate buffer into which we write. */
  tex_nbytes = tex_width * tex_height;

  if (tex_nbytes > tex_capacity) {
    if (NULL == pixels) {
      pixels = (unsigned char*)malloc(tex_nbytes);
      if (NULL == pixels) {
        printf("error: failed to allocate buffer for font, needed bytes: %lu\n", tex_nbytes);
        return -3;
      }
    }
    else {
      unsigned char* tmp = (unsigned char*)realloc(pixels, tex_nbytes);
      if (NULL == tmp) {
        printf("erorr: failed to reallocate buffer for font, needed bytes: %lu\n", tex_nbytes);
        return -4;
      }
      pixels = tmp;
    }
    tex_capacity = tex_nbytes;
  }

  /* when size changes we have to reset our 'canvas' */
  memset(pixels, 0x00, tex_capacity);

  /* render the font. */
  FT_Error err;
  FT_UInt glyph_index;
  FT_GlyphSlot slot = face->glyph;
  int pen_x = 0;
  int pen_y = 0;
  size_t len = str.size();
  int j = 0;
  size_t dest_dx = 0;
  size_t src_dx = 0;

  for (size_t i = 0; i < len; ++i) {

#if 0
    if (str[i] == '\n') {
      pen_y += 16;
      pen_x = 0;
      continue;
    }
#endif

    /* @todo -> isn't there a way to let freetype render into our buffer directly? */
    err = FT_Load_Char(face, str[i], FT_LOAD_RENDER);
    if (0 != err) {
      printf("error: failed to load and render the character: %c\n", str[i]);
      continue;
    }

    for (j = 0; j < slot->bitmap.rows; ++j) {
      dest_dx = (pen_y + j + (img_height - slot->bitmap_top - max_under_baseline)) * tex_width + pen_x;
      src_dx = j * slot->bitmap.width;
      memcpy(pixels + dest_dx, slot->bitmap.buffer + src_dx, slot->bitmap.pitch);
    }

    pen_x += (slot->advance.x >> 6);
  }

  /* Copy pixels + create texture*/
  if (0 == tex_id) {
     createTexture();
  }
  else {
    if (tex_width > prev_tex_w || tex_height > prev_tex_h) {
      /* grow texture, create new one. */
      glDeleteTextures(1, &tex_id);
      createTexture();
    }
    else {
      /* copy smaller sized bitmap into texture. */
      glPixelStorei(GL_UNPACK_ROW_LENGTH, tex_width);
      glBindTexture(GL_TEXTURE_2D, tex_id);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img_width, img_height, GL_RED, GL_UNSIGNED_BYTE, pixels);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }
    
    tex_scale = float(img_width) / tex_width;
  }

  return 0;
}

int FreetypeFont::createTexture() {
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return 0;
}

/* see freetype.h, `FT_GlyphSlotRec`, ~line 1669 */
int FreetypeFont::getSizeOfString(std::string str, int& w, int& h) {

  FT_Error err;

  if (NULL == face) {
    printf("error: cannot get size of string because you haven't opened a font yet.\n");
    return -1;
  }

  size_t len = str.size();
  int under_baseline = 0;
  int nlines = 1; /* @todo implement correct alignment for the lines. */
  int max_w = 0;
  w = 0;
  h = 0;
  max_under_baseline = 0;

  for (size_t i = 0; i < len; ++i) {

    if (str[i] == '\n') {
      w = 0;
      ++nlines;
      continue;
    }

    FT_UInt dx = FT_Get_Char_Index(face, (FT_ULong)str[i]);
    if (0 == dx) {
      printf("error: cannot find character index for char: `%c`\n", str[i]);
      continue;
    }

    /* @todo - is this the fastest solution to just get the metrics ? */
    err = FT_Load_Glyph(face, dx, FT_LOAD_DEFAULT);
    if (0 != err) {
      printf("error: failed loading the glyph at index %u\n", dx);
      continue;
    }

    under_baseline = (face->glyph->metrics.height - face->glyph->metrics.horiBearingY) >> 6;
    if (under_baseline > max_under_baseline) {
      max_under_baseline = under_baseline;
    }

    if (face->glyph->metrics.height > h) {
      h = face->glyph->metrics.height;
    }
    
    w += face->glyph->advance.x;
    if (w > max_w) {
      max_w = w;
    }
  }

  w = max_w;
  w >>= 6;
  
  if (FT_IS_SCALABLE(face)) {
    h = (face->bbox.yMax - face->bbox.yMin) * (face->size->metrics.y_ppem / (float)face->units_per_EM);
  }
  else {
    h = face->size->metrics.height >> 6;
  }

  h *= nlines;

  return 0;
}

void FreetypeFont::draw(int x, int y) {

  mm[0] = img_width;
  mm[5] = img_height;
  mm[12] = x;
  mm[13] = y;


  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glUseProgram(prog);

  if (0 == align) {
    float verts[] = { 0.0,  1.0, 0.0, 0.0, 1.0,  1.0,  1.0, 0.0 };
    glUniform1fv(u_verts, 8, verts);
  }
  else {

    float verts[] = { -0.5,  0.5, -0.5, -0.5,  0.5,  0.5,  0.5, -0.5 };
    glUniform1fv(u_verts, 8, verts);
  }

  glBindVertexArray(vao);
  glUniform1f(u_tex_coord_scale, tex_scale);
  glUniform4fv(u_color, 1, col);
  glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
  glUniform4fv(u_color, 1, col);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

int FreetypeFont::initOpenGl() {
  
  GLint viewport[4] = { 0 } ;

  glGenVertexArrays(1, &vao);
  
  vert = rx_create_shader(GL_VERTEX_SHADER, FREETYPE_FONT_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, FREETYPE_FONT_FS);
  prog = rx_create_program(vert, frag, true);

  glUseProgram(prog);
  glGetIntegerv(GL_VIEWPORT, viewport);

  u_pm = glGetUniformLocation(prog, "u_pm");
  if (-1 == u_pm) {
    printf("error: cannot get the u_pm uniform.\n");
    return -1;
  }

  u_mm = glGetUniformLocation(prog, "u_mm");
  if (-1 == u_pm) {
    printf("error: cannot get the u_mm uniform.\n");
    return -1;
  }

  u_verts = glGetUniformLocation(prog, "u_verts");
  if (-1 == u_verts) {
    printf("error: cannot get the u_verts uniform.\n");
    return -1;
  }

  u_tex = glGetUniformLocation(prog, "u_tex");
  if (-1 == u_tex) {
    printf("error: cannot get the u_tex uniform.\n");
    return -1;
  }

  u_tex_coord_scale  = glGetUniformLocation(prog, "u_tex_coord_scale");
  if (-1 == u_tex_coord_scale) {
    printf("error: cannot get the u_tex_coord_scale uniform.\n");
    return -1;
  }

  u_color  = glGetUniformLocation(prog, "u_color");
  if (-1 == u_color) { 
    printf("error: cannot get the u_color uniform.\n");
    return -1;
  }

  glUniform1f(u_tex_coord_scale, 1.0);
  glUniform1i(u_tex, 0);


  pm.ortho(0, viewport[2], viewport[3], 0, 0.0f, 100.0f);
  glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm.ptr());

  alignTopLeft();
  return 0;
}

void FreetypeFont::alignCenter() {
  align = 1;
}

void FreetypeFont::alignTopLeft() {
  align = 0;
}

void FreetypeFont::color(float r, float g, float b, float a) {
  col[0] = r;
  col[1] = g;
  col[2] = b;
  col[3] = a;
}
