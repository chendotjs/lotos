#include "misc.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void lotos_log(int priority, const char *format, ...) {
  FILE *fp;
  const char *color_prefix = NULL;

  switch (priority) {
  case LOG_ERR:
    fp = stderr;
    color_prefix = "\033[1;31m";
    break;
  case LOG_INFO:
    fp = stdout;
    color_prefix = "\033[1;32m";
    break;
  default:
    fp = stdout;
  }
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  fprintf(fp, "[%4d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900,
          tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  color_prefix == NULL ? 0 : fprintf(fp, "%s", color_prefix);
  va_list args;
  va_start(args, format);
  vfprintf(fp, format, args);
  fprintf(fp, "\033[0m\n");
  fflush(fp);
  va_end(args);
}
