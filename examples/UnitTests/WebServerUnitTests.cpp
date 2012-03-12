#include "unity.h"
#include <WiFlyWebServer.h>
#include "WebServerUnitTests.h"

WebServer webServer;
#define MAX_RESPONSE 200
size_t size = MAX_RESPONSE;
char response[MAX_RESPONSE] = {0};

static const char* ROOT_GET_REQUEST = "GET / HTTP/1.1";

void setUp(void)
{
  webServer = WebServer();
  size = MAX_RESPONSE;
  memset(response, 0, MAX_RESPONSE);
}
 
void tearDown(void)
{
  
}

/** @test Test correct behavior when NULL is passed in for request */
void test_processRequest_HandlesNullRequest()
{
  int result = 0;
  
  result = webServer.processRequest(0, 10, response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when 0 is passed in for request size */
void test_processRequest_HandlesZeroLengthRequest()
{
  int result = 0;
  
  result = webServer.processRequest(ROOT_GET_REQUEST, 0, response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for response buffer */
void test_processRequest_HandlesNullResponseBuffer()
{
  int result = 0;
  
  result = webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), 0, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when 0 is passed in for response buffer size */
void test_processRequest_HandlesZeroLengthResponseBuffer()
{
  int result = 0;
  
  size = 0;
  result = webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for request length
    output parameter
*/
void test_processRequest_HandlesNullResponseLength()
{
  int result = 0;
  
  result = webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, 0);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @brief Configure the FileSystemMock to have a fake file at the provided
    path with the provided contents
*/
void givenPageContents(const char *path, const char *fileContents)
{
  //add a fake mock file at path with fileContents
}

/** @test Test correct behavior when a simple 'GET /' request is received */
void test_processRequest_SimpleGetRequest()
{
  static const char *expected = "HTTP/1.1 200 OK\r\n" \
  "Content-Length: 27\r\n" \
  "Content-Type text/html; charset=utf-8\r\n" \
  "Transfer-Encoding: chunked\r\n\r\n" \
  "1B\r\n" \
  "these are the page contents\r\n" \
  "0\r\n\r\n";
  int result = 0;
  
  givenPageContents("/", "these are the page contents");
  
  size = 144;
  result = webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  
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
  "Transfer-Encoding: chunk";
  
  static const char *expected2 = "ed\r\n\r\n" \
  "1B\r\n" \
  "these are the page contents\r\n" \
  "0\r\n\r\n";
  int result = 0;
  
  givenPageContents("/", "these are the page contents");
  
  size = 100; //make buffer too small to fit it all
  result = webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  
  TEST_ASSERT_EQUAL_INT(1, result);
  TEST_ASSERT_EQUAL_INT(strlen(expected), size);
  
  TEST_ASSERT_EQUAL_UINT8_ARRAY((uint8_t *)expected, response, size);
  
  //read remaining bytes
  result = webServer.readRemaining(response, &size);
  
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
  
  result = webServer.readRemaining(response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for the response
    output buffer
*/
void test_readRemaining_HandlesNullResponseBuffer(void)
{
  int result = 0;
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 100; //make buffer too small to fit it all
  webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  size = 100;
  result = webServer.readRemaining(0, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when 0 is passed in for request output
    buffer length
*/
void test_readRemaining_HandlesZeroLengthResponseBuffer(void)
{
  int result = 0;
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 100; //make buffer too small to fit it all
  webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  size = 0;
  result = webServer.readRemaining(response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when NULL is passed in for request length
    output parameter
*/
void test_readRemaining_HandlesNullResponseLength(void)
{
  int result = 0;
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 100; //make buffer too small to fit it all
  webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  result = webServer.readRemaining(response, 0);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}

/** @test Test correct behavior when readRemaining is called after all bytes
    for this response have been read.
*/
void test_readRemaining_AfterAllReadReturnsFailed(void)
{
  int result = 0;
  
  //must call processRequest to avoid making this identical to
  //the "No active request" test case
  size = 100; //make buffer too small to fit it all
  webServer.processRequest(ROOT_GET_REQUEST, strlen(ROOT_GET_REQUEST), response, &size);
  size = 100;
  webServer.readRemaining(response, &size);
  size = 100;
  result = webServer.readRemaining(response, &size);
  
  TEST_ASSERT_EQUAL_INT(2, result);
}