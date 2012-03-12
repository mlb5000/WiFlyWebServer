#ifndef BAKERMATT_LOGGER_H
#define BAKERMATT_LOGGER_H

#include <avr/pgmspace.h>

class Logger
{
public:
  Logger() {}
  ~Logger() {}
  
  void log(const char *str);
  void log_P(const prog_char *str);
};

#endif