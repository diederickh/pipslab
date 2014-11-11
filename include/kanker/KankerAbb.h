/*

  KankerAbb
  ----------

  Used to communicate with ABB and prepare the characters we want to write. 
  You can write a string and we  will make sure that the given string will 
  fit inside the range of the ABB. 

  You set the width / height of the range in milimeters before calling 
  KankerAbb::write(). These can be set using the `min_x, min_y, max_x, 
  max_y` members.


*/
#ifndef KANKER_ABB_H
#define KANKER_ABB_H

#define ROXLU_USE_MATH
#define ROXLU_USE_LOG
#include <tinylib.h>
#include <rapidxml.hpp>
#include <kanker/Socket.h>
#include <kanker/Buffer.h>
#include <kanker/KankerFont.h>
#include <kanker/KankerGlyph.h>
#include <sstream>
#include <vector>
#include <fstream>
#include <stdint.h>

using namespace rapidxml;

/* ---------------------------------------------------------------------- */

#define ABB_CMD_POSITION 0               /* Send a position, command will be x,y,z (floats). */
#define ABB_CMD_IO 1                     /* We want to toggle an io port, command will be: port-num, on/off. */
#define ABB_CMD_DRAW 3                   /* When the robot receives this it will start moving all the received positions / commands. */ 
#define ABB_CMD_GET_STATE 4              /* Get the state of the ABB. */

#define ABB_STATE_UNKNOWN -1             /* Default, uninitialized state. */
#define ABB_STATE_READY 1                /* The ABB is ready to receive commands. */
#define ABB_STATE_DRAWING 2              /* The ABB is currently drawing something. */  
#define ABB_STATE_DISCONNECTED 3         /* We're disconnected and we'll automatically try to reconnect */ 
#define ABB_STATE_CONNECTED 4            /* We're connected with the ABB. */

/* ---------------------------------------------------------------------- */

class KankerAbbGlyph {
 public:
  KankerAbbGlyph();
  ~KankerAbbGlyph();

 public:
  KankerGlyph glyph;
  std::vector<std::vector<vec3> > segments;
};

/* ---------------------------------------------------------------------- */

class KankerAbbListener {
 public:
  virtual void onAbbReadyToDraw() {}                                                 /* Gets called when the ABB is ready to receive new drawing commands. */
  virtual void onAbbDrawing() {}                                                     /* Gets called whenever the ABB starts drawing. */
  virtual void onAbbConnected() {}                                                   /* Gets called when we're connected to the abb. */
  virtual void onAbbDisconnected() {}                                                /* Gets called when the socket connection with the ABB is lost. The KankerAbb object will try to reconnect so you don't have to call handle reconnecting yourself. */
  virtual void onAbbMessageReady() {}                                                /* Gets called when a complete message has been drawn with the abb. */
};

/* ---------------------------------------------------------------------- */

class KankerAbb : public SocketListener {

 public:
  KankerAbb();
  ~KankerAbb();

  int connect();                                                                     /* Connects to the robot. The user can call this or use the `auto connect` feature by calling `update()` often. */
  int update();                                                                      /* This must be called often. It will check the current state and acts upon that. E.g. when writing a message to the robot it will send each character at the correct time (when we received an ready event from the robot). */
  void print();                                                                      /* Prints some information about the object. */

  int write(KankerFont& font,                                                        /* Write a message using this font. */
            std::string str,                                                         /* The message to write. */
            std::vector<KankerAbbGlyph>& out,                                        /* We will fill this vector with glyphs that can be passed into `save()`. */
            std::vector<std::vector<vec3> >& segmentsOut);                           /* Contains only the line segments, used to draw. */

  std::vector<vec3> simplify(std::vector<vec3>& in, float minDist);                  /* Simplifies the given points by making sure none of the points in the segment is closer then `minDist`. */
  float getWordWidth(KankerFont& font, std::string word);                            /* Returns the width of the given word in milimeters. */
  int saveSettings(std::string filepath);                                            /* Save the current state of the font. */ 
  int loadSettings(std::string filepath);                                            /* Load the current state of the font. */ 
  vec3 convertFontPointToAbbPoint(vec3& v);                                          /* This makes sure that the input position can be used by the robot. */
  float getRangeWidth();                                                             /* Get the available width that can be used by the robot. */
  float getRangeHeight();                                                            /* Get the available height that can be used by the robot. */
  int setAbbListener(KankerAbbListener* lis);                                        /* Set the listener which will receive events from this object. */
  int sendText(std::vector<KankerAbbGlyph>& glyphs);                                 /* Send a complete text to the Abb. Make sure to call `update()` often because we send each glyph one at a time. */ 
  int sendNextGlyph();                                                               /* Is called internally when writing a message. This is called by `update()` when you issues a `writeText()` */
  int sendTestPositions();                                                           /* Sends some test positions that shows you the range in which the ABB is moving. */
  int sendCheckState();                                                              /* Sends the check state command to the Abb; used to get the state but also to detect if the abb is offline. */
  void onSocketConnected();                                                          /* Gets called by the `sock` member when we're connected with the Abb. */
  void onSocketDisconnected();                                                       /* Gets called by the `sock` member when we get disconnected. */   

 public:
  float offset_x;                                                                    /* Used to position all glyphs with an offset so that 0,0 nicely aligns with the range we have in which we can draw. */
  float offset_y;                                                                    /* Used to position all glyphs with a offset. */
  float char_scale;                                                                  /* How to scale the characters. Used to make sure the glyphs fit inside the min/max ranges. */
  float word_spacing;                                                                /* How many space between words? */ 
  float line_height;                                                                 /* Line height for multi line messages (baseline) */
  float min_point_dist;                                                              /* Mininum distance in pixels between two points; used to simplify the font because when points are too close ABB may get into trouble. */
  int min_x;                                                                         /* Min X position of the ABB, e.g. -680. X is from left to right. */
  int max_x;                                                                         /* Max X position of the ABB, e.g. 680, X is from left to right. */ 
  int min_y;                                                                         /* Min Y position of the ABB, e.g. -300 (bottom). Y is from top to bottom. */
  int max_y;                                                                         /* Max Y position of the ABB, e.g. 200. Y is from top to bottom. */  
  int abb_port;                                                                      /* The port of ABB to which we connect. */ 
  std::string abb_host;                                                              /* The ip of ABB to which we will send commands. */
  Socket sock;                                                                       /* Socket that we use to connect to the Abb. */
  Buffer buffer;                                                                     /* Buffer to write binary data that is sent to the Abb */
  char read_buffer[1024];                                                            /* Buffer that we used to read from the socket. */  
  uint64_t check_abb_state_timeout;                                                  /* When we will check the state of the Abb again. */  
  uint64_t check_abb_state_delay;                                                    /* Delay between the checks. */
  uint64_t abb_reconnect_timeout;                                                    /* When we will try to connect again when disconnected from Abb. */
  uint64_t abb_reconnect_delay;                                                      /* Delay between reconnect checks. */   
  uint8_t abb_state;                                                                 /* Robot state. */       
  KankerAbbListener* abb_listener;                                                   /* The listener that will be called when e.g. a glyph has been draw, connected, disconnected etc.. */ 
  std::vector<KankerAbbGlyph> curr_message;                                          /* Copy of the message that is given ito `sendText()` */
  size_t curr_glyph_index;                                                           /* When we're writing the curr_message this is the index of the glyph that is sent to the Abb. */
};

inline float KankerAbb::getRangeWidth() {
  return max_x - min_x;
}

inline float KankerAbb::getRangeHeight() {
  return max_y - min_y;
}

inline int KankerAbb::setAbbListener(KankerAbbListener* lis) {

  if (NULL == lis) {
    RX_ERROR("Failed to set the abb listener because it's NULL");
    return -1;
  }

  abb_listener = lis;

  return 0;
}

#endif
