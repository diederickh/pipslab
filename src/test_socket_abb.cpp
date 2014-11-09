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
  void onAbbReadyToDraw();
  void onAbbDrawing();
public:
  KankerAbb* abb;
};
/* ---------------------------------------------------------------------- */

int main() {

  signal(SIGINT, sighandler);

  rx_log_init();

  RX_VERBOSE("Socket ABB Test");
  
  socket_init();

#if USE_RAW_TEST

  Socket sock;

  if (0 != sock.connect("127.0.0.1", 1025)) {
    RX_ERROR("Cannot connect.");
    exit(1);
  }

  /* TEST RAW DATA */
  Buffer buffer;
  buffer.writePosition(0, -680, -300);
  buffer.writePosition(0, 680, -300);
  buffer.writePosition(0, 680, 220);
  buffer.writePosition(0, -680, 220);
  buffer.writePosition(0, -680, -300);

  RX_VERBOSE("Buffer contains: %lu bytes.", buffer.size());

  int num = 0;
  while (true) { 
    RX_VERBOSE("Running num: %d, sending: %d bytes", num, buffer.size());
    sock.send(buffer.ptr(), buffer.size());
    SLEEP_MILLIS(4000);
    ++num;
  }
#endif

#if USE_ABB_TEST
  /* TEST ABB WRAPPER */
  KankerAbb abb;
  AbbListener abb_listener(&abb);
  
  if (0 != abb.connect()) {
    RX_ERROR("Failed to connect");
    exit(1);
  }

  if (0 != abb.setAbbListener(&abb_listener)) {
    RX_ERROR("Failed to set the abb listener.");
    exit(1);
  }

  std::vector<vec3> positions;
  positions.push_back(vec3(0, -680, -300));
  positions.push_back(vec3(0, 680, -300));
  positions.push_back(vec3(0, 680, 220));
  positions.push_back(vec3(0, -680, 220));
  positions.push_back(vec3(0, -680, -300));

  SLEEP_MILLIS(1000);


  char read_buffer[1024] = { 0 } ;
  bool can_draw = true;
  int num = 0;
  while (true) { 
    //    RX_VERBOSE("Running num: %d", num);
    abb.processIncomingData();

    //SLEEP_MILLIS(100);
#if 0    
    if (can_draw) {
      for (size_t i = 0; i < positions.size(); ++i) {
        vec3& v = positions[i];
        abb.sendPosition(v.y, v.z, v.x);
        RX_VERBOSE("Sending: x: %f, y: %f, z: %f", v.x, v.y, v.z);
      }
      RX_VERBOSE("Sending draw");
      abb.sendDraw();
    }

    SLEEP_MILLIS(15000);
    
    /* Check if there is data on the socket. */
    if (0 == abb.sock.canRead(1, 0)) {
      RX_VERBOSE("There is some data on the socket.");
      int nread = abb.sock.read(read_buffer, sizeof(read_buffer));
      if (0 > nread) {
        RX_ERROR("Failed to read data from socket: %d", nread);
      }
      else {
        RX_VERBOSE("Read some data: %d, %c", nread, read_buffer[0]);
      }
    }
    
#endif
#if 0
    vec3& v = positions[num % positions.size()];
    abb.sendPosition(v.y, v.z, v.x);
    RX_VERBOSE("Sending: x: %f, y: %f, z: %f", v.x, v.y, v.z);
    SLEEP_MILLIS(50000);
#endif
    //RX_VERBOSE("Sending DRAW");
    //abb.sendDraw();
    //abb.sendResetPacketIndex();
    //abb.sendPosition(-680, -300, num);
   
    ++num;
  }
#endif

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
  RX_VERBOSE("The ABB is ready to draw, change..");

  if (NULL == abb) {
    RX_ERROR("Abb is ready to draw but the KankerAbb pointer is NULL.");
    return;
  }

  std::vector<vec3> positions;
  positions.push_back(vec3(0, -680, -300));
  positions.push_back(vec3(0, 680, -300));
  positions.push_back(vec3(0, 680, 220));
  positions.push_back(vec3(0, -680, 220));
  positions.push_back(vec3(0, -680, -300));

  for (size_t i = 0; i < positions.size(); ++i) {
    vec3& v = positions[i];
    abb->sendPosition(v.y, v.z, v.x);
    RX_VERBOSE("Sending: x: %f, y: %f, z: %f", v.x, v.y, v.z);
  }
  RX_VERBOSE("Sending draw");
  abb->sendDraw();
}

void AbbListener::onAbbDrawing() {
  RX_VERBOSE("Abb is drawing....");
}
