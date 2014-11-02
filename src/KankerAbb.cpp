#include <sstream>
#include <KankerAbb.h>

/* ---------------------------------------------------------------------- */

KankerAbbGlyph::KankerAbbGlyph() 
  :glyph(0)
{
}

KankerAbbGlyph::~KankerAbbGlyph() {
  glyph.clear();
}

/* ---------------------------------------------------------------------- */

KankerAbb::KankerAbb() 
  :range_width(0.0f)
  ,range_height(0.0f)
  ,char_scale(50.0f)
  ,word_spacing(10.0f)
  ,line_height(35.0f)
{
}

KankerAbb::~KankerAbb() {
}

int KankerAbb::init(float rangeW, float rangeH) {

  if (rangeW <= 0.0) {
    RX_ERROR("Invalid range width: %f", rangeW);
    return -1;
  }

  if (rangeH <= 0.0) {
    RX_ERROR("Invalid range height: %f", rangeH);
    return -2;
  }

  range_width = rangeW;
  range_height = rangeH;

  return 0;
}

int KankerAbb::write(KankerFont& font, std::string str, std::vector<KankerAbbGlyph>& result) {

  std::stringstream ss;
  std::string word;
  bool has_newline = true;
  float pen_x = 0;
  float pen_y = 0;
  float word_width = 0;
  float width_available = range_width;
  float height_available = range_height;

  if (0.0f == range_width) { RX_ERROR("Range width not yet set, call init() first.");  return -1;  }
  if (0.0f == range_height) { RX_ERROR("Range height not yet set, call init() first.");  return -2;  }
  if (0 == str.size()) { RX_ERROR("Trying to write to ABB but the given text size is 0.");  return -3;  }
  if (0 != result.size()) {
    RX_ERROR("It looks the given result argument already contains data. We're cleaning it!");
    result.clear();
  }

  ss << str;

  while (ss >> word) {

    /* Does this word fit on the current line? */
    word_width = getWidth(font, word);
    float left = width_available - word_width;
    if (left > 0) {
      width_available -= left;
    }
    else {
      width_available = range_width;
      pen_y += line_height;
      pen_x = 0;
      has_newline = true;
    }
    
    /* Generate vertices for this word. */
    for (size_t i = 0; i < word.size(); ++i) {
      
      KankerGlyph* glyph_ptr = font.getGlyphByCharCode(word[i]);
      if (NULL == glyph_ptr) {
        RX_ERROR("Cannot find the glyph for `%c`, not supposed to happen.", word[i]);
        continue;
      }

      KankerAbbGlyph abb_glyph;
      abb_glyph.glyph = *glyph_ptr;

      abb_glyph.glyph.normalize();
      abb_glyph.glyph.flipHorizontal();     
      abb_glyph.glyph.alignLeft();
      abb_glyph.glyph.alignBottom();
      abb_glyph.glyph.scale(char_scale);

      abb_glyph.glyph.translate(pen_x, pen_y);

      //abb_glyph.glyph.print();

      std::copy(abb_glyph.glyph.segments.begin(),
                abb_glyph.glyph.segments.end(), 
                std::back_inserter(abb_glyph.segments));

      result.push_back(abb_glyph);

      pen_x += abb_glyph.glyph.advance_x;
      //printf("%c, pen_x: %f, pen_y: %f\n", (char)word[i], pen_x, pen_y);
    }
    //printf("-\n");
    pen_x += word_spacing;

    /* Do we need to start the next wordt on a new line? */
    if (width_available < word_spacing) {
      width_available = range_width;
      pen_y += line_height;
      pen_x = 0;
      has_newline = true;
    }

    /* Do we still have enough space for a next line? */
    if ((height_available - (pen_y + line_height)) < 0) {
      RX_ERROR("Message is to big to fit in the available space. Stopping with writing now.\n");
      break;
    }
  }

  return 0;
}

float KankerAbb::getWidth(KankerFont& font, std::string word) {

  float width = 0.0f;

  if (0 == word.size()) {
    return 0.0f;
  }

  for (size_t i = 0; i < word.size(); ++i) {

    if (!font.hasGlyph(word[i])) {
      RX_ERROR("Glyph not found: %c", word[i]);
      continue;
    }

    KankerGlyph* g = font.getGlyphByCharCode(word[i]);
    if (NULL == g) {
      RX_ERROR("Cannot get glyph by char code... not supposed to happen.");
      continue;
    }

    KankerGlyph glyph = *g;
    glyph.normalize();
    width += glyph.advance_x;
  }

  width *= char_scale;

  return width;
}

