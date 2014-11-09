/*

  Make sure to call the following function before using this class:

    `curl_global_init(CURL_GLOBAL_ALL);`

  and when you're ready call:

     `curl_global_cleanup()`.

 */
#ifndef ROXLU_FTP_H
#define ROXLU_FTP_H

extern "C" {
#  include <curl/curl.h>
}

#include <fstream>
#include <string>

class FtpTask {
 public:
  FtpTask();
  ~FtpTask();

 public:
  std::string filepath;
  FILE* fp;
  CURL* curl;
};

class Ftp {
 public:
  Ftp();
  ~Ftp();
  int upload(std::string url, std::string filepath);
  int download(std::string url, std::string savepath);

 public:
  CURL* curl;
};

#endif
