#include "test_suite.h"
#include "tools.h"
#include "hsail_finalize.h"
#include "hsail_helper.h"
#include "hsail_module.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void construct_t(test_unit_t* t, int ctr, char* name,
    init_ptr_t init, result_ptr_t res) {
  t->ctr = ctr;
  strcpy(t->name, name);
  t->init = init;
  t->res = res;
}

// Returns -1 if failed, the array size otherwise
int init_tests(test_unit_t** suite) {
  const int size = 2;
  test_unit_t* s = malloc(sizeof(test_unit_t) * size);
  *suite = s;
  if (s == NULL) {
    return -1;
  }

  construct_t(&s[0], 1, "Message Passing",
    &test_racing_simple, &test_racing_simple_res);

  construct_t(&s[1], 1, "test racing multiple",
    &test_racing_mult, &test_racing_mult_res);

  return size;
}


int run_test(int ctr, test_unit_t* t, hsail_runtime_t* run) {
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

  return t->res(ctr+1, &run->args);
}

int run_tests(test_unit_t* suite, int size, hsail_runtime_t* run) {
  int err;
  int i = 0;
  while (i < size) { // While test exist
    printf("\nRunning test: %s\n", suite[i].name);
    int j = 0;
    while (j < suite[i].ctr) { // While running it multipletimes
      err = run_test(j, &suite[i], run);
      j++;
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
