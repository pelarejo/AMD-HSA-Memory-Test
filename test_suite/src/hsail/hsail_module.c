#include "hsail_module.h"
#include "hsail_memory.h"
#include "tools.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Name is assumed to fit easely in a 512 buffer
int new_hsail_module(hsail_module_t** list, char *name) {
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

int destroy_hsail_modules(hsail_module_t* list) {
  int err = HSA_STATUS_SUCCESS;
  while (list != NULL) {
    hsail_module_t* elem = list;
    list = list->next;
    free(elem->name);
    if (free_kernarg(elem->pkt_info.kernarg_address) == 1) {
      err = HSA_STATUS_ERROR; // Doesn't reset
    }
    free((char*)elem->module);
    free(elem);
  }
  check(Freeing kernel argument memory buffer, err);
  return (err == HSA_STATUS_SUCCESS) ? 0 : 1;
}
