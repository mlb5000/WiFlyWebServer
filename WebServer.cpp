#include "WebServer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define strncasecmp_P strncmp
#define strcpy_P strcpy
#define strlen_P strlen
#endif

#define min(a,b) a >= b ? b : a

//request types
static const char GET_REQUEST[] PROGMEM = {"GET"};

static const PROGMEM char HTTP_TRAILER[] = "\r\n";

//HTTP response codes
static const PROGMEM char ERR_200[] = "HTTP/1.1 200 OK";
static const PROGMEM char ERR_404[] = "HTTP/1.1 404 File Not Found";
static const PROGMEM char ERR_500[] = "HTTP/1.1 500 Internal Server Error";

//HTTP response header fields
static const char CONTENT_TYPE[] PROGMEM = {"Content-Type text/html; charset=utf-8"};
static const char TRANSFER_ENCODING[] PROGMEM = {"Transfer-Encoding: chunked"};
static const char FINAL_CHUNK[] PROGMEM = {"0\r\n\r\n"};

WebServer::WebServer(FileSystem &fileSystem)
: m_fileSystem(fileSystem),
  m_requestType(NONE),
  m_nextCallFinalize(false)
{
  memset(&m_file, 0, sizeof(FIL));
}

/** @brief process and HTTP request and populate a response buffer

    Algorithm:
        1. Tokenize request.
          a. First token assumed to be request type.
          b. Second token assumed to be file path ('/' resolves to index.html)
        2. If type is anything other than GET, return failure (not yet implemented).
        3. Call f_stat on the requested file path.
          a. If f_stat fails, return 404 response.
          b. Else, continue
        5. Fill in 200 OK error code
        6. Fill in Content Length based on f_stat results
        7. Fill in Content Type
        8. Fill in Transfer-Encoding
        9. Fill in HTTP_TRAILER to begin data section
       10. Max bytes of the file to read:
             *resSize - (bytesWrittenSoFar - 4)
               or
             file length, whichever is shorter.
           Subtract 2 bytes for chunk size and 2 bytes for \r\n.
       11. Read bytes from file into buffer.
       12. Fill in chunk header based on bytes read
       13. Fill in chunk data
*/
int WebServer::processRequest(
    const char *request,
    size_t reqSize,
    char *response,
    size_t *resSize) {
  if (0 == request || 0 == reqSize || 0 == response || 0 == resSize || 0 == *resSize) {
    return 2;
  }
  
  //temporary buffer used for incrementally filling response buffer
  char *tmpResponse = response;
  //temporary pointer used for tokenizing request
  char *tmpRequest = (char*)malloc(strlen(request) + 1);
  char *tmpRequestCopy = tmpRequest;
  strcpy(tmpRequest, request);
  
  //get request type
  char *p = strtok(tmpRequestCopy, " ");
  //for some reason strncasecmp_P was giving me problems, so GET_REQUEST is not a prog_char
  int res = strncasecmp_P(p, GET_REQUEST, min(strlen(p), strlen("GET")));
  if (0 != res) {
    free(tmpRequest);
    return 2;
  }
  
  //get file path
  p = strtok(NULL, " ");
  
  FILINFO info;
  char *path = 0;
  if (0 == strcmp(p, "/")) {
    path = "index.html";
  }
  else {
    path = p;
  }
  
  FRESULT fresult = m_fileSystem.myf_stat(path, &info);
  
  //if stat failed, assume the file does not exist
  if (FR_OK != fresult) {
    strcpy_P(response, ERR_404);
    *resSize = strlen(ERR_404);
    free(tmpRequest);
    return 0;
  }
  
  fresult = m_fileSystem.myf_open(&m_file, path, FA_READ);
  //don't need this anymore after this
  free(tmpRequest);
  path = 0;
  
  if (FR_OK != fresult) {
    strcpy_P(response, ERR_500);
    *resSize = strlen(ERR_500);
    memset(&m_file, 0, sizeof(FIL));
    return 0;
  }
  
  //found file, response 200
  strcpy_P(tmpResponse, ERR_200);
  tmpResponse += strlen_P(ERR_200);
  strcpy_P(tmpResponse, HTTP_TRAILER);
  tmpResponse += strlen_P(HTTP_TRAILER);
  
  //fill in Content-Length
  int written = sprintf(tmpResponse, "Content-Length: %lu", info.fsize);
  tmpResponse += written;
  strcpy_P(tmpResponse, HTTP_TRAILER);
  tmpResponse += strlen_P(HTTP_TRAILER);
  
  //fill in Content-Type
  strcpy_P(tmpResponse, CONTENT_TYPE);
  tmpResponse += strlen_P(CONTENT_TYPE);
  strcpy_P(tmpResponse, HTTP_TRAILER);
  tmpResponse += strlen_P(HTTP_TRAILER);
  
  //fill in Transfer-Encoding
  strcpy_P(tmpResponse, TRANSFER_ENCODING);
  tmpResponse += strlen_P(TRANSFER_ENCODING);
  strcpy_P(tmpResponse, HTTP_TRAILER);
  tmpResponse += strlen_P(HTTP_TRAILER);
  
  //finished with header, start data
  strcpy_P(tmpResponse, HTTP_TRAILER);
  tmpResponse += strlen_P(HTTP_TRAILER);
  
  //subtract 2 bytes for hex size, 2 bytes for HTTP_TRAILER
  size_t bytesFilled = tmpResponse-response;
  size_t bytesRemaining = *resSize - bytesFilled;
  
  int retval = 1;
  if (0 < bytesRemaining) {
    retval = readRemaining(tmpResponse, &bytesRemaining);
  }
  
  *resSize = bytesFilled + bytesRemaining;
  
  return retval;
}

int WebServer::readRemaining(
    char *response,
    size_t *resSize)
{ 
  int retval = 0;
  char *tmpResponse = response;
  size_t bytesFilled = 0;
  
  if (0 == response || 0 == resSize || 0 == *resSize) {
    return 2;
  }
  
  size_t bytesToRead = *resSize - bytesFilled;
  do
  {
    bytesToRead = *resSize - bytesFilled;
    
    retval = chunkedFileRead(tmpResponse, &bytesToRead);
    bytesFilled += bytesToRead;
    tmpResponse += bytesToRead;
  } while(bytesToRead > 0 && retval == 1 && bytesFilled < *resSize);
  
  *resSize = bytesFilled;
  
  return retval;
}

int WebServer::chunkedFileRead(
  char *outBuf,
  size_t *resSize)
{
  size_t bytesRead = 0;
  char *tmpOutBuf = outBuf;
  int retval = 0;
  
  if (!m_nextCallFinalize && 0 == m_file.fs) {
    *resSize = 0;
    return 2;
  }
  
  if (m_nextCallFinalize) {
    if (0 == resSize || strlen_P(FINAL_CHUNK) > *resSize) {
      return 2;
    }
    
    *resSize = strlen_P(FINAL_CHUNK);
    strcpy_P(outBuf, FINAL_CHUNK);
    m_nextCallFinalize = false;
    return 0;
  }
  
  //if resSize is less than the number of bytes we're about to read, just return
  //and tell them to call us again with a bigger buffer
  if (*resSize < (min(m_file.fsize, MIN_WEBSERVER_BUF_SIZE))) {
    //wait until next call to start writing chunks
    *resSize = 0;
    return 1;
  }
  
  //fill a temporary buffer with the file contents 
  if (FR_OK != m_fileSystem.myf_read(&m_file, m_tmpFileBuf, MIN_WEBSERVER_BUF_SIZE, &bytesRead)) {
    m_fileSystem.myf_close(&m_file);
    return 2;
  }
  
  if (MIN_WEBSERVER_BUF_SIZE == bytesRead) {
    //more bytes possibly remaining
    retval = 1;
  }
  else {
    m_fileSystem.myf_close(&m_file);
    memset(&m_file, 0, sizeof(FIL));
    retval = 1;
    m_nextCallFinalize = true;
  }
  
  //fill in chunk header
  sprintf(tmpOutBuf, "%02X", bytesRead);
  tmpOutBuf += 2;
  strcpy_P(tmpOutBuf, HTTP_TRAILER);
  tmpOutBuf += strlen_P(HTTP_TRAILER);
  
  //fill in chunk data
  memcpy(tmpOutBuf, m_tmpFileBuf, bytesRead);
  tmpOutBuf += bytesRead;
  
  //only append the trailer if we have read all the data for this chunk
  if (0 == retval || m_nextCallFinalize) {
	  //append trailer
	  strcpy_P(tmpOutBuf, HTTP_TRAILER);
	  tmpOutBuf += strlen_P(HTTP_TRAILER);
  }
  
  *resSize = tmpOutBuf-outBuf;
  
  return retval;
}