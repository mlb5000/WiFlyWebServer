#include "WebServer.h"
#include <string.h>
#include <stdio.h>

static const char *result = "HTTP/1.1 200 OK\r\n" \
"Content-Length: 27\r\n" \
"Content-Type text/html; charset=utf-8\r\n" \
"Transfer-Encoding: chunked\r\n\r\n" \
"1B\r\n" \
"these are the page contents\r\n" \
"0\r\n\r\n";

#define min(a,b) a >= b ? b : a

int WebServer::processRequest(
    const char *request,
    size_t reqSize,
    char *response,
    size_t *resSize) {
  int retval = 0;
  size_t bytesToWrite = 0;
  
  //reset current request out buffer
  m_outBuf = 0;
  
  if (0 == request || 0 == reqSize || 0 == response || 0 == resSize || 0 == *resSize) {
    return 2;
  }
  
  bytesToWrite = min(strlen(result), *resSize);
  strncpy(response, result, bytesToWrite);
  *resSize = bytesToWrite;
  
  if (*resSize < strlen(result)) {
    m_outBuf = (char*)result + *resSize;
    retval = 1;
  }
  
  return retval;
}

int WebServer::readRemaining(
    char *response,
    size_t *resSize)
{
  int retval = 0;
  size_t bytesToWrite = 0;
  
  if (0 == m_outBuf || 0 == response || 0 == resSize || 0 == *resSize) {
    return 2;
  }
  
  bytesToWrite = min(strlen(m_outBuf), *resSize);
  strncpy(response, m_outBuf, bytesToWrite);
  *resSize = bytesToWrite;
  
  if (*resSize < strlen(m_outBuf)) {
    m_outBuf += *resSize;
    retval = 1;
  }
  else {
    m_outBuf = 0;
  }
  
  return retval;      
}