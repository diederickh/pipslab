/*

  KankerAbb
  ----------

  Used to communicate with ABB and prepare the characters
  we want to write using the ABB. You can write a string and we 
  will make sure that the given string will fit inside the range 
  of the ABB. 

  You set the width / height of the range in milimeters before 
  calling KankerAbb::write()

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

#define ABB_CMD_POSITION 0               /* Send a position, command will be x,y,z (floats). */
#define ABB_CMD_IO 1                     /* We want to toggle an io port, command will be: port-num, on/off. */
#define ABB_CMD_RESET_PACKET_INDEX 2     /* Reset the read and write index of the packet array on the robot. */
#define ABB_CMD_DRAW 3                   /* When the robot receives this it will start moving all the received positions / commands. */ 
#define ABB_CMD_GET_STATE 4              /* Get the state of the ABB. */

#define ABB_STATE_UNKNOWN -1
#define ABB_STATE_READY 1
#define ABB_STATE_DRAWING 2
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
};

/* ---------------------------------------------------------------------- */

class KankerAbb : public SocketListener {

 public:
  KankerAbb();
  ~KankerAbb();

  int write(KankerFont& font,                                                        /* Write a message using this font. */
            std::string str,                                                         /* The message to write. */
            std::vector<KankerAbbGlyph>& out,                                        /* We will fill this vector with glyphs that can be passed into `save()`. */
            std::vector<std::vector<vec3> >& segmentsOut);                           /* Contains only the line segments, used to draw. */

  int saveAbbModule(std::string filepath, int64_t id, std::vector<KankerAbbGlyph>& message);     /* Save the given glyphs into a file that can be read by the ABB. */
  float getWordWidth(KankerFont& font, std::string word);                            /* Returns the width of the given word in milimeters. */
  int saveSettings(std::string filepath);                                            /* Save the current state of the font. */ 
  int loadSettings(std::string filepath);                                            /* Load the current state of the font. */ 
  void print();                                                                      /* Prints some information about the object. */

  void checkAbbPosition(vec3& v); /* This makes sure that the input position can be used by the robot. */

  /* --- BEGIN: TEST WITH SOCKET --- */
  int setAbbListener(KankerAbbListener* lis);
  int connect();                                      /* Connects to the robot. */
  int processIncomingData();                          /* Checks if there is any data from the ABB */
  int sendTestData();                                 /* Send some test coordinates to the robot. */
  int sendPosition(float x, float y, float z);
  int sendText(std::vector<KankerAbbGlyph>& glyphs);  /* Send a complete text to the Abb. */ 
  int sendTestPositions();                            /* Sends some test positions. */
  int sendResetPacketIndex();
  int sendCheckState();
  int sendDraw();
  void onSocketConnected();
  void onSocketDisconnected();
  /* --- END: TEST WITH SOCKET --- */

 public:
  float offset_x;       /* used to position all glyphs with a offset. */
  float offset_y;       /* used to position all glyphs with a offset. */
  float range_width;    /* range in milimeters */
  float range_height;   /* range in milimeters */
  float char_scale;     /* how to scale the characters. */
  float word_spacing;   /* how many mm between words? */ 
  float line_height;    /* mm per line. */
  int min_x;
  int max_x;
  int min_y;
  int max_y;
  std::string ftp_url;  /* used by the controller; it would be cleaner to store this in the controller; */
  std::string abb_host; 
  int abb_port; 

  Socket sock;          /* 2nd version where we communicate over a socket. */
  Buffer buffer;        /* 2nd version, buffer that we use to pack data. */
  char read_buffer[1024];
  uint64_t check_abb_state_timeout;
  uint64_t check_abb_state_delay;
  uint64_t abb_reconnect_timeout;
  uint64_t abb_reconnect_delay;
  uint8_t abb_state;
  KankerAbbListener* abb_listener;
};

inline int KankerAbb::setAbbListener(KankerAbbListener* lis) {

  if (NULL == lis) {
    RX_ERROR("Failed to set the abb listener because it's NULL");
    return -1;
  }

  abb_listener = lis;

  return 0;
}

#endif
