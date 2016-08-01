#include "test_suite.h"
#include "test_racing.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Returns -1 if failed, the array size otherwise
int init_tests(test_unit* suite) {
  int size = 1;
  suite = malloc(sizeof(test_unit) * size);
  if (suite == NULL) {
    return -1;
  }

  strcpy(suite[0].name, "test_racing_simple");
  suite[0].ctr = 1;
  suite[0].run = &test_racing_simple;

  return size;
}

int run_test(test_unit* t) {

//  t->run(in, out);
}

int run_tests(test_unit* suite, int size, hsail_runtime_t* runtime) {
  int err;
  int i = 0;
  while (i++ < size) { // While test exist
    printf("Running test: %s\n", suite[i].name);
    while (suite[i].ctr-- > 0) { // While running it multipletimes
      err = 0;//suite[i].run(in, out, runtime);
      if (err != 0) return err;
    }
  }
  return 0;
}

int destroy_tests(test_unit* suite) {
  free(suite);
  return 0;
}

test_details* new_test_details(char *name) {
  test_details* dts = malloc(sizeof(test_details));
  if (dts == NULL) return NULL;
  dts->name = strdup(name);
  dts->next = NULL;
  return dts;
}

int destroy_test_details(test_details* dts) {
  while (dts != NULL) {
    test_details* next = dts->next;
    free(dts->name);
    free(dts);
    dts = next;
  }
  return 0;
}
