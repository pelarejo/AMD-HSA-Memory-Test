#include "test_suite.h"
#include "tools.h"
#include "hsail_finalize.h"
#include "hsail_helper.h"
#include "hsail_module.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Returns -1 if failed, the array size otherwise
int init_tests(test_unit_t** suite) {
  const int size = 2;
  test_unit_t* s = malloc(sizeof(test_unit_t) * size);
  if (s == NULL) {
    return -1;
  }

  s[0].ctr = 1;
  strcpy(s[0].name, "test_racing_simple");
  s[0].init = &test_racing_simple;
  s[0].res = &test_racing_simple_res;

  s[1].ctr = 1;
  strcpy(s[1].name, "test_racing_simple_2");
  s[1].init = &test_racing_simple;
  s[1].res = &test_racing_simple_res;

  *suite = s;
  return size;
}

int run_test(test_unit_t* t, hsail_runtime_t* run) {
  hsail_finalize_t fin;
  hsail_module_t* list = t->init(&run->args);
  if (list == NULL) return 1;
  if (finalize_modules(list, run, &fin)) return 1;

  int size = 0;
  hsail_module_t* tmp = list;
  while (tmp != NULL) {
    allocate_kernarg(run->agent, &run->args, &tmp->pkt_info);
    size++;
    tmp = tmp->next;
  }

  // PREPARING SIGNAL
  //hsa_signal_silent_store_relaxed(run->signum, size);
  hsa_signal_store_relaxed(run->signum, size);

  // QUEUEING PACKET & EXECUTE
  int index;
  tmp = list;
  while (tmp != NULL) {
    index = enqueue_packet(run->queue, run->signum, &(tmp->pkt_info));
    hsa_signal_store_relaxed((run->queue)->doorbell_signal, index);
    tmp = tmp->next;
  }
  check(Dispatching the kernel, HSA_STATUS_SUCCESS);


  // Wait on the dispatch completion signal until the kernel is finished.
  while (hsa_signal_wait_acquire(run->signum, HSA_SIGNAL_CONDITION_EQ, 0,
    UINT64_MAX, HSA_WAIT_STATE_BLOCKED) != 0) {
  }

  hsa_status_t err;
  destroy_hsail_modules(list);
  err = hsa_executable_destroy(fin.executable);
  check(Destroying the executable, err);

  err = hsa_code_object_destroy(fin.code_object);
  check(Destroying the code object, err);

  return t->res(&run->args);
}

int run_tests(test_unit_t* suite, int size, hsail_runtime_t* run) {
  int err;
  int i = 0;
  while (i < size) { // While test exist
    printf("\nRunning test: %s\n", suite[i].name);
    while (suite[i].ctr-- > 0) { // While running it multipletimes
      err = run_test(&suite[i], run);
    }
    if (err == 0) printf("%s\n", "Passed validation");
    else printf("%s\n", "Validation failed");
    i++;
  }
  return 0;
}

int destroy_tests(test_unit_t* suite) {
  free(suite);
  return 0;
}
