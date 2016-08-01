#include "hsail_memory.h"
#include "hsail_helper.h"
#include "tools.h"
#include <string.h>

const int KERNARG_ALLOC_SIZE = 1024*1024*4;

int allocate_arguments(hsail_runtime_t* run, hsail_kargs_t* args) {
  hsa_status_t err;
  hsa_region_t finegrained_region;
  finegrained_region.handle=(uint64_t)-1;
  hsa_agent_iterate_regions(run->agent, get_fine_grained_memory_region, &finegrained_region);
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
  reset_arguments(args);

  return 0;
}

int reset_arguments(hsail_kargs_t* args) {
  memset(args->in, 1, KERNARG_ALLOC_SIZE);
  memset(args->out, 0, KERNARG_ALLOC_SIZE);
}

int allocate_kernarg(hsail_runtime_t* run, hsail_kargs_t* args, hsail_kobj_t* pkt_info) {
  hsa_status_t err;
  /*
   * Find a memory region that supports kernel arguments.
   */
  hsa_region_t kernarg_region;
  kernarg_region.handle=(uint64_t)-1;
  hsa_agent_iterate_regions(run->agent, get_kernarg_memory_region, &kernarg_region);
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
