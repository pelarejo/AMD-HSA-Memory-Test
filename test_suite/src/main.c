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

  test_unit* suite;
  int size;
  hsail_runtime_t run;

  //TODO: check error
  if (initialize_hsail(&run) != 0) return failed("Could not initialize_hsail");

  if (create_queue(run.agent, &run.queue) != 0) return failed("Could not create queue");

  hsail_kargs_t arg;
  allocate_arguments(&run, &arg);

  test_details* start = new_test_details("test_full");
  start->next = new_test_details("test_full_2");

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

  test_details* tmp = start;
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
  while (tmp != NULL) {
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

int test(int argc, char **argv) {
    parseArguments(argc, argv);

    hsail_runtime_t run;
    if (initialize_hsail(&run) != 0) return failed("Could not initialize_hsail");

    if (create_queue(run.agent, &run.queue) != 0) return failed("Could not create queue");

    hsa_status_t err;
    err = hsa_agent_get_info(run.agent, HSA_AGENT_INFO_PROFILE, &run.profile);
    check(Getting agent profile, err);
    if (run.profile != HSA_PROFILE_FULL) return failed("Profile not supported");

    /*
     * Load the BRIG binary.
     */
    hsa_ext_module_t module;
    hsa_ext_module_t module2;
    printf("%s\n", "----------> Running Full");
    if (load_module_from_file("./hsail/test_full.brig", &module) != 0) {
      return 1;
    }
    if (load_module_from_file("./hsail/test_full_2.brig", &module2) != 0) {
      return 1;
    }

    // program parts

    hsa_ext_program_t program;
    if (create_program(&program, &run) != 0) return failed("Could not create program");

    /*
     * Add the BRIG module to hsa program.
     */
    err = run.table_1_00.hsa_ext_program_add_module(program, module);
    err = run.table_1_00.hsa_ext_program_add_module(program, module2);
    check(Adding the brig module to the program, err);

    /*
     * Determine the agents ISA.
     */
    hsa_isa_t isa;
    err = hsa_agent_get_info(run.agent, HSA_AGENT_INFO_ISA, &isa);
    check(Query the agents isa, err);

    /*
     * Finalize the program and extract the code object.
     */
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    hsa_code_object_t code_object;
    err = run.table_1_00.hsa_ext_program_finalize(program, isa, 0, control_directives, "", HSA_CODE_OBJECT_TYPE_PROGRAM, &code_object);
    check(Finalizing the program, err);

    /*
     * Destroy the program, it is no longer needed.
     */
    err = run.table_1_00.hsa_ext_program_destroy(program);
    check(Destroying the program, err);

    /*
     * Create the empty executable.
     */
    hsa_executable_t executable;
    err = hsa_executable_create(run.profile, HSA_EXECUTABLE_STATE_UNFROZEN, "", &executable);
    check(Create the executable, err);

    /*
     * Load the code object.
     */
    err = hsa_executable_load_code_object(executable, run.agent, code_object, "");
    check(Loading the code object, err);

    /*
     * Freeze the executable; it can now be queried for symbols.
     */
    err = hsa_executable_freeze(executable, "");
    check(Freeze the executable, err);

   /*
    * Extract the symbol from the executable.
    */
    hsa_executable_symbol_t symbol;
    // deprecated but get_symbol_by_name doesn't exists
    err = hsa_executable_get_symbol(executable, NULL, "&__test_full_kernel", run.agent, 0, &symbol);
    check(Extract the symbol from the executable, err);

    hsa_executable_symbol_t symbol2;
    err = hsa_executable_get_symbol(executable, NULL, "&__test_full_2_kernel", run.agent, 0, &symbol2);
    check(Extract the symbol from the executable, err);


    /*
     * Extract dispatch information from the symbol
     */
    hsail_kobj_t pkt_info;
    hsail_kobj_t pkt_info2;
    extract_symbol(&pkt_info, symbol);
    extract_symbol(&pkt_info2, symbol2);


    // SIGNAL
    /*
     * Create a signal to wait for the dispatch to finish.
     */
    hsa_signal_t signal;
    err=hsa_signal_create(2, 0, NULL, &signal);
    check(Creating a HSA signal, err);


    // ARGUMENTS

    hsail_kargs_t args;

    allocate_arguments(&run, &args);

    allocate_kernarg(&run, &args, &pkt_info);
    pkt_info2.kernarg_address = pkt_info.kernarg_address;

    // QUEUEING PACKET

    int index = queue_packet(run.queue, signal, pkt_info);
    hsa_signal_store_relaxed((run.queue)->doorbell_signal, index);

    index = queue_packet(run.queue, signal, pkt_info2);
    hsa_signal_store_relaxed((run.queue)->doorbell_signal, index);

    check(Dispatching the kernel, err);
    /*
     * Wait on the dispatch completion signal until the kernel is finished.
     */
    while (hsa_signal_wait_acquire(signal, HSA_SIGNAL_CONDITION_LT, 1,
        UINT64_MAX, HSA_WAIT_STATE_BLOCKED) != 0);

    /*
     * Validate the data in the output buffer.
     */
    int valid=1;
    int fail_index=0;
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
    err = hsa_memory_free(pkt_info.kernarg_address);
    check(Freeing kernel argument memory buffer, err);

    err=hsa_signal_destroy(signal);
    check(Destroying the signal, err);

    err=hsa_executable_destroy(executable);
    check(Destroying the executable, err);

    err=hsa_code_object_destroy(code_object);
    check(Destroying the code object, err);

    err=hsa_queue_destroy(run.queue);
    check(Destroying the queue, err);

    err = hsa_memory_free(args.in);
    check(Freeing in argument memory buffer, err);

    err = hsa_memory_free(args.out);
    check(Freeing out argument memory buffer, err);

    err=hsa_shut_down();
    check(Shutting down the runtime, err);

    return 0;
}
