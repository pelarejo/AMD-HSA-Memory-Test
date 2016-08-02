#include <stdio.h>
#include <stdarg.h>
#include "tools.h"

int verbose_print = false;

void verb_printf(const char *format, ...) {
  if (verbose_print == true) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
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
