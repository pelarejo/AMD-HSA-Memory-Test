#include "tools.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int verbose_print = false;

// WARNING, FORMAT LIMITED TO 256
void verb_printf(const char *format, ...) {
  if (verbose_print == true) {
    va_list args;
    char fmt[256] = "   ";

    va_start(args, format);
    vprintf(strcat(fmt, format), args);
    va_end(args);
  }
}

int failed(const char* msg) {
  fprintf(stderr, "%s\n", msg);
  return 1;
}

int failed_i(const char* msg, int ret) {
  fprintf(stderr, "%s\n", msg);
  return ret;
}
