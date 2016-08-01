#ifndef HSAIL_RUNTIME_H_
#define HSAIL_RUNTIME_H_

#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

typedef struct {
  hsa_agent_t agent;
  hsa_ext_finalizer_1_00_pfn_t table_1_00;
  hsa_profile_t profile;
  hsa_queue_t* queue;
} hsail_runtime_t;

typedef struct {
  uint64_t kernel_object;
  uint32_t kernarg_segment_size;
  uint32_t group_segment_size;
  uint32_t private_segment_size;
  void* kernarg_address;
} hsail_kobj_t;

int initialize_hsail(hsail_runtime_t* run);
int create_queue(hsa_agent_t agent, hsa_queue_t** queue);
int create_program(hsa_ext_program_t* program, hsail_runtime_t* run);
int extract_symbol(hsail_kobj_t* kobj_info, hsa_executable_symbol_t symbol);
int queue_packet(hsa_queue_t* queue, hsa_signal_t sign, hsail_kobj_t kobj_info);

#endif
