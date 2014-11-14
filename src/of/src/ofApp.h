#pragma once

#include "ofMain.h"
#include <kanker/KankerAbb.h>
#include <kanker/KankerAbbController.h>
#include <ofxOsc.h>

class ofApp : public ofBaseApp, 
              public KankerAbbListener {
 public:
  void setup();
  void update();
  void draw();
		
  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  /* KankerAbbListener */
  void onAbbReadyToDraw();
  void onAbbDrawing();
  void onAbbConnected();
  void onAbbDisconnected();
  void onAbbMessageReady();

 public:

  /* We communicate with Keez's application over osc. */
  ofxOscReceiver osc_receiver;
  ofxOscSender osc_sender;

  /* The `KankerAbbController` takes care of all communication with the ABB and keeps state of the socket i/o */
  KankerAbbController abb;
  bool is_connected_with_abb;
  std::string last_message_text;
  int64_t last_message_id;
  bool can_write_to_abb;
};
