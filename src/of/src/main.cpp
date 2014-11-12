#include "ofMain.h"
#include "ofApp.h"
#include <kanker/Socket.h>
#include <tinylib.h>

#define ROXLU_USE_LOG
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

//========================================================================
int main( ){

  socket_init();
  rx_log_init();

	ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new ofApp());

}
