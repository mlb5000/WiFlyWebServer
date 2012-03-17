#ifndef BAKERMATT_WEB_SERVER_H
#define BAKERMATT_WEB_SERVER_H

#include "Logger.h"
#include "Network.h"
#include "FileSystem.h"

#define MIN_WEBSERVER_BUF_SIZE 128

class WebServer
{
public:
  WebServer(FileSystem &fileSystem, Network &network);
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
      size_t reqSize);

private:
  /** @brief Read a file in fixed size chunks
  
      @param file File to read
  */
  void chunkedFileRead(FIL *file);
  
  void netWrite(const char *toWrite, size_t size);
  void netWrite_P(const char *toWrite, size_t size);
  
  FileSystem &m_fileSystem;
  Network &m_network;

  #define TMP_FILE_BUF_SIZE 128
  char m_tmpFileBuf[TMP_FILE_BUF_SIZE];
};

#endif