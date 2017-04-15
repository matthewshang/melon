#include "clioptions.h"

#include <string.h>
#include <stdio.h>

static void print_help()
{
    printf("Usage  : melon.exe [options] <inputfile>\n");
    printf("\nMelon expects input files with extension .txt\n");
    printf("\nOptions:\n========\n");
    printf("\n[--help]  (-h)\n        Prints this help text\n");
    printf("\n[--show-ast]  (-ast)\n        Prints the syntax tree generated after compilation\n");
    printf("\n[--disasm-func]  (-dasm)\n        Prints the disassembled bytecode after compilation\n");
    printf("\n[--dump-cpool]  (-cpool)\n        Prints the contents of the main function's constant pool after compilation\n");
    printf("\n[--compile-only]  (-c)\n        Skips execution of the program after compilation\n");
}

static bool is_valid_input(const char *input)
{
    size_t len = strlen(input);
    if (len < 4) return false;
    return strcmp(&input[len - 4], ".txt") == 0;
}

static bool is_option(const char *arg, const char *longhand, const char *shorthand)
{
    return strcmp(arg, shorthand) == 0 || strcmp(arg, longhand) == 0;
}

cli_options_t parse_cli_options(int argc, char **argv)
{
    cli_options_t options;
    options.opt_exit = true;
    options.c_print_ast = false;
    options.c_func_disasm = false;
    options.c_dump_cpool = false;
    options.c_input = NULL;
    options.r_run = true;

    if (argc < 2)
    {
        printf("melon fatal   : No input file specified; use option --help (-h) for more information\n");
        return options;
    }

    if (is_option(argv[1], "--help", "-h"))
    {
        print_help();
        return options;
    }

    if (is_valid_input(argv[argc - 1]))
    {
        options.opt_exit = false;
        options.c_input = argv[argc - 1];
    }
    else
    {
        printf("melon fatal   : Invalid input file specified\n");
        return options;
    }

    for (int i = 1; i < argc - 1; i++)
    {
        if (is_option(argv[i], "--show-ast", "-ast"))
        {
            options.c_print_ast = true;
        }
        else if (is_option(argv[i], "--disasm-func", "-dasm"))
        {
            options.c_func_disasm = true;
        }
        else if (is_option(argv[i], "--dump-cpool", "-cpool"))
        {
            options.c_dump_cpool = true;
        }
        else if (is_option(argv[i], "--compile-only", "-c"))
        {
            options.r_run = false;
        }
        else
        {
            printf("melon warning : Unknown option %s; use option --help (-h) for more information\n", argv[i]);
        }
    }

    return options;
}