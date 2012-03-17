#ifndef NETWORK_MOCK_H
#define NETWORK_MOCK_H

#include "Network.h"
#ifdef _WIN32
#include <string>
#endif

class NetworkMock : public Network
{
public:
  NetworkMock() {}
  virtual ~NetworkMock() {}

  virtual int writeByte(unsigned char c);

  const char *getBytesWritten() {
#ifdef _WIN32
    return m_bytesWritten.c_str();
#else
    return m_bytesWritten;
#endif
  }

  size_t getNumBytesWritten() {
#ifdef _WIN32
    return m_bytesWritten.length();
#else
    return m_bytes;
#endif
  }

private:
#ifdef _WIN32
  std::string m_bytesWritten;
#else
  int m_bytes;
  char m_bytesWritten[500];
#endif
};

int NetworkMock::writeByte(unsigned char c) {
#ifdef _WIN32
  m_bytesWritten.append(1, c);
#else
  m_bytesWritten[m_bytes++] = c;
#endif
  return c;
}

#endif