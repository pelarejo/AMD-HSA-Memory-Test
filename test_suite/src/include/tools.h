#ifndef TOOLS_H_
#define TOOLS_H_

#define true 1
#define false 0

extern int verbose_print;

void verb_printf(const char *format, ...);

#define check(msg, status) \
if (status != HSA_STATUS_SUCCESS) { \
    verb_printf("%s failed.\n", #msg); \
    exit(1); \
} else { \
   verb_printf("%s succeeded.\n", #msg); \
}

#endif
