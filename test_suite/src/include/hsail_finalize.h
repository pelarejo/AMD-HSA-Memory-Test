#ifndef HSAIL_FINALIZE_H_
#define HSAIL_FINALIZE_H_

#include "hsail_module.h"
#include "hsail_runtime.h"
#include "hsa/hsa.h"

typedef struct {
  hsa_signal_t sign;
  hsa_executable_t executable;
  hsa_code_object_t code_object;
} hsail_finalize_t;

int compile_code_object(hsail_module_t* list,
  hsail_runtime_t* run, hsail_finalize_t* fin);
int generate_executable(hsail_runtime_t* run, hsail_finalize_t* fin);
int extract_dispatch_info(hsail_module_t* list,
  hsail_runtime_t* run, hsail_finalize_t* fin);
int finalize_modules(hsail_module_t* list,
  hsail_runtime_t* run, hsail_finalize_t* fin);

#endif
