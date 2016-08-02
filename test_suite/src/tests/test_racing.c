#include "tests.h"
#include <stdio.h>

hsail_module_t* test_racing_simple(hsail_kargs_t* args) {
  hsail_module_t* list = NULL;
  if (new_hsail_module(&list, "test_full") == 1
      || new_hsail_module(&list, "test_full_2") == 1) {
        return NULL;
    }

    reset_arguments(args);
    return list;
}

int test_racing_simple_res(hsail_kargs_t* args) {
  printf("%s", "-------out------> ");
  for(int i = 0; i < 10; i++) {
    printf("%d.", ((char*)args->out)[i]);
  }
  printf("%s", "\n");
  return 0;
}
