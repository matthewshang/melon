#ifndef __LEXER__
#define __LEXER__

#include "token.h"
#include "charstream.h"

typedef struct
{
    charstream_t source;
    int current;
    token_t *tokens;
    int ntokens;
} lexer_t;

lexer_t lexer_create(const char *source);
void lexer_destroy(lexer_t *lexer);

token_t lexer_consume(lexer_t *lexer, token_type type, const char *msg);
token_t lexer_advance(lexer_t *lexer);
token_t lexer_peek(lexer_t *lexer);
token_t lexer_previous(lexer_t *lexer);

bool lexer_match(lexer_t *lexer, token_type type);
bool lexer_check(lexer_t *lexer, token_type type);
bool lexer_end(lexer_t *lexer);

#endif