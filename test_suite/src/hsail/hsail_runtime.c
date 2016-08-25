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

#include "hsail_runtime.h"
#include "hsail_helper.h"
#include "tools.h"

#include <string.h>
#include <stdio.h>

int initialize_hsail(hsail_runtime_t* run) {
  hsa_status_t err;

  err = hsa_init();
  check(Initializing the hsa runtime, err);
  /*
   * Determine if the finalizer 1.0 extension is supported.
   */
  bool support;
  err = hsa_system_extension_supported(HSA_EXTENSION_FINALIZER, 1, 0, &support);
  check(Checking finalizer 1.0 extension support, err);
  /*
   * Generate the finalizer function table.
   */
  err = hsa_system_get_extension_table(HSA_EXTENSION_FINALIZER, 1, 0, &run->table_1_00);
  check(Generating function table for finalizer, err);
  /*
   * Iterate over the agents and pick the gpu agent using
   * the get_gpu_agent callback.
   */
  err = hsa_iterate_agents(get_gpu_agent, &run->agent);
  if(err == HSA_STATUS_INFO_BREAK) {
      err = HSA_STATUS_SUCCESS;
  } else { // No GPU agent was found.
      err = HSA_STATUS_ERROR;
  }
  check(Getting a gpu agent, err);
  // Query the name of the agent
  char name[64] = { 0 };
  err = hsa_agent_get_info(run->agent, HSA_AGENT_INFO_NAME, name);
  check(Querying the agent name, err);
  printf("The agent name is %s.\n", name);
  // Obtain agent profile
  err = hsa_agent_get_info(run->agent, HSA_AGENT_INFO_PROFILE, &run->profile);
  check(Getting agent profile, err);
  if (run->profile != HSA_PROFILE_FULL) return failed("Profile not supported");
  // Obtain Machine model
  err = hsa_agent_get_info(run->agent, HSA_AGENT_INFO_MACHINE_MODEL, &run->machine_model);
  check(Obtaining machine model, err);
  return 0;
}

int initialize_queue(hsa_agent_t agent, hsa_queue_t** queue) {
  /*
   * Query the maximum size of the queue.
   */
  hsa_status_t err;
  uint32_t queue_size = 0;
  err = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_size);
  check(Querying the agent maximum queue size, err);
  printf("The maximum queue size is %u.\n", (unsigned int) queue_size);

  /*
   * Create a queue using the maximum size.
   */
  err = hsa_queue_create(agent, queue_size, HSA_QUEUE_TYPE_SINGLE,
      NULL, NULL, UINT32_MAX, UINT32_MAX, queue);
  check(Creating the queue, err);
  return 0;
}

int enqueue_packet(hsa_queue_t* queue, hsa_signal_t sign, hsail_kobj_t* pkt_info) {
  /*
   * Obtain the current queue write index.
   * TODO: Should verify if queue not full
   */
  uint64_t index = hsa_queue_load_write_index_relaxed(queue);


  // Write the aql packet at the calculated queue index address.
  const uint32_t queueMask = queue->size - 1;
  hsa_kernel_dispatch_packet_t* dispatch_packet = &(((hsa_kernel_dispatch_packet_t*)(queue->base_address))[index&queueMask]);

  // Prepare dispatch packet
  dispatch_packet->setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
  dispatch_packet->workgroup_size_x = (uint16_t)256;
  dispatch_packet->workgroup_size_y = (uint16_t)1;
  dispatch_packet->workgroup_size_z = (uint16_t)1;
  dispatch_packet->grid_size_x = (uint32_t) (1024*1024);
  dispatch_packet->grid_size_y = 1;
  dispatch_packet->grid_size_z = 1;
  dispatch_packet->completion_signal = sign;
  dispatch_packet->kernel_object = pkt_info->kernel_object;
  dispatch_packet->kernarg_address = (void*) pkt_info->kernarg_address;
  dispatch_packet->private_segment_size = pkt_info->private_segment_size;
  dispatch_packet->group_segment_size = pkt_info->group_segment_size;

  // Header generation
  uint16_t header = 0;
  header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
  header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
  header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;

  __atomic_store_n((uint16_t*)(&dispatch_packet->header), header, __ATOMIC_RELEASE);


  // Increment the write index and ring the doorbell to dispatch the kernel.
  hsa_queue_store_write_index_relaxed(queue, index+1);
  return index;
}

int destroy_hsail(hsail_runtime_t* run) {
  hsa_status_t err;

  if (free_arguments(&run->args) == 1) {
    return 1;
  }

  err = hsa_signal_destroy(run->signum);
  check(Destroying the signal, err);

  err = hsa_queue_destroy(run->queue);
  check(Destroying the queue, err);


  err = hsa_shut_down();
  check(Shutting down the runtime, err);
  return 0;
}
