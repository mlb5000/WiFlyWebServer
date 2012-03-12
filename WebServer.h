#ifndef BAKERMATT_WEB_SERVER_H
#define BAKERMATT_WEB_SERVER_H

#include "Logger.h"

class WebServer
{
public:
  WebServer() : m_outBuf(0) {}
  ~WebServer() {}
  
  /** @brief Process a request and return a response

      @param request [in] The request as read from the network
      @param reqSize [in] The size of the request buffer
      @param response [out] The response buffer to be filled
      @param resSize [in/out] In - The maximum size of the response buffer
          Out - The number of bytes populated in response

      Call readRemaining to get the rest of the bytes for this response
      
      @retval 0 Success, Response is filled and no data remains
      @retval 1 Success, Response is filled, more data remains
      @retval 2 Failed
  **/
  int processRequest(
      const char *request,
      size_t reqSize,
      char *response,
      size_t *resSize);

  /** @brief Continue reading bytes for a previous request

      @param response [out] The response buffer to be filled
      @param resSize [in/out] In - The maximum size of the response buffer
          Out - The number of bytes populated in response

      Call readRemaining to get the rest of the bytes for this response
      
      @retval 0 Success, Response is filled and no data remains
      @retval 1 Success, Response is filled, more data remains
      @retval 2 Failed
  **/  
  int readRemaining(
      char *response,
      size_t *resSize);

private:
  /** @brief A pointer to the remaining output buffer */
  char *m_outBuf;
};

#endif