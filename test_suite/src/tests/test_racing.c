#include "tests.h"
#include <stdio.h>

#define R(n) ((char*)args->out)[n-1]

hsail_module_t* test_racing_simple(hsail_kargs_t* args) {
  hsail_module_t* list = NULL;
  if (new_hsail_module(&list, "test_racing_simple") == 1) {
    return NULL;
  }
  reset_arguments(0, args);
  return list;
}

int test_racing_simple_res(int nbr, hsail_kargs_t* args) {
  // Forbidden:
  if (R(1) == 1 && R(2) == 0) {
      printf("[%d]Output: %d, %d\n", nbr, R(1), R(2));
  } else {
    if (all_print == true)
      printf("[%d]Output: %d, %d\n", nbr, R(1), R(2));
    return 0;
  }
  return 1;
}

hsail_module_t* test_racing_mult(hsail_kargs_t* args) {
  hsail_module_t* list = NULL;
  if (new_hsail_module(&list, "test_racing_mult") == 1
      || new_hsail_module(&list, "test_racing_mult_2") == 1) {
        return NULL;
    }
    //reset_arguments(1, args);
    return list;
}

int test_racing_mult_res(int nbr, hsail_kargs_t* args) {
  if (R(1) == 1 && R(2) == 0) {
      printf("[%d]Output: %d, %d, %d, %d\n", nbr, R(1), R(2), R(3), R(4));
  } else {
    if (all_print == true)
      printf("[%d]Output: %d, %d, %d, %d\n", nbr, R(1), R(2), R(3), R(4));
    return 0;
  }
  return 1;
}
