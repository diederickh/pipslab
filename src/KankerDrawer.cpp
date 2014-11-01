#include <assert.h>
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
  ,mix_vao(0)
  ,mix_vert(0)
  ,mix_frag(0)
  ,mix_prog(0)
  ,u_pm(-1)
  ,u_mm(-1)
  ,u_vm(-1)
  ,capacity(0)
  ,rtt_width(-1)
  ,rtt_height(-1)
  ,win_width(-1)
  ,win_height(-1)
{
}

KankerDrawer::~KankerDrawer() {
}

int KankerDrawer::init(int rttWidth, int rttHeight, int winWidth, int winHeight) {

  if (0 != geom_vao) {  RX_ERROR("error: looks like we're already initialized in KankerDrawer.");  return -1;  }
  if (0 >= rttWidth) {  RX_ERROR("error: invalid rtt width: %d", rttWidth);  return -2;   }
  if (0 >= rttHeight) { RX_ERROR("error: invalid rtt height: %d", rttHeight);  return -3;  }
  if (0 >= winWidth) { RX_ERROR("error: invalid win width."); return -4; } 
  if (0 >= winHeight) { RX_ERROR("error: invalid win height."); return -5; } 

  rtt_width = rttWidth;
  rtt_height = rttHeight;
  win_width = winWidth;
  win_height = winHeight;

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
    RX_ERROR("error: cannot get uniforms, u_pm: %d, u_mm: %d, u_vm: %d.", u_pm, u_mm, u_vm);
    /* @todo cleanup. */
    return -2;
  }
                                                
  pm.perspective(45.0f, float(rtt_width) / rtt_height, 0.1f, 100.0f);
  mm.identity();
  mm.translate(0.0, 0.0, 0.0);
  vm.translate(0.0f, 0.0f, -2.5f);

  glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
  glUniformMatrix4fv(u_vm, 1, GL_FALSE, vm.ptr());

  unsigned char* pixels = NULL;
  int w, h, channels;
  rx_load_png(rx_to_data_path("light.png"), &pixels, w, h, channels);
  if (NULL == pixels) {
    RX_ERROR("error: cannot load the pixels for the light streak.");
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

  if (0 != blur.init(rtt_width, rtt_height, 5.0f)) {
    RX_ERROR("error: failed to initialize the blurfbo.");
    exit(EXIT_FAILURE);
  }

  fbo.init(rtt_width, rtt_height);
  fbo.addTexture(GL_RGBA, fbo.width, fbo.height, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);

  /* setup the mix pass. */
  glGenVertexArrays(1, &mix_vao);
  mix_vert = rx_create_shader(GL_VERTEX_SHADER, ROXLU_OPENGL_FULLSCREEN_VS);
  mix_frag = rx_create_shader(GL_FRAGMENT_SHADER, KANKER_MIX_FS);
  mix_prog = rx_create_program(mix_vert, mix_frag, true);
  glUseProgram(mix_prog);
  glUniform1i(glGetUniformLocation(mix_prog, "u_blur_tex"), 0);
  glUniform1i(glGetUniformLocation(mix_prog, "u_scene_tex"), 1);
  return 0;
}

void KankerDrawer::update() {
  mm.rotateY(0.03);
}

void KankerDrawer::renderAndDraw(int x, int y) {
  renderToTexture();
  draw(x, y);
}

void KankerDrawer::renderToTexture() {

  glViewport(0, 0, rtt_width, rtt_height);

  glBindVertexArray(geom_vao);
  glUseProgram(geom_prog);

  /* Draw textured version. */
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, geom_tex);
  
  glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
  glUniform1f(glGetUniformLocation(geom_prog, "u_time"), rx_millis());
  
  fbo.bind();
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMultiDrawArrays(GL_TRIANGLE_STRIP, &offsets[0], &counts[0], counts.size());
  }
  fbo.unbind();

  /* Blur. */
  blur.blur(fbo.textures[0]);

  glViewport(0, 0, win_width, win_height);
}

void KankerDrawer::draw(int x, int y) {
  assert(win_width > 0);
  assert(win_height > 0);
  assert(rtt_width > 0);
  assert(rtt_height > 0);

  glViewport(x, y, rtt_width, rtt_height);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  /* Draw the final version with blurred + textured layer. */
  glUseProgram(mix_prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, blur.tex());

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, fbo.textures[0]);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glViewport(0, 0, win_width, win_height);
}

int KankerDrawer::updateVertices(KankerGlyph glyph) {

  if (0 == glyph.segments.size()) {
    RX_ERROR("error: no segments in the glyph, cannot update vertices.");
    return -2;
  }

  vertices.clear();
  offsets.clear();
  counts.clear();

  glyph.normalizeAndCentralize();

  for (size_t i = 0; i < glyph.segments.size(); ++i) {

    std::vector<vec3>& points = glyph.segments[i];
    if (points.size() < 2) {
      RX_VERBOSE("Not engouh points in glyph segment, segment: %lu", i);
      continue;
    }

    offsets.push_back(vertices.size());

    for (size_t k = 1; k < points.size() - 1; ++k) {

      float p = float(k-1)/(points.size()-2);
      vec3& a = points[k - 1];
      vec3& b = points[k];
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

    counts.push_back(vertices.size() - offsets.back());
  }

  if (0 == vertices.size()) {
    RX_ERROR("error: no vertices found in KankerDrawer (?).");
    return -3;
  }

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


