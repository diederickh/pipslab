/*
  
  Buffer
  ------

  Basic wrapper around a `std::vector<uin8_t>` that we use to pack and unpack
  data we send to the ABB. We store all numeric multi byte values in big endian.

 */
#ifndef ROXLU_ABB_BUFFER_H
#define ROXLU_ABB_BUFFER_H

#include <limits.h>
#include <stdint.h>
#include <vector>
#include <sstream>

#define ROXLU_USE_LOG
#include <tinylib.h>

class Buffer {

 public:
  Buffer();
  ~Buffer();
  int size();
  void clear();
  uint8_t* ptr();
  void writePosition(float x, float y, float z, float rotationZ = 0.0);
  void writeU8(uint8_t v);
  void writeU32(uint32_t v);
  void writeFloat(float f);

 public:
  std::vector<uint8_t> data;          /* The data that was written to the buffer. */
};

/* ------------------------------------------------------------------------- */

inline int Buffer::size() {

  if (data.size() > INT_MAX) {
    RX_ERROR("The buffer size is currently too large to send.");
    return -1;
  }

  return (int)data.size();
}

inline void Buffer::clear() {
  data.clear();
}

inline uint8_t* Buffer::ptr() {
  return (uint8_t*)&data[0];
}

/* ------------------------------------------------------------------------- */
inline void Buffer::writePosition(float x, float y, float z, float rotationZ) {
  writeFloat(x);
  writeFloat(y);
  writeFloat(z);
  writeFloat(rotationZ);
}

inline void Buffer::writeFloat(float f) {
  uint8_t* p = (uint8_t*)&f;
  data.push_back(p[3]);
  data.push_back(p[2]);
  data.push_back(p[1]);
  data.push_back(p[0]);
}

inline void Buffer::writeU8(uint8_t v) {
  data.push_back(v);
}

inline void Buffer::writeU32(uint32_t v) {
  uint8_t* p = (uint8_t*)&v;
  data.push_back(p[3]);
  data.push_back(p[2]);
  data.push_back(p[1]);
  data.push_back(p[0]);
}

/* ------------------------------------------------------------------------- */

#endif
