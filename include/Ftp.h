/*


 */
#ifndef ROXLU_FTP_H
#define ROXLU_FTP_H

#include <curl/curl.h>
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
