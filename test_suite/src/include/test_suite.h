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
} test_unit_t;

int init_tests(test_unit_t* suite);
int run_tests(test_unit_t* suite, int size, hsail_runtime_t* runtime);
int destroy_tests(test_unit_t* suite);

#endif
