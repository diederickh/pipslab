/*
 
  BASIC GLFW + GLXW WINDOW AND OPENGL SETUP 
  ------------------------------------------
  See https://gist.github.com/roxlu/6698180 for the latest version of the example.
   
  We make use of the GLAD library for GL loading, see: https://github.com/Dav1dde/glad/
 
*/
#include <stdlib.h>
#include <stdio.h>

#include <KankerApp.h> 
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define REMOXLY_USE_OPENGL
#define REMOXLY_IMPLEMENTATION
#include <gui/Remoxly.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_USE_PNG
#define ROXLU_USE_LOG
#define ROXLU_USE_CURL
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

#include <FreetypeFont.h>
KankerApp* app_ptr = NULL;

/* ------------------------------------------------------------------------------------ */

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

/* ------------------------------------------------------------------------------------ */
 
int main() {
 
  glfwSetErrorCallback(error_callback);
 
  if(!glfwInit()) {
    printf("Error: cannot setup glfw.\n");
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
  GLFWwindow* win = NULL;
  int w = 1280;
  int h = 768;
 
  win = glfwCreateWindow(w, h, "KankerFont v0.0.0.1", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
 
  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCharCallback(win, char_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  if (!gladLoadGL()) {
    printf("Cannot load GL.\n");
    exit(1);
  }

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------
  rx_log_init();

  KankerApp app;
  if (0 != app.init()) {
    printf("error: cannot init the app.\n");
    exit(EXIT_FAILURE);
  }
  app_ptr = &app;

  while(!glfwWindowShouldClose(win)) {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
    app.draw();
    glfwSwapBuffers(win);
     
    glfwPollEvents();
  }
 
  glfwTerminate();

  return EXIT_SUCCESS;
}

void char_callback(GLFWwindow* win, unsigned int key) {

  if (app_ptr) {
    app_ptr->onChar(key);
  }
}
 
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if (GLFW_RELEASE == action) {
    if (app_ptr) {
      app_ptr->onKeyRelease(key, scancode, mods);
    }
  }
  else if(GLFW_PRESS == action) {
    if (app_ptr) {
      app_ptr->onKeyPress(key, scancode, mods);
    }
  }
 
  switch(key) {
    case GLFW_KEY_SPACE: {
      break;
    }
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
}
 
void resize_callback(GLFWwindow* window, int width, int height) {

  if (app_ptr) {
    app_ptr->onResize(width, height);
  }
}
 
void cursor_callback(GLFWwindow* win, double x, double y) {

  if (app_ptr) {
    app_ptr->onMouseMove(x, y);
  }
}
 
void button_callback(GLFWwindow* win, int bt, int action, int mods) {

  double mx = 0;
  double my = 0;
  glfwGetCursorPos(win, &mx, &my);

  if (GLFW_PRESS == action) {
    if (app_ptr) {
      app_ptr->onMousePress(mx, my, bt, mods);
    }
    
  }
  else if (GLFW_RELEASE == action) {
    if (app_ptr) {
      app_ptr->onMouseRelease(mx, my, bt, mods);
    }
  }
}
 
void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}

