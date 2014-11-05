/*

  KankerAbb
  ----------

  Used to communicate with ABB and prepare the characters
  we want to write using the ABB. You can write a string and we 
  will make sure that the given string will fit inside the range 
  of the ABB. 

  You set the width / height of the range in milimeters before 
  calling KankerAbb::write()

 */
#ifndef KANKER_ABB_H
#define KANKER_ABB_H

#define ROXLU_USE_MATH
#define ROXLU_USE_PNG
#define ROXLU_USE_LOG
#include <tinylib.h>

#include <KankerFont.h>
#include <KankerGlyph.h>
#include <vector>

/* ---------------------------------------------------------------------- */

class KankerAbbGlyph {
 public:
  KankerAbbGlyph();
  ~KankerAbbGlyph();

 public:
  KankerGlyph glyph;
  std::vector<std::vector<vec3> > segments;
};

/* ---------------------------------------------------------------------- */

class KankerAbb {

 public:
  KankerAbb();
  ~KankerAbb();

  int write(KankerFont& font,
            std::string str, 
            std::vector<KankerAbbGlyph>& out, 
            std::vector<std::vector<vec3> >& segmentsOut);

  int save(std::string filepath, std::vector<KankerAbbGlyph>& message);
  float getWidth(KankerFont& font, std::string word);

 public:
  float offset_x;       /* used to position all glyphs with a offset. */
  float offset_y;       /* used to position all glyphs with a offset. */
  float range_width;    /* range in milimeters */
  float range_height;   /* range in milimeters */
  float char_scale;     /* how to scale the characters. */
  float word_spacing;   /* how many mm between words? */ 
  float line_height;    /* mm per line. */
};

#endif
