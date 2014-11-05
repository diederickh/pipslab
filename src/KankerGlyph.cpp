#include <float.h>
#include <algorithm>
#include <kanker/KankerGlyph.h>

KankerGlyph::KankerGlyph(int charcode) 
  :charcode(charcode)
  ,min_x(FLT_MAX)
  ,min_y(FLT_MAX)
  ,max_x(FLT_MIN)
  ,max_y(FLT_MIN)
  ,width(0)
  ,height(0)
  ,advance_x(0)
{
}

KankerGlyph::KankerGlyph(const KankerGlyph& other) {

  if (&other != this) {
    *this = other;
  }
}

KankerGlyph& KankerGlyph::operator=(const KankerGlyph& other) {

  segments = other.segments;
  charcode = other.charcode;
  min_x = other.min_x;
  min_y = other.min_y;
  max_x = other.max_x;
  max_y = other.max_y;
  width = other.width;
  height = other.height;
  advance_x = other.advance_x;

  return *this;
}

KankerGlyph::~KankerGlyph() {

  charcode = 0;

  clear();
}

void KankerGlyph::onStartLine() {

  std::vector<vec3> seg;
  segments.push_back(seg);
}

void KankerGlyph::onEndLine() {
}

void KankerGlyph::clear() {
  segments.clear();
}

void KankerGlyph::normalizeAndCentralize() {
  centralize();
  normalize();
}

void KankerGlyph::normalize() {

  /* this will fit the character in a 1x1 box.  */
  float inv_width = 1.0f / width;
  float inv_height = 1.0f / height;
  float inv = inv_width;

  if (inv_height < inv_width) {
    inv = inv_height;
  }
  inv = inv_height;

#if 0
  /* we can't use the full height because of ascenders/descenders ! */
  float sh = height;
  if (height > 100) {
    sh = 100;
  }
  float inv = 1.0 / sh;
#endif

#if 0
  float inv = 1.0f / height;
#endif

  for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      points[j].x *= inv;     
      points[j].y *= -inv;  /* we need to flip ! */
    }
  }

  width *= inv;
  height *= inv;
  min_x *= inv;
  max_x *= inv;
  min_y *= inv;
  max_y *= inv;
  advance_x *= inv;
}

void KankerGlyph::normalize(float xHeight) {

  if (xHeight <= 0.0) {
    RX_ERROR("Trying to normalize with xheight, but the xheight <= %f", xHeight);
    return;
  }
  
  float inv = 1.0f / xHeight;

  for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      points[j].x *= inv;     
      points[j].y *= -inv;  /* we need to flip ! */
    }
  }

  width *= inv;
  height *= inv;
  min_x *= inv;
  max_x *= inv;
  min_y *= inv;
  max_y *= inv;
  advance_x *= inv;
}

void KankerGlyph::centralize() {

  float center_x = min_x + width * 0.5;
  float center_y = min_y + height * 0.5;

  for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      points[j].x -= center_x;  
      points[j].y -= center_y;  
    }
  }

  min_x -= center_x;
  max_x -= center_x;
  min_y -= center_y;
  max_y -= center_y;
}

void KankerGlyph::translate(float x, float y) {

  for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      vec3& point = points[j];
      point.x += x;
      point.y += y;
    }
  }

  min_x += x;
  max_x += x;
  min_y += y;
  max_y += y;
}

void KankerGlyph::scale(float s) {
  
  float scale_x = s;
  float scale_y = s;

  if (height > width) {
    scale_y = s;
    scale_x = (width / height) * s;
  }
  else {
    scale_x = s;
    scale_y = (height / width) * s;
  }

  scale_x = s;
  scale_y = s;
  
  #if 0
  printf("%c: width: %f, height: %f, scale_x: %f, scale_y: %f\n", 
         (char)charcode, width,height,scale_x,scale_y);
  #endif
  
  for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      vec3& point = points[j];
      point.x *= scale_x;
      point.y *= scale_y;
    }
  }

  min_x *= scale_x;
  max_x *= scale_x;
  min_y *= scale_y;
  max_y *= scale_y;
  width *= scale_x;
  height *= scale_y;
  advance_x *= scale_x;
}

void KankerGlyph::flipHorizontal() {

  for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      vec3& point = points[j];
      point.y *= -1;
    }
  }
}

void KankerGlyph::alignLeft() {
  
 for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      vec3& point = points[j];
      point.x -= min_x;
    }
  }

 min_x = 0;
 max_x -= min_x;
}

void KankerGlyph::alignBottom() {

  for (size_t i = 0; i < segments.size(); ++i) {
    std::vector<vec3>& points = segments[i];
    for (size_t j = 0; j < points.size(); ++j) {
      vec3& point = points[j];
      point.y -= min_y;
    }
  }

  min_y = 0;
  max_y -= min_y;
}



void KankerGlyph::print() {

  RX_VERBOSE("glyph.charcode: %d, char: %c", charcode, (char) charcode);
  RX_VERBOSE("glyph.min_x: %f", min_x);
  RX_VERBOSE("glyph.min_y: %f", min_y);
  RX_VERBOSE("glyph.max_x: %f", max_x);
  RX_VERBOSE("glyph.max_y: %f", max_y);
  RX_VERBOSE("glyph.width: %f", width);
  RX_VERBOSE("glyph.height: %f", height);
  RX_VERBOSE("glyph.advance_x: %f", advance_x);
  RX_VERBOSE("glyph.segments.size(): %lu", segments.size());
  RX_VERBOSE("------");
}
