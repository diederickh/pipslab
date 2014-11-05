/*
  
  Kanker ABB Controller
  ----------------------

  This class ties everything related to controlling the ABB
  together. It uses a `KankerFont` instance to generate the 
  draw commands for the ABB. It uses `Ftp` to upload a new set
  of commands to the ABB which will be read by the application 
  running on the ABB.  We poll the ABB to check if the state 
  changed. 

 */
#ifndef KANKER_ABB_CONTROLLER_H
#define KANKER_ABB_CONTROLLER_H

#include <vector>
#include <fstream>
#include <string>
#include <Ftp.h>
#include <KankerFont.h>
#include <KankerAbb.h>
#include <rapidxml.hpp>

using namespace rapidxml;

/* ----------------------------------------------------------------- */

class KankerAbbControllerSettings {
 public:
  std::string font_file;
  std::string settings_file;
};

/* ----------------------------------------------------------------- */

class KankerAbbController {

 public:
  KankerAbbController();
  ~KankerAbbController();
  int init(KankerAbbControllerSettings cfg);                        /* Initialize the controller. */
  int writeText(std::string text);                                  /* This make sure that the ABB will draw the given text */  
  void update();                                                    /* Call this often to make sure that we can read/update the remote state. */

 public:
  KankerAbbControllerSettings settings;
  KankerFont kanker_font;
  KankerAbb kanker_abb;
  Ftp ftp;
  std::vector<KankerAbbGlyph> abb_glyphs;                           /* Storage for the glyphs that are generated by the KankerAbb and KankerFont objects. */
  std::vector<std::vector<vec3> > abb_points;                       /* Storage for the glyphs that are generated by the KankerAbb; we don't actually use them here but they may be used to draw line segments that make up the font. */
  int is_init;
}; 

#endif