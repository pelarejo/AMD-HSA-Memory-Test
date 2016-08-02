#ifndef HSAIL_MODULES_H_
#define HSAIL_MODULES_H_

#include "hsail_helper.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

typedef struct module_s {
  char* name;
  hsail_kobj_t pkt_info;
  hsa_ext_module_t module;
  struct module_s *next;
} hsail_module_t;

int new_test_module(hsail_module_t** list, char *name);
int destroy_test_modules(hsail_module_t* list);

#endif
