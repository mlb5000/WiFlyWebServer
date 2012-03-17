#include "Network.h"

int Network::writeByte(unsigned char c) {
#ifdef _WIN32
  return putchar(c);
#else
  return WIFLY.print(c);
#endif
}