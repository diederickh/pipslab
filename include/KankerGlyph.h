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
  void onStartLine();                                 /* Must be called in input mode, when the user starts drawing a line. A glyph can contain multiple lines. */
  void onEndLine();                                   /* Must be called in input mode, when the user ends drawing a line. A glyph can contain multiple lines. */
  void normalize();                                   /* Normalizes the points of all segments. */ 
  vec4 getBoundingBox();                              /* Returns the bounding box of all points. */
  void translate(float x, float y);                   /* Translate all points by the given x and y. This will actually change the point values.*/
  void clear();                                       /* Removes all line segments / points, normalized points and segments .. */
  void clearSegments();                               /* Removes the line segments */
  void clearNormalizedSegments();                     /* Removes all the normalized line */

 public:
  int charcode;
  LineSegment* curr_segment;
  std::vector<LineSegment*> segments;
  std::vector<LineSegment*> normalized_segments;
  float min_x; 
  float min_y;
  float max_x;
  float max_y;
  float width;
  float height;
  float advance_x;
};

inline void KankerGlyph::addPoint(vec3& v) {
  addPoint(v.x, v.y, v.z);

}

inline void KankerGlyph::addPoint(float x, float y, float z) {


  if (x > max_x) {
    max_x = x;
    width = max_x - min_x;
  }
  if (x < min_x) {
    min_x = x;
    width = max_x - min_x;
  }
  if (y > max_y) {
    max_y = y;
    height = max_y - min_y;
  }
  if (y < min_y) {
    min_y = y;
    height = max_y - min_y;
  }

  if (NULL == curr_segment) {
    printf("error: cannot add point to glyph, because there is no segment. Did you call createLineSegment? \n");
    return;
  }
  //printf("Adding: %f, %f\n", x, y);
  curr_segment->points.push_back(vec3(x,y,z));
}

#endif
