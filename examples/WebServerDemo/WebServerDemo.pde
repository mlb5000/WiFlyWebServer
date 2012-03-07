/* */

#include "WebServerDemo.h"
#include <ArduinoFreeRTOS.h>
#include <RTOSSerial.h>

/* Default pins to 12 & 13 for digital readout of current task. */
#define TASK1_PIN (12)
#define TASK2_PIN (13)

#define PRINTING_TIMEOUT ( (portTickType) 25 )

/** @brief RTOSSerial port replacement for Serial */
RTOSSerialPort0(Serial);

/** @brief RTOSSerial port replacement for Serial2 */
RTOSSerialPort2(Serial2);

/** @brief The maximum size of a Response from WiFly */
#define RESPONSE_SIZE 500
#define WIFLY Serial2

/** @brief the access point to connect to */
#define ACCESS_POINT "BAKERS"

/** @brief the passphrase to connect to the access point */
#define PASSPHRASE "skiliberty"

void *task1_handle;
void *task2_handle;
xSemaphoreHandle printing_semphr;

int sendCommand(const char *command, const char *expectedResponse, unsigned delay_ms=10);
void flushSerial(bool display=false, unsigned to_sec=3, unsigned delay_ms=10);

// Pins are 17 for INCOMING TO Arduino, 16 for OUTGOING TO Wifly
// Arduino       WiFly
//  17 - receive  TX   (Send from Wifly, Receive to Arduino)
//  16 - send     RX   (Send from Arduino, Receive to WiFly) 
//WiFlySerial WiFly(17,16); 

#define fp(string) Serial.printf_P(PSTR(string));
#define fpl(string) Serial.println_P(PSTR(string));

#define wfp(string) Serial2.printf_P(PSTR(string));

void setup () {
  /* Use the standard Arduino HardwareSerial library for serial. */
  Serial.begin(115200);
  WIFLY.begin(9600);
 
  fpl("Starting");
  pinMode(TASK1_PIN, OUTPUT);
  pinMode(TASK2_PIN, OUTPUT);
  digitalWrite(TASK1_PIN, LOW);
  digitalWrite(TASK2_PIN, LOW);
  
  fpl("Configuring WiFly module");
  configureWify();

  /* Create binary semaphore used to protect printing resource.
   * A binary semaphore is acceptable because it is only used from
   * two tasks, and therefore cannot create a priority inversion. */
  vSemaphoreCreateBinary(printing_semphr);
  /* Start task1 and task2 with the a stack size of 200, no arguments,
   * and the same priority level (1). This will allow them to round robin
   * on vPortYield.
   */
  xTaskCreate(task1_func, (signed portCHAR *)"task1", 200,
              NULL, 1, &task1_handle);
  /*xTaskCreate(task2_func, (signed portCHAR *)"task2", 200,
              NULL, 1, &task2_handle);*/
  fpl("Starting tasks");
  vTaskStartScheduler();
  /* code after vTaskStartScheduler, and code in loop(), is never reached. */
}

void loop () {
  fpl("Never reached");
}

/** @brief A WiFly configuration option */
typedef struct wiflyConfig_t {
  const prog_char *message;
  const prog_char *command;
  const prog_char *errorMessage;  
} wiflyConfig_t;

#define WIFLY_OPTION(num, msg1, cmd1, fail1) const prog_char msg##num[] PROGMEM = msg1; \
const prog_char cmd##num[] PROGMEM = cmd1; \
const prog_char fail##num[] PROGMEM = fail1;

WIFLY_OPTION(1, "Disabling echoback", "set u m 0", "Fai1ed to disable echoback");
WIFLY_OPTION(2, "Setting UART buffer size", "set c s 1420", "Failed to set buffer size");
WIFLY_OPTION(3, "Setting flush time", "set c t 10", "Failed to set flush time");
WIFLY_OPTION(4, "Don't send bytes when remote connects", "set c r 0", "Failed to disable remote talkback");
WIFLY_OPTION(5, "Disable open string", "set c o 0", "Failed to disable open string");
WIFLY_OPTION(6, "Enable DHCP", "set i d 1", "Failed to enable DHCP");
WIFLY_OPTION(7, "Enabling TCP/IP mode", "set i p 2", "Failed to set TCP/IP mode");
WIFLY_OPTION(8, "Enabling autojoin", "set w j 1", "Failed to enable autojoin");
WIFLY_OPTION(9, "Listen on port 80", "set i l 80", "Failed to configure listening port");
WIFLY_OPTION(10, "Setting passphrase", "set w p " \
PASSPHRASE, "Failed to set passphrase");
WIFLY_OPTION(11, "Setting access point", "set w s " \
ACCESS_POINT, "Failed to set access point");

/** @brief The configuration options to issue to the WiFly module (in order) */
wiflyConfig_t options[] = {
  {msg1, cmd1, fail1},
  {msg2, cmd2, fail2},
  {msg3, cmd3, fail3},
  {msg4, cmd4, fail4},
  {msg5, cmd5, fail5},
  {msg6, cmd6, fail6},
  {msg7, cmd7, fail7},
  {msg8, cmd8, fail8},
  {msg9, cmd9, fail9},
  {msg10, cmd10, fail10},
  {msg11, cmd11, fail11}
};

const char *AOK = "AOK";

/** @brief Configure the WiFly module

    Configures the WiFly module for AdHoc networking
*/
void configureWify() {
  unsigned i;
  
  fpl("Configuring the RN-171");
  
  //This initial flush gets us the current state of the WiFly module
  flushSerial(true);
  
  //turn on command mode (only possible on clean startup, reset won't work)
  if (0 != enableCommandMode()) {
    fpl("Failed to enable command mode");
    goto CLEANUP;
  }
  
  //speed up the UART from the default 9800
  wfp("set u i 230400\r");
  WIFLY.flush();
  delay(1000);
  WIFLY.begin(230400);
  
  for (i = 0; i < sizeof(options) / sizeof(wiflyConfig_t); i++) {
    fp("  ");
    Serial.printf_P(options[i].message);
    fp(": ");
    if (0 != sendCommand(options[i].command, AOK)) {
      fp("    ");
      Serial.printf_P(options[i].errorMessage);
      fpl("    Retrying...");
      flushSerial();
      delay(1000);
      if (0 != sendCommand(options[i].command, AOK, 50)) {
        fpl("    Still failed...");
        goto CLEANUP; 
      }
      else {
        fpl("    Success!");
      }
    }
    else {
      fpl("success");
    }
  }

CLEANUP:
  if (0 != disableCommandMode()) {
    fpl("Failed to disable command mode, bad things may happen");
    return;
  }
}

/** @brief Pulls all bytes off of the serial line (WIFLY)

    @param display Whether or not to display the characters [default=false]
    @param to_sec The number of seconds to wait for characters [default=3]
*/
void flushSerial(bool display, unsigned to_sec, unsigned delay_ms) {
  unsigned long begin = millis();
  char read = 0;
  
  while (!WIFLY.available()) {
    delay(100);
    
    if ((millis()-begin)/1000 > to_sec) {
      return;
    }
  }
  
  while (WIFLY.available()) {
    read = WIFLY.read();
    if (display) {
      Serial.print(read);
    }
    delay(delay_ms);
  }
}

/** @brief Sends a command over UART to the WiFly module
    
    @param command The command to issue (e.g. set ip address 192.168.1.1)
    @param expectedResponse The response expected from this command (usually "AOK")
    @param delay_my The number of milliseconds to delay between UART reads
    
    @pre enableCommandMode must be called first and succeed
    
    @retval 0 Success
    @retval -1 Command failed
    @retval -2 NULL argument
*/
int sendCommand(const prog_char *command, const char *expectedResponse, unsigned delay_ms) {
  unsigned size = RESPONSE_SIZE;
  char response[RESPONSE_SIZE];
  int retval = 0;
  
  if (NULL == command || NULL == expectedResponse) {
    retval = -2;
  }
  
  if (0 == retval) {
    WIFLY.printf_P(command);
    wfp("\r"); //commit command
  }
  
  if (0 == retval && -1 == readFromWiFly(response, &size, 10, delay_ms)) {
    fpl("Timed out while reading from WiFly");
    retval = -1;
  }
  
  if (0 == retval) {  
    if (0 == strstr(response, expectedResponse)) {
      fpl("Did not receive expected response: ");
      Serial.println(expectedResponse);
      retval = -1;
    }
  }
  
  return retval;
}

/** @brief Enable WiFly command mode

    @pre Will only work during initial hard startup, resets will not work
    
    @retval 0 Success
    @retval -1 Failed to enable command mode
*/
int enableCommandMode() {
  unsigned size = RESPONSE_SIZE;
  char response[RESPONSE_SIZE];
  int retval = 0;
  int maxTries = 3;
  int tries = 0;
  
  do {
    tries++;
    wfp("$$$");
    WIFLY.flush();
    delay(275); //WiFly requires a 250ms delay following this sequence
    if (0 == retval && -1 == readFromWiFly(response, &size, 10, 100)) {
      fpl("Timed out while reading from WiFly");
      retval = -1;
    }

    if (response[0] != 'C' || response[1] != 'M' || response[2] != 'D') {
      fpl("Did not receive expected CMD response");
      retval = -1;
    } 
  } while(retval != 0 && tries < maxTries);
  
  return retval;
}

const prog_char exitCmd[] = "exit\r";

/** @brief Disable WiFly command mode

    @retval 0 Success
    @retval -1 Failure
*/
int disableCommandMode() {
  return sendCommand(exitCmd, "EXIT");
}

/** @brief Read a command over WiFly (i.e. WIFLY)
    @param [out] outBuf The buffer which will contain the output
    @param [in/out] bufSize The maximum size of outBuf on in, number of bytes written on out
    @param [opt] to_secs The maximum number of seconds to wait for a response (infinite by default)
    @param [in] delay_ms The number of milliseconds to delay between UART reads
    
    @retval 0 Success
    @retval -1 Timed out
    @retval -2 NULL argument
*/ 
int readFromWiFly(char *outBuf, unsigned *bufSize, unsigned to_secs, unsigned delay_ms) {
  unsigned long begin = millis();
  unsigned received = 0;
  char receivedByte = 0;
  int retval = 0;
  
  if (NULL == outBuf || NULL == bufSize) {
    retval = -2;
  }
  
  if (retval == 0) {
    while(!WIFLY.available()) {
      delay(10);
      if ((millis()-begin)/1000 > to_secs) {
        retval = -1;
      }
    } 
  }
  
  if (retval == 0) {
    while (WIFLY.available() && received < *bufSize-1) {
      receivedByte = WIFLY.read();
      outBuf[received] = receivedByte;
      received++;
      
      delay(delay_ms);
      
      if ((millis()-begin) / 1000 > to_secs) {
        retval = -1;
        fpl("Timed out");
        break;
      }
    }
    outBuf[received+1] = '\0';
    *bufSize = received;
  }
  
  return retval;
}

/* Task func is a void(void *). We passed NULL for params in xTaskCreate,
 * so we ignore them.
 * task1_func will be entered once after the scheduler begins. It can perform
 * any required setup and then loop indefinitely.
 * If task1_func were to return, you should let the scheduler know by calling
 * vTaskDelete. */
void task1_func(void *params)
{
  /* Ignoring the semaphore used to protect printing for now. */
  fpl("1: Entering Task");
  /* In the same way arduino would call loop(), we'll call task1_loop. */
  for(;;) {
    task1_loop();
  }
}

void task1_loop() {
  unsigned size = RESPONSE_SIZE;
  char response[RESPONSE_SIZE];
  int result = 0;
  
  result = readFromWiFly(response, &size, 10, 100);
  
  if (NULL != strstr(response, "GET /")) {
    fpl("     GET Request");
    handleGetRequest();
  }
  
  /* Take semaphore to use printing resource */
  if (xSemaphoreTake( printing_semphr, PRINTING_TIMEOUT ) == pdTRUE ) {
    /* Trivial task: set pin high, print, set pin low*/
    digitalWrite(TASK1_PIN, HIGH);
    if (result == 0) {
      Serial.print(response); 
    }
    fpl("1: Task Loop");
    digitalWrite(TASK2_PIN, LOW);
    /* Give up semaphore reserving print resource */
    xSemaphoreGive( printing_semphr );
    /* Yield so that task2 can be scheduled */
    vPortYield();
  } else {
    /* If the semaphore take timed out, something has gone wrong. */
    fpl("** Task 1 Error: could not take semaphore **");
    /* Hang thread rather than continue. */
    for(;;);
  }
}

#define BUFFER_SIZE 200
const char *HTTP_TRAILER = "\r\n";

void handleGetRequest() {
  int i;
  char buffer[BUFFER_SIZE + 2]; //100 for contents, 2 for \r\n
  int normalChunks = 0;
  int lastChunkSize = 0;
  
  normalChunks = strlen(page) / BUFFER_SIZE;
  lastChunkSize = strlen(page) - (normalChunks*BUFFER_SIZE);
  
  //Response Header
  wfp("HTTP/1.1 200 OK"); WIFLY.print(HTTP_TRAILER);
  wfp("Content-Length: ");
  WIFLY.print(strlen(page));
  WIFLY.print(HTTP_TRAILER);
  wfp("Content-Type text/html; charset=utf-8"); WIFLY.print(HTTP_TRAILER);
  //Necessary since we can't just send the whole thing in one response
  wfp("Transfer-Encoding: chunked"); WIFLY.print(HTTP_TRAILER);
  WIFLY.print(HTTP_TRAILER);
  
  /**
   *  Chunked reponses contain multiple writes with the size of the chunk
   *  (in hex, not including the HTTP_TRAILER) followed by the contents 
   *  and the HTTP_TRAILER.  The last chunk is simple a 0 with the trailer.
   *  For example:
   *  
   *  25
   *  This is the data in the first chunk
   *
   *  1C
   *  and this is the second one
   *
   *  3
   *  con
   *  8
   *  sequence
   *  0
   *
   */
  //send all the fixed-size chunks
  for (i = 0; i < normalChunks; i++) {
    strlcpy_P(buffer, (PGM_P)(pgm_read_word(&string_table[0])) + (i * BUFFER_SIZE), BUFFER_SIZE+1);
    buffer[BUFFER_SIZE] = '\r';
    buffer[BUFFER_SIZE+1] = '\n';
    WIFLY.print(BUFFER_SIZE, HEX);
    WIFLY.print(HTTP_TRAILER);
    WIFLY.write((const uint8_t *)buffer, BUFFER_SIZE);
    WIFLY.print(HTTP_TRAILER);
    //sleep 20 milliseconds to cause WiFly to send data
    //based on 'set comm time 10'
    delay(20);
  }
  
  //send the last chunk
  strlcpy_P(buffer, (PGM_P)(pgm_read_word(&string_table[0])) + (normalChunks * BUFFER_SIZE), lastChunkSize+1);
  buffer[lastChunkSize] = '\r';
  buffer[lastChunkSize+1] = '\n';
  WIFLY.print(lastChunkSize, HEX);
  WIFLY.print(HTTP_TRAILER);
  WIFLY.write((const uint8_t *)buffer, lastChunkSize);
  WIFLY.print(HTTP_TRAILER);
  delay(20);
  
  //send the "end chunks" message
  WIFLY.print(0);
  WIFLY.print(HTTP_TRAILER);
  delay(20);
  
  //close the connection
  wfp("$$$");
  delay(275);
  wfp("close\r");
  wfp("exit\r");
}

void task2_func(void *params)
{
  fpl("2: Entering Task");
  for(;;) {
    task2_loop();
  }
}

void task2_loop() {
  /* Take semaphore to use printing resource */
  if ( xSemaphoreTake( printing_semphr, PRINTING_TIMEOUT ) == pdTRUE ) {
    /* Trivial task: set pin high, print, set pin low*/
    digitalWrite(TASK2_PIN, HIGH);
    fpl("2: Task Loop");
    digitalWrite(TASK2_PIN, LOW);
    /* Give up semaphore reserving print resource */
    xSemaphoreGive( printing_semphr );
    /* Yield so that task1 can be scheduled */
    vPortYield();
  } else {
    /* If the semaphore take timed out, something has gone wrong. */
    Serial.println("** Task 2 Error: could not take semaphore **");
    /* Hang thread rather than continue. */
    for(;;);
  }
}