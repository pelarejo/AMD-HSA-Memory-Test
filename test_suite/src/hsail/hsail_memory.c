#include "hsail_memory.h"
#include "hsail_helper.h"
#include "tools.h"
#include <string.h>

const int KERNARG_ALLOC_SIZE = 1024*1024*4;

int reset_arguments(hsail_kargs_t* args) {
  memset(args->in, 1, KERNARG_ALLOC_SIZE);
  memset(args->out, 0, KERNARG_ALLOC_SIZE);
}

int allocate_arguments(hsa_agent_t agent, hsail_kargs_t* args) {
  hsa_status_t err;
  hsa_region_t finegrained_region;
  finegrained_region.handle=(uint64_t)-1;
  hsa_agent_iterate_regions(agent, get_fine_grained_memory_region, &finegrained_region);
  err = (finegrained_region.handle == (uint64_t)-1) ? HSA_STATUS_ERROR : HSA_STATUS_SUCCESS;
  check(Finding a fine grained memory region, err);

  /*
   * Allocate and initialize the kernel arguments from the fine
   * grained memory region.
   */
  char* in;
  err=hsa_memory_allocate(finegrained_region, KERNARG_ALLOC_SIZE, (void*) &in);
  check(Allocating argument memory for input parameter, err);

  char* out;
  err=hsa_memory_allocate(finegrained_region, KERNARG_ALLOC_SIZE, (void*) &out);
  check(Allocating argument memory for output parameter, err);

  args->in = in;
  args->out = out;

  return 0;
}

int allocate_kernarg(hsa_agent_t agent, hsail_kargs_t* args, hsail_kobj_t* pkt_info) {
  hsa_status_t err;
  /*
   * Find a memory region that supports kernel arguments.
   */
  hsa_region_t kernarg_region;
  kernarg_region.handle=(uint64_t)-1;
  hsa_agent_iterate_regions(agent, get_kernarg_memory_region, &kernarg_region);
  err = (kernarg_region.handle == (uint64_t)-1) ? HSA_STATUS_ERROR : HSA_STATUS_SUCCESS;
  check(Finding a kernarg memory region, err);

  /*
   * Allocate the kernel argument buffer from the correct region.
   */
  err = hsa_memory_allocate(kernarg_region,
      pkt_info->kernarg_segment_size, &pkt_info->kernarg_address);
  check(Allocating kernel argument memory buffer, err);
  memcpy(pkt_info->kernarg_address, args, sizeof(hsail_kargs_t));
}

int free_arguments(hsail_kargs_t* args) {
  hsa_status_t err;
  err = hsa_memory_free(args->in);
  check(Freeing in argument memory buffer, err);

  err = hsa_memory_free(args->out);
  check(Freeing out argument memory buffer, err);
  return 0;
}

int free_kernarg(void* kernarg_address) {
    return hsa_memory_free(kernarg_address) == HSA_STATUS_SUCCESS ? 0 : 1;
}
