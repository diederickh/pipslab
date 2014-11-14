#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

  ofBackground(0,0,0);
  
  can_write_to_abb = false;
  is_connected_with_abb = false;
  last_message_id = -1;

  /* Setup the KankerAbbController */
  KankerAbbControllerSettings cfg;
  cfg.font_file = ofToDataPath("keez.xml");
  cfg.settings_file = ofToDataPath("abb_settings.xml");

  if (0 != abb.init(cfg, this)) {
    printf("error: cannot init the abb controller.\n");
    ::exit(EXIT_FAILURE);
  }

  /* Setup OSC so we can communicate with Keez' app. */
  int receiver_port = 2233;
  int sender_port = 2244;
  std::string sender_host = "127.0.0.1";
  sender_host ="192.168.1.74";
  osc_receiver.setup(receiver_port);
  osc_sender.setup(sender_host, sender_port);
}

//--------------------------------------------------------------
void ofApp::update(){

  int r = 0;

  abb.update();

  /* Check for new messages from Keez' app. */
  while (osc_receiver.hasWaitingMessages()) {

    ofxOscMessage m;
    if (false == osc_receiver.getNextMessage(&m)) {
      RX_ERROR("Failed to retrieve the message from osc.");
      continue;
    }

    if (m.getAddress() == "/message/new") {

      /* Make sure we can write. */
      if (false == can_write_to_abb) {
        RX_ERROR("We received a /messag/new but we're not ready to write yet. Not supposed to happen.");        
        return;
      }

      /* Get the text */
      last_message_text = m.getArgAsString(0);
      if (0 == last_message_text.size()) {
        RX_ERROR("We recieved a /message/new but the given text is empty. Ignoring message.");
        return;
      }

      last_message_id = m.getArgAsInt32(1);

      /* And write the text with the ABB. */
      r = abb.writeText(last_message_id, last_message_text);
      if (0 != r) {
        RX_ERROR("Received an error when trying to write. Error code: %d. Message id: %lld, text: %s", r, last_message_id, last_message_text.c_str());
        return;
      }
    }
  } /* while */
}

//--------------------------------------------------------------
void ofApp::draw(){

  if (is_connected_with_abb) {
    ofSetColor(0, 255, 0);
    ofDrawBitmapString("Connected to robot.", 10, 30);
  }
  else {
    ofSetColor(255, 0, 0);
    ofDrawBitmapString("Disconnected from robot. We will automatically reconnect.", 10, 30);
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

  static int64_t msg_id = 0;

  if (key == 't' || key == 'T') {
    printf("Sending something to the abb.\n");
    abb.writeText(msg_id, "hola!");
    ++msg_id;
  }
  else if (key == 'r' || key == 'R') {
    sendReadyToKeez();
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::onAbbReadyToDraw() {
}

void ofApp::onAbbDrawing() {

}

void ofApp::onAbbConnected() {

  RX_VERBOSE("Connected to ABB.");

  is_connected_with_abb = true;
  can_write_to_abb = true;
}

void ofApp::onAbbDisconnected(){

  RX_VERBOSE("Disconnected from ABB. We will automatically reconnect in a couple of seconds.");

  is_connected_with_abb = false;
  can_write_to_abb = false;
}

void ofApp::onAbbMessageReady() {

  RX_VERBOSE("------------------------ READY -----------------------");

  can_write_to_abb = true;

  sendReadyToKeez();
}

void ofApp::sendReadyToKeez() {

  /* Notify Keez' application that we're ready with wring the message. */
  ofxOscMessage m;
  m.setAddress("/message/ready");
  m.addIntArg((int)last_message_id);

  osc_sender.sendMessage(m);
}
