set(SOURCE_FILES main.c ast.c astwalker.c charstream.c clioptions.c codegen.c 
    core.c debug.c hash.c lexer.c parser.c semantic.c symtable.c token.c utils.c value.c vm.c)

add_definitions(-Wall)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(melon ${SOURCE_FILES})

# Link with libm
target_link_libraries(melon m)