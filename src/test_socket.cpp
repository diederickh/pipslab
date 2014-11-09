#include <kanker/Socket.h>
#include <stdio.h>
#include <stdlib.h>
#define ROXLU_USE_LOG
#define ROXLU_IMPLEMENTATION
#include <tinylib.h>

int main() {

  rx_log_init();

  RX_VERBOSE("Socket Test");
  
  socket_init();

  Socket sock;

  if (0 != sock.connect("www.roxlu.com", 80)) {
    RX_ERROR("Cannot connect.");
    exit(1);
  }

  if (0 != sock.send("GET / HTTP/1.0\r\n\r\n")) {
    RX_ERROR("Cannot send.");
    exit(1);
  }

  char buffer[1024];
  int num = 4;
  int nread = 0;
  for (int i = 0; i < num; ++i) {
    if (0 == sock.canRead(1,0)) {
      nread = sock.read(buffer, sizeof(buffer));
      if (0 > nread) {
        RX_ERROR("Error while reading from socket: %d", nread);
      }
      else {
        for (int k = 0; k < nread; ++k) {
          printf("%c", buffer[k]);
        }
      }
      break;
    }
    else {
      RX_VERBOSE("Cannot read from socket yet.");
    }
  }

  socket_shutdown();

  return 0;
}
