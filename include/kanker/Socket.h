/*
  Socket
  -------

  Client socket implementation we use to connect to the ABB. 

*/
#ifndef ROXLU_SOCKET_H
#define ROXLU_SOCKET_H

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  define SOCKET_HANDLE SOCKET
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <errno.h>
#  include <unistd.h>
#  define SOCKET_HANDLE int
#endif

#if defined(__linux)
#  include <string.h>
#endif

#include <stdint.h>
#include <string>

#define ROXLU_USE_LOG
#include <tinylib.h>

/* ------------------------------------------------------------------------- */

int socket_init();                                                           /* Must be called by user once to initialize the socket library. */
int socket_shutdown();                                                       /* When you're ready with using sockets, make sure to call socket_shutdown to cleanup. */
int socket_is_recoverable_error(int err);                                    /* Cross platform way to check if the given error code is recoverable. */
int socket_get_error();                                                      /* Returns the last socket error. */

/* ------------------------------------------------------------------------- */

class SocketListener {
 public:
  virtual void onSocketConnected() {}                                        /* Gets called when the socket is connected. */
  virtual void onSocketDisconnected() {}                                     /* Gets called when the socket is disconnected. */   
};

/* ------------------------------------------------------------------------- */

class Socket {
 public:
  Socket();
  ~Socket();
  int connect(std::string host, uint16_t port);                               /* Connect to the HOST and PORT. Make sure to set the listener before calling init() if you want to handle disconnect events. */  
  int close();                                                                /* Close the socket when it's created. */
  int send(const char* data, int nbytes);                                     /* Send the given data. */                          
  int send(const std::string& data);                                          /* Send the given string. */
  int send(const char* data);                                                 /* Send the given string (must be null terminated). */
  int send(const uint8_t* data, int nbytes);                                  /* Send the given buffer of `nbytes`. */
  int read(char* buffer, int nbytes);                                         /* Read bytes into the given buffer of `nbytes` size. */
  int canRead(int sec, int usec);                                             /* Check if there is data available on the socket but timeout after `sec` and/or `usec` */ 
  int isConnected();                                                          /* Returns `0` when the socket is connected to the remote host. */

  int setListener(SocketListener* listener);
                                                                              
 public:                                                                      
  SOCKET_HANDLE handle;                                                       /* Reference to the OS specific socket handle, e.g. int on Posix and SOCKET on windows. */
  SocketListener* listener;
}; 

/* ------------------------------------------------------------------------- */

inline int Socket::setListener(SocketListener* lis) {

  if (NULL == lis) {
    return -1;
  }

  listener = lis;

  return 0;
}

inline int Socket::isConnected() {
  return (handle >= 0) ? 0 : -1;
}

inline int Socket::send(const std::string& data) {
  return send((const uint8_t*)data.data(), data.size());
}
  
inline int Socket::send(const char* data) {
  int len = strlen(data);
  return send(data, len);
}

inline int Socket::send(const uint8_t* data, int nbytes) {
  return send((const char*)data, nbytes);
}

/* ------------------------------------------------------------------------- */

inline int socket_is_recoverable_error(int err) {

#if defined(_WIN32)
  switch (err) {

    /* @todo -> Figure out what sock errors are recoverable... http://legacy.imatix.com/html/sfl/sfl369.htm */
    /* Recoverable */
    case 0:
    case WSAEINTR: 
    case WSAEINPROGRESS:
    case WSAEWOULDBLOCK: {
      return 0;
    }

      /* All others are not recoverable. */
    default: {

      return -1;
    }
  }
#else
  switch (err) {

    /* Recoverable */
    case 0:
    case EAGAIN:
    case EINTR: 
    case EINPROGRESS:
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK: 
#endif
      {
        return 0;
      }

      /* All others are not recoverable. */
    default: {

      return -1;
    }
  }
#endif
}

inline int socket_get_error() {
#if defined(_WIN32)
  return WSAGetLastError();
#else
  return errno;
#endif      
}
/* ------------------------------------------------------------------------- */

#endif
