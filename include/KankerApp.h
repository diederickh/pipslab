#ifndef KANKER_APP_H
#define KANKER_APP_H

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <FreetypeFont.h>
#include <KankerFont.h>
#include <KankerGlyph.h>
#include <KankerDrawer.h>

#define REMOXLY_USE_OPENGL
#include <gui/Remoxly.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_USE_PNG
#define ROXLU_USE_LOG
#include <glad/glad.h>
#include <tinylib.h>

using namespace rx;

enum {
  KSTATE_NONE,
  KSTATE_HOME,                                                                 /* Main screen. */ 
  KSTATE_CHAR_INPUT_TITLE,                                                     /* Shows a title, asks the user to press a key */
  KSTATE_CHAR_INPUT_DRAWING,                                                   /* User can draw the strokes */
  KSTATE_CHAR_EDIT,                                                            /* Change the baseline, advance-x etc.. */
  KSTATE_CHAR_PREVIEW,                                                         /* Preview the current character. */
  KSTATE_CHAR_OVERVIEW                                                         /* Shows all glyphs */
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
  void drawStateCharEdit();
  void drawStateCharOverview();                                                 /* Allows the user to select a glyph to edit. */
  void drawStateCharPreview();                                                  /* Draw a preview. */ 

  /* Helpers. */
  void drawHelperLines();
  void drawGlyphAsLine(KankerGlyph* glyph, float offsetX, float offsetY);
                                                                                
 public:                                                                        
  int state;                                                                    
  FreetypeFont title_font;                                                      /* Font for a big title. */
  FreetypeFont info_font;                                                       /* Used to show some info on screen. */
  FreetypeFont verbose_font;                                                    /* Used to show what character is select and maybe some more specs. */
  Painter painter;                                                              /* Used when drawing the captured input. */
  KankerFont kanker_font;                                                       /* The font we're adding glyphs to. */
  KankerGlyph* kanker_glyph;                                                    /* The current glyph to which points are added */
  KankerDrawer tiny_drawer;                                                     /* Used to draw the glyphs in a more interesting way. */
  KankerDrawer preview_drawer;                                                  /* Used to draw the preview of the character. */
  bool is_mouse_pressed;                                                        /* Is set to true when the user pressed the mouse */
  Container* gui_home;                                                          /* The gui container. */
  int gui_width;                                                                /* The width of the gui, used to position some graphical elements */ 
  std::string font_filename;                                                    /* The last set or loaded file name. */
  int selected_font_dx;                                                         /* Used when loading a font. */
  ssize_t glyph_dx;                                                             /* Used when showing a loaded font.  Points to a glyph index. */

  /* edit state */
  bool is_mouse_inside_char;
  bool is_mouse_inside_advance;                                                 /* Is the mouse inside the advance_x area, to manipulate the advance pos. */
  float mouse_down_x;
  float mouse_down_y;
  float char_offset_x;
  float char_offset_y;
  float advance_x;
};

#endif
