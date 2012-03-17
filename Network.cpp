#include "Network.h"
#include <WiFlyWebServer.h>

int Network::writeByte(unsigned char c) {
#ifdef _WIN32
  return putchar(c);
#else
  return serial2Write(c);
#endif
}