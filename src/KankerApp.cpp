#include <KankerApp.h>
#include <GLFW/glfw3.h>

/* ------------------------------------------------------------------------------------ */

static void on_add_characters_clicked(int id, void* user);
static void on_save_clicked(int id, void* user);
static void on_file_selected(int selectid, int optionid, void* user);
static void on_load_clicked(int id, void* user);

/* ------------------------------------------------------------------------------------ */

KankerApp::KankerApp() 
  :state(KSTATE_NONE)
  ,kanker_glyph(NULL)
  ,is_mouse_pressed(false)
  ,is_mouse_inside_char(false)
  ,is_mouse_inside_advance(false)
  ,gui_width(256)
  ,selected_font_dx(0)
  ,glyph_dx(0)
  ,mouse_down_x(0)
  ,mouse_down_y(0)
  ,char_offset_x(0)
  ,char_offset_y(0)
  ,advance_x(0)
{
}

KankerApp::~KankerApp() {
  delete gui_home;
  gui_home = NULL;
}

int KankerApp::init() {
  
  /* title font */
  if (0 != title_font.open(rx_to_data_path("appfont.otf"), 60)) {
    RX_ERROR("error: cannot load the font.");
    exit(EXIT_FAILURE);
  }

  title_font.color(1.0f, 1.0f, 1.0f, 1.0f);
  title_font.alignCenter();

  /* info font */
  if (0 != info_font.open(rx_to_data_path("appfont.otf"), 16)) {
    RX_ERROR("error: cannot load the font.");
    exit(EXIT_FAILURE);
  }
  info_font.color(1.0f, 1.0f, 1.0f, 1.0f);

  /* verbose font */
  if (0 != verbose_font.open(rx_to_data_path("appfont.otf"), 16)) {
    RX_ERROR("error: cannot load the font.");
    exit(EXIT_FAILURE);
  }
  verbose_font.color(0.0f, 0.7f, 0.1f, 1.0f);

  switchState(KSTATE_HOME);

  gui_home = new Container(new RenderGL());
  gui_home->add(new Button("Add characters",  0, GUI_ICON_FONT, on_add_characters_clicked, this,  GUI_CORNER_ALL | GUI_OUTLINE)).setPosition(painter.width() - 228, 31).setWidth(200);
  gui_home->add(new Text("Filename", font_filename, 135)).setPosition(painter.width() - 227, painter.height() - 105).setWidth(200);
  gui_home->add(new Button("Save",  0, GUI_ICON_FLOPPY_O, on_save_clicked, this,  GUI_CORNER_ALL | GUI_OUTLINE)).setPosition(painter.width() - 228, painter.height() - 80).setWidth(200);

  std::vector<std::string> fonts;
  if (0 == getFontFiles(fonts)) {
    Select* sel = new Select("Load file", 1, fonts, on_file_selected, this, GUI_CORNER_ALL | GUI_OUTLINE);
    sel->setDirection(GUI_DIRECTION_UP);
    gui_home->add(sel).setPosition(painter.width() - 228, painter.height() - 200).setWidth(200);
    gui_home->add(new Button("Load",  0, GUI_ICON_FOLDER_OPEN, on_load_clicked, this,  GUI_CORNER_ALL | GUI_OUTLINE)).setPosition(painter.width() - 228, painter.height() - 175).setWidth(200);
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

void KankerApp::draw() {

#if 0
  /* generate a string. */
  std::vector<std::vector<vec3> > v;
  kanker_font.write("diederick", v);
  if (0 != v.size()) {
    float offsetX = -150.0;
    float offsetY = 0.0;
    painter.clear();
    for (size_t i = 0; i < v.size(); ++i) {
      std::vector<vec3>& points = v[i];
      for (size_t j = 0; j < points.size() - 1; ++j) {
        vec3& a = points[j];
        vec3& b = points[j + 1];
        painter.line(a.x + offsetX, a.y + offsetY, b.x + offsetX, b.y + offsetY);
      }
    }
    painter.draw();
    return;
  }
#endif

  switch (state) {
    case KSTATE_HOME:               { drawStateHome();             break;    }
    case KSTATE_CHAR_INPUT_TITLE:   { drawStateCharInputTitle();   break;    }
    case KSTATE_CHAR_INPUT_DRAWING: { drawStateCharInputDrawing(); break;    }
    case KSTATE_CHAR_EDIT:          { drawStateCharEdit();         break;    }
    case KSTATE_CHAR_PREVIEW:       { drawStateCharPreview();      break;    }
    case KSTATE_CHAR_OVERVIEW:      { drawStateCharOverview();     break;    }
    default: {
      RX_ERROR("error: invalid state: %d", state);
      break;
    }
  };
}

void KankerApp::drawStateHome() {
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

  painter.hex("9F39DB");
  painter.line(0.0, painter.height() - gui_width, painter.width(), painter.height() - gui_width);

  painter.hex("004358");
  painter.line(gui_width, 0, gui_width, painter.height());

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
  gui_home->draw();
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
          advance_x = kanker_glyph->min_x + kanker_glyph->width;
          advance_x = CLAMP(advance_x, gui_width, painter.width() - gui_width);
        }
        else {
          advance_x = gui_width + kanker_glyph->advance_x;
        }
      }
      break;
    }
    case KSTATE_CHAR_PREVIEW: {
      if (kanker_glyph) {
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

  gui_home->onCharPress(key);

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

  gui_home->onKeyRelease(key, mods);

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
  gui_home->onKeyPress(key, mods);
}

void KankerApp::onResize(int w, int h) {
  painter.resize(w, h);
}

void KankerApp::onMouseMove(double x, double y) {

  gui_home->onMouseMove(x, y);

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

  gui_home->onMousePress(x, y, bt, mods);

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
  

  gui_home->onMouseRelease(x, y, bt, mods);

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

static void on_save_clicked(int id, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("error: cannot cast to KankerApp* in on_save_clicked().");
    return;
  }

  if (0 == app->font_filename.size()) {
    RX_ERROR("No filename entered. Cannot save.");
    return;
  }

  RX_VERBOSE("saving file: %s", app->font_filename.c_str());

  app->kanker_font.save(rx_to_data_path("fonts/" +app->font_filename));
}

static void on_file_selected(int selectid, int optionid, void* user) {

  KankerApp* app = static_cast<KankerApp*>(user);
  if (NULL == app) {
    RX_ERROR("error: cannot cast to KankerApp* in on_file_selected().");
    return;
  }

  app->selected_font_dx = optionid;
}

static void on_load_clicked(int id, void* user) {

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

  

  // std::vector<std::vector<KankerVertex> > vertices;
  // app->kanker_font.generateVerticesForText("abc", vertices);
}

/* ------------------------------------------------------------------------------------ */
