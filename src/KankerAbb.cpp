#include <KankerAbb.h>

/* ---------------------------------------------------------------------- */
template <class T> int read_xml(xml_node<>* node, std::string name, T defaultval, T& result);
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
  :offset_x(0.0f)
  ,offset_y(0.0f)
  ,range_width(500.0f)
  ,range_height(500.0f)
  ,char_scale(15.0f)
  ,word_spacing(40.0f)
  ,line_height(35.0f)
{
}

KankerAbb::~KankerAbb() {
}

int KankerAbb::write(KankerFont& font, 
                     std::string str,
                     std::vector<KankerAbbGlyph>& result,
                     std::vector<std::vector<vec3> >& segmentsOut)
{
  std::stringstream ss;
  std::string word;
  bool has_newline = false;
  float pen_x = 0;
  float pen_y = 0;
  float word_width = 0;
  float width_available = range_width;
  float height_available = range_height;
  float x_height = 0.0f;

  if (0.0f == range_width) { RX_ERROR("Range width not yet set, call init() first.");  return -1;  }
  if (0.0f == range_height) { RX_ERROR("Range height not yet set, call init() first.");  return -2;  }
  if (0 == str.size()) { RX_ERROR("Trying to write to ABB but the given text size is 0.");  return -3;  }

  if (0 != result.size()) {
    RX_ERROR("It looks the given result argument already contains data. We're cleaning it!");
    result.clear();
  }

  if (0 != font.getBaseHeight(x_height)) {
    RX_ERROR("Failed to retrieve the base height for the font so we cannot normalize it.");
    return -4;
  }

  ss << str;

  while (ss >> word) {

    /* Does this word fit on the current line? */
    word_width = getWordWidth(font, word);
    float left = width_available - word_width;

    if (left > 0) {
      width_available -= word_width;
    }
    else {
      has_newline = true;
      pen_x = 0;
      pen_y += line_height;
      width_available = range_width - word_width;
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
      abb_glyph.glyph.normalize(x_height);
      abb_glyph.glyph.flipHorizontal();     
      abb_glyph.glyph.alignLeft();
      abb_glyph.glyph.scale(char_scale);
      abb_glyph.glyph.translate(pen_x + offset_x, pen_y + offset_y);

      std::copy(abb_glyph.glyph.segments.begin(),
                abb_glyph.glyph.segments.end(), 
                std::back_inserter(abb_glyph.segments));

      result.push_back(abb_glyph);

     std::copy(abb_glyph.glyph.segments.begin(),
                abb_glyph.glyph.segments.end(), 
                std::back_inserter(segmentsOut));


      pen_x += abb_glyph.glyph.advance_x;
    }

    /* Do we need to start the next wordt on a new line? */
    if (width_available < word_spacing) {
      has_newline = true;
      pen_x = 0;
      pen_y += line_height;
      width_available = range_width;
    }
    else {
      pen_x += word_spacing;
      width_available -= word_spacing;
    }

    has_newline = false;
    
    /* Do we still have enough space for a next line? */
    if ((height_available - (pen_y + line_height)) < 0) {
      RX_ERROR("Message is to big to fit in the available space. Stopping with writing now.\n");
      break;
    }
  }

  return 0;
}

int KankerAbb::saveAbbModule(std::string filepath, std::vector<KankerAbbGlyph>& message) {

  if (0 == filepath.size()) {
    RX_ERROR("Cannot save, invalid filepath.");
    return -1;
  }

  if (0 == message.size()) {
    RX_ERROR("The message size is 0.");
    return -2;
  }

  std::ofstream ofs(filepath.c_str());
  if (false == ofs.is_open()) {
    RX_ERROR("Failed to open %s for writing.", filepath.c_str());
    return -3;
  }

  /* How many floats do we store? */
  size_t num_floats = 0;
  for (size_t i = 0; i < message.size(); ++i) {
    std::vector<std::vector<vec3> >& segments = message[i].segments;
    for (size_t j = 0; j < segments.size(); ++j) {
      num_floats += segments[j].size();
    }
  }

  /* Write the Glyph record. */
  ofs << "RECORD Glyph" << std::endl
      << "  num command;" << std::endl
      << "  num start_index;" << std::endl
      << "  num num_elements;" << std::endl
      << "ENDRECORD" << std::endl
      << std::endl;
  
  /* Write one huge array with all the points. */
  ofs << "CONST num points{" << num_floats << "} := [";
  for (size_t i = 0; i < message.size(); ++i) {
    std::vector<std::vector<vec3> >& segments = message[i].segments;
    for (size_t j = 0; j < segments.size(); ++j) {
      std::vector<vec3>& points = segments[j];
      for (size_t k = 0; k < points.size(); ++k) {
        vec3& v = points[k];
        ofs << v.x << ", " << v.y << "," << v.z;
        if (k < (points.size() - 1)) {
          ofs << ",";
        }
      }
    }
  }
  ofs << "];"
      << std::endl 
      << std::endl;

  /* Write the glyph array. */
  size_t offset = 0;;
  size_t count = 0;
  ofs << "CONST Glyph glyphs{" << message.size() << "} := [";
  for (size_t i = 0; i < message.size(); ++i) {
    std::vector<std::vector<vec3> >& segments = message[i].segments;
    count = 0;
    for (size_t j = 0; j < segments.size(); ++j) {
      std::vector<vec3>& points = segments[j];
      count += points.size();
    }

    ofs << "0,"          /* command */
        << offset << "," /* start index */
        << count;        /* number of floats/elements. */

    if (i < (message.size() - 1)) {
      ofs << ", ";
    }

    offset += count;
  }

  ofs << "];";
  ofs.close();

  return 0;
}

float KankerAbb::getWordWidth(KankerFont& font, std::string word) {

  float width = 0.0f;
  float x_height = 0.0f;

  if (0 == word.size()) {
    return 0.0f;
  }

  if (0 != font.getBaseHeight(x_height)) {
    RX_ERROR("Failed to retrieve the base height for the font so we cannot normalize it.");
    return -4;
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
    glyph.normalize(x_height);
    width += glyph.advance_x;
  }

  width *= char_scale;

  return width;
}

int KankerAbb::saveSettings(std::string filepath) {

  if (0 == filepath.size()) {
    RX_ERROR("Invalid filepath (empty).");
    return -1;
  }

  std::ofstream ofs(filepath.c_str(), std::ios::out);
  if (!ofs.is_open()) {
    RX_ERROR("Failed to open: %s", filepath.c_str());
    return -2;
  }
  
  ofs << "<config>" << std::endl
     << "  <offset_x>" << offset_x << "</offset_x>" << std::endl
     << "  <offset_y>" << offset_y << "</offset_y>" << std::endl 
     << "  <range_width>" << range_width << "</range_width>" << std::endl
     << "  <range_height>" << range_height << "</range_height>" << std::endl
     << "  <char_scale>" << char_scale << "</char_scale>" << std::endl
     << "  <word_spacing>" << word_spacing << "</word_spacing>" << std::endl
     << "  <line_height>" << line_height << "</line_height>" << std::endl
     << "  <ftp_url>" << ftp_url << "</ftp_url>" << std::endl
     << "</config>";

  ofs.close();
  
  return 0;
}

int KankerAbb::loadSettings(std::string filepath) {
  
  if (!rx_file_exists(filepath)) {
    RX_ERROR("Cannot find %s", filepath.c_str());
    return -1;
  }

  std::ifstream ifs(filepath.c_str(), std::ios::in);
  if(!ifs.is_open()) {
    RX_ERROR("Cannot open the settings file.");
    return -2;
  }
 
  std::string xml_str;
  xml_str.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

  if (0 == xml_str.size()) {
    RX_ERROR("Settings file is empty.");
    return -3;
  }

  xml_document<> doc;

  try {

    doc.parse<0>((char*)xml_str.c_str());
    
    xml_node<>* cfg = doc.first_node("config");
    read_xml<float>(cfg, "offset_x", 0, offset_x);
    read_xml<float>(cfg, "offset_y", 0, offset_y);
    read_xml<float>(cfg, "range_width", 500, range_width);
    read_xml<float>(cfg, "range_height", 500, range_height);
    read_xml<float>(cfg, "char_scale", 15.0f, char_scale);
    read_xml<float>(cfg, "word_spacing", 40.0f, word_spacing);
    read_xml<float>(cfg, "line_height", 35.0f, line_height);
    read_xml<std::string>(cfg, "ftp_url", "", ftp_url);

    print();
  }
  catch (...) {
    RX_ERROR("Caught xml exception.");
    return -4;
  }
  return 0;
}

void KankerAbb::print() {

  RX_VERBOSE("abb.offset_x: %f", offset_x);
  RX_VERBOSE("abb.offset_y: %f", offset_y);
  RX_VERBOSE("abb.range_width: %f", range_width);
  RX_VERBOSE("abb.range_height: %f", range_height);
  RX_VERBOSE("abb.char_scale: %f", char_scale);
  RX_VERBOSE("abb.word_spacing: %f", word_spacing);
  RX_VERBOSE("abb.line_height: %f", line_height);
  RX_VERBOSE("abb.ftp_url: %s", ftp_url.c_str());
}

/* ---------------------------------------------------------------------- */

template <class T> int read_xml(xml_node<>* node, std::string name, T defaultval, T& result) {

  result = defaultval;

  if (0 == name.size()) {
    RX_ERROR("name.size() == 0.");
    return -1;
  }

  if (NULL == node) {
    RX_ERROR("Container node for %s is NULL.", name.c_str());
    return -2;
  }

  xml_node<>* el = node->first_node(name.c_str());
  if (NULL == el) {
    RX_ERROR("%s not found in %s", name.c_str(), node->name());
    return -3;
  }

  std::stringstream ss;
  ss << el->value();
  ss >> result;
    
  return 0;
}
