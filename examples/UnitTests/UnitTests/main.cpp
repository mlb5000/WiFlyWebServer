#include "unity.h"
#include "WebServerUnitTests.h"

int main()
{
	UnityBegin();

	Unity.TestFile = "UnitTests/WebServerUnitTests.cpp";
	RUN_TEST(test_processRequest_HandlesNullRequest, 0);
	RUN_TEST(test_processRequest_HandlesZeroLengthRequest, 0);
	RUN_TEST(test_processRequest_SimpleGetRequest, 0);
	RUN_TEST(test_processRequest_GetRequestBufferTooSmall, 0);
	RUN_TEST(test_processRequest_Returns404WhenFileNotFound, 0);
	RUN_TEST(test_processRequest_MultipleChunks, 0);
  RUN_TEST(test_processRequest_GetAFileOtherThanRoot, 0);

	UnityEnd();

	getc(stdin);
}