#define ROXLU_USE_LOG
#include <tinylib.h>
#include <kanker/Ftp.h>

/* -------------------------------------------------------------------------------- */

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *user);
static size_t write_callback(void *buffer, size_t size, size_t nmemb, void *user);

/* -------------------------------------------------------------------------------- */

FtpTask::FtpTask()
  :curl(NULL)
  ,fp(NULL)
{
}

FtpTask::~FtpTask() {

  if (NULL != fp) {
    RX_ERROR("We expect the user to close the file handle.");
  }

  if (NULL != curl) {
    RX_ERROR("We expect the user to close the curl handle.");
  }

  curl = NULL;
  fp = NULL;
}

/* -------------------------------------------------------------------------------- */

Ftp::Ftp() {
}

Ftp::~Ftp() {
}

int Ftp::upload(std::string url, std::string filepath) {

  int r = 0;
  CURLcode err = CURLE_OK;
  FtpTask* task = NULL;

  struct curl_slist* slist = NULL;
  static const char* cmd_rnfr = "RNFR /mTEXT.mod";
  //  static const char* cmd_rnto = "RNTO PIPS_LAB/HOME/mTEXT.mod";
  static const char* cmd_dele = "DELE mTEXT.mod";
  static const char* cmd_rnto = "RNTO mTEXT.mod";

  url += "/" +rx_strip_dir(filepath);

  if (0 == filepath.size()) {
    RX_ERROR("Invalid filepath.");
    return -1;
  }

  if (false == rx_file_exists(filepath)) {
    RX_ERROR("Cannot find file: %s", filepath.c_str());
    return -2;
  }

  task = new FtpTask();
  if (NULL == task) {
    RX_ERROR("Cannot allocate a new FtpTask.");
    r = -3;
    goto error;
  }

  task->fp = fopen(filepath.c_str(), "rb");
  if (NULL == task->fp) {
    RX_ERROR("Failed ot open: %s", filepath.c_str());
    r = -4;
    goto error;
  }

  task->curl = curl_easy_init();
  if (NULL == task->curl) {
    RX_ERROR("Cannot create CURL*");
    r = -4;
    goto error;
  }

  /* Set read callback. */
  err = curl_easy_setopt(task->curl, CURLOPT_READFUNCTION, read_callback);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set the read callback.");
    r = -5;
    goto error;
  }

  /* Set read data. */
  err = curl_easy_setopt(task->curl, CURLOPT_READDATA, task);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set read data.");
    r = -6;
    goto error;
  }

  /* Enable uploading. */
  err = curl_easy_setopt(task->curl, CURLOPT_UPLOAD, 1L);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to enable uploading.");
    r = -6;
    goto error;
  }

  /* Move to other directory */
  slist = curl_slist_append(slist, cmd_dele);
  slist = curl_slist_append(slist, cmd_rnfr);
  slist = curl_slist_append(slist, cmd_rnto);
  curl_easy_setopt(task->curl, CURLOPT_POSTQUOTE, slist);

  /* Set FTP method (needed for ABB ftp) */
  err = curl_easy_setopt(task->curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_NOCWD);
  //  err = curl_easy_setopt(task->curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_SINGLECWD);
  //err = curl_easy_setopt(task->curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_MULTICWD);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set the FTP method.");
    r = -100;
    goto error;
  }

  /* Set remote url */
  err = curl_easy_setopt(task->curl, CURLOPT_URL, url.c_str());
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set url: %s", url.c_str());
    r = -7;
    goto error;
  }

  err = curl_easy_setopt(task->curl, CURLOPT_VERBOSE, 1L);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set verbose on ftp.");
    r = -8;
    goto error;
  }

  /* Upload! */
  err = curl_easy_perform(task->curl);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to upload file %s", filepath.c_str());
    r = -9;
    goto error;
  }

 error:
  if (task) {

    if (task->curl) {
      curl_easy_cleanup(task->curl);
      task->curl = NULL;
    }

    if (task->fp) {
      fclose(task->fp);
      task->fp = NULL;
    }

    delete task;
    task = NULL;
  }

  return r;
}

int Ftp::download(std::string url, std::string savepath) {

  int r = 0;
  CURLcode err = CURLE_OK;
  FtpTask* task = NULL;

  if (0 == url.size()) {
    RX_ERROR("Given url is invalid.");
    return -1;
  }

  if (0 == savepath.size()) {
    RX_ERROR("Given savepath is invalid.");
    return -2;
  }

  task = new FtpTask();
  if (NULL == task) {
    RX_ERROR("Cannot allocate the task to download.");
    r = -3;
    goto error;
  }

  task->fp = fopen(savepath.c_str(), "wb");
  if (NULL == task->fp) {
    RX_ERROR("Failed to open %s for writing.", savepath.c_str());
    r = -4;
    goto error;
  }

  task->curl = curl_easy_init();
  if (NULL == task->curl) {
    RX_ERROR("Cannot create CURL*");
    r = -5;
    goto error;
  }

  /* Set write callback. */
  err = curl_easy_setopt(task->curl, CURLOPT_WRITEFUNCTION, write_callback);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set the write callback.");
    r = -5;
    goto error;
  }

  /* Set write data. */
  err = curl_easy_setopt(task->curl, CURLOPT_WRITEDATA, task);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set write data.");
    r = -6;
    goto error;
  }

  /* Set remote url */
  err = curl_easy_setopt(task->curl, CURLOPT_URL, url.c_str());
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set url: %s", url.c_str());
    r = -7;
    goto error;
  }

  /* Enable verbose output. */
  err = curl_easy_setopt(task->curl, CURLOPT_VERBOSE, 1L);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to set verbose on ftp.");
    r = -8;
    goto error;
  }

  /* Download! */
  err = curl_easy_perform(task->curl);
  if (CURLE_OK != err) {
    RX_ERROR("Failed to download to file %s", savepath.c_str());
    r = -9;
    goto error;
  }

 error:

  if (task) {

    if (task->fp) {
      fclose(task->fp);
      task->fp = NULL;
    }

    if (task->curl) {
      curl_easy_cleanup(task->curl);
      task->curl = NULL;
    }

    delete task;
    task = NULL;
  }
  return r;

}

/* -------------------------------------------------------------------------------- */

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void* user) {

  FtpTask* task = static_cast<FtpTask*>(user);
  if (NULL == task) {
    RX_ERROR("Failed to get the task object in Curls read callback.");
    return 0;
  }

  curl_off_t nread;
  size_t retcode = fread(ptr, size, nmemb, task->fp);
 
  nread = (curl_off_t)retcode;
 
  RX_VERBOSE("We read %" CURL_FORMAT_CURL_OFF_T " bytes from file.", nread);

  return retcode;
}

static size_t write_callback(void *buffer, size_t size, size_t nmemb, void *user) {
  
  FtpTask* task = static_cast<FtpTask*>(user);
  if (NULL == task) {
    RX_ERROR("Failed to get the task object in Curls write callback.");
    return 0;
  }
  
  if (NULL == task->fp) {
    RX_ERROR("Failed to write data because the file handle was closed.");
    return 0;
  }

  return fwrite(buffer, size, nmemb, task->fp);
}



/* -------------------------------------------------------------------------------- */
