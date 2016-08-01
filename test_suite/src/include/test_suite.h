#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

#include "hsail_runtime.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

typedef int (*test_ptr)(char*, char*, hsail_runtime_t*);

typedef struct {
  char name[256];
  int ctr;
  test_ptr run;
} test_unit;

typedef struct test_details_s {
  char* name;
  hsa_ext_module_t module;
  hsa_executable_symbol_t symbol;
  hsail_kobj_t pkt_info;
  struct test_details_s *next;
} test_details;

int init_tests(test_unit* suite);
int run_tests(test_unit* suite, int size, hsail_runtime_t* runtime);
int destroy_tests(test_unit* suite);
test_details* new_test_details(char *name);

#endif
