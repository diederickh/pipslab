#ifndef KANKER_FONT_H
#define KANKER_FONT_H

#include <kanker/KankerGlyph.h>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#define ROXLU_USE_LOG
#include <tinylib.h>

class KankerFont {

 public:
  KankerFont();
  ~KankerFont();
  int save(std::string filepath);                                        /* Save the current font as xml. */
  int load(std::string filepath);                                        /* Load the font from the given xml file path. */
  void clear();                                                          /* Removes all added glyphs. */
  size_t size();                                                         /* Returns the number of elements in the glyphs member. */
  void write(std::string str, std::vector<std::vector<vec3> >& lines);   /* Fill the `lines` argument with the points with the correct positions to write the string. */
  int getBaseHeight(float& height);                                      /* Sets the 'x-height', so the x, u, v, w or z characters must be added. Is used to normalize the characters. Returns 0 on success else < 0*/
  KankerGlyph* getGlyphByCharCode(int charcode);                         /* Get the glyph for the given character code. */
  KankerGlyph* getGlyphByIndex(size_t dx);                               /* Get the glyph for the given index. */ 
  bool hasGlyph(int charcode);                                           /* Returns true when the given char code exists in the font. */
  void setOrigin(float originX, float originY);                          /* Sets the origin x/y that are used when editing glyphs. These origins are using to "normalize" and align characters int their own bounding boxes. */

 public:
  std::vector<KankerGlyph*> glyphs;                                      /* KankerFont is owner of the glyphs and it will free allocated memory. */
  float origin_x;                                                        /* The origin_x is the x-offset that was used to draw the character on screen so we can nicely reposition it when editing. This is the origin point. */
  float origin_y;                                                        /* The origin_y is the y-offset that was used to draw the character on screen so we can nicely reposition it when editing. This is the origin point. */
};

inline void KankerFont::setOrigin(float x, float y) {

  origin_x = x;
  origin_y = y;

  for (size_t i = 0; i < glyphs.size(); ++i) {
    glyphs[i]->origin_x = origin_x;
    glyphs[i]->origin_y = origin_y;
  }
}

inline size_t KankerFont::size() {
  return glyphs.size();
}

inline KankerGlyph* KankerFont::getGlyphByIndex(size_t dx) {

  if (dx >= glyphs.size()) {
    RX_ERROR("Trying to get a glyph outside the range of the available glyphs: %lu", dx);
    return NULL;
  }

  return glyphs[dx];
}

inline bool KankerFont::hasGlyph(int charcode) {

  for (size_t i = 0; i < glyphs.size(); ++i) {
    if (glyphs[i]->charcode == charcode) {
      return true;
    }
  }

  return false;
}

#endif
