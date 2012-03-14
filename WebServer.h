#ifndef BAKERMATT_WEB_SERVER_H
#define BAKERMATT_WEB_SERVER_H

#include "Logger.h"
#include "FileSystem.h"

#define MIN_WEBSERVER_BUF_SIZE 128

class WebServer
{
public:
  WebServer(FileSystem &fileSystem);
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
  /** @brief Read a file in fixed size chunks
  
      @param outBuf [out] Output buffer
      @param resSize [in/out] In - max size of outBuf, Out - number of bytes filled
      
      @retval 0 Success, Response is filled and no data remains
      @retval 1 Success, Response is filled, more data remains
      @retval 2 Failed
  */
  int chunkedFileRead(
    char *outBuf,
    size_t *resSize);
    
  enum RequestType
  {
    NONE,
    GET
  };
  
  RequestType m_requestType;
  
  /** @brief Pointer to a string containing the file path to get */
  char *m_getPath;
  
  FileSystem &m_fileSystem;
  FIL m_file;
  
  char m_tmpFileBuf[MIN_WEBSERVER_BUF_SIZE];
  
  /** @brief Finalize the chunked get request on the next call */
  bool m_nextCallFinalize;
};

#endif