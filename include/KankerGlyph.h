#ifndef KANKER_GLYPH_H
#define KANKER_GLYPH_H

#define ROXLU_USE_MATH
#include <tinylib.h>

#include <stdio.h>
#include <vector>

struct LineSegment {
  std::vector<vec3> points;
};

class KankerGlyph {
 public:
  KankerGlyph(int charcode);
  ~KankerGlyph();
  void addPoint(vec3& v);
  void addPoint(float x, float y, float z = 0.0);
  void clear();                                       /* Removes all line segments / points .. */
  void onStartLine();                                 /* Must be called in input mode, when the user starts drawing a line. A glyph can contain multiple lines. */
  void onEndLine();                                   /* Must be called in input mode, when the user ends drawing a line. A glyph can contain multiple lines. */
  void normalize();                                   /* Normalizes the points of all segments. */ 
  vec4 getBoundingBox();                              /* Returns the bounding box of all points. */

 public:
  int charcode;
  LineSegment* curr_segment;
  std::vector<LineSegment*> segments;
};

inline void KankerGlyph::addPoint(vec3& v) {
  addPoint(v.x, v.y, v.z);
}

inline void KankerGlyph::addPoint(float x, float y, float z) {

  if (NULL == curr_segment) {
    printf("error: cannot add point to glyph, because there is no segment. Did you call createLineSegment? \n");
    return;
  }
  //printf("Adding: %f, %f\n", x, y);
  curr_segment->points.push_back(vec3(x,y,z));
}

#endif
