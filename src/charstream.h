#ifndef __CHARSTREAM__
#define __CHARSTREAM__

#include <stdbool.h>

typedef struct
{
    const char *buffer;
    char *pos;
    unsigned int line, col, offset;
} charstream_t;

charstream_t charstream_create(const char *source);

char charstream_next(charstream_t *stream);
char charstream_peek(charstream_t *stream);
bool charstream_eof(charstream_t *stream);
void charstream_error(charstream_t *stream, const char *msg);

#endif