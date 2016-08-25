#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

#include "hsail_runtime.h"
#include "hsail_module.h"
#include "tests.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

typedef enum {
  FROM_FILE,
  FROM_SOURCE
} TYPE_E;

typedef struct {
  int ctr;
  char name[256];
  TYPE_E typ;
  int regs;
  init_ptr_t init;
  result_ptr_t res;
} test_unit_t;

void construct_t(test_unit_t* t, int ctr, char* name,
  init_ptr_t init, result_ptr_t res);
int init_tests(test_unit_t** suite);
int init_tests_from_file(char *name, int regs, test_unit_t** test);
int run_tests(test_unit_t* suite, int size, hsail_runtime_t* run);
int destroy_tests(test_unit_t* suite);

#endif
