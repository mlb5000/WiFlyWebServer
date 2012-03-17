#include "unity.h"
#include <WiFlyWebServer.h>
#include "WebServerUnitTests.h"
#include "FileSystemMock.h"
#include "NetworkMock.h"

FileSystemMock *fileSystem;
WebServer *webServer;
NetworkMock *network;

typedef unsigned char uint8_t;

static const char* ROOT_GET_REQUEST = "GET / HTTP/1.1";

void setUp(void)
{
  fileSystem = new FileSystemMock();
  network = new NetworkMock();
  webServer = new WebServer(*fileSystem, *network);
}
 
void tearDown(void)
{
  delete fileSystem;
  delete webServer;
  delete network;
}

/** @test Test correct behavior when NULL is passed in for request */
void test_processRequest_HandlesNullRequest()
{
  int result = 0;
  
  result = webServer->processRequest(0, 10);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when 0 is passed in for request size */
void test_processRequest_HandlesZeroLengthRequest()
{
  int result = 0;
  
  result = webServer->processRequest(ROOT_GET_REQUEST, 0);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @brief Configure the FileSystemMock to have a fake file at the provided
    path with the provided contents
*/
void givenPageContents(const char *path, const char *fileContents)
{
  //add a fake mock file at path with fileContents
  fileSystem->addFakeFile((XCHAR*)path, (XCHAR*)fileContents);
}

#define TEST_ASSERT_EQUAL_NETWORK_BYTES(expected, num_elements)                              UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, network->getBytesWritten(), num_elements, __LINE__, NULL)

/** @test Test correct behavior when a simple 'GET /' request is received */
void test_processRequest_SimpleGetRequest()
{
  //17, 20, 39, 30
  static const char *expected = "HTTP/1.1 200 OK\r\n" \
  "Content-Length: 27\r\n" \
  "Content-Type text/html; charset=utf-8\r\n" \
  "Transfer-Encoding: chunked\r\n\r\n" \
  "1B\r\n" \
  "these are the page contents\r\n" \
  "0\r\n\r\n";
  int result = 0;
  
  givenPageContents("index.html", "these are the page contents");
  
  result = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST));
  
  TEST_ASSERT_EQUAL_INT(0, result);
  TEST_ASSERT_EQUAL_INT(strlen(expected), network->getNumBytesWritten());
  TEST_ASSERT_EQUAL_NETWORK_BYTES((uint8_t *)expected, strlen(expected));
}

/** @test Test that buffers can be reused if the response is larger
    than the output buffer.
*/
void test_processRequest_GetRequestBufferTooSmall(void)
{
  static const char *expected = "HTTP/1.1 200 OK\r\n" \
  "Content-Length: 27\r\n" \
  "Content-Type text/html; charset=utf-8\r\n" \
  "Transfer-Encoding: chunked\r\n\r\n" \
  "1B\r\n" \
  "these are the page contents\r\n" \
  "0\r\n" \
  "\r\n";
  int result = 0;
  
  givenPageContents("index.html", "these are the page contents");
  
  result = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST));
  
  TEST_ASSERT_EQUAL_INT(0, result);
  TEST_ASSERT_EQUAL_INT(strlen(expected), network->getNumBytesWritten());
  TEST_ASSERT_EQUAL_NETWORK_BYTES((uint8_t *)expected, strlen(expected));
}

/** @test Test that web server can successfully return a 404 error */
void test_processRequest_Returns404WhenFileNotFound()
{
  const char *expected = "HTTP/1.1 404 File Not Found\r\n";
  
  givenPageContents("index1.html", "these are the page contents");
  
  int retval = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST));
  
  TEST_ASSERT_EQUAL_INT(0, retval);
  TEST_ASSERT_EQUAL_INT(strlen(expected), network->getNumBytesWritten());
  TEST_ASSERT_EQUAL_NETWORK_BYTES((uint8_t *)expected, strlen(expected));
}

/** @test Test that web server can handle splitting a file that is larger than
    a single buffer into multiple chunks with the correct trailer.
*/
void test_processRequest_MultipleChunks()
{
  static const char *expected = "HTTP/1.1 200 OK\r\n" \
  "Content-Length: 129\r\n" \
  "Content-Type text/html; charset=utf-8\r\n" \
  "Transfer-Encoding: chunked\r\n\r\n" \
  "80\r\n" \
  "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus vel vestibulum velit. Praesent sagittis mi leo. Nam lorem sed\r\n" \
  "01\r\n" \
  ".\r\n" \
  "0\r\n" \
  "\r\n";
  
  givenPageContents("index.html", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus vel vestibulum velit. Praesent sagittis mi leo. Nam lorem sed.");
  
  int retval = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST));

  TEST_ASSERT_EQUAL_INT(0, retval);
  TEST_ASSERT_EQUAL_INT(strlen(expected), network->getNumBytesWritten());
  TEST_ASSERT_EQUAL_NETWORK_BYTES((uint8_t *)expected, strlen(expected));
}

void test_processRequest_GetAFileOtherThanRoot()
{
  static const char *expected = "HTTP/1.1 200 OK\r\n" \
    "Content-Length: 129\r\n" \
    "Content-Type text/html; charset=utf-8\r\n" \
    "Transfer-Encoding: chunked\r\n\r\n" \
    "80\r\n" \
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus vel vestibulum velit. Praesent sagittis mi leo. Nam lorem sed\r\n" \
    "01\r\n" \
    ".\r\n" \
    "0\r\n" \
    "\r\n";
  const char *request = "GET /some/path/to/some/other/file.html HTTP/1.1";

  givenPageContents("/some/path/to/some/other/file.html", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus vel vestibulum velit. Praesent sagittis mi leo. Nam lorem sed.");

  int retval = webServer->processRequest(request, strlen(request));

  TEST_ASSERT_EQUAL_INT(0, retval);
  TEST_ASSERT_EQUAL_INT(strlen(expected), network->getNumBytesWritten());
  TEST_ASSERT_EQUAL_NETWORK_BYTES((uint8_t *)expected, strlen(expected));
}