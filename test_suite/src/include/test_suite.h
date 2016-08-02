#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

#include "hsail_runtime.h"
#include "hsail_module.h"
#include "tests.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

typedef struct {
  int ctr;
  char name[256];
  hsail_module_t* list;
  init_ptr_t init;
  result_ptr_t res;
} test_unit_t;

int init_tests(test_unit_t** suite);
int run_tests(test_unit_t* suite, int size, hsail_runtime_t* run);
int destroy_tests(test_unit_t* suite);

#endif
