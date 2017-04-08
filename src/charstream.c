#include "charstream.h"

#include <stdio.h>

charstream_t charstream_create(const char *source)
{
    charstream_t stream;
    stream.buffer = source;
    stream.pos = stream.buffer;
    stream.line = 1;
    stream.col = 0;
    stream.offset = 0;
    return stream;
}

char charstream_next(charstream_t *stream)
{
    char ch = *stream->pos++;
    stream->offset++;
    if (ch == '\n')
    {
        stream->line++;
        stream->col = 0;
    }
    else
    {
        stream->col++;
    }
    return ch;
}

char charstream_peek(charstream_t *stream)
{
    return *stream->pos;
}

bool charstream_eof(charstream_t *stream)
{
    return charstream_peek(stream) == '\0';
}

void charstream_error(charstream_t *stream, const char *msg)
{
    printf("%s (%d:%d) [%c]\n", msg, stream->line, stream->col, *stream->pos);
}