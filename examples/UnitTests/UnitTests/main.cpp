#include "unity.h"
#include "WebServerUnitTests.h"

int main()
{
	UnityBegin();

	Unity.TestFile = "UnitTests/WebServerUnitTests.cpp";
	RUN_TEST(test_processRequest_HandlesNullRequest, 0);
	RUN_TEST(test_processRequest_HandlesZeroLengthRequest, 0);
	RUN_TEST(test_processRequest_HandlesNullResponseBuffer, 0);
	RUN_TEST(test_processRequest_HandlesZeroLengthResponseBuffer, 0);
	RUN_TEST(test_processRequest_HandlesNullResponseLength, 0);
	RUN_TEST(test_processRequest_SimpleGetRequest, 0);
	RUN_TEST(test_processRequest_GetRequestBufferTooSmall, 0);
	RUN_TEST(test_readRemaining_ReturnsFailWhenNoActiveRequest, 0);
	RUN_TEST(test_readRemaining_HandlesNullResponseBuffer, 0);
	RUN_TEST(test_readRemaining_HandlesZeroLengthResponseBuffer, 0);
	RUN_TEST(test_readRemaining_HandlesNullResponseLength, 0);
	RUN_TEST(test_readRemaining_AfterAllReadReturnsFailed, 0);
	RUN_TEST(test_processRequest_Returns404WhenFileNotFound, 0);
	RUN_TEST(test_processRequest_MultipleChunks, 0);

	UnityEnd();

	getc(stdin);
}