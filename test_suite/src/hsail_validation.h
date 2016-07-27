#ifndef HSAIL_VALIDATION_H_
#define HSAIL_VALIDATION_H_

#include "hsa/hsa.h"

hsa_status_t get_gpu_agent(hsa_agent_t agent, void *data);
hsa_status_t get_kernarg_memory_region(hsa_region_t region, void* data);
hsa_status_t get_fine_grained_memory_region(hsa_region_t region, void* data);

#endif
