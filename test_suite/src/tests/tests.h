#ifndef TESTS_H_
#define TESTS_H_

#include "hsail_module.h"
#include "hsail_memory.h"

typedef hsail_module_t* (*init_ptr_t)(hsail_kargs_t* args);
typedef int (*result_ptr_t)(hsail_kargs_t* args);

hsail_module_t* test_racing_simple(hsail_kargs_t* args);
int test_racing_simple_res(hsail_kargs_t* args);

#endif
