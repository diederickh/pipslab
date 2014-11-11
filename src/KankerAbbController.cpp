#include <kanker/KankerAbbController.h>
#include <rapidxml.hpp>

using namespace rapidxml;


KankerAbbController::KankerAbbController() 
  :is_init(-1)
  ,state(KC_STATE_NONE)
  ,last_message_id(-1)
  ,listener(NULL)
{
}

KankerAbbController::~KankerAbbController() {
}

int KankerAbbController::init(KankerAbbControllerSettings cfg,
                              KankerAbbControllerListener* lis)
 {
   
  if (0 == is_init) {
    RX_ERROR("Already initialized.");
    return -1;
  }

  if (0 == cfg.font_file.size()) {
    RX_ERROR("Font file name length is 0.");
    return -2;
  }

  if (0 == cfg.settings_file.size()) {
    RX_ERROR("Settings file name length is 0.");
    return -3;
  }

  if (0 != kanker_abb.loadSettings(cfg.settings_file)) {
    return -5;
  }

  if (0 != kanker_font.load(cfg.font_file)) {
    return -7;
  }

  if (NULL == lis) {
    RX_ERROR("No listener passed into the controller. We need this.");
    return -8;
  }

  if (0 != kanker_abb.connect()) {
    RX_VERBOSE("Failed to connect to the ABB. We will retry in update().");
  }

  listener = lis;
  settings = cfg;
  is_init = 0;

  return 0;
}

int KankerAbbController::writeText(int64_t id, std::string text) {

  if (0 != is_init) {
    RX_ERROR("Not initialized, cannot write text.");
    return -1;
  }

  if (NULL == listener) {
    RX_ERROR("No listener set; cannot write text.");
    return -2;
  }

  if (id == last_message_id) {
    RX_WARNING("You want to write a new message but using the same message id as before. %lld (we continue with writing).", id);
  }

  last_message_id = id;

  abb_glyphs.clear();
  abb_points.clear();
  
  if (0 != kanker_abb.write(kanker_font, text, abb_glyphs, abb_points)) {
    RX_ERROR("Failed to write the text: %s", text.c_str());
    return -3;
  }

  if(0 != kanker_abb.sendText(abb_glyphs)) {
    RX_ERROR("Failed to send text.");
    return -4;
  }

  /* We're writing to the ABB */
  switchState(KC_STATE_WRITING);

  return 0;
}

void KankerAbbController::update() {
  kanker_abb.update();
}


void KankerAbbController::switchState(int st) {

  RX_VERBOSE("Switching ABB state: %d", st);

  if (st == state) {
    RX_VERBOSE("Trying to switch state but we are already in the requested state: %d", st);
    return;
  }

  state = st;
}


