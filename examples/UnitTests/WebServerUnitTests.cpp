#include "unity.h"
#include <WiFlyWebServer.h>
#include "WebServerUnitTests.h"
#include "FileSystemMock.h"

FileSystemMock *fileSystem;
WebServer *webServer;

#define MAX_RESPONSE 200
size_t size = MAX_RESPONSE;
char response[MAX_RESPONSE] = {0};

static const char* ROOT_GET_REQUEST = "GET / HTTP/1.1";

void setUp(void)
{
  fileSystem = new FileSystemMock();
  webServer = new WebServer(*fileSystem);
  
  size = MAX_RESPONSE;
  memset(response, 0, MAX_RESPONSE);
}
 
void tearDown(void)
{
  delete fileSystem;
  delete webServer;
}

/** @test Test correct behavior when NULL is passed in for request */
void test_processRequest_HandlesNullRequest()
{
  int result = 0;
  
  result = webServer->processRequest(0, 10, response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when 0 is passed in for request size */
void test_processRequest_HandlesZeroLengthRequest()
{
  int result = 0;
  
  result = webServer->processRequest(ROOT_GET_REQUEST, 0, response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for response buffer */
void test_processRequest_HandlesNullResponseBuffer()
{
  int result = 0;
  
  result = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), 0, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when 0 is passed in for response buffer size */
void test_processRequest_HandlesZeroLengthResponseBuffer()
{
  int result = 0;
  
  size = 0;
  result = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for request length
    output parameter
*/
void test_processRequest_HandlesNullResponseLength()
{
  int result = 0;
  
  result = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, 0);
  
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
  
  size = strlen(expected);
  result = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  
  TEST_ASSERT_EQUAL_INT(0, result);
  TEST_ASSERT_EQUAL_INT(strlen(expected), size);
  
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected, response, size >= MAX_RESPONSE ? MAX_RESPONSE : size);
}

/** @test Test that buffers can be reused if the response is larger
    than the output buffer.
*/
void test_processRequest_GetRequestBufferTooSmall(void)
{
  static const char *expected = "HTTP/1.1 200 OK\r\n" \
  "Content-Length: 27\r\n" \
  "Content-Type text/html; charset=utf-8\r\n" \
  "Transfer-Encoding: chunked\r\n\r\n";
  
  static const char *expected2 = "1B\r\n" \
  "these are the page contents\r\n" \
  "0\r\n\r\n";
  int result = 0;
  
  givenPageContents("index.html", "these are the page contents");
  
  size = 106; //make buffer too small to fit chunks, but just enough to fit header
  result = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  
  TEST_ASSERT_EQUAL_INT(1, result);
  TEST_ASSERT_EQUAL_INT(strlen(expected), size);
  
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected, response, size);
  
  //read remaining bytes
  result = webServer->readRemaining(response, &size);
  
  TEST_ASSERT_EQUAL_INT(0, result);
  TEST_ASSERT_EQUAL_INT(strlen(expected2), size);
  
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected2, response, size);
}

/** @test Test correct behavior readRemaining is called when there is no
    active request
*/
void test_readRemaining_ReturnsFailWhenNoActiveRequest(void)
{
  int result = 0;
  
  result = webServer->readRemaining(response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for the response
    output buffer
*/
void test_readRemaining_HandlesNullResponseBuffer(void)
{
  int result = 0;
  
  givenPageContents("index.html", "these are the page contents");
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 100; //make buffer too small to fit it all
  webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  size = 100;
  result = webServer->readRemaining(0, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when 0 is passed in for request output
    buffer length
*/
void test_readRemaining_HandlesZeroLengthResponseBuffer(void)
{
  int result = 0;
  
  givenPageContents("index.html", "these are the page contents");
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 100; //make buffer too small to fit it all
  webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  size = 0;
  result = webServer->readRemaining(response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for request length
    output parameter
*/
void test_readRemaining_HandlesNullResponseLength(void)
{
  int result = 0;
  
  givenPageContents("index.html", "these are the page contents");
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 100; //make buffer too small to fit it all
  webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  result = webServer->readRemaining(response, 0);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when readRemaining is called after all bytes
    for this response have been read.
*/
void test_readRemaining_AfterAllReadReturnsFailed(void)
{
  int result = 0;
  
  givenPageContents("index.html", "these are the page contents");
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 106; //make buffer too small to fit it all
  webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  size = MAX_RESPONSE;
  webServer->readRemaining(response, &size);
  size = MAX_RESPONSE;
  result = webServer->readRemaining(response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test that web server can successfully return a 404 error */
void test_processRequest_Returns404WhenFileNotFound()
{
  const char *expected = "HTTP/1.1 404 File Not Found\r\n";
  
  givenPageContents("index1.html", "these are the page contents");
  
  int retval = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  
  TEST_ASSERT_EQUAL_INT(0, retval);
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected, response, size); 
}

void test_processRequest_MultipleChunks()
{
  static const char *expected = "HTTP/1.1 200 OK\r\n" \
  "Content-Length: 129\r\n" \
  "Content-Type text/html; charset=utf-8\r\n" \
  "Transfer-Encoding: chunked\r\n\r\n";
  
  /*static const char *expected2 = "80\r\n" \
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus vel vestibulum velit. Praesent sagittis mi leo. Nam lorem sed";
  
  static const char *expected3 = ".\r\n0\r\n\r\n";*/
  
  givenPageContents("index.html", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus vel vestibulum velit. Praesent sagittis mi leo. Nam lorem sed.");
  
  //givenPageContents("index.html", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Phasellus vel vestibulum velit.");
  
  int retval = webServer->processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  TEST_ASSERT_EQUAL_INT(1, retval);
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected, response, size);
  
  /*size = MAX_RESPONSE;
  retval = webServer->readRemaining(response, &size);
  TEST_ASSERT_EQUAL_INT(1, retval);
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected2, response, size);
  
  size = MAX_RESPONSE;
  retval = webServer->readRemaining(response, &size);
  TEST_ASSERT_EQUAL_INT(0, retval);
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected3, response, size);*/
}