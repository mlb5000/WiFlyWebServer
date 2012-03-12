#ifndef WEBSERVERDEMO_H
#define WEBSERVERDEMO_H

#include <avr/pgmspace.h>

static const char WIFLY_SET[] PROGMEM = "set";

//WiFly Commands
#define WIFLY_CMD_UART = 'u';
#define WIFLY_CMD_COMM = 'c';
#define WIFLY_CMD_WLAN = 'w';
#define WIFLY_CMD_IP = 'i';

//UART Options
#define WIFLY_UART_MODE = 'm';
#define WIFLY_UART_MODE_NOECHO = '0';

//COMM Options
#define WIFLY_COMM_SIZE = 's';
#define WIFLY_COMM_TIME = 't';
#define WIFLY_COMM_OPEN = 'o';
#define WIFLY_COMM_CLOSE = 'c';
#define WIFLY_COMM_REMOTE = 'r';

//WLAN Options
#define WIFLY_WLAN_PASSPHRASE = 'p';
#define WIFLY_WLAN_SSID = 's';
#define WIFLY_WLAN_JOIN = 'j';
#define WIFLY_WLAN_JOIN_AUTO = '1';

//IP Options
#define WIFLY_IP_PROTOCOL = 'p';
#define WIFLY_IP_LOCALPORT = 'l';
#define WIFLY_IP_DHCP = 'd';
#define WIFLY_IP_DHCP_ENABLE = '1';

static const char page[] PROGMEM = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
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

static PGM_P string_table[] PROGMEM = {page};

#endif