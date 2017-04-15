#ifndef __CLIOPTIONS__
#define __CLIOPTIONS__

#include <stdbool.h>

typedef struct
{
    bool opt_exit;
    bool c_print_ast;
    bool c_func_disasm;
    bool c_dump_cpool;
    const char *c_input;
    bool r_run;
} cli_options_t;

cli_options_t parse_cli_options(int argc, char **argv);

#endif