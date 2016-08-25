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

static int regs = 0;
static int nbr_runs = 1;
static char* name = NULL;

int main(int argc, char **argv) {
  if (parseArguments(argc, argv) == false) return 1;

  // TODO: check error
  // TODO: provide masks to see which variable hsail var are set
  // INIT
  hsail_runtime_t run;
  if (initialize_hsail(&run) != 0) return failed("Could not initialize_hsail");

  if (initialize_queue(run.agent, &run.queue) != 0) return failed("Could not create queue");

  // SIGNAL
  hsa_status_t err;
  err = hsa_signal_create(0, 0, NULL, &run.signum);
  check(Creating a HSA signal, err);

  // ARGUMENTS
  if (allocate_arguments(run.agent, &run.args) == 1) return failed("Could not allocate arguments");

  printf("%s\n", "=== Testing Started ===");
  // RUN TESTS
  int size;
  test_unit_t* suite;
  if (name == NULL && (size = init_tests(&suite)) == -1) return failed("Could not allocate test suite");
  else if (name != NULL && (size = init_tests_from_file(name, regs, nbr_runs, &suite)) == -1) return failed("Could not allocate test suite");

  run_tests(suite, size, &run);

  printf("%s\n","\n=== Testing Ended ===");

  // CLEANUP.
  destroy_tests(suite);
  destroy_hsail(&run);

  return 0;
}

int parseArguments(int argc, char **argv) {
    int opt;

    while ((opt = getopt(argc, argv, "vanrih")) != -1) {
      switch (opt) {
        case 'a': all_print = true;
          break;
        case 'v': verbose_print = true;
          break;
        case 'n':
          if (optind < argc && argv[optind][0] != '-') name = argv[optind];
          else {
            fprintf(stderr, "%s\n", "Usage: -n file_name -r regs");
            return false;
          }
          break;
        case 'r':
          if (optind < argc && argv[optind][0] != '-') regs = atoi(argv[optind]);
          else {
            fprintf(stderr, "%s\n", "Usage: -n file_name -r regs");
            return false;
          }
          break;
        case 'i':
          if (optind < argc && argv[optind][0] != '-') {
            nbr_runs = atoi(argv[optind]);
          } else {
            fprintf(stderr, "%s\n", "Usage: -n file_name -r regs -i nbr_runs");
            return false;
          }
          break;
        case 'h':
            printf("Usage:%s [-va] [-n file_name -r regs]\n", argv[0]);
            printf("%s\t%s\n", "-h", "print this help message");
            printf("%s\t%s\n", "-v", "verbose mode");
            printf("%s\t%s\n", "-a", "all outputs mode");
            printf("%s\t%s\n\t%s\n", "-n", "brig file to read, requires -r option",
              "search in the ./hsail folder and file_name should exclude extension");
            printf("%s\t%s\n\t%s\n", "-r", "number of registers to output, requires -n option",
              "regs should be above 0 and within integer margin");
            printf("%s\t%s\n\t%s\n", "-i", "number of runs for the test, requires -nr options",
              "nbr_runs defaults to 1 and should be within integer margin over 0");
            return false; // Should be another 'true' value
        default:
        printf("Usage:%s [-va] [-n file_name -r regs]\n", argv[0]);
        return false;
      }
  }
  if ((regs == 0 && name != NULL) || (regs != 0 && name == NULL) || nbr_runs < 1) {
    fprintf(stderr, "%s\n", "Usage: -n file_name (!= NULL) -r regs (> 0) [-i nbr (> 0)]");
    return false;
  }
  return true;
}
