#ifndef HSAIL_MEMORY_H_
#define HSAIL_MEMORY_H_

#include "hsail_runtime.h"

extern const int KERNARG_ALLOC_SIZE;

typedef struct __attribute__ ((aligned(16))) {
    void* in;
    void* out;
} hsail_kargs_t;

int allocate_arguments(hsail_runtime_t* run, hsail_kargs_t* args);
int reset_arguments(hsail_kargs_t* args);
int allocate_kernarg(hsail_runtime_t* run, hsail_kargs_t* args, hsail_kobj_t* pkt_info);

#endif
