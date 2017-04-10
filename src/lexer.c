#include "lexer.h"

#include <stdio.h>
#include <ctype.h>

#include "vector.h"

static bool is_identifier(char c)
{
    return isalpha(c) || c == '_';
}

static bool is_digit(char c)
{
    return isdigit(c);
}

static bool is_punc(char c)
{
    return c == '(' || c == ')' || c == ',' || c == '{' || c == '}' || c == ';';
}

static bool is_comment(char c)
{
    return c == '#';
}

static bool is_op(char c)
{
    return c == '+' || c == '-' || c == '=' || c == '*' || c == '%' || c == '&' || 
        c == '|' || c == '<' || c == '>' || c == '!'; 
}

static bool is_semicolon(char c)
{
    return c == ';';
}

static bool is_string(char c)
{
    return c == '"' || c == '\'';
}

static bool is_space(char c)
{
    return isspace(c);
}

static void scan_comment(charstream_t *source)
{
    while (charstream_next(source) != '\n')
    {
        if (charstream_eof(source)) break;
    }
}

static token_t scan_string(charstream_t *source)
{
    charstream_next(source);

    int start = source->offset;
    int bytes = 0;

    while (!is_string(charstream_peek(source)))
    {
        charstream_next(source);
        bytes++;
        if (charstream_eof(source)) break;
    }
    charstream_next(source);

    return token_create(TOK_STR, start, bytes);
}

static token_t scan_number(charstream_t *source)
{
    int start = source->offset;
    int bytes = 0;

    while (is_digit(charstream_peek(source)))
    {
        charstream_next(source);
        bytes++;
        if (charstream_eof(source)) break;
    }

    return token_create(TOK_NUM, start, bytes);
}

static bool strequals(const char *s1, int len, const char *s2)
{
    int i = 0;
    while (i < len)
    {
        if (s1[i] != s2[i]) return false;
        i++;
    }

    return true;
}

static token_type get_keyword(charstream_t *source, int start, int bytes)
{
    char *iden = source->buffer + start;
    if (bytes == 2)
    {
        if (strequals(iden, bytes, "if")) return TOK_IF;
    }
    if (bytes == 3)
    {
        if (strequals(iden, bytes, "var")) return TOK_VAR;
    }
    if (bytes == 4)
    {
        if (strequals(iden, bytes, "else")) return TOK_ELSE;
        if (strequals(iden, bytes, "true")) return TOK_TRUE;
    }
    if (bytes == 5)
    {
        if (strequals(iden, bytes, "print")) return TOK_PRINT;
        if (strequals(iden, bytes, "while")) return TOK_WHILE;
        if (strequals(iden, bytes, "false")) return TOK_FALSE;
    }
    return TOK_IDENTIFIER;
}

static token_t scan_identifier(charstream_t *source)
{
    int start = source->offset;
    int bytes = 0;

    while (is_identifier(charstream_peek(source)) || 
        is_digit(charstream_peek(source)))
    {
        charstream_next(source);
        bytes++;
        if (charstream_eof(source)) break;
    }

    return token_create(get_keyword(source, start, bytes), start, bytes);
}

static token_t scan_punc(charstream_t *source)
{
    char c = *source->pos;
    charstream_next(source);
    return token_create(token_punc(c), source->offset - 1, 1);
}

static token_type get_op(charstream_t *source, int start, int bytes)
{
    if (bytes == 1)
    {
        char c = source->buffer[start];
        if (c == '=') return TOK_EQ;
        if (c == '!') return TOK_BANG;
        if (c == '+') return TOK_ADD;
        if (c == '-') return TOK_SUB;
        if (c == '*') return TOK_MUL;
        if (c == '%') return TOK_MOD;
        if (c == '<') return TOK_LT;
        if (c == '>') return TOK_GT;
    }
    else if (bytes == 2)
    {
        char c1 = source->buffer[start];
        char c2 = source->buffer[start + 1];
        if (c1 == '=' && c2 == '=') return TOK_EQEQ;
        if (c1 == '!' && c2 == '=') return TOK_NEQ;
        if (c1 == '&' && c2 == '&') return TOK_AND;
        if (c1 == '|' && c2 == '|') return TOK_OR;
        if (c1 == '<' && c2 == '=') return TOK_LTE;
        if (c1 == '>' && c2 == '=') return TOK_GTE;
    }
    return TOK_OP;
}

static token_t scan_op(charstream_t *source)
{
    int start = source->offset;
    int bytes = 0;

    while (is_op(charstream_peek(source)))
    {
        charstream_next(source);
        bytes++;
        if (charstream_eof(source)) break;
    }

    return token_create(get_op(source, start, bytes), start, bytes);
}

static token_t read_next(lexer_t *lexer)
{
    token_t token = token_error();
    if (charstream_eof(&lexer->source)) token = token_create(TOK_EOF, -1, 0);

    while (!charstream_eof(&lexer->source))
    {
        char c = charstream_peek(&lexer->source);


        if (is_space(c)) { charstream_next(&lexer->source); continue; }
        if (is_comment(c)) { scan_comment(&lexer->source); continue; }

        if (is_string(c)) { token = scan_string(&lexer->source); break; }
        if (is_digit(c)) { token = scan_number(&lexer->source); break; }
        if (is_identifier(c)) { token = scan_identifier(&lexer->source); break; }
        if (is_punc(c)) { token = scan_punc(&lexer->source); break; }
        if (is_op(c)) { token = scan_op(&lexer->source); break; }

        charstream_error(&lexer->source, "Can't handle character");
        charstream_next(&lexer->source);
    }

    return token;
}

lexer_t lexer_create(const char *source)
{
    lexer_t lexer;
    lexer.source = charstream_create(source);

    vector_t(token_t) tokens;
    vector_init(tokens);
    token_t current = read_next(&lexer);
    while (current.type != TOK_EOF)
    {
        //printf("arg: %d %.*s\n", current.type, current.length, source + current.offset);
        vector_push(token_t, tokens, current);
        current = read_next(&lexer);
    }

    lexer.ntokens = vector_size(tokens);
    lexer.tokens = tokens.a;
    lexer.current = 0;

    return lexer;
}

void lexer_destroy(lexer_t *lexer)
{
    if (lexer)
    {
        if (lexer->tokens) free(lexer->tokens);
    }
}

token_t lexer_consume(lexer_t *lexer, token_type type, const char *msg)
{
    if (lexer_check(lexer, type)) return lexer_advance(lexer);
    token_t error = lexer_peek(lexer);
    printf("Error: expected token %s but got token %.*s\n", 
        msg, error.length, lexer->source.buffer + error.offset);
    return token_error();
}

token_t lexer_advance(lexer_t *lexer)
{
    return lexer->tokens[lexer->current++];
}

token_t lexer_peek(lexer_t *lexer)
{
    return lexer->tokens[lexer->current];
}

token_t lexer_previous(lexer_t * lexer)
{
    return lexer->tokens[lexer->current - 1];
}

bool lexer_match(lexer_t * lexer, token_type type)
{
    if (lexer_check(lexer, type)) 
    {
        lexer_advance(lexer);
        return true;
    }
    return false;
}

bool lexer_check(lexer_t *lexer, token_type type)
{
    return lexer_peek(lexer).type == type;
}

bool lexer_end(lexer_t *lexer)
{
    return lexer->current >= lexer->ntokens;
}