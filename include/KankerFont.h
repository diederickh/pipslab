#ifndef KANKER_FONT_H
#define KANKER_FONT_H

#include <KankerGlyph.h>
#include <map>
#include <fstream>
#include <sstream>

#define ROXLU_USE_LOG
#include <tinylib.h>

class KankerFont {

 public:
  KankerFont();
  ~KankerFont();
  KankerGlyph* getGlyph(int charcode);     /* Get the glyph for the given character code. */
  int save(std::string filepath);          /* Save the current font as xml. */
  int load(std::string filepath);          /* Load the font from the given xml file path. */
  void clear();                            /* Removes all added glyphs. */

 public:
  std::map<int, KankerGlyph*> glyphs;      /* KankerFont is owner of the glyphs and it will free allocated memory. */
};

#endif
