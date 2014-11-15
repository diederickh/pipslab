#include <kanker/KankerApp.h>
#include <GLFW/glfw3.h>

/* ------------------------------------------------------------------------------------ */

static void on_add_characters_clicked(int id, void* user);
static void on_font_test_clicked(int id, void* user);
static void on_font_save_clicked(int id, void* user);
static void on_font_load_clicked(int id, void* user);
static void on_font_file_selected(int selectid, int optionid, void* user);
static void on_abb_load_settings_clicked(int id, void* user);
static void on_abb_save_settings_clicked(int id, void* user);
static void on_abb_test_upload_clicked(int id, void* user);
static void on_abb_send_message_to_robot_clicked(int id, void* user);
static void on_abb_send_test_clicked(int id, void* user);
static void on_abb_send_swipe_clicked(int id, void* user);

/* ------------------------------------------------------------------------------------ */

KankerApp::KankerApp() 
  :state(KSTATE_NONE)
  ,kanker_glyph(NULL)
  ,is_mouse_pressed(false)
  ,is_mouse_inside_char(false)
  ,is_mouse_inside_advance(false)
  ,gui_width(314)
  ,selected_font_dx(0)
  ,glyph_dx(0)
  ,mouse_down_x(0)
  ,mouse_down_y(0)
  ,char_offset_x(0)
  ,char_offset_y(0)
  ,advance_x(0)
  ,origin_x(gui_width)
  ,origin_y(0) /* @todo make use of the origin_y */
  ,gui(NULL)
{
}

KankerApp::~KankerApp() {

  if (NULL != gui) {
    delete gui;
  }

  gui = NULL;
}

int KankerApp::init() {
  
  /* title font */
  std::string font = "appfont.otf";
  if (0 != title_font.open(rx_to_data_path(font), 60)) {
    RX_ERROR("error: cannot load the font.");
    exit(EXIT_FAILURE);
  }

  title_font.color(1.0f, 1.0f, 1.0f, 1.0f);
  title_font.alignCenter();

  /* info font */
  if (0 != info_font.open(rx_to_data_path(font), 16)) {
    RX_ERROR("error: cannot load the font.");
    exit(EXIT_FAILURE);
  }
  info_font.color(1.0f, 1.0f, 1.0f, 1.0f);

  /* verbose font */
  if (0 != verbose_font.open(rx_to_data_path(font), 16)) {
    RX_ERROR("error: cannot load the font.");
    exit(EXIT_FAILURE);
  }
  verbose_font.color(0.0f, 0.7f, 0.1f, 1.0f);

  switchState(KSTATE_HOME);

  /* create the GUI. */
  if (0 != createGui()) {
    RX_ERROR("error: failed to create the gui.");
    exit(EXIT_FAILURE);
  }

  /* init the drawer. */
  if (0 != tiny_drawer.init(1024, 768, painter.width(), painter.height())) {
    RX_ERROR("error: failed to initialize the drawer.");
    return -1;
  }
  
  int pw = 1024;
  int ph = 768;
  if (0 != preview_drawer.init(pw, ph, painter.width(), painter.height())) {
    RX_ERROR("error: failed to initialize the preview drawer.");
    return -1;
  }

  /* TEST LOAD FONT */
  kanker_font.setOrigin(origin_x, origin_y);

  std::string test_font = rx_to_data_path("fonts/roxlu.xml");
  if (0 != kanker_font.load(test_font)) {
    RX_ERROR("Cannot load: %s", test_font.c_str());
  }

  /* Init the ABB interface. */
  //  kanker_abb.range_width = 500;
  //  kanker_abb.range_height = 500;

  /* Setup the controller that we use to test the ABB communication */
  KankerAbbControllerSettings cfg;
  cfg.font_file = rx_to_data_path("fonts/roxlu.xml");
  cfg.settings_file = rx_to_data_path("abb_settings.xml");
  if (0 != controller.init(cfg, this)) {
    RX_ERROR("Cannot initialize the controller.");
  }

  test_message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque ac fermentum ";
  //test_message = "roger abb";
  //test_message = "ik sta op tegen kanker ik sta op tegen kanker.";
  //test_message = "He Martin, gefeliciteerd met je verjaardag!";
  //test_message = "Dearest Tina, we both have a lot of emotions for you";
  //test_message = "Keez Duyves";
  //test_message = "i love ";
  //test_message = "diederick";
  //test_message = "Martin gefeliciteerd!";
  //test_message = "ipsum";
  //test_message = "d d d a a";
  //test_message = "aaaaaa";
  //test_message = "bbbbbb";
  //test_message = "x";
  //test_message = "a";
  //test_message = "Lieve papa, we denken aan je.";
  //test_message = "Ik sta op tegen kanker voor mijn lieve schoonvader";
  test_message = "Mijn, mijn, Lieve papa, we denken aan je.";


  /* Force a load for the settings. */
  on_abb_load_settings_clicked(0, this);

  return 0;
}

int KankerApp::createGui() {

  /* Main panel.*/
  gui = new Panel(new RenderGL(), painter.height() - 20, GUI_STYLE_NONE);
  if (NULL == gui) {
    RX_ERROR("Cannot allocate the gui.");
    return -1;
  }

  gui->setPosition(painter.width() - (gui->w + 30), 8);
  gui->lockPosition();
  
  /* Font options. */
  Group* group_font = gui->addGroup("Font", GUI_STYLE_NONE);
  if (NULL == group_font) {
    RX_ERROR("Failed to allocate a new gui group.");
    return -2;
  }

  group_font->add(new Button("Add character to loaded font",  0, GUI_ICON_FONT, on_add_characters_clicked, this,  GUI_STYLE_NONE));
  group_font->add(new Button("Show test message",  0, GUI_ICON_FONT, on_font_test_clicked, this,  GUI_STYLE_NONE));
  group_font->add(new Button("Send test message to robot", 0, GUI_ICON_UPLOAD, on_abb_send_message_to_robot_clicked, this, GUI_STYLE_NONE));
  group_font->add(new Button("Send test positions to robot",  0, GUI_ICON_UPLOAD, on_abb_send_test_clicked, this,  GUI_STYLE_NONE));
  group_font->add(new Button("Send SWIPE to robot",  0, GUI_ICON_UPLOAD, on_abb_send_swipe_clicked, this,  GUI_STYLE_NONE));

  /* Saving */
  Group* group_save = gui->addGroup("Save font", GUI_STYLE_NONE);
  if (NULL == group_save) {
    RX_ERROR("Failed to allocate a new gui group");
    return -3;
  }

  group_save->add(new Text("Filename", font_filename, 135));
  group_save->add(new Button("Save",  0, GUI_ICON_FLOPPY_O, on_font_save_clicked, this, GUI_STYLE_NONE));
  
  /* Loading */
  {
    std::vector<std::string> fonts;
    if (0 == getFontFiles(fonts)) {
      Group* group_load = gui->addGroup("Load font", GUI_STYLE_NONE);
      if (NULL == group_load) {
        RX_ERROR("Failed to allocate the load gui group");
        return -4;
      }

      Select* sel = new Select("Load file", 1, fonts, on_font_file_selected, this, GUI_STYLE_NONE);
      sel->setDirection(GUI_DIRECTION_UP);
      group_load->add(sel);
      group_load->add(new Button("Load",  0, GUI_ICON_FOLDER_OPEN, on_font_load_clicked, this, GUI_STYLE_NONE));
    }
  }

  /* Abb settings. */
  Group* group_abb = gui->addGroup("Abb", GUI_STYLE_NONE);
  if (NULL == group_abb) {
    RX_ERROR("Failed to allocate the abb gui group");
    return -5;
  }

  group_abb->add(new Slider<float>("ABB.offset_x", kanker_abb.offset_x, -1300, 1300, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<float>("ABB.offset_y", kanker_abb.offset_y, -1300, 1300, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<float>("ABB.char_scale", kanker_abb.char_scale, 0, 1000, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<float>("ABB.line_height", kanker_abb.line_height, 0, 1000, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<float>("ABB.word_spacing", kanker_abb.word_spacing, 0, 1000, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<int>("ABB.min_x", kanker_abb.min_x, -15000, 15000, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<int>("ABB.max_x", kanker_abb.max_x, -15000, 15000, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<int>("ABB.min_y", kanker_abb.min_y, -15000, 15000, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<int>("ABB.max_y", kanker_abb.max_y, -15000, 15000, 1, GUI_STYLE_NONE));
  group_abb->add(new Slider<float>("ABB.min_point_dist", kanker_abb.min_point_dist, 1.0, 50.0, 0.5, GUI_STYLE_NONE));
  group_abb->add(new Text("ABB.host", kanker_abb.abb_host));
  group_abb->add(new Slider<int>("ABB.port", kanker_abb.abb_port, 0, 999999, 1, GUI_STYLE_NONE));
  group_abb->add(new Button("Save ABB Settings", 0, GUI_ICON_FLOPPY_O, on_abb_save_settings_clicked, this, GUI_STYLE_NONE));
  group_abb->add(new Button("Load ABB Settings", 0, GUI_ICON_REFRESH, on_abb_load_settings_clicked, this, GUI_STYLE_NONE));
  return 0;
}

/* Returns 0 when we found some font files, otherwise < 0. */
int KankerApp::getFontFiles(std::vector<std::string>& files) {

  std::vector<std::string> full_paths;
  full_paths = rx_get_files(rx_to_data_path("fonts"), "xml");
  if (0 == full_paths.size()) {
    return -1;
  }

  for (size_t i = 0; i < full_paths.size(); ++i) {
    files.push_back(rx_strip_dir(full_paths[i]));
  }

  return 0;
}

void KankerApp::update() {

  /* 
     Not ideal, but the Font Editing app evolved from a pure 
     font editing/creation application to one which also has support
     to send the coordinates of the current font directly to the robot.
     This was easy for testing the font.

     The KankerAbbController instance is used to handle all 
     communication with the robot adn therefore we need to copy 
     some of the properties that are controlled by the gui to the 
     controller. 
  */
  controller.kanker_abb.offset_x = kanker_abb.offset_x;
  controller.kanker_abb.offset_y = kanker_abb.offset_y;
  controller.kanker_abb.line_height = kanker_abb.line_height;
  controller.kanker_abb.char_scale = kanker_abb.char_scale;
  controller.kanker_abb.min_x = kanker_abb.min_x;
  controller.kanker_abb.max_x = kanker_abb.max_x;
  controller.kanker_abb.min_y = kanker_abb.min_y;
  controller.kanker_abb.max_y = kanker_abb.max_y;
  
  controller.update();
}

void KankerApp::draw() {

  switch (state) {
    case KSTATE_HOME:               { drawStateHome();             break;    }
    case KSTATE_CHAR_INPUT_TITLE:   { drawStateCharInputTitle();   break;    }
    case KSTATE_CHAR_INPUT_DRAWING: { drawStateCharInputDrawing(); break;    }
    case KSTATE_CHAR_EDIT:          { drawStateCharEdit();         break;    }
    case KSTATE_CHAR_PREVIEW:       { drawStateCharPreview();      break;    }
    case KSTATE_CHAR_OVERVIEW:      { drawStateCharOverview();     break;    }
    case KSTATE_FONT_TEST:          { drawStateFontTest();         break;    } 
    default: {
      RX_ERROR("error: invalid state: %d", state);
      break;
    }
  };
}

void KankerApp::drawStateHome() {
  drawGui();
}

void KankerApp::drawStateFontTest() {

  if (0 != kanker_font.size()) {
    std::vector<KankerAbbGlyph> result;
    std::vector<std::vector<vec3> > points;
    kanker_abb.write(kanker_font, test_message, result, points);
    preview_drawer.updateVertices(points);
    preview_drawer.drawLines();
  }

  painter.hex("00FF00");
  painter.line(0.0, kanker_abb.line_height, painter.width(), kanker_abb.line_height);
  painter.draw();

  drawGui();
}

void KankerApp::drawStateCharInputTitle() {
  title_font.draw(painter.width() / 2 - gui_width / 2, painter.height() / 2 - 30);
  drawGui();
}

void KankerApp::drawStateCharInputDrawing() {

  info_font.draw(10, 10);
  verbose_font.draw(10, painter.height() - 30);
  painter.clear();

  if (kanker_glyph == NULL) {
    return;
  }

  /* Draw the baseline. */
  drawHelperLines();

  /* Draw the input lines. */
  painter.hex("FFFFFF");
  painter.hex("666666");
  drawGlyphAsLine(kanker_glyph, 0, 0);

  painter.draw();
  drawGui();
}

void KankerApp::drawGlyphAsLine(KankerGlyph* glyph, float offsetX, float offsetY) {

  for (size_t i = 0; i < glyph->segments.size(); ++i) {

    std::vector<vec3>& seg = glyph->segments[i];
    if (seg.size() < 2) {
      continue;
    }

    for (size_t j = 0; j < seg.size() - 1; ++j) {
      vec3& a = seg[j];
      vec3& b = seg[j + 1];
      painter.line(a.x + offsetX, a.y + offsetY, b.x + offsetX, b.y + offsetY);
    }
  }
}

void KankerApp::drawHelperLines() {
  
  float y = 0.0f;

  /* baseline */
  painter.hex("FF0000");
  painter.line(0.0, painter.height() - gui_width, painter.width(), painter.height() - gui_width);

  /* mean line */
  painter.hex("DDDDDD");
  y = painter.height() - (gui_width + 250);
  painter.line(0.0, y, painter.width(), y);

  /* descender line. */
  painter.hex("333333");
  y = painter.height() - (gui_width - 50);
  painter.line(0.0, y, painter.width(), y);

  /* ascender line */
  y = painter.height() - (gui_width + 200);
  painter.line(0.0, y, painter.width(), y);

  painter.hex("333333");
  y = painter.height() - (gui_width - 100);
  painter.line(0.0, y, painter.width(), y);

  /* origin vertical line. */
  painter.hex("DDDDDD");
  painter.line(origin_x, 0, origin_x, painter.height());

  painter.hex("DDDDDD");
  painter.line(gui_width + 250, 0, gui_width + 250, painter.height());

  /* origin vertical line. */
  painter.hex("333333");
  painter.line(gui_width + 100, 0, gui_width + 100, painter.height());

  /* origin point. */
  painter.hex("5D098F");
  painter.fill();
  painter.circle(gui_width, painter.height() - gui_width, 3);
}

void KankerApp::drawStateCharEdit() {
  
  /* Base helper lines. */
  drawHelperLines();

  /* The advance-x marker. */
  if (is_mouse_inside_advance) {
    painter.hex("79BD8F");

  }
  else {
    painter.hex("00A388");
  }

  advance_x = CLAMP(advance_x, gui_width, painter.width() - gui_width);
  painter.line(advance_x, 0, advance_x, painter.height());
  painter.rect(gui_width,  0, (advance_x - gui_width), 50); 

  if (is_mouse_inside_char) {
    painter.hex("FFFFFF");
  }
  else {
    painter.hex("666666");
  }

  drawGlyphAsLine(kanker_glyph, char_offset_x, char_offset_y);

  painter.draw();
  
  drawGui();

  info_font.draw(10, painter.height() - 30);
}

void KankerApp::drawStateCharPreview() {

  preview_drawer.update();
  preview_drawer.renderAndDraw(0, 0);
  drawGui();
}

void KankerApp::drawStateCharOverview() {

  tiny_drawer.renderAndDraw(0, 0);
  drawGui();
  info_font.draw(10, painter.height() - 30);
}

void KankerApp::drawGui() {

  painter.clear();
  painter.fill();
  painter.rgba(28, 29, 33);
  painter.rgba(49, 53, 61);
  painter.rgba(238, 239, 247);
  painter.rgba(72, 74, 71);
  painter.rect(painter.width() - gui_width, 0, gui_width, painter.height());

  painter.draw();
  painter.nofill();

  if (NULL != gui) {
    gui->draw();
  }
}

void KankerApp::switchState(int newstate) {

  if (newstate == state) {
    RX_VERBOSE("warning: trying to switch to the same state? %d", state);
  }

  state = newstate;

  switch (state) {
    case KSTATE_CHAR_INPUT_TITLE: {
      title_font.write("Type a character to record:");  
      break;
    }
    case KSTATE_CHAR_INPUT_DRAWING: {
      info_font.write("Drag with the mouse to add points to the character. "
                      "Press backspace to restart. Space when happy with the character.");
      verbose_font.write((char)kanker_glyph->charcode);
      break;
    }
    case KSTATE_CHAR_EDIT: {
      info_font.write("Position the origin (dot) and set advance-x. Press space when ready.");
      if (kanker_glyph) {

        if (kanker_glyph->advance_x == 0.0f) {
          /* Auto calc advance x. (just the width). */
          advance_x = kanker_glyph->min_x + kanker_glyph->width;
          advance_x = CLAMP(advance_x, gui_width, painter.width() - gui_width);
        }
        else {
          advance_x = gui_width + kanker_glyph->advance_x;
        }

        /* Set the initial (or loaded) advance_x on the glyph. */
        kanker_glyph->advance_x = (advance_x - gui_width);
        kanker_glyph->origin_x = origin_x;
      }
      break;
    }
    case KSTATE_CHAR_PREVIEW: {
      if (kanker_glyph) {

        if (0.0f == kanker_glyph->advance_x) {
          RX_ERROR("The glyph advance_x is 0.0 expect incorrect results..");
        }

        KankerGlyph copy = *kanker_glyph;
        preview_drawer.updateVertices(copy);
      }
      else {
        RX_WARNING("Changing to preview state, but the glyph is NULL.");
      }
      break;
    }
    case KSTATE_CHAR_OVERVIEW: {
      glyph_dx = -1;
      onKeyRelease(GLFW_KEY_RIGHT, 0, 0);
      info_font.write("Press left and right arrows to switch character.");
      break;
    }
    default: {
      break;
    }
  }
}

void KankerApp::onChar(unsigned int key) {

  if (NULL != gui) {
    gui->onCharPress(key);
  }

  switch (state) {
    case KSTATE_CHAR_INPUT_TITLE: {
      kanker_glyph = kanker_font.getGlyphByCharCode(key);
      if (NULL == kanker_glyph) {
        RX_ERROR("error: not supposed to happen, but the font couldn't create of find a the glyph.");
        return;
      }
      switchState(KSTATE_CHAR_INPUT_DRAWING);
      break;
    }
    default: {
      break;
    }
  }
}

void KankerApp::onKeyRelease(int key, int scancode, int mods) {

  if (NULL != gui) {
    gui->onKeyRelease(key, mods);
  }

  switch (state) {
    case KSTATE_CHAR_INPUT_DRAWING: {
      if (GLFW_KEY_BACKSPACE == key) {
        if (NULL != kanker_glyph) {
          kanker_glyph->clear();
        }
      }
      else if (GLFW_KEY_SPACE == key) {
        switchState(KSTATE_CHAR_EDIT);
      }
      break;
    }
    case KSTATE_CHAR_EDIT: {
      if (GLFW_KEY_SPACE == key) {
        switchState(KSTATE_CHAR_PREVIEW);
      }
      break;
    }
    case KSTATE_CHAR_PREVIEW: { 
      if (GLFW_KEY_SPACE == key) {
        switchState(KSTATE_CHAR_INPUT_TITLE);
      }
      break;
    }
    case KSTATE_CHAR_OVERVIEW: {
      if (0 != kanker_font.glyphs.size()) {
        if (GLFW_KEY_RIGHT == key) {
          ++glyph_dx %= kanker_font.size();
          KankerGlyph* glyph = kanker_font.getGlyphByIndex(glyph_dx);
          if (NULL != glyph) {
            tiny_drawer.updateVertices(*glyph);
            break;
          }
        }
        else if (GLFW_KEY_LEFT == key) {
          if (0 == glyph_dx) {
            glyph_dx = kanker_font.size() - 1;
          }
          else {
            glyph_dx--;
          }
          KankerGlyph* glyph = kanker_font.getGlyphByIndex(glyph_dx);
          if (NULL != glyph) {
            tiny_drawer.updateVertices(*glyph);
            break;
          }
        }
      } /* 0 != kanker_font.glyphs.size() */
      break;
    }
    default: {
      break;
    }
  }
}

void KankerApp::onKeyPress(int key, int scancode, int mods) {

  if (NULL != gui) {
    gui->onKeyPress(key, mods);
  }
}

void KankerApp::onResize(int w, int h) {
  painter.resize(w, h);
}

void KankerApp::onMouseMove(double x, double y) {

  if (NULL != gui) {
    gui->onMouseMove(x, y);
  }

  switch (state) {
    case KSTATE_CHAR_INPUT_DRAWING: {
      if (is_mouse_pressed) {
        if (x < (painter.width() - gui_width)) {
          kanker_glyph->addPoint(x, y, 0.0f);
        }
      }
      break;
    }
    case KSTATE_CHAR_EDIT: {
      if (kanker_glyph) {
        
        if (is_mouse_inside_char && is_mouse_pressed) {
          char_offset_x = x - mouse_down_x;
          char_offset_y = y - mouse_down_y;
          return;
        }

        if (IS_INSIDE(x, y, kanker_glyph->min_x, kanker_glyph->min_y, kanker_glyph->width, kanker_glyph->height)) {
          is_mouse_inside_char = true;
        }
        else { 
          is_mouse_inside_char = false;
        }

        if (is_mouse_inside_advance && is_mouse_pressed) {
          advance_x = x;
        }
        
        if (IS_INSIDE(x, y, gui_width, 0, (advance_x - gui_width), 50)) {
          is_mouse_inside_advance = true;
        }
        else {
          is_mouse_inside_advance = false;
        }
      }
      break;
    }
    default: { 
      break;
    }
  }
}

void KankerApp::onMousePress(double x, double y, int bt, int mods) {

  is_mouse_pressed = true;

  if (NULL != gui) {
    gui->onMousePress(x, y, bt, mods);
  }

  switch (state) {
    case KSTATE_CHAR_INPUT_DRAWING: {
      if (x < (painter.width() - gui_width)) {
        kanker_glyph->onStartLine();
        kanker_glyph->addPoint(x, y, 0.0f);
      }
      break;
    }
    case KSTATE_CHAR_EDIT: { 
      if (IS_INSIDE(x, y, kanker_glyph->min_x, kanker_glyph->min_y, kanker_glyph->width, kanker_glyph->height)) {
        is_mouse_inside_char = true;
        mouse_down_x = x;
        mouse_down_y = y;
      } 
      else {
        is_mouse_inside_char = false;
      }

      break;
    }
    default: { 
      break;
    }
  }
}

void KankerApp::onMouseRelease(double x, double y, int bt, int mods) {

  is_mouse_pressed = false;

  if (NULL != gui) {
    gui->onMouseRelease(x, y, bt, mods);
  }

  switch (state) {
    case KSTATE_CHAR_INPUT_DRAWING: {
      kanker_glyph->onEndLine();
      break;
    }
    case KSTATE_CHAR_EDIT: {
      if (is_mouse_inside_char) {
        if (kanker_glyph) {
          kanker_glyph->translate(char_offset_x, char_offset_y);
        }
        is_mouse_inside_char = false;
        char_offset_x = 0.0f;
        char_offset_y = 0.0f;
      }

      if (kanker_glyph) {
        kanker_glyph->advance_x = advance_x - gui_width;
        RX_VERBOSE("advance_x: %f", kanker_glyph->advance_x);
      }
      break;
    }
    default: { 
      break;
    }
  }
}

/* ------------------------------------------------------------------------------------ */

static void on_add_characters_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("error: cannot cast to KankerApp*.");
    return;
  }

  app->switchState(KSTATE_CHAR_INPUT_TITLE);
}

static void on_font_test_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("error: cannot cast to KankerApp*.");
    return;
  }

  app->switchState(KSTATE_FONT_TEST);
}


static void on_font_save_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("error: cannot cast to KankerApp* in on_save_clicked().");
    return;
  }

  if (0 == app->font_filename.size()) {
    RX_ERROR("No filename entered. Cannot save.");
    return;
  }

  RX_VERBOSE("Saving file: %s, origin_x: %f", app->font_filename.c_str(), app->kanker_font.origin_x);
  
  app->kanker_font.save(rx_to_data_path("fonts/" +app->font_filename));
}

static void on_font_file_selected(int selectid, int optionid, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("error: cannot cast to KankerApp* in on_file_selected().");
    return;
  }

  app->selected_font_dx = optionid;
}

static void on_font_load_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("error: cannot cast to KankerApp* in on_file_selected().");
    return;
  }

  std::vector<std::string> files;
  if (0 != app->getFontFiles(files)) {
    RX_ERROR("error: cannot load font, file not found.");
    return;
  }

  if (app->selected_font_dx >= files.size()) {
    RX_ERROR("error: selected font dx is too big: %d, files.size() = %lu.", app->selected_font_dx, files.size());
    return;
  }

  std::string filepath = rx_to_data_path("fonts/" +files[app->selected_font_dx]);
  if (!rx_file_exists(filepath)) {
    RX_ERROR("error: cannot load file; file seems to be removed?");
    return;
  }
  
  if (0 != app->kanker_font.load(filepath)) {
    RX_ERROR("error: font failed to load: %s\n", filepath.c_str());
    return;
  }

  app->font_filename = files[app->selected_font_dx];

  app->switchState(KSTATE_CHAR_OVERVIEW);
}

static void on_abb_save_settings_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("Failed to cast to KankerApp");
    return;
  }

  if (0 != app->kanker_abb.saveSettings(rx_to_data_path("abb_settings.xml"))) {
    RX_ERROR("Failed to save the settings.");
  }
}

static void on_abb_load_settings_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("Failed to cast to KankerApp");
    return;
  }

  if (0 != app->kanker_abb.loadSettings(rx_to_data_path("abb_settings.xml"))) {
    RX_ERROR("Failed to load the settings.");
  }
}

static void on_abb_send_message_to_robot_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("Failed to cast to KankerApp");
    return;
  }
  
  if (0 != app->controller.writeText(12, app->test_message)) {
    RX_ERROR("Failed to write the text to the ABB.");
    return;
  }
}

static void on_abb_send_test_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("Failed to cast to kankerApp");
    return;
  }
  
  app->controller.kanker_abb.sendTestPositions();
}

static void on_abb_send_swipe_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("Failed to cast to KankerApp");
    return;
  }
  
  app->controller.kanker_abb.sendSwipePositions();
}

/* ------------------------------------------------------------------------------------ */
