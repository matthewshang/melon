#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ast.h"
#include "clioptions.h"
#include "codegen.h"
#include "core.h"
#include "debug.h"
#include "utils.h"
#include "lexer.h"
#include "semantic.h"
#include "vector.h"
#include "vm.h"
#include "parser.h"

int melon_compile(const char *file, function_t *func, cli_options_t *options)
{
    if (!file) return 1;
    lexer_t lexer = lexer_create(file);
    if (lexer.nerrors > 0) goto lexer_abort;

    node_t *ast = parse(&lexer);
    if (lexer.nerrors > 0) goto compile_abort;

    if (options->c_print_ast) ast_print(ast);

    if (!semantic_process(ast, &lexer)) goto compile_abort;

    codegen_t gen = codegen_create(func);
    codegen_run(&gen, ast);

    if (options->c_func_disasm) function_disassemble(func);
    if (options->c_dump_cpool) function_cpool_dump(func);

    codegen_destroy(&gen);
    ast_free(ast);
    lexer_destroy(&lexer);
    return 0;

compile_abort:
    ast_free(ast);
lexer_abort:
    lexer_destroy(&lexer);
    printf("\nmelon fatal  : Errors in compilation\n");
    return 1;
}

int main(int argc, char **argv)
{
    cli_options_t options = parse_cli_options(argc, argv);
    if (options.opt_exit) goto abort_file;

    const char *file = file_read(options.c_input);
    if (!file)
    {
        printf("melon fatal : Could not load file at %s\n", options.c_input);
        goto abort_file;
    }

    function_t *main_func = function_new(strdup("$main"));
    if (melon_compile(file, main_func, &options)) goto abort_compile;
     
    if (options.r_run)
    {
        double start = milliseconds();
        vm_t vm = vm_create(main_func);
        vm_run(&vm);

        double time = milliseconds() - start;
        printf("melon run time: %f ms\n", time);

        vm_destroy(&vm);
        core_free();
    }

abort_compile:
    function_free(main_func);
abort_file:
	return 0;
}