#include <KankerAbbController.h>

/* ----------------------------------------------------------------- */
KankerAbbController::KankerAbbController() 
  :is_init(-1)
{
}

KankerAbbController::~KankerAbbController() {
}

int KankerAbbController::init(KankerAbbControllerSettings cfg) {

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

  if (0 == kanker_abb.ftp_url.size()) {
    RX_ERROR("Not ftp_url found in the settings file. Make sure to add <ftp_url>ftp://username:password@upload.host.com/</ftp_url> to the settings.");
    return -6;
  }

  if (0 != kanker_font.load(cfg.font_file)) {
    return -7;
  }

  settings = cfg;
  is_init = 0;

  return 0;
}

int KankerAbbController::writeText(std::string text) {

  if (0 != is_init) {
    RX_ERROR("Not initialized, cannot write text.");
    return -1;
  }

  abb_glyphs.clear();
  abb_points.clear();
  
  if (0 != kanker_abb.write(kanker_font, text, abb_glyphs, abb_points)) {
    RX_ERROR("Failed to write the text: %s", text.c_str());
    return -2;
  }

  std::string mod_filepath = rx_to_data_path("text.mod");

  if (0 != kanker_abb.saveAbbModule(mod_filepath, abb_glyphs)) {
    RX_ERROR("Failed to save the ABB module file.");
    return -3;
  }

  /* TMP do we upload here? */
  if (0 != ftp.upload(kanker_abb.ftp_url, mod_filepath)) {
    RX_ERROR("Failed to upload the module file. See verbose output in console.");
    return -4;
  }

  return 0;
}

void KankerAbbController::update() {
}
