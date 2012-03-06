/* */

#include <ArduinoFreeRTOS.h>
//#include <WiFlyWebServer.h>
#include <avr/pgmspace.h>

/* Default pins to 12 & 13 for digital readout of current task. */
#define TASK1_PIN (12)
#define TASK2_PIN (13)

#define PRINTING_TIMEOUT ( (portTickType) 25 )

/** @brief The maximum size of a Response from WiFly */
#define RESPONSE_SIZE 500
#define WIFLY Serial2

/** @brief the access point to connect to */
#define ACCESS_POINT "<router_SSID>"

/** @brief the passphrase to connect to the access point */
#define PASSPHRASE "<passphrase>"

void *task1_handle;
void *task2_handle;
xSemaphoreHandle printing_semphr;
prog_char page[] PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n" \
"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n" \
"	<head>\n" \
"		<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n" \
"		<title>Websocket client</title>\n" \
"		<style type=\"text/css\" media=\"screen\">\n" \
"			body {\n" \
"				margin-top:0;\n" \
"			}\n" \
"			.green {\n" \
"				color: #067E06;\n" \
"			}\n" \
"			.red {\n" \
"				color: #cd0e0e;\n" \
"			}\n" \
"			.data {\n" \
"				border: 1px solid #AAA;\n" \
"			}\n" \
"			form{\n" \
"				display: inline;\n" \
"			}\n" \
"			#toolbar{\n" \
"				border-bottom: 2px solid #CCCCFF;\n" \
"				line-height: 2em;\n" \
"				vertical-align: middle;\n" \
"			}\n" \
"			#log {\n" \
"				line-height: 1.3;\n" \
"			}\n" \
"		</style>\n" \
"	</head>\n" \
"	<body>\n" \
"		<div id=\"toolbar\">\n" \
"			<div id=\"connect\">\n" \
"				<input type=\"text\" id=\"wsserver\" value=\"ws://localhost:8080/echo\" /> <input type=\"button\" value=\"Connect\" onclick=\"return ws_connect();\"/>\n" \
"			</div>\n" \
"			<div id=\"tools\" style=\"display:none;\">\n" \
"				<input type=\"text\" id=\"val\" value=\"\" />\n" \
"				<input type=\"button\" onclick=\"return ws_send_data();\" value=\"Send\" />\n" \
"				<input type=\"button\" value=\"Disconnect\" onclick=\"return ws_disconnect();\"/>\n" \
"			</div>\n" \
"		</div>\n" \
"		<div id=\"log\"></div>\n" \
"		<script type=\"text/javascript\">\n" \
"		//<![CDATA[\n" \
"		var log = document.getElementById(\"log\");\n" \
"		var connect = document.getElementById(\"connect\");\n" \
"		var tools = document.getElementById(\"tools\");\n" \
"		var val = document.getElementById(\"val\");\n" \
"		var ws;\n" \
"		\n" \
"		function ws_connect() {\n" \
"			if (\"WebSocket\" in window) {\n" \
"				ws = new WebSocket(document.getElementById(\"wsserver\").value);\n" \
"				ws.onopen = function(event) {\n" \
"					log.innerHTML+=\n" \
"						\"<div class='green'>websocket connected</div>\";\n" \
"					connect.style.display = 'none';\n" \
"					tools.style.display = 'block';\n" \
"				};\n" \
"\n" \
"				ws.onerror = function(event) {\n" \
"					log.innerHTML+=\n" \
"						\"<div class='red'>error on websocket!</div>\";\n" \
"				};\n" \
"\n" \
"				ws.onmessage = function(event) {\n" \
"					log.innerHTML+=\n" \
"						\"<div>get message:<span class='data'>\"+event.data+\"</span></div>\";\n" \
"				};\n" \
"\n" \
"				ws.onclose = function(event) {\n" \
"					log.innerHTML+=\n" \
"						\"<div class='red'>websocket closed</div>\";\n" \
"					connect.style.display = 'block';\n" \
"					tools.style.display = 'none';\n" \
"				};\n" \
"			} else {\n" \
"				// the browser doesn't support WebSocket\n" \
"				alert(\"WebSocket NOT supported here!\\r\\n\\r\\nBrowser: \" +\n" \
"					navigator.appName + \" \" + navigator.appVersion);\n" \
"			}\n" \
"\n" \
"			return false;\n" \
"		}\n" \
"\n" \
"		function ws_disconnect() {\n" \
"			ws.close();\n" \
"\n" \
"			return false;\n" \
"		}\n" \
"\n" \
"		function ws_send_data() {\n" \
"			log.innerHTML+=\n" \
"				\"<div>send message:<span class='data'>\"+val.value+\"</span></div>\";\n" \
"			ws.send(val.value);\n" \
"\n" \
"			return false;\n" \
"		}\n" \
"		//]]>\n" \
"		</script>\n" \
"	</body>\n" \
"</html>\n";

PGM_P string_table[] PROGMEM = {page};

int sendCommand(const char *command, const char *expectedResponse, unsigned delay_ms=10);
void flushSerial(bool display=false, unsigned to_sec=3, unsigned delay_ms=10);

// Pins are 17 for INCOMING TO Arduino, 16 for OUTGOING TO Wifly
// Arduino       WiFly
//  17 - receive  TX   (Send from Wifly, Receive to Arduino)
//  16 - send     RX   (Send from Arduino, Receive to WiFly) 
//WiFlySerial WiFly(17,16); 

void setup () {
  /* Use the standard Arduino HardwareSerial library for serial. */
  Serial.begin(115200);
  WIFLY.begin(9600);
 
  Serial.println("Starting");
  pinMode(TASK1_PIN, OUTPUT);
  pinMode(TASK2_PIN, OUTPUT);
  digitalWrite(TASK1_PIN, LOW);
  digitalWrite(TASK2_PIN, LOW);
  
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
  vTaskStartScheduler();
  /* code after vTaskStartScheduler, and code in loop(), is never reached. */
}

void loop () {
  Serial.println("Never reached");
}

/** @brief A WiFly configuration option */
typedef struct wiflyConfig_t {
  const char *message;
  const char *command;
  const char *expectedResponse;
  const char *errorMessage;  
} wiflyConfig_t;

/** @brief The configuration options to issue to the WiFly module (in order) */
wiflyConfig_t options[] = {
  {"Disabling echoback", "set uart mode 0", "AOK", "Failed to disable echoback"},
  {"Setting UART buffer size", "set comm size 1420", "AOK", "Failed to set buffer size"},
  {"Setting flush time", "set comm time 10", "AOK", "Failed to set flush time"},
  {"Don't send bytes when remote connects", "set comm remote 0", "AOK", "Failed to disable remote talkback"},
  {"Disable open string", "set comm open 0", "AOK", "Failed to disable open string"},
  {"Disable close string", "set comm close 0", "AOK", "Failed to disable close string"},
  {"Enable DHCP", "set ip dhcp 1", "AOK", "Failed to enable DHCP"},
  {"Enabling TCP/IP mode", "set ip protocol 2", "AOK", "Failed to set TCP/IP mode"},
  {"Enabling autojoin", "set wlan join 1", "AOK", "Failed to enable autojoin"},
  {"Listen on port 80", "set ip localport 80", "AOK", "Failed to configure listening port"},
  {"Setting passphrase", 
"set w p " \
PASSPHRASE, "AOK", "Failed to set passphrase"},
  {"Setting access point",
"set w s " \
ACCESS_POINT, "AOK", "Failed to set access point"}
};

/** @brief Configure the WiFly module

    Configures the WiFly module for AdHoc networking
*/
void configureWify() {
  unsigned i;
  
  /*
  //this is what it would be if we supported Arduino 1.0
  WiFly.begin();
  
  WiFly.setAuthMode(WIFLY_AUTH_OPEN);
  WiFly.setJoinMode(WIFLY_JOIN_MAKE_ADHOC);
  WiFly.setDHCPMode(WIFLY_DHCP_OFF);*/
  
  Serial.println("Configuring the RN-171");
  
  //This initial flush gets us the current state of the WiFly module
  flushSerial(true);
  
  //turn on command mode (only possible on clean startup, reset won't work)
  if (0 != enableCommandMode()) {
    Serial.println("Failed to enable command mode");
    goto CLEANUP;
  }
  
  //speed up the UART from the default 9800
  WIFLY.print("set u i 230400\r");
  WIFLY.flush();
  delay(1000);
  WIFLY.begin(230400);
  
  //WIFLY.print("set comm size 1420\rset comm time 10\rset comm remote 0\rset comm open 0\rset comm close 0\rset ip dhcp 1\rset ip protocol 2\rset wlan join 1\rset ip localport 80\rset w p skiliberty\rset w s BAKERS\r");
  //flushSerial(true, 100);
  
  for (i = 0; i < sizeof(options) / sizeof(wiflyConfig_t); i++) {
    Serial.print("  ");
    Serial.print(options[i].message);
    Serial.print(": ");
    if (0 != sendCommand(options[i].command, options[i].expectedResponse)) {
      Serial.print("    ");
      Serial.println(options[i].errorMessage);
      Serial.println("    Retrying...");
      flushSerial();
      delay(1000);
      if (0 != sendCommand(options[i].command, options[i].expectedResponse, 50)) {
        Serial.println("    Still failed...");
        goto CLEANUP; 
      }
      else {
        Serial.println("    Success!");
      }
    }
    else {
      Serial.println("success");
    }
  }

CLEANUP:
  if (0 != disableCommandMode()) {
    Serial.println("Failed to disable command mode, bad things may happen");
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
int sendCommand(const char *command, const char *expectedResponse, unsigned delay_ms) {
  unsigned size = RESPONSE_SIZE;
  char response[RESPONSE_SIZE];
  int retval = 0;
  
  if (NULL == command || NULL == expectedResponse) {
    retval = -2;
  }
  
  if (0 == retval) {
    WIFLY.print(command);
    WIFLY.print("\r"); //commit command
  }
  
  if (0 == retval && -1 == readFromWiFly(response, &size, 10, delay_ms)) {
    Serial.println("Timed out while reading from WiFly");
    retval = -1;
  }
  
  if (0 == retval) {  
    if (0 == strstr(response, expectedResponse)) {
      Serial.print("Did not receive expected response: ");
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
    WIFLY.print("$$$");
    WIFLY.flush();
    delay(275); //WiFly requires a 250ms delay following this sequence
    if (0 == retval && -1 == readFromWiFly(response, &size, 10, 100)) {
      Serial.println("Timed out while reading from WiFly");
      retval = -1;
    }

    if (response[0] != 'C' || response[1] != 'M' || response[2] != 'D') {
      Serial.println("Did not receive expected CMD response");
      retval = -1;
    } 
  } while(retval != 0 && tries < maxTries);
  
  return retval;
}

/** @brief Disable WiFly command mode

    @retval 0 Success
    @retval -1 Failure
*/
int disableCommandMode() {
  return sendCommand("exit\r", "EXIT");
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
        Serial.println("Timed out");
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
  Serial.println("1: Entering Task");
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
    Serial.println("     GET Request");
    handleGetRequest();
  }
  
  /* Take semaphore to use printing resource */
  if (xSemaphoreTake( printing_semphr, PRINTING_TIMEOUT ) == pdTRUE ) {
    /* Trivial task: set pin high, print, set pin low*/
    digitalWrite(TASK1_PIN, HIGH);
    if (result == 0) {
      Serial.print(response); 
    }
    Serial.println("1: Task Loop");
    digitalWrite(TASK2_PIN, LOW);
    /* Give up semaphore reserving print resource */
    xSemaphoreGive( printing_semphr );
    /* Yield so that task2 can be scheduled */
    vPortYield();
  } else {
    /* If the semaphore take timed out, something has gone wrong. */
    Serial.println("** Task 1 Error: could not take semaphore **");
    /* Hang thread rather than continue. */
    for(;;);
  }
}

#define BUFFER_SIZE 200
#define HTTP_TRAILER "\r\n"

void handleGetRequest() {
  int i;
  char buffer[BUFFER_SIZE + 2]; //100 for contents, 2 for \r\n
  int normalChunks = 0;
  int lastChunkSize = 0;
  
  normalChunks = strlen(page) / BUFFER_SIZE;
  lastChunkSize = strlen(page) - (normalChunks*BUFFER_SIZE);
  
  //Response Header
  WIFLY.print("HTTP/1.1 200 OK\r\n");
  WIFLY.print("Content-Length: ");
  WIFLY.print(strlen(page));
  WIFLY.print(HTTP_TRAILER);
  WIFLY.print("Content-Type text/html; charset=utf-8\r\n");
  //Necessary since we can't just send the whole thing in one response
  WIFLY.print("Transfer-Encoding: chunked\r\n");
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
  WIFLY.print("$$$");
  delay(275);
  WIFLY.print("close\r");
  WIFLY.print("exit\r");
}

void task2_func(void *params)
{
  Serial.println("2: Entering Task");
  for(;;) {
    task2_loop();
  }
}

void task2_loop() {
  /* Take semaphore to use printing resource */
  if ( xSemaphoreTake( printing_semphr, PRINTING_TIMEOUT ) == pdTRUE ) {
    /* Trivial task: set pin high, print, set pin low*/
    digitalWrite(TASK2_PIN, HIGH);
    Serial.println("2: Task Loop");
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