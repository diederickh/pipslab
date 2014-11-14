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
  ,char_scale(15.0f)
  ,word_spacing(40.0f)
  ,line_height(35.0f)
  ,min_x(0)
  ,max_x(0)
  ,min_y(0)
  ,max_y(0)
  ,min_point_dist(3.0)
  ,check_abb_state_timeout(0)
  ,check_abb_state_delay(10e9)
  ,abb_reconnect_timeout(0)
  ,abb_reconnect_delay(10e9)
  ,abb_state(ABB_STATE_DISCONNECTED)
  ,abb_listener(NULL)
  ,curr_glyph_index(0)
{
  memset(read_buffer, 0x00, sizeof(read_buffer));
}

KankerAbb::~KankerAbb() {

  if (0 == sock.isConnected()) {
    sock.close();
  }
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
  float width_available = getRangeWidth();
  float height_available = getRangeHeight();
  float x_height = 0.0f;

  if (0.0f == width_available) { RX_ERROR("Range width not yet set, call init() first.");  return -1;  }
  if (0.0f == height_available) { RX_ERROR("Range height not yet set, call init() first.");  return -2;  }
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
      width_available = getRangeWidth() - word_width;
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

      /* Copy the simplified version into our abb_glyph copy before adding it to the result. */
      for (size_t k = 0; k < abb_glyph.glyph.segments.size(); ++k) {

        std::vector<vec3> simplified_segment = simplify(abb_glyph.glyph.segments[k], min_point_dist);
        if (0 == simplified_segment.size()) {
          RX_VERBOSE("After simplifying the segment we haven't got anythin left.");
          continue;
        }

        abb_glyph.segments.push_back(simplified_segment);
        segmentsOut.push_back(simplified_segment);
      }

      result.push_back(abb_glyph);
      pen_x += abb_glyph.glyph.advance_x;
    }

    /* Do we need to start the next wordt on a new line? */
    if (width_available < word_spacing) {
      has_newline = true;
      pen_x = 0;
      pen_y += line_height;
      width_available = getRangeWidth();
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

std::vector<vec3> KankerAbb::simplify(std::vector<vec3>& points, float minDist) {

  /* Points closer then `min_dist` pixels are removed */
  float min_dist_sq = minDist * minDist;
  float dist_sq;
  int to_big = 0;
  size_t total= 0;
  size_t read_dx = 1;
  std::vector<vec3> new_points;

  if (0 == points.size()) {
    return points;
  } 

  vec3& point_from = points[0];
  new_points.push_back(point_from);
  dist_sq = 0;

  while (dist_sq < min_dist_sq && read_dx < points.size()) {

      vec3& a = points[read_dx];
      vec3 d = point_from - a;
      dist_sq = dot(d, d);

      if (dist_sq >= min_dist_sq) {
        point_from = points[read_dx];
        new_points.push_back(point_from);
        dist_sq = 0;
      }

      read_dx++;
  }

  return new_points;
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
      << "  <char_scale>" << char_scale << "</char_scale>" << std::endl
      << "  <word_spacing>" << word_spacing << "</word_spacing>" << std::endl
      << "  <line_height>" << line_height << "</line_height>" << std::endl
      << "  <abb_host>" << abb_host << "</abb_host>" << std::endl
      << "  <abb_port>" << abb_port << "</abb_port>" << std::endl
      << "  <min_x>" << min_x << "</min_x>" << std::endl
      << "  <max_x>" << max_x << "</max_x>" << std::endl
      << "  <min_y>" << min_y << "</min_y>" << std::endl
      << "  <max_y>" << max_y << "</max_y>" << std::endl
      << "  <min_point_dist>" << min_point_dist << "</min_point_dist>" << std::endl
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
    read_xml<float>(cfg, "char_scale", 15.0f, char_scale);
    read_xml<float>(cfg, "word_spacing", 40.0f, word_spacing);
    read_xml<float>(cfg, "line_height", 35.0f, line_height);
    read_xml<int>(cfg, "abb_port", 1025, abb_port);
    read_xml<std::string>(cfg, "abb_host", "127.0.0.1", abb_host);
    read_xml<int>(cfg, "min_x", 0, min_x);
    read_xml<int>(cfg, "max_x", 0, max_x);
    read_xml<int>(cfg, "min_y", 0, min_y);
    read_xml<int>(cfg, "max_y", 0, max_y);
    read_xml<float>(cfg, "min_point_dist", 0, min_point_dist);

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
  RX_VERBOSE("abb.range width: %f", getRangeWidth());
  RX_VERBOSE("abb.range height: %f", getRangeHeight());
  RX_VERBOSE("abb.char_scale: %f", char_scale);
  RX_VERBOSE("abb.word_spacing: %f", word_spacing);
  RX_VERBOSE("abb.line_height: %f", line_height);
  RX_VERBOSE("abb.abb_host: %s", abb_host.c_str());
  RX_VERBOSE("abb.abb_port: %u", abb_port);
  RX_VERBOSE("abb.min_x: %d", min_x);
  RX_VERBOSE("abb.max_x: %d", max_x);
  RX_VERBOSE("abb.min_y: %d", min_y);
  RX_VERBOSE("abb.max_y: %d", max_y);
}

/* ---------------------------------------------------------------------- */

int KankerAbb::connect() {

  if (0 == abb_host.size()) {
    RX_ERROR("Cannot connect to abb because the `abb_host` member is not set.");
    return -1;
  }

  if (0 == abb_port) {
    RX_ERROR("Cannot connect to abb because the `abb_port` is not set.");
    return -2;
  }

  if (0 != sock.setListener(this)) {
    RX_ERROR("Couldn't set the socket listener.");
    return -2;
  }

  if (0 != sock.connect(abb_host, abb_port)) {
    RX_ERROR("Couldn't connect to the ABB");
    return -1;
  }

  return 0;
}

int KankerAbb::update() {

  /* When disconnected we try to reconnect every abb_reconnect_delay ns. */
  if (ABB_STATE_DISCONNECTED == abb_state) {
    uint64_t n = rx_hrtime();
    if (n > abb_reconnect_timeout) {
      if (0 != connect()) {
        RX_ERROR("After being disconnected we couldn't reconnect");
      }
      abb_reconnect_timeout = n + abb_reconnect_delay;
    }
    return 0;
  }

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
      if ('r' == read_buffer[0]) { 
        if (ABB_STATE_READY != abb_state) {

          RX_VERBOSE("Abb is ready to start drawing the next glyph.");

          if (NULL != abb_listener) {
            abb_listener->onAbbReadyToDraw();
          }
          else {
            RX_VERBOSE("We're checking the Abb state but you haven't set a listener so it doesn't really make sense.");
          }

          abb_state = ABB_STATE_READY;

          sendNextGlyph();
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

  /* Do we need to update our state? */
  /* 
     @todo when we didn't receive any data after X-calls it could be that 
     the robot stopped executing its instructions and it should be reset.
     This can happen when the robot is turned off manually or some other
     unhandled situation occurs. 
  */
  uint64_t n = rx_hrtime();
  if (n > check_abb_state_timeout) {
    check_abb_state_timeout = n + check_abb_state_delay;
    sendCheckState();
  }

  return 0;
}

void KankerAbb::onSocketConnected() {

  RX_VERBOSE("Socket connected");
  
  abb_state = ABB_STATE_CONNECTED;

  if (NULL != abb_listener) {
    abb_listener->onAbbConnected();
  }
}

void KankerAbb::onSocketDisconnected() {

  RX_ERROR("Disconnected from ABB");

  abb_state = ABB_STATE_DISCONNECTED;

  if (NULL != abb_listener) {
    abb_listener->onAbbDisconnected();
  }
}

int KankerAbb::sendCheckState() {
  buffer.clear();
  buffer.writeU8(ABB_CMD_GET_STATE);
  sock.send(buffer.ptr(), buffer.size());
  return 0;
}

int KankerAbb::sendTestPositions() {

  std::vector<vec3> positions;
  std::vector<std::vector<vec3> >segments;

  /* move in max area */
  positions.push_back(vec3(0, 0, 0));
  positions.push_back(vec3(min_x, min_y, 0));
  positions.push_back(vec3(max_x, min_y, 0));
  positions.push_back(vec3(max_x, max_y, 0));
  positions.push_back(vec3(min_x, max_y, 0));
  positions.push_back(vec3(min_x, min_y, 0));

  segments.push_back(positions);
  positions.clear();

  /* from left to right */
  positions.push_back(vec3(min_x, 0, 0));
  positions.push_back(vec3(max_x, 0, 0));
  positions.push_back(vec3(0, 0, 0));

  segments.push_back(positions);
  positions.clear();

  /* bow */
  positions.push_back(vec3(0, 0, 0));
  positions.push_back(vec3(0, min_y,0));
  positions.push_back(vec3(0, 0, 0));
  positions.push_back(vec3(0, min_y, 0));
  positions.push_back(vec3(0, 0, 0));
  positions.push_back(vec3(0, min_y, 0));
  positions.push_back(vec3(0, 0, 0));
  positions.push_back(vec3(min_x, min_y, 0));

  segments.push_back(positions);
  positions.clear();

  buffer.clear();

  for (size_t i = 0; i < segments.size(); ++i) {
    positions = segments[i];

    /* Power on a I/O */
    buffer.writeU8(ABB_CMD_IO);
    buffer.writeFloat(0);
    buffer.writeFloat(1);

    for (size_t j = 0; j < positions.size(); ++j) {
      vec3& v = positions[j];
      buffer.writeU8(ABB_CMD_POSITION);

      /* We dont need to convert the positions because the 
         coordinates we use -are- in ABB space */
      buffer.writeFloat(v.z); /* depth */
      buffer.writeFloat(v.x); /* left right */
      buffer.writeFloat(v.y); /* up/down */
      buffer.writeFloat(0.0f); /* z-rotation */
    }

    /* Power off an I/O */
    buffer.writeU8(ABB_CMD_IO);
    buffer.writeFloat(0);
    buffer.writeFloat(1);
  }

  buffer.writeU8(ABB_CMD_DRAW);

  RX_VERBOSE("Sending test, with %lu bytes.", buffer.size());
  sock.send(buffer.ptr(), buffer.size());

  return 0;
}

int KankerAbb::sendSwipePositions() {

  buffer.clear();
  
  if (0 != addSwipeToBuffer()) {
    RX_VERBOSE("Error: failed to add the swipe to the buffer");
    return -1;
  }

  RX_VERBOSE("Sending test, with %lu bytes.", buffer.size());
  sock.send(buffer.ptr(), buffer.size());
  
  return 0;
}

int KankerAbb::addSwipeToBuffer() {

  std::vector<vec3> positions;
  int num_points = 10;
  float radius = 30.0f;
  float angle = 0.0f;
  float angle_step = TWO_PI / num_points;
  float perc = 0.0f;
  size_t start_offset = buffer.size();

  RX_VERBOSE(">>>>>>>> %lu", start_offset);

  buffer.writeU8(ABB_CMD_POSITION);
  buffer.writePosition(0, min_x, min_y);

  /* Turn on the I/O */
  buffer.writeU8(ABB_CMD_IO);
  buffer.writeFloat(1);
  buffer.writeFloat(1);

  /* Move in max area */
  positions.push_back(vec3(min_x, min_y, 0));
  positions.push_back(vec3(max_x, min_y, 0));
  positions.push_back(vec3(max_x, max_y, 0));
  positions.push_back(vec3(min_x, max_y, 0));
  positions.push_back(vec3(min_x, min_y, 0));

  /* Some degrees we use to change the tcp. */
  std::vector<float> degrees;
  degrees.push_back(65);
  degrees.push_back(-65);
  degrees.push_back(65);
  degrees.push_back(-65);

  for (size_t j = 0; j < positions.size(); ++j) {
    vec3& v = positions[j];
    buffer.writeU8(ABB_CMD_POSITION);

    /* We dont need to convert the positions because the 
       coordinates we use -are- in ABB space */
    buffer.writeFloat(v.z); /* depth */
    buffer.writeFloat(v.x); /* left right */
    buffer.writeFloat(v.y); /* up/down */

    size_t dx = j % degrees.size();
    buffer.writeFloat(degrees[dx]);
  }

  /* Add a wave */
  float max_h = (max_y - min_y) * 0.4; 
  float cy = min_y + (max_y - min_y) * 0.5;
  int steps = 30.0f;
  float freq = 5.0f;

  for (int i = 0; i < steps; ++i) {
    float perc = float(i)/(steps-1);
    float x = min_x + (max_x - min_x) * perc;
    float y = sin(perc * TWO_PI * freq) * max_h;
    buffer.writeU8(ABB_CMD_POSITION);
    buffer.writeFloat(0.0f);
    buffer.writeFloat(x);
    buffer.writeFloat(y);
    buffer.writeFloat(0.0f);
  }

  /* Turn off the i/o. */
  buffer.writeU8(ABB_CMD_IO);
  buffer.writeFloat(1);
  buffer.writeFloat(0);

  /* Back to center. */
  buffer.writeU8(ABB_CMD_POSITION);
  buffer.writeFloat(0.0f);
  buffer.writeFloat(0.0f);
  buffer.writeFloat(0.0f);
  buffer.writeFloat(0.0f);

  /* Just some safety... */
  if (buffer.size() > 1024) {
    RX_ERROR("The buffer contains more then 1024 bytes. At this moment we cannot handle this. We have %lu bytes.", buffer.size());
    buffer.data.erase(buffer.data.begin() + start_offset, buffer.data.end());
    return -1;
  }

  RX_VERBOSE("After adding swipe we have %lu bytes in the buffer.", buffer.size());

  return 0;
}

int KankerAbb::sendNextGlyph() {

  buffer.clear();

  if (curr_glyph_index >= curr_message.size()) {
    if (NULL != abb_listener) {
      abb_listener->onAbbMessageReady();
    }
    return 0;
  }

  /* get the segments that make up the glyph. */
  KankerAbbGlyph& g = curr_message[curr_glyph_index];
  std::vector<std::vector<vec3> >& segments = g.segments;
  if (0 == segments.size()) {
    RX_ERROR("No semgents in current glyph.");
  }

  for (size_t j = 0; j < segments.size(); ++j) {

    std::vector<vec3>& points = segments[j];
    if (0 == points.size()) {
      RX_ERROR("No points in the segment.");
      continue;
    }


    for (size_t k = 0; k < points.size(); ++k) {
      
      vec3& v = points[k];
      vec3 p = convertFontPointToAbbPoint(v);
      buffer.writeU8(ABB_CMD_POSITION);
      buffer.writePosition(p.x, p.y, p.z);

      if (0 == k) {
        /* Power on the I/O port 0 */
        buffer.writeU8(ABB_CMD_IO);
        buffer.writeFloat(0);
        buffer.writeFloat(1);

        buffer.writeU8(ABB_CMD_POSITION);
        buffer.writePosition(p.x, p.y, p.z);
      }
    }

    /* Power off the I/O port 0 */
    buffer.writeU8(ABB_CMD_IO);
    buffer.writeFloat(0);
    buffer.writeFloat(0);

    /* 
       RAPID can only store 1024 bytes :/, therefore we transfer position data per segment
       as the are most of the time less then 1024 bytes. At this moment we're not yet
       handling buffers > 1024 in rapid. 

       @todo When we fixed handling buffers that are bigger then 1024 bytes update this comment. 
    */
    if (1024 < buffer.size()) {
      RX_ERROR("The buffer contains to much bytes, %lu", buffer.size());
    }

    RX_VERBOSE("Sending %lu bytes", buffer.size());

    sock.send(buffer.ptr(), buffer.size());
    buffer.clear();
  }


  if ((curr_glyph_index+1) >= curr_message.size()) {
    addSwipeToBuffer();
  }

  buffer.writeU8(ABB_CMD_DRAW);
    sock.send(buffer.ptr(), buffer.size());
  buffer.clear();


  curr_glyph_index++;
}

/*
  We send each glyph one at a time and only after receiving 
   a ABB_STATE_READY from the ABB. After calling `sendText()` 
   you need to make sure to call `update()` often because that 
   will process the complete message. 
*/
int KankerAbb::sendText(std::vector<KankerAbbGlyph>& message) {

  RX_VERBOSE("Requested a sendText()");

  if (0 == message.size()) {
    RX_ERROR("Trying to send an empty text. Message vector size if 0.");
    return -1;
  }

  curr_glyph_index = 0;
  curr_message = message;

  sendNextGlyph();

  return 0;
}

/* Converts/flips font points to abb points */
vec3 KankerAbb::convertFontPointToAbbPoint(vec3& v) {

  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float range_x = max_x - min_x;
  float range_y = max_y - min_y;
  float px = (v.x / range_x);
  float py = (v.y / range_y);

  if (0 != v.z) {
    // RX_VERBOSE("Currently we're using a fixed depth value of 0.");
  }

  x = min_x + px * range_x;
  y = max_y - (py * range_y);
  
  /* clamp x/y  */
  if (x < min_x) {
    x = min_x;
  }
  else if (x > max_x) {
    x = max_x;
  }

  if (y < min_y) {
    y = min_y;

  }
  else if (y > max_y) {
    y = max_y;
  }

  vec3 r(z, x, y);
  return r;
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
