#ifndef HSAIL_HELPER_H_
#define HSAIL_HELPER_H_

#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

typedef struct {
  uint64_t kernel_object;
  uint32_t kernarg_segment_size;
  uint32_t group_segment_size;
  uint32_t private_segment_size;
  void* kernarg_address;
} hsail_kobj_t;

hsa_status_t get_gpu_agent(hsa_agent_t agent, void *data);
hsa_status_t get_kernarg_memory_region(hsa_region_t region, void* data);
hsa_status_t get_fine_grained_memory_region(hsa_region_t region, void* data);
hsa_status_t load_module_from_file(const char* file_name, hsa_ext_module_t* module);
hsa_status_t get_symbol_info(hsa_executable_symbol_t symbol, hsail_kobj_t* pkt_info);

#endif
