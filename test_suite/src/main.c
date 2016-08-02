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

int main(int argc, char **argv) {
  if (parseArguments(argc, argv) == false) return 1;

  // TODO: check error
  // TODO: provide masks to see which variable is set
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

  printf("%s\n", "=== Starting Testing ===");
  // RUN TESTS
  int size;
  test_unit_t* suite;
  if ((size = init_tests(&suite)) == -1) return failed("Could not allocate test suite");

  run_tests(suite, size, &run);

  printf("%s\n","\n=== Testing Ended ===");

  // CLEANUP.
  destroy_tests(suite);
  destroy_hsail(&run);

  return 0;
}

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
