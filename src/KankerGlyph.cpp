#include <KankerGlyph.h>

KankerGlyph::KankerGlyph(int charcode) 
  :charcode(charcode)
  ,curr_segment(NULL)
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

  for (size_t i = 0; i < segments.size(); ++i) {
    delete segments[i];
  }
  segments.clear();

  curr_segment = NULL;
}
