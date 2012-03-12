#ifndef TESTSUITE_H
#define TESTSUITE_H

void setUp(void);
void tearDown(void);
void test_processRequest_HandlesNullRequest(void);
void test_processRequest_HandlesZeroLengthRequest(void);
void test_processRequest_HandlesNullResponseBuffer(void);
void test_processRequest_HandlesZeroLengthResponseBuffer(void);
void test_processRequest_HandlesNullResponseLength(void);
void test_processRequest_SimpleGetRequest(void);
void test_processRequest_GetRequestBufferTooSmall(void);
void test_readRemaining_ReturnsFailWhenNoActiveRequest(void);
void test_readRemaining_HandlesNullResponseBuffer(void);
void test_readRemaining_HandlesZeroLengthResponseBuffer(void);
void test_readRemaining_HandlesNullResponseLength(void);
void test_readRemaining_AfterAllReadReturnsFailed(void);

#endif
