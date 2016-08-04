#ifndef HSAIL_MEMORY_H_
#define HSAIL_MEMORY_H_

#include "hsail_helper.h"
#include "hsa/hsa.h"

extern const int KERNARG_ALLOC_SIZE;

typedef struct __attribute__ ((aligned(16))) {
    void* in;
    void* out;
} hsail_kargs_t;

int reset_arguments(int in, hsail_kargs_t* args);
int allocate_arguments(hsa_agent_t agent, hsail_kargs_t* args);
int allocate_kernarg(hsa_agent_t agent, hsail_kargs_t* args, hsail_kobj_t* pkt_info);
int free_arguments(hsail_kargs_t* args);
int free_kernarg(void* kernarg_address);

#endif
