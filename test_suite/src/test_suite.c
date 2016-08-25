#include "test_suite.h"
#include "tools.h"
#include "hsail_finalize.h"
#include "hsail_helper.h"
#include "hsail_module.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SUCCESS 0
#define FAILURE 1
#define CRITICAL 2

#define PRINT_RTO 1000

void construct_t(test_unit_t* t, int ctr, char* name,
    init_ptr_t init, result_ptr_t res) {
  t->ctr = ctr;
  strcpy(t->name, name);
  t->init = init;
  t->res = res;
  t->typ = FROM_SOURCE;
}

// Returns -1 if failed, the array size otherwise
int init_tests(test_unit_t** suite) {
  const int size = 2;
  test_unit_t* s = malloc(sizeof(test_unit_t) * size);
  *suite = s;
  if (s == NULL) {
    return -1;
  }

  construct_t(&s[0], 1, "Test Racing Simple",
    &test_racing_simple, &test_racing_simple_res);

  construct_t(&s[1], 1, "Test Racing Multiple",
    &test_racing_mult, &test_racing_mult_res);

  /*construct_t(&s[1], 2000, "Test Message Passing",
    &test_mp, &test_mp_res);*/

  return size;
}

int init_tests_from_file(char *name, int regs, test_unit_t** test) {
  *test = malloc(sizeof(test_unit_t));
  construct_t(*test, 1, name,
    (init_ptr_t)&test_from_file, (result_ptr_t)&test_from_file_res);
  (*test)->typ = FROM_FILE;
  (*test)->regs = regs;
  return 1;
}

int run_test(int ctr, test_unit_t* t, hsail_runtime_t* run) {
  hsail_finalize_t fin;
  hsail_module_t* list;
  if (t->typ == FROM_FILE) {
    list = ((init_file_ptr_t)t->init)(&run->args, t->name);
  } else {
    list = t->init(&run->args);
  }
  if (list == NULL) return CRITICAL;
  if (finalize_modules(list, run, &fin) == 1) return CRITICAL;

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
  ccheck(Destroying the executable, err, CRITICAL);

  err = hsa_code_object_destroy(fin.code_object);
  ccheck(Destroying the code object, err, CRITICAL);

  if (t->typ == FROM_FILE) {
    return ((res_file_ptr_t)t->res)(ctr+1, &run->args, t->regs) == 0 ? SUCCESS : FAILURE;
  }
  return t->res(ctr+1, &run->args) == 0 ? SUCCESS : FAILURE;
}

static int print_thousands(int nbr) {
  const int th = nbr/PRINT_RTO;
  if (nbr == 1 || nbr == th*PRINT_RTO) {
    printf("%dk.", th);
    fflush(stdout);
  }
}

// TODO: Save outputs and print seperately
int run_tests(test_unit_t* suite, int size, hsail_runtime_t* run) {
  int i = 0;
  while (i < size) { // While test exist
    printf("\nRunning test: %s\n", suite[i].name);
    int j = 0;
    int err = 0;
    int failed = 0;
    // While running it multiple times
    while (err != CRITICAL && j < suite[i].ctr) {
      print_thousands(j+1);
      err = run_test(j, &suite[i], run);
      if (err == FAILURE) failed++;
      j++;
    }
    if (err == CRITICAL) printf("\n%s\n", "Critical error, test aborted");
    else if (failed > 0) {
      double ratio = (double) failed / suite[i].ctr;
      printf("\n%s: %.2f%% failures\n", "Validation failed", ratio * 100.0);
    } else printf("\n%s\n", "Passed validation");
    i++;
  }
  return 0;
}

int destroy_tests(test_unit_t* suite) {
  free(suite);
  return 0;
}
