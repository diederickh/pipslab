#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

  KankerAbbControllerSettings cfg;
  cfg.font_file = ofToDataPath("keez.xml");
  cfg.settings_file = ofToDataPath("abb_settings.xml");

  if (0 != abb.init(cfg, this)) {
    printf("error: cannot init the abb controller.\n");
    ::exit(EXIT_FAILURE);
  }
}

//--------------------------------------------------------------
void ofApp::update(){
  abb.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  static int64_t msg_id = 0;
  abb.writeText(msg_id, "hola!");
  ++msg_id;
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

}

void ofApp::onAbbDisconnected(){
}

void ofApp::onAbbMessageReady() {

}

void ofApp::onAbbStateChanged(int state, int64_t messageID) {
  printf("State changed.\n");
}
