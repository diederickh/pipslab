#include <kanker/KankerAbb.h>

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

  ,check_abb_state_timeout(0)
  ,check_abb_state_delay(10e9)
  ,abb_state(ABB_STATE_UNKNOWN)
  ,abb_listener(NULL)
{
  memset(read_buffer, 0x00, sizeof(read_buffer));
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

int KankerAbb::saveAbbModule(std::string filepath, int64_t id, std::vector<KankerAbbGlyph>& message) {

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

  /* 
     How many floats do we store? 

     - Each glyph can contain multiple line segments so we loop 
       over each line segment and count the number of points per 
       segment. 

     - For every glyph we need to add 2 extra points. The first one 
       is at the start of a segment which indicates that the I/O
       port needs to be turned ON and after each segment another 
       one to turn the I/O port OFF. 

  */

  std::vector<std::string> rapid_points;

  for (size_t i = 0; i < message.size(); ++i) {

    /* get the segments that make up the glyph. */
    std::vector<std::vector<vec3> >& segments = message[i].segments;
    
    for (size_t j = 0; j < segments.size(); ++j) {

      std::vector<vec3>& points = segments[j];

      for (size_t k = 0; k < points.size(); ++k) {

        vec3& point = points[k];
        //point *= 5;
        //point.y += 650;
        point.z = 660;
        //point.x += 0;
        point.print();

        if (k == 0) {
          /* start of segment. */
          std::stringstream ss; 
          std::string point_str; 
          //          ss << "[" << point.x <<"," << point.y << "," << point.z << ",0]";
          ss << "[" << (point.z) <<"," << point.x << "," << point.y << ",0]\n";
          point_str = ss.str();
          rapid_points.push_back(point_str);
        }
        
        /* point */
        std::stringstream ss; 
        std::string point_str; 
        //        ss << "[" << point.x <<"," << point.y << "," << point.z << ",1]";
        ss << "[" << ( point.z) <<"," << point.x << "," << point.y << ",1]\n";
        point_str = ss.str();
        rapid_points.push_back(point_str);

        if (k == points.size() - 1) {
          /* end of segment. */
          std::stringstream ss; 
          std::string point_str; 
          //          ss << "[" << point.x <<"," << point.y << "," << point.z << ",0]";
          ss << "[" << ( point.z) <<"," << point.x << "," << point.y << ",0]\n";
          point_str = ss.str();
          rapid_points.push_back(point_str);
        }
      }
    }
  } /* for message */

  ofs << "MODULE mTEXT" << std::endl << std::endl;

  /* Write the Glyph record. */
  //  << "PERS Bool bNewModule := TRUE;" << std::endl
  ofs << "PERS num nMessageId := " << id << ";" << std::endl
      << "PERS num nTotalPoints := " << rapid_points.size() << ";" << std::endl
      << std::endl;
  
  /* Write one huge array with all the points. */
  ofs << "PERS num nXYZ_Value{" << rapid_points.size() << ",4} := [";
  for (size_t i = 0; i < rapid_points.size(); ++i) {
    ofs << rapid_points[i];
    if (i < rapid_points.size() - 1) {
      ofs << ",";
    }
  }

  ofs << "];";

  /* Write the procedure */
  ofs << std::endl 
      << std::endl;

  ofs << "PROC Calculate()" << std::endl
      << "  nTotalPoints_Robot := nTotalPoints;" << std::endl
      << "  FOR i from 1 to nTotalPoints DO" << std::endl
      << "      nXYZ_Value_Robot{i, 1} := nXYZ_Value{i,1};" << std::endl
      << "      nXYZ_Value_Robot{i, 2} := nXYZ_Value{i,2};" << std::endl
      << "      nXYZ_Value_Robot{i, 3} := nXYZ_Value{i,3};" << std::endl
      << "      nXYZ_Value_Robot{i, 4} := nXYZ_Value{i,4};" << std::endl
      << "  ENDFOR" << std::endl
    //      << "  nXYZ_Value_Robot{" << rapid_points.size() << ",4} := nXYZ_Value{" << rapid_points.size() << ",4};" << std::endl
    //      << "  bNewModule := FALSE; " << std::endl
      << "  nMessageId_Robot := nMessageId;" << std::endl
      << "ENDPROC" << std::endl
      << std::endl
      << "ENDMODULE";

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

int KankerAbb::connect() {

  if (0 != sock.connect("127.0.0.1", 1025)) {
    RX_ERROR("Couldn't connect to the ABB");
    return -1;
  }

  return 0;
}

int KankerAbb::processIncomingData() {

  if (0 != sock.isConnected()) {
    RX_ERROR("We're not connected to the ABB.");
    return -1;
  }

  /* Check if there is data on the socket. */
  if (0 == sock.canRead(0, 100)) {
    int nread = sock.read(read_buffer, sizeof(read_buffer));
    if (0 > nread) {
      RX_ERROR("Got an error while trying to read from socket, we're probably disconnected: %d", nread);
      return -2;
    }
    else if (0 < nread) {
      RX_VERBOSE("Read: %d bytes, %c", nread, read_buffer[0]);

      /* r = ready to accept new commands, c = client is connected, waiting for commands. */
      if ('r' == read_buffer[0] || 'c' == read_buffer[0]) {
        if (ABB_STATE_READY != abb_state) {
          RX_VERBOSE("Abb is ready to start drawing!");
          if (NULL != abb_listener) {
            abb_listener->onAbbReadyToDraw();
          }
          else {
            RX_VERBOSE("We're checking the Abb state but you haven't set a listener so it doesn't really make sense.");
          }
          abb_state = ABB_STATE_READY;
        }
      }
      else if ('d' == read_buffer[0]) {
        RX_VERBOSE("Abb is drawing");
        if (NULL != abb_listener) {
          abb_listener->onAbbDrawing();
        }
        abb_state = ABB_STATE_DRAWING;
      }
    }
  }
  else {                                        
  }

  /* Do we need to update our state? */
  uint64_t n = rx_hrtime();
  if (n > check_abb_state_timeout) {
    RX_VERBOSE("Check state.");
    check_abb_state_timeout = n + check_abb_state_delay;
    sendCheckState();
  }
  return 0;
}

int KankerAbb::sendCheckState() {
  buffer.clear();
  buffer.writeU8(ABB_CMD_GET_STATE);
  sock.send(buffer.ptr(), buffer.size());
  return 0;
}
int KankerAbb::sendTestData() {
  return 0;
}

int KankerAbb::sendPosition(float x, float y, float z) {
  buffer.clear();
  buffer.writeU8(ABB_CMD_POSITION);
  buffer.writePosition(z, x, y); /* We flip the coordinates. */
  sock.send(buffer.ptr(), buffer.size());
  return 0;
}

int KankerAbb::sendResetPacketIndex() {
  buffer.clear();
  buffer.writeU8(ABB_CMD_RESET_PACKET_INDEX);
  sock.send(buffer.ptr(), buffer.size());
  return 0;
}

int KankerAbb::sendDraw() {
  buffer.clear();
  buffer.writeU8(ABB_CMD_DRAW);
  sock.send(buffer.ptr(), buffer.size());
  return 0;
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
