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
  return (err == HSA_STATUS_SUCCESS) ? 0 : 1;
}

int create_queue(hsa_agent_t agent, hsa_queue_t** queue) {
  hsa_status_t err;
  /*
   * Query the maximum size of the queue.
   */
  uint32_t queue_size = 0;
  err = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_size);
  check(Querying the agent maximum queue size, err);
  verb_printf("The maximum queue size is %u.\n", (unsigned int) queue_size);

  /*
   * Create a queue using the maximum size.
   */
  err = hsa_queue_create(agent, queue_size, HSA_QUEUE_TYPE_SINGLE,
      NULL, NULL, UINT32_MAX, UINT32_MAX, queue);
  check(Creating the queue, err);
  return (err == HSA_STATUS_SUCCESS) ? 0 : 1;
}

int create_program(hsa_ext_program_t* program, hsail_runtime_t* run) {
  hsa_status_t err;
  /*
   * Obtain the agent's machine model
   * TODO: Could be done once
   */
  hsa_machine_model_t machine_model;
  err = hsa_agent_get_info(run->agent, HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
  check(Obtaining machine model, err);

  /*
   * Create hsa program.
   */
  memset(program, 0, sizeof(hsa_ext_program_t));
  err = run->table_1_00.hsa_ext_program_create(machine_model, run->profile,
      HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT, NULL, program);
  check(Create the program, err);

  return (err == HSA_STATUS_SUCCESS) ? 0 : 1;
}

int extract_symbol(hsail_kobj_t* pkt_info, hsa_executable_symbol_t symbol) {
  hsa_status_t err;

  err = hsa_executable_symbol_get_info(symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &pkt_info->kernel_object);
  check(Extracting the symbol from the executable, err);
  err = hsa_executable_symbol_get_info(symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &pkt_info->kernarg_segment_size);
  check(Extracting the kernarg segment size from the executable, err);
  err = hsa_executable_symbol_get_info(symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &pkt_info->group_segment_size);
  check(Extracting the group segment size from the executable, err);
  err = hsa_executable_symbol_get_info(symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &pkt_info->private_segment_size);
  check(Extracting the private segment from the executable, err);
  return 0;
}

int queue_packet(hsa_queue_t* queue, hsa_signal_t sign, hsail_kobj_t* pkt_info) {
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
