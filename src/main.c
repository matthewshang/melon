#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ast.h"
#include "codegen.h"
#include "debug.h"
#include "utils.h"
#include "lexer.h"
#include "token.h"
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
        codegen_t gen = codegen_create();
        codegen_run(&gen, ast);

        //disassemble_code(&gen.code);
      
        vm_t vm = vm_create(&gen.code, gen.constants);
        vm_run(&vm);
        vm_destroy(&vm);

        ast_free(ast);
        codegen_destroy(&gen);
        lexer_destroy(&lexer);
        free(file);
    }
    else
    {
        printf("Could not load file\n");
    }

	return 0;
}