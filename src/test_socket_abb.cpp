#include <kanker/Socket.h>
#include <kanker/Buffer.h>
#include <kanker/KankerAbb.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <vector>

#define ROXLU_USE_LOG
#define ROXLU_USE_MATH
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

#define USE_RAW_TEST 0
#define USE_ABB_TEST 1

bool must_run = true;
static void sighandler(int s);

/* ---------------------------------------------------------------------- */
class AbbListener : public KankerAbbListener {
public:
  AbbListener(KankerAbb* abb);
  void onAbbConnected();
  void onAbbDisconnected();
  void onAbbReadyToDraw();
  void onAbbDrawing();
public:
  KankerAbb* abb;
};

/* ---------------------------------------------------------------------- */

int main() {

  signal(SIGINT, sighandler);
  rx_log_init();
  socket_init();

  RX_VERBOSE("Socket ABB Test");
  
  KankerAbb abb;
  AbbListener abb_listener(&abb);

  /* These values are found by trial and error. */
  abb.min_x = -680;
  abb.max_x = 680;
  abb.min_y = -300;
  abb.max_y = 200;

 if (0 != abb.setAbbListener(&abb_listener)) {
    RX_ERROR("Failed to set the abb listener.");
    exit(1);
  }
 
  if (0 != abb.connect()) {
    RX_ERROR("Failed to connect. KankerAbb will try to reconnect when calling processIncomingData.");
  }

  RX_VERBOSE("Starting socket loop");

  while (true) { 
    abb.processIncomingData();
  }

  socket_shutdown();

  return 0;
}

static void sighandler(int s) {
  RX_VERBOSE("Got signal.");
  must_run = false;
}

/* ---------------------------------------------------------------------- */
AbbListener::AbbListener(KankerAbb* abb) 
  :abb(abb)
{
}

void AbbListener::onAbbReadyToDraw() {

  RX_VERBOSE("Returning directly in onAbbReadyToDraw");
  return; 

  static uint64_t counter = 0;

  RX_VERBOSE("The ABB is ready to draw, change..");

  if (NULL == abb) {
    RX_ERROR("Abb is ready to draw but the KankerAbb pointer is NULL.");
    return;
  }

  std::vector<vec3> positions;
  int draw_mode = counter % 3;
  if (0 == draw_mode) {
    positions.push_back(vec3(-680, -300, 0));
    positions.push_back(vec3(680, -300, 0));
    positions.push_back(vec3(680, 220, 0));
    positions.push_back(vec3(-680, 220, 0));
    positions.push_back(vec3(-680, -300, 0));


  }
  else if (1 == draw_mode) {
    positions.push_back(vec3(-680, 0, 0));
    positions.push_back(vec3(680, 0, 0));
    positions.push_back(vec3(0, 0, 0));
  }
  else if (2 == draw_mode) {
    positions.push_back(vec3(0, 0, 0));
    positions.push_back(vec3(0, -300, 0));
    positions.push_back(vec3(0, 0, 0));
    positions.push_back(vec3(0, -300, 0));
    positions.push_back(vec3(0, 0, 0));
    positions.push_back(vec3(0, -300, 0));
    positions.push_back(vec3(0, 0, 0));
  }
  else {
    RX_VERBOSE("Unhandled draw mode: %d", draw_mode);
  }

  for (size_t i = 0; i < positions.size(); ++i) {
    vec3& v = positions[i];
    abb->sendPosition(v.y, v.z, v.x);
    //abb->sendPosition(v.x, v.y, v.z);
    RX_VERBOSE("Sending: x: %f, y: %f, z: %f", v.x, v.y, v.z);
  }
  RX_VERBOSE("Sending draw");
  abb->sendDraw();

  counter++;
}

void AbbListener::onAbbDrawing() {
  RX_VERBOSE("Abb is drawing....");
}

void AbbListener::onAbbConnected() {
  RX_VERBOSE("Connected with Abb");

  std::vector<KankerAbbGlyph> test_message;
  std::vector<vec3> positions;
  KankerAbbGlyph glyph;
  
  {
    positions.push_back(vec3(-680, -300, 0));
    positions.push_back(vec3(680, -300, 0));
    positions.push_back(vec3(680, 220, 0));
    positions.push_back(vec3(-680, 220, 0 ));
    positions.push_back(vec3(-680, -300, 0));

    glyph.segments.push_back(positions);
    test_message.push_back(glyph);
    positions.clear();
    glyph.segments.clear();
  }

  {
    positions.push_back(vec3(-680, 0, 0));
    positions.push_back(vec3(680, 0, 0));
    positions.push_back(vec3(0, 0, 0));

    glyph.segments.push_back(positions);
    test_message.push_back(glyph);
    positions.clear();
    glyph.segments.clear();
  }

  {
    positions.push_back(vec3(0, 0, 0));
    positions.push_back(vec3(0, -300, 0 ));
    positions.push_back(vec3(0, 0, 0));
    positions.push_back(vec3(0, -300, 0));
    positions.push_back(vec3(0, 0, 0));
    positions.push_back(vec3(0, -300, 0));
    positions.push_back(vec3(0, 0, 0));

    glyph.segments.push_back(positions);
    test_message.push_back(glyph);
    positions.clear();
    glyph.segments.clear();
  }

  RX_VERBOSE("Send the test message");
  abb->sendText(test_message);
}

void AbbListener::onAbbDisconnected() {
  RX_VERBOSE("Got disconnected from ...");
}
