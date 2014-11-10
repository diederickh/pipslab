#include <kanker/Socket.h>

/* ------------------------------------------------------------------------- */

#if defined(_WIN32) 

int socket_init() {
  WORD version = MAKEWORD(2,2);
  WSADATA wsa_data;
  int err = WSAStartup(version, &wsa_data);
  if (0 != err) {
    RX_ERROR("Failed to initialize winsock, %d", err);
    return -1;
  }
  return 0;
}
  
int socket_shutdown() {
  return WSACleanup();
}
  
#else

int socket_init() { return 0; }
int socket_shutdown() { return 0; }

#endif

/* ------------------------------------------------------------------------- */

Socket::Socket() 
  :handle(-1)
  ,listener(NULL)
{
  if (-1 != handle) {
    close();
  }
}

Socket::~Socket() {
  RX_ERROR("We need to cleanup / close the socket when it's open.");
  listener = NULL;
}

int Socket::connect(std::string host, uint16_t port) {

  int r;
  std::stringstream ss;
  struct addrinfo* result, *rp, hints;

  if (-1 != handle) {
    RX_ERROR("Already connected, call close(), handle is: %d", handle);
    return -1;
  }

  if (0 == host.size()) {
    RX_ERROR("Invalid host (empty)");
    return -2;
  }

  if (0 == port) {
    RX_ERROR("Invald port: 0.");
    return -3;
  }

  ss << port;

#if defined(_WIN32)
  ZeroMemory(&hints, sizeof(hints));
#endif

  /* Get address info for the given host. */
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  r = getaddrinfo(host.c_str(), ss.str().c_str(), &hints, &result);
  if (0 != r) {
    RX_ERROR("Cannot get address info for %s:%u. Error: %s", host.c_str(), port, gai_strerror(r));
    return -4;
  }

  /* Iterate over the found addresses and create socket. Use first one that works. */
  r = -99;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    handle = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (handle < 0) {
      continue;
    }
      
    r = ::connect(handle, rp->ai_addr, rp->ai_addrlen);
    if (0 == r) {
      break;
    }
  }

  /* When we arrive here and r == -99 we're not connected. */
  if (0 != r) {
    close();
    freeaddrinfo(result);
    RX_ERROR("Could not find host %s:%u. Cleanly reset the socket handle. handle is: %d", host.c_str(), port, handle);
    return -5;
  }

  freeaddrinfo(result);

  /* Connection failed? */
  if (NULL == rp) {
    RX_ERROR("Could not connect to %s:%u", host.c_str(), port);
    close();
    return -5;
  }

  if (NULL != listener) {
    listener->onSocketConnected();
  }

  return 0;
}

int Socket::send(const char* data, int nbytes) {

  int err = 0;
  int done = 0;

  if (NULL == data) {
    RX_ERROR("Trying to send NULL data.");
    return -1;
  }

  if (0 == nbytes) {
    RX_ERROR("Trying to send 0 bytes.");
    return -2;
  }

  if (0 != isConnected()) {
    RX_ERROR("Cannot send because we're not connected.");
    return -3;
  }

  while (nbytes > 0 && 0 == isConnected()) {
#if defined(_WIN32)      
    done = ::send(handle, (const char*) data, nbytes, 0);
#else
    done = ::send(handle, (const void*) data, nbytes, 0);
#endif

    if (done != nbytes) {
      /* @todo implement the situation where we couldn't send all bytes. */
      RX_ERROR("Failed to send all bytes, this is a situation we need to handle correctly. We sent: %d, but should: %lu.", done, nbytes);
      exit(1);
    }

    if (done < 0) {
      err = socket_get_error();
      if (0 == socket_is_recoverable_error(err)) {
        return 0;
      }

      RX_ERROR("Error while sending data over socket: %s", strerror(errno));

      if (0 != close()) {
        RX_ERROR("Failed to cleanly close the socket after being disconnected.");
      }

      if (NULL != listener) {
        listener->onSocketDisconnected();
      }

      return -3;
    }
      
    nbytes -= done;
  }

  return 0;
}

int Socket::canRead(int sec, int usec) {

  struct timeval timeout;
  fd_set readset;
  int r;

  if (0 != isConnected()) {
    RX_ERROR("Not connected so we cannot read.");
    return -1;
  }

  timeout.tv_sec = sec;
  timeout.tv_usec = usec;
    
  FD_ZERO(&readset);
  FD_SET(handle, &readset);

  r = select(handle + 1, &readset, NULL, NULL, &timeout);
  if (0 == r) {
    return -1;
  }
  else if (r < 0) {
    RX_ERROR("Socket error: %s", strerror(errno));
    if (0 != socket_is_recoverable_error(errno)) {

      if (0 != close()) {
        RX_ERROR("Erorr while trying to closing the socket after an unrecoverable error occured.");
        return -1;
      }

      if (NULL != listener) {
        listener->onSocketDisconnected();
      }
    }
    return -1;
  }

  return (FD_ISSET(handle, &readset)) ? 0 : -1;
}

int Socket::read(char* buffer, int nbytes) {
    
  int err;
  int r;
  int chunk_size = 1024;

  if (NULL == buffer) {
    RX_ERROR("Trying to read into NULL buffer.");
    return -1;
  }
  if (0 == nbytes) {
    RX_ERROR("Buffer size is 0. No way we can read into that.");
    return -2;
  }

  if (0 != isConnected()) {
    return -3;
  }

  r = recv(handle, buffer, nbytes, 0);

  if (r < 0) {
    err = socket_get_error();
    if (0 == socket_is_recoverable_error(err)) {
      return 0;
    }

    RX_ERROR("Could not read data from socket: %s", strerror(errno));
    close();
    return -4;
  }

  if (0 == r) {
    close();
    return -5;
  }

//  if (NULL != listener) {
//    listener->onSocketRead(buffer, r);
//  }
  return r;
}

int Socket::close() {

  /* Already closed? */
  if (0 != isConnected()) {
    return 0; 
  }

  /* Close */
  if (-1 != handle) {
    errno = EINTR;
#if defined(_WIN32)      
    int result = -1;
    int err = 0;
    do { 

      result = ::closesocket(handle);
      handle = -1; /* event when closesocket returns < 0, we unset the handle so we can reuse it. */

      if (0 != result) {

        err = socket_get_error();

        switch(err) {
          case WSANOTINITIALISED: {
            RX_ERROR("Socket not initialized.");
            return 0;
          }
          case WSAENETDOWN: {
            RX_ERROR("Network is down. Socket closed");
            return 0;
          }
          case WSAENOTSOCK: {
            RX_ERROR("The `sock` member is not a real socket. This is not supposed to happen but occurs when connect() fails.");
            return 0;
          }
          case WSAEINPROGRESS: {
            RX_VERBOSE("We're in the process of closing the socket; continuing.");
            return 0;
          }
          case WSAEINTR: {
            RX_VERBOSE("Call was interrupted.");
            return 0;
          }
          case WSAEWOULDBLOCK: {
            RX_ERROR("WSAWOULDBLOCK");
            return 0;
          }
          default: {
            RX_ERROR("Unhandled error in close(), error: %d", err);
            return 0;
          }
        }
      }
    } while (0 != result);
#else
    while (::close(handle) != 0 && EINTR == errno) { }
#endif
    handle = -1;
  }

  return 0;
}
