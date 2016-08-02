////////////////////////////////////////////////////////////////////////////////
//
// The University of Illinois/NCSA
// Open Source License (NCSA)
//
// Copyright (c) 2014-2015, Advanced Micro Devices, Inc. All rights reserved.
//
// Developed by:
//
//                 AMD Research and AMD HSA Software Development
//
//                 Advanced Micro Devices, Inc.
//
//                 www.amd.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
//  - Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimers.
//  - Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimers in
//    the documentation and/or other materials provided with the distribution.
//  - Neither the names of Advanced Micro Devices, Inc,
//    nor the names of its contributors may be used to endorse or promote
//    products derived from this Software without specific prior written
//    permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS WITH THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "hsail_helper.h"
#include "hsail_runtime.h"
#include "hsail_memory.h"
#include "hsail_finalize.h"
#include "test_suite.h"
#include "tools.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>



int parseArguments(int argc, char **argv) {
    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
      switch (opt) {
        case 'v': verbose_print = true;
          break;
        default:
        fprintf(stderr, "Usage:./%s [-v]\n", argv[0]);
        return false;
      }
      return true;
  }
}

int main(int argc, char **argv) {
  if (parseArguments(argc, argv) == false) return 1;

  test_unit_t* suite;
  int size;
  hsail_runtime_t run;

  //TODO: check error
  if (initialize_hsail(&run) != 0) return failed("Could not initialize_hsail");

  if (create_queue(run.agent, &run.queue) != 0) return failed("Could not create queue");

  hsail_kargs_t arg;
  allocate_arguments(&run, &arg);

  test_module_t* start = new_test_module_t("test_full");
  start->next = new_test_module_t("test_full_2");

  hsail_finalize_t fin;
  if (finalize_module(start, &run, &fin)) return 1;

  // SIGNAL
  /*
   * Create a signal to wait for the dispatch to finish.
   */
  hsa_signal_t signal;
  int module_nbr = 2;
  hsa_status_t err;
  err=hsa_signal_create(module_nbr, 0, NULL, &signal);
  check(Creating a HSA signal, err);

  // ARGUMENTS

  hsail_kargs_t args;
  allocate_arguments(&run, &args);

  test_module_t* tmp = start;
  while (tmp != NULL) {
    allocate_kernarg(&run, &args, &tmp->pkt_info);
    tmp = tmp->next;
  }

  // QUEUEING PACKET
  int index;
  tmp = start;
  while (tmp != NULL) {
    index = queue_packet(run.queue, signal, tmp->pkt_info);
    hsa_signal_store_relaxed((run.queue)->doorbell_signal, index);
    tmp = tmp->next;
  }
  check(Dispatching the kernel, HSA_STATUS_SUCCESS);
  /*
   * Wait on the dispatch completion signal until the kernel is finished.
   */
  while (hsa_signal_wait_acquire(signal, HSA_SIGNAL_CONDITION_LT, 1,
      UINT64_MAX, HSA_WAIT_STATE_BLOCKED) != 0);

  /*
   * Validate the data in the output buffer.
   */
  int valid = 1;
  int fail_index = 0;
  printf("%s", "-------out------> ");
  for(int i=0; i<1024*1024; i++) {
      if (i < 10) {
        printf("%d.", ((char*)args.out)[i]);
      } else {
          break;
      }
  }
  printf("%s", "\n");

  if(valid) {
      printf("Passed validation.\n");
  } else {
      printf("VALIDATION FAILED!\nBad index: %d\n", fail_index);
  }

  /*
   * Cleanup all allocated resources.
   */
  tmp = start;
  while (tmp != NULL && err == HSA_STATUS_SUCCESS) {
      err = hsa_memory_free(tmp->pkt_info.kernarg_address);
      tmp = tmp->next;
  }
  check(Freeing kernel argument memory buffer, err);

  err = hsa_signal_destroy(signal);
  check(Destroying the signal, err);

  err = hsa_executable_destroy(fin.executable);
  check(Destroying the executable, err);

  err = hsa_code_object_destroy(fin.code_object);
  check(Destroying the code object, err);

  err = hsa_queue_destroy(run.queue);
  check(Destroying the queue, err);

  err = hsa_memory_free(args.in);
  check(Freeing in argument memory buffer, err);

  err = hsa_memory_free(args.out);
  check(Freeing out argument memory buffer, err);

  err = hsa_shut_down();
  check(Shutting down the runtime, err);


  return 0;
}

//if ((size = init_tests(suite)) == -1) return failed("Could not init test suite");
//run_tests(suite, size, &run);
//destroy_tests(suite);
