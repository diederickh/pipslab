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
#define ROXLU_USE_LOG
#include <tinylib.h>
#include <rapidxml.hpp>
#include <kanker/KankerFont.h>
#include <kanker/KankerGlyph.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <stdint.h>

using namespace rapidxml;

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

  int write(KankerFont& font,                                                        /* Write a message using this font. */
            std::string str,                                                         /* The message to write. */
            std::vector<KankerAbbGlyph>& out,                                        /* We will fill this vector with glyphs that can be passed into `save()`. */
            std::vector<std::vector<vec3> >& segmentsOut);                           /* Contains only the line segments, used to draw. */

  int saveAbbModule(std::string filepath, int64_t id, std::vector<KankerAbbGlyph>& message);     /* Save the given glyphs into a file that can be read by the ABB. */
  float getWordWidth(KankerFont& font, std::string word);                            /* Returns the width of the given word in milimeters. */
  int saveSettings(std::string filepath);                                            /* Save the current state of the font. */ 
  int loadSettings(std::string filepath);                                            /* Load the current state of the font. */ 
  void print();                                                                      /* Prints some information about the object. */

 public:
  float offset_x;       /* used to position all glyphs with a offset. */
  float offset_y;       /* used to position all glyphs with a offset. */
  float range_width;    /* range in milimeters */
  float range_height;   /* range in milimeters */
  float char_scale;     /* how to scale the characters. */
  float word_spacing;   /* how many mm between words? */ 
  float line_height;    /* mm per line. */
  std::string ftp_url;  /* used by the controller; it would be cleaner to store this in the controller; */
};

#endif
