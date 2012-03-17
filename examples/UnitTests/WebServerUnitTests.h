#ifndef TESTSUITE_H
#define TESTSUITE_H

void setUp(void);
void tearDown(void);
void test_processRequest_HandlesNullRequest(void);
void test_processRequest_HandlesZeroLengthRequest(void);
void test_processRequest_SimpleGetRequest(void);
void test_processRequest_GetRequestBufferTooSmall(void);
void test_processRequest_Returns404WhenFileNotFound(void);
void test_processRequest_MultipleChunks(void);
void test_processRequest_GetAFileOtherThanRoot(void);

#endif
