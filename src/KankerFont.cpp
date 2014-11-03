#include <KankerFont.h>
#include <iterator>
#include <algorithm>
#include <rapidxml.hpp>

using namespace rapidxml;

/* --------------------------------------------------------------------------------- */

template<class T> T read_attribute(xml_node<>* node, std::string name, T def) {

  if (NULL == node) {
    RX_ERROR("error: failed to read attribute, given node is NULL.");
    return def;
  }
  if (0 == name.size()) {
    RX_ERROR("error: failed to read attribute, because attribute name is invalid.");
    return def;
  }

  xml_attribute<>* attr = node->first_attribute(name.c_str());
  if (NULL == attr) {
    RX_ERROR("error: failed to find attribute: %s", name.c_str());
    return def;
  }

  T result;
  std::stringstream ss;
  ss << attr->value();
  ss >> result;

  return result;
}

/* --------------------------------------------------------------------------------- */

KankerFont::KankerFont() {
}

KankerFont::~KankerFont() {
  clear();
}

void KankerFont::clear() {

  for (size_t i = 0; i < glyphs.size(); ++i) {
    delete glyphs[i];
  }
  glyphs.clear();
}

KankerGlyph* KankerFont::getGlyphByCharCode(int charcode) {
  
  for (size_t i = 0; i < glyphs.size(); ++i) {
    if (glyphs[i]->charcode == charcode) {
      return glyphs[i];
    }
  }

  KankerGlyph* new_glyph = new KankerGlyph(charcode);
  if (NULL == new_glyph) {
    RX_ERROR("error: cannot allocate glyph.");
    return NULL;
  }

  glyphs.push_back(new_glyph);
  return new_glyph;
}

int KankerFont::save(std::string filepath) {

  if (0 == filepath.size()) {
    RX_ERROR("error: invalid filepath, is empty.");
    return -1;
  }

  std::ofstream ofs(filepath.c_str(), std::ios::out);
  if (!ofs.is_open()) {
    RX_ERROR("error: failed to open %s, no permission maybe?", filepath.c_str());
    return -2;
  }

  RX_VERBOSE("Iterating over font...");

  std::stringstream ss;
  ss << "<font>\n";
  
   std::vector<KankerGlyph*>::iterator it = glyphs.begin();

  while (it != glyphs.end()) {
 
    KankerGlyph* glyph = *it;

    if (0 == glyph->segments.size()) {
      ++it;
      continue;
    }

    if (0.0f == glyph->advance_x) {
      RX_ERROR("The advance_x of the character `%c` is 0.0.", (char)glyph->charcode);
    }

    ss << "  <glyph "
       << " charcode=\"" << glyph->charcode << "\""
       << " char=\""     <<(char)glyph->charcode  << "\""
       << " advancex=\"" << glyph->advance_x  << "\""
       << ">\n";

    RX_VERBOSE("writing: %c, %lu segments", (char)glyph->charcode, glyph->segments.size());

    for (size_t i = 0; i < glyph->segments.size(); ++i) {

      ss << "    <line>\n";
      
      std::vector<vec3>& points = glyph->segments[i];
      for (size_t k = 0; k < points.size(); ++k) {
        vec3& v = points[k];
        ss << "      <p x=\"" << v.x << "\" y=\"" << v.y << "\" z=\"" << v.z << "\" />\n";
      }

      ss << "    </line>\n";
    }

    ss << "  </glyph>\n";
    ++it;
  }

  ss << "</font>";

  std::string out = ss.str();

  /* write out.*/
  ofs << out;
  ofs.flush();
  ofs.close();

  return 0;
}

void KankerFont::write(std::string str, std::vector<std::vector<vec3> >& lines) {

  float pen_x = 0.0;

  for (size_t i = 0; i < str.size(); ++i) {

    if (!hasGlyph(str[i])) {
      RX_VERBOSE("The font can't create a valid string because the character '%c' is not found.", (char)str[i]);
      continue;
    }

    KankerGlyph* g = getGlyphByCharCode(str[i]);
    if (NULL == g) {
      RX_ERROR("Cannot get glyph by char code... not supposed to happen.");
      continue;
    }

    KankerGlyph glyph = *g;
    glyph.translate(pen_x, 0);
    std::copy(glyph.segments.begin(), glyph.segments.end(), std::back_inserter(lines));
    pen_x += glyph.advance_x;
  }
}


int KankerFont::load(std::string filepath) {

  clear();

  if (0 == filepath.size()) {
    RX_ERROR("error: invalid filepath given, is empty. cannot load.");
    return -1;
  }

  std::ifstream ifs(filepath.c_str(),  std::ios::in);
  if(!ifs.is_open()) {
    RX_ERROR("error: failed to load file: %s, wrong path?", filepath.c_str());
    return false;
  }
 
  std::string xml_str;
  xml_str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

  xml_document<> doc;

  try {

    doc.parse<0>((char*)xml_str.c_str());

    xml_node<>* font = doc.first_node("font");
    if (NULL == font) {
      RX_ERROR("error: cannot find main <font> element.");
      return -2;
    }

    xml_node<>* glyph_el = font->first_node("glyph");
    if (NULL == glyph_el) {
      RX_ERROR("error: the font xml is invald, not <glyph> element found.");
      return -2;
    }

    while (glyph_el) {

      int charcode = read_attribute<int>(glyph_el, "charcode", -1);
      if (-1 == charcode) {
        RX_ERROR("error: invalid charcode, or glyph xml doesn't have a charcode (?).");
        glyph_el = glyph_el->next_sibling();
        continue;
      }

      KankerGlyph* glyph = getGlyphByCharCode(charcode);
      if (NULL == glyph) {
        RX_ERROR("error: cannot find or create the glyph for the given charcode: %d", charcode);
        glyph_el = glyph_el->next_sibling();
        continue;
      }

      glyph->advance_x = read_attribute<int>(glyph_el, "advancex", -1);
      if (-1 == glyph->advance_x) {
        RX_VERBOSE("No advance_x found in glyph, not set yet? Char: %c", (char)glyph->charcode);
      }

      /* make sure the glyph is empty. */
      glyph->clear();

      xml_node<>* line = glyph_el->first_node("line");
      if (NULL == line) {
        RX_ERROR("error: the current glyph_el does not hold any line segments.");
        glyph_el = glyph_el->next_sibling();
        continue;
      }

      while (line) {
        glyph->onStartLine();
        {
          xml_node<>* point = line->first_node("p");

          while (point) {

            vec3 v;
            v.x = read_attribute<float>(point, "x", 0.0f);
            v.y = read_attribute<float>(point, "y", 0.0f);
            v.z = read_attribute<float>(point, "z", 0.0f);

            glyph->addPoint(v);

            point = point->next_sibling();
          }
        }
        glyph->onEndLine();
        line = line->next_sibling();
      }
      glyph_el = glyph_el->next_sibling();
    }
  }
  catch (...) {
    RX_ERROR("Catched an error while parsing the font xml.");
    return -100;
  }

  return 0;
}

int KankerFont::getBaseHeight(float& height) {

  std::string chars = "xuvwz";
  for (size_t i = 0; i < chars.size(); ++i) {
    KankerGlyph* g = getGlyphByCharCode(chars[i]);
    if (NULL == g) {
      continue;
    }
    height = g->height;
    return 0;
  }

  return -1;
}

