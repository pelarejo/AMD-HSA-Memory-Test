#include "hsail_module.h"
#include "tools.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Name is assumed to fit easely in a 512 buffer
int new_test_module(hsail_module_t** list, char *name) {
  char buff[512];

  hsail_module_t* elem = malloc(sizeof(hsail_module_t));
  if (elem == NULL) return failed("Module allocation problem");
  elem->name = strdup(name);
  elem->next = NULL;

  sprintf(buff, "./hsail/%s.brig", elem->name);
  if (load_module_from_file(buff, &(elem->module)) != 0) {
    return failed("Could not load file");
  }

  if (*list == NULL) {
    *list = elem;
  } else {
    hsail_module_t* tmp = *list;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = elem;
  }
  return 0;
}

int destroy_test_modules(hsail_module_t* list) {
  hsa_status_t err = HSA_STATUS_SUCCESS;
  hsa_status_t tmp;
  while (list != NULL) {
    hsail_module_t* elem = list;
    list = list->next;
    free(elem->name);
    tmp = hsa_memory_free(elem->pkt_info.kernarg_address);
    if (err == HSA_STATUS_SUCCESS) err = tmp;
    free((char*)elem->module);
    free(elem);
  }
  check(Freeing kernel argument memory buffer, err);
  return err;
}
