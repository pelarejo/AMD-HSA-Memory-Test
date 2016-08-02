#ifndef HSAIL_RUNTIME_H_
#define HSAIL_RUNTIME_H_

#include "hsail_helper.h"
#include "hsail_memory.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"

typedef struct {
  hsa_agent_t agent;
  hsa_profile_t profile;
  hsa_machine_model_t machine_model;
  hsa_ext_finalizer_1_00_pfn_t table_1_00;
  hsa_signal_t signum;
  hsail_kargs_t args;
  hsa_queue_t* queue;
} hsail_runtime_t;

int initialize_hsail(hsail_runtime_t* run);
int initialize_queue(hsa_agent_t agent, hsa_queue_t** queue);
int enqueue_packet(hsa_queue_t* queue, hsa_signal_t sign, hsail_kobj_t* pkt_info);
int destroy_hsail(hsail_runtime_t* run);

#endif
