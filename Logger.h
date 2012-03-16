#ifndef BAKERMATT_LOGGER_H
#define BAKERMATT_LOGGER_H

#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

typedef PROGMEM char prog_char;

class Logger
{
public:
  Logger() {}
  ~Logger() {}
  
  void log(const char *str);
  void log_P(const prog_char *str);
};

#endif