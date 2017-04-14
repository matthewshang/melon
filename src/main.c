#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ast.h"
#include "codegen.h"
#include "debug.h"
#include "utils.h"
#include "lexer.h"
#include "vector.h"
#include "vm.h"
#include "parser.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: lang.exe <source>\n");
        return 0;
    }

    const char *file = file_read(argv[1]);

    if (file)
    {
        lexer_t lexer = lexer_create(file);
        node_t *ast = parse(&lexer);
        //ast_print(ast);
     
        function_t *main_func = function_new(strdup("$main"));
        codegen_t gen = codegen_create(main_func);
        codegen_run(&gen, ast);

        //function_disassemble(main_func);
        //function_cpool_dump(main_func);

        vm_t vm = vm_create(main_func);

        ast_free(ast);
        codegen_destroy(&gen);
        lexer_destroy(&lexer);
        free(file);

        vm_run(&vm);
        vm_destroy(&vm);

        function_free(main_func);
    }
    else
    {
        printf("Could not load file %s\n", argv[1]);
    }

	return 0;
}