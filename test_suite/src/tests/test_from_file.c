#include "tests_p.h"

hsail_module_t* test_from_file(hsail_kargs_t* args, char *name) {
  hsail_module_t* list = NULL;
  if (new_hsail_module(&list, name) == 1) {
    return NULL;
  }
  reset_arguments(1, args);
  return list;
}

int test_from_file_res(int nbr, hsail_kargs_t* args, int regs) {
  // Forbidden:
  if (regs < 1) return 1;
  printf("Output[%d]: ", nbr);
  for (int i = 1; i < regs; ++i) {
      printf("%d, ", R(i));
  }
  printf("%d\n", R(regs));
  return 0;
}
