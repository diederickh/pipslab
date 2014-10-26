#ifndef KANKER_APP_H
#define KANKER_APP_H


#include <stdio.h>
#include <string>
#include <vector>
#include <FreetypeFont.h>
#include <KankerFont.h>
#include <KankerGlyph.h>

#define REMOXLY_USE_OPENGL
#include <gui/Remoxly.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_USE_PNG
#include <glad/glad.h>
#include <tinylib.h>

using namespace rx;

enum {
  KSTATE_NONE,
  KSTATE_HOME,                      /* Main screen. */ 
  KSTATE_CHAR_INPUT_TITLE,          /* Shows a title, asks the user to press a key */
  KSTATE_CHAR_INPUT_DRAWING,        /* User can draw the strokes */
  KSTATE_CHAR_EDIT                  /* Change the baseline, advance-x etc.. */
};

class KankerApp {

 public:
  KankerApp();
  ~KankerApp();
  int init();
  void update();
  void draw();
  void switchState(int newstate);
  int getFontFiles(std::vector<std::string>& result);

  /* Key and mouse interaction */
  void onChar(unsigned int key);
  void onKeyPress(int key, int scancode, int mods);
  void onKeyRelease(int key, int scancode, int mods);
  void onResize(int w, int h);
  void onMouseMove(double x, double y);
  void onMousePress(double x, double y, int bt, int mods);
  void onMouseRelease(double x, double y, int bt, int mods);

 private:
  void drawGui();
  void drawStateHome();
  void drawStateCharInputTitle();
  void drawStateCharInputDrawing();

 public:
  int state;
  FreetypeFont title_font;      /* Font for a big title. */
  FreetypeFont info_font;       /* Used to show some info on screen. */
  FreetypeFont verbose_font;    /* Used to show what character is select and maybe some more specs. */
  Painter painter;              /* Used when drawing the captured input. */
  KankerFont kanker_font;
  KankerGlyph* kanker_glyph;
  bool is_mouse_pressed;        /* Is set to true when the user pressed the mouse */
  Container* gui_home;
  int gui_width;
  std::string font_filename;
  int selected_font_dx;         /* Used when loading a font. */
};

#endif
