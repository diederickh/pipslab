#ifndef KANKER_GLYPH_H
#define KANKER_GLYPH_H

#define ROXLU_USE_MATH
#include <tinylib.h>
#include <stdio.h>
#include <vector>

#define ROXLU_USE_MATH
#define ROXLU_USE_LOG
#include <tinylib.h>

class KankerGlyph {

 public:
  KankerGlyph(int charcode);
  KankerGlyph(const KankerGlyph& other);
  KankerGlyph& operator=(const KankerGlyph& other);
  ~KankerGlyph();
  void addPoint(vec3& v);
  void addPoint(float x, float y, float z = 0.0);
  void onStartLine();                                 /* Must be called in input mode, when the user starts drawing a line. A glyph can contain multiple lines. */
  void onEndLine();                                   /* Must be called in input mode, when the user ends drawing a line. A glyph can contain multiple lines. */
  void clear();                                       /* Removes all line segments / points, normalized points and segments .. */
  void print();                                       /* Prints out some info about the glyph. */

  void centralize();                                  /* Centralizes the vertices. */
  void normalize();                                   /* Normalizes the points of all segments. */ 
  void normalize(float xHeight);                      /* Normalize using the given x-height. */
  void normalizeAndCentralize();                      /* The order in which we normalize/centralize is important; this function will make sure that the glyph is correctly positioned at 0,0 and can be displayed with e.g. openGL. */
  void translate(float x, float y);                   /* Translate all points by the given x and y. This will actually change the point values.*/
  void scale(float s);                                /* Scale all points + members */
  void flipHorizontal();
  void alignLeft();                                   /* Make sure that the min_x is 0. */ 
  void alignBottom();

 public:
  std::vector<std::vector<vec3> > segments;
  int charcode;
  float min_x; 
  float min_y;
  float max_x;
  float max_y;
  float width;
  float height;
  float advance_x;
  float origin_x; 
  float origin_y; /* @todo use this to align the font. */
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

  if (0 == segments.size()) {
    RX_VERBOSE("No segments found yet, not supposed to happen be we will create one.");
    std::vector<vec3> segment;
    segments.push_back(segment);
  }

  segments.back().push_back(vec3(x, y, z));
}

#endif
