#pragma once

#include "ofMain.h"
#include <kanker/KankerAbb.h>
#include <kanker/KankerAbbController.h>

class ofApp : public ofBaseApp,
  public KankerAbbListener,
  public KankerAbbControllerListener {
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

/* KankerAbbControllerListener */
void onAbbStateChanged(int state, int64_t messageID);

 public:
  KankerAbbController abb;
//KankerAbb abb;
};
