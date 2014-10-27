#include <KankerDrawer.h>

#define DRAW_LINES 0
#define DRAW_STRIPS 1

KankerDrawer::KankerDrawer() 
  :geom_vbo(0)
  ,geom_vao(0)
  ,geom_vert(0)
  ,geom_prog(0)
  ,geom_frag(0)
  ,geom_tex(0)
  ,u_pm(-1)
  ,u_mm(-1)
  ,u_vm(-1)
{
}

KankerDrawer::~KankerDrawer() {
}

int KankerDrawer::init() {

  if (0 != geom_vao) {
    printf("error: looks like we're already initialized in KankerDrawer.\n");
    return -1;
  }

  /* We start with a GEOM_VBO that can hold 1000 geom_vertices. */
  capacity = sizeof(KankerVertex) * 1000;

  glGenVertexArrays(1, &geom_vao);
  glBindVertexArray(geom_vao);
  glGenBuffers(1, &geom_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, geom_vbo);
  glBufferData(GL_ARRAY_BUFFER, capacity, NULL, GL_STREAM_DRAW);

  glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, sizeof(KankerVertex), (GLvoid*)0); /* pos. */
  glVertexAttribPointer(1, 2,  GL_FLOAT, GL_FALSE, sizeof(KankerVertex), (GLvoid*)12); /* tex. */
  glEnableVertexAttribArray(0); /* pos. */
  glEnableVertexAttribArray(1); /* tex. */

  geom_vert = rx_create_shader(GL_VERTEX_SHADER, KANKER_VS);
  geom_frag = rx_create_shader(GL_FRAGMENT_SHADER, KANKER_FS);
  geom_prog = rx_create_program(geom_vert, geom_frag, true);

  glUseProgram(geom_prog);
  u_pm = glGetUniformLocation(geom_prog, "u_pm");
  u_mm = glGetUniformLocation(geom_prog, "u_mm");
  u_vm = glGetUniformLocation(geom_prog, "u_vm");

  if (-1 == u_pm || -1 == u_mm || -1 == u_vm) {
    printf("error: cannot get uniforms, u_pm: %d, u_mm: %d, u_vm: %d\n", u_pm, u_mm, u_vm);
    /* @todo cleanup. */
    return -2;
  }
                                                
  pm.perspective(45.0f, 1280.0 / 768.0, 0.1f, 100.0f);
  mm.identity();
#if DRAW_LINES
  mm.scale(0.005f, 0.005, 0.005);
  vm.translate(0.0f, 0.0f, -4.5f);
#elif DRAW_STRIPS
  mm.translate(0.0, 0.0, 0.0);
  vm.translate(0.0f, 0.0f, -2.5f);
#endif

  glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
  glUniformMatrix4fv(u_vm, 1, GL_FALSE, vm.ptr());

  unsigned char* pixels = NULL;
  int w, h, channels;
  rx_load_png(rx_to_data_path("light.png"), &pixels, w, h, channels);
  if (NULL == pixels) {
    printf("error: cannot load the pixels for the light streak.\n");
    exit(EXIT_FAILURE);
  }
  
  glGenTextures(1, &geom_tex);
  glBindTexture(GL_TEXTURE_2D, geom_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  delete[] pixels;
  pixels = NULL;

  glUniform1i(glGetUniformLocation(geom_prog, "u_tex"), 0);

  if (0 != blur.init(1280, 768, 5.0f)) {
    printf("error: failed to initialize the blurfbo.\n");
    exit(EXIT_FAILURE);
  }

  fbo.init(1280, 768);
  fbo.addTexture(GL_RGBA, fbo.width, fbo.height, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);

  /* setup the mix pass. */
  glGenVertexArrays(1, &mix_vao);
  mix_vert = rx_create_shader(GL_VERTEX_SHADER, ROXLU_OPENGL_FULLSCREEN_VS);
  mix_frag = rx_create_shader(GL_FRAGMENT_SHADER, KANKER_MIX_FS);
  mix_prog = rx_create_program(mix_vert, mix_frag, true);
  glUseProgram(mix_prog);
  glUniform1i(glGetUniformLocation(mix_prog, "u_blur_tex"), 0);
  glUniform1i(glGetUniformLocation(mix_prog, "u_scene_tex"), 1);
  glUniform1i(glGetUniformLocation(mix_prog, "u_bg_tex"), 2);

  /* background texture */
  w = h = channels = 0;
  rx_load_png(rx_to_data_path("bg.png"), &pixels, w, h, channels);
  if (NULL == pixels) {
    printf("error: cannot load the pixels for the light streak.\n");
    exit(EXIT_FAILURE);
  }
  
  glGenTextures(1, &bg_tex);
  glBindTexture(GL_TEXTURE_2D, bg_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  delete[] pixels;
  pixels = NULL;

  return 0;
}

void KankerDrawer::draw() {

  glBindVertexArray(geom_vao);
  glUseProgram(geom_prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, geom_tex);

  mm.rotateY(0.03);
  glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
  glUniform1f(glGetUniformLocation(geom_prog, "u_time"), rx_millis());

#if DRAW_LINES
  glMultiDrawArrays(GL_LINE_STRIP, &offsets[0], &counts[0], counts.size());
#elif DRAW_STRIPS

#if 0
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glMultiDrawArrays(GL_TRIANGLE_STRIP, &offsets[0], &counts[0], counts.size());
#else
  fbo.bind();
  {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMultiDrawArrays(GL_TRIANGLE_STRIP, &offsets[0], &counts[0], counts.size());
  }
  fbo.unbind();

  blur.blur(fbo.textures[0]);
  /* @todo - create custom shader. */
  //fbo.blit(GL_COLOR_ATTACHMENT0, 0, 0, fbo.width, fbo.height);
  // blur.fbo.blit(GL_COLOR_ATTACHMENT0, 0, 0, fbo.width, fbo.height);

  glDisable(GL_BLEND);
  glUseProgram(mix_prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, blur.tex());

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, fbo.textures[0]);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, bg_tex);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  
  //glClear(GL_COLOR_BUFFER_BIT);
  //glEnable(GL_BLEND);

#endif
  /*
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  painter.clear();
  painter.texture(blur.tex(), 0, 0, fbo.width, fbo.height);
  painter.texture(fbo.textures[0], 0, 0, fbo.width, fbo.height);
  painter.draw();
  */
#endif

}

int KankerDrawer::updateVertices(KankerGlyph* glyph) {

  if (NULL == glyph) {
    printf("error: invalid glyph given to updateVertices, is null.\n");
    return -1;
  }

  if (0 == glyph->segments.size()) {
    printf("error: no segments in the glyph, cannot update vertices.\n");
    return -2;
  }

  vertices.clear();
  offsets.clear();
  counts.clear();

  float scale = 1.1f;

#if DRAW_STRIPS
  glyph->normalize();
#endif
 
  for (size_t i = 0; i < glyph->segments.size(); ++i) {

    LineSegment* seg = glyph->segments[i];
    if (0 == seg->points.size()) {
      continue;
    }

    offsets.push_back(vertices.size());

#if DRAW_LINES

    for (size_t k = 0; k < seg->points.size(); ++k) {
      KankerVertex v;
      v.pos.set(seg->points[k].x,seg->points[k].y, seg->points[k].z);
      vertices.push_back(v);
    }

#elif DRAW_STRIPS

    if (seg->points.size() < 2) {
      continue;
    }

    for (size_t k = 1; k < seg->points.size() - 1; ++k) {

      float p = float(k-1)/(seg->points.size()-2);
      vec3& a = seg->points[k - 1];
      vec3& b = seg->points[k];
      vec3 dir = b - a;
      vec3 up(0.0, 0.0, 1.0);
      vec3 crossed = normalized(cross(dir, up));
      float w = sin(p * 3.1415);
      vec3 aa = a + (crossed * 0.05f * w); //  * (0.5 + sin(p * 10) *1.2));
      vec3 bb = a - (crossed * 0.05f * w);

      KankerVertex vert_a;
      KankerVertex vert_b;

      vert_a.pos.set(aa.x, aa.y, aa.z);
      vert_b.pos.set(bb.x, bb.y, bb.z);

      vert_a.tex.set(1.0, p);
      vert_b.tex.set(0.0, p);

      vertices.push_back(vert_a);
      vertices.push_back(vert_b);
    }

#endif

    counts.push_back(vertices.size() - offsets.back());
  }

  if (0 == vertices.size()) {
    printf("error: no vertices found in KankerDrawer (?).\n");
    return -3;
  }

  printf("offsets: %lu, counts: %lu, vertices: %lu\n", offsets.size(), counts.size(), vertices.size());
  
  glBindVertexArray(geom_vao);
  glBindBuffer(GL_ARRAY_BUFFER, geom_vbo);
  
  size_t needed = sizeof(KankerVertex) * vertices.size();
  if (needed > capacity) {
    glBufferData(GL_ARRAY_BUFFER, needed, vertices[0].ptr(), GL_STREAM_DRAW);
    capacity = needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, needed, vertices[0].ptr());
  }

  return 0;
}

