#include "tests_p.h"

hsail_module_t* test_mp(hsail_kargs_t* args) {
  hsail_module_t* list = NULL;
  if (new_hsail_module(&list, "test_mp_3th") == 1) {
    return NULL;
  }
  reset_arguments(0, args);
  return list;
}

int test_mp_res(int nbr, hsail_kargs_t* args) {
  // Forbidden:
  if (R(1) == 1 && R(2) == 0) {
      printf("Output[%d]: %d, %d\n", nbr, R(1), R(2));
  } else {
    if (all_print == true)
      printf("Output[%d]: %d, %d\n", nbr, R(1), R(2));
    return 0;
  }
  return 1;
}
