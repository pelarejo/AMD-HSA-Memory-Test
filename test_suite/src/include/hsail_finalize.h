#ifndef HSAIL_FINALIZE_H_
#define HSAIL_FINALIZE_H_

#include "test_suite.h"
#include "hsa/hsa.h"

typedef struct {
  hsa_signal_t sign;
  hsa_executable_t executable;
  hsa_code_object_t code_object;
} hsail_finalize_t;

int finalize_module(test_module_t* list, hsail_runtime_t* run, hsail_finalize_t* fin);

#endif
