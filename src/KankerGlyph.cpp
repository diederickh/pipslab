#include <float.h>
#include <algorithm>
#include <KankerGlyph.h>

KankerGlyph::KankerGlyph(int charcode) 
  :charcode(charcode)
  ,curr_segment(NULL)
  ,min_x(FLT_MAX)
  ,min_y(FLT_MAX)
  ,max_x(FLT_MIN)
  ,max_y(FLT_MIN)
  ,width(0)
  ,height(0)
  ,advance_x(0)
{
}

KankerGlyph::~KankerGlyph() {

  charcode = 0;

  clear();
}

void KankerGlyph::onStartLine() {

  curr_segment = new LineSegment();
  if (NULL == curr_segment) {
    printf("error: failed to create line segment, out of mem?\n");
    return;
  }

  segments.push_back(curr_segment);
}

void KankerGlyph::onEndLine() {

  if (NULL == curr_segment) {
    printf("error: no segment in onEndDrawingLine.\n");
    return;
  }

  curr_segment = NULL;
}

void KankerGlyph::clear() {

  clearSegments();
  clearNormalizedSegments();
  curr_segment = NULL;
}

void KankerGlyph::clearSegments() {
  for (size_t i = 0; i < segments.size(); ++i) {
    delete segments[i];
  }
  segments.clear();
}

void KankerGlyph::clearNormalizedSegments() {

  for (size_t i = 0; i < normalized_segments.size(); ++i) {
    delete normalized_segments[i];
  }
  normalized_segments.clear();
}

void KankerGlyph::normalize() {

  if (0 != normalized_segments.size()) {
    clearNormalizedSegments();
  }

  /* copy the segments. */
  for (size_t i = 0; i < segments.size(); ++i) {
    LineSegment* source = segments[i];
    if (0 == source->points.size()) {
      continue;
    }

    LineSegment* copy = new LineSegment();
    if (NULL == copy) {
      printf("error: failed to allocate a new LineSegment while trying to normalize.\n");
      return;
    }

    std::copy(source->points.begin(), source->points.end(), std::back_inserter(copy->points));
    normalized_segments.push_back(copy);
  }

  float center_x = min_x + width * 0.5;
  float center_y = min_y + height * 0.5;
  float inv_width = 1.0f / width;
  float inv_height = 1.0f / height;
  float inv = inv_width;

  if (inv_height < inv_width) {
    inv = inv_height;
  }

  for (size_t i = 0; i < normalized_segments.size(); ++i) {
    LineSegment* seg = normalized_segments[i];
    for (size_t j = 0; j < seg->points.size(); ++j) {

      seg->points[j].x -= center_x;   /* centralize */
      seg->points[j].y -= center_y;   /* centralize */

      seg->points[j].x *= inv;        /* normalize */
      seg->points[j].y *= -inv;       /* normalize */
    }
  }

#if 0
  vec4 bbox = getBoundingBox();
  float width = bbox[2] - bbox[0];
  float height = bbox[3] - bbox[1];

  float center_x = bbox[0] + width * 0.5;
  float center_y = bbox[1] + height * 0.5;
  float inv_width = 1.0f / width;
  float inv_height = 1.0f / height;

  float inv = inv_width;
  if (inv_height < inv_width) {
    inv = inv_height;
  }
  
  printf("height: %f, width: %f, inv_width: %f, inv_height: %f\n", height, width, inv_width, inv_height);
  bbox.print();

  for (size_t i = 0; i < segments.size(); ++i) {
    LineSegment* seg = segments[i];
    for (size_t j = 0; j < seg->points.size(); ++j) {

      seg->points[j].x -= center_x;   /* centralize */
      seg->points[j].y -= center_y;   /* centralize */

      seg->points[j].x *= inv;        /* normalize */
      seg->points[j].y *= -inv;        /* normalize */
    }
  }
#endif
}

vec4 KankerGlyph::getBoundingBox() {
  vec4 bbox; /* x = top left (x), y = top left (y), z = bottom righ (x)t, w = bottom right (y) */
  bbox.x = FLT_MAX;
  bbox.y = FLT_MAX;
  bbox.w = FLT_MIN;
  bbox.z = FLT_MIN;

  for (size_t i = 0; i < segments.size(); ++i) {
    LineSegment* seg = segments[i];
    for (size_t j = 0; j < seg->points.size(); ++j) {
      vec3 point = seg->points[j];
      if (point.x < bbox.x) {
        bbox.x = point.x;
      }
      if (point.y < bbox.y) {
        bbox.y = point.y;
      }
      if (point.x > bbox.z) {
        bbox.z = point.x;
      }
      if (point.y > bbox.w) {
        bbox.w = point.y;
      }
    }
  }

  return bbox;
}

void KankerGlyph::translate(float x, float y) {

  for (size_t i = 0; i < segments.size(); ++i) {
    LineSegment* seg = segments[i];
    for (size_t j = 0; j < seg->points.size(); ++j) {
      vec3& point = seg->points[j];
      point.x += x;
      point.y += y;
    }
  }

  min_x += x;
  max_x += x;
  min_y += y;
  max_y += y;
}
