#include "hsail_finalize.h"
#include "hsail_helper.h"
#include "tools.h"
#include "hsa/hsa.h"
#include "hsa/hsa_ext_finalize.h"
#include <stdio.h>
#include <string.h>

int compile_code_object(hsail_module_t* list,
  hsail_runtime_t* run,
  hsail_finalize_t* fin) {

    hsa_status_t err;
    hsa_ext_program_t program;
    // Create program
    memset(&program, 0, sizeof(hsa_ext_program_t));
    err = run->table_1_00.hsa_ext_program_create(run->machine_model, run->profile,
        HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT, NULL, &program);
    check(Create the program, err);

    // Add the BRIG module to hsa program.
    hsail_module_t* tmp = list;
    while (tmp != NULL && err == HSA_STATUS_SUCCESS) {
       err = run->table_1_00.hsa_ext_program_add_module(program, tmp->module);
       tmp = tmp->next;
    }
    check(Adding the brig module to the program, err);

    // Determine the agents ISA.
    hsa_isa_t isa;
    err = hsa_agent_get_info(run->agent, HSA_AGENT_INFO_ISA, &isa);
    check(Query the agents isa, err);

    // Finalize the program and extract the code object.
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    err = run->table_1_00.hsa_ext_program_finalize(program, isa, 0, control_directives,
        "", HSA_CODE_OBJECT_TYPE_PROGRAM, &(fin->code_object));
    check(Finalizing the program, err);

    // Destroy the program, it is no longer needed.
    err = run->table_1_00.hsa_ext_program_destroy(program);
    check(Destroying the program, err);
}

int generate_executable(hsail_runtime_t* run, hsail_finalize_t* fin) {
   // Create the empty executable.
  hsa_status_t err;
  err = hsa_executable_create(run->profile, HSA_EXECUTABLE_STATE_UNFROZEN,
    "", &(fin->executable));
  check(Create the executable, err);

  // Load the code object.
  err = hsa_executable_load_code_object(fin->executable, run->agent, fin->code_object, "");
  check(Loading the code object, err);

  // Freeze the executable; it can now be queried for symbols.
  err = hsa_executable_freeze(fin->executable, "");
  check(Freeze the executable, err);
  return 0;
}

// Assuming list.name fits in 512 easely
int extract_dispatch_info(hsail_module_t* list,
  hsail_runtime_t* run,
  hsail_finalize_t* fin) {

    /*
    * Extract the symbol from the executable
    * & extract dispatch information from the symbol.
    */
    char buff[512];
    hsa_status_t err = HSA_STATUS_SUCCESS;
    hsail_module_t* tmp = list;
    while (tmp != NULL && err == HSA_STATUS_SUCCESS) {
      sprintf(buff, "&__%s_kernel", tmp->name);
      // deprecated but get_symbol_by_name doesn't exists
      hsa_executable_symbol_t symbol;
      err = hsa_executable_get_symbol(fin->executable, NULL, buff,
        run->agent, 0, &symbol);
      err = get_symbol_info(symbol, &(tmp->pkt_info));
      tmp = tmp->next;
    }
    check(Extract the symbol information from the executable, err);
}

int finalize_modules(hsail_module_t* list,
  hsail_runtime_t* run,
  hsail_finalize_t* fin) {

    compile_code_object(list, run, fin);
    generate_executable(run, fin);
    extract_dispatch_info(list, run, fin);

    return 0;
}
