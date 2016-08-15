#ifndef TESTS_H_
#define TESTS_H_

#include "hsail_module.h"
#include "hsail_memory.h"
#include "tools.h"

typedef hsail_module_t* (*init_ptr_t)(hsail_kargs_t* args);
typedef int (*result_ptr_t)(int nbr, hsail_kargs_t* args);

hsail_module_t* test_racing_simple(hsail_kargs_t* args);
int test_racing_simple_res(int nbr, hsail_kargs_t* args);

hsail_module_t* test_racing_mult(hsail_kargs_t* args);
int test_racing_mult_res(int nbr, hsail_kargs_t* args);

// MESSAGE PASSING
hsail_module_t* test_mp(hsail_kargs_t* args);
int test_mp_res(int nbr, hsail_kargs_t* args);

#endif
