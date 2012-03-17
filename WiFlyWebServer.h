#ifndef __WIFLY_WEBSERVER_H__
#define __WIFLY_WEBSERVER_H__

#include <stdlib.h>
#include <string.h>

#define min(a,b) a >= b ? b : a

int serial2Write(char c);

#include <WebServer.h>
#include <FileSystem.h>
#if __AVR__
#include <MemoryFree.h>
#endif

#endif // __WIFLY_WEBSERVER_H__
