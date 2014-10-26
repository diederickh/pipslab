#include <KankerDrawer.h>

KankerDrawer::KankerDrawer() 
  :vbo(0)
  ,vao(0)
  ,vert(0)
  ,prog(0)
  ,frag(0)
  ,u_pm(-1)
  ,u_mm(-1)
  ,u_vm(-1)
{
}

KankerDrawer::~KankerDrawer() {
}

int KankerDrawer::init() {

  if (0 != vao) {
    printf("error: looks like we're already initialized in KankerDrawer.\n");
    return -1;
  }

  /* We start with a VBO that can hold 1000 vertices. */
  capacity = sizeof(KankerVertex) * 1000;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, capacity, NULL, GL_STREAM_DRAW);

  glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, sizeof(KankerVertex), (GLvoid*)0); /* pos. */
  glEnableVertexAttribArray(0); /* pos. */

  vert = rx_create_shader(GL_VERTEX_SHADER, KANKER_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, KANKER_FS);
  prog = rx_create_program(vert, frag, true);

  glUseProgram(prog);
  u_pm = glGetUniformLocation(prog, "u_pm");
  u_mm = glGetUniformLocation(prog, "u_mm");
  u_vm = glGetUniformLocation(prog, "u_vm");

  if (-1 == u_pm || -1 == u_mm || -1 == u_vm) {
    printf("error: cannot get uniforms, u_pm: %d, u_mm: %d, u_vm: %d\n", u_pm, u_mm, u_vm);
    /* @todo cleanup. */
    return -2;
  }
                                                
  pm.perspective(64.0f, 4.0 / 3.0, 0.0f, 100.0f);
  mm.identity();
  mm.scale(0.005f, -0.005, 0.005);
  vm.translate(0.0f, 2.0f, -4.5f);
  mm.print();
  glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm.ptr());
  glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());
  glUniformMatrix4fv(u_vm, 1, GL_FALSE, vm.ptr());

  return 0;
}

void KankerDrawer::draw() {
  
  glBindVertexArray(vao);
  glUseProgram(prog);

  mm.rotateY(0.01);
  glUniformMatrix4fv(u_mm, 1, GL_FALSE, mm.ptr());

  glMultiDrawArrays(GL_LINE_STRIP, &offsets[0], &counts[0], counts.size());
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
 
  for (size_t i = 0; i < glyph->segments.size(); ++i) {

    LineSegment* seg = glyph->segments[i];
    if (0 == seg->points.size()) {
      continue;
    }

    offsets.push_back(vertices.size());

    for (size_t k = 0; k < seg->points.size(); ++k) {
      KankerVertex v;
      v.set(seg->points[k]);
      vertices.push_back(v);
    }

    counts.push_back(vertices.size() - offsets.back());
  }

  if (0 == vertices.size()) {
    printf("error: no vertices found in KankerDrawer (?).\n");
    return -3;
  }

  printf("offsets: %lu, counts: %lu, vertices: %lu\n", offsets.size(), counts.size(), vertices.size());
  
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  
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

