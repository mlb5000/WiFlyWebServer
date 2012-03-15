/*
  UnitTests
  Created 2012-03-10 by Matt Baker
*/

#include <ArduinoFreeRTOS.h>
#include <RTOSSerial.h>
#include "WebServer.h"
#include "unity.h"
#include "WebServerUnitTests.h"
#include <stdlib.h> // for malloc and free

void* operator new(size_t size) { return malloc(size); }
void operator delete(void* ptr) { free(ptr); }
  
void *task1_handle;

/** @brief RTOSSerial port replacement for Serial */
RTOSSerialPort0(Serial);

RTOSSerialPort2(Serial2);

int serialWrite(char c, FILE *f) {
    Serial.write(c);
    return 0;
}

void setup () {
  Serial.begin(115200);
  Serial2.begin(9800);
  
  stdout = stderr = fdevopen(serialWrite, NULL);
  
  /* Create binary semaphore used to protect printing resource.
   * A binary semaphore is acceptable because it is only used from
   * two tasks, and therefore cannot create a priority inversion. */
  //vSemaphoreCreateBinary(printing_semphr);
  /* Start task1 and task2 with the a stack size of 200, no arguments,
   * and the same priority level (1). This will allow them to round robin
   * on vPortYield.
   */
  xTaskCreate(task1_func, (signed portCHAR *)"task1", 200,
              NULL, 1, &task1_handle);
              
  vTaskStartScheduler();
  /* code after vTaskStartScheduler, and code in loop(), is never reached. */
}

void loop() {
}

void task1_func(void *params) {
  Serial.println("Running Unity Tests");
  Serial.println("");
  
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
  //RUN_TEST(test_processRequest_MultipleChunks, 0);

  UnityEnd();
  
  for(;;) {}
}