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
#define sprintf_P sprintf
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
static const char CONTENT_LENGTH_FMT[] PROGMEM = {"Content-Length: %lu"};

WebServer::WebServer(FileSystem &fileSystem, Network &network)
: m_fileSystem(fileSystem),
  m_network(network)
{}

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
    size_t reqSize) {
  //just define variables up front to support possible C99 transition in the future
  char *tmpRequest = 0;
  char *tmpRequestCopy = 0;
  char *p = 0;
  int res = 0;
  FILINFO info;
  FIL file;
  char *path = 0;
  FRESULT fresult;
  int written;

  if (0 == request || 0 == reqSize) {
    return 2;
  }

  //temporary pointer used for tokenizing request
  tmpRequest = (char*)malloc(strlen(request) + 1);
  tmpRequestCopy = tmpRequest;
  strcpy(tmpRequest, request);
  
  //get request type
  p = strtok(tmpRequestCopy, " ");
  //for some reason strncasecmp_P was giving me problems, so GET_REQUEST is not a prog_char
  res = strncasecmp_P(p, GET_REQUEST, min(strlen(p), strlen("GET")));
  if (0 != res) {
    free(tmpRequest);
    return 2;
  }
  
  //get file path
  p = strtok(NULL, " ");
  
  if (0 == strcmp(p, "/")) {
    path = "index.html";
  }
  else {
    path = p;
  }
  
  fresult = m_fileSystem.myf_stat(path, &info);
  
  //if stat failed, assume the file does not exist
  if (FR_OK != fresult) {
    netWrite_P(ERR_404, strlen_P(ERR_404));
    free(tmpRequest);
    return 0;
  }
  
  fresult = m_fileSystem.myf_open(&file, path, FA_READ);
  //don't need this anymore after this
  free(tmpRequest);
  path = 0;
  
  if (FR_OK != fresult) {
    netWrite_P(ERR_500, strlen_P(ERR_500));
    return 0;
  }
  
  //found file, response 200
  netWrite_P(ERR_200, strlen_P(ERR_200));
  netWrite_P(HTTP_TRAILER, strlen(HTTP_TRAILER));

  //fill in Content-Length
  char tmpBuf[30];
  written = sprintf(tmpBuf, CONTENT_LENGTH_FMT, info.fsize);
  netWrite_P(tmpBuf, written);
  netWrite_P(HTTP_TRAILER, strlen(HTTP_TRAILER));

  //fill in Content-Type
  netWrite_P(CONTENT_TYPE, strlen(CONTENT_TYPE));
  netWrite_P(HTTP_TRAILER, strlen(HTTP_TRAILER));
  
  //fill in Transfer-Encoding
  netWrite_P(TRANSFER_ENCODING, strlen(TRANSFER_ENCODING));
  netWrite_P(HTTP_TRAILER, strlen(HTTP_TRAILER));
  
  //finished with header, start data
  netWrite_P(HTTP_TRAILER, strlen(HTTP_TRAILER));

  chunkedFileRead(&file);

  return 0;
}

void WebServer::chunkedFileRead(FIL *file)
{
  size_t bytesRead = 0;
  int retval = 0;
  size_t tmp = 0;
  char tmpBuf[10];
  int written;

  if (0 == file) {
    return;
  }
  
  do 
  {
    m_fileSystem.myf_read(file, m_tmpFileBuf, TMP_FILE_BUF_SIZE, &bytesRead);

    //write chunk header
    written = sprintf(tmpBuf, "%02X", bytesRead);
    netWrite(tmpBuf, written);
    netWrite_P(HTTP_TRAILER, strlen_P(HTTP_TRAILER));

    //write chunk data
    netWrite(m_tmpFileBuf, bytesRead);
    netWrite_P(HTTP_TRAILER, strlen_P(HTTP_TRAILER));
  } while (TMP_FILE_BUF_SIZE == bytesRead);

  m_fileSystem.myf_close(file);

  //write final chunk
  netWrite_P(FINAL_CHUNK, strlen_P(FINAL_CHUNK));
}

void WebServer::netWrite_P(const char *toWrite, size_t size) {
  size_t i;

  for (i = 0; i < size; i++) {
    m_network.writeByte(toWrite[i]);
  }
}

void WebServer::netWrite(const char *toWrite, size_t size) {
  size_t i;

  for (i = 0; i < size; i++) {
    m_network.writeByte(toWrite[i]);
  }
}
