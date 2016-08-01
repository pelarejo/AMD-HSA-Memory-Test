#ifndef HSAIL_HELPER_H_
#define HSAIL_HELPER_H_

#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

hsa_status_t get_gpu_agent(hsa_agent_t agent, void *data);
hsa_status_t get_kernarg_memory_region(hsa_region_t region, void* data);
hsa_status_t get_fine_grained_memory_region(hsa_region_t region, void* data);
int load_module_from_file(const char* file_name, hsa_ext_module_t* module);

#endif
