/*
 * Default fprintf-family wrapper.
 *
 * This file is the real home of the public fprintf() wrapper.
 */

#include "printf/__printf_stream_api.h"

int
fprintf(FILE *stream, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_stream(stream, fmt, ap);
    va_end(ap);

    return i;
}
