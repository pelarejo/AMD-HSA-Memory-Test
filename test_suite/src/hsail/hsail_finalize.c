#include "hsail_finalize.h"
#include "hsail_helper.h"
#include "tools.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"
#include <stdio.h>
#include <string.h>

int finalize_module(test_details* list, hsail_runtime_t* run, hsail_finalize_t* fin) {
  char buff[512];
  test_details* tmp = list;
  while (tmp != NULL) {
    sprintf(buff, "./hsail/%s.brig", tmp->name);
    if (load_module_from_file(buff, &(tmp->module)) != 0) {
      return failed("Could not load file");
    }
    tmp = tmp->next;
  }

  hsa_status_t err;

  // program parts

  hsa_ext_program_t program;
  if (create_program(&program, run) != 0) {
    return failed("Could not create program");
  }

  /*
   * Add the BRIG module to hsa program.
   */
   tmp = list;
   while (tmp != NULL) {
     err = run->table_1_00.hsa_ext_program_add_module(program, tmp->module);
     if (err != HSA_STATUS_SUCCESS) {
       check(Adding the brig module to the program, err);
     }
     tmp = tmp->next;
   }
  check(Adding the brig module to the program, err);

  /*
   * Determine the agents ISA.
   */
  hsa_isa_t isa;
  err = hsa_agent_get_info(run->agent, HSA_AGENT_INFO_ISA, &isa);
  check(Query the agents isa, err);

  /*
   * Finalize the program and extract the code object.
   */
  hsa_ext_control_directives_t control_directives;
  memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
  err = run->table_1_00.hsa_ext_program_finalize(program, isa, 0, control_directives,
      "", HSA_CODE_OBJECT_TYPE_PROGRAM, &(fin->code_object));
  check(Finalizing the program, err);

  /*
   * Destroy the program, it is no longer needed.
   */
  err = run->table_1_00.hsa_ext_program_destroy(program);
  check(Destroying the program, err);

  /*
   * Create the empty executable.
   */
  err = hsa_executable_create(run->profile, HSA_EXECUTABLE_STATE_UNFROZEN,
      "", &(fin->executable));
  check(Create the executable, err);

  /*
   * Load the code object.
   */
  err = hsa_executable_load_code_object(fin->executable, run->agent, fin->code_object, "");
  check(Loading the code object, err);

  /*
   * Freeze the executable; it can now be queried for symbols.
   */
  err = hsa_executable_freeze(fin->executable, "");
  check(Freeze the executable, err);

  /*
  * Extract the symbol from the executable.
  */
  tmp = list;
  while (tmp != NULL) {
    // deprecated but get_symbol_by_name doesn't exists
    sprintf(buff, "&__%s_kernel", tmp->name);
    err = hsa_executable_get_symbol(fin->executable, NULL, buff,
        run->agent, 0, &(tmp->symbol));
    if (err != HSA_STATUS_SUCCESS) {
      check(Extract the symbol from the executable, err);
    }
    tmp = tmp->next;
  }
  check(Extract the symbol from the executable, err);

  /*
   * Extract dispatch information from the symbol
   */
  tmp = list;
  while (tmp != NULL) {
    // deprecated but get_symbol_by_name doesn't exists
    if (extract_symbol(&(tmp->pkt_info), tmp->symbol) != 0) {
      return failed("Could not extract symbol");
    }
    tmp = tmp->next;
  }
  return 0;
}
