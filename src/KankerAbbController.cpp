#include <kanker/KankerAbbController.h>
#include <rapidxml.hpp>

using namespace rapidxml;

/* ----------------------------------------------------------------- */

template <class T> int read_xml(xml_node<>* node, std::string name, T defaultval, T& result);

/* ----------------------------------------------------------------- */

KankerAbbController::KankerAbbController() 
  :is_init(-1)
  ,state(KC_STATE_NONE)
  ,last_message_id(-1)
  ,poll_delay(1e9 * 10)     /* Poll the ABB every `X` seconds. */
  ,poll_timeout(0)
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

#if 0
  if (0 == kanker_abb.ftp_url.size()) {
    RX_ERROR("Not ftp_url found in the settings file. Make sure to add <ftp_url>ftp://username:password@upload.host.com/</ftp_url> to the settings.");
    return -6;
  }
#endif

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
#if 0

  /* Update: we're currently testing a solution w/o FTP. */
  std::string mod_filepath = rx_to_data_path("mTEXT.mod");

  if (0 != kanker_abb.saveAbbModule(mod_filepath, id, abb_glyphs)) {
    RX_ERROR("Failed to save the ABB module file.");
    return -4;
  }

  if (0 != ftp.upload(kanker_abb.ftp_url, mod_filepath)) {
    RX_ERROR("Failed to upload the module file. See verbose output in console.");
    return -5;
  }
#endif

  /* We're writing to the ABB */
  switchState(KC_STATE_WRITING);

  return 0;
}

void KankerAbbController::update() {

  /* When we're writing check if the ABB is ready by checking the state. */
#if 0
  if (KC_STATE_WRITING == state) {
    uint64_t n = rx_hrtime();
    if (n >= poll_timeout) {
      if (0 != checkAbbState()) {
        RX_ERROR("Failed to check the ABBs state.");
      }
      poll_timeout = n + poll_delay;
    }
  }
#endif

  kanker_abb.processIncomingData();
}

int KankerAbbController::checkAbbState() {

  if (0 == kanker_abb.ftp_url.size()) {
    RX_ERROR("Cannot check the ABBs state because the ftp url is not set.");
    return -1;
  }

  std::string outfile = rx_to_data_path("abb_state.txt");
  std::string url = kanker_abb.ftp_url;

  if ( url[url.size()-1] != '/') {
    url += "/";
  }

  RX_VERBOSE("Checking ABB state");

  url += "abb_state.txt";
  
  if (0 != ftp.download(url, outfile)) {
    RX_ERROR("Failed to download the ABB state file.");
    return -2;
  }

  if (!rx_file_exists(outfile)) {
    RX_ERROR("The download of the ABB state succeeded but we cannot find the file on disk (?).");
    return -3;
  }

  std::string str = rx_read_file(outfile);
  
  if (0 != parseStateXml(str)) {
    RX_ERROR("Failed to parse the state xml.");
    return -4;
  }

  return 0;
}

void KankerAbbController::switchState(int st) {

  RX_VERBOSE("Switching ABB state: %d", st);

  if (st == state) {
    RX_VERBOSE("Trying to switch state but we are already in the requested state: %d", st);
    return;
  }

  state = st;
  
  if (KC_STATE_WRITING == state) {
    poll_timeout = rx_hrtime() + poll_delay;
  }
}

/* 
   The `str` will looks like:

   <state>
    <value>ready</value>
    <id>1233</id>
   </state>


   The <value> describes the current state. We have:
     - ready: The ABB is ready with writing.
     - busy: The ABB is currently writing.
   
   The <id> holds the value of the message ID that we 
   uploaded to the ABB. 

 */
int KankerAbbController::parseStateXml(std::string str) {

  if (0 == str.size()) {
    RX_ERROR("The xml string is empty.");
    return -1;
  }

  int r = 0;
  std::string state_value;
  int64_t state_message_id = -1;
  xml_document<> doc;

  try {

    doc.parse<0>((char*)str.c_str());
    
    xml_node<>* cfg = doc.first_node("state");

    /* Read state value. */
    r = read_xml<std::string>(cfg, "value", "none", state_value);
    if (0 != r) {
      RX_ERROR("Cannot read the <value> element of the ABB state.");
      return -1;
    }

    /* Read message id. */
    r = read_xml<int64_t>(cfg, "id", -1, state_message_id);
    if (0 != r) {
      RX_ERROR("Failed to read the <id> element of the ABB state.");
      return -2;
    }

    if (state_message_id == last_message_id) {
      if (state_value == "ready" && KC_STATE_READY != state) {
        RX_VERBOSE("The ABB is ready with writing %lld", state_message_id);
        switchState(KC_STATE_READY);

        /* Notify listener. */
        if (listener) {
          listener->onAbbStateChanged(state, state_message_id);
        }
      }
      else {
        RX_VERBOSE("The ABB has current state: %s for %lld", state_value.c_str(), state_message_id);
      }
    }
    else if (state_value == "ready") {
      RX_VERBOSE("The message ID on the ABB is not the same as our last id. This is not supposed to happen. We're going to reset our state. This can happen when this application is shutdown and the ABB was still running.");
      last_message_id = state_message_id;
      switchState(KC_STATE_READY);
      
      if (listener) {
        listener->onAbbStateChanged(state, state_message_id);
      }
    }
    else {
      RX_VERBOSE("Current ABB state: %s, for message ID: %lld", state_value.c_str(), state_message_id);
    }
  }
  catch (...) {
    RX_ERROR("Caught xml exception.");
    return -2;
  }
  return 0;
}

/* ----------------------------------------------------------------- */

template <class T> int read_xml(xml_node<>* node, std::string name, T defaultval, T& result) {
  result = defaultval;

  if (0 == name.size()) {
    RX_ERROR("name.size() == 0.");
    return -1;
  }

  if (NULL == node) {
    RX_ERROR("Container node for %s is NULL.", name.c_str());
    return -2;
  }

  xml_node<>* el = node->first_node(name.c_str());
  if (NULL == el) {
    RX_ERROR("%s not found in %s", name.c_str(), node->name());
    return -3;
  }

  std::stringstream ss;
  ss << el->value();
  ss >> result;
    
  return 0;
}
