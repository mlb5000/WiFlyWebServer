#ifndef NETWORK_H
#define NETWORK_H
#include <stdio.h>

#define TEST_VIRTUAL virtual

/** @brief Class providing an abstraction around the network interface

    It is currently write only until a need for reading arises.
*/
class Network
{
public:
  Network() {}
  TEST_VIRTUAL ~Network() {}

  /** @brief Write a byte onto the network

      @param c Byte to write
      @return Returns the character written or -1 on failure
  */
  TEST_VIRTUAL int writeByte(unsigned char c);
};

#endif